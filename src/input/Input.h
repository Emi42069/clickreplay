// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <atomic>
#include <functional>
#include <mutex>

class Input {
public:
    Input() = default;
    ~Input();

    bool install_mouse_hook();
    void uninstall_mouse_hook();

    bool mouse_held() const;
    bool key_down(int vk) const;
    bool key_just_pressed(int vk, bool& was_down) const;

    void set_mouse_down_callback(std::function<void()> callback);

    static const char* key_name(int vk);

private:
    static LRESULT CALLBACK mouse_proc(int n_code, WPARAM w_param, LPARAM l_param);

    std::atomic<bool> left_button_down_{false};
    HHOOK mouse_hook_ = nullptr;

    mutable std::mutex callback_mutex_;
    std::function<void()> mouse_down_callback_;
};
