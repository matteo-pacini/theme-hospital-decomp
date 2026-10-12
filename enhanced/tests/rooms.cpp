#include "common.h"
#include "rooms_internal.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/layer.h"
#include "core/ui/town_map.h"
#include "core/ui/windows.h"
#include "core/world/advisor.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/disaster.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/objects.h"
#include "core/world/quake.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{

constexpr int kDesk = 11, kBench = 4, kPlantKind = 45;

// Level 1 started with the buy objects requester's interface.
struct FurnishFixture {
    th::GameData gd;
    th::RequesterArt art;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<th::Game> game = std::make_unique<th::Game>();
    th::Interface ui;
    th::ObjectUi oui;

    bool start(const char *dir)
    {
        std::string error;
        CHECK(gd.load(dir, &error) && art.load(dir, &error));
        level = gd.load_level(1, &error);
        CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
        if (!level)
            return false;
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        game->skipped_calls = th::kSkipComputerAction;
        th::init_object_ui(oui, gd.palette());
        return true;
    }

    // A press then a release at (x, y) of the buy objects requester, then
    // the turn's commands.
    void click(int x, int y, bool right)
    {
        const th::Window &win = ui.window(int(th::Requester::BuyObjects));
        th::UiMouse m;
        m.x = win.x + x;
        m.y = win.y + y;
        (right ? m.right_pressed : m.left_pressed) = true;
        (right ? m.right_held : m.left_held) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, {});
        m = {};
        m.x = win.x + x;
        m.y = win.y + y;
        (right ? m.right_released : m.left_released) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, {});
        game->process_turn();
        th::sync_object_requesters(ui, *game);
    }
};

// Furnishing the corridor opens the buy objects requester on the kinds
// level 1 offers; its plus, minus and entries buy and sell, and a desk too
// dear is refused. Returns the balance once the requester is open.
int check_buying(FurnishFixture &t)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = game.world.placing;
    const int desk_cost = t.settings.objects[kDesk].start_cost;
    th::furnish_corridor(game);
    game.process_turn();
    CHECK(p.requester == th::ObjectRequester::BuyObjects && p.menu_disabled && p.arg == 1);
    std::vector<int> offered;
    for (uint8_t kind : p.offered)
        if (kind)
            offered.push_back(kind);
    // Reception desk, bench, drinks machine, fire extinguisher, radiator and
    // plant; level 1 makes no litter bombs available.
    CHECK((offered == std::vector<int>{11, 4, 7, 43, 44, 45}));
    CHECK(p.item == kDesk);
    th::sync_object_requesters(t.ui, game);
    CHECK(t.ui.requester() == th::Requester::BuyObjects && t.ui.window_count() == 1);

    int balance = game.balance;
    t.click(331 + 8, 37 + 8, false); // the desk's plus
    CHECK(p.buy_count[kDesk] == 1 && game.balance == balance);
    t.click(200, 55 + 9, true); // sell a bench: none bought
    CHECK(p.buy_count[kBench] == 0 && p.item == kBench);
    t.click(200, 55 + 9, false); // the bench's entry: buys
    CHECK(p.buy_count[kBench] == 1);
    t.click(287 + 8, 56 + 8, false); // its minus
    CHECK(p.buy_count[kBench] == 0);
    game.balance = desk_cost + desk_cost / 2;
    t.click(331 + 8, 37 + 8, false); // a second desk: too dear
    CHECK(p.buy_count[kDesk] == 1);
    game.balance = balance;
    return balance;
}

// The requester's own drawing: text and the selected kind's picture.
void check_buy_objects_drawn(FurnishFixture &t)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, t.ui, t.art);
    std::vector<uint32_t> plain = layer.to_argb(t.gd.palette());
    th::draw_object_requesters(layer, t.ui, t.oui, *t.game, t.art, t.gd);
    std::vector<uint32_t> drawn = layer.to_argb(t.gd.palette());
    int changed = 0;
    for (size_t i = 0; i < drawn.size(); i++)
        changed += drawn[i] != plain[i];
    CHECK(changed > 1000);
}

// The first cell the object being placed may go on, -1 if none.
int first_placement_tile(th::World &world, th::ObjectPlacing &p)
{
    int tile = -1;
    for (int t = 0; t < th::kMapCells && tile < 0; t++) {
        th::update_placement(world, p, t);
        if (p.ok && p.tile == t)
            tile = t;
    }
    return tile;
}

