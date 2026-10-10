#include "core/render/panel.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <span>
#include <string>

namespace th
{

namespace
{

// A panel icon: its sprite and the sprite drawn while it is held, 0 for an
// icon without one (PanelIcon.kind 0x40).
struct PanelIconDef {
    uint8_t sprite;
    uint8_t pressed;
};

// panel_icons_normal and panel_icons_over (src/array.cpp), laid left to
// right and bottom-aligned as setup_panel_icons does.
constexpr std::array<PanelIconDef, 33> kPanelIconsNormal = {{
    {1, 2},  {3, 0},  {4, 5},  {6, 7},  {8, 9},  {10, 11}, {12, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0},
    {13, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0},  {13, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0},
    {13, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0}, {13, 0},  {13, 0}, {13, 0}, {13, 0}, {13, 0}, {14, 0},
}};
constexpr std::array<PanelIconDef, 13> kPanelIconsOver = {{
    {1, 2},
    {3, 0},
    {4, 5},
    {6, 7},
    {8, 9},
    {10, 11},
    {15, 16},
    {17, 18},
    {19, 20},
    {21, 22},
    {23, 24},
    {25, 26},
    {27, 28},
}};

std::span<const PanelIconDef> panel_icons(bool raised)
{
    if (raised)
        return kPanelIconsOver;
    return kPanelIconsNormal;
}

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
    for (PanelIconDef icon : kPanelIconsNormal) {
        Sprite s = panel.at(icon.sprite);
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
    for (PanelIconDef icon : kPanelIconsNormal) {
        Sprite s = panel.at(icon.sprite);
        draw_sprite(dst, x, dst.height() - s.height, s);
        x += s.width;
    }
    return {(dst.width() - width) / 2, dst.height() - height, width, height};
}

std::vector<uint32_t> panel_to_argb(const GameData &data, const PanelFigures *figures, int &width, int &height)
{
    int panel_height;
    panel_size(data.panel(), width, panel_height);
    // The watch's button is the highest thing drawn over the panel.
    height = std::max(panel_height, kUiHeight - kPanelWatchButtonY);
    UiLayer layer;
    layer.resize(std::max(width, kUiWidth), kUiHeight);
    layer.clear();
    PanelState state;
    if (figures)
        state.shown_balance = figures->balance;
    draw_panel_ui(layer, state, figures, data);
    if (figures)
        draw_emergency(layer, *figures, data);
    std::vector<uint32_t> all = layer.to_argb(data.palette());
    std::vector<uint32_t> pixels(size_t(width) * size_t(height));
    for (int y = 0; y < height; y++) {
        auto row = all.begin() + (kUiHeight - height + y) * layer.width();
        std::copy(row, row + width, pixels.begin() + y * width);
    }
    return pixels;
}

int panel_top(const GameData &data)
{
    // choose_panel_icons: the top of icon 1, the same in both sets.
    return kUiHeight - data.panel().at(kPanelIconsNormal[1].sprite).height;
}

Point panel_icon_at(const GameData &data, bool raised, int index)
{
    std::span<const PanelIconDef> icons = panel_icons(raised);
    int x = 0;
    int i = 0;
    for (; i < index && i < int(icons.size()); i++)
        x += data.panel().at(icons[size_t(i)].sprite).width;
    Sprite top = i < int(icons.size()) ? data.panel().at(icons[size_t(i)].sprite) : Sprite{};
    return {x, kUiHeight - top.height};
}

int panel_icon_at(const GameData &data, bool raised, int x, int y)
{
    if (y < panel_top(data))
        return kNoPanelIcon;
    int left = 0;
    std::span<const PanelIconDef> icons = panel_icons(raised);
    for (size_t i = 0; i < icons.size(); i++) {
        int right = left + data.panel().at(icons[i].sprite).width;
        if (left <= x && right >= x)
            return int(i);
        left = right;
    }
    return kNoPanelIcon;
}

PanelAction panel_frame(PanelState &state, const UiMouse &mouse, const GameData &data, bool paused, bool input)
{
    bool on_panel = mouse.y >= panel_top(data);
    state.raised = on_panel && !paused;
    state.pressed = kNoPanelIcon;
    if (paused || !input) {
        state.left_click = false;
        return PanelAction::None;
    }
    if (!on_panel) {
        // process_mouse hands a pending left click to the map.
        state.left_click = false;
        return PanelAction::None;
    }
    int icon = panel_icon_at(data, state.raised, mouse.x, mouse.y);
    auto press = [&] {
        // panel_ml_button
        if (icon != kNoPanelIcon && panel_icons(state.raised)[size_t(icon)].pressed)
            state.pressed = icon;
    };

    PanelAction action = PanelAction::None;
    if (mouse.left_pressed)
        state.left_click = true;
    if (mouse.left_held) {
        press();
    } else if (state.left_click) {
        // panel_l_button
        state.left_click = false;
        constexpr std::array<PanelAction, 13> left = {
            PanelAction::BankManager,     PanelAction::Refused,  PanelAction::BuildRoom,
            PanelAction::FurnishCorridor, PanelAction::EditRoom, PanelAction::HireStaff,
            PanelAction::StaffManagement, PanelAction::TownMap,  PanelAction::Casebook,
            PanelAction::Research,        PanelAction::Status,   PanelAction::Charts,
            PanelAction::Policy,
        };
        if (icon != kNoPanelIcon)
            action = left[size_t(icon)];
    }
    if (mouse.right_pressed) {
        // panel_r_button
        if (icon == kIconBank)
            action = PanelAction::BankStatement;
        else if (icon == kIconHireStaff)
            action = PanelAction::StaffManagement;
        else if (icon != kNoPanelIcon)
            action = PanelAction::Refused;
    } else if (mouse.right_held) {
        press();
    }
    return action;
}

namespace
{

// draw_money_on_panel, draw_date_on_panel and draw_rep_on_panel.
void draw_panel_figures(UiLayer &layer, const PanelState &state, const PanelFigures *figures, const GameData &data)
{
    const SpriteSheet &panel = data.panel();
    // Positions are the game's (HiresBoxes, HiresCoords in src/coords.cpp).
    const Font &money = data.font(5);
    // draw_money_on_panel clips to the height of font sprite 18 ('1').
    int digit_height = money.glyph('1').height;
    char buf[16];
    std::snprintf(buf, sizeof buf, "%7d", figures->balance);
    if (state.shown_balance == figures->balance) {
        layer.draw_text(money, 44, 441, buf, {{0, 0, kUiWidth}, {46, 442, 89, digit_height}});
    } else {
        // The game's count of changed digit columns breaks out of its loop
        // on the first column it counts, so it is always 1.
        constexpr int kChangedColumns = 1;
        constexpr int kDigitWidth = 13;
        int step = state.balance_roll_step * 6;
        layer.draw_text(money, 44, 441 - (digit_height - step), buf,
                        {{0, 0, kUiWidth},
                         {46 + kDigitWidth * kChangedColumns, 442, 89 - kDigitWidth * kChangedColumns, digit_height}});
        char shown[16];
        std::snprintf(shown, sizeof shown, "%7d", state.shown_balance);
        layer.draw_text(money, 44, 441 + step, shown,
                        {{0, 0, kUiWidth},
                         {46 + kDigitWidth * kChangedColumns, 442, 89 - kDigitWidth * kChangedColumns, digit_height}});
        layer.draw_text(money, 44, 441, buf,
                        {{0, 0, kUiWidth}, {46, 442, kDigitWidth * (kChangedColumns - 3) + 37, digit_height}});
    }

    std::snprintf(buf, sizeof buf, "%d %s", figures->day, std::string(data.text().month(figures->month)).c_str());
    Rect date_box{144, 445, 52, 23};
    layer.draw_text(data.font(6), 0, 6, buf, {{date_box.x, date_box.y, date_box.w}, date_box, kTextCentre});

    layer.draw_sprite(55 + (119 - 55) * figures->reputation / 1000, 467, panel.at(36));
}

} // namespace

void draw_panel_ui(UiLayer &layer, const PanelState &state, const PanelFigures *figures, const GameData &data)
{
    const SpriteSheet &panel = data.panel();
    int x = 0;
    std::span<const PanelIconDef> icons = panel_icons(state.raised);
    for (size_t i = 0; i < icons.size(); i++) {
        Sprite sprite = panel.at(icons[i].sprite);
        bool pressed = int(i) == state.pressed && icons[i].pressed;
        layer.draw_sprite(x, kUiHeight - sprite.height, pressed ? panel.at(icons[i].pressed) : sprite);
        x += sprite.width;
    }
    if (figures)
        draw_panel_figures(layer, state, figures, data);
}

void draw_panel_icon_pressed(UiLayer &layer, const PanelState &state, int index, const GameData &data)
{
    std::span<const PanelIconDef> icons = panel_icons(state.raised);
    if (index < 0 || index >= int(icons.size()) || !icons[size_t(index)].pressed)
        return;
    const SpriteSheet &panel = data.panel();
    Point at = panel_icon_at(data, state.raised, index);
    layer.draw_sprite(at.x, kUiHeight - panel.at(icons[size_t(index)].sprite).height,
                      panel.at(icons[size_t(index)].pressed));
}

void redraw_panel_left(UiLayer &layer, const PanelState &state, const PanelFigures *figures, const GameData &data)
{
    const SpriteSheet &panel = data.panel();
    std::span<const PanelIconDef> icons = panel_icons(state.raised);
    int x = 0;
    for (size_t i = 0; i < 2; i++) {
        Sprite sprite = panel.at(icons[i].sprite);
        layer.draw_sprite(x, kUiHeight - sprite.height, sprite);
        x += sprite.width;
    }
    if (figures)
        draw_panel_figures(layer, state, figures, data);
}

void draw_emergency(UiLayer &layer, const PanelFigures &figures, const GameData &data)
{
    const SpriteSheet &watch = data.watch();
    bool countdown = figures.opening_countdown > 0 && figures.open_countdown >= 13;
    bool watch_new = figures.emergency_timer > 0 || countdown || figures.epidemic_timer > 0;
    if (watch_new || figures.emergency_state > 1)
        layer.draw_sprite(kPanelWatchX, kPanelWatchY, watch.at(13));
    if (figures.emergency_timer > 0) {
        // The emergency's time left, in eighths of 100 half days.
        int face = figures.emergency_timer / 8;
        if (face >= 6)
            layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(1));
        layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(size_t(watch_face(face))));
    } else if (figures.epidemic_timer > 0) {
        // The cover-up's time left, with the vaccination button beside it.
        int face = figures.epidemic_timer / 8;
        layer.draw_sprite(kPanelWatchButtonX, kPanelWatchButtonY, watch.at(14));
        if (face >= 6)
            layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(1));
        layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(size_t(watch_face(face))));
    } else if (countdown) {
        // The opening countdown: the "open now" button and a face for each
        // thirteenth of the countdown left.
        layer.draw_sprite(kPanelWatchButtonX, kPanelWatchButtonY, watch.at(16));
        int face = figures.opening_countdown / (figures.open_countdown / 13);
        if (face >= 6)
            layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(1));
        layer.draw_sprite(kPanelWatchX + 2, kPanelWatchY + 19, watch.at(size_t(watch_face(face))));
    }
}

