// main.cpp
#include "ui_imgui/ui.h"

#include <emscripten.h>
#include <cstdio>

static Ui* g_ui = nullptr;

void main_loop() {
    g_ui->frame();
    if (g_ui->done();) {
        emscripten_cancel_main_loop();
    }
}

int main() {
    // Initialize your UI object
    Ui ui;
    g_ui = &ui;

    // Setup Emscripten main loop
    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
