#include "common.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/render/shadows.h"
#include "core/render/surface.h"
#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/layer.h"
#include "core/ui/windows.h"
#include "core/world/advisor.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/events.h"
#include "core/world/finance.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
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
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{

constexpr int kDesk = 11, kBench = 4;

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
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, t.oui, t.gd);
    int coloured = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            coloured += view.row(y)[x] != 0;
    CHECK(coloured > 200);
    th::Surface colour_view = view;
    view = base;
    p.ok = false;
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, t.oui, t.gd);
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
    CHECK(th::object_escape(game));
    game.process_turn();
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[kDesk] == 0);
    CHECK(game.balance == balance && world.things[world.first_of_kind[kDesk]].type_obj_next == 0);
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
}

namespace
{

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
    CHECK(n.queued == 7 && n.sat == 7 && n.fed_up == 5 && max_queue == 3 && sold == 100 && w.seed == 0x8287c6fe);
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
    CHECK(calls == 8 && served == 8 && to_office == 8 && office.queue_length == 2 && w.seed == 0x570f87c1);
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

namespace
{

constexpr int kGp = th::kRoomGp, kGpDesk = 1, kCabinet = 2, kChair = 6, kPlant = 45;

// Building a GP's office on level 1 through the build requesters, the
// pointer and the build commands, step by step.
struct BuildFixture {
    th::GameData gd;
    th::RequesterArt art;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<th::Game> game = std::make_unique<th::Game>();
    th::Interface ui;
    th::BuildUi bui;
    th::ObjectUi oui;
    th::UiMouse m;
    th::InterfaceEvents ev;
    int room_cost = 0, objects_cost = 0;
    int balance = 0; // before the room was paid for
    uint16_t r = 0;  // the room being built
    int site = -1, x0 = 0, y0 = 0, x1 = 0, y1 = 0, door_cell = 0;

    bool start(const char *dir)
    {
        std::string error;
        CHECK(gd.load(dir, &error) && art.load(dir, &error));
        level = gd.load_level(1, &error);
        CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
        if (!level)
            return false;
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        th::init_object_ui(oui, gd.palette());
        room_cost = settings.rooms[kGp].cost;
        objects_cost = settings.objects[kGpDesk].start_cost + settings.objects[kCabinet].start_cost +
                       settings.objects[kChair].start_cost;
        CHECK(room_cost > 0 && objects_cost > 0);
        return true;
    }

    th::World &world() { return game->world; }
    th::ObjectPlacing &placing() { return game->world.placing; }
    th::Building &building() { return game->world.building; }
    th::BuildState &build() { return game->world.building.build; }
    const th::RoomInfo &room() { return game->world.rooms[r]; }
    const th::MapCell &cell(int x, int y) { return game->world.map[size_t(x + y * th::kMapSize)]; }

    // Drags a blueprint out with the mouse from one cell to another.
    void drag(int from_x, int from_y, int to_x, int to_y)
    {
        th::UiMouse d;
        d.left_pressed = d.left_held = true;
        th::build_mouse(bui, *game, from_x + from_y * th::kMapSize, d, true);
        run_build_command(*game);
        d = {};
        d.left_held = true;
        th::build_mouse(bui, *game, to_x + to_y * th::kMapSize, d, true);
        run_build_command(*game);
        th::build_mouse(bui, *game, to_x + to_y * th::kMapSize, {}, true);
        run_build_command(*game);
    }

    // Places each object left to place where it first may go. Returns how
    // many were placed.
    int place_all()
    {
        th::ObjectPlacing &p = placing();
        int n = 0;
        while (p.placing && n < 8) {
            int kind = p.kind, tile = -1;
            for (int t = 0; t < th::kMapCells && tile < 0; t++) {
                th::update_placement(world(), p, t);
                if (p.ok)
                    tile = p.tile;
            }
            if (tile < 0)
                break;
            game->event = {th::EV_PLACE_OBJECT, uint16_t(tile), 0, uint16_t(kind | p.dir << 8)};
            run_build_command(*game);
            n++;
        }
        return n;
    }
};

// The panel's icon opens the requester; the diagnosis department lists the
// GP's office first.
void check_build_requester_opens(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = t.placing();
    CHECK(!th::build_tool_busy(game));
    th::open_build_room(game);
    run_build_command(game);
    CHECK(p.requester == th::ObjectRequester::BuildRoom && p.requester_mode == 1 && p.menu_disabled);
    th::sync_build_requester(t.ui, game);
    CHECK(t.ui.requester() == th::Requester::BuildRoom);
    t.ev.triggered_window = int(th::Requester::BuildRoom);
    t.ev.triggered_button = 1;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, t.m);
    CHECK(game.event.type == th::EV_REQ1_RESET_ROOMS && game.event.value == 3);
    run_build_command(game);
    CHECK(p.requester_mode == 4 && p.arg == 3 && p.offered[0] == kGp);
}

// The requester's own drawing, with the pointer on the GP's office.
void check_build_requester_drawn(BuildFixture &t)
{
    const th::Window &win = t.ui.window(int(th::Requester::BuildRoom));
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, t.ui, t.art);
    std::vector<uint32_t> plain = layer.to_argb(t.gd.palette());
    t.m.x = win.x + 170;
    t.m.y = win.y + 40;
    th::build_requesters_input(t.ui, t.bui, *t.game, {}, t.m);
    CHECK(t.placing().room_type == kGp && t.bui.hovered_entry == 0);
    th::draw_build_requester(layer, t.ui, t.bui, *t.game, t.art, t.gd);
    std::vector<uint32_t> drawn = layer.to_argb(t.gd.palette());
    int changed = 0;
    for (size_t i = 0; i < drawn.size(); i++)
        changed += drawn[i] != plain[i];
    CHECK(changed > 1000);
}

// Refused while the balance does not cover the room and its objects; then
// the blueprint started, with its objects to buy.
void check_blueprint_started(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = t.placing();
    t.balance = game.balance;
    game.balance = t.room_cost + t.objects_cost;
    t.ev.triggered_button = 16;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, t.m);
    CHECK(game.event.type == th::EV_NONE);
    game.balance = t.balance;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, t.m);
    CHECK(game.event.type == th::EV_INIT_BLUEPRINT && game.event.value == kGp);
    run_build_command(game);
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.requester_mode == 0 && !p.menu_disabled);
    CHECK(p.buy_count[kGpDesk] == 1 && p.buy_count[kCabinet] == 1 && p.buy_count[kChair] == 1);
    CHECK(t.building().mouse == th::BuildMouse::Blueprint && th::build_tool_busy(game) && t.build().room != 0);
    t.r = t.build().room;
    CHECK(t.world().rooms[t.r].type == kGp && t.world().rooms[t.r].state == th::kRoomStateBlueprint);
    th::sync_build_requester(t.ui, game);
    th::sync_object_requesters(t.ui, game);
    CHECK(t.ui.requester() == th::Requester::EditRoom && t.ui.window_count() == 1);
}

