#include "common/presets.h"

const std::vector<AgentPreset>& presetAgents() {
    static const std::vector<AgentPreset> presets {
        { "Winding Wines", 0.185f, 5.0f, 0.6f, 1.0f, 0.95f, 0.4f },
        { "Neural Network", 0.106f, 5.0f, 0.598f, 1.0f, 0.944f, 0.45f },
        { "Trypophobia", 0.185f, 5.0f, 0.6f, 2.25f, 0.91f, 0.35f },
        { "Circuit Meltdown", 1.1f, 3.0f, 0.7f, 2.5f, 0.88f, 0.1f },
        { "Wobbling Worms", 1.4f, 2.5f, 0.6f, 1.5f, 0.7f, .2f },
        { "Slowmo Storm", 0.42f, 4.2f, 1.0f, 1.0f, 0.9f, 0.125f },
        { "Fiery Ribbons", 1.42f, 8.0f, 0.05f, 1.0f, 0.8f, .75f },
        { "Quantum Entaglement", 2.0f, 8.6f, 0.11f, 1.0f, .95f, .6f },
        { "Converging", 1.0f, 6.3f, 0.4f, 1.5f, 0.8f, .3f },
        { "Detonating Cords", 0.14f, 7.6f, 0.45f, 1.5f, 0.7f, .4f},
    };
    return presets;
}

const std::vector<PalettePreset>& presetPalettes() {
    static const std::vector<PalettePreset> presets {
        { "Candy Shop", {{ { 0.31f, 0.14f, 0.33f }, { 0.87f, 0.85f, 0.65f }, { 0.54f, 0.99f, 0.77f }}} },
        { "Biolab", {{{0.12f, 0.07f, 0.15f}, {0.10f, 0.31f, 0.20f}, {0.87f, 0.93f, 0.53f}}} },
        { "Forest", {{{0.13f, 0.11f, 0.0f}, {0.4f, 0.7f, 0.2f}, {0.9f, 1.0f, 0.6f}}} },
        { "Deep Ocean", {{{0.0f, 0.1f, 0.3f}, {0.2f, 0.6f, 0.8f}, {0.95f, 0.95f, 0.8f}}} },
        { "Electric Purple", {{{0.1f, 0.0f, 0.3f}, {0.5f, 0.0f, 0.8f}, {1.0f, 0.8f, 0.9f}}} },
        { "Fire", {{{0.15f, 0.0f, 0.15f}, {0.8f, 0.5f, 0.3f}, {1.0f, 0.9f, 0.66f}}} },
        { "Magma", {{{ 0.22f, 0.05f, 0.27f}, { .8f, .66f,.2f}, { .985f, 0.985f, 0.7f}}} },
        { "Solarized Light", {{{ .992f, .965f, .89f}, { .522f, .6f, .0f}, { .0f, 0.17f, .212f}}} },
        { "Solarized Dark", {{{ .0f, 0.17f, .212f}, { .522f, .6f, .0f}, { .992f, .965f, .89f}}} }
    };
    return presets;
}
