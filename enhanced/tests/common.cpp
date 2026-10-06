#include "common.h"

#include "core/data/level.h"
#include "core/ui/build_room.h"
#include "core/ui/charts.h"
#include "core/ui/layer.h"
#include "core/world/build.h"
#include "core/world/events.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

int failures = 0;

void check(bool ok, const char *file, int line, const char *text)
{
    if (ok)
        return;
    std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", file, line, text);
    failures++;
}

bool cell_lists_consistent(const th::World &w)
{
    for (int i = 0; i < th::kMapCells; i++) {
        int prev = 0;
        for (int id = w.map[size_t(i)].first; id > 0; id = w.things[size_t(id)].cell_next) {
            const th::Thing &t = w.things[size_t(id)];
            if (t.cell_prev != prev || t.x.tile + t.y.tile * th::kMapSize != i)
                return false;
            prev = id;
        }
    }
    return true;
}

int free_staff_cell(const th::World &w, const th::Level &level)
{
    int start = level.camera_start[0];
    for (int r = 0; r < 20; r++)
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++) {
                int c = start + dx + dy * th::kMapSize;
                if (c >= 0 && c < th::kMapCells && w.map[size_t(c)].first == 0 &&
                    th::staff_place_ok(w, c, th::kStaffReceptionist))
                    return c;
            }
    return -1;
}

void start_rooms(QueueFixture &f)
{
    th::Game &g = *f.game;
    th::start_hospital_settings(g.hospital, g.settings());
    th::start_rooms_in_use(g, f.gd.person_complexes(), f.gd.staff_complexes());
}

namespace
{

// Whether a 5 x 5 blueprint at x,y with a clear ring round it may be laid
// out: `b`'s bounds are left set to the ring's.
bool gp_site_ok(const th::World &w, th::BuildState &b, int x, int y)
{
    if (x < 2 || y < 2 || x + 6 >= th::kMapSize || y + 6 >= th::kMapSize)
        return false;
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

// The cell of the GP's office site nearest `near`, searching rings out to a
// distance of 40; -1 if none. `b` is as it was on return.
int find_gp_site(const th::World &w, th::BuildState &b, int near)
{
    const th::BuildState saved = b;
    const int nx = near % th::kMapSize, ny = near / th::kMapSize;
    int site = -1;
    for (int d = 0; d < 40 && site < 0; d++) {
        for (int y = ny - d; y <= ny + d && site < 0; y++) {
            for (int x = nx - d; x <= nx + d && site < 0; x++) {
                if (std::max(std::abs(x - nx), std::abs(y - ny)) == d && gp_site_ok(w, b, x, y))
                    site = x + y * th::kMapSize;
            }
        }
    }
    b = saved;
    return site;
}

} // namespace

uint16_t lay_out_gp_office(QueueFixture &f, int near)
{
    th::Game &game = *f.game;
    th::World &w = game.world;
    th::BuildState &b = w.building.build;
    th::open_build_room(game);
    run_build_command(game);
    game.event = {th::EV_INIT_BLUEPRINT, uint16_t(th::kRoomGp), 0, uint16_t(th::kRoomGp)};
    run_build_command(game);
    const uint16_t r = b.room;
    if (!r)
        return 0;
    const int site = find_gp_site(w, b, near);
    if (site < 0)
        return 0;
    th::BuildUi bui;
    auto mouse_at = [&](int cell, bool pressed, bool held) {
        th::UiMouse m;
        m.left_pressed = pressed;
        m.left_held = held;
        th::build_mouse(bui, game, cell, m, true);
        run_build_command(game);
    };
    const int corner = site + 4 + 4 * th::kMapSize;
    mouse_at(site, true, true);
    mouse_at(corner, false, true);
    mouse_at(corner, false, false);
    game.event = {th::EV_COMMIT_BLUEPRINT};
    run_build_command(game);
    const int door_cell = site + 2;
    game.event = {th::EV_TEMP_PLACE_DOOR, uint16_t(door_cell - th::kMapSize)};
    run_build_command(game);
    game.event = {th::EV_PLACE_BP_DOOR, 0};
    run_build_command(game);
    return w.rooms[r].door_tile ? r : 0;
}

bool finish_gp_office(QueueFixture &f, uint16_t r)
{
    th::Game &game = *f.game;
    th::World &w = game.world;
    th::ObjectPlacing &p = w.placing;
    game.event = {th::EV_COMMIT_DOORS, uint16_t(th::kRoomGp)};
    run_build_command(game);
    for (int i = 0; i < 3000 && !p.placing; i++)
        f.turn();
    for (int n = 0; p.placing && n < 8; n++) {
        int kind = p.kind, tile = -1;
        for (int t = 0; t < th::kMapCells && tile < 0; t++) {
            th::update_placement(w, p, t);
            if (p.ok)
                tile = p.tile;
        }
        if (tile < 0)
            return false;
        game.event = {th::EV_PLACE_OBJECT, uint16_t(tile), 0, uint16_t(kind | p.dir << 8)};
        run_build_command(game);
    }
    game.event = {th::EV_OPEN_ROOM};
    run_build_command(game);
    return w.rooms[r].state == 0 && w.room_of_type[th::kRoomGp] == r;
}

uint16_t hire_near(QueueFixture &f, uint8_t kind, int from)
{
    th::World &w = f.game->world;
    uint16_t id = 0;
    for (uint16_t i = 1; i < w.things.size() && !id; i++)
        if (w.things[i].type == th::kThingStaff && w.things[i].subtype == kind &&
            w.things[i].state == th::kStaffApplicant)
            id = i;
    for (int d = 0; id && d < 400; d++) {
        if (w.map[size_t(from + d)].first != 0 || !th::staff_place_ok(w, from + d, kind))
            continue;
        bool placed = th::place_staff_on_map(w, {f.gd.anims(), f.settings, f.game->turn_count}, from + d, id, false);
        return placed ? id : 0;
    }
    return 0;
}

void run_build_command(th::Game &game)
{
    th::Event e = game.event;
    if (th::apply_build_event(game, e))
        game.event = {};
    else
        game.process_turn();
}
