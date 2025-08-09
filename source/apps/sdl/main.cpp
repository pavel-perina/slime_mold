//! \file main.cpp

#ifdef _WIN32
// Open without console on Windows
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#include "ui_imgui/ui.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static Ui* g_ui = nullptr;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    g_ui = new Ui{};
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    g_ui->frame();
    return g_ui->done()
        ? SDL_APP_SUCCESS
        : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    // Handle events directly if needed
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    delete g_ui;
    g_ui = nullptr;
}
