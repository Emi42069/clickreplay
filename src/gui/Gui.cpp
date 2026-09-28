// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Gui.h"

#include "../AppState.h"
#include "../input/Input.h"
#include "Dashboard.h"
#include "DatasetPage.h"
#include "RecorderPage.h"
#include "AnalyticsPage.h"
#include "SettingsPage.h"
#include "LogsPage.h"

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#include <d3d11.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "comdlg32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd,
    UINT message,
    WPARAM w_param,
    LPARAM l_param);

namespace {
Gui* g_gui = nullptr;
}

Gui::~Gui() {
    shutdown();
}

bool Gui::initialize(HINSTANCE instance, AppState& app) {
    instance_ = instance;
    app_ = &app;
    g_gui = this;

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_CLASSDC;
    window_class.lpfnWndProc = wnd_proc;
    window_class.hInstance = instance_;
    window_class.lpszClassName = L"ClickReplayWindow";

    if (!RegisterClassExW(&window_class)) {
        g_gui = nullptr;
        return false;
    }

    hwnd_ = CreateWindowW(window_class.lpszClassName,
                          L"Click Replay",
                          WS_OVERLAPPEDWINDOW,
                          100,
                          100,
                          1200,
                          800,
                          nullptr,
                          nullptr,
                          instance_,
                          nullptr);
    if (!hwnd_) {
        UnregisterClassW(window_class.lpszClassName, instance_);
        g_gui = nullptr;
        return false;
    }

    if (!create_device(hwnd_)) {
        shutdown();
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    setup_theme();

    if (!ImGui_ImplWin32_Init(hwnd_) || !ImGui_ImplDX11_Init(device_, context_)) {
        shutdown();
        return false;
    }

    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);

    if (!app.input.install_mouse_hook()) {
        app.statistics.log("Warning: unable to install the mouse hook.");
    }

    app.statistics.log("Click Replay GUI ready.");
    app.statistics.log("Select a CSV dataset, then press START REPLAY.");
    app.statistics.log("Default replay toggle: F6.");
    app.statistics.log("Default inventory pause: R.");
    app.statistics.log("Default recording toggle: F7.");
    app.statistics.log("Timing: absolute deadlines / DOWN-to-DOWN.");

    return true;
}

int Gui::run() {
    MSG message{};
    bool quit = false;

    while (!quit) {
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);

            if (message.message == WM_QUIT) {
                quit = true;
            }
        }

        if (app_ && app_->recording_config.toggle_hotkey_enabled && !capture_recording_hotkey_ &&
            app_->input.key_just_pressed(app_->recording_config.toggle_key,
                                         recording_toggle_was_down_)) {
            const bool replay_running =
                app_->replay.snapshot().status != ReplayStatus::stopped;

            if (replay_running) {
                app_->statistics.log("Stop the replay before starting a recording.");
            } else if (app_->recorder.snapshot().status == RecordingStatus::recording) {
                app_->recorder.stop();
            } else {
                app_->recorder.start();
            }
        }

        if (quit) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        render();

        ImGui::Render();

        const float clear_color[] = {0.025f, 0.028f, 0.036f, 1.0f};
        context_->OMSetRenderTargets(1, &render_target_, nullptr);
        context_->ClearRenderTargetView(render_target_, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swap_chain_->Present(1, 0);
    }

    return 0;
}

void Gui::shutdown() {
    if (!hwnd_ && !device_) {
        return;
    }

    if (app_) {
        app_->replay.stop();
        app_->input.uninstall_mouse_hook();
    }

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    cleanup_device();

    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }

    if (instance_) {
        UnregisterClassW(L"ClickReplayWindow", instance_);
    }

    app_ = nullptr;
    instance_ = nullptr;
    g_gui = nullptr;
}

bool Gui::create_device(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = hwnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL feature_level{};

    const HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION,
        &swap_chain_desc,
        &swap_chain_,
        &device_,
        &feature_level,
        &context_);

    if (FAILED(result)) {
        return false;
    }

    create_render_target();
    return render_target_ != nullptr;
}

void Gui::create_render_target() {
    ID3D11Texture2D* back_buffer = nullptr;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
        return;
    }

    device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_);
    back_buffer->Release();
}

void Gui::cleanup_device() {
    if (render_target_) {
        render_target_->Release();
        render_target_ = nullptr;
    }

    if (swap_chain_) {
        swap_chain_->Release();
        swap_chain_ = nullptr;
    }

    if (context_) {
        context_->Release();
        context_ = nullptr;
    }

    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
}

