//! \file slime_mold_simulation.h

#pragma once

#include "common/presets.h"

#include <memory>

class SlimeMoldSimulation final
{
public:
    // WARNING: WIDTH*HEIGHT must be divisible by 8 due to vectorization code
    SlimeMoldSimulation(size_t width, size_t height, size_t numAgents);
    ~SlimeMoldSimulation();

    void step(const AgentPreset&);
    void reset();
    const float* data();

private:
    class Private;
    std::unique_ptr<Private> m_p;
};