// Dragging a blueprint out with the mouse, on public land: invalid.
void check_public_land_refused(BuildFixture &t)
{
    const th::BuildState &b = t.build();
    t.drag(2, 2, 6, 6);
    CHECK(b.x0 == 2 && b.y0 == 2 && b.x1 == 6 && b.y1 == 6 && b.drag == th::kDragNone && b.draw_mode == 1);
    CHECK(th::blueprint_floor_invalid(t.world(), 3, 3));
    t.game->event = {th::EV_COMMIT_BLUEPRINT};
    run_build_command(*t.game);
    CHECK(t.building().mouse == th::BuildMouse::Blueprint && t.game->advisor.forced_advice_id == 4 &&
          t.placing().requester_mode == 0);
}

// Whether a 5 x 5 blueprint at x,y with a clear ring round it may be laid
// out: `b`'s bounds are left set to the ring's.
bool clear_site(const th::World &w, th::BuildState &b, int x, int y)
{
    b.x0 = uint8_t(x - 1);
    b.y0 = uint8_t(y - 1);
    b.x1 = uint8_t(x + 5);
    b.y1 = uint8_t(y + 5);
    for (int cy = y - 1; cy <= y + 5; cy++)
        for (int cx = x - 1; cx <= x + 5; cx++)
            if (th::blueprint_floor_invalid(w, cx, cy))
                return false;
    return true;
}

// The first 5 x 5 site on the hospital's land with a clear ring round it,
// along the rows; -1 if none. `b` is as it was on return.
int first_clear_site(const th::World &w, th::BuildState &b)
{
    int site = -1;
    const th::BuildState saved = b;
    for (int y = 1; y + 6 < th::kMapSize && site < 0; y++)
        for (int x = 1; x + 6 < th::kMapSize && site < 0; x++)
            if (clear_site(w, b, x, y))
                site = x + y * th::kMapSize;
    b = saved;
    return site;
}

// Too small for a GP's office, then the bottom-right corner dragged out to
// 5 x 5; drawn over the view.
void check_blueprint_dragged(BuildFixture &t)
{
    const th::BuildState &b = t.build();
    const int x0 = t.x0, y0 = t.y0, x1 = t.x1, y1 = t.y1;
    t.drag(x0, y0, x0 + 2, y0 + 2);
    CHECK(th::blueprint_floor_invalid(t.world(), x0, y0));
    t.drag(x0 + 2, y0 + 2, x1, y1);
    CHECK(b.x0 == x0 && b.y0 == y0 && b.x1 == x1 && b.y1 == y1);
    CHECK(!th::blueprint_floor_invalid(t.world(), x0, y0) && !th::blueprint_outline_invalid(t.world(), x1 + 1, y1 + 1));

    th::Surface view;
    view.resize(640, 480);
    view.fill(0);
    th::Point centre = th::cell_to_world(x0 + 2, y0 + 2);
    th::Point camera = {centre.x - 320, centre.y - 240};
    th::build_mouse(t.bui, *t.game, x0 + 2 + (y0 + 2) * th::kMapSize, {}, true);
    run_build_command(*t.game);
    th::draw_build_overlay({view, {0, 0, 640, 480}, camera}, *t.game, t.bui, t.oui, t.gd);
    int lit = 0;
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++)
            lit += view.row(y)[x] != 0;
    CHECK(lit > 5000);
}

// The edit room requester's confirm commits the blueprint: its outline goes
// on the map, and the door is placed next.
void check_blueprint_committed(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::World &world = t.world();
    const th::BuildState &b = t.build();
    t.ev.triggered_window = int(th::Requester::EditRoom);
    t.ev.triggered_button = th::kEditRoomConfirm;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, {});
    CHECK(game.event.type == th::EV_COMMIT_BLUEPRINT);
    run_build_command(game);
    CHECK(t.building().mouse == th::BuildMouse::PlaceDoor && t.placing().requester_mode == 2 && b.state == 1 &&
          b.door_kind == 1);
    const th::RoomInfo &room = t.room();
    CHECK(room.pos == t.x0 + t.y0 * th::kMapSize && room.width == 5 && room.height == 5);
    CHECK(th::find_small_thing(world, t.site) == 3 && th::find_small_thing(world, t.site + 2) == 1 &&
          th::find_small_thing(world, t.site + 2 * th::kMapSize) == 2);
    CHECK(world.map[size_t(t.site + 1 + th::kMapSize)].floor_block == 0x17);
    CHECK(game.balance == t.balance);
}

// The door, in the north wall's middle; then a window can not go on it.
void check_door_placed(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::World &world = t.world();
    const th::Building &bd = t.building();
    const th::BuildState &b = t.build();
    const th::RoomInfo &room = t.room();
    const int door_cell = t.door_cell;
    t.bui.prev_cell = door_cell - th::kMapSize;
    th::build_mouse(t.bui, game, door_cell - th::kMapSize, {}, true);
    CHECK(game.event.type == th::EV_TEMP_PLACE_DOOR);
    run_build_command(game);
    CHECK(bd.door_shown && !bd.door_invalid && b.door_tile == door_cell && b.door_type == 4);
    th::UiMouse click;
    click.left_pressed = click.left_held = true;
    th::build_mouse(t.bui, game, door_cell - th::kMapSize, click, true);
    run_build_command(game);
    CHECK(t.placing().requester_mode == 4 && b.door_kind == 2 && th::find_small_thing(world, door_cell) == 4);
    CHECK(room.door_tile == door_cell - th::kMapSize && room.door_inside_tile == door_cell);
    game.event = {th::EV_TEMP_PLACE_DOOR, uint16_t(door_cell - th::kMapSize)};
    run_build_command(game);
    CHECK(bd.door_invalid && b.door_type == 8);
    game.event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(game);
    CHECK(room.windows == 0 && th::find_small_thing(world, door_cell) == 4);
}

// A window in the west wall, taken out with a right click and put back.
void check_window_placed(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::World &world = t.world();
    const th::BuildState &b = t.build();
    const th::RoomInfo &room = t.room();
    const int window_cell = t.x0 + (t.y0 + 2) * th::kMapSize;
    game.event = {th::EV_TEMP_PLACE_DOOR, uint16_t(window_cell - 1)};
    run_build_command(game);
    CHECK(!t.building().door_invalid && b.door_tile == window_cell && b.door_type == 10);
    game.event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(game);
    CHECK(room.windows == 1 && th::find_small_thing(world, window_cell) == 9);
    game.event = {th::EV_PLACE_BP_DOOR, 1};
    run_build_command(game);
    CHECK(room.windows == 0 && th::find_small_thing(world, window_cell) == 2);
    game.event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(game);
    CHECK(room.windows == 1);
}