// The desk following the pointer on `tile`, in colour and, where it may not
// go, in greys.
void check_desk_at_pointer(FurnishFixture &t, int tile)
{
    th::ObjectPlacing &p = t.game->world.placing;
    th::UiMouse m;
    th::placement_mouse(t.oui, *t.game, tile, m, true);
    CHECK(t.oui.cursor_shown && p.ok);
    th::Surface view, base;
    view.resize(320, 240);
    view.fill(0);
    base = view;
    th::Point w = th::cell_to_world(tile % th::kMapSize, tile / th::kMapSize);
    th::Point camera = {w.x - 160, w.y - 120};
    th::draw_placement(view, {0, 0, 320, 240}, camera, t.game->world, t.oui, t.gd);
    int coloured = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            coloured += view.row(y)[x] != 0;
    CHECK(coloured > 200);
    th::Surface colour_view = view;
    view = base;
    p.ok = false;
    th::draw_placement(view, {0, 0, 320, 240}, camera, t.game->world, t.oui, t.gd);
    int greyed = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            greyed += colour_view.row(y)[x] && view.row(y)[x] == t.oui.grey[colour_view.row(y)[x]];
    CHECK(greyed > coloured * 8 / 10);
    p.redraw = true;
}

// A left click on `tile` places the desk: paid for from `balance`, on the
// map and in the desks' list. Returns whether it was placed.
bool check_desk_placed(FurnishFixture &t, int tile, int balance)
{
    th::Game &game = *t.game;
    th::World &world = game.world;
    const th::ObjectPlacing &p = world.placing;
    th::UiMouse m;
    m.left_pressed = m.left_held = true;
    th::placement_mouse(t.oui, game, tile, m, true);
    CHECK(game.event.type == th::EV_PLACE_OBJECT && game.event.value == tile);
    game.process_turn();
    th::sync_object_requesters(t.ui, game);
    CHECK(game.balance == balance - t.settings.objects[kDesk].start_cost);
    uint16_t id = world.first_of_kind[kDesk];
    CHECK(id != 0 && world.map[size_t(tile)].first == int16_t(id));
    if (!id)
        return false;
    const th::Thing &placed = world.things[id];
    CHECK(placed.type == th::kThingObject && placed.subtype == kDesk && placed.owner == 0);
    CHECK(placed.x.tile == tile % th::kMapSize && placed.y.tile == tile / th::kMapSize);
    CHECK(world.map[size_t(tile)].flags & 3);
    CHECK((world.map[placed.use_pos[0]].flags & 4) && (world.map[placed.use_pos[1]].flags & 4));
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[kDesk] == 0);
    CHECK(t.ui.window_count() == 0 && t.ui.requester() == th::Requester::None);
    return true;
}

// A second desk may not go on the first, on `tile`; Escape drops it
// unbought, leaving the balance at `balance`.
void check_second_desk_dropped(FurnishFixture &t, int tile, int balance)
{
    th::Game &game = *t.game;
    th::World &world = game.world;
    th::ObjectPlacing &p = world.placing;
    game.event = {th::EV_BUY_OBJECT, uint16_t(kDesk), 0, 1};
    game.process_turn();
    game.event = {th::EV_PLACE_NEXT_OBJECT, 0, 0, 0};
    game.process_turn();
    CHECK(p.placing && p.kind == kDesk);
    th::update_placement(world, p, tile);
    CHECK(!p.ok);
    // The cursor marks the cell the placed desk covers as occupied (0x80), and
    // the cells of an empty stretch of floor as free (0x40).
    int occupied = 0;
    for (const th::CellMark &m : th::placement_marks(world, p))
        occupied += m.cell == tile && m.flags == 0x80;
    CHECK(occupied > 0);
    p.tile = uint16_t(tile + 3 * th::kMapSize);
    std::vector<th::CellMark> free_marks = th::placement_marks(world, p);
    CHECK(!free_marks.empty() &&
          std::all_of(free_marks.begin(), free_marks.end(), [](const th::CellMark &m) { return m.flags == 0x40; }));
    p.tile = uint16_t(tile);
    CHECK(th::object_escape(game));
    game.process_turn();
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[kDesk] == 0);
    CHECK(game.balance == balance && world.things[world.first_of_kind[kDesk]].type_obj_next == 0);
}

