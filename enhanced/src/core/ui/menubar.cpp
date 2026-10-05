#include "core/ui/menubar.h"

#include "core/data/gamedata.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace th
{

namespace
{

// HiresCoords' pulldown_text_dy (src/coords.cpp; 1 in 320x200).
constexpr int kTextDy = 2;

// pmenu.count for the game's menus: kPulldownDefs holds pulldowns[1] to
// [kPulldownEnd - 1], the bar's menus up to [kBarCount - 1].
constexpr int kBarCount = 6;
constexpr int kPulldownEnd = 1 + int(std::size(kPulldownDefs));

struct Origin {
    int x;
    int y;
};

bool fail(std::string *error, const char *why)
{
    if (error)
        *error = std::string("pulldown_text: ") + why;
    return false;
}

bool over_hotspot(int x, int y, int rx, int ry, int w, int h)
{
    return x >= rx && x < rx + w && y >= ry && y < ry + h;
}

// x of top-level menu `p` on the bar: the widths of the shown menus before
// it.
int title_x(const MenuBar &bar, int p)
{
    int x = 0;
    for (int i = 1; i < p; i++)
        if (bar.pulldowns[i].active)
            x += bar.pulldowns[i].title_length * bar.char_width;
    return x;
}

// Where pulldown `p`'s box is drawn: under its title, or for a submenu to
// the right of its parent's box, level with the item that opens it.
Origin pulldown_origin(const MenuBar &bar, int p)
{
    if (p < bar.count)
        return {title_x(bar, p), bar.line_height};
    int parent = bar.pulldowns[p].parent;
    int x = title_x(bar, parent) + bar.pulldowns[parent].width * bar.char_width + bar.sprites.at(6).width;
    return {x, bar.pulldowns[p].y};
}

// The pulldowns drawn while the button is held, in draw_menus' order: the
// open one, or a submenu's parent and then the submenu.
int drawn_pulldowns(const MenuBar &bar, int out[2])
{
    if (!bar.held || bar.drawn_open == 0)
        return 0;
    if (bar.drawn_open < bar.count) {
        out[0] = bar.drawn_open;
        return bar.pulldowns[out[0]].active ? 1 : 0;
    }
    out[0] = bar.pulldowns[bar.drawn_open].parent;
    out[1] = bar.drawn_open;
    return 2;
}

// The hover part of draw_pulldown: the item of pulldown `p` under the mouse
// becomes selected_option, sounding a change.
void hover_pulldown(MenuBar &bar, int p, MenuEvent &ev)
{
    const MenuBar::Pulldown &pd = bar.pulldowns[p];
    Origin o = pulldown_origin(bar, p);
    for (int i = 0; i < pd.count; i++) {
        if (!over_hotspot(bar.mouse_x, bar.mouse_y, o.x, o.y + i * bar.line_height, pd.width * bar.char_width,
                          bar.line_height))
            continue;
        if (bar.selected_option != i + 1 || bar.last_p != p) {
            bar.last_p = p;
            ev.sound = MenuSound::Highlight;
        }
        bar.selected_option = i + 1;
    }
}

int item_value(const MenuItemDef &item, int index)
{
    switch (item.action) {
    case MenuAction::Load:
    case MenuAction::Save:
        return index + 1;
    case MenuAction::SoundVolume:
    case MenuAction::AnnouncementVolume:
    case MenuAction::MusicVolume:
        return index;
    default:
        return item.value;
    }
}

bool ticked(const MenuItemDef &item, int index, const MenuContext &c)
{
    switch (item.tick) {
    case MenuTick::None:
        return false;
    case MenuTick::Sound:
        return c.sound;
    case MenuTick::Announcements:
        return c.announcements;
    case MenuTick::Music:
        return c.music;
    case MenuTick::Autosave:
        return c.autosave;
    case MenuTick::SaveSlot:
        return index < int(c.save_slots.size()) && c.save_slots[size_t(index)];
    case MenuTick::HighRes:
        return c.high_res;
    case MenuTick::LowRes:
        return c.low_res;
    case MenuTick::Shadows:
        return c.shadows;
    case MenuTick::SoundVolume:
        return c.sound_volume == index;
    case MenuTick::AnnouncementVolume:
        return c.announcement_volume == index;
    case MenuTick::MusicVolume:
        return c.music_volume == index;
    case MenuTick::Speed:
        return c.speed == index;
    }
    return false;
}

// draw_pmenu_box: a pull-down's frame and background, w x h inside, at
// (x, y). The game sets draw flag 8 (see-through) for it, but reqspr_draw
// ignores the draw flags, so it is solid.
void draw_pmenu_box(UiLayer &layer, const SpriteSheet &s, int x, int y, int w, int h)
{
    const Sprite fill = s.at(5);
    for (int cy = y + fill.height; cy <= y + h; cy += fill.height)
        for (int cx = x + fill.width; cx <= x + w; cx += fill.width)
            layer.draw_sprite(cx, cy, fill);
    for (int cx = x + s.at(2).width; cx <= x + w; cx += s.at(2).width) {
        layer.draw_sprite(cx, y, s.at(2));
        layer.draw_sprite(cx, y + h, s.at(8));
    }
    for (int cy = y + s.at(4).height; cy <= y + h; cy += s.at(4).height) {
        layer.draw_sprite(x, cy, s.at(4));
        layer.draw_sprite(x + w, cy, s.at(6));
    }
    layer.draw_sprite(x, y, s.at(1));
    layer.draw_sprite(x + w, y, s.at(3));
    layer.draw_sprite(x, y + h, s.at(7));
    layer.draw_sprite(x + w, y + h, s.at(9));
}

void draw_label(UiLayer &layer, const Font &font, int x, int y, const std::string &text)
{
    layer.draw_text(font, {0, 0, layer.width()}, x, y, text, 0, {0, 0, layer.width(), layer.height()});
}

// draw_pulldown: pulldown `p`'s box at (x, y), its items and check marks,
// the item under the mouse in the highlight font.
void draw_pulldown(UiLayer &layer, const MenuBar &bar, int x, int y, int p)
{
    const MenuBar::Pulldown &pd = bar.pulldowns[p];
    draw_pmenu_box(layer, bar.sprites, x, y, pd.width * bar.char_width, pd.count * bar.line_height);
    for (int i = 0; i < pd.count; i++) {
        const MenuItemDef &item = kMenuItems[pd.first + i];
        int ty = y + i * bar.line_height + bar.text_dy;
        if (ticked(item, i, bar.context))
            layer.draw_sprite(x, ty, bar.sprites.at(10));
        const std::string &label = bar.text[item.text];
        draw_label(layer, bar.font, x, ty, label);
        if (over_hotspot(bar.mouse_x, bar.mouse_y, x, y + i * bar.line_height, pd.width * bar.char_width,
                         bar.line_height))
            draw_label(layer, bar.font_hi, x, ty, label);
    }
}

} // namespace

// setup_menus (src/menu.cpp), with the items of pmenu's initial data.
bool MenuBar::load(const std::filesystem::path &dir, const LanguageText &lang, std::string *error)
{
    *this = MenuBar{};
    auto tab = load_data_file(dir, "data/pulldv.tab", error);
    if (!tab)
        return false;
    auto dat = load_data_file(dir, "data/pulldv.dat", error);
    if (!dat)
        return false;
    if (!sprites.load(std::move(*tab), std::move(*dat), SpriteFormat::Req) || sprites.size() < 11) {
        if (error)
            *error = "data/pulldv: bad sprite index";
        return false;
    }
    if (!load_font(dir, 1, font, error) || !load_font(dir, 2, font_hi, error))
        return false;

    for (size_t i = 0; i < lang.size(TextSection::Pulldown); i++)
        text.emplace_back(lang.at(TextSection::Pulldown, int(i)));
    auto at = [&](size_t i) -> const std::string * { return i < text.size() ? &text[i] : nullptr; };

    text_dy = kTextDy;
    line_height = font.glyph('A').height + text_dy;
    char_width = font.glyph('A').width;

    // Titles up to ".", then each menu's items up to ".", the submenus' last
    // ending with "..".
    size_t i = 1;
    for (;; i++) {
        const std::string *s = at(i);
        if (!s || i >= pulldowns.size())
            return fail(error, "no end to the titles");
        pulldowns[i].title_length = int(s->size());
        if (*s == ".")
            break;
    }
    count = int(i);
    i++;
    bool done = false;
    for (int j = 1; !done; j++) {
        if (j >= int(pulldowns.size()))
            return fail(error, "too many menus");
        Pulldown &pd = pulldowns[size_t(j)];
        for (int sub = 0;; sub++, i++) {
            const std::string *s = at(i);
            if (!s)
                return fail(error, "no end to the menus");
            if (*s == "." || (j >= count && *s == "..")) {
                done = *s == "..";
                pd.count = sub;
                i++;
                break;
            }
            pd.width = std::max(pd.width, int(s->size()));
        }
    }
    if (count != kBarCount)
        return fail(error, "unexpected menus");
    for (int p = 1; p < kPulldownEnd; p++) {
        const PulldownDef &def = kPulldownDefs[p - 1];
        Pulldown &pd = pulldowns[size_t(p)];
        pd.active = def.active;
        pd.text = def.text;
        pd.first = def.first;
        pd.count = std::min(pd.count, int(def.count));
        for (int k = 0; k < pd.count; k++)
            if (text.size() <= kMenuItems[def.first + k].text)
                return fail(error, "too few strings");
    }

    int sub = count;
    for (int p = 1; p < count; p++)
        for (int j = 0; j < pulldowns[size_t(p)].count; j++) {
            if (!kMenuItems[pulldowns[size_t(p)].first + j].submenu)
                continue;
            if (sub >= kPulldownEnd)
                return fail(error, "unexpected submenus");
            pulldowns[size_t(sub)].parent = p;
            pulldowns[size_t(sub)].y = line_height + j * line_height;
            pulldowns[size_t(p)].submenu[size_t(j)] = sub++;
        }
    subcount = sub;
    return true;
}

MenuEvent menu_frame(MenuBar &bar, const UiMouse &mouse, const MenuContext &context)
{
    MenuEvent ev;
    if (bar.count == 0)
        return ev;
    const int h1 = bar.sprites.at(1).height;
    const int h2 = bar.sprites.at(4).height;
    const int h3 = bar.sprites.at(7).height * 2;

    // draw_menus, which the game runs before process_menus: whether the bar
    // shows, and the item under the mouse in the open menus.
    bar.context = context;
    bar.mouse_x = mouse.x;
    bar.mouse_y = mouse.y;
    bar.held = mouse.left_held;
    bar.drawn_open = bar.open;
    bar.active = ((mouse.y >= 0 && mouse.y < h1 + h2 + h3) || bar.open) && !context.disabled;
    if (bar.active) {
        int shown[2];
        int n = drawn_pulldowns(bar, shown);
        for (int k = 0; k < n; k++)
            hover_pulldown(bar, shown[k], ev);
    }

    // process_menus. Only the open pulldown can hold the released item.
    if (mouse.left_released && !context.disabled && bar.open && bar.pulldowns[size_t(bar.open)].active) {
        const MenuBar::Pulldown &pd = bar.pulldowns[size_t(bar.open)];
        Origin o = pulldown_origin(bar, bar.open);
        for (int j = 0; j < pd.count; j++) {
            if (!over_hotspot(mouse.x, mouse.y, o.x, o.y + bar.line_height * j, pd.width * bar.char_width,
                              bar.line_height))
                continue;
            const MenuItemDef &item = kMenuItems[pd.first + j];
            if ((!item.usable_paused && context.paused) || (context.rat_level && !item.usable_rat_level)) {
                ev.sound = MenuSound::Wrong;
            } else {
                ev.action = item.action;
                ev.value = item_value(item, j);
                ev.sound = MenuSound::Select;
            }
            break;
        }
    }
    if (!mouse.left_held || context.disabled) {
        bar.open = 0;
        return ev;
    }
    if (mouse.y < h1 + h2) {
        // Over the bar: the title under the mouse opens.
        int x2 = 0;
        int w = bar.char_width * bar.pulldowns[1].title_length;
        for (int i = 1; i < bar.count; i++) {
            if (!bar.pulldowns[size_t(i)].active)
                continue;
            if (over_hotspot(mouse.x, mouse.y, x2, 0, w, bar.line_height)) {
                if (bar.open != i)
                    ev.sound = MenuSound::Highlight;
                bar.open = i;
                break;
            }
            x2 += w;
            w = bar.pulldowns[size_t(i) + 1].title_length * bar.char_width;
        }
        return ev;
    }
    // Back over a submenu's parent, then onto an item that opens a submenu.
    if (bar.open >= bar.count) {
        int parent = bar.pulldowns[size_t(bar.open)].parent;
        const MenuBar::Pulldown &pd = bar.pulldowns[size_t(parent)];
        if (over_hotspot(mouse.x, mouse.y, title_x(bar, parent), bar.line_height, pd.width * bar.char_width,
                         pd.count * bar.line_height))
            bar.open = parent;
    }
    if (bar.open && bar.pulldowns[size_t(bar.open)].active) {
        const MenuBar::Pulldown &pd = bar.pulldowns[size_t(bar.open)];
        int x = title_x(bar, pd.parent ? pd.parent : bar.open);
        for (int j = 0; j < pd.count; j++) {
            if (pd.submenu[size_t(j)] && over_hotspot(mouse.x, mouse.y, x, bar.line_height + bar.line_height * j,
                                                      pd.width * bar.char_width, bar.line_height)) {
                bar.open = pd.submenu[size_t(j)];
                break;
            }
        }
    }
    return ev;
}

void draw_menu_bar(UiLayer &layer, const MenuBar &bar)
{
    if (!bar.active || bar.count == 0)
        return;
    const SpriteSheet &s = bar.sprites;
    const int h1 = s.at(1).height;
    const int h2 = s.at(4).height;
    const int tile = s.at(1).width;
    const int w = layer.width();
    layer.draw_sprite(0, 0, s.at(1));
    layer.draw_sprite(0, h1, s.at(4));
    layer.draw_sprite(0, h1 + h2, s.at(7));
    for (int x = tile; x < w - tile; x += tile) {
        layer.draw_sprite(x, 0, s.at(2));
        layer.draw_sprite(x, h1, s.at(5));
        layer.draw_sprite(x, h1 + h2, s.at(8));
    }
    layer.draw_sprite(w - tile, 0, s.at(3));
    layer.draw_sprite(w - tile, h1, s.at(6));
    layer.draw_sprite(w - tile, h1 + h2, s.at(9));

    int x = 0;
    for (int i = 1; i < bar.count; i++) {
        const MenuBar::Pulldown &pd = bar.pulldowns[size_t(i)];
        if (!pd.active)
            continue;
        draw_label(layer, bar.font, x, bar.text_dy, bar.text[size_t(pd.text)]);
        if (bar.held && bar.drawn_open == i)
            draw_pulldown(layer, bar, x, bar.line_height, i);
        x += pd.title_length * bar.char_width;
    }
    if (bar.held && bar.drawn_open >= bar.count) {
        int parent = bar.pulldowns[size_t(bar.drawn_open)].parent;
        draw_pulldown(layer, bar, title_x(bar, parent), bar.line_height, parent);
        Origin o = pulldown_origin(bar, bar.drawn_open);
        draw_pulldown(layer, bar, o.x, o.y, bar.drawn_open);
    }
}

} // namespace th
