#pragma once

#include "core/render/iso.h"

#include <cstdint>

namespace th
{

struct World;

// The path finding tool of the tools window: shows people's planned routes
// over the view, and sends a selected person to a clicked cell.
struct NavTool {
    bool show_paths = true;
    uint16_t selected = 0; // thing id of the selected person, 0 for none
    bool placing = false;  // the next click on the map adds a walker there
};

// The tool's section of the tools window, for a level in play.
void draw_nav_tool(NavTool &tool, World &world);

// Draws the planned routes (the person, its next waypoints and its
// destination) over the view with ImGui, the selected person's highlighted.
// `camera` is the view's map pixel position and `scale` the size of a view
// pixel in ImGui units.
void draw_nav_overlay(const NavTool &tool, const World &world, Point camera, float scale);

// The first person on cell (col, row), or 0.
uint16_t person_at(const World &world, int col, int row);

} // namespace th
