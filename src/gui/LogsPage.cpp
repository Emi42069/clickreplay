#include "LogsPage.h"

#include "../AppState.h"
#include "imgui.h"

#include <string>
#include <vector>

void draw_logs_page(AppState& app) {
    ImGui::Text("Logs");
    ImGui::SameLine();
    ImGui::TextDisabled("/ replay events");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Clear Logs")) {
        app.statistics.clear_logs();
    }

    ImGui::Spacing();
    ImGui::BeginChild("LogView", ImVec2(0, 0), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    const std::vector<std::string> logs = app.statistics.logs();
    for (const std::string& line : logs) {
        ImGui::TextUnformatted(line.c_str());
    }

    ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}
