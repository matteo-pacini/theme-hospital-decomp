#include "core/ui/popups.h"

#include "core/data/gamedata.h"
#include "core/world/popups.h"
#include "core/world/world.h"

#include <algorithm>

namespace th
{

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks

// panel_popup_coord (src/panel.cpp): per slide step, the icon's offset; up
// to step 0x22 from slot 0's x, then from its own slot's. (0, 0) ends it.
constexpr std::array<Point, 40> kPopupSlide = {{
    {0, 0},    {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29}, {174, 29},
    {174, 29}, {174, 29}, {174, 29}, {174, 25}, {174, 20}, {174, 14}, {174, 7},  {174, 0},  {174, 0},  {174, 0},
    {174, 0},  {174, 0},  {174, 0},  {170, 0},  {165, 0},  {153, 0},  {140, 0},  {125, 0},  {113, 0},  {93, 0},
    {73, 0},   {53, 0},   {33, 0},   {1, 0},    {1, 0},    {2, 0},    {4, 0},    {4, 0},    {2, 0},    {0, 0},
}};
constexpr uint8_t kSlideFromSlot = 0x22;

// panel_emergency_spr (src/tables.cpp): the emergency icon's flash, panel
// sprites; 0 starts it again.
constexpr std::array<uint8_t, 16> kPanelEmergencySprites = {60, 60, 59, 59, 58, 58, 57, 57, 57, 57, 57, 57};

// The emergency icon's sprite (popup_icons[2]).
constexpr uint8_t kEmergencyIcon = 43;

// othervar.popup_shutter_state: open (1), closing (2), closed (3),
// opening (4).
constexpr uint8_t kShutterOpen = 1;
constexpr uint8_t kShutterClosing = 2;
constexpr uint8_t kShutterClosed = 3;
constexpr uint8_t kShutterOpening = 4;
constexpr uint8_t kShutterFull = 0x18;
constexpr uint8_t kShutterHold = 0x28;

bool slide_end(Point p)
{
    return p.x == 0 && p.y == 0;
}

Point slide(uint8_t step)
{
    return step < kPopupSlide.size() ? kPopupSlide[step] : Point{0, 0};
}

// Where draw_popup_icons puts a call's icon, relative to the icons' window.
Point popup_position(const World::PlayerCall &call)
{
    if (!call.slide_step)
        return {kPopupSlotX[call.slot], 0};
    int x = call.slide_step > kSlideFromSlot ? kPopupSlotX[call.slot] : kPopupSlotX[0];
    Point d = slide(call.slide_step);
    return {x + d.x, d.y};
}

// draw_popup_icons' move of a call's icon along its slide: held at its first
// step until the shutter has closed; steps short of its slot's x are passed
// over, and the slide ends at its last step.
void popup_slide(World::PlayerCall &call, uint8_t shutter_state)
{
    if (!call.slide_step)
        return;
    if (slide_end(slide(call.slide_step))) {
        call.slide_step = 0;
        return;
    }
    if (call.slide_step != 1 || shutter_state == kShutterClosed)
        call.slide_step++;
    while (slide(call.slide_step).x < kPopupSlotX[call.slot]) {
        if (slide_end(slide(call.slide_step))) {
            // Short of the last step, slots 4-7 end at once and the others
            // jump to step 0x22; neither happens with the game's table.
            bool short_slot = call.slot >= 4 && call.slot <= 7;
            call.slide_step = call.slide_step > kSlideFromSlot || short_slot ? 0 : kSlideFromSlot;
            break;
        }
        call.slide_step++;
    }
}

// draw_popup_icons' shutter, as it runs in high resolution (its drawing is
// the low resolution's).
void popup_shutter(PanelPopups &p)
{
    switch (p.shutter_state) {
    case kShutterClosing:
        p.shutter_width = p.shutter_width >= 2 ? uint8_t(p.shutter_width - 2) : 0;
        if (p.shutter_width == 0) {
            p.shutter_state = kShutterClosed;
            p.shutter_timer = 0;
        }
        break;
    case kShutterClosed:
        p.shutter_width = 0;
        if (++p.shutter_timer > kShutterHold)
            p.shutter_state = kShutterOpening;
        break;
    case kShutterOpening:
        if (p.shutter_timer <= kShutterHold) {
            p.shutter_timer++;
            break;
        }
        p.shutter_width = p.shutter_width + 2 < kShutterFull ? uint8_t(p.shutter_width + 2) : kShutterFull + 1;
        if (p.shutter_width >= kShutterFull)
            p.shutter_state = kShutterOpen;
        break;
    case kShutterOpen:
        p.shutter_width = kShutterFull;
        break;
    default:
        p.shutter_state = kShutterOpen;
        break;
    }
}

// The icons' window: its top (4 pixels below where the icons would stand on
// the panel's left end) and height.
Rect popup_window(const GameData &data)
{
    int height = data.panel().at(kPopupIcons[1]).height;
    int panel_y = kUiHeight - data.panel().at(1).height;
    return {0, panel_y - height + kPopupWindowDy, kUiWidth, height};
}

// One game frame of draw_popup_icons: the rising icons as they stand before
// moving on, then those level with their slots after.
void popup_icons_step(PopupIcons &icons, World &world, const GameData &data)
{
    maintain_panel_popup(world);
    PanelPopups &p = world.popups;
    if (kPanelEmergencySprites[++icons.emergency_frame % kPanelEmergencySprites.size()] == 0)
        icons.emergency_frame = 0;
    icons.redraw = p.shutter_state >= kShutterOpen && p.shutter_state <= kShutterOpening;
    icons.rising_count = icons.level_count = icons.hover_count = 0;
    Rect window = popup_window(data);
    bool still = true;
    for (uint8_t kind = 1; kind < kPopupIcons.size(); kind++) {
        uint16_t head = world.first_call[0][kind];
        if (!head || world.calls[head].slot >= kPopupSlotX.size())
            continue;
        World::PlayerCall &call = world.calls[head];
        Point at = popup_position(call);
        popup_slide(call, p.shutter_state);
        PopupIcons::Icon icon = {kind, at.x, at.y, window.y - kPopupWindowDy + at.y};
        icons.hover[icons.hover_count++] = icon;
        if (at.y) {
            icons.rising[icons.rising_count++] = icon;
            p.shutter_state = kShutterClosing;
            still = false;
        }
        icons.redraw = true;
    }
    if (still)
        p.shutter_state = kShutterOpening;
    for (uint8_t kind = 1; kind < kPopupIcons.size(); kind++) {
        uint16_t head = world.first_call[0][kind];
        if (!head || world.calls[head].slot >= kPopupSlotX.size())
            continue;
        Point at = popup_position(world.calls[head]);
        if (at.y == 0)
            icons.level[icons.level_count++] = {kind, at.x, 0, window.y - kPopupWindowDy};
    }
    popup_shutter(p);
}

void draw_popup_icon(UiLayer &layer, const PopupIcons &icons, const GameData &data, const PopupIcons::Icon &icon,
                     int top)
{
    uint8_t sprite = kPopupIcons[icon.kind];
    layer.draw_sprite(icon.x, top + icon.y, data.panel().at(sprite));
    if (sprite == kEmergencyIcon)
        layer.draw_sprite(icon.x + kPopupEmergencyFlash.x, top + icon.y + kPopupEmergencyFlash.y,
                          data.panel().at(kPanelEmergencySprites[icons.emergency_frame]));
}

} // namespace