// A plant being placed is drawn at its health when it was picked up
// (get_frame_for_placing_plant): a withered one differs from a healthy one.
void check_plant_placing_frame(FurnishFixture &t, int tile)
{
    th::World &world = t.game->world;
    th::ObjectPlacing &p = world.placing;
    const uint16_t id = 7;
    th::Thing &plant = world.things[id];
    const th::Thing saved = plant;
    plant.type = th::kThingObject;
    plant.subtype = kPlantKind;
    plant.owner = 0;
    p.placing = true;
    p.kind = kPlantKind;
    p.tile = uint16_t(tile);
    p.ok = true;
    t.oui.cursor_shown = true;
    world.building.build.objects[0] = id;
    th::Point w = th::cell_to_world(tile % th::kMapSize, tile / th::kMapSize);
    th::Point camera = {w.x - 160, w.y - 120};
    th::Surface healthy, withered;
    healthy.resize(320, 240);
    healthy.fill(0);
    withered = healthy;
    plant.plant_health = 400;
    th::draw_placement(healthy, {0, 0, 320, 240}, camera, world, t.oui, t.gd);
    plant.plant_health = 0;
    th::draw_placement(withered, {0, 0, 320, 240}, camera, world, t.oui, t.gd);
    int differing = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            differing += healthy.row(y)[x] != withered.row(y)[x];
    CHECK(differing > 0);
    world.building.build.objects[0] = 0;
    plant = saved;
    p.placing = false;
    t.oui.cursor_shown = false;
}

} // namespace

// Furnishing level 1's corridor: the buy objects requester, buying and
// selling, and placing a reception desk through the commands.
void test_furnish_corridor(const char *dir)
{
    FurnishFixture t;
    if (!t.start(dir))
        return;
    th::Game &game = *t.game;
    th::World &world = game.world;
    th::ObjectPlacing &p = world.placing;
    const int desk_cost = t.settings.objects[kDesk].start_cost;
    CHECK(desk_cost > 0);
    const int balance = check_buying(t);
    check_buy_objects_drawn(t);

    t.click(156 + 50, 240 + 10, false); // accept
    CHECK(p.placing && p.kind == kDesk && p.mode == 2 && !p.menu_disabled);
    CHECK(p.requester == th::ObjectRequester::EditRoom && t.ui.requester() == th::Requester::EditRoom);

    const int tile = first_placement_tile(world, p);
    CHECK(tile >= 0);
    if (tile < 0)
        return;
    th::update_placement(world, p, 0); // public land, off the hospital
    CHECK(!p.ok);
    game.event = {th::EV_PLACE_OBJECT, 0, 0, uint16_t(kDesk | p.dir << 8)};
    game.process_turn();
    CHECK(world.first_of_kind[kDesk] == 0 && game.balance == balance && p.buy_count[kDesk] == 1);

    check_desk_at_pointer(t, tile);
    if (!check_desk_placed(t, tile, balance))
        return;
    check_second_desk_dropped(t, tile, balance - desk_cost);
    check_plant_placing_frame(t, tile);
}

// Every room's queue is well linked, its length is its patients' count, and
// every patient in a queue is in its room's list.
bool queues_consistent(const th::World &w)
{
    int queued = 0;
    for (uint16_t r = 1; r < th::kRoomCount; r++) {
        int count = 0;
        uint16_t prev = 0;
        for (uint16_t id = w.rooms[r].queue_head; id; id = w.things[id].queue_next) {
            const th::Thing &p = w.things[id];
            if (p.type != th::kThingPatient || p.queue_room != r || p.queue_prev != prev || count > th::kThingCount)
                return false;
            prev = id;
            count++;
        }
        if (count != w.rooms[r].queue_length)
            return false;
        queued += count;
    }
    for (const th::Thing &t : w.things)
        if (t.type == th::kThingPatient && t.queue_room)
            queued--;
    return queued == 0;
}

namespace
{

// The patients of a room's queue, head first.
std::vector<uint16_t> queue_of(const th::World &w, uint16_t room)
{
    std::vector<uint16_t> ids;
    for (uint16_t id = w.rooms[room].queue_head; id; id = w.things[id].queue_next)
        ids.push_back(id);
    return ids;
}

} // namespace

