#include "core/ui/build_room.h"

#include "core/audio/sounds.h"
#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/ui/buy_objects.h"
#include "core/world/build.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/objects.h"

#include <algorithm>
#include <climits>
#include <string>

namespace th
{

namespace
{

constexpr int kBuildWindow = int(Requester::BuildRoom);
constexpr int kEditWindow = int(Requester::EditRoom);

// The build room requester's buttons, counted from 1 in its window
// (hires_requester_buttons[0x17] to [0x25], src/reqbut.cpp): the four
// departments, the ten rooms and close.
constexpr int kDeptButton = 1;
constexpr int kDeptButtons = 4;
constexpr int kRoomButton = 16;
constexpr int kCloseButton = 27;
constexpr int kEntries = 10;

// The first department the requester lists (diagnosis).
constexpr int kFirstDepartment = 3;

// Its layout, relative to the window (HiresCoords[0x16] to [0x18], [0x26]
// on and [0x2a] on, HiresBoxes[1] on; src/coords.cpp): the instructions, the
// price, the room's picture, department row r kDeptStep * r lower than the
// first, room entry i kEntryStep * i lower than the first.
constexpr Point kHelpText = {165, 19};
constexpr Point kCostText = {165, 233};
constexpr Point kPreview = {76, 221};
constexpr Point kDeptText = {21, 22};
constexpr int kDeptStep = 32;
constexpr Point kRoomText = {165, 41};
constexpr Rect kRoomBox = {160, 36, 125, 18};
constexpr int kEntryStep = 19;

// req1_help strings: choose a department, choose a room, the price prefix.
constexpr int kChooseDeptText = 1;
constexpr int kChooseRoomText = 2;
constexpr int kCostPrefixText = 4;

// objects[].preview_anim (src/tables.cpp) of the rooms' main objects: the
// anim start of the picture the requester shows.
constexpr std::array<uint16_t, kObjectKinds> kPreviewAnims = {
    0,    450,  2527, 52,   451,  452,  2528, 453,  455,  454,  2529, 2530, 0,    459,  460,  461,
    463,  2531, 2532, 2533, 458,  2526, 2534, 2535, 2536, 2537, 466,  2538, 984,  2539, 2540, 1155,
    2541, 1177, 1179, 463,  2542, 2543, 0,    2544, 2545, 2546, 2547, 456,  457,  467,  465,  464,
    1275, 455,  2548, 2549, 998,  1003, 2550, 2551, 2552, 2553, 158,  154,  2554, 2555, 0,
};

// Blocks draw_blueprint_area draws: a floor cell the room may not cover,
// and the blueprint's floor (blocks 36-39 mark an even-sized one's centre).
constexpr uint8_t kInvalidFloor = 66;
constexpr uint8_t kBlueprintFloor = 0x17;

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

// What a room's blueprint costs before it may be started: the room's price
// and its compulsory objects' (req10).
int64_t room_price(const Game &game, int room_type)
{
    const RoomTypeInfo &type = kRoomTypeInfo[size_t(room_type) % kRoomTypes];
    int64_t total = game.settings().rooms[size_t(room_type) % kRoomTypes].cost;
    for (int k = 0; k < type.compulsory_count; k++)
        total += object_cost(game.world, type.objects[size_t(k)]);
    return total;
}

AnimStart anim_start(const GameData &data, uint32_t anim)
{
    const auto &starts = data.anims().starts;
    return anim < starts.size() ? starts[anim] : AnimStart{};
}

Sprite block(const GameData &data, uint8_t id)
{
    int index = block_sprite(id);
    return index < 0 ? Sprite{} : data.blocks().at(size_t(index));
}

// draw_a_one_colour_frame: the elements of composite frame `frame_id` with
// its origin at `at` in `colour` alone.
void draw_frame_one_colour(Surface &dst, Point at, uint32_t frame_id, bool flip, uint8_t colour, const GameData &data)
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
        int ex = e.x - kElementBiasX, ey = e.y - kElementBiasY;
        unsigned f = e.flags & (kDrawFlipX | kDrawFlipY);
        if (flip) {
            f ^= kDrawFlipX;
            ex = -ex - s.width;
        }
        draw_sprite_one_colour(dst, at.x + ex, at.y + ey, s, colour, f);
    }
}

