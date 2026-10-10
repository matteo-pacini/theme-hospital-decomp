#include "app/ui/tools.h"

#include "core/world/build.h"
#include "core/world/debug_dump.h"
#include "core/world/world.h"

#include <cstdint>
#include <cstdio>
#include <imgui.h>
#include <string>
#include <vector>

namespace th
{
namespace
{

const ImVec4 kSetupErrorColor(1.0f, 0.45f, 0.4f, 1.0f);

// Greyed text that wraps at the window's edge.
void text_disabled_wrapped(const std::string &text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", text.c_str());
    ImGui::PopStyleColor();
}

void draw_places_looked_at(const DataSearch &search)
{
    ImGui::TextDisabled("Places looked at:");
    for (const DataCandidate &c : search.tried) {
        text_disabled_wrapped("- " + c.source + ": " + c.path.string());
        if (!c.missing.empty()) {
            constexpr size_t kShown = 5;
            std::string missing = "missing ";
            for (size_t i = 0; i < c.missing.size() && i < kShown; i++)
                missing += (i ? ", " : "") + c.missing[i];
            if (c.missing.size() > kShown)
                missing += " and " + std::to_string(c.missing.size() - kShown) + " more";
            ImGui::Indent();
            text_disabled_wrapped(missing);
            ImGui::Unindent();
        }
    }
    ImGui::TextDisabled("You can also pass the folder on the command line: --data <folder>");
}

SetupAction draw_setup_progress(const SetupView &view)
{
    ImGui::TextUnformatted(view.verifying ? "Verifying the installed game files..." : "Installing the game files...");
    float fraction = view.total ? float(double(view.done) / double(view.total)) : 0.0f;
    char label[64];
    std::snprintf(label, sizeof label, "%.0f MB of %.0f MB", double(view.done) / 1e6, double(view.total) / 1e6);
    ImGui::ProgressBar(fraction, ImVec2(-FLT_MIN, 0.0f), label);
    return ImGui::Button("Cancel") ? SetupAction::Cancel : SetupAction::None;
}

SetupAction draw_setup_actions(const SetupView &view)
{
    SetupAction action = SetupAction::None;
    if (ImGui::Button("Install from GOG's installer..."))
        action = SetupAction::InstallFromInstaller;
    ImGui::SameLine();
    if (ImGui::Button("Install from a game folder..."))
        action = SetupAction::InstallFromFolder;
    if (view.game_dir_exists) {
        ImGui::SameLine();
        if (ImGui::Button("Verify files"))
            action = SetupAction::Verify;
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit"))
        action = SetupAction::Quit;
    return action;
}

} // namespace

SetupAction draw_setup_screen(const SetupView &view)
{
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::Begin("Theme Hospital setup", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Theme Hospital setup");
    ImGui::Separator();
    ImGui::TextWrapped("This program contains no game files. It installs them from your own copy of Theme Hospital: "
                       "GOG's installer (setup_theme_hospital_v3_(28027).exe) or a folder with the game installed "
                       "from it. Drop the installer or the folder on this window, or choose it below.");
    ImGui::Spacing();
    if (view.game_dir_incomplete) {
        ImGui::TextColored(kSetupErrorColor, "The installed game files are incomplete, install again.");
        ImGui::Spacing();
    }
    SetupAction action = view.working ? draw_setup_progress(view) : draw_setup_actions(view);
    if (!view.error.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(kSetupErrorColor, "%s", view.error.c_str());
    }
    if (!view.notice.empty()) {
        ImGui::Spacing();
        ImGui::TextWrapped("%s", view.notice.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();
    text_disabled_wrapped("The game is installed into " + view.game_dir);
    draw_places_looked_at(view.search);
    ImGui::End();
    return action;
}

namespace
{

// One field's read-only control: a number box (with its name beside it, if
// any), a greyed checkbox or a text box.
void draw_inspect_value(const InspectField &f)
{
    switch (f.kind) {
    case InspectKind::Number: {
        int64_t value = f.number;
        ImGui::SetNextItemWidth(f.note.empty() ? -FLT_MIN : 120.0f);
        ImGui::InputScalar("##value", ImGuiDataType_S64, &value, nullptr, nullptr, nullptr,
                           ImGuiInputTextFlags_ReadOnly);
        if (!f.note.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", f.note.c_str());
        }
        break;
    }
    case InspectKind::Flag: {
        bool value = f.number != 0;
        ImGui::BeginDisabled();
        ImGui::Checkbox("##value", &value);
        ImGui::EndDisabled();
        break;
    }
    case InspectKind::Text: {
        std::string text = f.value;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##value", text.data(), text.size() + 1, ImGuiInputTextFlags_ReadOnly);
        break;
    }
    }
}

void draw_inspect_fields(const InspectSection &section)
{
    if (!ImGui::BeginTable("fields", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
        return;
    ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 160.0f);
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    for (const InspectField &f : section.fields) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(f.name.c_str());
        ImGui::TableNextColumn();
        ImGui::PushID(f.name.c_str());
        draw_inspect_value(f);
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void draw_inspected_thing(const World &world, uint16_t id)
{
    std::vector<InspectSection> sections = inspect_thing(world, id);
    ImGui::BeginChild("thing", ImVec2(560, 480));
    for (size_t i = 0; i < sections.size(); i++) {
        ImGui::PushID(int(i));
        if (i == 0)
            ImGui::SeparatorText(sections[i].title.c_str());
        if (i == 0 || ImGui::CollapsingHeader(sections[i].title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            draw_inspect_fields(sections[i]);
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void draw_inspect_tool(InspectTool &tool, const World &world)
{
    if (tool.picking) {
        ImGui::TextUnformatted("Click a person or object in the hospital (right click cancels).");
        if (ImGui::Button("Cancel"))
            tool.picking = false;
        return;
    }
    if (ImGui::Button(tool.thing ? "Pick another" : "Pick"))
        tool.picking = true;
    if (!tool.thing) {
        ImGui::SameLine();
        ImGui::TextDisabled("Nothing inspected.");
        return;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        tool.thing = 0;
        return;
    }
    const Thing &t = world.things[tool.thing];
    if (t.type == kThingFree) {
        ImGui::TextDisabled("Thing %u is gone.", tool.thing);
        return;
    }
    uint16_t room = room_at(world, uint16_t(t.x.tile + t.y.tile * kMapSize));
    ImGui::Text("Cell %d,%d, room %u", t.x.tile, t.y.tile, room);
    draw_inspected_thing(world, tool.thing);
}

void draw_debug_report(DebugReport &report)
{
    ImGui::TextDisabled("A screenshot, the game's state and this note.");
    ImGui::InputTextMultiline("##note", report.note.data(), report.note.size(), ImVec2(420, 120));
    if (ImGui::Button("Save report"))
        report.save = true;
    if (!report.last.empty())
        ImGui::TextWrapped("%s", report.last.c_str());
}

} // namespace

void draw_tools(ToolsState &state, const World *world)
{
    if (!state.open)
        return;
    if (ImGui::Begin("Tools (F12)", &state.open, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::BeginTabBar("tools")) {
            if (world && ImGui::BeginTabItem("Inspect")) {
                draw_inspect_tool(state.inspect, *world);
                ImGui::EndTabItem();
            }
            if (world && ImGui::BeginTabItem("Path finding")) {
                draw_nav_tool(state.nav, *world);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug report")) {
                draw_debug_report(state.report);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Display")) {
                const char *scales[] = {"Automatic", "1x", "2x", "3x", "4x"};
                ImGui::Combo("UI scale", &state.ui_scale, scales, IM_ARRAYSIZE(scales));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

QuickNoteAction draw_quick_report_note(std::array<char, 4096> &note, bool &focus)
{
    QuickNoteAction action = QuickNoteAction::None;
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImVec2 size(460.0f, 150.0f);
    ImVec2 pos(vp->WorkPos.x + (vp->WorkSize.x - size.x) * 0.5f, vp->WorkPos.y + (vp->WorkSize.y - size.y) * 0.5f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
    ImGui::Begin("Quick debug report", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextUnformatted("Enter saves the report, Shift+Enter adds a line, Esc discards.");
    if (focus)
        ImGui::SetKeyboardFocusHere();
    if (ImGui::InputTextMultiline("##note", note.data(), note.size(), ImVec2(-FLT_MIN, 60.0f),
                                  ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine))
        action = QuickNoteAction::Save;
    if (ImGui::IsItemActive())
        focus = false;
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        action = QuickNoteAction::Discard;
    ImGui::End();
    return action;
}

void draw_toast(const std::string &text)
{
    if (text.empty())
        return;
    const ImGuiViewport *vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - 64.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGui::Begin("##toast", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::TextUnformatted(text.c_str());
    ImGui::End();
}

} // namespace th