// Confirmed: the walls, door and window built, the room's floor and
// colours, and its objects to place.
void check_doors_committed(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::World &world = t.world();
    const th::ObjectPlacing &p = t.placing();
    const int x0 = t.x0, y0 = t.y0, x1 = t.x1, y1 = t.y1;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, {});
    CHECK(game.event.type == th::EV_COMMIT_DOORS && game.event.value == kGp);
    run_build_command(game);
    CHECK(t.building().mouse == th::BuildMouse::None && t.build().state == 0 && t.room().state == th::kRoomStateEdited);
    CHECK(t.cell(x0 + 1, y0).wall_n == 0x50 && t.cell(x0, y0 + 1).wall_w == 0x51);
    CHECK(t.cell(x0 + 1, y1 + 1).wall_n == 0x58 && t.cell(x1 + 1, y0 + 1).wall_w == 0x59);
    // The blueprint's invisible wall, and the door's nested in it.
    CHECK(t.cell(x0 + 2, y0).wall_n == 0x90);
    CHECK(t.cell(x0, y0 + 2).wall_w == 0x7f);
    CHECK(t.cell(x0 + 2, y0 + 2).floor_block == th::kRoomTypeInfo[kGp].floor_block);
    CHECK(th::room_at(world, uint16_t(t.site + 2 + 2 * th::kMapSize)) == t.r &&
          th::room_at(world, uint16_t(t.site - 1)) == 0);
    int doors = 0;
    for (int id = t.cell(x0 + 2, y0).first; id > 0; id = world.things[size_t(id)].cell_next) {
        const th::Thing &d = world.things[size_t(id)];
        if (d.type == th::kThingDoor && d.subtype == 3 && d.room_id == t.r && d.facing == 0)
            doors++;
    }
    CHECK(doors == 1);
    CHECK(th::find_small_thing(world, t.site) == 3);
    CHECK(p.placing && p.kind == kGpDesk && p.mode == 0 && p.room == t.r && p.requester_mode == 6);
    CHECK(p.arg == 1u << th::kEditRoomConfirm && p.hint == 0xd);
    CHECK(th::check_room_access(world, 0));
}

// The objects, each where it may go in the room, paid for; one tried off
// the room is refused.
void check_objects_placed(BuildFixture &t)
{
    th::Game &game = *t.game;
    th::World &world = t.world();
    th::ObjectPlacing &p = t.placing();
    const th::RoomInfo &room = t.room();
    game.event = {th::EV_PLACE_OBJECT, 0, 0, uint16_t(kGpDesk)};
    run_build_command(game);
    CHECK(room.first_object == 0 && game.balance == t.balance);
    int placed = 0;
    while (p.placing && placed < 3) {
        int kind = p.kind;
        int tile = -1;
        for (int c = 0; c < th::kMapCells && tile < 0; c++) {
            th::update_placement(world, p, c);
            if (p.ok)
                tile = p.tile;
        }
        CHECK(tile >= 0 && th::room_at(world, uint16_t(tile)) == t.r);
        if (tile < 0)
            break;
        int before = game.balance;
        game.event = {th::EV_PLACE_OBJECT, uint16_t(tile), 0, uint16_t(kind | p.dir << 8)};
        run_build_command(game);
        CHECK(game.balance == before - t.settings.objects[size_t(kind)].start_cost);
        CHECK(room.first_object && world.things[room.first_object].subtype == kind &&
              world.things[room.first_object].room_id == t.r);
        placed++;
    }
    CHECK(placed == 3 && !p.placing && th::room_has_compulsory_objects(world, t.r));
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.arg == 0 && p.hint == 0xe);
    CHECK(game.balance == t.balance - t.objects_cost);
}

// Buy more: a plant, then back to placing it in the room.
void check_plant_bought(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = t.placing();
    t.ev.triggered_button = th::kEditRoomFurnish;
    th::build_requesters_input(t.ui, t.bui, game, t.ev, {});
    CHECK(game.event.type == th::EV_FURNISH_EDITED_ROOM);
    run_build_command(game);
    CHECK(p.requester == th::ObjectRequester::BuyObjects && p.arg3 == (t.room().pos & 0xff) && p.room_type == kGp);
    CHECK(std::find(p.offered.begin(), p.offered.end(), kPlant) != p.offered.end());
    game.event = {th::EV_BUY_OBJECT, kPlant, 0, 1};
    run_build_command(game);
    game.event = {th::EV_BACK_TO_PLACE_OBJECT, 0};
    run_build_command(game);
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.placing && p.kind == kPlant && p.room == t.r);
    CHECK(t.place_all() == 1 && game.balance == t.balance - t.objects_cost - t.settings.objects[kPlant].start_cost);
    t.ev.triggered_button = th::kEditRoomConfirm;
}

// Back to a blueprint (Escape): the objects lifted, the walls and door
// down; built again, the objects go back at no charge and no draw. Returns
// what was spent on the room's objects.
int check_back_to_blueprint(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = t.placing();
    const th::BuildState &b = t.build();
    const th::RoomInfo &room = t.room();
    const int spent = t.balance - game.balance;
    CHECK(th::object_escape(game) && game.event.type == th::EV_EDIT_BLUEPRINT);
    run_build_command(game);
    CHECK(t.building().mouse == th::BuildMouse::Blueprint && p.requester_mode == 0 &&
          room.state == th::kRoomStateBlueprint);
    CHECK(room.width == 0 && room.first_object == 0 && b.draw_mode == 1 && b.x0 == t.x0 && b.y1 == t.y1);
    CHECK(p.buy_count[kGpDesk] == 1 && p.buy_count[kChair] == 1 && p.buy_count[kCabinet] == 1 &&
          p.buy_count[kPlant] == 1);
    CHECK(std::count_if(b.objects.begin(), b.objects.end(), [](uint16_t id) { return id != 0; }) == 4);
    CHECK(t.cell(t.x0 + 1, t.y0).wall_n == 0 && th::find_small_thing(t.world(), t.site) == 0);
    game.event = {th::EV_COMMIT_BLUEPRINT};
    run_build_command(game);
    CHECK(t.building().mouse == th::BuildMouse::PlaceDoor);
    game.event = {th::EV_TEMP_PLACE_DOOR, uint16_t(t.door_cell - th::kMapSize)};
    run_build_command(game);
    game.event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(game);
    game.event = {th::EV_COMMIT_DOORS, uint16_t(kGp)};
    run_build_command(game);
    CHECK(p.placing && room.state == th::kRoomStateEdited);
    uint32_t seed = t.world().seed;
    CHECK(t.place_all() == 4 && game.balance == t.balance - spent && t.world().seed == seed);
    return spent;
}

// Confirmed: the room opens and is paid for, the objects having cost
// `spent`.
void check_room_opened(BuildFixture &t, int spent)
{
    th::Game &game = *t.game;
    const th::ObjectPlacing &p = t.placing();
    const th::RoomInfo &room = t.room();
    th::build_requesters_input(t.ui, t.bui, game, t.ev, {});
    CHECK(game.event.type == th::EV_OPEN_ROOM);
    run_build_command(game);
    CHECK(room.state == 0 && room.paid_for == 1 && t.build().room == 0 && t.building().last_built_room == kGp);
    CHECK(p.requester == th::ObjectRequester::None && th::find_small_thing(t.world(), t.site) == 0);
    CHECK(game.balance == t.balance - spent - t.room_cost);
    CHECK(game.finance.entry_count >= 2 &&
          game.finance.entries[game.finance.entry_count - 1u].type == th::kTxBuildRoom);
    CHECK(th::check_room_access(t.world(), 0));
}

