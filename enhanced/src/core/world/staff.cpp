#include "core/world/staff.h"

#include "core/world/complex_tables.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/reception.h"
#include "core/world/staff_complex.h"

#include <algorithm>
#include <bit>
#include <cstdlib>

namespace th
{

namespace
{

// allocate_thing's reserve for new staff (place_staff_in_struct).
constexpr size_t kStaffReserve = 0x19;

// staff[kind].layer5_count (src/tables.cpp): the layer-5 variants an
// applicant picks from, 0 for none.
constexpr std::array<uint8_t, 5> kLayer5Count = {0, 2, 1, 0, 1};

// staff[kind].anim (src/tables.cpp): the staff_anim_starts entry shown
// placed or held, 5 (standing) or 0 (walking).
constexpr std::array<uint8_t, 5> kHeldAnim = {5, 5, 5, 0, 5};

// staff_states[rooms[0].staff_state_row] (src/tables.cpp, row 0): the state
// of staff placed in the corridor, by kind.
constexpr std::array<uint8_t, 5> kCorridorStates = {kStaffInCorridor, kStaffInCorridor, kStaffHandymanWorks,
                                                    kStaffReceptionistFindsReception, kStaffInCorridor};

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

// calculate_speed (0x2c268), for staff at a cell's middle outside rooms.
void calculate_speed(Thing &staff, const Balance &settings)
{
    if (staff.x.frac != 0x80 || staff.y.frac != 0x80)
        return;
    const Balance::Gbv &gbv = settings.gbv;
    uint32_t speed = 3;
    if (staff.tiredness > int(gbv.crack_up_tired))
        speed = 1;
    else if (staff.tiredness > int(gbv.very_tired))
        speed = 2;
    if (staff.ability >= gbv.consultant_threshold)
        speed += 30;
    else if (staff.ability > gbv.doctor_threshold)
        speed += 20;
    else
        speed += 10;
    switch (speed) {
    case 11:
    case 12:
    case 13:
    case 21:
    case 22:
    case 31:
        staff.speed = 0x10;
        staff.anim_speed = 2;
        break;
    case 23:
    case 32:
        staff.speed = 0x20;
        staff.anim_speed = 1;
        break;
    case 33:
        staff.speed = 0x40;
        staff.anim_speed = 1;
        break;
    }
}

// staff_in_corridor (0x2abb0): wanders from where they stand.
void staff_in_corridor(Thing &staff, const AnimData &anims)
{
    staff.dest_x = staff.x.tile;
    staff.dest_y = staff.y.tile;
    staff.state = kStaffWanders;
    staff.substate = 6;
    staff.countdown = 1;
    play_frame(staff, anims, kStaffAnims[staff.subtype].stand);
}

constexpr uint8_t kObjReceptionDesk = 11;

// The complexes the staff's states give (S_COMP.LEX).
constexpr uint16_t kComplexGoToDesk = 0x50;
constexpr uint16_t kComplexCallPatient = 0x1e;
constexpr uint16_t kComplexPatientCured = 0xe6;
constexpr uint16_t kComplexPatientNotCured = 0xf0;
constexpr uint16_t kComplexWalkTo = 0xbe;
constexpr uint16_t kComplexSweepUp = 0x46;
constexpr uint16_t kComplexAfterSweeping = 0x30c;
// The complex the patient called to the desk runs (P_COMP.LEX).
constexpr uint16_t kComplexToDesk = 0x6e;

// The complexes of a receptionist's idle tasks at the desk, after their
// count (the game's recept_tasks, src/tables.cpp).
constexpr std::array<uint16_t, 6> kReceptTasks = {5, 90, 100, 110, 120, 130};

// The game's showanim_angle (src/tables.cpp): the facing a handyman turns
// to to sweep, by his facing.
constexpr std::array<uint8_t, 4> kShowanimAngle = {1, 1, 2, 2};

// The game's special_around (src/tables.cpp): a cell and its eight
// neighbours, as cell offsets.
constexpr std::array<int16_t, 9> kSpecialAround = {0, 1, 128, -1, -128, -129, 129, -127, 127};

// The animation columns of kStaffAnimStarts with a name in the game.
constexpr size_t kAnimWalk = 0;
constexpr size_t kAnimStand = 5;

// MapCell.owner_flags: an outdoor cell.
constexpr uint8_t kCellOutdoor = 0x10;

struct Ctx {
    World &world;
    const AnimData &anims;
    const PersonComplexes &person;
    const StaffComplexes &staff;
    uint32_t game_turn;
};

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

void play_staff_anim(const AnimData &anims, Thing &s, size_t column)
{
    play_frame(s, anims, kStaffAnimStarts[s.subtype][column]);
}

uint8_t process(const Ctx &c, uint16_t id)
{
    return process_staff_complex(c.world, c.anims, c.person, c.staff, id, c.game_turn);
}

// can_i_get_to (0x19918) for a person: whether a path leads from their
// cell to `pos`.
bool can_i_get_to(const World &world, const Thing &person, uint16_t pos)
{
    OrthonavPath path;
    orthonav_init(world, path, person.x.tile, person.y.tile, pos % kMapSize, pos / kMapSize);
    return path.count != 0;
}

// Moves a walking person speed / 256 cell on in their facing; one stuck
// (facing 4) stays.
void step_on(World &world, uint16_t id)
{
    Thing &t = world.things[id];
    if (t.facing >= 4)
        return;
    uint16_t raw_x = uint16_t((t.x.tile << 8 | t.x.frac) + t.speed * kAroundXy[t.facing][0]);
    uint16_t raw_y = uint16_t((t.y.tile << 8 | t.y.frac) + kAroundXy[t.facing][1] * t.speed);
    move_node(world, id, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
}

// Whether `cell` holds litter to sweep (MapCell.litter_shadow's low nibble:
// 0 none, 7 and 8 are not swept).
bool has_litter(const MapCell &cell)
{
    uint8_t kind = cell.litter_shadow & 0xf;
    return !(kind == 0 || kind == 7 || kind == 8);
}

// listhead[owner].rooms[ROOM_RECEPTION] != 0: place_existing_object sets it
// when the hospital's first reception desk is placed (to an uninitialised
// value, taken to be nonzero) and nothing clears it. Desks cannot be sold
// yet, so it is whether the hospital has one; only player 0's are listed.
bool has_reception(const World &world, uint8_t owner)
{
    return owner == 0 && world.first_of_kind[kObjReceptionDesk] != 0;
}

// shall_i_resign (0x2bfc8): draws the months (six to nine) after which
// discontented staff resign. Discontent and poaching (modify_staff) are not
// kept, so no one resigns or hands in notice.
bool shall_i_resign(World &world)
{
    old_newrand(world.seed);
    return false;
}

// staff_nav_failure (0x2b5cc), with no rooms: stuck (SS_STUCK). The game
// also announces it for the player's staff.
void staff_nav_failure(Thing &staff)
{
    staff.saved_state = staff.state;
    staff.saved_dest_x = staff.dest_x;
    staff.saved_dest_y = staff.dest_y;
    staff.state = kStaffStuck;
    staff.countdown = 0;
}

// find_free_object_anywhere (0x3800c), outside rooms: the nearest free
// object of `kind` of the seeker's hospital (by cells across and down)
// whose use cell (0 for staff, 2 for handymen, 1 for patients) a path from
// the seeker reaches. 0 if none.
uint16_t find_free_object_anywhere(const World &world, uint16_t id, uint8_t kind)
{
    const Thing &seeker = world.things[id];
    if (seeker.owner != 0)
        return 0;
    size_t side = 1;
    if (seeker.type == kThingStaff)
        side = seeker.subtype == kStaffHandyman ? 2 : 0;
    uint16_t best = 0;
    uint32_t best_dist = 9999;
    for (uint16_t o = world.first_of_kind[kind]; o; o = world.things[o].type_obj_next) {
        const Thing &obj = world.things[o];
        if (obj.in_use != 0 || !can_i_get_to(world, seeker, obj.use_pos[side]) || obj.room_id)
            continue;
        auto dist = uint32_t(std::abs(seeker.x.tile - obj.x.tile) + std::abs(seeker.y.tile - obj.y.tile));
        if (dist < best_dist) {
            best_dist = dist;
            best = o;
        }
    }
    return best;
}

// try_staff_wander (0x778b4): if the cell (dx, dy) from the wanderer is
// walkable corridor of their hospital and their destination moved by (dx,
// dy) is on the map, moves it there and sets off in `substate`; otherwise
// substate 6, to decide again.
void try_staff_wander(const Ctx &c, uint16_t id, int dx, int dy, uint8_t substate)
{
    Thing &e = c.world.things[id];
    int x = e.x.tile + dx;
    int y = e.y.tile + dy;
    auto tile = uint16_t(x + (y << 7));
    // which_room: there are no rooms.
    if (!(tile < kMapCells && !(c.world.map[tile].flags & 3) && x >= 0 && x < kMapSize && y >= 0 && y < kMapSize) ||
        c.world.map[tile].owner_flags != 1 << e.owner) {
        e.substate = 6;
        return;
    }
    if (!(e.dest_x + dx >= 0 && e.dest_x + dx < kMapSize && e.dest_y + dy >= 0 && e.dest_y + dy < kMapSize)) {
        e.substate = 6;
        return;
    }
    e.dest_x = uint8_t(e.dest_x + dx);
    e.dest_y = uint8_t(e.dest_y + dy);
    play_staff_anim(c.anims, e, kAnimWalk);
    nav_init(c.world, e);
    e.countdown = 1;
    e.substate = substate;
}

// The decision of staff_wanders: a stroll of 3 cells back the way they
// came (85%), else, standing, a pause of 20-49 turns, a stroll of 3 cells
// in a random direction or one to a random cell within 3.
// Vaccinating, breaks and pages have no rooms or requests to answer yet.
void wander_decide(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (old_newrand(world.seed) % 100 < 0x55) {
        switch (e.substate) {
        case 1:
            try_staff_wander(c, id, 0, 3, 3);
            return;
        case 3:
            try_staff_wander(c, id, 0, -3, 1);
            return;
        case 2:
            try_staff_wander(c, id, 3, 0, 4);
            return;
        case 4:
            try_staff_wander(c, id, -3, 0, 2);
            return;
        }
    }
    e.substate = 6;
    play_staff_anim(c.anims, e, kAnimStand);
    uint32_t roll = old_newrand(world.seed) % 100;
    if (roll < 0xf) {
        e.substate = 0;
        e.countdown = int16_t(old_newrand(world.seed) % 30 + 20);
    } else if (roll < 0x17) {
        try_staff_wander(c, id, 0, -3, 1);
    } else if (roll < 0x1f) {
        try_staff_wander(c, id, -3, 0, 2);
    } else if (roll < 0x27) {
        try_staff_wander(c, id, 0, 3, 3);
    } else if (roll < 0x2f) {
        try_staff_wander(c, id, 3, 0, 4);
    } else {
        // The game takes old_newrand() % 7 - 3 in unsigned arithmetic into a
        // signed byte: -3 to 3.
        auto dx = int8_t(old_newrand(world.seed) % 7 - 3);
        auto dy = int8_t(old_newrand(world.seed) % 7 - 3);
        try_staff_wander(c, id, dx, dy, 5);
    }
}

// staff_wanders (0x77ab8), with no blueprints, rooms or requests.
void staff_wanders(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (e.subtype == kStaffHandyman) {
        e.state = kStaffHandymanWorks;
        e.substate = 0;
        return;
    }
    if (shall_i_resign(world))
        return;
    if (e.substate == 0) {
        if (--e.countdown <= 0)
            wander_decide(c, id);
        return;
    }
    if (e.substate == 6) {
        wander_decide(c, id);
        return;
    }
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0) {
        if (e.subtype == kStaffReceptionist && has_reception(world, e.owner) &&
            find_free_object_anywhere(world, id, kObjReceptionDesk)) {
            e.state = kStaffReceptionistFindsReception;
            return;
        }
        if (shall_i_resign(world))
            return;
        e.countdown = int16_t(0x100 / e.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
        case kNavNoPath:
            e.countdown = 1;
            wander_decide(c, id);
            return;
        case kNavBlockedByPeople:
            play_staff_anim(c.anims, e, kAnimStand);
            e.countdown = 0;
            return;
        case kNavWalledIn:
            staff_nav_failure(e);
            return;
        default:
            if (e.facing != dir) {
                e.facing = uint8_t(dir);
                play_staff_anim(c.anims, e, kAnimWalk);
            }
            break;
        }
    }
    step_on(world, id);
}

// handyman_check_for_litter (0x7b554): looks at a random cell near the
// handyman (nearer the more able he is) and remembers it if it has litter
// on his hospital's land and is nearer than what he remembers.
void handyman_check_for_litter(World &world, Thing &h)
{
    int range = std::max(10 - h.ability / 100, 1);
    // Unsigned arithmetic into a signed int, as in the game.
    auto dx = int32_t(old_newrand(world.seed) % uint32_t(range * 2) - uint32_t(range));
    auto dy = int32_t(old_newrand(world.seed) % uint32_t(range * 2) - uint32_t(range));
    auto pos = uint16_t(cell_of(h) + (dy << 7) + dx);
    // The game reads past the map for cells off its end; here they hold no
    // litter.
    if (pos >= kMapCells || !has_litter(world.map[pos]) || world.map[pos].owner_flags != 1 << h.owner)
        return;
    auto dist = [&](uint16_t cell) {
        int x = cell % 128 - h.x.tile;
        int y = cell / 128 - h.y.tile;
        return uint32_t(x * x + y * y);
    };
    uint16_t nearest = h.work_target;
    if (pos == nearest || pos == h.work_target2)
        return;
    if (dist(pos) < dist(nearest)) {
        h.work_target2 = nearest;
        h.work_target = pos;
    } else if (dist(pos) < dist(h.work_target2)) {
        h.work_target2 = pos;
    }
}

// handyman_try_litter (0x7c6a0): sends the handyman to sweep litter on or
// beside his cell or, failing that, the litter he remembers nearest
// (forgetting it). Returns whether he set off.
bool handyman_try_litter(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    uint16_t target = 0;
    uint16_t pos = cell_of(h);
    for (int16_t offset : kSpecialAround) {
        auto cell = uint16_t(pos + offset);
        if (cell < kMapCells && has_litter(world.map[cell])) {
            target = cell;
            break;
        }
    }
    if (target == 0 && h.work_target) {
        if (has_litter(world.map[h.work_target]))
            target = h.work_target;
        h.work_target = h.work_target2;
        h.work_target2 = 0;
    }
    if (!target)
        return false;
    h.next_state = kStaffHandymanWorks;
    h.resume_state = kStaffHandymanWorks;
    set_complex(world, id, kComplexWalkTo, target);
    process(c, id);
    h.substate = 2;
    return true;
}

// handyman_works (0x7a8a8) in the corridor. Substate 0 decides: there are
// no pages for plants or repairs yet, so he sweeps litter he finds or else
// strolls to a random cell within 7 he can reach. Substates 2 and 3 sweep.
// Watering (4-6) and repairs (8-10) need pages, which nothing makes yet.
void handyman_works(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    switch (h.substate) {
    case 0: {
        if (shall_i_resign(world))
            break;
        // The repair pager's highest priority, 0xff with no pages, lets
        // handyman_try_fix look for one first: there is none. Then the
        // jobs, least done (for their priority) first: litter, plants and
        // repairs; plants and repairs have no pages.
        std::array<uint32_t, 3> scores = {uint32_t(h.litter_count * 100 / (h.prio_litter + 1)),
                                          uint32_t(h.plant_count * 1000 / (h.prio_plants + 1)),
                                          uint32_t(h.repair_count * 2000 / (h.prio_repair + 1))};
        for (;;) {
            uint32_t best = 2000000;
            size_t idx = 0;
            for (size_t n = 0; n < 3; n++) {
                if (scores[n] < best) {
                    best = scores[n];
                    idx = n;
                }
            }
            // The game tries again while a score is from 1000000 to 1999999,
            // never to end; here it gives up.
            if (best >= 1000000)
                break;
            scores[idx] = 2000000;
            if (idx == 0 && handyman_try_litter(c, id))
                return;
        }
        // Unsigned arithmetic into a signed int, as in the game.
        auto dx = int32_t(old_newrand(world.seed) % 15 - 7);
        auto dy = int32_t(old_newrand(world.seed) % 15 - 7);
        auto pos = uint16_t(cell_of(h) + (dy << 7) + dx);
        // The game reads past the map for cells off its end; here they are
        // not his land.
        if (pos < kMapCells && world.map[pos].owner_flags == 1 << h.owner && can_i_get_to(world, h, pos) &&
            !(world.map[pos].owner_flags & kCellOutdoor)) {
            h.next_state = h.resume_state = kStaffHandymanWorks;
            set_complex(world, id, kComplexWalkTo, pos);
            process(c, id);
        }
        break;
    }
    case 2:
        if (h.x.tile != h.dest_x || h.y.tile != h.dest_y) {
            h.substate = 0;
            break;
        }
        if (h.work_target == cell_of(h))
            h.work_target = 0;
        h.facing = kShowanimAngle[h.facing & 3];
        h.next_state = kStaffHandymanWorks;
        h.substate = 3;
        set_complex(world, id, kComplexSweepUp, 0);
        process(c, id);
        break;
    case 3: {
        // remove_litter (0x7a814); the hospital's litter count is not kept.
        MapCell &cell = world.map[cell_of(h)];
        if (cell.owner_flags == 1 << h.owner && has_litter(cell))
            cell.litter_shadow &= 0xf0;
        set_complex(world, id, kComplexAfterSweeping, 0);
        h.next_state = kStaffHandymanWorks;
        process(c, id);
        h.substate = 0;
        h.litter_count++;
        break;
    }
    default:
        break;
    }
}

// staff_goto_object (0x77508), with no blueprints: walks to the
// destination and runs the complex's next step there.
void staff_goto_object(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (e.subtype == kStaffHandyman)
        handyman_check_for_litter(world, e);
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0) {
        e.countdown = int16_t(0x100 / e.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
            process(c, id);
            return;
        case kNavBlockedByPeople:
            play_staff_anim(c.anims, e, kAnimStand);
            e.countdown = 0;
            return;
        case kNavWalledIn:
            staff_nav_failure(e);
            return;
        case kNavNoPath:
            if (e.resume_state)
                fail_staff_complex(world, c.anims, id);
            else
                staff_nav_failure(e);
            return;
        default: {
            size_t stand = size_t(kStaffAnimStarts[e.subtype][kAnimStand]) + e.facing;
            if (e.facing != dir || (stand < c.anims.starts.size() && c.anims.starts[stand].frame == e.sprite)) {
                e.facing = uint8_t(dir);
                play_staff_anim(c.anims, e, kAnimWalk);
            }
            break;
        }
        }
    }
    step_on(world, id);
}

// peep_stuck (0x788d8) for staff: stands for 100 turns, then until a path
// to the saved destination opens tries random directions, then resumes
// saved_state.
void staff_stuck(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0) {
        if (e.facing != 4) {
            e.countdown = 100;
            play_staff_anim(c.anims, e, kAnimStand);
            e.facing = 4;
            return;
        }
        uint16_t dest = uint16_t(e.saved_dest_x + (e.saved_dest_y << 7));
        OrthonavPath path;
        orthonav_init(world, path, e.x.tile, e.y.tile, dest % kMapSize, dest / kMapSize);
        if (path.count != 0) {
            e.state = e.saved_state;
            e.dest_x = e.saved_dest_x;
            e.dest_y = e.saved_dest_y;
            return;
        }
        auto dir = uint8_t(old_newrand(world.seed) & 3);
        if (!wall_in_way(world, e.x.tile, e.y.tile, dir)) {
            e.facing = dir;
            e.countdown = int16_t(0x100 / e.speed);
            play_staff_anim(c.anims, e, kAnimWalk);
        }
    }
    step_on(world, id);
}