// LbDrawLine, clipped to `dst`'s clip rectangle.
void draw_line(Surface &dst, Point from, Point to, uint8_t colour, Rect clip)
{
    int x0 = from.x, y0 = from.y;
    const int x1 = to.x, y1 = to.y;
    int dx = std::abs(x1 - x0), dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        if (x0 >= clip.x && x0 < clip.x + clip.w && y0 >= clip.y && y0 < clip.y + clip.h)
            dst.row(y0)[x0] = colour;
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

} // namespace

bool build_tool_busy(const Game &game)
{
    switch (game.world.building.mouse) {
    case BuildMouse::Blueprint:
    case BuildMouse::PlaceDoor:
    case BuildMouse::WaitClear:
        return true;
    default:
        return false;
    }
}

void open_build_room(Game &game)
{
    game.world.placing.placing = false;
    game.event = {EV_OPEN_BUILD_ROOM, 0, 0, 0};
}

void sync_build_requester(Interface &ui, const Game &game)
{
    if (ui.modal())
        return;
    bool want = game.world.placing.requester == ObjectRequester::BuildRoom;
    bool open = window_open(ui, kBuildWindow);
    if (want && !open) {
        if (ui.requester() != Requester::None)
            ui.close_requester(ui.requester());
        ui.open_requester(Requester::BuildRoom);
    } else if (!want && open) {
        if (ui.requester() == Requester::BuildRoom)
            ui.close_requester(Requester::BuildRoom);
        else
            ui.close_window(kBuildWindow);
    }
}

namespace
{

// The room entry under the pointer becomes the room type the requester
// offers.
void build_requesters_hover(const Interface &ui, BuildUi &bui, ObjectPlacing &p, std::vector<uint8_t> &sounds,
                            const UiMouse &mouse)
{
    if (!window_open(ui, kBuildWindow) || p.requester_mode != 4)
        return;
    const Window &win = ui.window(kBuildWindow);
    for (int i = 0; i < kEntries; i++) {
        if (!over(mouse.x, mouse.y, win.x + kRoomBox.x, win.y + kRoomBox.y + kEntryStep * i, kRoomBox.w, kRoomBox.h))
            continue;
        if (p.room_type != p.offered[size_t(i)])
            sounds.push_back(kSfxHighlight5);
        p.room_type = p.offered[size_t(i)];
        bui.hovered_entry = i;
    }
}

void build_requesters_build_button(Game &game, int i)
{
    ObjectPlacing &p = game.world.placing;
    std::vector<uint8_t> &sounds = game.world.building.sounds;
    if (i >= kDeptButton && i < kDeptButton + kDeptButtons) {
        game.event = {EV_REQ1_RESET_ROOMS, uint16_t(kFirstDepartment + i - kDeptButton), 0, 0};
        sounds.push_back(kSfxSelectX);
    } else if (i == kCloseButton) {
        game.event = {EV_CLOSE_REQUESTERS, 0, 0, 0};
        sounds.push_back(kSfxNo4);
    } else if (p.requester_mode == 4 && i >= kRoomButton && i < kRoomButton + kEntries) {
        if (int kind = p.room_type) {
            if (room_price(game, kind) < game.balance) {
                game.event = {EV_INIT_BLUEPRINT, uint16_t(kind), 0, uint16_t(kind)};
                sounds.push_back(kSfxYesX);
            } else {
                sounds.push_back(kSfxWrong2);
            }
        }
        sounds.push_back(kSfxYesX);
    }
}

void build_requesters_edit_button(Game &game, int i)
{
    ObjectPlacing &p = game.world.placing;
    std::vector<uint8_t> &sounds = game.world.building.sounds;
    if (i == kEditRoomFurnish && !(p.arg & (1u << kEditRoomFurnish))) {
        game.event = {EV_FURNISH_EDITED_ROOM, 0, 0, 0};
        sounds.push_back(kSfxSelectX);
        return;
    }
    if (i == kEditRoomPickUp && !(p.arg & (1u << kEditRoomPickUp))) {
        game.world.building.mouse = BuildMouse::PickUpObject;
        set_room_requester_mode(game.world, 8);
        p.arg3 = 0;
        sounds.push_back(kSfxSelectX);
        return;
    }
    if (i != kEditRoomConfirm || (p.arg & (1u << kEditRoomConfirm)))
        return;
    switch (p.requester_mode) {
    case 0:
        game.event = {EV_COMMIT_BLUEPRINT, 0, 0, 0};
        game.world.building.mouse = BuildMouse::None;
        break;
    case 4:
        game.event = {EV_COMMIT_DOORS, p.room_type, 0, 0};
        sounds.push_back(kSfxYesX);
        break;
    case 6:
    case 8:
        game.event = {EV_OPEN_ROOM, 0, 0, 0};
        sounds.push_back(kSfxYesX);
        break;
    default:
        break;
    }
}

} // namespace

