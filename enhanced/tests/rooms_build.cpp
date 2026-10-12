// A GP's office built through the build commands, from the requester to the
// opened room, and a blueprint cleared of a patient (commit_doors and
// move_peep_from_blueprint, src/blueprnt.cpp).
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
#include <utility>
#include <vector>

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
        game->skipped_calls = th::kSkipComputerAction;
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
        th::build_mouse(bui, *game, from_x + from_y * th::kMapSize, 0, d, true);
        run_build_command(*game);
        d = {};
        d.left_held = true;
        th::build_mouse(bui, *game, to_x + to_y * th::kMapSize, 0, d, true);
        run_build_command(*game);
        th::build_mouse(bui, *game, to_x + to_y * th::kMapSize, 0, {}, true);
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
    th::build_mouse(t.bui, *t.game, x0 + 2 + (y0 + 2) * th::kMapSize, 0, {}, true);
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
    th::build_mouse(t.bui, game, door_cell - th::kMapSize, 0, {}, true);
    CHECK(game.event.type == th::EV_TEMP_PLACE_DOOR);
    run_build_command(game);
    CHECK(bd.door_shown && !bd.door_invalid && b.door_tile == door_cell && b.door_type == 4);
    th::UiMouse click;
    click.left_pressed = click.left_held = true;
    th::build_mouse(t.bui, game, door_cell - th::kMapSize, 0, click, true);
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
    CHECK(doctor_in == 1125 && calls == 13 && n.seated == 13 && n.consulted == 13 && n.waiting == 12 &&
          n.gone_home == 12 && treat == 800 && claims == 500 && w.seed == 0x9374d6f1);
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