// allocate_reception_roominfo (0x486a8): takes a free entry of
// World::rooms as a reception of `owner`. Returns it, 0 if there is none.
uint16_t allocate_reception_roominfo(World &world, uint8_t owner)
{
    for (uint16_t i = 1; i < world.rooms.size(); i++) {
        RoomInfo &room = world.rooms[i];
        if (room.type == kRoomFree) {
            room.type = kRoomReception;
            room.pos = 0x3fff;
            room.width = room.height = 1;
            room.state = 0;
            room.owner = owner;
            room.queue_capacity = 0x14;
            return i;
        }
    }
    return 0;
}

// receptionist_finds_reception (0x97ce0). Her status icons are not ported.
void receptionist_finds_reception(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    uint16_t desk = has_reception(world, r.owner) ? find_free_object_anywhere(world, id, kObjReceptionDesk) : 0;
    if (!desk) {
        staff_in_corridor(r, c.anims);
        return;
    }
    r.next_state = kStaffReceptionistSetup;
    r.object = desk;
    set_complex(world, id, kComplexGoToDesk, world.things[desk].use_pos[0]);
    r.resume_state = kStaffReceptionistFindsReception;
    process(c, id);
    r.slave = 0;
}

// receptionist_setup (0x97de0).
void receptionist_setup(World &world, uint16_t id)
{
    Thing &r = world.things[id];
    uint16_t room = 0;
    for (uint16_t o = r.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; o; o = world.things[o].type_obj_next) {
        const Thing &desk = world.things[o];
        if (o == r.object || desk.in_use == 0 || std::abs(desk.x.tile - r.x.tile) > 10 ||
            std::abs(desk.y.tile - r.y.tile) > 10)
            continue;
        if (desk.user) {
            room = world.things[desk.user].room;
            break;
        }
    }
    r.room = room ? room : allocate_reception_roominfo(world, r.owner);
    r.state = kStaffReceptionistWorks;
}