void build_requesters_input(Interface &ui, BuildUi &bui, Game &game, const InterfaceEvents &events,
                            const UiMouse &mouse)
{
    bui.hovered_entry = -1;
    build_requesters_hover(ui, bui, game.world.placing, game.world.building.sounds, mouse);
    if (events.triggered_window == kBuildWindow)
        build_requesters_build_button(game, events.triggered_button);
    if (events.triggered_window == kEditWindow)
        build_requesters_edit_button(game, events.triggered_button);
}

// build_mouse in the edit-room modes: MM_EDIT_ROOM hovers and picks a room or
// a corridor object, MM_PICK_UP_OBJECT lifts one of the room's objects.
void build_mouse_edit(BuildUi &bui, Game &game, int cell, uint16_t pointed, const UiMouse &mouse, BuildMouse mode)
{
    game.world.placing.mouse_cell = cell;
    if (mode == BuildMouse::PickUpObject) {
        bui.pickup_thing = compute_object_pointed(game.world, pointed, game.world.building.build.room);
        if (mouse.left_pressed && bui.pickup_thing &&
            game.world.things[bui.pickup_thing].room_id == game.world.building.build.room)
            game.event = {EV_PICK_UP_OBJECT, bui.pickup_thing, 0, 1};
        return;
    }
    compute_room_pointed(game.world, cell, pointed, bui.pickup_thing);
    // Only the player's own land may be clicked, as process_mouse tests.
    if (mouse.left_pressed && game.world.map[size_t(cell)].owner_flags == 1u << 0) {
        if (Event e = clicked_for_edit(game.world, bui.pickup_thing); e.type)
            game.event = e;
    }
}

void build_mouse(BuildUi &bui, Game &game, int cell, uint16_t pointed, const UiMouse &mouse, bool on_map)
{
    if (!on_map || cell < 0 || cell >= kMapCells) {
        bui.cell = -1;
        return;
    }
    bui.cell = cell;
    switch (game.world.building.mouse) {
    case BuildMouse::Blueprint:
        if (mouse.left_pressed)
            game.event = {EV_MODIFY_BLUEPRINT, uint16_t(cell), 0, 0};
        else if (game.event.type == EV_NONE)
            game.event = {uint8_t(mouse.left_held ? EV_MODIFY_BLUEPRINT : EV_HOVER_BLUEPRINT), uint16_t(cell), 0, 0};
        break;
    case BuildMouse::PlaceDoor:
        if (mouse.left_pressed)
            game.event = {EV_PLACE_BP_DOOR, 0, 0, 0};
        else if (mouse.right_pressed)
            game.event = {EV_PLACE_BP_DOOR, 1, 0, 0};
        else if (!mouse.left_held && !mouse.right_held)
            game.event = {EV_TEMP_PLACE_DOOR, uint16_t(bui.prev_cell >= 0 ? bui.prev_cell : cell), 0, 0};
        break;
    case BuildMouse::EditRoom:
    case BuildMouse::PickUpObject:
        build_mouse_edit(bui, game, cell, pointed, mouse, game.world.building.mouse);
        break;
    default:
        break;
    }
    bui.prev_cell = cell;
}