void advance_balance_roll(PanelState &state, int balance)
{
    if (state.shown_balance == balance)
        return;
    if (++state.balance_roll_step == 4) {
        state.balance_roll_step = 0;
        state.shown_balance = balance;
    }
}

namespace
{

// panel_icons_ratfest (src/array.cpp), laid left to right and bottom-aligned
// as setup_panel_icons does: the icons' sprite and the sprite drawn for the
// weapon in hand (0 for none).
constexpr std::array<PanelIconDef, 6> kRatPanelIcons = {{{10, 0}, {11, 16}, {12, 18}, {13, 20}, {14, 22}, {15, 0}}};

// The litter bar's four segments' sprites at `level` (draw_ratfest_on_panel's
// switch): 0x1a full, 0x19 and 0x18 partly, 0x1b at level 0 on the turns the
// bar flashes, 0 for an empty segment.
std::array<uint8_t, 4> rat_bar_sprites(uint8_t level, uint32_t turn)
{
    constexpr uint8_t full = 0x1a, partly = 0x19, little = 0x18;
    switch (level) {
    case 0:
        return turn & 3 ? std::array<uint8_t, 4>{full, full, full, full}
                        : std::array<uint8_t, 4>{0x1b, 0x1b, 0x1b, 0x1b};
    case 1:
        return {full, full, full, full};
    case 2:
        return {full, full, full, partly};
    case 3:
        return {full, full, full, little};
    case 4:
        return {full, full, full, 0};
    case 5:
        return {full, full, partly, 0};
    case 6:
        return {full, full, little, 0};
    case 7:
        return {full, full, 0, 0};
    case 8:
        return {full, partly, 0, 0};
    case 9:
        return {full, little, 0, 0};
    case 10:
        return {full, 0, 0, 0};
    case 11:
        return {partly, 0, 0, 0};
    case 12:
        return {little, 0, 0, 0};
    default:
        return {0, 0, 0, 0};
    }
}

} // namespace