// receptionist_works (0x97fac). othervar.rand_pct, which it overwrites, is
// not kept. Her status icons are not ported.
void receptionist_works(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (r.room == 0 || c.game_turn % r.anim_speed != 0)
        return;
    r.sprite = c.anims.frames[r.sprite].next;
    if (!(c.anims.frames[r.sprite].flags & 1))
        return;
    uint16_t head = reception_queue_head(world, r.room);
    if (head) {
        uint16_t p = head;
        while (p && !reception_emergency(world, p))
            p = reception_queue_next(world, p);
        if (!p)
            p = head;
        for (; p; p = reception_queue_next(world, p)) {
            // The game keeps the capacity in a u16.
            if (uint16_t(all_queue_capacity(world, world.things[p].treatment, true, r.owner)) == 0 || !r.object)
                continue;
            uint16_t pos = world.things[r.object].use_pos[1];
            if (call_person_to_room(world, c.anims, c.person, id, p, kComplexToDesk, pos, false, c.game_turn)) {
                r.next_state = kStaffReceptWaitsForPatient;
                set_complex(world, id, kComplexCallPatient, 0);
                process(c, id);
                break;
            }
        }
        return;
    }
    if (old_newrand(world.seed) % 100 < 30) {
        r.next_state = kStaffReceptionistWorks;
        uint16_t action = kReceptTasks[old_newrand(world.seed) % kReceptTasks[0] + 1];
        set_complex(world, id, action, 0);
        process(c, id);
    } else {
        play_staff_anim(c.anims, r, 2);
        r.state = kStaffReceptionistWorks;
    }
}