namespace
{

void put_text(UiLayer &layer, const Window &win, const Font &font, Point at, std::string_view s)
{
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    layer.draw_text(font, win.x + at.x, win.y + at.y, s, {{0, 0, kUiWidth}, screen});
}

// The picture of room type `croom`'s main object, fitted to 121 pixels when
// either side is larger.
void draw_build_requester_preview(UiLayer &layer, const Window &win, int croom, const GameData &data)
{
    AnimStart start = anim_start(data, kPreviewAnims[kRoomTypeInfo[size_t(croom)].main_object]);
    const AnimData &anims = data.anims();
    if (start.frame >= anims.frames.size())
        return;
    const AnimFrame &frame = anims.frames[start.frame];
    int scale_x = frame.width ? 0x790000 / frame.width : INT_MAX;
    int scale_y = frame.height ? 0x790000 / frame.height : INT_MAX;
    int scale = 0x10000;
    if (!(scale_y >= 0x10000 && scale_x >= 0x10000))
        scale = (std::min(scale_x, scale_y) >> 8) << 8;
    draw_layer_frame(layer, data, start.frame, {win.x + kPreview.x, win.y + kPreview.y}, start.flip != 0, scale);
}

// The flashing hint (req10_overdraw): the first department button (highlight
// 8) or the first room's (0xb) in its active look, hires_requester_sprites
// [0x2f] and [0x30].
void draw_build_requester_flash(UiLayer &layer, const Window &win, const SpriteSheet &sheet, TutorialFlash &flash)
{
    constexpr int kDeptSprite = 0xd3, kRoomSprite = 0xdf;
    if (!flash.tutorial || !flash.tick() || (flash.highlight != 8 && flash.highlight != 0xb))
        return;
    bool dept = flash.highlight == 8;
    const WindowButton &b = win.buttons[size_t((dept ? kDeptButton : kRoomButton) - 1)];
    layer.draw_sprite(win.x + b.x, win.y + b.y, sheet.at(size_t(dept ? kDeptSprite : kRoomSprite)));
    flash.drawn++;
}

// The room choice: the instructions, the preview, the offered rooms and the
// price.
void draw_build_requester_rooms(UiLayer &layer, const Window &win, const BuildUi &bui, const Game &game,
                                const RequesterArt &art, const GameData &data)
{
    const ObjectPlacing &p = game.world.placing;
    const LanguageText &text = data.text();
    put_text(layer, win, art.font(), kHelpText, text.at(TextSection::Req1Help, kChooseRoomText));
    int croom = p.room_type % kRoomTypes;
    if (croom)
        draw_build_requester_preview(layer, win, croom, data);
    for (int i = 0; i < kEntries; i++) {
        int room = p.offered[size_t(i)];
        const Font &font = bui.hovered_entry == i ? art.highlight_font() : art.font();
        if (room)
            put_text(layer, win, font, {kRoomText.x, kRoomText.y + kEntryStep * i},
                     text.at(TextSection::RoomNames, kRoomTypeInfo[size_t(room) % kRoomTypes].name));
    }
    put_text(layer, win, art.font(), kCostText,
             std::string(text.at(TextSection::Req1Help, kCostPrefixText)) + std::to_string(room_price(game, croom)));
}

} // namespace

