#include "core/ui/buy_objects.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/text.h"
#include "core/ui/hire_staff.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/objects.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace th
{

namespace
{

constexpr int kBuyWindow = int(Requester::BuyObjects);
constexpr int kEditWindow = int(Requester::EditRoom);

// SFX_* (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxSwoosh = 0x22;
constexpr uint8_t kSfxYesX = 0x41;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxSelectX = 0x43;
constexpr uint8_t kSfxHighlight5 = 0x44;
constexpr uint8_t kSfxAddItemJ = 0x5b;
constexpr uint8_t kSfxDeleteItemJ = 0x5c;

// The buy objects requester's buttons, counted from 1 in its window
// (hires_requester_buttons[0x2f] to [0x4e], src/reqbut.cpp): accept, the
// ten entries (a left click buys, a right one sells), cancel, then each
// entry's minus and plus.
constexpr int kBuyAccept = 17;
constexpr int kBuyEntry = 28;
constexpr int kBuyCancel = 38;
constexpr int kBuyMinus = 39;
constexpr int kBuyPlus = 49;
constexpr int kEntries = 10;

// Its layout, relative to the window (HiresCoords[0x19] to [0x1c], [0x34]
// and [0x3e] on; HiresBoxes[0xb] on; src/coords.cpp). Entry i is
// kEntryStep * i lower than entry 0.
constexpr Point kBuyTitle = {165, 20};
constexpr Point kBuyPrice = {26, 175};
constexpr Point kBuyTotal = {26, 204};
constexpr Point kBuyPreview = {75, 75};
constexpr Point kEntryName = {165, 41};
constexpr Point kEntryCount = {311, 41};
constexpr Rect kEntryBox = {160, 36, 193, 17};
constexpr int kEntryStep = 19;

// The edit room requester's layout (HiresCoords[0x6e] to [0x71],
// HiresBoxes[0x25] to [0x29]): its list starts at kListOrigin, a row every
// kListStep lines, at most kListRows rows in high resolution.
constexpr Point kListOrigin = {2, 152};
constexpr int kListStep = 29;
constexpr int kListTopY = 148;
constexpr int kListBottomDy = -33;
constexpr int kListRows = 8;
constexpr TextWindow kRoomNameBox = {17, 23, 153};
constexpr TextWindow kHintBox = {22, 48, 143};
constexpr Rect kRowBox = {22, 13, 119, 13};
constexpr TextWindow kRowName = {22, 15, 119};
constexpr TextWindow kRowCount = {154, 15, 16};
// The row's text clip is as wide as the name, the count and 20 more.
constexpr int kRowClipWidth = 16 + 119 + 0x14;

// REQ05V's sprites (hires_requester_sprites[0x14] to [0x1e]): per button
// from cancel to confirm, drawn over it while disabled, and while it is
// the mode (pick up's); the list's top, rows and bottom.
constexpr std::array<int, 4> kEditDisabledSprite = {0x7e, 0x7f, 0x80, 0x81};
constexpr int kListTopSprite = 0x7b;
constexpr int kListRowSprite = 0x7c;
constexpr int kListBottomSprite = 0x9c;

// req1_help strings: the buy objects requester's title and the prefixes of
// the price and the total.
constexpr int kTitleText = 3;
constexpr int kPriceText = 5;
constexpr int kTotalText = 6;

// The game draws below the view's bottom only on the panel: requester text
// is clipped to the view's height (statvar.view_height, 480 - 0x30).
constexpr Rect kViewClip = {0, 0, kUiWidth, kUiHeight - 0x30};

// objects[].preview_anim (src/tables.cpp): the anim start of each kind's
// picture in the requesters.
constexpr std::array<uint16_t, kObjectKinds> kPreviewAnims = {
    0,    450,  2527, 52,   451,  452,  2528, 453,  455,  454,  2529, 2530, 0,    459,  460,  461,
    463,  2531, 2532, 2533, 458,  2526, 2534, 2535, 2536, 2537, 466,  2538, 984,  2539, 2540, 1155,
    2541, 1177, 1179, 463,  2542, 2543, 0,    2544, 2545, 2546, 2547, 456,  457,  467,  465,  464,
    1275, 455,  2548, 2549, 998,  1003, 2550, 2551, 2552, 2553, 158,  154,  2554, 2555, 0,
};

// The anim starts get_frame_for_placing_bomb and get_frame_for_placing_plant
// (src/cursor.cpp) draw when no such object was picked up.
constexpr uint16_t kBombAnim = 0x9bf;
constexpr uint16_t kPlantAnim = 0x3cf;
constexpr uint8_t kObjLitterBomb = 17;
constexpr uint8_t kObjPlant = 45;

// MM_NORMAL, MM_BLUEPRINT, MM_PLACE_DOOR, MM_PLACE_OBJECT and 0x10
// (include/enums.h), EV_CANCEL_BUILD's arg.
constexpr uint16_t kMouseNormal = 0;
constexpr uint16_t kMouseBlueprint = 1;
constexpr uint16_t kMousePlaceDoor = 2;
constexpr uint16_t kMousePlaceObject = 5;
constexpr uint16_t kMouseWaitClear = 0x10;

// over_hotspot (src/utils.cpp).
bool over(int x, int y, int rx, int ry, int w, int h)
{
    return x >= rx && x < rx + w && y >= ry && y < ry + h;
}

bool window_open(const Interface &ui, int window)
{
    for (int i = 0; i < ui.window_count(); i++)
        if (ui.open_window_at(i) == window)
            return true;
    return false;
}

// LbPaletteFindColour (src/lib/bf1/gpalette.cpp) for 6-bit r, g, b: least
// squared distance, then least sum of differences, then the darkest, then
// the lowest index.
uint8_t find_colour(const Palette &palette, int r, int g, int b)
{
    auto channel = [&](int i, int shift) { return int((palette[size_t(i)] >> shift) & 0xff) >> 2; };
    auto sq = [](int v) { return v * v; };
    auto dist = [&](int i) { return sq(r - channel(i, 16)) + sq(g - channel(i, 8)) + sq(b - channel(i, 0)); };
    auto diff = [&](int i) {
        return std::abs(r - channel(i, 16)) + std::abs(g - channel(i, 8)) + std::abs(b - channel(i, 0));
    };
    auto dark = [&](int i) { return 2 * sq(channel(i, 16)) + 2 * sq(channel(i, 8)) + sq(channel(i, 0)); };
    int best = 999999;
    for (int i = 0; i < 256; i++)
        best = std::min(best, dist(i));
    int best_diff = 999999, best_dark = 999999, found = 0;
    for (int i = 0; i < 256; i++)
        if (dist(i) == best)
            best_diff = std::min(best_diff, diff(i));
    for (int i = 0; i < 256; i++) {
        if (dist(i) == best && diff(i) == best_diff && dark(i) < best_dark) {
            best_dark = dark(i);
            found = i;
        }
    }
    return uint8_t(found);
}

// Calls put(x, y, colour) for each pixel of composite frame `frame_id` with
// its origin at `at`, mirrored if `flip`, its elements placed and sized
// by `scale` (16.16) as draw_a_frame_scaled places them; 0x10000 places them
// as draw_a_frame and draw_from_frame. Pixels are taken from the nearest
// sprite pixel. Ghost flags are not applied.
template <typename Put>
void walk_frame(const GameData &data, uint32_t frame_id, Point at, bool flip, int scale, Put &&put)
{
    const AnimData &anims = data.anims();
    if (frame_id >= anims.frames.size())
        return;
    for (size_t i = anims.frames[frame_id].list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        uint16_t id = anims.lists[i];
        if (id >= anims.elements.size() || anims.elements[id].sprite == 0)
            continue;
        const AnimElement &e = anims.elements[id];
        Sprite s = data.sprites().at(e.sprite / 6);
        if (s.empty())
            continue;
        int ex = e.x - kElementBiasX, ey = e.y - kElementBiasY;
        unsigned flags = e.flags & (kDrawFlipX | kDrawFlipY);
        if (flip)
            flags ^= kDrawFlipX;
        int left = flip ? at.x - ((ex + s.width) * scale >> 16) : at.x + (ex * scale >> 16);
        int top = at.y + (ey * scale >> 16);
        int w = s.width * scale >> 16, h = s.height * scale >> 16;
        Surface pixels, mask;
        pixels.resize(s.width, s.height);
        mask.resize(s.width, s.height);
        mask.fill(0);
        draw_sprite(pixels, 0, 0, s, flags);
        draw_sprite_one_colour(mask, 0, 0, s, 1, flags);
        for (int dy = 0; dy < h; dy++) {
            int sy = dy * s.height / h;
            for (int dx = 0; dx < w; dx++) {
                int sx = dx * s.width / w;
                if (mask.row(sy)[sx])
                    put(left + dx, top + dy, pixels.row(sy)[sx]);
            }
        }
    }
}

// cost_of_all_selected_objects (src/req1.cpp).
int64_t cost_of_all_selected(const ObjectPlacing &p, const World &world)
{
    int64_t total = 0;
    for (int i = 0; i < kObjectKinds - 1; i++)
        total += int64_t(p.buy_count[size_t(i)]) * object_cost(world, i);
    return total;
}

// clicked_buy_objects (src/req11.cpp): `buy` or sell back `amount` of the
// selected kind if entry `entry` is listed, with its sound (SFX_ADDITEMJ,
// SFX_DELITEMJ, or SFX_WRONG2 for what it refuses).
void clicked_buy_objects(Game &game, int entry, bool buy, bool shift, bool ctrl)
{
    ObjectPlacing &p = game.world.placing;
    std::vector<uint8_t> &sounds = game.world.building.sounds;
    int amount = ctrl ? 10 : shift ? 5 : 1;
    int obj = p.item;
    if (!p.offered[size_t(entry)]) {
        // An empty entry only plays the error sound.
        sounds.push_back(kSfxWrong2);
        return;
    }
    if (obj >= kObjectKinds)
        return;
    if (buy) {
        int64_t cost = cost_of_all_selected(p, game.world) + int64_t(object_cost(game.world, obj)) * amount;
        if (game.balance > 0 && cost <= game.balance) {
            game.event = {EV_BUY_OBJECT, uint16_t(obj), 0, uint16_t(amount)};
            sounds.push_back(p.buy_count[size_t(obj)] + amount < 0x63 ? kSfxAddItemJ : kSfxWrong2);
        } else {
            sounds.push_back(kSfxWrong2);
        }
    } else if (p.buy_count[size_t(obj)] >= amount) {
        game.event = {EV_SELL_OBJECT, uint16_t(obj), 0, uint16_t(amount)};
        sounds.push_back(kSfxDeleteItemJ);
    } else {
        sounds.push_back(kSfxWrong2);
    }
}

// The edit room requester's list: calls row(kind, row) for each kind left
// to place, in kind order, up to kListRows; returns the rows.
template <typename Row> int for_each_list_row(const ObjectPlacing &p, Row &&row)
{
    int rows = 0;
    for (int obj = 0; obj < kObjectKinds - 1 && rows < kListRows; obj++)
        if (p.buy_count[size_t(obj)])
            row(obj, rows++);
    return rows;
}

void draw_buy_objects(UiLayer &layer, const Window &win, const ObjectUi &oui, const Game &game, const RequesterArt &art,
                      const GameData &data)
{
    const ObjectPlacing &p = game.world.placing;
    const LanguageText &text = data.text();
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    auto put = [&](const Font &font, int x, int y, std::string_view s) {
        layer.draw_text(font, win.x + x, win.y + y, s, {{0, 0, kUiWidth}, screen});
    };
    auto cost = [&](int kind) { return object_cost(game.world, kind); };
    int total = 0;
    for (int i = 0; i < kEntries && p.offered[size_t(i)]; i++)
        total += p.buy_count[p.offered[size_t(i)]] * cost(p.offered[size_t(i)]);
    put(art.font(), kBuyTitle.x, kBuyTitle.y, text.at(TextSection::Req1Help, kTitleText));
    put(art.font(), kBuyPrice.x, kBuyPrice.y,
        std::string(text.at(TextSection::Req1Help, kPriceText)) + std::to_string(cost(p.item)));
    put(art.font(), kBuyTotal.x, kBuyTotal.y,
        std::string(text.at(TextSection::Req1Help, kTotalText)) + std::to_string(total));

    const AnimData &anims = data.anims();
    if (p.item && p.item < kObjectKinds && kPreviewAnims[p.item] < anims.starts.size()) {
        const AnimStart &start = anims.starts[kPreviewAnims[p.item]];
        int x = win.x + kBuyPreview.x, y = win.y + kBuyPreview.y;
        int scale = 0x10000;
        if (start.frame < anims.frames.size()) {
            // Pictures are fitted to 121 pixels, but only when both their width
            // and their height are larger.
            const AnimFrame &frame = anims.frames[start.frame];
            int scale_x = frame.width ? 0x790000 / frame.width : 0x10000;
            int scale_y = frame.height ? 0x790000 / frame.height : 0x10000;
            if (scale_y < 0x10000 && scale_x < 0x10000)
                scale = std::min(scale_x, scale_y);
        }
        draw_layer_frame(layer, data, start.frame, {x, y}, start.flip != 0, scale);
    }

    for (int i = 0; i < kEntries && p.offered[size_t(i)]; i++) {
        int item = p.offered[size_t(i)];
        const Font &font = oui.hovered_entry == i ? art.highlight_font() : art.font();
        // objects[kind].name is the kind for every kind a requester offers.
        put(font, kEntryName.x, kEntryName.y + kEntryStep * i, text.at(TextSection::ObjectNames, item));
        put(font, kEntryCount.x, kEntryCount.y + kEntryStep * i, std::to_string(p.buy_count[size_t(item)]));
    }
}

void draw_edit_room(UiLayer &layer, const Window &win, const ObjectUi &oui, const Game &game, const RequesterArt &art,
                    const GameData &data)
{
    const ObjectPlacing &p = game.world.placing;
    const LanguageText &text = data.text();
    const SpriteSheet &sheet = art.sheet(kEditWindow);
    int ox = win.x, oy = win.y;
    for (size_t j = 1; j <= win.buttons.size(); j++) {
        int k = int(j) - kEditRoomCancel;
        if ((p.arg & (1u << j)) && k >= 0 && k < int(kEditDisabledSprite.size()))
            layer.draw_sprite(ox + win.buttons[j - 1].x, oy + win.buttons[j - 1].y,
                              sheet.at(size_t(kEditDisabledSprite[size_t(k)])));
    }
    layer.draw_text(art.font(), 0, 0, text.at(TextSection::RoomNames, kRoomTypeInfo[p.room_type % kRoomTypes].name),
                    {{ox + kRoomNameBox.x, oy + kRoomNameBox.y, kRoomNameBox.width}, kViewClip, kTextCentre});
    layer.draw_text(art.font(), 0, 0, text.misc(p.hint),
                    {{ox + kHintBox.x, oy + kHintBox.y, kHintBox.width}, kViewClip, kTextWrap});
    if (p.requester_mode < 6)
        return;

    int bx = ox + kListOrigin.x, top = oy + kListOrigin.y;
    int rows = for_each_list_row(
        p, [&](int, int row) { layer.draw_sprite(bx, top + row * kListStep, sheet.at(kListRowSprite)); });
    if (rows) {
        layer.draw_sprite(bx, top + rows * kListStep + kListBottomDy, sheet.at(kListBottomSprite));
        layer.draw_sprite(bx, oy + kListTopY, sheet.at(kListTopSprite));
    }
    for_each_list_row(p, [&](int obj, int row) {
        int by = top + row * kListStep;
        if (p.arg3 == obj) {
            // LbDrawBox's outline (draw_flags 0x10).
            Rect r = {ox + kRowBox.x, by + kRowBox.y, kRowBox.w, kRowBox.h};
            layer.fill({r.x, r.y, r.w, 1}, oui.red);
            layer.fill({r.x, r.y + r.h - 1, r.w, 1}, oui.red);
            layer.fill({r.x, r.y + 1, 1, r.h - 2}, oui.red);
            layer.fill({r.x + r.w - 1, r.y + 1, 1, r.h - 2}, oui.red);
        }
        const Font &font = obj == oui.highlighted ? art.highlight_font() : art.font();
        Rect clip = {ox + kRowBox.x, by + kRowBox.y, kRowClipWidth, kRowBox.h};
        layer.draw_text(font, 0, 0, text.at(TextSection::ObjectNames, obj),
                        {{ox + kRowName.x, by + kRowName.y, kRowName.width}, clip, kTextCentre});
        layer.draw_text(font, 0, 0, std::to_string(p.buy_count[size_t(obj)]),
                        {{ox + kRowCount.x, by + kRowCount.y, kRowCount.width}, clip, kTextCentre});
    });
}

// The flashing hint of req11_overdraw: the accept button (highlight 9) or
// the first entry's (0xa) in its active look, hires_requester_sprites[0x13]
// and [0x12].
void draw_buy_objects_flash(UiLayer &layer, const Window &win, const SpriteSheet &sheet, TutorialFlash &flash)
{
    constexpr int kAcceptSprite = 0xee, kEntrySprite = 0xf1;
    if (!flash.tutorial || !flash.tick() || (flash.highlight != 9 && flash.highlight != 0xa))
        return;
    bool accept = flash.highlight == 9;
    const WindowButton &b = win.buttons[size_t((accept ? kBuyAccept : kBuyEntry) - 1)];
    layer.draw_sprite(win.x + b.x, win.y + b.y, sheet.at(size_t(accept ? kAcceptSprite : kEntrySprite)));
    flash.drawn++;
}

// The flashing hint of req14_overdraw, drawn before the disabled buttons
// (which cover it): the button the highlight (9 to 12) names, in its sprite
// of hires_requester_sprites[0x18] to [0x1b].
void draw_edit_room_flash(UiLayer &layer, const Window &win, const SpriteSheet &sheet, TutorialFlash &flash)
{
    constexpr std::array<int, 4> kFlashSprite = {0x74, 0x76, 0x78, 0x7a};
    int k = int(flash.highlight) - kEditRoomCancel;
    if (!flash.tutorial || !flash.tick() || k < 0 || k >= int(kFlashSprite.size()))
        return;
    const WindowButton &b = win.buttons[size_t(kEditRoomCancel + k - 1)];
    layer.draw_sprite(win.x + b.x, win.y + b.y, sheet.at(size_t(kFlashSprite[size_t(k)])));
    flash.drawn++;
}

} // namespace

void draw_layer_frame(UiLayer &layer, const GameData &data, uint32_t frame, Point at, bool flip, int scale)
{
    Surface &pixels = layer.pixels();
    walk_frame(data, frame, at, flip, scale, [&](int px, int py, uint8_t c) {
        if (px >= 0 && px < layer.width() && py >= 0 && py < layer.height()) {
            pixels.row(py)[px] = c;
            layer.mark({px, py, 1, 1});
        }
    });
}

void init_object_ui(ObjectUi &ui, const Palette &palette)
{
    ui = {};
    ui.grey = grey_remap(palette);
    ui.red = find_colour(palette, 0x3f, 0, 0);
}

void furnish_corridor(Game &game)
{
    game.world.placing.placing = false;
    game.event = {EV_OPEN_FURNISH_CORRIDOR, 0, 0, 0};
}

void sync_object_requesters(Interface &ui, const Game &game)
{
    if (ui.modal())
        return;
    ObjectRequester want = game.world.placing.requester;
    for (auto [id, mine] : {std::pair{Requester::BuyObjects, ObjectRequester::BuyObjects},
                            std::pair{Requester::EditRoom, ObjectRequester::EditRoom}}) {
        bool open = window_open(ui, int(id));
        if (want == mine && !open) {
            if (ui.requester() != Requester::None)
                ui.close_requester(ui.requester());
            close_irequesters(ui);
            ui.open_requester(id);
        } else if (want != mine && open) {
            if (ui.requester() == id)
                ui.close_requester(id);
            else
                ui.close_window(int(id));
        }
    }
}

namespace
{

// The buy requester's entry under the pointer becomes the item it offers,
// with the beep on a change (req11_overdraw's hilight).
void object_requesters_buy_hover(const Interface &ui, ObjectUi &oui, ObjectPlacing &p, const UiMouse &mouse,
                                 std::vector<uint8_t> &sounds)
{
    const Window &win = ui.window(kBuyWindow);
    for (int i = 0; i < kEntries && p.offered[size_t(i)]; i++) {
        if (over(mouse.x, mouse.y, win.x + kEntryBox.x, win.y + kEntryBox.y + kEntryStep * i, kEntryBox.w,
                 kEntryBox.h)) {
            oui.hovered_entry = i;
            if (p.item != p.offered[size_t(i)])
                sounds.push_back(kSfxHighlight5);
            p.item = p.offered[size_t(i)];
        }
    }
}

// The edit room requester's object list: the row under the pointer, picking
// one up, the beep on a change and the window's height.
void object_requesters_edit_list(Interface &ui, ObjectUi &oui, ObjectPlacing &p, const UiMouse &mouse,
                                 std::vector<uint8_t> &sounds)
{
    const Window &win = ui.window(kEditWindow);
    int top = win.y + kListOrigin.y, rows = 0;
    if (p.requester_mode >= 6) {
        int selected = -1;
        rows = for_each_list_row(p, [&](int obj, int row) {
            int by = top + row * kListStep;
            if (mouse.x >= win.x + kRowBox.x && mouse.x < win.x + kRowBox.x + kRowBox.w && mouse.y >= by + kRowName.y &&
                mouse.y < by + kRowBox.y + kRowBox.h) {
                oui.highlighted = obj;
                if (oui.last_highlight != obj)
                    sounds.push_back(kSfxHighlight5);
                oui.last_highlight = obj;
                if (mouse.left_held || mouse.left_pressed)
                    selected = obj;
            }
        });
        if (selected >= 0) {
            if (p.arg3 != selected)
                sounds.push_back(kSfxSelectX);
            select_object_to_place(p, selected);
        }
        if (oui.highlighted < 0)
            oui.last_highlight = -1;
        p.buy_kind_count = uint8_t(rows);
    }
    // The game sets the height to the list's end on the screen, not
    // relative to the window.
    ui.set_window_height(kEditWindow, top + rows * kListStep);
}

void object_requesters_buy_button(Game &game, int i, bool old_left, UiModifiers keys)
{
    ObjectPlacing &p = game.world.placing;
    std::vector<uint8_t> &sounds = game.world.building.sounds;
    if (i >= kBuyEntry && i < kBuyEntry + kEntries) {
        clicked_buy_objects(game, i - kBuyEntry, old_left, keys.shift, keys.ctrl);
    } else if (i >= kBuyPlus && i < kBuyPlus + kEntries) {
        clicked_buy_objects(game, i - kBuyPlus, true, keys.shift, keys.ctrl);
    } else if (i >= kBuyMinus && i < kBuyMinus + kEntries) {
        clicked_buy_objects(game, i - kBuyMinus, false, keys.shift, keys.ctrl);
    } else if (i == kBuyAccept) {
        // Furnishing a room (arg3), back to placing with what was bought;
        // SFX_YESX always (req11.cpp:46).
        if (p.arg == 1)
            game.event = {EV_PLACE_NEXT_OBJECT, 0, 0, 0};
        else if (p.arg3)
            game.event = {EV_BACK_TO_PLACE_OBJECT, 0, 0, 0};
        sounds.push_back(kSfxYesX);
    } else if (i == kBuyCancel && p.arg3) {
        game.event = {EV_BACK_TO_PLACE_OBJECT, 1, 0, 0};
        sounds.push_back(kSfxNo4);
    } else if (i == kBuyCancel) {
        for (int k = 0; k < kObjectKinds - 1; k++)
            p.buy_count[size_t(k)] = 0;
        game.event = {EV_CANCEL_BUY_OBJECTS, 0, 0, 0};
        sounds.push_back(kSfxNo4);
    }
}

} // namespace

void object_requesters_input(Interface &ui, ObjectUi &oui, Game &game, const InterfaceEvents &events,
                             const UiMouse &mouse, UiModifiers keys)
{
    ObjectPlacing &p = game.world.placing;
    std::vector<uint8_t> &sounds = game.world.building.sounds;
    oui.hovered_entry = -1;
    oui.highlighted = -1;
    if (window_open(ui, kBuyWindow))
        object_requesters_buy_hover(ui, oui, p, mouse, sounds);
    if (window_open(ui, kEditWindow))
        object_requesters_edit_list(ui, oui, p, mouse, sounds);
    if (events.triggered_window == kBuyWindow)
        object_requesters_buy_button(game, events.triggered_button, events.old_left, keys);
    // Buy more, pick up and confirm are disabled in the corridor.
    if (events.triggered_window == kEditWindow && events.triggered_button == kEditRoomCancel &&
        !(p.arg & (1u << kEditRoomCancel))) {
        object_escape(game);
        sounds.push_back(kSfxNo4);
    }
}

void placement_mouse(ObjectUi &oui, Game &game, int cell, const UiMouse &mouse, bool on_map)
{
    ObjectPlacing &p = game.world.placing;
    oui.cursor_shown = false;
    if (!on_map)
        return;
    if (!p.placing) {
        // Not while a room is built or furnished: its edit room requester
        // stays (process_right_button).
        bool room_edited = game.world.building.build.room && p.requester == ObjectRequester::EditRoom;
        if (mouse.right_pressed && p.requester != ObjectRequester::None && !room_edited)
            object_escape(game);
        return;
    }
    if (cell < 0 || cell >= kMapCells)
        return;
    // Objects hugging a room's wall turn with the wall instead (mode 1).
    if (mouse.right_pressed && p.mode != 1 && change_placing_angle(p, -1))
        game.world.building.sounds.push_back(kSfxSwoosh);
    game.update_placement(cell);
    oui.cursor_shown = true;
    if (mouse.left_pressed && p.ok)
        game.event = {EV_PLACE_OBJECT, p.tile, 0, uint16_t(p.kind | p.dir << 8)};
}

bool object_escape(Game &game)
{
    const ObjectPlacing &p = game.world.placing;
    BuildMouse build = game.world.building.mouse;
    if (p.requester == ObjectRequester::None && !p.placing && build == BuildMouse::None)
        return false;
    if (p.requester == ObjectRequester::BuyObjects && p.arg3) {
        // Furnishing a room: back to placing what was to be placed before.
        game.event = {EV_BACK_TO_PLACE_OBJECT, 1, 0, 0};
        return true;
    }
    if (p.requester == ObjectRequester::EditRoom && p.requester_mode == 0) {
        // An edited room asks first (yesno_text[4]): the app opens the
        // requester for this EV_OPEN_YES_NO.
        if (game.world.editing_room)
            game.event = {EV_OPEN_YES_NO, kDeleteEditedRoomQuestion, 0, 0};
        else
            game.event = {EV_DELETE_ROOM, 0, 0, 0};
        return true;
    }
    if (p.requester == ObjectRequester::EditRoom && p.room_type != kRoomCorridor) {
        game.event = {EV_EDIT_BLUEPRINT, 0, 0, 0};
        return true;
    }
    uint16_t mouse = build == BuildMouse::Blueprint   ? kMouseBlueprint
                     : build == BuildMouse::PlaceDoor ? kMousePlaceDoor
                     : build == BuildMouse::WaitClear ? kMouseWaitClear
                     : p.placing                      ? kMousePlaceObject
                                                      : kMouseNormal;
    game.event = {EV_CANCEL_BUILD, 0, 0, mouse};
    return true;
}

void draw_object_requesters(UiLayer &layer, const Interface &ui, const ObjectUi &oui, const Game &game,
                            const RequesterArt &art, const GameData &data)
{
    if (window_open(ui, kBuyWindow)) {
        draw_buy_objects_flash(layer, ui.window(kBuyWindow), art.sheet(kBuyWindow), ui.flash());
        draw_buy_objects(layer, ui.window(kBuyWindow), oui, game, art, data);
    }
    if (window_open(ui, kEditWindow)) {
        draw_edit_room_flash(layer, ui.window(kEditWindow), art.sheet(kEditWindow), ui.flash());
        draw_edit_room(layer, ui.window(kEditWindow), oui, game, art, data);
    }
}

namespace
{

// get_frame_for_placing_plant (src/cursor.cpp): the frame of the player's plant
// lifted off the map by its health, else a healthy plant's.
uint32_t placing_plant_frame(const World &world, const AnimData &anims)
{
    for (uint16_t id : world.building.build.objects) {
        const Thing &obj = world.things[id];
        if (id && obj.subtype == kObjPlant && obj.owner == 0)
            return compute_plant_frame(obj, anims);
    }
    return anims.starts[kPlantAnim].frame;
}

} // namespace

void draw_placement(Surface &dst, Rect view, Point camera, const World &world, const ObjectUi &oui,
                    const GameData &data)
{
    const ObjectPlacing &p = world.placing;
    if (!p.placing || !oui.cursor_shown || p.kind >= kObjectKinds)
        return;
    const AnimData &anims = data.anims();
    auto start_of = [&](int anim) {
        return size_t(anim) < anims.starts.size() ? anims.starts[size_t(anim)] : AnimStart{};
    };
    auto draw = [&](int tile, AnimStart start) {
        int col = tile % kMapSize, row = tile / kMapSize;
        int x = (col - row) * kTileWidth / 2 - camera.x + view.x;
        int y = (col + row) * kTileHeight / 2 + kTileHeight / 2 - camera.y + view.y;
        if (p.ok) {
            draw_frame(dst, {x, y}, start.frame, start.flip != 0, data);
            return;
        }
        // draw_from_frame with bw_remap (LbSpriteDrawRemap).
        walk_frame(data, start.frame, {x, y}, start.flip != 0, 0x10000, [&](int px, int py, uint8_t c) {
            if (px >= view.x && px < view.x + view.w && py >= view.y && py < view.y + view.h)
                dst.row(py)[px] = oui.grey[c];
        });
    };
    dst.set_clip(view);
    const ObjectInfo &control = kObjects[p.kind];
    // A litter bomb is drawn at its first frame: the game uses that of a
    // lifted bomb by its fuse (get_frame_for_placing_bomb), and the port keeps
    // no fuse.
    AnimStart start = p.kind == kObjLitterBomb ? AnimStart{start_of(kBombAnim).frame, 0}
                      : p.kind == kObjPlant    ? AnimStart{placing_plant_frame(world, anims), 0}
                                               : start_of(control.anim_start + p.dir);
    draw(p.tile, start);
    // The game draws the second part first for some facings (next_part's
    // draw_part_first), which is not tabulated here; no object the corridor
    // offers has one.
    if (control.next_part != -1) {
        const NextPartInfo &part = kNextParts[size_t(control.next_part)];
        draw(p.tile + part.offset[p.dir], start_of(kObjects[part.kind].anim_start + part.dir[p.dir]));
    }
    dst.reset_clip();
}

} // namespace th