// recept_waits_for_patient (0x982a8). othervar.recept_call_timer, which it
// clears, is not kept.
void recept_waits_for_patient(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (!r.slave) {
        r.state = kStaffReceptionistWorks;
        return;
    }
    Thing &patient = world.things[r.slave];
    if (patient.state == kPsArrivedFromCall) {
        set_complex(world, id,
                    reception_cure_available(world, r.slave) ? kComplexPatientCured : kComplexPatientNotCured, 0);
        patient.facing = kOppositeAngle[r.facing & 3];
        r.next_state = kStaffReceptAdmitsPatient;
        process(c, id);
    } else if (--r.timer <= 0) {
        if (!r.object)
            return;
        uint16_t pos = world.things[r.object].use_pos[1];
        uint8_t result = call_person_again(world, c.anims, c.person, id, kComplexToDesk, pos, c.game_turn);
        if (result == 0) {
            r.state = kStaffReceptionistWorks;
            r.slave = 0;
        } else if (result == 1) {
            r.state = kStaffReceptionistWorks;
            interrupt_person(world, c.anims, c.person, r.slave, c.game_turn);
            start_person_going(world, c.anims, c.person, r.slave, c.game_turn);
            r.slave = 0;
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexToDesk, c.game_turn)) {
        r.state = kStaffReceptionistWorks;
        r.slave = 0;
    }
}

