#include "core/ui/queue_window.h"

#include "core/data/gamedata.h"
#include "core/render/frame.h"
#include "core/render/view.h"
#include "core/render/wibble.h"
#include "core/ui/buy_objects.h"
#include "core/world/build.h"
#include "core/world/complex_tables.h"
#include "core/world/game.h"
#include "core/world/queue.h"
#include "core/world/world.h"

#include <string>
#include <utility>

namespace th
{

namespace
{

constexpr int kQueueWindow = int(Requester::Queue);

// HiresCoords (src/coords.cpp; coords_QueuePanelp): the queue's labels
// (labels[8] at 0x3b8 = HiresCoords[0x77]) and the patients' area
// (queue_x/y/w at 0x48-0x50, drop_gap_divisor at 0x5a0).
constexpr Point kLabel[8] = {{24, 23}, {24, 46}, {24, 69}, {24, 94}, {142, 23}, {142, 46}, {142, 69}, {121, 94}};
constexpr Point kQueueAt = {240, 100};
constexpr int kQueueW = 266;
constexpr int kGapDivisor = 32;

// HiresBoxes (src/coords.cpp; boxes_QueueLayoutp, QueueLayout.r at 0x420 =
// HiresBoxes[0x42]): the hover area (r1), the front, back, close and door
// hotspots, the right-click bar's origin, row and reception/home entries.
constexpr Rect kHover = {222, 0, 314, 124};
constexpr Rect kFront = {0, 0, 222, 124};
constexpr Rect kBack = {536, 46, 70, 78};
constexpr Rect kCloseHot = {564, 20, 23, 22};
constexpr Rect kDoorHot1 = {170, 27, 44, 79};
constexpr Rect kDoorHot2 = {546, 53, 43, 53};
constexpr Rect kBarOrigin = {60, 124, 0, 0};
constexpr Point kBarRow = {30, 68};
constexpr Rect kBarReception = {12, 12, 81, 54};
constexpr Rect kBarHome = {95, 12, 81, 54};
constexpr Rect kBarRefer = {12, 7, 164, 32};
// r23: the hospital's name in a refer entry.
constexpr Rect kBarReferName = {69, 11, 99, 13};

// The queue window's buttons, counted from 1 in window 6
// (hires_requester_buttons[0xa] to [0xc]): capacity down, capacity up and
// close.
constexpr int kQueueDown = 68;
constexpr int kQueueUp = 69;
constexpr int kQueueClose = 70;

// hires_requester_sprites [0x28] to [0x2e]: the right-click bar.
constexpr int kBarBgSprite = 0x177;
constexpr int kBarReferSprite = 0x178;
constexpr int kBarBottomSprite = 0x179;
constexpr int kBarReceptionOnSprite = 0x17a;
constexpr int kBarHomeOnSprite = 0x17b;
constexpr int kBarReferOnSprite = 0x17c;

// The sounds req7 plays (SFX_SELECTX, SFX_WRONG2, SFX_NO4, SFX_SACK).
constexpr uint8_t kSfxSelectX = 0x43;
constexpr uint8_t kSfxWrong2 = 0x08;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxSack = 0xaa;

bool over(int mx, int my, Point at, Rect box)
{
    return mx >= at.x + box.x && mx < at.x + box.x + box.w && my >= at.y + box.y && my < at.y + box.y + box.h;
}

bool over(int mx, int my, Rect box)
{
    return mx >= box.x && mx < box.x + box.w && my >= box.y && my < box.y + box.h;
}

// The standing animation (patient_stand) facing 2 (down) of patient kind
// `subtype`, or an empty start.
AnimStart patient_stand(const GameData &data, uint8_t subtype)
{
    uint16_t anim = kPatientAnimStarts[subtype % kPatientAnimStarts.size()][9];
    if (anim == 9999)
        return {};
    const auto &starts = data.anims().starts;
    uint32_t index = uint32_t(anim) + 2;
    return index < starts.size() ? starts[index] : AnimStart{};
}

// The size of composite frame `frame_id` in `data`.
void frame_size(const GameData &data, uint16_t frame_id, int &w, int &h)
{
    const auto &frames = data.anims().frames;
    if (frame_id < frames.size()) {
        w = frames[frame_id].width;
        h = frames[frame_id].height;
    }
}

// The patient's screen x at queue position `sel` (1-based, 0 for none) of
// room `room` in the window at `at`, or 0. Does not leave the drag gap.
int queue_person_x(const World &world, uint16_t room, uint8_t sel, Point at)
{
    uint16_t size = queue_size(world, room);
    const RoomInfo &info = world.rooms[room % kRoomCount];
    uint16_t id = info.queue_head;
    if (!size || !id)
        return 0;
    int step = kQueueW / size;
    int px = at.x + kQueueAt.x;
    for (uint16_t position = 1; id; position++) {
        if (position == sel)
            return px;
        px += step;
        id = world.things[id].queue_next;
    }
    return 0;
}

// over_queue_person (0x969f4): the queue position (1-based) of the patient
// under the pointer in `in`, in the window at `at`, or 0; `panel_thing`
// becomes it. Uses the patient's current sprite and leaves the drag gap
// around `sel`.
uint8_t over_queue_person(const World &world, uint16_t room, uint8_t sel, Point at, const QueueInputs &in,
                          uint16_t &panel_thing)
{
    uint16_t size = queue_size(world, room);
    const RoomInfo &info = world.rooms[room % kRoomCount];
    uint16_t id = info.queue_head;
    if (!size || !id)
        return 0;
    int step = kQueueW / size;
    int px = at.x + kQueueAt.x;
    int py = at.y + kQueueAt.y;
    for (uint16_t position = 1; id; position++) {
        const Thing &p = world.things[id];
        if (position == sel)
            px += kUiWidth / kGapDivisor;
        int w = 16, h = 32;
        frame_size(in.data, p.sprite, w, h);
        int x0 = px - w / 2, x1 = x0 + w, y0 = py - h, y1 = y0 + h;
        if (in.mouse.x > x0 && in.mouse.x < x1 - kUiWidth / 32 && in.mouse.y > y0 && in.mouse.y < y1) {
            panel_thing = id;
            return uint8_t(position);
        }
        if (position == sel)
            px += kUiWidth / kGapDivisor;
        px += step;
        id = p.queue_next;
    }
    return 0;
}

// What display_queue_people draws a patient with: a frame at the patient's
// feet, in a scratch surface whose drawn pixels go onto the layer.
constexpr Point kScratchOrigin = {100, 150};
constexpr int kScratchSize = 200;

// Puts the pixels of a drawing onto the layer, its origin at `at`: those
// `pixel(x, y)` gives a colour for, not a negative number.
template <typename Pixel> void copy_drawn(UiLayer &layer, Point at, Pixel &&pixel)
{
    for (int y = 0; y < kScratchSize; y++) {
        for (int x = 0; x < kScratchSize; x++) {
            int lx = at.x - kScratchOrigin.x + x, ly = at.y - kScratchOrigin.y + y;
            if (pixel(x, y) < 0 || lx < 0 || lx >= layer.width() || ly < 0 || ly >= layer.height())
                continue;
            layer.pixels().row(ly)[lx] = uint8_t(pixel(x, y));
            layer.mark({lx, ly, 1, 1});
        }
    }
}

// draw_wibble_frame for a jellyitis patient standing at `at`: draw_wibble
// leaves out the pixels of colour kMaskColour, as the game does.
void draw_queue_wibble(UiLayer &layer, const GameData &data, const Thing &p, AnimStart start, Point at, uint32_t turn)
{
    Surface scratch;
    scratch.resize(kScratchSize, kScratchSize);
    scratch.fill(kMaskColour);
    Surface buffer = make_wibble_buffer();
    draw_wibble_frame(scratch, buffer, {kScratchOrigin, start.frame, start.flip != 0, turn}, data,
                      {.layers = p.layers.data()});
    copy_drawn(layer, at, [&](int x, int y) { return scratch.row(y)[x] == kMaskColour ? -1 : scratch.row(y)[x]; });
}

// draw_glow_frame for a patient with radiation standing at `at`. The game
// draws every pixel of the frame, and a glow can blend to colour 0xfe, so the
// frame is drawn twice, on 0xfe and on another colour (the blend does not read
// what is under it): a pixel is drawn where either differs from its base.
void draw_queue_glow(UiLayer &layer, const GameData &data, const Thing &p, AnimStart start, Point at, uint32_t turn)
{
    constexpr uint8_t kOtherBase = kMaskColour - 1;
    FrameOptions options = {.layers = p.layers.data(), .glow = glow_colour(turn, p.layers.data())};
    if (p.icon_id > 0)
        options.icon = p.icon_frame;
    Surface on_mask, on_other;
    for (auto [surface, base] : {std::pair{&on_mask, kMaskColour}, std::pair{&on_other, kOtherBase}}) {
        surface->resize(kScratchSize, kScratchSize);
        surface->fill(base);
        draw_frame(*surface, kScratchOrigin, start.frame, start.flip != 0, data, options);
    }
    copy_drawn(layer, at, [&](int x, int y) {
        const int a = on_mask.row(y)[x], b = on_other.row(y)[x];
        return a != kMaskColour ? a : b != kOtherBase ? b : -1;
    });
}

// draw_queue_patient: patient `id` standing at (px, py) (its feet).
void draw_queue_patient(UiLayer &layer, const World &world, const GameData &data, uint16_t id, int px, int py)
{
    const Thing &p = world.things[id];
    AnimStart start = patient_stand(data, p.subtype);
    if (start.frame)
        draw_layer_frame(layer, data, start.frame, {px, py}, start.flip != 0, 0x10000);
}

// display_queue_people (0x96c34): room `room`'s queue in the window at `at`,
// a gap around position `sel`.
void draw_queue_people(UiLayer &layer, const World &world, const GameData &data, const QueueWindow &q, Point at,
                       uint32_t turn)
{
    uint16_t size = queue_size(world, q.room);
    const RoomInfo &info = world.rooms[q.room % kRoomCount];
    uint16_t id = info.queue_head;
    if (!size || !id)
        return;
    int step = kQueueW / size;
    int px = at.x + kQueueAt.x;
    int py = at.y + kQueueAt.y;
    for (uint16_t position = 1; id; position++) {
        const Thing &p = world.things[id];
        if (position == q.pos)
            px += kUiWidth / kGapDivisor;
        draw_queue_person(layer, world, data, id, {px, py}, turn);
        if (position == q.pos)
            px += kUiWidth / kGapDivisor;
        px += step;
        id = p.queue_next;
    }
}

// display_queue_bar (0x971f8): the right-click menu for the patient at
// (qx, at.y): reception, home and one entry per other player.
void draw_queue_bar(UiLayer &layer, const QueueWindow &q, const RequesterArt &art, const World &world, Point at, int qx)
{
    const SpriteSheet &sheet = art.sheet(kQueueWindow);
    int bx = qx - kBarOrigin.x, by = at.y + kBarOrigin.y;
    layer.draw_sprite(bx, by, sheet.at(kBarBgSprite));
    int mx = q.mouse_x, my = q.mouse_y;
    if (over(mx, my, {bx + kBarReception.x, by + kBarReception.y, kBarReception.w, kBarReception.h})) {
        layer.draw_sprite(bx + kBarReception.x, by + kBarReception.y, sheet.at(kBarReceptionOnSprite));
    } else if (over(mx, my, {bx + kBarHome.x, by + kBarHome.y, kBarHome.w, kBarHome.h})) {
        layer.draw_sprite(bx + kBarHome.x, by + kBarHome.y, sheet.at(kBarHomeOnSprite));
    }
    // The other players' refer entries: a computer player's hospital each.
    int off = 0;
    int refer_h = sheet.at(kBarReferSprite).height;
    for (int i = 1; i < 4; i++) {
        layer.draw_sprite(bx, by + off + kBarRow.y, sheet.at(kBarReferSprite));
        if (over(mx, my, {bx + kBarRefer.x, by + off + kBarRow.y, kBarRefer.w, kBarRefer.h}))
            layer.draw_sprite(bx + kBarRefer.x, by + off + kBarRow.y, sheet.at(kBarReferOnSprite));
        const Rect name = {bx + kBarReferName.x, by + off + kBarRow.y + kBarReferName.y, kBarReferName.w,
                           kBarReferName.h};
        layer.draw_text(art.font(), 0, 0, world.rivals[size_t(i)].name.data(),
                        {{name.x, name.y, name.w}, name, kTextCentre});
        off += refer_h;
    }
    // The bar's bottom, one row less its own lift above the last refer entry.
    off -= kBarRefer.y;
    layer.draw_sprite(bx, by + off + kBarRow.y, sheet.at(kBarBottomSprite));
}

// req7 (0x957f0): the capacity and close buttons.
void queue_window_buttons(QueueWindow &q, Game &game, const InterfaceEvents &events)
{
    if (events.triggered_window != kQueueWindow)
        return;
    const RoomInfo &room = game.world.rooms[q.room % kRoomCount];
    switch (events.triggered_button) {
    case kQueueDown:
        game.event = {EV_QUEUE_CAPACITY_DOWN, q.room, 0, 0};
        game.world.building.sounds.push_back(room.queue_capacity ? kSfxSelectX : kSfxWrong2);
        break;
    case kQueueUp:
        game.event = {EV_QUEUE_CAPACITY_UP, q.room, 0, 0};
        game.world.building.sounds.push_back(room.queue_capacity < 30 ? kSfxSelectX : kSfxWrong2);
        break;
    case kQueueClose:
        game.event = {EV_CLOSE_QUEUE_WINDOW, 0, 0, 0};
        game.world.building.sounds.push_back(kSfxNo4);
        break;
    default:
        break;
    }
}

// req7_overdraw on the left button's release (queue_drag 1): the close
// hotspot, the front or back of the queue, or (off the hover area) another
// room's queue.
Event queue_window_release(const QueueWindow &q, World &world, const UiMouse &mouse, Point at)
{
    if (over(mouse.x, mouse.y, at, kCloseHot)) {
        world.building.sounds.push_back(kSfxNo4);
        return {EV_CLOSE_QUEUE_WINDOW, 0, 0, 0};
    }
    if (!q.pos)
        return {};
    if (over(mouse.x, mouse.y, at, kFront)) {
        world.building.sounds.push_back(kSfxSelectX);
        return {EV_FRONT_QUEUE, q.room, 0, q.pos};
    }
    if (over(mouse.x, mouse.y, at, kBack)) {
        world.building.sounds.push_back(kSfxSelectX);
        return {EV_BACK_QUEUE, q.room, 0, q.pos};
    }
    if (over(mouse.x, mouse.y, at, kHover))
        return {};
    if (uint16_t dest = world.building.build.edit_room) {
        world.building.sounds.push_back(kSfxSelectX);
        return {EV_QUEUE_PERSON_TO_NEW_QUEUE, dest, 0, q.selected};
    }
    return {};
}

// req7_overdraw on the right button's release (queue_drag 2): reception,
// home or, for a single player, home from a refer row too.
Event queue_window_bar_click(const QueueWindow &q, World &world, const RequesterArt &art, const UiMouse &mouse,
                             Point at)
{
    if (!q.pos)
        return {};
    int qx = queue_person_x(world, q.room, q.pos, at);
    if (!qx)
        qx = at.x + kQueueAt.x;
    int bx = qx - kBarOrigin.x, by = at.y + kBarOrigin.y;
    if (over(mouse.x, mouse.y, {bx + kBarReception.x, by + kBarReception.y, kBarReception.w, kBarReception.h})) {
        world.building.sounds.push_back(kSfxSelectX);
        return {EV_QUEUE_PERSON_GO_RECEPTION, q.room, 0, q.pos};
    }
    if (over(mouse.x, mouse.y, {bx + kBarHome.x, by + kBarHome.y, kBarHome.w, kBarHome.h})) {
        world.building.sounds.push_back(kSfxSack);
        return {EV_QUEUE_PERSON_GO_HOME, q.room, 0, q.pos};
    }
    int off = 0;
    int refer_h = art.sheet(kQueueWindow).at(kBarReferSprite).height;
    for (int i = 1; i < 4; i++) {
        if (over(mouse.x, mouse.y, {bx + kBarRefer.x, by + off + kBarRow.y, kBarRefer.w, kBarRefer.h})) {
            world.building.sounds.push_back(kSfxSack);
            return {EV_QUEUE_PERSON_GO_HOME, q.room, 0, q.pos};
        }
        off += refer_h;
    }
    return {};
}

// queue_window_input's drag: while a patient is lifted off the window the
// mouse mode follows it and the room under the pointer is highlighted.
void queue_window_input_drag(QueueWindow &q, Game &game, const QueueInputs &in, bool right)
{
    if (!q.drag || !q.pos || right)
        return;
    World &world = game.world;
    if (in.events.mouse_on_window) {
        world.building.mouse = BuildMouse::None;
        return;
    }
    world.building.mouse = BuildMouse::MoveQueuePerson;
    q.selected = get_real_person_from_room(world, q.room, q.pos);
    uint16_t scratch = 0;
    uint16_t under = in.map_cell >= 0 ? room_at(world, uint16_t(in.map_cell)) : 0;
    if (under && world.rooms[under % kRoomCount].type == world.rooms[q.room % kRoomCount].type)
        compute_room_pointed(world, in.map_cell, 0, scratch);
}

// queue_window_input_hover's patient under the pointer (over_queue_person).
void queue_window_input_hover(QueueWindow &q, const World &world, const QueueInputs &in, Point at)
{
    if (!over(in.mouse.x, in.mouse.y, at, kHover)) {
        q.pos = 0;
        return;
    }
    uint8_t i = over_queue_person(world, q.room, q.pos, at, in, q.panel_thing);
    if (i)
        q.pos = i;
    if (q.panel_thing)
        q.panel_info = 2;
}

// queue_window_input's kept or dropped patient and drag state (queue_drag).
void queue_window_input_state(QueueWindow &q, const World &world, bool left, bool right)
{
    if (!get_real_person_from_room(world, q.room, q.pos) && q.drag != 0) {
        q.pos = 0;
        q.hilight = 0;
    } else {
        q.hilight = get_real_person_from_room(world, q.room, q.pos);
    }
    if (left)
        q.drag = 1;
    else if (right)
        q.drag = q.pos ? 2 : 0;
}

// The tooltip while a bar is held (req7_overdraw's compute_help_text 0x12).
void queue_window_input_help(QueueWindow &q, const World &world, Point at, bool right)
{
    if (q.drag != 2 || !q.pos || !right)
        return;
    int qx = queue_person_x(world, q.room, q.pos, at);
    if (!qx)
        return;
    q.help_x = qx - kBarOrigin.x;
    q.help_y = at.y + kBarOrigin.y;
    q.help_group = 0x12;
}

// queue_window_input with no button down: the release, the bar or the hover.
void queue_window_input_complete(QueueWindow &q, Game &game, const QueueInputs &in, Point at)
{
    World &world = game.world;
    Event e;
    if (q.drag == 1)
        e = queue_window_release(q, world, in.mouse, at);
    else if (q.drag == 2)
        e = queue_window_bar_click(q, world, in.art, in.mouse, at);
    else
        queue_window_input_hover(q, world, in, at);
    q.drag = 0;
    world.building.mouse = BuildMouse::None;
    if (e.type)
        game.event = e;
}

// queue_window_input's left click on a room's door hotspots (r25/r26), for
// rooms of type > 6: scroll the view to the door.
void queue_window_input_door(QueueWindow &q, const RoomInfo &room, const QueueInputs &in, Point at, bool left)
{
    if (!left || q.left_was_down || room.type <= 6)
        return;
    if (over(in.mouse.x, in.mouse.y, at, kDoorHot1) || over(in.mouse.x, in.mouse.y, at, kDoorHot2))
        q.goto_cell = int(room.door_tile);
}

} // namespace

void draw_queue_person(UiLayer &layer, const World &world, const GameData &data, uint16_t id, Point at, uint32_t turn)
{
    constexpr uint8_t kIllSeriousRadiation = 6, kIllJellyitis = 12;
    const Thing &p = world.things[id];
    AnimStart start = patient_stand(data, p.subtype);
    if (p.illness == kIllSeriousRadiation)
        draw_queue_glow(layer, data, p, start, at, turn);
    else if (p.illness == kIllJellyitis)
        draw_queue_wibble(layer, data, p, start, at, turn);
    else
        draw_queue_patient(layer, world, data, id, at.x, at.y);
}

void queue_window_event(QueueWindow &q, Interface &ui, const World &world, const Event &event)
{
    switch (event.type) {
    case EV_OPEN_QUEUE_WINDOW:
        if (ui.requester() == Requester::None && event.value > 0 && event.value < world.rooms.size()) {
            q.room = event.value;
            q.cell = event.arg;
            q.pos = 0;
            q.drag = 0;
            q.hilight = 0;
            close_irequesters(ui);
            ui.open_requester(Requester::Queue);
        }
        break;
    case EV_CLOSE_QUEUE_WINDOW:
        if (ui.requester() == Requester::Queue) {
            q.pos = 0;
            q.hilight = 0;
            q.drag = 0;
            q.panel_thing = 0;
            q.panel_info = 0;
            ui.close_requester(Requester::Queue);
        }
        break;
    default:
        break;
    }
}

void queue_window_input(QueueWindow &q, Interface &ui, Game &game, const QueueInputs &in)
{
    if (!ui.is_open(kQueueWindow) || q.room >= game.world.rooms.size())
        return;
    World &world = game.world;
    const Window &win = ui.window(kQueueWindow);
    const RoomInfo &room = world.rooms[q.room % kRoomCount];
    const Point at = {win.x, win.y};
    const UiMouse &mouse = in.mouse;
    q.mouse_x = mouse.x;
    q.mouse_y = mouse.y;
    q.panel_thing = 0;
    q.panel_info = 0;
    q.goto_cell = -1;
    q.help_group = 0;
    queue_window_buttons(q, game, in.events);

    const bool left = mouse.left_held || mouse.left_pressed;
    const bool right = mouse.right_held || mouse.right_pressed;
    queue_window_input_state(q, world, left, right);
    queue_window_input_drag(q, game, in, right);
    queue_window_input_help(q, world, at, right);
    if (left + right == 0)
        queue_window_input_complete(q, game, in, at);
    else
        queue_window_input_door(q, room, in, at, left);
    q.left_was_down = left;
}

void draw_queue_window(UiLayer &layer, const QueueWindow &q, const Interface &ui, const Game &game,
                       const RequesterArt &art, const GameData &data)
{
    const World &world = game.world;
    if (!ui.is_open(kQueueWindow) || q.room >= world.rooms.size())
        return;
    const RoomInfo &room = world.rooms[q.room % kRoomCount];
    const Window &win = ui.window(kQueueWindow);
    const Point at = {win.x, win.y};
    const Font &font = art.font();
    const TextWindow whole = {0, 0, kUiWidth};
    const Rect screen = {0, 0, kUiWidth, kUiHeight};
    const LanguageText &text = data.text();
    auto put = [&](Point p, std::string_view s) { layer.draw_text(font, at.x + p.x, at.y + p.y, s, {whole, screen}); };
    for (int i = 0; i < 4; i++)
        put(kLabel[i], text.at(TextSection::Queue, i));
    put(kLabel[4], std::to_string(queue_size(world, q.room)));
    put(kLabel[5], std::to_string(room.queue_incoming));
    put(kLabel[6], std::to_string(room.patients_seen));
    put(kLabel[7], std::to_string(room.queue_capacity));

    draw_queue_people(layer, world, data, q, at, game.turn_count);
    if (q.drag == 1 && q.pos)
        draw_queue_patient(layer, world, data, get_real_person_from_room(world, q.room, q.pos), q.mouse_x, q.mouse_y);
    if (q.drag == 2 && q.pos) {
        int qx = queue_person_x(world, q.room, q.pos, at);
        if (qx)
            draw_queue_bar(layer, q, art, world, at, qx);
    }
}

} // namespace th
