#include "app/ui/nav_tool.h"

#include "core/world/world.h"

#include <imgui.h>

namespace th
{

namespace
{

bool is_person(const Thing &t)
{
    return t.type == kThingPatient || t.type == kThingStaff;
}

// The middle of cell (col, row)'s diamond, in map pixels.
Point cell_middle(int col, int row)
{
    Point p = cell_to_world(col, row);
    return {p.x, p.y + kTileHeight / 2};
}

} // namespace

void draw_nav_tool(NavTool &tool, World &world)
{
    int people = 0;
    for (const Thing &t : world.things)
        people += is_person(t);
    ImGui::Checkbox("Show planned paths", &tool.show_paths);
    int planner = world.path_finder == PathFinder::AStar ? 0 : 1;
    const char *planners[] = {"A*", "Original (wall following)"};
    if (ImGui::Combo("Route planner", &planner, planners, IM_ARRAYSIZE(planners)))
        world.path_finder = planner == 0 ? PathFinder::AStar : PathFinder::Original;
    if (ImGui::Button(tool.placing ? "Click the map to place..." : "Add a walker"))
        tool.placing = !tool.placing;
    ImGui::Text("%d people", people);
    if (tool.selected && tool.selected < world.things.size() && is_person(world.things[tool.selected])) {
        const Thing &t = world.things[tool.selected];
        ImGui::Text("Selected %u at %d,%d going to %d,%d", tool.selected, t.x.tile, t.y.tile, t.dest_x, t.dest_y);
        ImGui::TextDisabled("Left click a cell to send it there, right click to deselect.");
    } else {
        ImGui::TextDisabled("Left click a person to select them.");
    }
}

void draw_nav_overlay(const NavTool &tool, const World &world, Point camera, float scale)
{
    if (!tool.show_paths)
        return;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    auto screen = [&](Point p) { return ImVec2(float(p.x - camera.x) * scale, float(p.y - camera.y) * scale); };
    for (size_t id = 1; id < world.things.size(); id++) {
        const Thing &t = world.things[id];
        if (!is_person(t))
            continue;
        bool selected = id == tool.selected;
        ImU32 colour = selected ? IM_COL32(255, 220, 40, 255) : IM_COL32(80, 220, 255, 200);
        float width = selected ? 3.0f : 1.5f;
        ImVec2 at = screen(thing_to_world(t));
        // The path: from the last waypoint reached, through the next ones,
        // to the destination (x1 0xff: to be planned again).
        const OrthonavPath &p = t.path;
        ImVec2 prev = at;
        for (int i = 0; i < p.count && i < int(p.steps.size()); i++) {
            ImVec2 next = screen(cell_middle(p.steps[size_t(i)].x, p.steps[size_t(i)].y));
            dl->AddLine(prev, next, colour, width);
            dl->AddCircleFilled(next, 3.0f * scale, colour);
            prev = next;
        }
        ImVec2 dest = screen(cell_middle(t.dest_x, t.dest_y));
        dl->AddCircle(dest, 6.0f * scale, colour, 0, width);
        dl->AddCircle(at, (selected ? 9.0f : 6.0f) * scale, colour, 0, width);
    }
}

uint16_t person_at(const World &world, int col, int row)
{
    for (size_t id = 1; id < world.things.size(); id++) {
        const Thing &t = world.things[id];
        if (is_person(t) && t.x.tile == col && t.y.tile == row)
            return uint16_t(id);
    }
    return 0;
}

} // namespace th