// recept_admits_patient (0x984a0).
void recept_admits_patient(World &world, uint16_t id)
{
    Thing &r = world.things[id];
    if (!r.slave)
        return;
    reception_serves_patient(world, id, r.slave);
    RoomInfo &room = world.rooms[r.room];
    room.state = 0;
    room.patients_seen++;
    r.slave = 0;
    r.state = kStaffReceptionistWorks;
}
} // namespace

uint16_t staffrand(uint32_t &seed)
{
    seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(seed);
}

uint32_t normal_staffrand(uint32_t &seed, uint32_t low, uint32_t high)
{
    int32_t range = int32_t((high - low) / 2 + 1);
    if (range <= 0)
        range = 1;
    uint32_t sum = uint32_t(staffrand(seed) % range);
    sum += uint32_t(staffrand(seed) % range);
    return sum + low;
}

std::array<uint8_t, 3> reference_counts(const LanguageText &text)
{
    std::array<uint8_t, 3> counts{};
    const TextSection sections[3] = {TextSection::RefGeneral, TextSection::RefGood, TextSection::RefBad};
    for (size_t s = 0; s < 3; s++) {
        // The game's counter is a byte, and the list ends at a string starting
        // with '.'.
        uint8_t i = 0;
        while (i < text.size(sections[s]) && !text.at(sections[s], i).starts_with('.'))
            i++;
        counts[s] = i;
    }
    return counts;
}

