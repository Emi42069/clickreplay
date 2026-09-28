// Copyright (c) 2026 Emi.
//
// This file is part of Click Replay GUI.
// Licensed under the Click Replay GUI Non-Commercial Source License 1.0.
// See LICENSE in the project root for the full license text.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "RecorderPage.h"

#include "../AppState.h"
#include "../input/Input.h"
#include "../recording/RecordingSnapshot.h"
#include "imgui.h"

#include <array>
#include <commdlg.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace {
void choose_save_path(AppState& app) {
    std::array<char, MAX_PATH> filename{};
    std::strncpy(filename.data(), "recording.csv", filename.size() - 1);

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFile = filename.data();
    dialog.nMaxFile = static_cast<DWORD>(filename.size());
    dialog.lpstrFilter = "CSV files (*.csv)\0*.csv\0All files (*.*)\0*.*\0";
    dialog.lpstrDefExt = "csv";
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    dialog.lpstrTitle = "Save recorded dataset";

    if (!GetSaveFileNameA(&dialog)) {
        return;
    }

    std::string error;
    if (app.dataset.save_csv(filename.data(), error)) {
        app.csv_path = filename.data();
        app.statistics.log("Recorded dataset saved to " + app.csv_path);
    } else {
        app.statistics.log(error);
    }
}

const char* status_text(RecordingStatus status) {
    return status == RecordingStatus::recording ? "RECORDING" : "IDLE";
}

void draw_stat(const char* label, const char* value, float width) {
    ImGui::BeginChild(label, ImVec2(width, 92), true);
    ImGui::TextDisabled("%s", label);
    ImGui::Spacing();
    ImGui::Text("%s", value);
    ImGui::EndChild();
}
}

void draw_recorder_page(AppState& app) {
    const RecordingSnapshot recording = app.recorder.snapshot();
    const bool is_recording = recording.status == RecordingStatus::recording;
    const bool replay_running = app.replay.snapshot().status != ReplayStatus::stopped;

    ImGui::Text("Recorder");
    ImGui::SameLine();
    ImGui::TextDisabled("/ build a click timing dataset");
    ImGui::Separator();
    ImGui::Spacing();

    char clicks[64];
    char duration[64];
    char cps[64];
    char last_interval[64];

    std::snprintf(clicks, sizeof(clicks), "%zu", recording.click_count);
    std::snprintf(duration, sizeof(duration), "%.2f s", recording.duration_ms / 1000.0);
    std::snprintf(cps, sizeof(cps), "%.2f", recording.cps);
    std::snprintf(last_interval, sizeof(last_interval), "%lld ms", recording.last_interval_ms);

    draw_stat("STATUS", status_text(recording.status), 180);
    ImGui::SameLine();
    draw_stat("CLICKS", clicks, 180);
    ImGui::SameLine();
    draw_stat("DURATION", duration, 180);
    ImGui::SameLine();
    draw_stat("CPS", cps, 180);

    ImGui::Spacing();
    ImGui::Text("Intervals recorded: %zu", recording.interval_count);
    ImGui::Text("Last interval: %s", recording.interval_count > 0 ? last_interval : "--");
    ImGui::TextDisabled("Intervals measure physical LEFTDOWN-to-LEFTDOWN time.");

    ImGui::Spacing();
    if (is_recording) {
        if (ImGui::Button("STOP RECORDING", ImVec2(190, 42))) {
            app.recorder.stop();
        }
    } else {
        const bool can_start = !replay_running;
        if (!can_start) {
            ImGui::BeginDisabled();
        }

        if (ImGui::Button("START RECORDING", ImVec2(190, 42))) {
            app.recorder.start();
        }

        if (!can_start) {
            ImGui::EndDisabled();
        }
    }

    ImGui::SameLine();
    if (recording.interval_count == 0 || is_recording) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("SAVE DATASET", ImVec2(150, 42))) {
        choose_save_path(app);
    }

    if (recording.interval_count == 0 || is_recording) {
        ImGui::EndDisabled();
    }

    if (!is_recording && replay_running) {
        ImGui::Spacing();
        ImGui::TextDisabled("Stop the replay before starting a new recording.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("How it works");
    ImGui::TextDisabled("Start recording, then click normally. The low-level mouse hook ignores injected clicks.");
    ImGui::TextDisabled("The first click starts the timing sequence; each following click adds one interval.");
    ImGui::TextDisabled("Toggle recording: %s", app.recording_config.toggle_hotkey_enabled ? Input::key_name(app.recording_config.toggle_key) : "Disabled");
}