// A second blueprint abandoned with Escape: no charge, the room freed.
void check_second_blueprint_abandoned(BuildFixture &t)
{
    th::Game &game = *t.game;
    const th::World &world = t.world();
    const th::ObjectPlacing &p = t.placing();
    const th::BuildState &b = t.build();
    const int balance = game.balance;
    th::open_build_room(game);
    run_build_command(game);
    game.event = {th::EV_INIT_BLUEPRINT, uint16_t(kGp), 0, uint16_t(kGp)};
    run_build_command(game);
    uint16_t r2 = b.room;
    CHECK(r2 != 0 && r2 != t.r && world.rooms[r2].type == kGp);
    CHECK(th::object_escape(game) && game.event.type == th::EV_DELETE_ROOM);
    run_build_command(game);
    CHECK(world.rooms[r2].type == th::kRoomFree && b.room == 0 && t.building().mouse == th::BuildMouse::None);
    CHECK(p.requester == th::ObjectRequester::None && p.buy_count[kGpDesk] == 0 && game.balance == balance);
}

} // namespace

void test_build_gp_office(const char *dir)
{
    auto t = std::make_unique<BuildFixture>();
    if (!t->start(dir))
        return;
    check_build_requester_opens(*t);
    check_build_requester_drawn(*t);
    check_blueprint_started(*t);
    check_public_land_refused(*t);

    t->site = first_clear_site(t->world(), t->build());
    CHECK(t->site >= 0);
    if (t->site < 0)
        return;
    t->x0 = t->site % th::kMapSize;
    t->y0 = t->site / th::kMapSize;
    t->x1 = t->x0 + 4;
    t->y1 = t->y0 + 4;
    t->door_cell = t->x0 + 2 + t->y0 * th::kMapSize;
    check_blueprint_dragged(*t);
    check_blueprint_committed(*t);
    check_door_placed(*t);
    check_window_placed(*t);
    check_doors_committed(*t);
    check_objects_placed(*t);
    check_plant_bought(*t);
    const int spent = check_back_to_blueprint(*t);
    check_room_opened(*t, spent);
    check_second_blueprint_abandoned(*t);
}

namespace
{

// A manned reception desk near the view start and a GP's office built
// through the build commands, with a doctor hired once it is open. Returns
// the doctor, 0 if any of it could not be done; `gp` is the office.
uint16_t staff_gp_office(QueueFixture &f, uint16_t &gp)
{
    start_rooms(f);
    th::World &w = f.game->world;
    const int start = f.level->camera_start[0];
    uint16_t recep = hire_near(f, th::kStaffReceptionist, start);
    uint16_t desk = f.place_near(start + 3, 11);
    CHECK(recep && desk);
    if (!recep || !desk)
        return 0;
    gp = lay_out_gp_office(f, start - 10 * th::kMapSize);
    CHECK(gp != 0);
    if (!gp)
        return 0;
    // No doctor yet: the blueprint makes nothing available.
    CHECK(w.cure_available[th::kIllGeneralPractice] == 0);
    CHECK(finish_gp_office(f, gp));
    CHECK(w.cure_available[th::kIllGeneralPractice] == 0 && th::check_room_access(w, 0));
    const th::RoomInfo &office = w.rooms[gp];
    // Staff may be put down in the office by kind (staff_states): a
    // doctor, not a receptionist.
    CHECK(th::staff_place_ok(w, office.door_inside_tile, th::kStaffDoctor));
    CHECK(!th::staff_place_ok(w, office.door_inside_tile, th::kStaffReceptionist));
    uint16_t doctor = hire_near(f, th::kStaffDoctor, start + 2 * th::kMapSize);
    CHECK(doctor != 0);
    if (!doctor)
        return 0;
    CHECK(w.cure_available[th::kIllGeneralPractice] == 1);
    return doctor;
}

// What the GP's office's patients did, each counted once.
struct GpCounts {
    int seated = 0, consulted = 0, waiting = 0, gone_home = 0;
    bool consistent = true; // every one consulted charged and diagnosed
    std::vector<bool> on_chair = std::vector<bool>(th::kThingCount);
    std::vector<bool> done = std::vector<bool>(th::kThingCount);
    std::vector<bool> deciding = std::vector<bool>(th::kThingCount);

    // Counts this turn's patients that sat on the office's chair, were
    // consulted, waited for a decision or went home after waiting.
    void count(const th::World &w, uint16_t gp)
    {
        for (uint16_t id = 1; id < w.things.size(); id++) {
            const th::Thing &p = w.things[id];
            if (p.type != th::kThingPatient) {
                on_chair[id] = done[id] = deciding[id] = false;
                continue;
            }
            if (p.room == gp && p.object && w.things[p.object].subtype == 6 && !on_chair[id]) {
                on_chair[id] = true;
                seated++;
            }
            if (p.treatments[0] == th::kIllGeneralPractice && !done[id]) {
                done[id] = true;
                consulted++;
                // Charged and diagnosed, on its way out or to wait.
                consistent &= p.fee != 0 || p.treatment == 0;
                consistent &= p.diagnosis > 0;
            }
            if (p.state == th::kPsWaitsForDecision && !deciding[id]) {
                deciding[id] = true;
                waiting++;
            }
            if (done[id] && p.state == th::kPsGotoDestination && p.complex_action == th::kComplexLeave && p.room == 0 &&
                deciding[id]) {
                deciding[id] = false;
                gone_home++;
            }
        }
    }
};

// Treatment income: paid in cash (kTxTreat lines), and claimed from
// insurers, paid or still due.
std::pair<int32_t, int32_t> treatment_income(const th::Game &g)
{
    int32_t treat = 0, claims = 0;
    for (size_t i = 0; i < g.finance.entry_count; i++)
        if (g.finance.entries[i].type == th::kTxTreat)
            treat += g.finance.entries[i].amount;
        else if (g.finance.entries[i].type == th::kTxInsurance)
            claims += g.finance.entries[i].amount;
    for (const auto &company : g.hospital.insurance_claims)
        for (int32_t c : company)
            claims += c;
    return {treat, claims};
}

// One run of test_gp_office_level1. Returns whether it got to the end;
// `seed` is the generator then.
bool gp_office_run(const char *dir, uint32_t &seed)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return false;
    uint16_t gp = 0;
    const uint16_t doctor = staff_gp_office(f, gp);
    if (!doctor)
        return false;
    th::Game &g = *f.game;
    th::World &w = g.world;
    const th::RoomInfo &office = w.rooms[gp];
    const th::Thing &d = w.things[doctor];