int count_applicants(const World &world, uint8_t kind)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingStaff && t.subtype == kind && t.owner == kNoOwner &&
             (t.state == kStaffApplicant || t.state == kStaffHeld);
    return n;
}

void appraise_staff(Thing &staff, const Balance &settings)
{
    const Balance::Gbv &gbv = settings.gbv;
    int32_t value = staff.ability / gbv.salary_ability_divisor + settings.staff[staff.subtype].min_salary;
    // The game reads SalaryAdd[3..8] (gbv.salary_add, include/hosp.h).
    const uint16_t bits[6] = {kSkillJunior,       kSkillDoctor,     kSkillSurgeon,
                              kSkillPsychiatrist, kSkillConsultant, kSkillResearcher};
    for (size_t i = 0; i < 6; i++)
        if (staff.skills & bits[i])
            value += gbv.salary_add[i + 3];
    staff.wage_demand = value;
    int happiness = int16_t(staff.happiness);
    happiness += staff.salary < value ? gbv.salary_too_low : gbv.salary_too_high;
    staff.happiness = uint16_t(std::clamp(int(int16_t(happiness)), 0, 100));
}

uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn)
{
    uint16_t id = 0;
    for (size_t i = 1; i < world.things.size() - kStaffReserve; i++) {
        if (world.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    }
    if (!id)
        return 0;
    uint32_t &seed = market.seed;
    Thing &s = world.things[id];
    s.subtype = kind;
    s.type = kThingStaff;
    s.facing = 0;
    s.owner = kNoOwner;
    s.layers[5] = kLayer5Count[kind] ? uint8_t(staffrand(seed) % kLayer5Count[kind] + 1) : 0;
    for (size_t layer = 6; layer <= 9; layer++)
        s.layers[layer] = uint8_t(staffrand(seed) & 3);
    if (kind == kStaffNurse || kind == kStaffReceptionist)
        s.layers[5] = 1;
    s.hire_expiry = game_turn + 2500 + staffrand(seed) % 2500;
    s.work_delay = uint8_t(normal_staffrand(seed, 1, 10));
    s.happiness = 100;
    s.warmth = 50;
    s.state = kStaffApplicant;

    const Balance::Gbv &gbv = settings.gbv;
    uint32_t ability_min = 30, ability_max = 1000;
    switch (kind) {
    case kStaffNurse:
        s.skills = kSkillNurse;
        s.female = 1;
        s.face_head = 12;
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        break;
    case kStaffDoctor:
        ability_min = gbv.doctor_threshold;
        ability_max = gbv.consultant_threshold;
        s.skills |= kSkillDoctor;
        if (market.shrink_rate > 0 && staffrand(seed) % market.shrink_rate == 0)
            s.skills |= kSkillPsychiatrist;
        if (market.surgeon_rate > 0 && staffrand(seed) % market.surgeon_rate == 0)
            s.skills |= kSkillSurgeon;
        if (market.researcher_rate > 0 && staffrand(seed) % market.researcher_rate == 0)
            s.skills |= kSkillResearcher;
        if (market.consultant_rate > 0 && staffrand(seed) % market.consultant_rate == 0) {
            s.skills |= kSkillConsultant;
            ability_min = gbv.consultant_threshold;
            ability_max = 1000;
        } else if (market.junior_rate > 0 && staffrand(seed) % market.junior_rate == 0) {
            s.skills |= kSkillJunior;
            ability_min = 30;
            ability_max = gbv.doctor_threshold;
        }
        s.female = staffrand(seed) % 100 < 53;
        s.face_head = uint8_t(staffrand(seed) % 5);
        s.face_body = uint8_t(staffrand(seed) % 5);
        s.face_legs = uint8_t(staffrand(seed) % 5);
        if (s.layers[5] == 2) {
            s.face_legs += 5;
            s.face_body += 5;
            s.face_head += 5;
        }
        break;
    case kStaffHandyman:
        s.skills = kSkillHandyman;
        s.female = 0;
        s.prio_litter = 0x21;
        s.prio_plants = 0x21;
        s.prio_repair = 0x22;
        s.face_head = uint8_t(staffrand(seed) % 3 + 15);
        s.face_body = uint8_t(staffrand(seed) % 3 + 15);
        s.face_legs = uint8_t(staffrand(seed) % 3 + 15);
        break;
    case kStaffReceptionist:
        s.skills = kSkillReceptionist;
        s.female = 1;
        s.face_head = uint8_t(staffrand(seed) % 4 + 10);
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        if (s.face_head == 12)
            s.face_head = 14;
        break;
    }
    s.skills |= extra_skills;
    s.ability = uint16_t(normal_staffrand(seed, ability_min, ability_max));
    appraise_staff(s, settings);
    s.salary = s.wage_demand;
    s.initial = uint8_t(staffrand(seed) % 26 + 'A');
    s.surname_a = uint8_t(staffrand(seed) % 33);
    s.surname_b = uint8_t(staffrand(seed) % 26);
    // The game divides by the counts unchecked; a language without
    // references would crash it.
    s.ref_general = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[0], 1));
    s.ref_good = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[1], 1));
    s.ref_bad = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[2], 1));
    return id;
}

