#include "common.h"

#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/render/text.h"
#include "core/ui/info_windows.h"
#include "core/ui/layer.h"
#include "core/ui/panel_info.h"
#include "core/world/world.h"

#include <cstdint>
#include <string>

namespace
{

// Whether the bar's line `line` (from the origin, kInfoTextY and the line
// height) has any drawn pixel.
bool line_drawn(const std::vector<uint32_t> &argb, th::Point origin, int line)
{
    int y0 = origin.y + 10 + line * 10;
    for (int y = y0; y < y0 + 10 && y < th::kUiHeight; y++)
        for (int x = origin.x; x < origin.x + 250 && x < th::kUiWidth; x++)
            if (argb[size_t(y) * th::kUiWidth + size_t(x)])
                return true;
    return false;
}

} // namespace

// The panel's information bar (core/ui/panel_info.cpp): it is drawn in the
// panel's message bar (panel icon 7, the game's panel->x/y) and shows a
// machine's or a VIP's information.
void test_panel_info(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    const th::GameData &gd = f.gd;

    // The bar's origin is panel icon 7's top-left, in the message bar.
    th::Point origin = th::panel_icon_at(gd, false, 7);
    CHECK(origin.x > 300 && origin.x < 640);
    CHECK(origin.y > 400 && origin.y < 480);

    // A machine in the world, shown as panel info kind 3.
    th::Thing machine;
    machine.type = th::kThingObject;
    machine.subtype = 9;
    machine.strength = 7;
    machine.times_used = 12;
    uint16_t id = uint16_t(f.game->world.things.size());
    f.game->world.things.push_back(machine);

    th::Font font;
    std::string error;
    CHECK(th::load_font(dir, 1, font, &error));
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_panel_info({layer, th::panel_top(gd), font}, 3, id, *f.game, gd);
    std::vector<uint32_t> argb = layer.to_argb(gd.palette());
    CHECK(line_drawn(argb, origin, 0));

    // The machine's uses and strength go on the third line (hires).
    CHECK(line_drawn(argb, origin, 2));

    // A VIP shows panel info 5 and the mayor's lines.
    th::Thing mayor;
    mayor.type = th::kThingMayor;
    uint16_t mid = uint16_t(f.game->world.things.size());
    f.game->world.things.push_back(mayor);
    f.game->world.mayor.name = 3;
    th::MapHover h = th::map_hover(f.game->world, mid);
    CHECK(h.hit && h.info == 5);
    layer.clear();
    th::draw_panel_info({layer, th::panel_top(gd), font}, 5, mid, *f.game, gd);
    argb = layer.to_argb(gd.palette());
    CHECK(line_drawn(argb, origin, 0));
}