    int doctor_in = -1, calls = 0, max_queue = 0;
    uint16_t slave = 0;
    bool consistent = true;
    GpCounts n;
    for (int i = 0; i < 12000; i++) {
        f.turn();
        consistent &= queues_consistent(w) && cell_lists_consistent(w);
        if (doctor_in < 0 && d.room == gp && d.state == th::kStaffDoctorWorksAtDesk)
            doctor_in = i;
        if (d.slave && d.slave != slave)
            calls++;
        slave = d.slave;
        max_queue = std::max(max_queue, int(office.queue_length));
        n.count(w, gp);
    }
    auto [treat, claims] = treatment_income(g);
    std::printf("GP's office: doctor at the desk after %d turns, %d calls, %d seated, %d consulted, %d waiting, "
                "%d home, queue up to %d, treatments $%d in cash, $%d claimed, income $%d, seed %#x\n",
                doctor_in, calls, n.seated, n.consulted, n.waiting, n.gone_home, max_queue, int(treat), int(claims),
                int(g.hospital.illnesses[1].income), unsigned(w.seed));
    CHECK(consistent && n.consistent);
    CHECK(doctor_in >= 0 && calls > 0 && n.seated > 0 && n.consulted > 0 && n.waiting > 0 && n.gone_home > 0);
    CHECK(treat + claims == g.hospital.illnesses[1].income && treat > 0);
    CHECK(doctor_in == 1130 && calls == 17 && n.seated == 17 && n.consulted == 17 && n.waiting == 17 &&
          n.gone_home == 15 && treat == 1100 && claims == 600 && w.seed == 0x1b1bae8f);
    seed = w.seed;
    return true;
}

} // namespace

// With the game data: on level 1 opened at once, a receptionist mans a desk
// and a GP's office is built through the build commands; general practice
// becomes available once a doctor is hired (set_expertise_staff, the room
// listed by commit_doors). The doctor answers the page of the first
// patient sent to the office's queue, walks in through its door and sits
// at the desk; he calls the patients queueing (call_person_to_room), each
// walks in, sits on the chair and is consulted; then it is charged the
// GP's price (in cash, a kTxTreat line, or by its insurer at the month's
// end), its diagnosis has progressed and it walks out: with no diagnosis
// rooms nor treatment rooms, to wait for the player's decision (a player
// call) and, unanswered, home after 1500 turns. Discontented, the doctor
// hands in his notice and leaves before the run ends. Regression values,
// not checked against the game; the two runs agree.
void test_gp_office_level1(const char *dir)
{
    uint32_t seeds[2] = {};
    for (int run = 0; run < 2; run++)
        if (!gp_office_run(dir, seeds[run]))
            return;
    CHECK(seeds[0] == seeds[1]);
}

// With the game data: a patient standing where a blueprint is committed is
// sent off it (move_peep_from_blueprint, PS_BP_GOTO) and walks out, so the
// room gets built (commit_doors waits for its floor to clear).
void test_blueprint_cleared_level1(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    start_rooms(f);
    th::World &w = f.game->world;
    const int start = f.level->camera_start[0];
    uint16_t gp = lay_out_gp_office(f, start - 10 * th::kMapSize);
    CHECK(gp != 0);
    if (!gp)
        return;
    const th::RoomInfo &room = w.rooms[gp];
    int inside = room.pos + 2 + 2 * th::kMapSize;
    uint16_t walker = th::make_a_peep(w, f.game->arrivals, {f.settings, f.gd.anims(), 1}, uint16_t(inside), 16, 0);
    CHECK(walker != 0);
    if (!walker)
        return;
    // Waiting there, as one queueing at a reception would.
    th::Thing &p = w.things[walker];
    p.state = th::kPsEntertainsThemself;
    p.countdown = 1;
    bool moved = false;
    int turns = 0;
    f.game->event = {th::EV_COMMIT_DOORS, uint16_t(th::kRoomGp)};
    run_build_command(*f.game);
    CHECK(w.building.build.state == 2);
    while (turns < 2000 && w.building.build.state == 2) {
        f.turn();
        moved |= p.state == th::kPsBpGoto;
        turns++;
    }
    std::printf("blueprint cleared in %d turns, patient at %d,%d\n", turns, p.x.tile, p.y.tile);
    CHECK(moved && w.building.build.state == 0 && w.placing.placing);
    CHECK(th::room_at(w, uint16_t(p.x.tile + (p.y.tile << 7))) != gp);
}

namespace
{

// The player's commands of tools/replay/scripts/gp_office.txt (level 1): a
// reception desk and receptionist, a GP's office and its doctor.
constexpr const char *kGpOfficeScript = "5 98 0 0\n6 37 11 1\n7 43 0 0\n8 7 9534 11\n10 48 3 22\n11 5 9788 22\n"
                                        "101 55 0 0\n102 53 7 7\n103 2 8382 0\n104 2 8898 0\n105 8 8898 0\n"
                                        "106 18 0 0\n107 26 8256 0\n108 3 0 0\n109 4 7 0\n110 7 8640 1\n"
                                        "111 7 8386 2\n112 7 8641 6\n113 1 0 0\n114 48 1 11\n115 5 10044 11\n";

// A level started as th_replay starts it, played with a replay script's
// commands (lines "turn type value arg").
struct ScriptedGame {
    th::GameData gd;
    std::unique_ptr<th::Game> game;
    std::vector<std::pair<uint32_t, std::string>> lines;
    size_t next = 0;

    bool start(const char *dir, int level_number, const std::string &script)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        auto level = gd.load_level(level_number, &error);
        th::Balance settings;
        if (!level || !th::load_balance(dir, level_number, th::kDefaultBalanceId, settings, &error))
            return false;
        game = std::make_unique<th::Game>();
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, level_number, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        th::add_static_shadows(game->world.map);
        th::start_advisor(game->advisor, false);
        th::start_hospital_settings(game->hospital, game->settings());
        th::setup_insurances(game->world, 0);
        game->world.path_finder = th::PathFinder::Original;
        size_t at = 0;
        while (at < script.size()) {
            size_t end = script.find('\n', at);
            std::string line = script.substr(at, end - at);
            at = end == std::string::npos ? script.size() : end + 1;
            size_t space = line.find(' ');
            lines.emplace_back(uint32_t(std::stoul(line.substr(0, space))), line.substr(space + 1));
        }
        return true;
    }

    // Runs the turns up to `turn`, calling `each` after every one.
    bool run_to(uint32_t turn, const std::function<void()> &each)
    {
        while (game->turn_count < turn) {
            for (; next < lines.size() && lines[next].first <= game->turn_count + 1; next++)
                if (lines[next].first == game->turn_count + 1 && !game->replay_input(lines[next].second))
                    return false;
            game->turn();
            each();
            game->events.events.clear();
        }
        return true;
    }
};

// Whether patient `p` has had treatment `treatment`.
bool had_treatment(const th::Thing &p, uint8_t treatment)
{
    return std::find(p.treatments.begin(), p.treatments.end(), treatment) != p.treatments.end();
}

// A diagnosis room run_diagnosis_room checks: its name, the level and
// script that build it, the treatment it gives, its room type, its staff's
// state while working there, the turn to run to and the original's seed
// then.
struct DiagnosisRoom {
    const char *name;
    int level_number;
    const std::string &script;
    uint8_t treatment;
    uint8_t room_type;
    uint8_t staff_state;
    uint32_t turns;
    uint32_t seed;
};