void Gui::setup_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 7.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 7.0f;
    style.WindowPadding = ImVec2(18, 18);
    style.FramePadding = ImVec2(10, 7);
    style.ItemSpacing = ImVec2(10, 9);
    style.ItemInnerSpacing = ImVec2(8, 6);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(.92f, .94f, .98f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(.48f, .52f, .60f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(.035f, .040f, .050f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(.045f, .050f, .062f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(.055f, .060f, .075f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(.12f, .14f, .18f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(.075f, .085f, .105f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(.10f, .115f, .145f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(.12f, .135f, .17f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(.035f, .040f, .050f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(.035f, .040f, .050f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(.10f, .12f, .16f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(.14f, .17f, .22f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(.17f, .20f, .27f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(.09f, .11f, .15f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(.13f, .16f, .21f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(.16f, .19f, .25f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(.35f, .70f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(.30f, .65f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(.42f, .75f, 1.0f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(.12f, .14f, .18f, 1.0f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(.25f, .50f, .75f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(.07f, .08f, .105f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(.12f, .16f, .22f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(.10f, .14f, .19f, 1.0f);
}

void Gui::render() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

    ImGui::Begin("##MainWindow",
                 nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::BeginChild("Sidebar", ImVec2(190, 0), true);
    ImGui::Spacing();
    ImGui::Text("CLICK");
    ImGui::TextColored(ImVec4(.35f, .70f, 1.0f, 1.0f), "REPLAY");
    ImGui::Separator();
    ImGui::Spacing();

    const auto draw_navigation_button = [this](const char* label, Page page) {
        const bool selected = app_->page == page;
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(.12f, .25f, .40f, 1.0f));
        }

        if (ImGui::Button(label, ImVec2(-1, 42))) {
            app_->page = page;
        }

        if (selected) {
            ImGui::PopStyleColor();
        }
    };

    draw_navigation_button("Dashboard", Page::Dashboard);
    draw_navigation_button("Dataset", Page::Dataset);
    draw_navigation_button("Recorder", Page::Recorder);
    draw_navigation_button("Analytics", Page::Analytics);
    draw_navigation_button("Settings", Page::Settings);
    draw_navigation_button("Logs", Page::Logs);

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 75);
    ImGui::Separator();
    ImGui::TextDisabled("Dear ImGui");
    ImGui::TextDisabled("DirectX 11");
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("Content", ImVec2(0, 0), false);

    switch (app_->page) {
    case Page::Dashboard:
        draw_dashboard(*app_);
        break;
    case Page::Dataset:
        draw_dataset_page(*app_);
        break;
    case Page::Recorder:
        draw_recorder_page(*app_);
        break;
    case Page::Analytics:
        draw_analytics_page(*app_);
        break;
    case Page::Settings:
        draw_settings_page(*app_, capture_toggle_hotkey_, capture_inventory_hotkey_, capture_recording_hotkey_);
        break;
    case Page::Logs:
        draw_logs_page(*app_);
        break;
    }

    ImGui::EndChild();
    ImGui::End();
}

LRESULT WINAPI Gui::wnd_proc(HWND hwnd,
                             UINT message,
                             WPARAM w_param,
                             LPARAM l_param) {
    if (g_gui && g_gui->app_ &&
        (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)) {
        if (g_gui->capture_toggle_hotkey_) {
            g_gui->app_->config.toggle_hotkey_enabled = true;
            g_gui->app_->config.toggle_key = static_cast<int>(w_param);
            g_gui->capture_toggle_hotkey_ = false;
            g_gui->app_->statistics.log(
                std::string("Replay toggle hotkey set to ") +
                Input::key_name(static_cast<int>(w_param)));
            return 0;
        }

        if (g_gui->capture_inventory_hotkey_) {
            g_gui->app_->config.inventory_hotkey_enabled = true;
            g_gui->app_->config.inventory_key = static_cast<int>(w_param);
            g_gui->capture_inventory_hotkey_ = false;
            g_gui->app_->statistics.log(
                std::string("Inventory pause hotkey set to ") +
                Input::key_name(static_cast<int>(w_param)));
            return 0;
        }

        if (g_gui->capture_recording_hotkey_) {
            g_gui->app_->recording_config.toggle_hotkey_enabled = true;
            g_gui->app_->recording_config.toggle_key = static_cast<int>(w_param);
            g_gui->capture_recording_hotkey_ = false;
            g_gui->recording_toggle_was_down_ = true;
            g_gui->app_->statistics.log(
                std::string("Recording toggle hotkey set to ") +
                Input::key_name(static_cast<int>(w_param)));
            return 0;
        }
    }

    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, w_param, l_param)) {
        return true;
    }

    switch (message) {
    case WM_SIZE:
        if (g_gui && g_gui->device_ && w_param != SIZE_MINIMIZED) {
            if (g_gui->render_target_) {
                g_gui->render_target_->Release();
                g_gui->render_target_ = nullptr;
            }

            g_gui->swap_chain_->ResizeBuffers(
                0,
                LOWORD(l_param),
                HIWORD(l_param),
                DXGI_FORMAT_UNKNOWN,
                0);
            g_gui->create_render_target();
        }
        return 0;

    case WM_SYSCOMMAND:
        if ((w_param & 0xfff0) == SC_KEYMENU) {
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProc(hwnd, message, w_param, l_param);
}