void popup_icons_frame(PopupIcons &icons, World &world, const UiMouse &mouse, const GameData &data, double seconds)
{
    icons.frame_time += seconds;
    if (icons.frame_time >= kGameFrame) {
        icons.frame_time = std::min(icons.frame_time - kGameFrame, kGameFrame);
        popup_icons_step(icons, world, data);
    }
    // The last icon found under the mouse wins, as in the game.
    icons.hovered = 0;
    int height = data.panel().at(kPopupIcons[1]).height;
    for (uint8_t i = 0; i < icons.hover_count; i++) {
        const PopupIcons::Icon &icon = icons.hover[i];
        int width = data.panel().at(kPopupIcons[icon.kind]).width;
        if (mouse.x >= icon.x && mouse.x < icon.x + width && mouse.y >= icon.hover_y && mouse.y < icon.hover_y + height)
            icons.hovered = icon.kind;
    }
}

void draw_popup_icons(UiLayer &layer, const PopupIcons &icons, const GameData &data, const PanelState &state,
                      const PanelFigures *figures)
{
    Rect window = popup_window(data);
    Surface &pixels = layer.pixels();
    pixels.set_clip(window);
    for (uint8_t i = 0; i < icons.rising_count; i++)
        draw_popup_icon(layer, icons, data, icons.rising[i], window.y);
    pixels.reset_clip();
    if (icons.redraw)
        redraw_panel_left(layer, state, figures, data);
    pixels.set_clip(window);
    for (uint8_t i = 0; i < icons.level_count; i++)
        draw_popup_icon(layer, icons, data, icons.level[i], window.y);
    pixels.reset_clip();
}

Event popup_icon_click(const PopupIcons &icons)
{
    if (!icons.hovered)
        return {};
    return {EV_ANSWER_PLAYERCALL, icons.hovered, 0, 0};
}

} // namespace th
