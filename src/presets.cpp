#include "presets.h"

const std::vector<AgentPreset> presetAgents() {
    return {
        { "Winding Wines", 0.185f, 5.0f, 0.6f, 1.0f, 0.95f, 0.4f },
        {"Neural Network", 0.106f, 5.0f, 0.598f, 1.0f, 0.944f, 0.009f},
        {"Lightning Storm", 1.2f, 4.0f, 0.8f, 2.0f, 0.85f, 0.2f},
        {"Coral Growth", 0.3f, 8.0f, 0.15f, 0.8f, 0.98f, 0.7f},
        {"Galactic Web", 0.8f, 9.0f, 0.25f, 1.5f, 0.96f, 0.4f},
        {"Mycelium Madness", 0.4f, 6.0f, 0.45f, 1.2f, 0.93f, 0.6f},
        {"Circuit Meltdown", 1.0f, 3.0f, 0.7f, 2.5f, 0.88f, 0.1f},
        {"Bacterial Bloom", 0.2f, 12.0f, 0.1f, 0.6f, 0.99f, 0.8f},
        {"Quantum Entanglement", 0.6f, 7.0f, 0.35f, 1.0f, 0.95f, 0.3f},
    };
}

const std::vector<PalettePreset> presetPalettes() {
    return {
        {"Candy Shop",{ 0.31f, 0.14f, 0.33f }, { 0.87f, 0.85f, 0.65f }, { 0.54f, 0.99f, 0.77f }},
        {"Biolab", {0.12f, 0.07f, 0.15f}, {0.10f, 0.31f, 0.20f}, {0.87f, 0.93f, 0.53f}},
        {"Deep Ocean", {0.0f, 0.1f, 0.3f}, {0.2f, 0.6f, 0.8f}, {0.9f, 0.9f, 1.0f}},
        {"Electric Purple", {0.1f, 0.0f, 0.3f}, {0.5f, 0.0f, 0.8f}, {1.0f, 0.8f, 0.9f}},
        {"Neon Green", {0.0f, 0.1f, 0.0f}, {0.3f, 0.8f, 0.3f}, {0.8f, 1.0f, 0.8f}},
        {"Fire", {0.2f, 0.0f, 0.0f}, {0.8f, 0.3f, 0.0f}, {1.0f, 1.0f, 0.4f}},
        {"Cosmic", {0.0f, 0.0f, 0.1f}, {0.3f, 0.1f, 0.5f}, {0.8f, 0.6f, 1.0f}},
        {"Forest", {0.1f, 0.2f, 0.0f}, {0.4f, 0.7f, 0.2f}, {0.9f, 1.0f, 0.6f}},
        {"Sunset", {0.1f, 0.0f, 0.1f}, {0.8f, 0.4f, 0.2f}, {1.0f, 0.9f, 0.7f}},
    };
}
