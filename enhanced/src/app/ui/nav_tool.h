#pragma once

#include "core/render/iso.h"

namespace th
{

struct World;

// The path finding tool of the tools window: shows people's planned routes
// over the view.
struct NavTool {
    bool show_paths = true;
};

// The tool's section of the tools window, for a level in play.
void draw_nav_tool(NavTool &tool, const World &world);

// Draws the planned routes (the person, its next waypoints and its
// destination) over the view with ImGui. `camera` is the view's map pixel
// position and `scale` the size of a view pixel in ImGui units.
void draw_nav_overlay(const NavTool &tool, const World &world, Point camera, float scale);

} // namespace th