// With the game data, a diagnosis room in use as tools/replay/scripts/<room>.txt
// builds it, run to the turn the replay check agrees with the original to:
// its member of staff works there, patients come in for its examination,
// pay its fee and are sent on, ending on the original's seed. `seen` is called after
// every turn. Returns the patients diagnosed there.
int run_diagnosis_room(const char *dir, const DiagnosisRoom &spec,
                       const std::function<void(const th::Game &)> &seen = {})
{
    ScriptedGame s;
    CHECK(s.start(dir, spec.level_number, spec.script));
    if (!s.game)
        return 0;
    th::Game &g = *s.game;
    th::World &w = g.world;
    std::vector<bool> examined(th::kThingCount);
    int diagnosed = 0, sent_on = 0;
    bool working = false;
    CHECK(s.run_to(spec.turns, [&] {
        const uint16_t room = w.room_of_type[spec.room_type];
        for (uint16_t id = 1; id < w.things.size(); id++) {
            const th::Thing &t = w.things[id];
            if (t.type == th::kThingStaff && room && t.room == room && t.state == spec.staff_state)
                working = true;
            if (t.type != th::kThingPatient) {
                examined[id] = false;
                continue;
            }
            if (!examined[id] && room && t.room == room && t.treatment == spec.treatment) {
                examined[id] = true;
                diagnosed++;
            }
        }
        for (const th::TraceEvent &e : g.events.events)
            sent_on += e.text.starts_with("next_treatment ");
        if (seen)
            seen(g);
    }));
    std::printf("%s: %d patients examined, income $%d, seed %#x\n", spec.name, diagnosed,
                int(g.hospital.illnesses[spec.treatment].income), unsigned(w.seed));
    CHECK(working && diagnosed > 0 && sent_on > 0 && g.hospital.illnesses[spec.treatment].income > 0);
    CHECK(w.rooms[w.room_of_type[spec.room_type]].patients_seen > 0);
    CHECK(w.seed == spec.seed);
    return diagnosed;
}

} // namespace

// tools/replay/scripts/general_diag.txt: a general diagnosis room and a
// second doctor, who examines patients there (SS_DOCTOR_IN_MACHINE_ROOM)
// and sends them back to the GP.
void test_general_diag_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8373 0\n137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n"
                               "142 48 1 13\n143 5 10044 13";
    bool back_to_gp = false;
    run_diagnosis_room(dir, {"general diagnosis", 1, script, 41, 27, 0x24, 18249, 0x9d56f095}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things)
            back_to_gp |= t.type == th::kThingPatient && had_treatment(t, 41) &&
                          t.treatment == th::kIllGeneralPractice && t.state == th::kPsGotoQueue;
    });
    CHECK(back_to_gp);
}

// tools/replay/scripts/cardiogram.txt (level 6): a cardiogram room, its
// doctor and a handyman, who repairs the worn machine (SS_PLAY_ANISEQ).
void test_cardiogram_level6(const char *dir)
{
    const std::string script =
        "5 98 0 0\n6 37 11 1\n7 43 0 0\n8 7 7481 11\n10 48 3 33\n11 5 7735 33\n101 55 0 0\n102 53 7 7\n"
        "103 2 6188 0\n104 2 6704 0\n105 8 6704 0\n106 18 0 0\n107 26 6062 0\n108 3 0 0\n109 4 7 0\n"
        "110 7 6446 1\n111 7 6192 2\n112 7 6447 6\n113 1 0 0\n114 48 1 18\n115 5 6969 18\n130 55 0 0\n"
        "131 53 12 12\n132 2 6196 0\n133 2 6583 0\n134 8 6583 0\n135 18 0 0\n136 26 6069 0\n137 3 0 0\n"
        "138 4 12 0\n139 7 6454 13\n140 7 6452 272\n141 1 0 0\n142 48 1 19\n143 5 6970 19\n144 48 2 28\n"
        "145 5 6841 28";
    int32_t most_used = 0;
    bool repairing = false, smoke = false, offered = false;
    run_diagnosis_room(dir, {"cardiogram", 6, script, 38, 12, 0x24, 18249, 0x79fe7886}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things) {
            if (t.type == th::kThingObject && t.subtype == 13) {
                most_used = std::max(most_used, t.times_used);
                smoke |= t.smoke[0] || t.smoke[1] || t.smoke[2];
            }
            repairing |= t.type == th::kThingStaff && t.state == th::kStaffPlayAniseq;
        }
        offered |= th::check_playercall(g.world, 2, 0, 0) != 0;
    });
    // Worn past three quarters (smoking), paged and repaired; an emergency
    // offered and expired.
    CHECK(most_used >= 10 && smoke && repairing && offered);
}

// tools/replay/scripts/ward.txt: a ward and a nurse, who puts patients to
// bed for their diagnosis (PS_IN_BED, SS_NURSE_IN_WARD).
void test_ward_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 9 9\n132 2 7602 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8371 0\n137 3 0 0\n138 4 9 0\n139 7 7731 264\n140 7 7990 1\n141 1 0 0\n"
                               "142 48 0 3\n143 5 10044 3";
    bool in_bed = false;
    run_diagnosis_room(dir, {"ward", 1, script, 42, 9, 0x32, 18249, 0x65d035ec}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things)
            in_bed |= t.type == th::kThingPatient && t.state == th::kPsInBed && t.object &&
                      g.world.things[t.object].subtype == 8;
    });
    CHECK(in_bed);
}

// tools/replay/scripts/psychiatry.txt: a psychiatry room and a
// psychiatrist, who examines patients there and cures multiple TV
// personalities (cure_illness).
void test_psychiatry_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 8 8\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8373 0\n137 3 0 0\n138 4 8 0\n139 7 7859 530\n140 7 8119 16\n"
                               "141 7 7733 61\n142 1 0 0\n143 48 1 15\n144 5 10044 15";
    uint16_t cured = 0;
    run_diagnosis_room(dir, {"psychiatry", 1, script, 43, 8, 0x24, 18249, 0x4c473759},
                       [&](const th::Game &g) { cured = g.hospital.illnesses[22].cured; });
    CHECK(cured > 0);
}

