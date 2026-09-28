#pragma once

#include <windows.h>

class AppState;

class Gui {
public:
    Gui() = default;
    ~Gui();

    bool initialize(HINSTANCE instance, AppState& app);
    int run();
    void shutdown();

private:
    static LRESULT WINAPI wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

    bool create_device(HWND hwnd);
    void create_render_target();
    void cleanup_device();
    void setup_theme();
    void render();

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    AppState* app_ = nullptr;

    bool capture_toggle_hotkey_ = false;
    bool capture_inventory_hotkey_ = false;

    struct ID3D11Device* device_ = nullptr;
    struct ID3D11DeviceContext* context_ = nullptr;
    struct IDXGISwapChain* swap_chain_ = nullptr;
    struct ID3D11RenderTargetView* render_target_ = nullptr;
};
