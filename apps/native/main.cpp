// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif
#define USE_AVX 1

#include "ui_imgui/ui.h"

#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <array>



int main() {
    Ui ui;

    bool done = false;
    while (!done) {
        ui.frame();
        done = ui.done();

    }

    
    return 0;
}
