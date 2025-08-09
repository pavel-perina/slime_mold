//! \file slime_mold_simulation.h

#pragma once

#include "common/colors.h"
#include "common/presets.h"

#include <memory>

class SlimeMoldSimulation
{
public:
    // WARNING: WIDTH*HEIGHT must be divisible by 8 due to vectorization code
    static constexpr int WIDTH  = 640;
    static constexpr int HEIGHT = 480;
    static constexpr int NUM_AGENTS = 250000;


    SlimeMoldSimulation();
    ~SlimeMoldSimulation();

    void step(const AgentPreset&);
    void reset();
    const float* data();

private:
    class Private;
    std::unique_ptr<Private> m_p;
};
