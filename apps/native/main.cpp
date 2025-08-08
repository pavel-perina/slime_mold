// slimemold.cpp
#ifdef _WIN32
#define SDL_MAIN_HANDLED
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#include "ui_imgui/ui.h"

int main()
{
    Ui ui;
    // TODO: we need to use RAII resource destruction
    //       or class destructor will crash on shutdown
    if (!ui.initialized()) {
        return -1;
    }

    bool done = false;
    while (!done) {
        ui.frame();
        done = ui.done();
    }

    return 0;
}
