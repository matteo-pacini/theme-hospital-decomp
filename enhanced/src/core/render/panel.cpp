#include "core/render/panel.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

namespace th
{

namespace
{

// Sprites of panel_icons_normal (src/array.cpp), laid left to right and
// bottom-aligned as setup_panel_icons does.
constexpr std::array<int, 33> kPanelSprites = {
    1,  3,  4,  6,  8,  10, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14,
};

// The panel watch and its "open now" button (HiresCoords).
constexpr int kWatchX = 5, kWatchY = 350;
constexpr int kWatchButtonX = 9, kWatchButtonY = 335;

// The watch sprite for each thirteenth of a countdown left (the game's
// watch_face, src/tables.cpp; 0 past its listed entries).
int watch_face(int thirteenths)
{
    constexpr std::array<int, 12> faces = {0, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
    return thirteenths >= 0 && thirteenths < 12 ? faces[size_t(thirteenths)] : 0;
}

void panel_size(const SpriteSheet &panel, int &width, int &height)
{
    width = 0;
    height = 0;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        width += s.width;
        height = std::max(height, s.height);
    }
}

} // namespace

Rect draw_panel(Surface &dst, const SpriteSheet &panel)
{
    int width, height;
    panel_size(panel, width, height);
    int x = (dst.width() - width) / 2;
    for (int index : kPanelSprites) {
        Sprite s = panel.at(size_t(index));
        draw_sprite(dst, x, dst.height() - s.height, s);
        x += s.width;
    }
    return {(dst.width() - width) / 2, dst.height() - height, width, height};
}

std::vector<uint32_t> panel_to_argb(const GameData &data, const PanelFigures *figures, int &width, int &height)
{
    const SpriteSheet &panel = data.panel();
    int panel_height;
    panel_size(panel, width, panel_height);
    // The watch's button is the highest thing drawn over the panel.
    height = std::max(panel_height, 480 - kWatchButtonY);
    // Positions below are the game's, in 640 x 480 screen coordinates
    // (HiresBoxes, HiresCoords in src/coords.cpp); `top` maps them here.
    const int top = 480 - height;

    Surface s, covered;
    s.resize(width, height);
    covered.resize(width, height);
    auto put = [&](const Sprite &sprite, int x, int y) {
        draw_sprite(s, x, y - top, sprite);
        draw_sprite_one_colour(covered, x, y - top, sprite, 1);
    };
    int x = 0;
    for (int index : kPanelSprites) {
        Sprite sprite = panel.at(size_t(index));
        put(sprite, x, 480 - sprite.height);
        x += sprite.width;
    }

    if (figures) {
        const Font &money = data.font(5);
        // draw_money_on_panel clips to the height of font sprite 18 ('1').
        int digit_height = money.glyph('1').height;
        char buf[16];
        std::snprintf(buf, sizeof buf, "%7d", figures->balance);
        draw_text(s, money, {0, -top, 640}, 44, 441, buf, 0, {46, 442 - top, 89, digit_height});

        std::snprintf(buf, sizeof buf, "%d %s", figures->day, std::string(data.text().month(figures->month)).c_str());
        Rect date_box{144, 445 - top, 52, 23};
        draw_text(s, data.font(6), {date_box.x, date_box.y, date_box.w}, 0, 6, buf, kTextCentre, date_box);

        put(panel.at(36), 55 + (119 - 55) * figures->reputation / 1000, 467);

        // draw_emergency for the opening countdown: the watch, its "open now"
        // button and a face for each thirteenth of the countdown left.
        if (figures->opening_countdown > 0 && figures->open_countdown >= 13) {
            const SpriteSheet &watch = data.watch();
            put(watch.at(13), kWatchX, kWatchY);
            put(watch.at(16), kWatchButtonX, kWatchButtonY);
            int face = figures->opening_countdown / (figures->open_countdown / 13);
            if (face >= 6)
                put(watch.at(1), kWatchX + 2, kWatchY + 19);
            put(watch.at(size_t(watch_face(face))), kWatchX + 2, kWatchY + 19);
        }
    }

    std::vector<uint32_t> pixels(size_t(width) * size_t(height), 0);
    for (size_t i = 0; i < pixels.size(); i++)
        if (covered.data()[i])
            pixels[i] = data.palette()[s.data()[i]];
    return pixels;
}

} // namespace th