void draw_build_requester(UiLayer &layer, const Interface &ui, const BuildUi &bui, const Game &game,
                          const RequesterArt &art, const GameData &data)
{
    if (!window_open(ui, kBuildWindow))
        return;
    const Window &win = ui.window(kBuildWindow);
    const ObjectPlacing &p = game.world.placing;
    const LanguageText &text = data.text();
    draw_build_requester_flash(layer, win, art.sheet(kBuildWindow), ui.flash());
    if (p.requester_mode == 1)
        put_text(layer, win, art.font(), kHelpText, text.at(TextSection::Req1Help, kChooseDeptText));
    else if (p.requester_mode == 4)
        draw_build_requester_rooms(layer, win, bui, game, art, data);
    int row = 0;
    for (size_t i = 0; i < kDepartments.size(); i++) {
        if (!kDepartments[i].listed)
            continue;
        const Font &font = p.arg == i ? art.highlight_font() : art.font();
        put_text(layer, win, font, {kDeptText.x, kDeptText.y + kDeptStep * row},
                 text.at(TextSection::DepartmentNames, int(i)));
        row++;
    }
}

namespace
{

// What draw_build_overlay's pieces draw into and with.
struct OverlayPainter {
    Surface &dst;
    Rect view;
    Point camera;
    const ObjectUi &oui;
    const GameData &data;

    // give_screen_xy_of_cell_at_column_row: the left corner's x and the
    // bottom corner's y of a cell's diamond.
    int cell_x(int col, int row) const { return (col - row - 1) * kTileWidth / 2 - camera.x + view.x; }
    int cell_y(int col, int row) const { return (col + row + 2) * kTileHeight / 2 - camera.y + view.y; }

    void draw_block(int col, int row, uint8_t id) const
    {
        Sprite s = block(data, id);
        draw_sprite(dst, cell_x(col, row), cell_y(col, row) - s.height, s);
    }

