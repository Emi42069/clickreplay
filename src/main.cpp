#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "AppState.h"
#include "gui/Gui.h"

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE, LPSTR, int) {
    AppState app;
    Gui gui;

    if (!gui.initialize(h_instance, app)) {
        MessageBoxA(nullptr,
                    "Failed to initialize Click Replay.",
                    "Click Replay",
                    MB_ICONERROR);
        return 1;
    }

    const int result = gui.run();
    gui.shutdown();
    return result;
}