int rat_panel_top(const GameData &data)
{
    return kUiHeight - data.rat_panel().at(kRatPanelIcons[1].sprite).height;
}

void draw_rat_panel(UiLayer &layer, const RatPanelFigures &figures, const GameData &data)
{
    const SpriteSheet &panel = data.rat_panel();
    std::array<int, kRatPanelIcons.size()> left{};
    int x = 0;
    for (size_t i = 0; i < kRatPanelIcons.size(); i++) {
        Sprite sprite = panel.at(kRatPanelIcons[i].sprite);
        left[i] = x;
        layer.draw_sprite(x, kUiHeight - sprite.height, sprite);
        x += sprite.width;
    }
    char buf[16];
    std::snprintf(buf, sizeof buf, "%5d", figures.kills);
    layer.draw_text(data.font(0x29), 0x5d, 0x1be, buf, {{0, 0, kUiWidth}, {0, 0, kUiWidth, kUiHeight}});
    // The weapon in hand: its icon's pressed look, 11 pixels down.
    if (figures.powerup >= 0 && figures.powerup <= 3) {
        size_t icon = size_t(figures.powerup) + 1;
        Sprite pressed = panel.at(kRatPanelIcons[icon].pressed);
        layer.draw_sprite(left[icon], kUiHeight - panel.at(kRatPanelIcons[icon].sprite).height + 0xb, pressed);
    }
    // The litter bar: 10 less an eighth of the litter below 100, as the
    // game's byte (so 96 and more wrap past its cases and draw nothing).
    uint8_t level = figures.litter < 100 ? uint8_t(10 - figures.litter / 8) : 10;
    std::array<uint8_t, 4> segments = rat_bar_sprites(level, figures.turn);
    int bar_y = kUiHeight - panel.at(kRatPanelIcons[5].sprite).height + 0xc;
    int segment_width = panel.at(26).width;
    for (size_t i = 0; i < segments.size(); i++)
        layer.draw_sprite(left[5] + 0x2c + segment_width * int(i), bar_y, panel.at(segments[i]));
}

} // namespace th