// tools/replay/scripts/treatment.txt: a pharmacy and its nurse, an
// inflation room and its doctor, with a policy guessing cures early.
// Patients are cured by drugs (administer_drugs), one drinking in the
// cabinet (PS_ENERGISES) and one inflated; others die (person_dies), float
// away (PS_FLYS) or are taken into the gates of hell by the grim reaper
// (summon_the_reaper), the gates removed afterwards; ending on the
// original's seed.
void test_treatment_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8373 0\n"
        "137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n142 48 1 13\n143 5 10044 13\n"
        "160 55 0 0\n161 53 11 11\n162 2 7737 0\n163 2 8124 0\n164 8 8124 0\n165 18 0 0\n166 26 8250 0\n"
        "167 3 0 0\n168 4 11 0\n169 7 7740 39\n170 1 0 0\n171 48 0 3\n172 5 10044 3\n"
        "190 55 0 0\n191 53 17 17\n192 2 8388 0\n193 2 8775 0\n194 8 8775 0\n195 18 0 0\n196 26 8515 0\n"
        "197 3 0 0\n198 4 17 0\n199 7 8518 9\n200 1 0 0\n201 48 1 12\n202 5 10044 12\n"
        "220 113 1 0\n221 113 2 10";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    bool energised = false, flying = false, reaper = false, into_hell = false;
    int drugs = 0, gates_placed = 0, gates_removed = 0;
    CHECK(s.run_to(18249, [&] {
        for (const th::Thing &t : w.things) {
            energised |= t.type == th::kThingPatient && t.state == th::kPsEnergises;
            flying |= t.type == th::kThingPatient && t.state == th::kPsFlys;
            into_hell |= t.type == th::kThingPatient && t.state == th::kPsAtHellGate;
            reaper |= t.type == th::kThingReaper;
        }
        for (const th::TraceEvent &e : g.events.events) {
            drugs += e.text.starts_with("drugs ");
            gates_placed += e.text.starts_with("place_object ") && e.text.find(" kind=48 ") != std::string::npos;
            gates_removed += e.text.starts_with("remove_object ");
        }
    }));
    const auto &ill = g.hospital.illnesses;
    std::printf("treatment: %d drugs given, %u cured, %u dead, seed %#x\n", drugs, unsigned(g.finance.cures),
                unsigned(g.finance.deaths), unsigned(w.seed));
    // Drugs for invisibility (5), the uncommon cold (16) and the squits
    // (29); inflation for bloaty head (2).
    CHECK(drugs > 0 && ill[5].cured > 0 && ill[16].cured > 0 && ill[29].cured > 0 && ill[2].cured > 0);
    CHECK(energised && flying && reaper && into_hell);
    CHECK(gates_placed > 0 && gates_removed == gates_placed);
    CHECK(g.finance.deaths > 0 && ill[5].deaths > 0 && g.finance.cures > 0);
    CHECK(w.seed == 0x8d8b8fc1);
}

// tools/replay/scripts/info_windows.txt: treatment.txt with a handyman whose
// priorities are changed, patients sent home (EV_PERSON_GO_HOME) and made to
// guess a cure (EV_FORCE_CURE), waiting, walking to a queue, with their
// doctor and in the inflator; ending on the original's seed at turn 18000.
void test_info_windows_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8373 0\n"
        "137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n142 48 1 13\n143 5 10044 13\n"
        "160 55 0 0\n161 53 11 11\n162 2 7737 0\n163 2 8124 0\n164 8 8124 0\n165 18 0 0\n166 26 8250 0\n"
        "167 3 0 0\n168 4 11 0\n169 7 7740 39\n170 1 0 0\n171 48 0 3\n172 5 10044 3\n"
        "190 55 0 0\n191 53 17 17\n192 2 8388 0\n193 2 8775 0\n194 8 8775 0\n195 18 0 0\n196 26 8515 0\n"
        "197 3 0 0\n198 4 17 0\n199 7 8518 9\n200 1 0 0\n201 48 1 12\n202 5 10044 12\n"
        "220 113 1 0\n221 113 2 10\n"
        "230 48 2 20\n231 5 10044 20\n240 115 20 29\n241 115 20 29\n242 115 20 31\n"
        "7350 116 46 0\n7420 134 47 0\n"
        "9000 115 20 31\n9001 115 20 31\n9002 115 20 31\n9003 115 20 31\n9004 115 20 31\n9005 115 20 31\n"
        "9006 115 20 31\n9007 115 20 31\n9008 115 20 31\n9009 115 20 31\n9010 115 20 31\n9011 115 20 31\n"
        "9500 115 20 29\n9501 115 20 30\n"
        "12550 116 26 0\n12560 134 9 0\n12600 134 50 0\n12700 116 5 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    std::vector<std::string> seen;
    bool sent_home = false, guessed = false, priorities = false;
    CHECK(s.run_to(18000, [&] {
        for (const th::TraceEvent &e : g.events.events)
            if (e.text.starts_with("handyman_values ") || e.text.starts_with("force_cure ") ||
                e.text.starts_with("go_home 46 ") || e.text.starts_with("go_home 26 "))
                seen.push_back(e.text);
        if (g.turn_count == 7350)
            sent_home = (w.things[46].person_flags & th::kPfSentHome) && w.things[46].treatment == 0;
        if (g.turn_count == 7420)
            guessed = (w.things[47].person_flags & th::kPfGuessed) && w.things[47].known_illness == 5;
        if (g.turn_count == 9501) {
            const th::Thing &h = w.things[20];
            priorities = h.prio_litter == 6 && h.prio_plants == 6 && h.prio_repair == 88;
        }
    }));
    auto has = [&](const std::string &text) { return std::find(seen.begin(), seen.end(), text) != seen.end(); };
    CHECK(sent_home && guessed && priorities);
    CHECK(has("handyman_values 20 litter=39 plants=31 repair=30"));
    CHECK(has("handyman_values 20 litter=0 plants=0 repair=100"));
    CHECK(has("force_cure 47") && has("force_cure 9") && has("force_cure 50"));
    CHECK(has("go_home 46 state=24") && has("go_home 26 state=32"));
    CHECK(std::find(g.command_log.begin(), g.command_log.end(), "7420 134 47 0") != g.command_log.end());
    CHECK(w.seed == 0x397debaa);
}

namespace
{

// The commands of tools/replay/scripts/toilets.txt before the toilets' own:
// gp_office.txt's up to the GP's office opened (no doctor), and the warm
// corridor with a drinks machine.
std::string toilets_script(const std::string &toilets)
{
    std::string gp = kGpOfficeScript;
    gp = gp.substr(0, gp.find("114 48 1 11\n"));
    return gp + toilets +
           "160 98 0 0\n161 37 7 1\n162 37 44 3\n163 43 0 0\n164 7 9658 7\n165 7 9276 44\n166 7 9410 44\n"
           "167 7 10047 44\n170 79 10 0";
}

// The seeds the original ends on at turn 18000 with staff_room.txt,
// toilets.txt and toilets_none.txt.
constexpr uint32_t kStaffRoomSeed = 0xe62dae65;
constexpr uint32_t kToiletsSeed = 0x3d222da5;
constexpr uint32_t kToiletsNoneSeed = 0xf5d9b905;

// The toilets (64,52)-(67,55) of toilets.txt, up to their objects.
constexpr const char *kToiletsRoom = "130 55 0 0\n131 53 29 29\n132 2 6720 0\n133 2 7107 0\n134 8 7107 0\n"
                                     "135 18 0 0\n136 26 7233 0\n137 3 0 0\n138 4 29 0\n";

// Counts the events of each kind (their first word) a run records.
struct EventCounts {
    std::vector<std::pair<std::string, int>> counts;