    void draw_anim(int col, int row, uint32_t anim, bool red) const
    {
        AnimStart start = anim_start(data, anim);
        int x = cell_x(col, row), y = cell_y(col, row);
        if (red)
            draw_frame_one_colour(dst, {x, y}, start.frame, start.flip != 0, oui.red, data);
        else
            draw_frame(dst, {x, y}, start.frame, start.flip != 0, data);
    }
};

// The floor block of blueprint cell (x, y).
uint8_t blueprint_floor_block(const World &world, const BuildState &b, int x, int y)
{
    int width = b.x1 - b.x0 + 1, height = b.y1 - b.y0 + 1;
    if (blueprint_floor_invalid(world, x, y))
        return kInvalidFloor;
    if (height % 2 == 0 && width % 2 == 0 && width > 3 && height > 3) {
        if (y == (height - 1) / 2 + b.y0 && x == (width - 1) / 2 + b.x0)
            return 38;
        if (y == (height - 1) / 2 + b.y0 && x == (width + 1) / 2 + b.x0)
            return 37;
        if (y == (height + 1) / 2 + b.y0 && x == (width - 1) / 2 + b.x0)
            return 36;
        if (y == (height + 1) / 2 + b.y0 && x == (width + 1) / 2 + b.x0)
            return 39;
    }
    return kBlueprintFloor;
}

// The kBlueprints outline piece drawn at corner (x, y), 0 for none.
int blueprint_outline_piece(const BuildState &b, int x, int y)
{
    if (y == b.y0)
        return x == b.x0 ? 3 : x == b.x1 + 1 ? 2 : 1;
    if (y == b.y1 + 1)
        return x == b.x1 + 1 ? 0 : 1;
    return x == b.x0 || x == b.x1 + 1 ? 2 : 0;
}

// draw_overdrawn_blueprints: the blueprint while it is laid out.
void draw_overdrawn_blueprints(const OverlayPainter &paint, const World &world, const BuildState &b)
{
    for (int y = b.y0; y <= b.y1; y++)
        for (int x = b.x0; x <= b.x1; x++)
            paint.draw_block(x, y, blueprint_floor_block(world, b, x, y));
    for (int y = b.y0; y <= b.y1 + 1; y++) {
        for (int x = b.x0; x <= b.x1 + 1; x++) {
            int piece = blueprint_outline_piece(b, x, y);
            if (piece)
                paint.draw_anim(x, y - 1, kBlueprints[size_t(piece)].anim, blueprint_outline_invalid(world, x, y));
        }
    }
}

// draw_blueprint_area in mode 2: the room being edited highlighted, its
// outline only, through the ghost table with the flag rotating each of six
// turns (and nothing on the fourth), as the game flashes it.
void draw_edit_room_outline(const OverlayPainter &paint, const BuildState &b, int turn)
{
    int phase = turn % 6;
    if (phase == 3)
        return;
    unsigned flags = phase == 1 || phase == 5   ? unsigned(kDrawGhost)
                     : phase == 2 || phase == 4 ? unsigned(kDrawGhostUnder)
                                                : 0u;
    for (int y = b.y0; y <= b.y1 + 1; y++) {
        for (int x = b.x0; x <= b.x1 + 1; x++) {
            int piece = blueprint_outline_piece(b, x, y);
            if (!piece)
                continue;
            AnimStart start = anim_start(paint.data, kBlueprints[size_t(piece)].anim);
            int px = paint.cell_x(x, y - 1), py = paint.cell_y(x, y - 1);
            draw_frame(paint.dst, {px, py}, start.frame, start.flip != 0, paint.data, FrameOptions{.flags = flags});
        }
    }
}

// draw_cell_no: the outline of cell `cell`.
void draw_cell_no(const OverlayPainter &paint, int cell)
{
    Surface &dst = paint.dst;
    const Rect view = paint.view;
    const uint8_t red = paint.oui.red;
    int col = cell % kMapSize, row = cell / kMapSize;
    int top_x = paint.cell_x(col, row) + kTileWidth / 2, top_y = paint.cell_y(col, row) - kTileHeight;
    const int hw = (kTileWidth - 1) / 2, hh = (kTileHeight - 1) / 2;
    draw_line(dst, {top_x, top_y}, {top_x + hw - 1, top_y + hh}, red, view);
    draw_line(dst, {top_x + hw, top_y + hh}, {top_x, top_y + kTileHeight - 1}, red, view);
    draw_line(dst, {top_x, top_y + kTileHeight - 1}, {top_x - hw, top_y + hh}, red, view);
    draw_line(dst, {top_x - hw, top_y + hh}, {top_x, top_y}, red, view);
}

// temp_place_door's drawing: the door or window where it would go.
void draw_temp_door(const OverlayPainter &paint, const Building &bd)
{
    const BuildState &b = bd.build;
    const DoorType &door = kDoorTypes[b.door_type % kDoorTypes.size()];
    for (int j = 0; j < door.count; j++) {
        int floor = b.door_tile + door.tiles[size_t(j)] - door.offset;
        paint.draw_block(uint16_t(floor) % kMapSize, uint16_t(floor) / kMapSize, door.floor_blocks[size_t(j)]);
        int piece = b.door_tile + door.tiles[size_t(j)];
        paint.draw_anim(uint16_t(piece) % kMapSize, uint16_t(piece) / kMapSize - 1, door.cursor_anim,
                        bd.door_piece_invalid[size_t(j)]);
    }
}

} // namespace

void draw_build_overlay(const ViewTarget &target, const Game &game, const BuildUi &bui, const ObjectUi &oui,
                        const GameData &data)
{
    const OverlayPainter paint{target.dst, target.view, target.camera, oui, data};
    const World &world = game.world;
    const Building &bd = world.building;
    target.dst.set_clip(target.view);
    if (bd.build.draw_mode == 1)
        draw_overdrawn_blueprints(paint, world, bd.build);
    else if (bd.build.draw_mode == 2)
        draw_edit_room_outline(paint, bd.build, game.turn_count);
    // The cell under the pointer while the blueprint is laid out.
    if (bd.mouse == BuildMouse::Blueprint && bui.cell >= 0)
        draw_cell_no(paint, bui.cell);
    if (bd.mouse == BuildMouse::PlaceDoor && bd.door_shown)
        draw_temp_door(paint, bd);
    target.dst.reset_clip();
}

} // namespace th
