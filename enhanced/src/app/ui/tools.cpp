#include "app/ui/tools.h"

#include "core/world/build.h"
#include "core/world/debug_dump.h"
#include "core/world/world.h"

#include <imgui.h>
#include <string>
#include <vector>

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
