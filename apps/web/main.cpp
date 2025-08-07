// main.cpp
#include "ui_imgui/ui.h"

#include <emscripten.h>
#include <cstdio>

static Ui* g_ui = nullptr;
static bool g_done = false;

void main_loop() {
    if (g_done) {
        emscripten_cancel_main_loop();
        return;
    }
    g_ui->frame();
    g_done = g_ui->done();
}

int main() {
    // Initialize your UI object
    Ui ui;
    g_ui = &ui;

    // Setup Emscripten main loop
    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
