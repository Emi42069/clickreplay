// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#include "SettingsPage.h"

#include "../AppState.h"
#include "../input/Input.h"
#include "../replay/ReplaySnapshot.h"
#include "imgui.h"

#include <string>

namespace {
void draw_hotkey(const char* label, bool& enabled, int& key, bool& capture) {
    ImGui::PushID(label);

    ImGui::Text("%s", label);
    ImGui::SameLine(220);

    if (!enabled) {
        ImGui::BeginDisabled();
        ImGui::Button("Disabled", ImVec2(170, 0));
        ImGui::EndDisabled();
    } else {
        const std::string button_text = capture ? "Press a key..." : Input::key_name(key);
        if (ImGui::Button(button_text.c_str(), ImVec2(170, 0))) {
            capture = true;
        }
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(enabled ? "X" : "Enable")) {
        enabled = !enabled;
        capture = false;
    }

    if (capture) {
        ImGui::SameLine();
        ImGui::TextDisabled("Waiting...");
    }

    ImGui::PopID();
}
}

void draw_settings_page(AppState& app,
                         bool& capture_toggle_hotkey,
                         bool& capture_inventory_hotkey,
                         bool& capture_recording_hotkey) {
    const ReplaySnapshot replay = app.replay.snapshot();
    const bool running = replay.status != ReplayStatus::stopped;

    ImGui::Text("Settings");
    ImGui::SameLine();
    ImGui::TextDisabled("/ replay configuration");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("Changes apply to the next replay.");
    if (running) {
        ImGui::BeginDisabled();
    }

    ImGui::Text("Playback");
    ImGui::SetNextItemWidth(300);
    ImGui::SliderFloat("Speed", &app.config.speed, 0.1f, 10.0f, "%.2fx");
    ImGui::Checkbox("Dry run (does not send clicks)", &app.config.dry_run);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Seed");
    ImGui::Checkbox("Random seed", &app.config.random_seed);

    ImGui::BeginDisabled(app.config.random_seed);
    ImGui::SetNextItemWidth(250);
    ImGui::InputScalar("Manual seed", ImGuiDataType_U32, &app.config.seed);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Hotkeys");
    ImGui::TextDisabled("Click a button, then press the key you want to assign.");
    draw_hotkey("Replay toggle", app.config.toggle_hotkey_enabled, app.config.toggle_key, capture_toggle_hotkey);
    draw_hotkey("Inventory pause", app.config.inventory_hotkey_enabled, app.config.inventory_key, capture_inventory_hotkey);
    draw_hotkey("Recording toggle", app.recording_config.toggle_hotkey_enabled, app.recording_config.toggle_key, capture_recording_hotkey);

    if (running) {
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Replay toggle: %s", app.config.toggle_hotkey_enabled ? Input::key_name(app.config.toggle_key) : "Disabled");
    ImGui::TextDisabled("Inventory pause: %s", app.config.inventory_hotkey_enabled ? Input::key_name(app.config.inventory_key) : "Disabled");
    ImGui::TextDisabled("Recording toggle: %s", app.recording_config.toggle_hotkey_enabled ? Input::key_name(app.recording_config.toggle_key) : "Disabled");
    ImGui::TextDisabled("Hold the left mouse button to generate clicks.");
    ImGui::TextDisabled("Clicks are sent only when javaw.exe is the foreground window.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Timing");
    ImGui::TextDisabled("CSV intervals are the target time between LEFTDOWN events.");
    ImGui::TextDisabled("The scheduler uses absolute deadlines to avoid cumulative drift.");
    ImGui::TextDisabled("Actual CPS is calculated from measured DOWN-to-DOWN intervals.");
}
