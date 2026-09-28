#include "AnalyticsPage.h"

#include "../AppState.h"
#include "imgui.h"

#include <algorithm>
#include <vector>

namespace {
void draw_graph(const char* label,
               const char* id,
               const std::vector<float>& values,
               float padding) {
    ImGui::TextDisabled("%s", label);

    if (values.empty()) {
        ImGui::TextDisabled("No data available.");
        return;
    }

    float max_value = 1.0f;
    for (const float value : values) {
        max_value = std::max(max_value, value);
    }

    ImGui::PlotLines(id,
                     values.data(),
                     static_cast<int>(values.size()),
                     0,
                     nullptr,
                     0.0f,
                     max_value * padding,
                     ImVec2(-1, 220));
}
}

void draw_analytics_page(AppState& app) {
    const std::vector<float> intervals = app.statistics.interval_history();
    const std::vector<float> cps = app.statistics.cps_history();

    ImGui::Text("Analytics");
    ImGui::SameLine();
    ImGui::TextDisabled("/ replay statistics");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Dataset CPS: %.2f", app.dataset.cps());
    ImGui::Text("Target CPS: %.2f", app.statistics.target_cps());
    ImGui::Text("Actual CPS: %.2f", app.statistics.actual_cps());
    ImGui::Spacing();

    draw_graph("CPS", "##cps_graph", cps, 1.10f);
    ImGui::Spacing();
    draw_graph("ACTUAL INTERVAL (ms)", "##interval_graph", intervals, 1.05f);

    double average_interval = 0.0;
    for (const float value : intervals) {
        average_interval += value;
    }

    if (!intervals.empty()) {
        average_interval /= intervals.size();
    }

    const double average_cps = average_interval > 0.0
        ? 1000.0 / average_interval
        : 0.0;

    ImGui::Text("Actual average DOWN-to-DOWN: %.2f ms", average_interval);
    ImGui::Text("Actual average CPS: %.2f", average_cps);
}