    void add(const th::Game &g)
    {
        for (const th::TraceEvent &e : g.events.events) {
            std::string kind = e.text.substr(0, e.text.find(' '));
            auto it = std::find_if(counts.begin(), counts.end(), [&](const auto &c) { return c.first == kind; });
            if (it == counts.end())
                counts.emplace_back(kind, 1);
            else
                it->second++;
        }
    }

    int operator[](const std::string &kind) const
    {
        auto it = std::find_if(counts.begin(), counts.end(), [&](const auto &c) { return c.first == kind; });
        return it == counts.end() ? 0 : it->second;
    }
};

} // namespace

// get_nearest_room_info: the room of the kind whose door is nearest, of the
// person's hospital, in use (state 0 or 1), nearer than 9999 (squared).
void test_nearest_room_info()
{
    auto world = std::make_unique<th::World>();
    th::World &w = *world;
    th::Thing p;
    p.type = th::kThingStaff;
    p.x.tile = 10;
    p.y.tile = 10;
    auto room = [&](uint16_t id, int x, int y, int8_t state, uint8_t owner) {
        th::RoomInfo &r = w.rooms[id];
        r.type = 25;
        r.state = state;
        r.owner = owner;
        r.door_tile = uint16_t(x + y * 128);
    };
    CHECK(th::get_nearest_room_info(w, p, 25) == 0);
    room(5, 20, 10, 0, 0);
    room(6, 12, 12, 4, 0);
    room(7, 11, 11, 0, 1);
    CHECK(th::get_nearest_room_info(w, p, 25) == 5);
    room(8, 10, 15, 1, 0);
    CHECK(th::get_nearest_room_info(w, p, 25) == 8);
    CHECK(th::get_nearest_room_info(w, p, 29) == 0);
    // 100 cells away squares past 9999: not found.
    room(5, 110, 10, 0, 0);
    room(8, 10, 110, 0, 0);
    CHECK(th::get_nearest_room_info(w, p, 25) == 0);
}

// tools/replay/scripts/staff_room.txt: with the policy's rest_threshold at
// 100, the doctor, the nurse and the handyman take breaks (take_a_break) in
// the staff room, resting (rest_staff) on its sofa, at pool and the video
// game until no longer tired; one picked up mid-game, one ordered out, one
// put down in the staff room; ending on the original's seed at turn 18000.
void test_staff_room_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 9 9\n132 2 7602 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8371 0\n"
        "137 3 0 0\n138 4 9 0\n139 7 7731 264\n140 7 7990 1\n141 1 0 0\n142 48 0 3\n143 5 10044 3\n"
        "160 55 0 0\n161 53 25 25\n162 2 6714 0\n163 2 7230 0\n164 8 7230 0\n165 18 0 0\n166 26 7356 0\n"
        "167 3 0 0\n168 4 25 0\n169 37 10 1\n170 37 57 1\n171 43 0 0\n172 7 6973 19\n173 7 6716 10\n"
        "174 7 7098 57\n175 1 0 0\n180 48 2 19\n181 5 10044 19\n182 48 3 23\n183 5 10044 23\n220 113 4 100\n"
        "2100 104 19 0\n2101 5 10044 19\n8100 97 3 0\n9000 113 4 300\n10000 104 11 0\n10001 5 6971 11\n"
        "17000 94 23 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    // The staff room is the eighth room listed.
    constexpr uint16_t kStaffRoom = 8;
    EventCounts counts;
    std::array<bool, 3> rested{};
    bool nurse_in = false, nurse_rested = false, nurse_out = false, put_in = false;
    CHECK(s.run_to(18000, [&] {
        counts.add(g);
        for (const th::TraceEvent &e : g.events.events) {
            rested[0] |= e.text.starts_with("rested 3 ");
            rested[1] |= e.text.starts_with("rested 11 ");
            rested[2] |= e.text.starts_with("rested 19 ");
            if (g.turn_count > 8100 && g.turn_count <= 8400)
                nurse_rested |= e.text.starts_with("rested 3 ");
        }
        if (g.turn_count == 8100)
            nurse_in = w.things[3].room == kStaffRoom;
        if (g.turn_count == 8400)
            nurse_out = w.things[3].room != kStaffRoom && !(w.things[3].orders & 0xf);
        if (g.turn_count == 10001)
            put_in = w.things[11].room == kStaffRoom;
    }));
    std::printf("staff_room: %d breaks, %d rests, %d pastimes, seed %#x\n", counts["take_a_break"],
                counts["rest_staff"], counts["staff_room"], unsigned(w.seed));
    // The nurse ordered out leaves before she is rested.
    CHECK(rested[0] && rested[1] && rested[2] && nurse_in && nurse_out && !nurse_rested && put_in);
    CHECK(counts["take_a_break"] >= 8 && counts["staff_room"] > 20 && counts["policy_rest"] == 2);
    CHECK(w.seed == kStaffRoomSeed);
}

// tools/replay/scripts/toilets.txt: waiting patients with a full bladder
// leave the queue for the toilets, use a toilet (relieved: bladder emptied)
// and a sink, and walk out again; one sent home on the toilet; ending on
// the original's seed at turn 18000.
void test_toilets_level1(const char *dir)
{
    const std::string script =
        toilets_script(std::string(kToiletsRoom) + "139 37 51 1\n140 43 0 0\n142 7 6720 51\n143 7 6723 32\n"
                                                   "144 7 6721 51\n145 1 0 0\n") +
        "\n8100 116 19 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    EventCounts counts;
    int sinks = 0;
    bool emptied = true, sent_home = false;
    CHECK(s.run_to(18000, [&] {
        counts.add(g);
        for (const th::TraceEvent &e : g.events.events) {
            sinks += e.text.starts_with("use_object ") && e.text.ends_with(" kind=32");
            // PS_PLAY_COMPLEX_ANIM: on the toilet.
            sent_home |= e.text == "go_home 19 state=16";
            if (e.text.starts_with("relieved ")) {
                const th::Thing &p = w.things[std::stoul(e.text.substr(9))];
                emptied &= p.bladder == 0 || p.illness == 29;
            }
        }
    }));
    std::printf("toilets: %d visits, %d relieved, %d sinks, seed %#x\n", counts["to_toilets"], counts["relieved"],
                sinks, unsigned(w.seed));
    CHECK(counts["to_toilets"] >= 10 && counts["relieved"] >= 10 && sinks > 0 && emptied);
    CHECK(counts["leave_toilets"] >= 10 && sent_home);
    CHECK(w.seed == kToiletsSeed);
}

// tools/replay/scripts/toilets_none.txt: with no toilets, a patient's
// bladder overflows in the corridor (wets, PF_WETS); ending on the
// original's seed at turn 18000.
void test_toilets_none_level1(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 1, toilets_script("")));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    EventCounts counts;
    CHECK(s.run_to(18000, [&] { counts.add(g); }));
    CHECK(counts["wets"] > 0 && counts["to_toilets"] == 0);
    CHECK(g.world.seed == kToiletsNoneSeed);
}
