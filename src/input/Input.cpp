#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Input.h"

namespace {
Input* g_input = nullptr;
}

Input::~Input() {
    uninstall_mouse_hook();
}

bool Input::install_mouse_hook() {
    if (mouse_hook_) {
        return true;
    }

    g_input = this;
    mouse_hook_ = SetWindowsHookExW(WH_MOUSE_LL, mouse_proc, nullptr, 0);
    if (!mouse_hook_) {
        g_input = nullptr;
        return false;
    }

    return true;
}

void Input::uninstall_mouse_hook() {
    if (mouse_hook_) {
        UnhookWindowsHookEx(mouse_hook_);
        mouse_hook_ = nullptr;
    }

    if (g_input == this) {
        g_input = nullptr;
    }
}

bool Input::mouse_held() const {
    return left_button_down_.load(std::memory_order_relaxed);
}

bool Input::key_down(int vk) const {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool Input::key_just_pressed(int vk, bool& was_down) const {
    const bool down = key_down(vk);
    const bool pressed = down && !was_down;
    was_down = down;
    return pressed;
}

const char* Input::key_name(int vk) {
    switch (vk) {
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_SPACE: return "SPACE";
    case VK_TAB: return "TAB";
    case VK_RETURN: return "ENTER";
    case VK_ESCAPE: return "ESC";
    case VK_BACK: return "BACKSPACE";
    case VK_SHIFT: return "SHIFT";
    case VK_CONTROL: return "CTRL";
    case VK_MENU: return "ALT";
    case VK_CAPITAL: return "CAPS LOCK";
    case VK_INSERT: return "INSERT";
    case VK_DELETE: return "DELETE";
    case VK_HOME: return "HOME";
    case VK_END: return "END";
    case VK_PRIOR: return "PAGE UP";
    case VK_NEXT: return "PAGE DOWN";
    case VK_UP: return "UP";
    case VK_DOWN: return "DOWN";
    case VK_LEFT: return "LEFT";
    case VK_RIGHT: return "RIGHT";
    case VK_LBUTTON: return "MOUSE 1";
    case VK_RBUTTON: return "MOUSE 2";
    case VK_MBUTTON: return "MOUSE 3";
    default: break;
    }

    static char buffer[2];
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
        buffer[0] = static_cast<char>(vk);
        buffer[1] = '\0';
        return buffer;
    }

    return "UNKNOWN";
}

LRESULT CALLBACK Input::mouse_proc(int n_code, WPARAM w_param, LPARAM l_param) {
    if (n_code == HC_ACTION && l_param && g_input) {
        const auto* info = reinterpret_cast<const MSLLHOOKSTRUCT*>(l_param);
        if ((info->flags & LLMHF_INJECTED) == 0) {
            if (w_param == WM_LBUTTONDOWN) {
                g_input->left_button_down_.store(true, std::memory_order_relaxed);
            } else if (w_param == WM_LBUTTONUP) {
                g_input->left_button_down_.store(false, std::memory_order_relaxed);
            }
        }
    }

    return CallNextHookEx(g_input ? g_input->mouse_hook_ : nullptr,
                          n_code,
                          w_param,
                          l_param);
}