void create_job_market(World &world, StaffMarket &market, const Balance &settings, int game_month, uint32_t game_turn)
{
    for (const Balance::StaffLevel &level : settings.staff_levels) {
        if (level.month == game_month) {
            market.seed = level.seed;
            market.wanted = {level.nurses, level.doctors, level.handymen, level.receptionists};
            market.shrink_rate = level.shrk_rate;
            market.surgeon_rate = level.surg_rate;
            market.researcher_rate = level.rsch_rate;
            market.consultant_rate = level.cons_rate;
            market.junior_rate = level.jr_rate;
            break;
        }
        if (level.month > game_month)
            break;
    }
    for (uint8_t kind = 0; kind <= 3; kind++) {
        int have = count_applicants(world, kind);
        for (int j = have; j < market.wanted[kind]; j++)
            place_staff_in_struct(world, market, settings, kind, 0, game_turn);
    }
}

uint16_t staff_held_anim(uint8_t kind)
{
    const PersonAnims &a = kStaffAnims[kind];
    return kHeldAnim[kind] == 0 ? a.walk : a.stand;
}

bool staff_place_ok(const World &world, int cell)
{
    if (cell < 0 || cell >= kMapCells || world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    // check_points_connected to the door of the first entrance
    // (allhosps[player].entrances[0]).
    int door = world.entrances[kPlayer][0];
    OrthonavPath path;
    orthonav_init(world, path, cell % kMapSize, cell / kMapSize, door % kMapSize, door / kMapSize);
    return path.count != 0;
}

bool place_staff_on_map(World &world, const AnimData &anims, const Balance &settings, int cell, uint16_t id, bool force,
                        uint32_t game_turn)
{
    if (id == 0 || id >= world.things.size() || cell < 0 || cell >= kMapCells)
        return false;
    if (!force && world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    Thing &s = world.things[id];
    bool new_hire = s.owner == kNoOwner;
    s.owner = kPlayer;
    if (s.facing > 3)
        s.facing = 0;
    play_frame(s, anims, staff_held_anim(s.subtype));
    s.x = {0x80, uint8_t(cell % kMapSize)};
    s.y = {0x80, uint8_t(cell / kMapSize)};
    s.path.x1 = 0xff;
    // No rooms exist yet: which_room finds none, so the corridor's state.
    s.state = kCorridorStates[s.subtype];
    s.substate = 0;
    s.complex_action = 0;
    calculate_speed(s, settings);
    insert_node(world, id, s.x, s.y);
    // The game tests that no picked-up member of staff is held
    // (allhosps[owner].held_staff); picking staff up is not ported, so this
    // is a new hire. The hospital's staff counts and skills, nurses' and
    // handymen's work and the pager are not ported.
    if (new_hire) {
        s.icon_id = 0;
        s.hire_expiry = 0;
        s.hire_time = int16_t(game_turn);
    }
    return true;
}

void staff_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    const Ctx c{world, anims, person_complexes, staff_complexes, game_turn};
    Thing &s = world.things[id];
    if (staff_complex_turn(world, anims, person_complexes, staff_complexes, id, game_turn))
        return;
    switch (s.state) {
    case kStaffApplicant:
        // remove_staff_in_struct.
        if (game_turn > s.hire_expiry)
            s = Thing{};
        break;
    case kStaffInCorridor:
        staff_in_corridor(s, anims);
        break;
    case kStaffWanders:
        staff_wanders(c, id);
        break;
    case kStaffGotoObject:
        staff_goto_object(c, id);
        break;
    case kStaffStuck:
        staff_stuck(c, id);
        break;
    case kStaffHandymanWorks:
        handyman_works(c, id);
        break;
    case kStaffReceptionistFindsReception:
        receptionist_finds_reception(c, id);
        break;
    case kStaffReceptionistSetup:
        receptionist_setup(world, id);
        break;
    case kStaffReceptionistWorks:
        receptionist_works(c, id);
        break;
    case kStaffReceptWaitsForPatient:
        recept_waits_for_patient(c, id);
        break;
    case kStaffReceptAdmitsPatient:
        recept_admits_patient(world, id);
        break;
    default:
        break;
    }
}

bool apply_staff_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_MARK_EMPLOYING:
        // req2_mark_employing; the advisor's staff happiness total is not
        // ported.
        if (event.arg && event.arg < world.things.size())
            world.things[event.arg].state = kStaffHeld;
        return true;
    case EV_PLACE_STAFF:
        if (event.arg && event.arg < world.things.size()) {
            Thing &s = world.things[event.arg];
            if (place_staff_on_map(world, game.anims(), game.settings(), event.value, event.arg, false,
                                   game.turn_count))
                change_balance(game.finance, game.balance, game.date.day, game.date.month, kTxHireStaff,
                               -(s.salary * 12) / 10);
        }
        return true;
    case EV_CANCEL_BUILD:
        // req2_exit_employing.
        if (event.value && event.value < world.things.size())
            world.things[event.value].state = kStaffApplicant;
        return false;
    default:
        return false;
    }
}

} // namespace th
