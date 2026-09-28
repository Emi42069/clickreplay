// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#include "Dashboard.h"

#include "../AppState.h"
#include "../input/Input.h"
#include "../replay/ReplaySnapshot.h"
#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <cstddef>
#include <string>
#include <vector>

namespace {
void draw_stat_card(const char* title,
                    const char* value,
                    const char* subtitle,
                    float width) {
    ImGui::BeginChild(title, ImVec2(width, 100), true);
    ImGui::TextDisabled("%s", title);
    ImGui::Spacing();
    ImGui::Text("%s", value);
    ImGui::TextDisabled("%s", subtitle);
    ImGui::EndChild();
}

const char* status_text(ReplayStatus status) {
    switch (status) {
    case ReplayStatus::stopped: return "IDLE";
    case ReplayStatus::disarmed: return "DISARMED";
    case ReplayStatus::paused: return "PAUSED";
    case ReplayStatus::active: return "ACTIVE";
    case ReplayStatus::inventory_paused: return "INVENTORY PAUSED";
    }

    return "UNKNOWN";
}

void draw_next_click(const ReplaySnapshot& replay) {
    ImGui::BeginChild("NextClick", ImVec2(0, 150), true);
    ImGui::TextDisabled("NEXT CLICK");

    if (replay.status != ReplayStatus::active || replay.next_click_ms <= 0) {
        ImGui::Text("--");
        ImGui::TextDisabled("Replay paused");
        ImGui::EndChild();
        return;
    }

    ImGui::Text("%.3f s", replay.next_click_ms / 1000.0);

    const long long interval = std::max(1LL, replay.current_interval_ms);
    const float progress = 1.0f -
        static_cast<float>(replay.next_click_ms) / static_cast<float>(interval);

    ImGui::ProgressBar(std::clamp(progress, 0.0f, 1.0f), ImVec2(-1, 10), "");
    ImGui::EndChild();
}
}

void draw_dashboard(AppState& app) {
    const ReplaySnapshot replay = app.replay.snapshot();
    const bool running = replay.status != ReplayStatus::stopped;
    const bool recording = app.recorder.snapshot().status == RecordingStatus::recording;
    const std::size_t total = replay.dataset_size;

    ImGui::Text("Dashboard");
    ImGui::SameLine();
    ImGui::TextDisabled("/ live replay monitor");
    ImGui::Separator();
    ImGui::Spacing();

    const char* status = status_text(replay.status);
    char clicks[64];
    char speed[32];
    char actual_cps[32];

    std::snprintf(clicks, sizeof(clicks), "%zu / %zu", replay.current_index, total);
    std::snprintf(speed, sizeof(speed), "%.2fx", replay.speed);
    std::snprintf(actual_cps, sizeof(actual_cps), "%.2f", replay.actual_cps);

    draw_stat_card("STATUS", status, running ? "Replay engine running" : "Ready", 180);
    ImGui::SameLine();
    draw_stat_card("CLICKS", clicks, "Dataset progress", 180);
    ImGui::SameLine();
    draw_stat_card("SPEED", speed, "Playback speed", 180);
    ImGui::SameLine();
    draw_stat_card("ACTUAL CPS", actual_cps, "DOWN-to-DOWN", 180);

    ImGui::Spacing();
    draw_next_click(replay);
    ImGui::Spacing();

    ImGui::Text("Dataset CPS: %.2f", app.dataset.cps());
    ImGui::SameLine();
    ImGui::Text(" | Target CPS: %.2f", replay.target_cps);
    ImGui::SameLine();
    ImGui::Text(" | Actual CPS: %.2f", replay.actual_cps);

    ImGui::Spacing();
    ImGui::TextDisabled("DATASET PROGRESS");

    const float progress = total == 0
        ? 0.0f
        : static_cast<float>(replay.current_index) / static_cast<float>(total);
    ImGui::ProgressBar(progress, ImVec2(-1, 18));

    ImGui::Spacing();
    ImGui::TextDisabled("ACTUAL DOWN-to-DOWN INTERVAL HISTORY");

    const std::vector<float> intervals = app.statistics.interval_history();
    if (intervals.empty()) {
        ImGui::TextDisabled("No data available.");
    } else {
        float max_value = 1.0f;
        for (const float value : intervals) {
            max_value = std::max(max_value, value);
        }

        ImGui::PlotLines("##interval", intervals.data(), static_cast<int>(intervals.size()),
                         0, nullptr, 0.0f, max_value * 1.05f, ImVec2(-1, 150));
    }

    ImGui::Spacing();
    const char* button_label = running ? "STOP REPLAY" : "START REPLAY";
    if (recording) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(button_label, ImVec2(180, 42))) {
        if (running) {
            app.replay.stop();
        } else if (app.load_dataset()) {
            app.replay.start(app.config);
        }
    }

    if (recording) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (recording) {
        ImGui::TextDisabled("Stop recording before starting replay.");
    } else {
        ImGui::TextDisabled("Toggle: %s | Hold LEFT mouse button | Target: javaw.exe",
                           app.config.toggle_hotkey_enabled ? Input::key_name(app.config.toggle_key) : "Disabled");
    }
}
