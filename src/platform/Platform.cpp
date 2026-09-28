#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Platform.h"

#include <windows.h>

#include <chrono>
#include <cwchar>
#include <sstream>
#include <thread>

namespace {
constexpr auto k_click_hold_time = std::chrono::milliseconds(5);
}

bool Platform::is_javaw_foreground() const {
    const HWND window = GetForegroundWindow();
    if (!window) {
        return false;
    }

    DWORD process_id = 0;
    GetWindowThreadProcessId(window, &process_id);
    if (!process_id) {
        return false;
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
    if (!process) {
        return false;
    }

    wchar_t path[MAX_PATH] = {};
    DWORD path_size = ARRAYSIZE(path);
    bool is_javaw = false;

    if (QueryFullProcessImageNameW(process, 0, path, &path_size)) {
        const wchar_t* name = std::wcsrchr(path, L'\\');
        name = name ? name + 1 : path;
        is_javaw = _wcsicmp(name, L"javaw.exe") == 0;
    }

    CloseHandle(process);
    return is_javaw;
}

long long Platform::send_left_click(std::string& error) const {
    INPUT down{};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    INPUT up{};
    up.type = INPUT_MOUSE;
    up.mi.dwFlags = MOUSEEVENTF_LEFTUP;

    const auto start = std::chrono::steady_clock::now();
    const UINT down_sent = SendInput(1, &down, sizeof(down));

    std::this_thread::sleep_for(k_click_hold_time);

    const UINT up_sent = SendInput(1, &up, sizeof(up));

    if (down_sent != 1 || up_sent != 1) {
        std::ostringstream message;
        message << "SendInput failed: down=" << down_sent
                << " up=" << up_sent
                << " GetLastError=" << GetLastError();
        error = message.str();
    } else {
        error.clear();
    }

    return std::chrono::duration_cast<std::chrono::milliseconds>(
        start.time_since_epoch()).count();
}