// Level 1 starts with hospital 0's four entrances in rooms 1-4, as load_level
// makes them, every hospital's entrances those rooms, and room 5 free for the
// first room built; get_room_info finds the first entrance.
void test_entrance_rooms_level1(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    const th::World &w = f.game->world;
    for (uint16_t r = 1; r <= 4; r++) {
        const th::RoomInfo &room = w.rooms[r];
        uint16_t cell = f.level->entrances[0][r - 1];
        CHECK(room.type == th::kRoomEntrance && room.owner == 0 && room.state == 0);
        CHECK(room.width == 0 && room.height == 0);
        CHECK(room.pos == cell && room.door_tile == cell && room.door_inside_tile == cell);
        CHECK(room.queue_capacity == 0 && room.queue_head == 0 && room.first_object == 0);
    }
    CHECK(w.rooms[0].type == th::kRoomFree && w.rooms[5].type == th::kRoomFree);
    for (const auto &entrances : w.entrances)
        CHECK((entrances == std::array<uint16_t, 4>{1, 2, 3, 4}));
    CHECK(th::get_room_info(w, th::kRoomEntrance, 0x65, 0) == 1);
    CHECK(th::get_room_info(w, th::kRoomEntrance, 0x65, 1) == 0);
}

namespace
{

// The first receptionist applicant, and the first cell from `start` on along
// the rows where she may be put down; 0 and -1 if none.
std::pair<uint16_t, int> receptionist_spot(const th::World &w, int start)
{
    uint16_t recep = 0;
    for (uint16_t i = 1; i < w.things.size() && !recep; i++)
        if (w.things[i].type == th::kThingStaff && w.things[i].subtype == th::kStaffReceptionist &&
            w.things[i].state == th::kStaffApplicant)
            recep = i;
    int cell = -1;
    for (int d = 0; d < 400 && cell < 0; d++)
        if (w.map[size_t(start + d)].first == 0 && th::staff_place_ok(w, start + d, th::kStaffReceptionist))
            cell = start + d;
    return {recep, cell};
}

// A manned reception desk with a bench and a drinks machine near by.
struct ReceptionDesk {
    uint16_t recep = 0, desk = 0, bench = 0, drinks = 0, reception = 0;
    int cell = -1; // where the receptionist was put down
};

// Hires a receptionist near the view start, places a desk, a bench and a
// drinks machine near her and runs turns until she mans the desk. Returns
// whether all could be placed.
bool man_reception(QueueFixture &f, ReceptionDesk &d)
{
    th::World &w = f.game->world;
    std::tie(d.recep, d.cell) = receptionist_spot(w, f.level->camera_start[0]);
    CHECK(d.recep != 0 && d.cell >= 0);
    if (!d.recep || d.cell < 0)
        return false;
    CHECK(th::place_staff_on_map(w, {f.gd.anims(), f.settings, f.game->turn_count}, d.cell, d.recep, false));
    d.desk = f.place_near(d.cell + 3, 11);
    d.bench = f.place_near(d.cell + 3 + 3 * th::kMapSize, 4);
    d.drinks = f.place_near(d.cell - 4 + 2 * th::kMapSize, 7);
    CHECK(d.desk && d.bench && d.drinks);
    if (!d.desk || !d.bench || !d.drinks)
        return false;
    const th::Thing &r = w.things[d.recep];
    int turns = 0;
    while (turns < 2000 && !(w.things[d.desk].user == d.recep && r.state == th::kStaffReceptionistWorks)) {
        f.turn();
        turns++;
    }
    CHECK(r.state == th::kStaffReceptionistWorks && r.room != 0);
    d.reception = r.room;
    CHECK(w.rooms[d.reception].type == th::kRoomReception && w.rooms[d.reception].queue_capacity == 20);
    return true;
}

// What patients did at a reception with no GP's office, each counted once.
struct NoGpCounts {
    int queued = 0, sat = 0, fed_up = 0;
    std::vector<bool> seen = std::vector<bool>(th::kThingCount);
    std::vector<bool> was_queued = std::vector<bool>(th::kThingCount);

