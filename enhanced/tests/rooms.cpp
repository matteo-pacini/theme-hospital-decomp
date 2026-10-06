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
#include <string>
#include <utility>
#include <vector>

// Furnishing level 1's corridor: the buy objects requester, buying and
// selling, and placing a reception desk through the commands.
void test_furnish_corridor(const char *dir)
{
    th::GameData gd;
    th::RequesterArt art;
    std::string error;
    CHECK(gd.load(dir, &error) && art.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    th::World &world = game->world;
    th::ObjectPlacing &p = world.placing;
    th::Interface ui;
    th::ObjectUi oui;
    th::init_object_ui(oui, gd.palette());
    const int desk = 11, bench = 4;
    const int desk_cost = settings.objects[desk].start_cost;
    CHECK(desk_cost > 0);

    th::furnish_corridor(*game);
    game->process_turn();
    CHECK(p.requester == th::ObjectRequester::BuyObjects && p.menu_disabled && p.arg == 1);
    std::vector<int> offered;
    for (uint8_t kind : p.offered)
        if (kind)
            offered.push_back(kind);
    // Reception desk, bench, drinks machine, fire extinguisher, radiator and
    // plant; level 1 makes no litter bombs available.
    CHECK((offered == std::vector<int>{11, 4, 7, 43, 44, 45}));
    CHECK(p.item == desk);
    th::sync_object_requesters(ui, *game);
    CHECK(ui.requester() == th::Requester::BuyObjects && ui.window_count() == 1);
    const th::Window &win = ui.window(int(th::Requester::BuyObjects));

    // A press then a release at (x, y), then the turn's commands.
    auto click = [&](int x, int y, bool right) {
        th::UiMouse m;
        m.x = x;
        m.y = y;
        (right ? m.right_pressed : m.left_pressed) = true;
        (right ? m.right_held : m.left_held) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, false, false);
        m = {};
        m.x = x;
        m.y = y;
        (right ? m.right_released : m.left_released) = true;
        th::object_requesters_input(ui, oui, *game, ui.frame(m), m, false, false);
        game->process_turn();
        th::sync_object_requesters(ui, *game);
    };
    int balance = game->balance;
    click(win.x + 331 + 8, win.y + 37 + 8, false); // the desk's plus
    CHECK(p.buy_count[desk] == 1 && game->balance == balance);
    click(win.x + 200, win.y + 55 + 9, true); // sell a bench: none bought
    CHECK(p.buy_count[bench] == 0 && p.item == bench);
    click(win.x + 200, win.y + 55 + 9, false); // the bench's entry: buys
    CHECK(p.buy_count[bench] == 1);
    click(win.x + 287 + 8, win.y + 56 + 8, false); // its minus
    CHECK(p.buy_count[bench] == 0);
    game->balance = desk_cost + desk_cost / 2;
    click(win.x + 331 + 8, win.y + 37 + 8, false); // a second desk: too dear
    CHECK(p.buy_count[desk] == 1);
    game->balance = balance;

    // The requester's own drawing: text and the selected kind's picture.
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> plain = layer.to_argb(gd.palette());
    th::draw_object_requesters(layer, ui, oui, *game, art, gd);
    std::vector<uint32_t> drawn = layer.to_argb(gd.palette());
    int changed = 0;
    for (size_t i = 0; i < drawn.size(); i++)
        changed += drawn[i] != plain[i];
    CHECK(changed > 1000);

    click(win.x + 156 + 50, win.y + 240 + 10, false); // accept
    CHECK(p.placing && p.kind == desk && p.mode == 2 && !p.menu_disabled);
    CHECK(p.requester == th::ObjectRequester::EditRoom && ui.requester() == th::Requester::EditRoom);

    // The first cell the desk may go on.
    int tile = -1;
    for (int t = 0; t < th::kMapCells && tile < 0; t++) {
        th::update_placement(world, p, t);
        if (p.ok && p.tile == t)
            tile = t;
    }
    CHECK(tile >= 0);
    if (tile < 0)
        return;
    th::update_placement(world, p, 0); // public land, off the hospital
    CHECK(!p.ok);
    game->event = {th::EV_PLACE_OBJECT, 0, 0, uint16_t(desk | p.dir << 8)};
    game->process_turn();
    CHECK(world.first_of_kind[desk] == 0 && game->balance == balance && p.buy_count[desk] == 1);

    // The desk following the pointer, in colour and, where it may not go,
    // in greys.
    th::UiMouse m;
    th::placement_mouse(oui, *game, tile, m, true);
    CHECK(oui.cursor_shown && p.ok);
    th::Surface view, base;
    view.resize(320, 240);
    view.fill(0);
    base = view;
    th::Point w = th::cell_to_world(tile % th::kMapSize, tile / th::kMapSize);
    th::Point camera = {w.x - 160, w.y - 120};
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, oui, gd);
    int coloured = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            coloured += view.row(y)[x] != 0;
    CHECK(coloured > 200);
    th::Surface colour_view = view;
    view = base;
    p.ok = false;
    th::draw_placement(view, {0, 0, 320, 240}, camera, p, oui, gd);
    int greyed = 0;
    for (int y = 0; y < 240; y++)
        for (int x = 0; x < 320; x++)
            greyed += colour_view.row(y)[x] && view.row(y)[x] == oui.grey[colour_view.row(y)[x]];
    CHECK(greyed > coloured * 8 / 10);
    p.redraw = true;

    // A left click places it: paid for, on the map and in the desks' list.
    m.left_pressed = m.left_held = true;
    th::placement_mouse(oui, *game, tile, m, true);
    CHECK(game->event.type == th::EV_PLACE_OBJECT && game->event.value == tile);
    game->process_turn();
    th::sync_object_requesters(ui, *game);
    CHECK(game->balance == balance - desk_cost);
    uint16_t id = world.first_of_kind[desk];
    CHECK(id != 0 && world.map[size_t(tile)].first == int16_t(id));
    if (!id)
        return;
    const th::Thing &placed = world.things[id];
    CHECK(placed.type == th::kThingObject && placed.subtype == desk && placed.owner == 0);
    CHECK(placed.x.tile == tile % th::kMapSize && placed.y.tile == tile / th::kMapSize);
    CHECK(world.map[size_t(tile)].flags & 3);
    CHECK((world.map[placed.use_pos[0]].flags & 4) && (world.map[placed.use_pos[1]].flags & 4));
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[desk] == 0);
    CHECK(ui.window_count() == 0 && ui.requester() == th::Requester::None);

    // A second desk may not go on the first; Escape drops it unbought.
    game->event = {th::EV_BUY_OBJECT, uint16_t(desk), 0, 1};
    game->process_turn();
    game->event = {th::EV_PLACE_NEXT_OBJECT, 0, 0, 0};
    game->process_turn();
    CHECK(p.placing && p.kind == desk);
    th::update_placement(world, p, tile);
    CHECK(!p.ok);
    CHECK(th::object_escape(*game));
    game->process_turn();
    CHECK(!p.placing && p.requester == th::ObjectRequester::None && p.buy_count[desk] == 0);
    CHECK(game->balance == balance - desk_cost && world.things[world.first_of_kind[desk]].type_obj_next == 0);
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
    for (int run = 0; run < 2; run++) {
        QueueFixture f;
        CHECK(f.start(dir));
        if (!f.level)
            return;
        th::World &w = f.game->world;
        CHECK(w.cure_available[0] == 1 && w.cure_available[th::kIllGeneralPractice] == 0);

        uint16_t recep = 0;
        for (uint16_t i = 1; i < w.things.size() && !recep; i++)
            if (w.things[i].type == th::kThingStaff && w.things[i].subtype == th::kStaffReceptionist &&
                w.things[i].state == th::kStaffApplicant)
                recep = i;
        int start = f.level->camera_start[0];
        int cell = -1;
        for (int d = 0; d < 400 && cell < 0; d++)
            if (w.map[size_t(start + d)].first == 0 && th::staff_place_ok(w, start + d, th::kStaffReceptionist))
                cell = start + d;
        CHECK(recep != 0 && cell >= 0);
        if (!recep || cell < 0)
            return;
        CHECK(th::place_staff_on_map(w, f.gd.anims(), f.settings, cell, recep, false, f.game->turn_count));
        uint16_t desk = f.place_near(cell + 3, 11);
        uint16_t bench = f.place_near(cell + 3 + 3 * th::kMapSize, 4);
        uint16_t drinks = f.place_near(cell - 4 + 2 * th::kMapSize, 7);
        CHECK(desk && bench && drinks);
        if (!desk || !bench || !drinks)
            return;
        const th::Thing &r = w.things[recep];
        int turns = 0;
        while (turns < 2000 && !(w.things[desk].user == recep && r.state == th::kStaffReceptionistWorks)) {
            f.turn();
            turns++;
        }
        CHECK(r.state == th::kStaffReceptionistWorks && r.room != 0);
        uint16_t reception = r.room;
        CHECK(w.rooms[reception].type == th::kRoomReception && w.rooms[reception].queue_capacity == 20);

        // No GP's office: patients queue and wait until fed up.
        bool consistent = true;
        bool called = false;
        int queued = 0, sat = 0, fed_up = 0, max_queue = 0;
        std::vector<bool> seen(th::kThingCount), was_queued(th::kThingCount);
        for (int i = 0; i < 8000; i++) {
            f.turn();
            consistent &= queues_consistent(w) && cell_lists_consistent(w);
            called |= r.slave != 0;
            max_queue = std::max(max_queue, int(w.rooms[reception].queue_length));
            for (uint16_t id = 1; id < w.things.size(); id++) {
                const th::Thing &p = w.things[id];
                if (p.type != th::kThingPatient) {
                    seen[id] = was_queued[id] = false;
                    continue;
                }
                if (p.queue_room == reception && !was_queued[id]) {
                    was_queued[id] = true;
                    queued++;
                }
                if (p.object == bench && w.things[bench].user == id && !seen[id]) {
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
        // Banked at each month's end.
        int32_t sold = w.drinks_income;
        for (size_t i = 0; i < f.game->finance.entry_count; i++)
            if (f.game->finance.entries[i].type == th::kTxIncomeDrinks)
                sold += f.game->finance.entries[i].amount;
        std::printf("reception, no GP: %d queued, %d sat, %d fed up (%u counted), queue up to %d, drinks %d, "
                    "seed %#x\n",
                    queued, sat, fed_up, unsigned(w.patients_left_count), max_queue, int(sold), unsigned(w.seed));
        CHECK(consistent && !called);
        CHECK(queued > 0 && sat > 0 && fed_up > 0 && w.patients_left_count >= fed_up);
        CHECK(queued == 7 && sat == 7 && fed_up == 5 && max_queue == 3 && sold == 100 && w.seed == 0x8287c6fe);

        // A GP's office: she calls the patients in queue order and serves
        // them.
        start_rooms(f);
        uint16_t gp = lay_out_gp_office(f, cell - 10 * th::kMapSize);
        CHECK(gp != 0 && finish_gp_office(f, gp));
        if (!gp)
            return;
        th::RoomInfo &office = w.rooms[gp];
        office.queue_capacity = 30;
        w.cure_available[th::kIllGeneralPractice] = 1;
        bool in_order = true;
        int calls = 0, served = 0, to_office = 0;
        uint16_t slave = 0;
        uint16_t seen_before = w.rooms[reception].patients_seen;
        std::vector<bool> going(th::kThingCount);
        for (int i = 0; i < 3000; i++) {
            std::vector<uint16_t> before = queue_of(w, reception);
            f.turn();
            consistent &= queues_consistent(w) && cell_lists_consistent(w);
            if (r.slave && r.slave != slave) {
                calls++;
                // Those queueing ahead of the one called have all left the
                // queue.
                for (uint16_t id : before) {
                    if (id == r.slave)
                        break;
                    in_order &= w.things[id].queue_room != reception;
                }
            }
            slave = r.slave;
            for (uint16_t id = 1; id < w.things.size(); id++) {
                const th::Thing &p = w.things[id];
                if (p.type != th::kThingPatient) {
                    going[id] = false;
                    continue;
                }
                if (p.dest_room == gp && !going[id]) {
                    going[id] = true;
                    to_office++;
                }
            }
        }
        served = w.rooms[reception].patients_seen - seen_before;
        std::printf("reception, GP: %d calls, %d served, %d sent to the office, its queue %d (%d coming), seed %#x\n",
                    calls, served, to_office, int(office.queue_length), int(office.queue_incoming), unsigned(w.seed));
        CHECK(consistent && in_order);
        CHECK(calls > 0 && served > 0 && to_office > 0 && office.queue_length > 0);
        CHECK(calls == 8 && served == 8 && to_office == 8 && office.queue_length == 2 && w.seed == 0x570f87c1);

        // With the reception's queue full, a new patient leaves at once.
        w.rooms[reception].queue_capacity = w.rooms[reception].queue_length;
        uint16_t late =
            th::make_a_peep(w, f.game->arrivals, f.settings, f.gd.anims(), 1, f.level->entrances[0][0], 16, 0);
        CHECK(late != 0);
        if (!late)
            return;
        th::start_person_going(w, f.gd.anims(), f.gd.person_complexes(), late, f.game->turn_count);
        CHECK(w.things[late].complex_action == th::kComplexLeave && w.things[late].dest_room == 0);
    }
}

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
    th::World &w = f.game->world;
    uint16_t recep = 0;
    for (uint16_t i = 1; i < w.things.size() && !recep; i++)
        if (w.things[i].type == th::kThingStaff && w.things[i].subtype == th::kStaffReceptionist &&
            w.things[i].state == th::kStaffApplicant)
            recep = i;
    int start = f.level->camera_start[0];
    int cell = -1;
    for (int d = 0; d < 400 && cell < 0; d++)
        if (w.map[size_t(start + d)].first == 0 && th::staff_place_ok(w, start + d, th::kStaffReceptionist))
            cell = start + d;
    CHECK(recep != 0 && cell >= 0);
    if (!recep || cell < 0)
        return;
    CHECK(th::place_staff_on_map(w, f.gd.anims(), f.settings, cell, recep, false, f.game->turn_count));
    CHECK(f.place_near(cell + 3, 11) != 0);
    start_rooms(f);
    uint16_t gp = lay_out_gp_office(f, cell - 10 * th::kMapSize);
    CHECK(gp != 0 && finish_gp_office(f, gp));
    if (!gp)
        return;
    w.rooms[gp].queue_capacity = 30;
    w.cure_available[th::kIllGeneralPractice] = 1;

    th::EventLog &log = f.game->events;
    log.limit = 0;
    std::vector<std::string> events;
    uint16_t patient = 0;
    bool rand_seen = false;
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
    CHECK(patient != 0 && !rand_seen);
    // Each expected event of the patient's, after the one before.
    std::string id = std::to_string(patient);
    const std::string expected[] = {"peep " + id + " ",
                                    "start_going " + id,
                                    "join_queue " + id + " ",
                                    "call " + std::to_string(recep) + " id=" + id + " ",
                                    "checkin " + id + " receptionist=" + std::to_string(recep),
                                    "shall_pay " + id + " "};
    size_t at = 0;
    for (const std::string &want : expected) {
        while (at < events.size() && !events[at].starts_with(want))
            at++;
        CHECK(at < events.size());
        if (at == events.size())
            std::printf("events: no \"%s\" in order\n", want.c_str());
    }

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

void test_build_gp_office(const char *dir)
{
    th::GameData gd;
    th::RequesterArt art;
    std::string error;
    CHECK(gd.load(dir, &error) && art.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    th::World &world = game->world;
    th::ObjectPlacing &p = world.placing;
    th::Building &bd = world.building;
    th::BuildState &b = bd.build;
    th::Interface ui;
    th::BuildUi bui;
    th::ObjectUi oui;
    th::init_object_ui(oui, gd.palette());
    const int gp = th::kRoomGp, desk = 1, cabinet = 2, chair = 6;
    const int room_cost = settings.rooms[gp].cost;
    const int objects_cost =
        settings.objects[desk].start_cost + settings.objects[cabinet].start_cost + settings.objects[chair].start_cost;
    CHECK(room_cost > 0 && objects_cost > 0);

    // The panel's icon opens the requester; the diagnosis department lists
    // the GP's office first.
    CHECK(!th::build_tool_busy(*game));
    th::open_build_room(*game);
    run_build_command(*game);
    CHECK(p.requester == th::ObjectRequester::BuildRoom && p.requester_mode == 1 && p.menu_disabled);
    th::sync_build_requester(ui, *game);
    CHECK(ui.requester() == th::Requester::BuildRoom);
    const th::Window &win = ui.window(int(th::Requester::BuildRoom));
    th::UiMouse m;
    th::InterfaceEvents ev;
    ev.triggered_window = int(th::Requester::BuildRoom);
    ev.triggered_button = 1;
    th::build_requesters_input(ui, bui, *game, ev, m);
    CHECK(game->event.type == th::EV_REQ1_RESET_ROOMS && game->event.value == 3);
    run_build_command(*game);
    CHECK(p.requester_mode == 4 && p.arg == 3 && p.offered[0] == gp);

    // The requester's own drawing.
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    th::draw_interface(layer, ui, art);
    std::vector<uint32_t> plain = layer.to_argb(gd.palette());
    m.x = win.x + 170;
    m.y = win.y + 40;
    th::build_requesters_input(ui, bui, *game, {}, m);
    CHECK(p.room_type == gp && bui.hovered_entry == 0);
    th::draw_build_requester(layer, ui, bui, *game, art, gd);
    std::vector<uint32_t> drawn = layer.to_argb(gd.palette());
    int changed = 0;
    for (size_t i = 0; i < drawn.size(); i++)
        changed += drawn[i] != plain[i];
    CHECK(changed > 1000);

    // Refused while the balance does not cover the room and its objects.
    int balance = game->balance;
    game->balance = room_cost + objects_cost;
    ev.triggered_button = 16;
    th::build_requesters_input(ui, bui, *game, ev, m);
    CHECK(game->event.type == th::EV_NONE);
    game->balance = balance;
    th::build_requesters_input(ui, bui, *game, ev, m);
    CHECK(game->event.type == th::EV_INIT_BLUEPRINT && game->event.value == gp);
    run_build_command(*game);
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.requester_mode == 0 && !p.menu_disabled);
    CHECK(p.buy_count[desk] == 1 && p.buy_count[cabinet] == 1 && p.buy_count[chair] == 1);
    CHECK(bd.mouse == th::BuildMouse::Blueprint && th::build_tool_busy(*game) && b.room != 0);
    const uint16_t r = b.room;
    CHECK(world.rooms[r].type == gp && world.rooms[r].state == th::kRoomStateBlueprint);
    th::sync_build_requester(ui, *game);
    th::sync_object_requesters(ui, *game);
    CHECK(ui.requester() == th::Requester::EditRoom && ui.window_count() == 1);

    // Dragging a blueprint out with the mouse, on public land: invalid.
    auto drag = [&](int x0, int y0, int x1, int y1) {
        th::UiMouse d;
        d.left_pressed = d.left_held = true;
        th::build_mouse(bui, *game, x0 + y0 * th::kMapSize, d, true);
        run_build_command(*game);
        d = {};
        d.left_held = true;
        th::build_mouse(bui, *game, x1 + y1 * th::kMapSize, d, true);
        run_build_command(*game);
        th::build_mouse(bui, *game, x1 + y1 * th::kMapSize, {}, true);
        run_build_command(*game);
    };
    drag(2, 2, 6, 6);
    CHECK(b.x0 == 2 && b.y0 == 2 && b.x1 == 6 && b.y1 == 6 && b.drag == th::kDragNone && b.draw_mode == 1);
    CHECK(th::blueprint_floor_invalid(world, 3, 3));
    game->event = {th::EV_COMMIT_BLUEPRINT};
    run_build_command(*game);
    CHECK(bd.mouse == th::BuildMouse::Blueprint && game->advisor.forced_advice_id == 4 && p.requester_mode == 0);

    // A 5 x 5 site on the hospital's land with a clear ring round it.
    int site = -1;
    th::BuildState saved = b;
    for (int y = 1; y + 6 < th::kMapSize && site < 0; y++) {
        for (int x = 1; x + 6 < th::kMapSize && site < 0; x++) {
            b.x0 = uint8_t(x - 1);
            b.y0 = uint8_t(y - 1);
            b.x1 = uint8_t(x + 5);
            b.y1 = uint8_t(y + 5);
            bool ok = true;
            for (int cy = y - 1; cy <= y + 5 && ok; cy++)
                for (int cx = x - 1; cx <= x + 5 && ok; cx++)
                    ok = !th::blueprint_floor_invalid(world, cx, cy);
            if (ok)
                site = x + y * th::kMapSize;
        }
    }
    b = saved;
    CHECK(site >= 0);
    if (site < 0)
        return;
    const int x0 = site % th::kMapSize, y0 = site / th::kMapSize, x1 = x0 + 4, y1 = y0 + 4;
    // Too small for a GP's office.
    drag(x0, y0, x0 + 2, y0 + 2);
    CHECK(th::blueprint_floor_invalid(world, x0, y0));
    drag(x0 + 2, y0 + 2, x1, y1); // the bottom-right corner, dragged out
    CHECK(b.x0 == x0 && b.y0 == y0 && b.x1 == x1 && b.y1 == y1);
    CHECK(!th::blueprint_floor_invalid(world, x0, y0) && !th::blueprint_outline_invalid(world, x1 + 1, y1 + 1));

    // Drawn over the view.
    th::Surface view;
    view.resize(640, 480);
    view.fill(0);
    th::Point centre = th::cell_to_world(x0 + 2, y0 + 2);
    th::Point camera = {centre.x - 320, centre.y - 240};
    th::build_mouse(bui, *game, x0 + 2 + (y0 + 2) * th::kMapSize, {}, true);
    run_build_command(*game);
    th::draw_build_overlay(view, {0, 0, 640, 480}, camera, *game, bui, oui, gd);
    int lit = 0;
    for (int y = 0; y < 480; y++)
        for (int x = 0; x < 640; x++)
            lit += view.row(y)[x] != 0;
    CHECK(lit > 5000);

    // The edit room requester's confirm commits it: its outline goes on the
    // map, and the door is placed next.
    ev.triggered_window = int(th::Requester::EditRoom);
    ev.triggered_button = th::kEditRoomConfirm;
    th::build_requesters_input(ui, bui, *game, ev, {});
    CHECK(game->event.type == th::EV_COMMIT_BLUEPRINT);
    run_build_command(*game);
    CHECK(bd.mouse == th::BuildMouse::PlaceDoor && p.requester_mode == 2 && b.state == 1 && b.door_kind == 1);
    const th::RoomInfo &room = world.rooms[r];
    CHECK(room.pos == x0 + y0 * th::kMapSize && room.width == 5 && room.height == 5);
    CHECK(th::find_small_thing(world, site) == 3 && th::find_small_thing(world, site + 2) == 1 &&
          th::find_small_thing(world, site + 2 * th::kMapSize) == 2);
    CHECK(world.map[size_t(site + 1 + th::kMapSize)].floor_block == 0x17);
    CHECK(game->balance == balance);

    // The door, in the north wall's middle; then a window can not go on it.
    const int door_cell = x0 + 2 + y0 * th::kMapSize;
    bui.prev_cell = door_cell - th::kMapSize;
    th::build_mouse(bui, *game, door_cell - th::kMapSize, {}, true);
    CHECK(game->event.type == th::EV_TEMP_PLACE_DOOR);
    run_build_command(*game);
    CHECK(bd.door_shown && !bd.door_invalid && b.door_tile == door_cell && b.door_type == 4);
    th::UiMouse click;
    click.left_pressed = click.left_held = true;
    th::build_mouse(bui, *game, door_cell - th::kMapSize, click, true);
    run_build_command(*game);
    CHECK(p.requester_mode == 4 && b.door_kind == 2 && th::find_small_thing(world, door_cell) == 4);
    CHECK(room.door_tile == door_cell - th::kMapSize && room.door_inside_tile == door_cell);
    game->event = {th::EV_TEMP_PLACE_DOOR, uint16_t(door_cell - th::kMapSize)};
    run_build_command(*game);
    CHECK(bd.door_invalid && b.door_type == 8);
    game->event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(*game);
    CHECK(room.windows == 0 && th::find_small_thing(world, door_cell) == 4);

    // A window in the west wall, taken out with a right click and put back.
    const int window_cell = x0 + (y0 + 2) * th::kMapSize;
    game->event = {th::EV_TEMP_PLACE_DOOR, uint16_t(window_cell - 1)};
    run_build_command(*game);
    CHECK(!bd.door_invalid && b.door_tile == window_cell && b.door_type == 10);
    game->event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(*game);
    CHECK(room.windows == 1 && th::find_small_thing(world, window_cell) == 9);
    game->event = {th::EV_PLACE_BP_DOOR, 1};
    run_build_command(*game);
    CHECK(room.windows == 0 && th::find_small_thing(world, window_cell) == 2);
    game->event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(*game);
    CHECK(room.windows == 1);

    // Confirmed: the walls, door and window built, the room's floor and
    // colours, and its objects to place.
    th::build_requesters_input(ui, bui, *game, ev, {});
    CHECK(game->event.type == th::EV_COMMIT_DOORS && game->event.value == gp);
    run_build_command(*game);
    CHECK(bd.mouse == th::BuildMouse::None && b.state == 0 && room.state == th::kRoomStateEdited);
    auto cell = [&](int x, int y) -> const th::MapCell & { return world.map[size_t(x + y * th::kMapSize)]; };
    CHECK(cell(x0 + 1, y0).wall_n == 0x50 && cell(x0, y0 + 1).wall_w == 0x51);
    CHECK(cell(x0 + 1, y1 + 1).wall_n == 0x58 && cell(x1 + 1, y0 + 1).wall_w == 0x59);
    // The blueprint's invisible wall, and the door's nested in it.
    CHECK(cell(x0 + 2, y0).wall_n == 0x90);
    CHECK(cell(x0, y0 + 2).wall_w == 0x7f);
    CHECK(cell(x0 + 2, y0 + 2).floor_block == th::kRoomTypeInfo[gp].floor_block);
    CHECK(th::room_at(world, uint16_t(site + 2 + 2 * th::kMapSize)) == r &&
          th::room_at(world, uint16_t(site - 1)) == 0);
    int doors = 0;
    for (int id = cell(x0 + 2, y0).first; id > 0; id = world.things[size_t(id)].cell_next) {
        const th::Thing &t = world.things[size_t(id)];
        if (t.type == th::kThingDoor && t.subtype == 3 && t.room_id == r && t.facing == 0)
            doors++;
    }
    CHECK(doors == 1);
    CHECK(th::find_small_thing(world, site) == 3);
    CHECK(p.placing && p.kind == desk && p.mode == 0 && p.room == r && p.requester_mode == 6);
    CHECK(p.arg == 1u << th::kEditRoomConfirm && p.hint == 0xd);
    CHECK(th::check_room_access(world, 0));

    // The objects, each where it may go in the room; one tried off the room
    // is refused.
    game->event = {th::EV_PLACE_OBJECT, 0, 0, uint16_t(desk)};
    run_build_command(*game);
    CHECK(room.first_object == 0 && game->balance == balance);
    int placed = 0;
    while (p.placing && placed < 3) {
        int kind = p.kind;
        int tile = -1;
        for (int t = 0; t < th::kMapCells && tile < 0; t++) {
            th::update_placement(world, p, t);
            if (p.ok)
                tile = p.tile;
        }
        CHECK(tile >= 0 && th::room_at(world, uint16_t(tile)) == r);
        if (tile < 0)
            break;
        int before = game->balance;
        game->event = {th::EV_PLACE_OBJECT, uint16_t(tile), 0, uint16_t(kind | p.dir << 8)};
        run_build_command(*game);
        CHECK(game->balance == before - settings.objects[size_t(kind)].start_cost);
        CHECK(room.first_object && world.things[room.first_object].subtype == kind &&
              world.things[room.first_object].room_id == r);
        placed++;
    }
    CHECK(placed == 3 && !p.placing && th::room_has_compulsory_objects(world, r));
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.arg == 0 && p.hint == 0xe);
    CHECK(game->balance == balance - objects_cost);

    // Each object left to place, where it first may go.
    auto place_all = [&]() {
        int n = 0;
        while (p.placing && n < 8) {
            int kind = p.kind, tile = -1;
            for (int t = 0; t < th::kMapCells && tile < 0; t++) {
                th::update_placement(world, p, t);
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
    };

    // Buy more: a plant, then back to placing it in the room.
    const int plant = 45;
    ev.triggered_button = th::kEditRoomFurnish;
    th::build_requesters_input(ui, bui, *game, ev, {});
    CHECK(game->event.type == th::EV_FURNISH_EDITED_ROOM);
    run_build_command(*game);
    CHECK(p.requester == th::ObjectRequester::BuyObjects && p.arg3 == (room.pos & 0xff) && p.room_type == gp);
    CHECK(std::find(p.offered.begin(), p.offered.end(), plant) != p.offered.end());
    game->event = {th::EV_BUY_OBJECT, plant, 0, 1};
    run_build_command(*game);
    game->event = {th::EV_BACK_TO_PLACE_OBJECT, 0};
    run_build_command(*game);
    CHECK(p.requester == th::ObjectRequester::EditRoom && p.placing && p.kind == plant && p.room == r);
    CHECK(place_all() == 1 && game->balance == balance - objects_cost - settings.objects[plant].start_cost);
    ev.triggered_button = th::kEditRoomConfirm;

    // Back to a blueprint (Escape): the objects lifted, the walls and door
    // down; built again, the objects go back at no charge and no draw.
    const int spent = balance - game->balance;
    CHECK(th::object_escape(*game) && game->event.type == th::EV_EDIT_BLUEPRINT);
    run_build_command(*game);
    CHECK(bd.mouse == th::BuildMouse::Blueprint && p.requester_mode == 0 && room.state == th::kRoomStateBlueprint);
    CHECK(room.width == 0 && room.first_object == 0 && b.draw_mode == 1 && b.x0 == x0 && b.y1 == y1);
    CHECK(p.buy_count[desk] == 1 && p.buy_count[chair] == 1 && p.buy_count[cabinet] == 1 && p.buy_count[plant] == 1);
    CHECK(std::count_if(b.objects.begin(), b.objects.end(), [](uint16_t id) { return id != 0; }) == 4);
    CHECK(cell(x0 + 1, y0).wall_n == 0 && th::find_small_thing(world, site) == 0);
    game->event = {th::EV_COMMIT_BLUEPRINT};
    run_build_command(*game);
    CHECK(bd.mouse == th::BuildMouse::PlaceDoor);
    game->event = {th::EV_TEMP_PLACE_DOOR, uint16_t(door_cell - th::kMapSize)};
    run_build_command(*game);
    game->event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(*game);
    game->event = {th::EV_COMMIT_DOORS, uint16_t(gp)};
    run_build_command(*game);
    CHECK(p.placing && room.state == th::kRoomStateEdited);
    uint32_t seed = world.seed;
    CHECK(place_all() == 4 && game->balance == balance - spent && world.seed == seed);

    // Confirmed: the room opens and is paid for.
    th::build_requesters_input(ui, bui, *game, ev, {});
    CHECK(game->event.type == th::EV_OPEN_ROOM);
    run_build_command(*game);
    CHECK(room.state == 0 && room.paid_for == 1 && b.room == 0 && bd.last_built_room == gp);
    CHECK(p.requester == th::ObjectRequester::None && th::find_small_thing(world, site) == 0);
    CHECK(game->balance == balance - spent - room_cost);
    CHECK(game->finance.entry_count >= 2 &&
          game->finance.entries[game->finance.entry_count - 1u].type == th::kTxBuildRoom);
    CHECK(th::check_room_access(world, 0));

    // A second blueprint abandoned with Escape: no charge, the room freed.
    balance = game->balance;
    th::open_build_room(*game);
    run_build_command(*game);
    game->event = {th::EV_INIT_BLUEPRINT, uint16_t(gp), 0, uint16_t(gp)};
    run_build_command(*game);
    uint16_t r2 = b.room;
    CHECK(r2 != 0 && r2 != r && world.rooms[r2].type == gp);
    CHECK(th::object_escape(*game) && game->event.type == th::EV_DELETE_ROOM);
    run_build_command(*game);
    CHECK(world.rooms[r2].type == th::kRoomFree && b.room == 0 && bd.mouse == th::BuildMouse::None);
    CHECK(p.requester == th::ObjectRequester::None && p.buy_count[desk] == 0 && game->balance == balance);
}

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
    for (int run = 0; run < 2; run++) {
        QueueFixture f;
        CHECK(f.start(dir));
        if (!f.level)
            return;
        start_rooms(f);
        th::Game &g = *f.game;
        th::World &w = g.world;
        const int start = f.level->camera_start[0];
        uint16_t recep = hire_near(f, th::kStaffReceptionist, start);
        uint16_t desk = f.place_near(start + 3, 11);
        CHECK(recep && desk);
        if (!recep || !desk)
            return;
        uint16_t gp = lay_out_gp_office(f, start - 10 * th::kMapSize);
        CHECK(gp != 0);
        if (!gp)
            return;
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
            return;
        CHECK(w.cure_available[th::kIllGeneralPractice] == 1);
        const th::Thing &d = w.things[doctor];

        int doctor_in = -1, calls = 0, seated = 0, consulted = 0, waiting = 0, gone_home = 0, max_queue = 0;
        uint16_t slave = 0;
        bool consistent = true;
        std::vector<bool> on_chair(th::kThingCount), done(th::kThingCount), deciding(th::kThingCount);
        for (int i = 0; i < 12000; i++) {
            f.turn();
            consistent &= queues_consistent(w) && cell_lists_consistent(w);
            if (doctor_in < 0 && d.room == gp && d.state == th::kStaffDoctorWorksAtDesk)
                doctor_in = i;
            if (d.slave && d.slave != slave)
                calls++;
            slave = d.slave;
            max_queue = std::max(max_queue, int(office.queue_length));
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
                if (done[id] && p.state == th::kPsGotoDestination && p.complex_action == th::kComplexLeave &&
                    p.room == 0 && deciding[id]) {
                    deciding[id] = false;
                    gone_home++;
                }
            }
        }
        int32_t treat = 0, claims = 0;
        for (size_t i = 0; i < g.finance.entry_count; i++)
            if (g.finance.entries[i].type == th::kTxTreat)
                treat += g.finance.entries[i].amount;
            else if (g.finance.entries[i].type == th::kTxInsurance)
                claims += g.finance.entries[i].amount;
        for (const auto &company : g.hospital.insurance_claims)
            for (int32_t c : company)
                claims += c;
        std::printf("GP's office: doctor at the desk after %d turns, %d calls, %d seated, %d consulted, %d waiting, "
                    "%d home, queue up to %d, treatments $%d in cash, $%d claimed, income $%d, seed %#x\n",
                    doctor_in, calls, seated, consulted, waiting, gone_home, max_queue, int(treat), int(claims),
                    int(g.hospital.illnesses[1].income), unsigned(w.seed));
        CHECK(consistent);
        CHECK(doctor_in >= 0 && calls > 0 && seated > 0 && consulted > 0 && waiting > 0 && gone_home > 0);
        CHECK(treat + claims == g.hospital.illnesses[1].income && treat > 0);
        CHECK(doctor_in == 1130 && calls == 17 && seated == 17 && consulted == 17 && waiting == 17 && gone_home == 15 &&
              treat == 1100 && claims == 600 && w.seed == 0x1b1bae8f);
        seeds[run] = w.seed;
    }
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
    uint16_t walker = th::make_a_peep(w, f.game->arrivals, f.settings, f.gd.anims(), 1, uint16_t(inside), 16, 0);
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

// With the game data, a diagnosis room in use as tools/replay/scripts/<room>.txt
// builds it, run to the turn the replay check agrees with the original to:
// its member of staff works there, patients come in for its examination,
// pay its fee and are sent on, ending on the original's seed. `seen` is called after
// every turn. Returns the patients diagnosed there.
int run_diagnosis_room(const char *dir, const char *name, int level_number, const std::string &script,
                       uint8_t treatment, uint8_t room_type, uint8_t staff_state, uint32_t turns, uint32_t seed,
                       const std::function<void(const th::Game &)> &seen = {})
{
    ScriptedGame s;
    CHECK(s.start(dir, level_number, script));
    if (!s.game)
        return 0;
    th::Game &g = *s.game;
    th::World &w = g.world;
    std::vector<bool> examined(th::kThingCount);
    int diagnosed = 0, sent_on = 0;
    bool working = false;
    CHECK(s.run_to(turns, [&] {
        const uint16_t room = w.room_of_type[room_type];
        for (uint16_t id = 1; id < w.things.size(); id++) {
            const th::Thing &t = w.things[id];
            if (t.type == th::kThingStaff && room && t.room == room && t.state == staff_state)
                working = true;
            if (t.type != th::kThingPatient) {
                examined[id] = false;
                continue;
            }
            if (!examined[id] && room && t.room == room && t.treatment == treatment) {
                examined[id] = true;
                diagnosed++;
            }
        }
        for (const th::TraceEvent &e : g.events.events)
            sent_on += e.text.starts_with("next_treatment ");
        if (seen)
            seen(g);
    }));
    std::printf("%s: %d patients examined, income $%d, seed %#x\n", name, diagnosed,
                int(g.hospital.illnesses[treatment].income), unsigned(w.seed));
    CHECK(working && diagnosed > 0 && sent_on > 0 && g.hospital.illnesses[treatment].income > 0);
    CHECK(w.rooms[w.room_of_type[room_type]].patients_seen > 0);
    CHECK(w.seed == seed);
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
    run_diagnosis_room(dir, "general diagnosis", 1, script, 41, 27, 0x24, 18249, 0x9d56f095, [&](const th::Game &g) {
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
    run_diagnosis_room(dir, "cardiogram", 6, script, 38, 12, 0x24, 18249, 0x79fe7886, [&](const th::Game &g) {
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
    run_diagnosis_room(dir, "ward", 1, script, 42, 9, 0x32, 18249, 0x65d035ec, [&](const th::Game &g) {
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
    run_diagnosis_room(dir, "psychiatry", 1, script, 43, 8, 0x24, 18249, 0x4c473759,
                       [&](const th::Game &g) { cured = g.hospital.illnesses[22].cured; });
    CHECK(cured > 0);
}
