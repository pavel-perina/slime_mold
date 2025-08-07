#include "common/slime_mold.h"
#include "common/presets.h"

#include <cmath>
#include <vector>
#include <numbers>

#if USE_AVX
#include <immintrin.h>
#endif

namespace {

struct Agent {
    float x, y, dx, dy;
};


inline void rotate(float& dx, float& dy, float cos_a, float sin_a) {
    float ndx = dx * cos_a - dy * sin_a;
    float ndy = dx * sin_a + dy * cos_a;
    dx = ndx;
    dy = ndy;
}

} // anonymous namespace



class SlimeMoldSimulation::Private
{
public:
    Private();
    inline float sampleField(float x, float y) const;
    inline void deposit(Agent& a);
    std::vector<Agent> agents;
    std::vector<float> field;
    void resetAgents();
    void diffuse(float evaporate);
    void clearField();
    void updateAgents(const AgentPreset& p);
};

SlimeMoldSimulation::Private::Private()
{
    agents.resize(NUM_AGENTS);
    field.resize(WIDTH * HEIGHT, 0.0f);
    resetAgents();
}


void SlimeMoldSimulation::Private::resetAgents()
{
    srand((unsigned)time(0));
    for (auto& a : agents) {
        a.x = rand() % WIDTH;
        a.y = rand() % HEIGHT;
        float angle = (rand() / (float)RAND_MAX) * 2.0f * std::numbers::pi_v<float>;
        a.dx = std::cos(angle);
        a.dy = std::sin(angle);
    }
}


void SlimeMoldSimulation::Private::diffuse(float evaporate)
{
    // Evaporation only for simplicity
#if USE_AVX
    const size_t count = field.size();
    constexpr size_t avxWidth = 8; // 8 floats per register
    float* data = field.data();
    const __m256 evaporateVec = _mm256_set1_ps(evaporate);
    for (size_t i = 0; i < count; i += avxWidth) {
        __m256 values = _mm256_loadu_ps(data + i);
        values = _mm256_mul_ps(values, evaporateVec);
        _mm256_storeu_ps(&data[i], values);
    }
#else
    for (auto& v : field)
        v *= evaporate;
#endif
}


void SlimeMoldSimulation::Private::clearField()
{
    for (auto& v : field)
        v = 0.0f;
}


float SlimeMoldSimulation::Private::sampleField(float x, float y) const
{
    int xi = ((int)(x + 0.5f) + WIDTH) % WIDTH;
    int yi = ((int)(y + 0.5f) + HEIGHT) % HEIGHT;
    int idx = yi * WIDTH + xi;
    return field[idx];
}


void SlimeMoldSimulation::Private::deposit(Agent& a) {
    int xi = ((int)(a.x + 0.5f) + WIDTH) % WIDTH;
    int yi = ((int)(a.y + 0.5f) + HEIGHT) % HEIGHT;
    int idx = yi * WIDTH + xi;
    field[idx] += 1.0f;
}



void SlimeMoldSimulation::Private::updateAgents(const AgentPreset &p) {
    const float SENSOR_LEFT_COS = std::cos(-p.sensor_angle);
    const float SENSOR_LEFT_SIN = std::sin(-p.sensor_angle);
    const float SENSOR_RIGHT_COS = std::cos(p.sensor_angle);
    const float SENSOR_RIGHT_SIN = std::sin(p.sensor_angle);

    const float TURN_LEFT_COS = std::cos(-p.turn_angle);
    const float TURN_LEFT_SIN = std::sin(-p.turn_angle);
    const float TURN_RIGHT_COS = std::cos(p.turn_angle);
    const float TURN_RIGHT_SIN = std::sin(p.turn_angle);

    const float sensor_dist = p.sensor_dist;
    const float step_size = p.step_size;

    for (auto& a : agents) {
        // Sensor positions
        float cx = a.x + a.dx * sensor_dist;
        float cy = a.y + a.dy * sensor_dist;

        float ldx = a.dx * SENSOR_LEFT_COS - a.dy * SENSOR_LEFT_SIN;
        float ldy = a.dx * SENSOR_LEFT_SIN + a.dy * SENSOR_LEFT_COS;
        float lx = a.x + ldx * sensor_dist;
        float ly = a.y + ldy * sensor_dist;

        float rdx = a.dx * SENSOR_RIGHT_COS - a.dy * SENSOR_RIGHT_SIN;
        float rdy = a.dx * SENSOR_RIGHT_SIN + a.dy * SENSOR_RIGHT_COS;
        float rx = a.x + rdx * sensor_dist;
        float ry = a.y + rdy * sensor_dist;

        // Sample sensors
        float c = sampleField(cx, cy);
        float l = sampleField(lx, ly);
        float r = sampleField(rx, ry);


        // Adjust angle
        if (c > l && c > r) {
            // keep direction
        }
        else if (l > r) {
            rotate(a.dx, a.dy, TURN_LEFT_COS, TURN_LEFT_SIN);
        }
        else if (r > l) {
            rotate(a.dx, a.dy, TURN_RIGHT_COS, TURN_RIGHT_SIN);
        }
        else {
            if (rand() % 2) {
                rotate(a.dx, a.dy, TURN_LEFT_COS, TURN_LEFT_SIN);
            }
            else {
                rotate(a.dx, a.dy, TURN_RIGHT_COS, TURN_RIGHT_SIN);
            }
        }

        // Move
        a.x += a.dx * step_size;
        a.y += a.dy * step_size;

        // Wrap around
        if (a.x < 0) a.x += WIDTH;
        if (a.x >= WIDTH) a.x -= WIDTH;
        if (a.y < 0) a.y += HEIGHT;
        if (a.y >= HEIGHT) a.y -= HEIGHT;

        deposit(a);
    }
}


SlimeMoldSimulation::SlimeMoldSimulation()
    : m_p (std::make_unique<Private>())
{
}


SlimeMoldSimulation::~SlimeMoldSimulation() = default;


void SlimeMoldSimulation::step(const AgentPreset &p)
{
    m_p->updateAgents(p);
    m_p->diffuse(p.evaporate);
}


void SlimeMoldSimulation::reset()
{
    m_p->clearField();
    m_p->resetAgents();
}


const float * SlimeMoldSimulation::data()
{
    return m_p->field.data();
}
