#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "DatasetPage.h"

#include "../AppState.h"
#include "../replay/ReplaySnapshot.h"
#include "imgui.h"

#include <windows.h>
#include <commdlg.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

namespace {
void choose_csv(AppState& app) {
    std::array<char, MAX_PATH> filename{};

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.lpstrFile = filename.data();
    dialog.nMaxFile = static_cast<DWORD>(filename.size());
    dialog.lpstrFilter = "CSV files (*.csv)\0*.csv\0All files (*.*)\0*.*\0";
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    dialog.lpstrTitle = "Select CSV dataset";

    if (GetOpenFileNameA(&dialog)) {
        app.csv_path = filename.data();
        app.load_dataset();
    }
}
}

void draw_dataset_page(AppState& app) {
    const ReplaySnapshot replay = app.replay.snapshot();
    const bool running = replay.status != ReplayStatus::stopped;

    ImGui::Text("Dataset");
    ImGui::SameLine();
    ImGui::TextDisabled("/ preview");
    ImGui::Separator();
    ImGui::Spacing();

    std::array<char, 2048> path{};
    std::strncpy(path.data(), app.csv_path.c_str(), path.size() - 1);

    ImGui::SetNextItemWidth(-120);
    if (!running && ImGui::InputText("##dataset_path", path.data(), path.size())) {
        app.csv_path = path.data();
    }

    ImGui::SameLine();
    if (!running && ImGui::Button("Browse", ImVec2(100, 0))) {
        choose_csv(app);
    }

    ImGui::SameLine();
    if (!running && ImGui::Button("Load Dataset", ImVec2(120, 0))) {
        app.load_dataset();
    }

    ImGui::Spacing();

    if (app.dataset.empty()) {
        ImGui::TextDisabled("No dataset loaded.");
        return;
    }

    ImGui::Text("Loaded intervals: %zu", app.dataset.size());
    ImGui::Text("Dataset CPS: %.2f", app.dataset.cps());
    ImGui::Spacing();

    const std::vector<long> intervals = app.dataset.values();
    const std::size_t row_count = std::min<std::size_t>(intervals.size(), 500);

    ImGui::BeginChild("DatasetTable", ImVec2(0, 0), true);
    if (ImGui::BeginTable("Intervals", 4,
                          ImGuiTableFlags_Borders |
                          ImGuiTableFlags_RowBg |
                          ImGuiTableFlags_ScrollY |
                          ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("#");
        ImGui::TableSetupColumn("Interval");
        ImGui::TableSetupColumn("Seconds");
        ImGui::TableSetupColumn("Approx. CPS");
        ImGui::TableHeadersRow();

        for (std::size_t i = 0; i < row_count; ++i) {
            const float interval = static_cast<float>(intervals[i]);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%zu", i + 1);
            ImGui::TableNextColumn();
            ImGui::Text("%.0f ms", interval);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f s", interval / 1000.0f);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", 1000.0f / interval);
        }

        ImGui::EndTable();
    }
    ImGui::EndChild();
}