    // Counts this turn's patients that joined `d`'s queue, sat on its bench
    // or left fed up.
    void count(const th::World &w, const ReceptionDesk &d)
    {
        for (uint16_t id = 1; id < w.things.size(); id++) {
            const th::Thing &p = w.things[id];
            if (p.type != th::kThingPatient) {
                seen[id] = was_queued[id] = false;
                continue;
            }
            if (p.queue_room == d.reception && !was_queued[id]) {
                was_queued[id] = true;
                queued++;
            }
            if (p.object == d.bench && w.things[d.bench].user == id && !seen[id]) {
                seen[id] = true;
                sat++;
            }
            if ((p.person_flags & th::kPfFedUp) && p.complex_action == th::kComplexLeave &&
                p.state == th::kPsGotoDestination && was_queued[id]) {
                was_queued[id] = false;
                fed_up++;
            }
        }
    }
};

// The drinks sold: banked at each month's end, and this month's.
int32_t drinks_sold(const th::Game &game)
{
    int32_t sold = game.world.drinks_income;
    for (size_t i = 0; i < game.finance.entry_count; i++)
        if (game.finance.entries[i].type == th::kTxIncomeDrinks)
            sold += game.finance.entries[i].amount;
    return sold;
}

// No GP's office: patients queue and wait until fed up. `consistent` is
// cleared if the queues or cell lists ever were not.
void check_reception_no_gp(QueueFixture &f, const ReceptionDesk &d, bool &consistent)
{
    th::World &w = f.game->world;
    bool called = false;
    int max_queue = 0;
    NoGpCounts n;
    for (int i = 0; i < 8000; i++) {
        f.turn();
        consistent &= queues_consistent(w) && cell_lists_consistent(w);
        called |= w.things[d.recep].slave != 0;
        max_queue = std::max(max_queue, int(w.rooms[d.reception].queue_length));
        n.count(w, d);
    }
    int32_t sold = drinks_sold(*f.game);
    std::printf("reception, no GP: %d queued, %d sat, %d fed up (%u counted), queue up to %d, drinks %d, "
                "seed %#x\n",
                n.queued, n.sat, n.fed_up, unsigned(w.patients_left_count), max_queue, int(sold), unsigned(w.seed));
    CHECK(consistent && !called);
    CHECK(n.queued > 0 && n.sat > 0 && n.fed_up > 0 && w.patients_left_count >= n.fed_up);
    CHECK(n.queued == 6 && n.sat == 5 && n.fed_up == 5 && max_queue == 3 && sold == 100 && w.seed == 0x7dfba2b5);
}

// Whether everyone queueing in `room` ahead of `called` in `before` has
// left the queue.
bool called_in_order(const th::World &w, const std::vector<uint16_t> &before, uint16_t called, uint16_t room)
{
    bool in_order = true;
    for (uint16_t id : before) {
        if (id == called)
            break;
        in_order &= w.things[id].queue_room != room;
    }
    return in_order;
}

// The patients newly on their way to room `room`, marked in `going`.
int newly_going(const th::World &w, std::vector<bool> &going, uint16_t room)
{
    int n = 0;
    for (uint16_t id = 1; id < w.things.size(); id++) {
        const th::Thing &p = w.things[id];
        if (p.type != th::kThingPatient) {
            going[id] = false;
            continue;
        }
        if (p.dest_room == room && !going[id]) {
            going[id] = true;
            n++;
        }
    }
    return n;
}

// A GP's office: she calls the patients in queue order and serves them.
// Returns whether the office could be built; `consistent` is cleared if the
// queues or cell lists ever were not.
bool check_reception_gp(QueueFixture &f, const ReceptionDesk &d, bool &consistent)
{
    th::World &w = f.game->world;
    const th::Thing &r = w.things[d.recep];
    start_rooms(f);
    uint16_t gp = lay_out_gp_office(f, d.cell - 10 * th::kMapSize);
    CHECK(gp != 0 && finish_gp_office(f, gp));
    if (!gp)
        return false;
    th::RoomInfo &office = w.rooms[gp];
    office.queue_capacity = 30;
    w.cure_available[th::kIllGeneralPractice] = 1;
    bool in_order = true;
    int calls = 0, to_office = 0;
    uint16_t slave = 0;
    uint16_t seen_before = w.rooms[d.reception].patients_seen;
    std::vector<bool> going(th::kThingCount);
    for (int i = 0; i < 3000; i++) {
        std::vector<uint16_t> before = queue_of(w, d.reception);
        f.turn();
        consistent &= queues_consistent(w) && cell_lists_consistent(w);
        if (r.slave && r.slave != slave) {
            calls++;
            // Those queueing ahead of the one called have all left the
            // queue.
            in_order &= called_in_order(w, before, r.slave, d.reception);
        }
        slave = r.slave;
        to_office += newly_going(w, going, gp);
    }
    int served = w.rooms[d.reception].patients_seen - seen_before;
    std::printf("reception, GP: %d calls, %d served, %d sent to the office, its queue %d (%d coming), seed %#x\n",
                calls, served, to_office, int(office.queue_length), int(office.queue_incoming), unsigned(w.seed));
    CHECK(consistent && in_order);
    CHECK(calls > 0 && served > 0 && to_office > 0 && office.queue_length > 0);
    CHECK(calls == 6 && served == 6 && to_office == 6 && office.queue_length == 4 && w.seed == 0xf7f5f55c);
    return true;
}

// One run of test_reception_queue_level1. Returns whether it got to the
// end.
bool reception_queue_run(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return false;
    th::World &w = f.game->world;
    CHECK(w.cure_available[0] == 1 && w.cure_available[th::kIllGeneralPractice] == 0);
    ReceptionDesk d;
    if (!man_reception(f, d))
        return false;
    bool consistent = true;
    check_reception_no_gp(f, d, consistent);
    if (!check_reception_gp(f, d, consistent))
        return false;

    // With the reception's queue full, a new patient leaves at once.
    w.rooms[d.reception].queue_capacity = w.rooms[d.reception].queue_length;
    uint16_t late =
        th::make_a_peep(w, f.game->arrivals, {f.settings, f.gd.anims(), 1}, f.level->entrances[0][0], 16, 0);
    CHECK(late != 0);
    if (!late)
        return false;
    th::start_person_going(w, f.gd.anims(), f.gd.person_complexes(), late, f.game->turn_count);
    CHECK(w.things[late].complex_action == th::kComplexLeave && w.things[late].dest_room == 0);
    return true;
}

} // namespace

