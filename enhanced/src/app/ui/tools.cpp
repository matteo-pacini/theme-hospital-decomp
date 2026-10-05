#include "app/ui/tools.h"

#include <imgui.h>

namespace th
{

MissingDataAction draw_missing_data_screen(const DataSearch &search, const std::string &error)
{
    MissingDataAction action = MissingDataAction::None;
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("Theme Hospital data needed", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Theme Hospital data needed");
    ImGui::Separator();
    ImGui::TextWrapped("This program contains no game files. It needs the data from your own copy of Theme Hospital "
                       "(for example GOG's release): the folder that holds DATA, LEVELS and QDATA.");
    ImGui::Spacing();
    if (ImGui::Button("Choose the game folder..."))
        action = MissingDataAction::Choose;
    ImGui::SameLine();
    if (ImGui::Button("Quit"))
        action = MissingDataAction::Quit;
    if (!error.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.4f, 1.0f), "%s", error.c_str());
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Places looked at:");
    for (const DataCandidate &c : search.tried) {
        ImGui::BulletText("%s: %s", c.source.c_str(), c.path.string().c_str());
        if (!c.missing.empty()) {
            ImGui::Indent();
            std::string missing = "missing ";
            for (size_t i = 0; i < c.missing.size(); i++)
                missing += (i ? ", " : "") + c.missing[i];
            ImGui::TextDisabled("%s", missing.c_str());
            ImGui::Unindent();
        }
    }
    ImGui::Spacing();
    ImGui::TextDisabled("You can also pass the folder on the command line: --data <folder>");
    ImGui::End();
    return action;
}

void draw_tools(ToolsState &state, World *world)
{
    if (!state.open)
        return;
    if (ImGui::Begin("Tools (F12)", &state.open, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char *scales[] = {"Automatic", "1x", "2x", "3x", "4x"};
        ImGui::Combo("UI scale", &state.ui_scale, scales, IM_ARRAYSIZE(scales));
        if (world)
            draw_nav_tool(state.nav, *world);
    }
    ImGui::End();
}

} // namespace th
