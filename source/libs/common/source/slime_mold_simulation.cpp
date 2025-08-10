#include "common/slime_mold_simulation.h"
#include "common/presets.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

#if defined(USE_AVX2)
#include <immintrin.h>
#endif

#define DO_SORTING 1

namespace {

struct Agent
{
    float x, y, dx, dy;
};


inline void rotate(float& dx, float& dy, float cos_a, float sin_a)
{
    const float ndx = dx * cos_a - dy * sin_a;
    const float ndy = dx * sin_a + dy * cos_a;
    dx = ndx;
    dy = ndy;
}

} // anonymous namespace



class SlimeMoldSimulation::Private final
{
public:
    Private(size_t width, size_t height, size_t numAgents);
    inline float sampleField(float x, float y) const;
    inline void deposit(const Agent& a);
    void resetAgents();
    void diffuse(float evaporate);
    void clearField();
    void updateAgents(const AgentPreset& p);
    void sortAgents();

    size_t m_width, m_height;
    size_t m_numAgents;
    std::vector<Agent> m_agents;
    std::vector<float> m_field;
    size_t m_passes;
};


SlimeMoldSimulation::Private::Private(size_t width, size_t height, size_t numAgents)
    : m_width(width)
    , m_height(height)
    , m_numAgents(numAgents)
    , m_passes(0)
{
    m_agents.resize(numAgents);
    m_field.resize(width * height, 0.0f);
    resetAgents();
}


void SlimeMoldSimulation::Private::resetAgents()
{
    srand((unsigned)time(0));
    for (auto& a : m_agents) {
        a.x = rand() % m_width;
        a.y = rand() % m_height;
        float angle = (rand() / (float)RAND_MAX) * 2.0f * std::numbers::pi_v<float>;
        a.dx = std::cos(angle);
        a.dy = std::sin(angle);
    }
}


void SlimeMoldSimulation::Private::diffuse(float evaporate)
{
    // Evaporation only for simplicity
#if defined(USE_AVX2)
    const size_t count = m_field.size();
    constexpr size_t avxWidth = 8; // 8 floats per register
    float* data = m_field.data();
    const __m256 evaporateVec = _mm256_set1_ps(evaporate);
    for (size_t i = 0; i < count; i += avxWidth) {
        __m256 values = _mm256_loadu_ps(data + i);
        values = _mm256_mul_ps(values, evaporateVec);
        _mm256_storeu_ps(&data[i], values);
    }
#else
    for (float& v : field)
        v *= evaporate;
#endif
}


void SlimeMoldSimulation::Private::clearField()
{
    diffuse(0.0);
}


inline float SlimeMoldSimulation::Private::sampleField(float x, float y) const
{
    const int xi = ((int)(x + 0.5f) +  m_width) % m_width;
    const int yi = ((int)(y + 0.5f) + m_height) % m_height;
    const int idx = yi * m_width + xi;
    return m_field[idx];
}


inline void SlimeMoldSimulation::Private::deposit(const Agent& a) {
    const int xi = ((int)(a.x + 0.5f) +  m_width) % m_width;
    const int yi = ((int)(a.y + 0.5f) + m_height) % m_height;
    const int idx = yi * m_width + xi;
    m_field[idx] += 1.0f;
}



void SlimeMoldSimulation::Private::updateAgents(const AgentPreset &p) {
    const float SENSOR_LEFT_COS  = std::cos(-p.sensor_angle);
    const float SENSOR_LEFT_SIN  = std::sin(-p.sensor_angle);
    const float SENSOR_RIGHT_COS = std::cos(p.sensor_angle);
    const float SENSOR_RIGHT_SIN = std::sin(p.sensor_angle);

    const float TURN_LEFT_COS  = std::cos(-p.turn_angle);
    const float TURN_LEFT_SIN  = std::sin(-p.turn_angle);
    const float TURN_RIGHT_COS = std::cos(p.turn_angle);
    const float TURN_RIGHT_SIN = std::sin(p.turn_angle);

    const float sensor_dist = p.sensor_dist;
    const float step_size = p.step_size;

    for (auto& a : m_agents) {
        // Sensor positions
        const float cx = a.x + a.dx * sensor_dist;
        const float cy = a.y + a.dy * sensor_dist;

        const float ldx = a.dx * SENSOR_LEFT_COS - a.dy * SENSOR_LEFT_SIN;
        const float ldy = a.dx * SENSOR_LEFT_SIN + a.dy * SENSOR_LEFT_COS;
        const float lx = a.x + ldx * sensor_dist;
        const float ly = a.y + ldy * sensor_dist;

        const float rdx = a.dx * SENSOR_RIGHT_COS - a.dy * SENSOR_RIGHT_SIN;
        const float rdy = a.dx * SENSOR_RIGHT_SIN + a.dy * SENSOR_RIGHT_COS;
        const float rx = a.x + rdx * sensor_dist;
        const float ry = a.y + rdy * sensor_dist;

        // Sample sensors
        const float c = sampleField(cx, cy);
        const float l = sampleField(lx, ly);
        const float r = sampleField(rx, ry);


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
        if (a.x < 0)         a.x += m_width;
        if (a.x >= m_width)  a.x -= m_width;
        if (a.y < 0)         a.y += m_height;
        if (a.y >= m_height) a.y -= m_height;

    }
    for (const auto& a : m_agents) {
        deposit(a);
    }


    ++m_passes;
#if DO_SORTING
    if (m_passes % 32) {
        sortAgents();
    }
#endif

}


SlimeMoldSimulation::SlimeMoldSimulation(size_t width, size_t height, size_t numAgents)
    : m_p (std::make_unique<Private>(width, height, numAgents))
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
    return m_p->m_field.data();
}


void SlimeMoldSimulation::Private::sortAgents()
{
    // reuse vectors, resize when needed
    static std::vector<size_t> bucketCounts;
    static std::vector<size_t> bucketStartOffsets;
    static std::vector<size_t> bucketWriteOffsets;
    static std::vector<Agent> tempAgents;
    bucketCounts.resize(m_height);
    bucketStartOffsets.resize(m_height);
    bucketWriteOffsets.resize(m_height);
    tempAgents.resize(m_agents.size());

    // count occurrences per row
    std::ranges::fill(bucketCounts, 0);
    for (const auto& a : m_agents) {
        const size_t row = a.y;
        assert(row < m_height);
        ++bucketCounts[row];
    }

    // accumulate them
    bucketStartOffsets[0] = 0;
    for (size_t i = 1; i < m_height; ++i) {
        bucketStartOffsets[i] = bucketStartOffsets[i - 1] + bucketCounts[i - 1];
    }

    std::ranges::fill(bucketWriteOffsets, 0);

    // copy agents to temporary sorted by row
    for (const auto& a : m_agents) {
        const size_t row = a.y;
        const size_t writeOffset = bucketStartOffsets[row] + bucketWriteOffsets[row];
        tempAgents[writeOffset] = a;
        ++bucketWriteOffsets[row];
    }

    std::ranges::copy(tempAgents, m_agents.begin());
}