// With the game data: on level 1 opened at once, a receptionist mans a
// reception desk with a bench and a drinks machine near by. With no GP's
// office the hospital cannot give general practice, so, as in the game, she
// calls no one: patients walk in to the desk, queue, sit on the bench, drink
// and, past gbv.LeaveMax impatience, go home fed up; with the queue full,
// new ones leave at once. Then, with a GP's office built and general
// practice given (as if a doctor were hired: none is), she calls them in
// queue order and serves them; those who pay are sent to the office's
// queue, where, with no doctor to answer their page, they wait.
// The queues and cell lists are consistent every turn; fixed numbers from
// the level's start (regression values, not checked against the game).
void test_reception_queue_level1(const char *dir)
{
    for (int run = 0; run < 2; run++)
        if (!reception_queue_run(dir))
            return;
}

namespace
{

// Puts the first receptionist applicant down near the view start beside a
// reception desk, and builds a GP's office giving general practice.
// Returns the receptionist, 0 if any of it could not be done.
uint16_t open_reception_and_gp(QueueFixture &f)
{
    th::World &w = f.game->world;
    auto [recep, cell] = receptionist_spot(w, f.level->camera_start[0]);
    CHECK(recep != 0 && cell >= 0);
    if (!recep || cell < 0)
        return 0;
    CHECK(th::place_staff_on_map(w, {f.gd.anims(), f.settings, f.game->turn_count}, cell, recep, false));
    CHECK(f.place_near(cell + 3, 11) != 0);
    start_rooms(f);
    uint16_t gp = lay_out_gp_office(f, cell - 10 * th::kMapSize);
    CHECK(gp != 0 && finish_gp_office(f, gp));
    if (!gp)
        return 0;
    w.rooms[gp].queue_capacity = 30;
    w.cure_available[th::kIllGeneralPractice] = 1;
    return recep;
}

// The events traced, each in its own turn, until the first patient checks
// in, at most 6000 turns. `patient` is that patient, 0 if none;
// `rand_seen` is whether a draw was traced.
std::vector<std::string> events_to_checkin(QueueFixture &f, uint16_t &patient, bool &rand_seen)
{
    th::EventLog &log = f.game->events;
    std::vector<std::string> events;
    patient = 0;
    rand_seen = false;
    for (int i = 0; i < 6000 && !patient; i++) {
        f.turn();
        for (const th::TraceEvent &e : log.events) {
            CHECK(e.turn == f.game->turn_count);
            rand_seen |= e.text.starts_with("rand ");
            events.push_back(e.text);
            if (!patient && e.text.starts_with("checkin "))
                patient = uint16_t(std::atoi(e.text.c_str() + 8));
        }
        log.events.clear();
    }
    return events;
}

// Each of `expected` starts an event of `events` after the one before.
void check_events_in_order(const std::vector<std::string> &events, const std::vector<std::string> &expected)
{
    size_t at = 0;
    for (const std::string &want : expected) {
        while (at < events.size() && !events[at].starts_with(want))
            at++;
        CHECK(at < events.size());
        if (at == events.size())
            std::printf("events: no \"%s\" in order\n", want.c_str());
    }
}

} // namespace

// The event trace (core/world/trace.h), with the game data: on level 1
// opened at once, with a manned reception desk and a GP's office giving
// general practice, a patient's events come in order as it arrives, queues,
// is called and checks in; draws are traced only when asked for, as the turn
// starts; a limited log keeps the latest events.
void test_events_level1(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    const uint16_t recep = open_reception_and_gp(f);
    if (!recep)
        return;

    th::EventLog &log = f.game->events;
    log.limit = 0;
    uint16_t patient = 0;
    bool rand_seen = false;
    const std::vector<std::string> events = events_to_checkin(f, patient, rand_seen);
    CHECK(patient != 0 && !rand_seen);
    // Each expected event of the patient's, after the one before.
    std::string id = std::to_string(patient);
    check_events_in_order(events,
                          {"peep " + id + " ", "start_going " + id, "join_queue " + id + " ",
                           "call " + std::to_string(recep) + " id=" + id + " ",
                           "checkin " + id + " receptionist=" + std::to_string(recep), "shall_pay " + id + " "});

    log.rand = true;
    f.turn();
    CHECK(log.events.size() >= 3 && log.events.front().text.starts_with("rand start_turn "));
    log.limit = 5;
    log.rand = false;
    for (int i = 0; i < 200; i++)
        f.turn();
    CHECK(log.events.size() == 5);
}

// queue.cpp's lists: joining, leaving, moving to the front and back, and a
// patient on its way counted in queue_incoming until it joins.
void test_queue_lists()
{
    auto w = std::make_unique<th::World>();
    th::RoomInfo &room = w->rooms[1];
    room.type = th::kRoomGp;
    room.queue_capacity = 6;
    for (uint16_t id = 1; id <= 4; id++)
        w->things[id].type = th::kThingPatient;
    w->things[4].dest_room = 1;
    room.queue_incoming = 1;
    for (uint16_t id = 1; id <= 4; id++)
        th::join_queue(*w, id, 1);
    CHECK((queue_of(*w, 1) == std::vector<uint16_t>{1, 2, 3, 4}) && room.queue_incoming == 0);
    CHECK(queues_consistent(*w) && th::which_queue(*w, 3) == 1 && th::get_real_person_from_room(*w, 1, 2) == 2);
    CHECK(th::spare_queue_capacity(*w, th::kIllGeneralPractice, true, 0, 0) == 2);
    CHECK(th::all_queue_capacity(*w, th::kIllGeneralPractice, true, 0) == 0);
    w->cure_available[th::kIllGeneralPractice] = 1;
    CHECK(th::all_queue_capacity(*w, th::kIllGeneralPractice, true, 0) == 2);
    th::front_queue(*w, 1, 3);
    CHECK((queue_of(*w, 1) == std::vector<uint16_t>{3, 1, 2, 4}) && queues_consistent(*w));
    th::back_queue(*w, 1, 1);
    CHECK((queue_of(*w, 1) == std::vector<uint16_t>{1, 2, 4, 3}) && queues_consistent(*w));
    th::leave_queue(*w, 2);
    CHECK((queue_of(*w, 1) == std::vector<uint16_t>{1, 4, 3}) && queues_consistent(*w));
    th::leave_queue(*w, 1);
    th::leave_queue(*w, 3);
    CHECK((queue_of(*w, 1) == std::vector<uint16_t>{4}) && queues_consistent(*w) && room.queue_length == 1);
    w->things[2].dest_room = 1;
    room.queue_incoming = 1;
    th::leave_queue(*w, 2);
    CHECK(room.queue_incoming == 0 && w->things[2].dest_room == 0);
}
