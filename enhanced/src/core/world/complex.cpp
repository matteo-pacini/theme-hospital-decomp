#include "core/world/complex.h"

#include "core/data/gamedata.h"
#include "core/world/complex_tables.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/staff.h"

#include <cstdlib>

namespace th
{

namespace
{

// Object states (the game's OS_*, include/enums.h).
constexpr uint8_t kOsStatic = 0x02;
constexpr uint8_t kOsInUse = 0x04;
constexpr uint8_t kOsRemoved = 0x12;
constexpr uint8_t kOsSlidingDoorClosed = 0x14;
constexpr uint8_t kOsSlidingDoorOpens = 0x16;
constexpr uint8_t kOsSlidingDoorOpen = 0x18;
constexpr uint8_t kOsSlidingDoorCloses = 0x1a;

constexpr uint8_t kEntranceLeftDoor = 58;
// MapCell.flags: something on the cell covers more than one cell.
constexpr uint8_t kCellBig = 0x20;

// The complex abandon_complex runs to get a person off what it uses.
constexpr uint16_t kComplexAbandon = 0xf0;

// The animation columns of kPatientAnimStarts with a name in the game.
constexpr int kAnimWalk = 0;
constexpr int kAnimStand = 9;

// Object kinds (the game's OBJ_*).
constexpr uint8_t kObjBench = 4;
constexpr uint8_t kObjDrinksMachine = 7;
constexpr uint8_t kObjReceptionDesk = 11;

// RoomInfo.type of the toilets (the game's ROOM_TOILETS).
constexpr uint8_t kRoomToilets = 29;

// The complexes that get a patient off its object (interrupt_complex) and
// back to its queue (return_to_queue_area).
constexpr uint16_t kComplexGetOff = 160;
constexpr uint16_t kComplexReturnToQueue = 30;

// What each patient kind can do (the game's abilities, src/tables.cpp): [0]
// drink, [1] sit on a bench, [2] use the toilet; [3] to [5] are read
// elsewhere.
constexpr std::array<std::array<uint8_t, 6>, 15> kAbilities = {{
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {0},
    {0},
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
    {},
}};

// The cells find_standing_place tries, after their count, as offsets from
// the patient's cell (the game's standing_place, src/tables.cpp); the entry
// after the last is 0.
constexpr std::array<int16_t, 64> kStandingPlace = {
    62,   128,  1,    -128, -1,   129, -256, -2,   -127, -129, 2,    127,  256,  257,  126, -126,
    -384, -257, 384,  130,  -3,   255, -255, -130, 3,    383,  258,  -385, -131, -125, 254, 131,
    -258, -254, 385,  125,  -383, 512, -4,   4,    -512, 386,  -253, 382,  -259, 513,  132, 253,
    -513, -124, -386, 124,  259,  511, -511, 5,    -132, 640,  -5,   -640, -382, 0,
};

struct Run {
    World &world;
    const AnimData &anims;
    const PersonComplexes &complexes;
    uint32_t game_turn;
};

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// The settings the patients' states read: World::settings, or the game's
// static ones before start_queues.
const Balance &settings_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}

int heating_level_of(const World &world)
{
    return world.finance ? world.finance->heating_level : kStartHeatingLevel;
}

// which_room (0x2384c): the room whose floor holds `tile`, 0 for a
// corridor cell.
uint16_t which_room(const World &world, uint16_t tile)
{
    int x = tile % kMapSize;
    int y = tile / kMapSize;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.type == kRoomFree)
            continue;
        int x0 = room.pos % kMapSize;
        int y0 = room.pos / kMapSize;
        if (x0 <= x && x <= x0 + room.width - 1 && y0 <= y && y <= y0 + room.height - 1)
            return r;
    }
    return 0;
}

// can_i_get_to (0x19918): whether a path leads from (x, y) to `tile`.
bool path_exists(const World &world, int x, int y, uint16_t tile)
{
    OrthonavPath path;
    orthonav_init(world, path, x, y, tile % kMapSize, tile / kMapSize);
    return path.count != 0;
}

// The first room of hospital `owner` of `type` (listhead.rooms[type]), 0 if
// it has none.
uint16_t first_room(const World &world, uint8_t owner, uint8_t type)
{
    for (uint16_t r = 1; r < kRoomCount; r++)
        if (world.rooms[r].type == type && world.rooms[r].owner == owner)
            return r;
    return 0;
}

// find_object (0x3791c) with flag 0x96 (free objects only), for a cell in
// the corridor (rooms hold no objects yet): an object of `kind` of `owner`'s
// within 7 cells of `tile` on each axis whose use cell a path joins to it,
// the first or, if `random`, one drawn from world.seed among the first 30
// (a 31st found is taken at once). 0 if none.
uint16_t find_free_object_near(World &world, uint16_t tile, uint8_t kind, bool random, uint8_t owner)
{
    if (which_room(world, tile) || owner != 0)
        return 0;
    std::array<uint16_t, 30> found{};
    uint8_t count = 0;
    int x = tile % kMapSize;
    int y = tile / kMapSize;
    for (uint16_t o = world.first_of_kind[kind]; o; o = world.things[o].type_obj_next) {
        const Thing &obj = world.things[o];
        if (obj.in_use != 0 || std::abs(x - obj.x.tile) > 7 || std::abs(y - obj.y.tile) > 7)
            continue;
        if (!path_exists(world, obj.use_pos[1] % kMapSize, obj.use_pos[1] / kMapSize, tile) || obj.room_id)
            continue;
        if (!random || count == found.size())
            return o;
        found[count++] = o;
    }
    return count ? found[old_newrand(world.seed) % count] : 0;
}

void play_patient_anim(const AnimData &anims, Thing &p, int column)
{
    play_frame(p, anims, kPatientAnimStarts[p.subtype][size_t(column)]);
}

// The game's PLAY_FRAME after person_head_for: faces the step's direction,
// if any, and starts walking.
void head_off(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    int dir = person_head_for(r.world, id);
    if (dir >= 0 && dir < 4)
        p.facing = uint8_t(dir);
    play_patient_anim(r.anims, p, kAnimWalk);
}

} // namespace

uint16_t find_nearest_free_object(const World &world, uint16_t id, uint8_t kind)
{
    const Thing &seeker = world.things[id];
    // Only player 0's objects exist (World::first_of_kind).
    if (seeker.owner != 0)
        return 0;
    size_t side = 1;
    if (seeker.type == kThingStaff)
        side = seeker.subtype == kStaffHandyman ? 2 : 0;
    uint16_t best = 0;
    int best_dist = 9999;
    for (uint16_t o = world.first_of_kind[kind]; o; o = world.things[o].type_obj_next) {
        const Thing &obj = world.things[o];
        if (obj.in_use != 0)
            continue;
        int dx = std::abs(seeker.x.tile - obj.x.tile);
        int dy = std::abs(seeker.y.tile - obj.y.tile);
        if (dx > 7 || dy > 7)
            continue;
        uint16_t pos = obj.use_pos[side];
        OrthonavPath path;
        orthonav_init(world, path, obj.use_pos[1] % kMapSize, obj.use_pos[1] / kMapSize, pos % kMapSize,
                      pos / kMapSize);
        if (path.count != 0 && obj.room_id == 0 && dx + dy < best_dist) {
            best_dist = dx + dy;
            best = o;
        }
    }
    return best;
}

void adjust_big_cell_flag(World &world, uint16_t cell)
{
    MapCell &c = world.map[cell];
    for (int16_t i = c.first; i > 0; i = world.things[size_t(i)].cell_next) {
        const Thing &t = world.things[size_t(i)];
        if (t.type == kThingPatient || t.type == kThingStaff) {
            if (t.object && world.things[t.object].user == uint16_t(i)) {
                c.flags |= kCellBig;
                return;
            }
        } else if (t.type == kThingObject) {
            if (kShapes[kObjects[t.subtype].shapes[t.facing]].count > 1) {
                c.flags |= kCellBig;
                return;
            }
        }
    }
    c.flags &= uint8_t(~kCellBig);
}

namespace
{

// person_nav_failure (0x1752c), for a patient in no room heading for no
// room (there are none yet): back to queueing if it was to queue, else
// PS_STUCK. The game also announces a stuck patient of the player's.
void person_nav_failure(World &world, uint16_t id)
{
    Thing &p = world.things[id];
    if (p.next_state == kPsInQueue || p.next_state == kPsEntertainsThemself) {
        p.state = p.next_state;
        return;
    }
    p.saved_state = p.state;
    p.saved_dest_x = p.dest_x;
    p.saved_dest_y = p.dest_y;
    p.state = kPsStuck;
    p.countdown = 0;
}

uint8_t process(const Run &r, uint16_t id);
bool turn(const Run &r, uint16_t id);

// complex_success (0x423c8) for a patient: the effect of a step of action
// 52 of `kind`. The drinks sold go to World::drinks_income for hospital 0;
// the cans counted for the awards, the jukebox's takings and an object's
// use count (kind 24) are not kept. The staff's kinds do nothing here.
void complex_success(World &world, uint16_t id, uint16_t kind)
{
    Thing &w = world.things[id];
    const Balance::Gbv &gbv = settings_of(world).gbv;
    switch (kind) {
    case 2:
        w.refreshment = 100;
        w.happiness = uint16_t(w.happiness + gbv.drink_happy);
        add_litter_urge(w, 10);
        w.bladder = int16_t(w.bladder + 10);
        if (int16_t(w.warmth) > 60)
            w.warmth = 60;
        w.cash -= 20;
        if (w.owner == 0)
            world.drinks_income += 20;
        break;
    case 4:
        w.happiness++;
        w.cash -= 10;
        break;
    case 6:
        w.happiness = uint16_t(w.happiness + gbv.toilet_happy);
        w.bladder = w.illness == kIllTheSquits ? int16_t(gbv.bowel_full / 2) : int16_t(0);
        w.impatience = 0;
        w.waiting_time = 0;
        break;
    default:
        break;
    }
    if (int16_t(w.happiness) < 0)
        w.happiness = 0;
    else if (int16_t(w.happiness) > 100)
        w.happiness = 100;
}

// The game's PLAY_FRAME of patient_stand after a failed or abandoned
// complex.
void stand(const Run &r, Thing &p)
{
    play_patient_anim(r.anims, p, kAnimStand);
}

void fail(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    if (p.resume_state) {
        p.state = p.resume_state;
        p.complex_pc = p.complex_queued = p.complex_queued2 = 0;
        p.complex_action = p.complex_resume = 0;
        stand(r, p);
    } else {
        p.state = kPsComplexRetry;
    }
}

uint8_t process(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    bool run_things = false;
    if (p.complex_pc == 0)
        return 2;
    uint8_t op = r.complexes.at(p.complex_pc);
    if (op == 0) {
        p.complex_pc = 0;
        return 2;
    }
    uint16_t tile = cell_of(p);
    const ComplexStep &entry = kPersonComplex[op];
    // The game leaves this unset for a step of action 2 without an object
    // kind (person_complex[108], which no complex of P_COMP.LEX uses).
    uint16_t obj_id = 0;
    if (entry.object) {
        obj_id = find_nearest_free_object(world, id, entry.object);
        if (obj_id == 0) {
            fail(r, id);
            return 0;
        }
    }
    if (entry.state)
        p.state = entry.state;
    switch (entry.action) {
    case 2:
        p.object = obj_id;
        [[fallthrough]];
    case 44:
        set_destination_coords(world, id, world.things[p.object].use_pos[1]);
        head_off(r, id);
        p.countdown = 0;
        break;
    case 4:
        play_patient_anim(r.anims, p, entry.anim);
        break;
    case 6: {
        Thing &obj = world.things[p.object];
        if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject || obj.use_pos[1] != tile) {
            fail(r, id);
            return 3;
        }
        obj.in_use = 1;
        obj.user = id;
        p.facing = obj.facing;
        if (obj.use_style == 1 || obj.use_style == 2 || obj.use_style == 3) {
            p.exit_tile = obj.use_pos[4];
            move_node(world, id, obj.use_x, obj.use_y);
            obj.state = kOsInUse;
            p.depth = obj.depth;
            if (kObjectUse[obj.subtype].overlay_anim) {
                obj.saved_depth = obj.depth;
                obj.depth = 8;
            }
            world.map[cell_of(p)].flags |= kCellBig;
        }
        for (size_t i = 10; i < 13; i++)
            p.layers[i] = obj.layers[i];
        for (size_t i = 0; i < 5; i++)
            obj.layers[i] = p.layers[i];
        break;
    }
    case 8: {
        Thing &obj = world.things[p.object];
        obj.user = 0;
        p.object = 0;
        obj.in_use = 0;
        if (obj.use_style == 1 || obj.use_style == 3) {
            adjust_big_cell_flag(world, tile);
            move_node(world, id, {0x80, uint8_t(obj.use_pos[1] % kMapSize)},
                      {0x80, uint8_t(obj.use_pos[1] / kMapSize)});
            p.depth = 0;
            if (kObjectUse[obj.subtype].overlay_anim) {
                obj.depth = obj.saved_depth;
                obj.saved_depth = 0;
            }
            obj.state = kOsStatic;
        }
        stand(r, p);
        p.layers[10] = p.layers[11] = p.layers[12] = 0;
        break;
    }
    case 10:
        if (p.object)
            play_frame(world.things[p.object], r.anims, entry.anim);
        break;
    case 16: {
        const GroupAnim &group = kGroupAnims[entry.anim];
        uint32_t choice = old_newrand(world.seed) % uint32_t(group.count);
        play_patient_anim(r.anims, p, group.anims[choice]);
        break;
    }
    case 14:
        // Faces away from the door find_door finds: doors belong to rooms,
        // and there are none yet.
        break;
    case 20:
        // Takes the door find_door finds; there is none (see 14).
        fail(r, id);
        return 0;
    case 24:
        p.complex_pc++;
        p.complex_resume = uint16_t(r.complexes.at(p.complex_pc) * 10);
        break;
    case 32: {
        // transform_peep_who (0x16b30).
        int8_t kind = kChangePersonWho[entry.anim][p.subtype];
        p.orig_subtype = p.subtype;
        if (kind != -1) {
            p.subtype = uint8_t(kind);
            play_patient_anim(r.anims, p, kAnimWalk);
        }
        break;
    }
    case 34:
        p.subtype = p.orig_subtype;
        break;
    case 46:
        head_off(r, id);
        break;
    case 18:
        if (p.object) {
            const Thing &obj = world.things[p.object];
            int face = kObjectUse[obj.subtype].face_dirn;
            // Kinds whose face_dirn is -2 or -99 make the game read before
            // face_dirns; here they leave the facing.
            if (face >= 0 && size_t(face) < kFaceDirns.size()) {
                int8_t d = kFaceDirns[size_t(face)][size_t(obj.facing) + 4];
                if (d != -1) {
                    p.facing = uint8_t(d);
                    stand(r, p);
                }
            }
        }
        break;
    case 50:
        run_things = true;
        break;
    case 52:
        complex_success(world, id, entry.anim);
        break;
    default:
        // 0, 12 and 26 do nothing; 22 (leaving a door) is not ported, see
        // process_person_complex.
        break;
    }
    if (entry.repeat) {
        if (p.complex_repeat) {
            p.complex_repeat--;
            if (p.complex_repeat == 0)
                p.complex_pc++;
        } else if (entry.repeat < 0) {
            p.complex_repeat = uint8_t(old_newrand(world.seed) % uint32_t(-entry.repeat) + 1);
        } else {
            p.complex_repeat = uint8_t(entry.repeat);
        }
    } else {
        p.complex_pc++;
    }
    if (r.complexes.at(p.complex_pc) == 0) {
        p.complex_pc = 0;
        uint16_t action = p.complex_queued;
        if (action) {
            uint16_t arg = uint16_t(p.queued_arg_x + (p.queued_arg_y << 7));
            p.complex_queued = p.complex_queued2;
            p.complex_queued2 = 0;
            p.queued_arg_x = p.queued2_arg_x;
            p.queued2_arg_x = 0;
            p.queued_arg_y = p.queued2_arg_y;
            p.queued2_arg_y = 0;
            if (set_complex(world, id, action, arg))
                process(r, id);
        } else if (p.next_state) {
            p.state = p.next_state;
            p.next_state = 0;
        }
    } else if (entry.chain) {
        process(r, id);
    }
    // The game runs process_things for the person alone.
    if (run_things)
        turn(r, id);
    return 1;
}

// abandon_complex (0x41f84), for a patient: runs kComplexAbandon (queued
// behind a running complex) for up to 12 steps, each from its destination
// cell, puts it back where it was unless it uses its object or changed
// room, and stands it in resume_state.
void abandon_complex(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    MapPos old_x = p.x;
    MapPos old_y = p.y;
    bool user = p.object && world.things[p.object].user == id;
    set_complex(world, id, kComplexAbandon, 0);
    uint16_t room = p.room;
    for (int i = 0; i < 12; i++) {
        move_node(world, id, {0x80, p.dest_x}, {0x80, p.dest_y});
        if (process(r, id) != 1)
            break;
    }
    if (!user && room == p.room)
        move_node(world, id, old_x, old_y);
    p.state = p.resume_state;
    stand(r, p);
}

void remove(const Run &r, uint16_t id)
{
    leave_queue(r.world, id);
    abandon_complex(r, id);
    remove_node(r.world, id);
    r.world.things[id] = Thing{};
}

void go_home(const Run &r, uint16_t id)
{
    person_go_home(r.world, r.anims, r.complexes, id, r.game_turn);
}

void start_going(const Run &r, uint16_t id)
{
    start_person_going(r.world, r.anims, r.complexes, id, r.game_turn);
}

// Advances a person's animation every anim_speed turns, as the waiting
// states do; returns whether it reached the end of the animation.
bool animate(const Run &r, Thing &p)
{
    if (r.game_turn % p.anim_speed != 0)
        return false;
    p.sprite = r.anims.frames[p.sprite].next;
    return r.anims.frames[p.sprite].flags & 1;
}

// The game's PLAY_FRAME of a walk after person_head_for chose direction
// `dir` (0-3): a new direction, or setting off from the standing frame,
// starts the walk.
void face_step(const Run &r, Thing &p, int dir)
{
    size_t stand = size_t(kPatientAnims[p.subtype].stand) + p.facing;
    if (p.facing != dir || (stand < r.anims.starts.size() && r.anims.starts[stand].frame == p.sprite)) {
        p.facing = uint8_t(dir);
        play_patient_anim(r.anims, p, kAnimWalk);
    }
}

// Moves a walking person speed / 256 cell on in their facing. The game
// indexes around_xy with a stuck person's facing 4 too; here such a person
// does not move.
void step_on(World &world, uint16_t id)
{
    Thing &t = world.things[id];
    if (t.facing >= 4)
        return;
    uint16_t raw_x = uint16_t((t.x.tile << 8 | t.x.frac) + t.speed * kAroundXy[t.facing][0]);
    uint16_t raw_y = uint16_t((t.y.tile << 8 | t.y.frac) + kAroundXy[t.facing][1] * t.speed);
    move_node(world, id, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
}

// The needs and reaction of a walking patient deciding its step, as
// person_goto_object and person_goto_queue check them: true if it does not
// step this turn (dead, or reacting).
bool needs_stop_walk(const Run &r, uint16_t id)
{
    World &world = r.world;
    const Balance &settings = settings_of(world);
    modify_person(world, settings, heating_level_of(world), id, r.game_turn);
    if (world.things[id].health <= 0)
        return true;
    return do_reaction(world, r.anims, settings, id);
}

// person_goto_destination (0x763d0). Blueprints, which do not exist yet,
// are not stepped off.
void goto_destination(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        const Balance &settings = settings_of(world);
        if (person_needs_turn(world, r.anims, settings, heating_level_of(world), id, r.game_turn))
            return;
        p.countdown = int16_t(0x100 / p.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
            if (process(r, id) == 2) {
                p.state = kPsWaitsForBusy;
                p.timer = 200;
            }
            p.countdown = 0;
            return;
        case kNavBlockedByPeople:
            stand(r, p);
            p.countdown = 0;
            return;
        case kNavWalledIn:
        case kNavNoPath:
            person_nav_failure(world, id);
            return;
        default:
            face_step(r, p, dir);
            break;
        }
    }
    step_on(world, id);
}

// person_goto_object (0x76768): walks to the object of its complex; near it
// (within World::object_near_dist) an object in use or gone counts as
// reached, and the complex's next step (using it) fails. With no route, the
// step runs with the object taken as busy.
void goto_object(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (needs_stop_walk(r, id))
            return;
        p.countdown = int16_t(0x100 / p.speed);
        int dir = person_head_for(world, id);
        if (dir >= 0 && dir <= 3 && std::abs(p.x.tile - p.dest_x) < world.object_near_dist &&
            std::abs(p.y.tile - p.dest_y) < world.object_near_dist) {
            const Thing &obj = world.things[p.object];
            if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject)
                dir = kNavArrived;
        }
        switch (dir) {
        case kNavArrived:
            process(r, id);
            p.countdown = 0;
            return;
        case kNavBlockedByPeople:
            stand(r, p);
            p.countdown = 0;
            return;
        case kNavWalledIn:
            person_nav_failure(world, id);
            return;
        case kNavNoPath:
            if (p.object) {
                Thing &obj = world.things[p.object];
                uint8_t in_use = obj.in_use;
                obj.in_use = 1;
                process(r, id);
                obj.in_use = in_use;
            } else {
                person_nav_failure(world, id);
            }
            return;
        default:
            face_step(r, p, dir);
            break;
        }
    }
    step_on(world, id);
}

// person_goto_queue (0x76c44): walks to the queue of dest_room; within
// World::queue_near_dist of the destination (of a reception, or of a room
// with a queue already) it joins the queue: PS_IN_QUEUE at a reception,
// PS_ARRIVES_AT_DOOR at a room, first walking on to the room's door if that
// is further. Sent on again (start_person_going) if the room is not one for
// its treatment or has no door. Staff are not paged.
void goto_queue(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (needs_stop_walk(r, id))
            return;
        p.countdown = int16_t(0x100 / p.speed);
        uint16_t room_id = p.dest_room;
        RoomInfo &room = world.rooms[room_id];
        int near = world.queue_near_dist;
        int dir = person_head_for(world, id);
        if (dir >= 0 && dir <= 3 && std::abs(p.x.tile - p.dest_x) <= near && std::abs(p.y.tile - p.dest_y) <= near &&
            (room.type == kRoomReception || queue_size(world, room_id) > 0))
            dir = kNavArrived;
        switch (dir) {
        case kNavArrived:
            if (room.type != kRoomReception) {
                if (room.type != kExpertiseCategory[p.treatment] || room.door_tile == 0) {
                    if (p.dest_room && room.door_tile != 0)
                        room.queue_incoming--;
                    p.dest_room = 0;
                    p.complex_pc = p.complex_queued = p.complex_queued2 = 0;
                    start_going(r, id);
                    return;
                }
                if (std::abs(p.x.tile - room.door_tile % kMapSize) > near ||
                    std::abs(p.y.tile - room.door_tile / kMapSize) > near) {
                    set_destination_coords(world, id, room.door_tile);
                    p.countdown = 0;
                    return;
                }
            }
            p.dest_room = 0;
            p.complex_pc = p.complex_queued = p.complex_queued2 = 0;
            join_queue(world, id, room_id);
            if (room.type != kRoomReception) {
                p.state = kPsArrivesAtDoor;
            } else {
                p.state = kPsInQueue;
                stand(r, p);
            }
            p.countdown = 0;
            return;
        case kNavBlockedByPeople:
            stand(r, p);
            p.countdown = 0;
            return;
        case kNavWalledIn:
        case kNavNoPath:
            person_nav_failure(world, id);
            return;
        default:
            face_step(r, p, dir);
            break;
        }
    }
    step_on(world, id);
}

// find_standing_place (0x164f4): a patient sharing its cell with someone
// ahead of it in the cell's list, or standing on an object's cells, walks
// (complex kComplexStandingPlace, then back to its state) to the first free,
// reachable indoor cell of kStandingPlace from a random one of the first 12
// on. Returns whether it was sent. Cells off the map, which the game reads
// past its map, are skipped.
bool find_standing_place(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (p.cell_prev == 0 && !(world.map[cell_of(p)].flags & 7))
        return false;
    int base = p.x.tile + (p.y.tile << 7);
    uint16_t tile = 0;
    size_t i = old_newrand(world.seed) % 12 + 1;
    for (; i <= size_t(kStandingPlace[0]); i++) {
        tile = uint16_t(base + kStandingPlace[i]);
        if (tile >= kMapCells)
            continue;
        const MapCell &target = world.map[tile];
        if (target.first == 0 && !(target.flags & 7) && path_exists(world, p.x.tile, p.y.tile, tile) &&
            !(target.owner_flags & kCellOutdoor))
            break;
    }
    if (kStandingPlace[i] == 0)
        return false;
    p.resume_state = p.state;
    p.next_state = p.state;
    set_complex(world, id, kComplexStandingPlace, tile);
    process(r, id);
    return true;
}

// Starts complex `action` (a drink or a bench) to come back from to
// PS_ENTERTAINS_THEMSELF.
void start_pastime(const Run &r, uint16_t id, uint16_t action)
{
    Thing &p = r.world.things[id];
    p.resume_state = kPsEntertainsThemself;
    p.next_state = kPsEntertainsThemself;
    set_complex(r.world, id, action, 0);
    process(r, id);
}

// person_in_queue (0x15d48) and, with `entertaining`, person_entertains_themself
// (0x160a8): at the end of each animation, once the countdown runs out,
// the patient's needs (modify_person); then, alive, it goes home fed up
// past gbv.leave_max impatience (entertaining only), fetches a drink when
// thirsty, sits on a free bench near by (the bench shortage counted down 5,
// or up 1 with none), or stands, moving off a crowded cell. A patient in
// the queue moves on to PS_ENTERTAINS_THEMSELF. Staff are not paged and the
// toilets, which cannot be built, are not looked for.
void wait_in_queue(const Run &r, uint16_t id, bool entertaining)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!animate(r, p) || --p.countdown > 0)
        return;
    const Balance &settings = settings_of(world);
    modify_person(world, settings, heating_level_of(world), id, r.game_turn);
    if (p.health <= 0)
        return;
    if (entertaining && (p.impatience > settings.gbv.leave_max || (p.person_flags & kPfSentHomeByInspector))) {
        if (p.owner == 0)
            world.patients_left_count++;
        go_home(r, id);
        p.person_flags |= kPfFedUp;
        return;
    }
    const auto &can = kAbilities[p.subtype];
    if (can[0] && p.refreshment < 30 && p.complex_action != kComplexDrink &&
        find_free_object_near(world, cell_of(p), kObjDrinksMachine, true, p.owner)) {
        start_pastime(r, id, kComplexDrink);
        return;
    }
    if (p.complex_action != kComplexBench) {
        if (can[1]) {
            if (find_nearest_free_object(world, id, kObjBench)) {
                if (p.owner == 0)
                    world.bench_shortage -= 5;
                start_pastime(r, id, kComplexBench);
                return;
            }
            if (p.owner == 0)
                world.bench_shortage++;
        }
        stand(r, p);
        if (find_standing_place(r, id))
            return;
    }
    p.countdown = int16_t(0x100 / p.speed);
    if (!entertaining)
        p.state = kPsEntertainsThemself;
}

// return_to_queue_area (0x179f8): back to the queue it is in (to its
// hospital's first reception desk, or the room's door), entertaining itself
// there.
void return_to_queue_area(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (p.queue_room) {
        const RoomInfo &room = world.rooms[p.queue_room];
        p.resume_state = kPsEntertainsThemself;
        p.next_state = kPsEntertainsThemself;
        uint16_t arg = 0;
        if (room.type == kRoomReception) {
            if (uint16_t desk = p.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0)
                arg = world.things[desk].use_pos[1];
        } else {
            arg = room.door_tile;
        }
        if (arg) {
            set_complex(world, id, kComplexReturnToQueue, arg);
            process(r, id);
            return;
        }
    }
    p.state = kPsEntertainsThemself;
}

// get_expertise_dest (0x49e88): of hospital `owner`'s rooms with a door for
// treatment `type`, the one with queue space scoring least by its queue
// (gbv.q_points), distance (gbv.dist_points) and staff (gbv.no_staff_points
// unless a member of staff is in it), else the last such room; 0 if the
// hospital cannot give the treatment.
uint16_t get_expertise_dest(const World &world, uint16_t type, const Thing &patient)
{
    const Balance::Gbv &gbv = settings_of(world).gbv;
    uint8_t kind = kExpertiseCategory[type];
    uint16_t best = 0;
    uint16_t fallback = 0;
    uint32_t best_score = 9999;
    if (patient.owner != 0 || world.cure_available[type] != 1 || !first_room(world, patient.owner, kind))
        return 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &room = world.rooms[r];
        if (room.owner != patient.owner || room.type != kind || room.door_tile == 0)
            continue;
        fallback = r;
        if (!spare_queue_capacity(world, 0, true, r, patient.owner))
            continue;
        uint32_t score = room.queue_length * gbv.q_points;
        score += uint32_t((std::abs(patient.x.tile - room.door_tile % kMapSize) +
                           std::abs(patient.y.tile - room.door_tile / kMapSize)) *
                          gbv.dist_points);
        score += gbv.no_staff_points;
        for (const Thing &t : world.things) {
            if (t.type == kThingStaff && t.room == r) {
                score -= gbv.no_staff_points;
                break;
            }
        }
        if (score < best_score) {
            best_score = score;
            best = r;
        }
    }
    return best ? best : fallback;
}

// person_go_for_treatment (0x3d91c): the patient pays or not
// (shall_peep_pay); without the treatment (or paying for it) it goes home
// through the first entrance; with no room for it, it waits for a decision;
// else it walks to the queue of the room for it (get_expertise_dest),
// counted on its way there. The reputation lost for a treatment the
// hospital cannot give and paging staff are not ported; wards and research,
// which cannot be built, are not handled.
void go_for_treatment(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint16_t treatment = p.treatment;
    if (!shall_peep_pay(world, id))
        treatment = 0;
    uint8_t category = kExpertiseCategory[treatment];
    if (p.person_flags & kPfEmergency) {
        if (!first_room(world, p.owner, category))
            treatment = 0;
    } else if (p.owner != 0 || world.cure_available[treatment] == 0) {
        treatment = 0;
    }
    category = kExpertiseCategory[treatment];
    if (treatment != 0 && !first_room(world, p.owner, category)) {
        p.person_flags |= kPfWaitingForRoom;
        p.timer = int16_t(r.game_turn);
        set_complex(world, id, kComplexAbandon, 0);
        p.next_state = kPsWaitsForDecision;
        process(r, id);
        return;
    }
    switch (treatment) {
    case 0:
        play_patient_anim(r.anims, p, kAnimWalk);
        set_complex(world, id, kComplexLeave, world.entrances[p.owner & 3][0]);
        process(r, id);
        break;
    case kIllDiagWard:
    case kIllAutopsy:
        break;
    default: {
        uint16_t room_id = world.cure_available[treatment] == 1 ? get_expertise_dest(world, treatment, p)
                                                                : first_room(world, p.owner, category);
        RoomInfo &room = world.rooms[room_id];
        room.queue_incoming++;
        p.dest_room = room_id;
        play_patient_anim(r.anims, p, kAnimWalk);
        p.state = kPsGotoQueue;
        set_destination_coords(world, id, room.door_tile);
        break;
    }
    }
}

// person_waits_for_decision (0x17e30) for a patient waiting for a room for
// its treatment: once the hospital can give the treatment and has a room
// for it, it goes for it; 1500 turns after it began waiting, it goes home.
// Meanwhile it stands, moving off a crowded cell. The player's call about
// it and a patient waiting for a diagnosis room (diagnosis is not ported)
// are not handled.
void waits_for_decision(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!animate(r, p) || --p.countdown > 0)
        return;
    if (!(p.person_flags & kPfNeedsDiagnosisRoom)) {
        uint8_t treatment = p.next_treatment ? p.next_treatment : p.treatment;
        bool done = p.owner == 0 && world.cure_available[treatment] == 1 &&
                    first_room(world, p.owner, kExpertiseCategory[treatment]);
        if (done || uint16_t(uint16_t(r.game_turn) - uint16_t(p.timer)) >= 1500) {
            p.person_flags &= ~kPfAwaitingDecision;
            if (p.person_flags & kPfWaitingForRoom) {
                p.person_flags &= ~kPfWaitingForRoom;
                if (!done) {
                    go_home(r, id);
                    p.person_flags |= kPfNoTreatment;
                } else {
                    p.state = kPsGoForTreatment;
                }
            } else {
                go_home(r, id);
                p.person_flags |= kPfNoTreatment;
            }
            return;
        }
    }
    stand(r, p);
    if (find_standing_place(r, id))
        return;
    p.countdown = int16_t(0x100 / p.speed);
}

// peep_stuck (0x788d8) for a patient: stands for 100 turns, then, until a
// path to its saved destination opens (never once fed up), grows 15 more
// impatient each try, going home fed up past gbv.leave_max, and tries a
// random direction.
void stuck(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (p.facing != 4) {
            p.countdown = 100;
            stand(r, p);
            p.facing = 4;
            return;
        }
        uint16_t dest = uint16_t(p.saved_dest_x + (p.saved_dest_y << 7));
        if (!(p.person_flags & kPfFedUp) && path_exists(world, p.x.tile, p.y.tile, dest)) {
            p.state = p.saved_state;
            p.dest_x = p.saved_dest_x;
            p.dest_y = p.saved_dest_y;
            return;
        }
        p.impatience = uint16_t(p.impatience + 0xf);
        if (p.impatience > settings_of(world).gbv.leave_max) {
            p.impatience = 0;
            if (!(p.person_flags & kPfFedUp) && p.owner == 0)
                world.patients_left_count++;
            go_home(r, id);
            p.person_flags |= kPfFedUp;
            return;
        }
        auto dir = uint8_t(old_newrand(world.seed) & 3);
        if (!wall_in_way(world, p.x.tile, p.y.tile, dir)) {
            p.facing = dir;
            p.countdown = int16_t(0x100 / p.speed);
            play_patient_anim(r.anims, p, kAnimWalk);
        }
    }
    step_on(world, id);
}

bool turn(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (p.state) {
    case kPsWaitsForBusy:
        // person_waits_for_busy (0x15c50).
        if (r.game_turn % p.anim_speed == 0)
            p.sprite = r.anims.frames[p.sprite].next;
        if (p.timer-- <= 0)
            go_home(r, id);
        break;
    case kPsGotoDestination:
        goto_destination(r, id);
        break;
    case kPsReturnToQueueArea:
        return_to_queue_area(r, id);
        break;
    case kPsGoHome:
        go_home(r, id);
        break;
    case kPsGone:
        // person_gone (0x1673c).
        remove(r, id);
        break;
    case kPsPlayComplexAnim:
        // play_complex_person_anim (0x418dc).
        if (r.game_turn % p.anim_speed == 0) {
            uint16_t next = r.anims.frames[p.sprite].next;
            if (r.anims.frames[next].flags & 1)
                process(r, id);
            else
                p.sprite = next;
        }
        break;
    case kPsArrivesAtDoor:
        // person_arrives_at_door (0x15cc0): rooms with doors cannot be
        // built yet, so it queues.
        p.state = kPsInQueue;
        break;
    case kPsInQueue:
        wait_in_queue(r, id, false);
        break;
    case kPsGotoObject:
        goto_object(r, id);
        break;
    case kPsEntertainsThemself:
        wait_in_queue(r, id, true);
        break;
    case kPsGotoQueue:
        goto_queue(r, id);
        break;
    case kPsGoForTreatment:
        go_for_treatment(r, id);
        break;
    case kPsComplexRetry:
        // person_complex_retry (0x41a54).
        if (r.game_turn % p.anim_speed == 0) {
            p.sprite = r.anims.frames[p.sprite].next;
            if (r.anims.frames[p.sprite].flags & 1)
                process(r, id);
        }
        break;
    case kPsShowanim:
        person_showanim_turn(world, r.anims, id, r.game_turn);
        break;
    case kPsStuck:
        stuck(r, id);
        break;
    case kPsWaitsForDecision:
        waits_for_decision(r, id);
        break;
    case kPsArrivedFromCall:
        // person_arrived_from_call (0x18f88) acts in a room only; at a
        // reception desk the receptionist takes it from here.
        break;
    default:
        break;
    }
    return world.things[id].type == kThingPatient;
}

// should_sliding_door_open (0x39a90).
bool someone_at_door(const World &world, const Thing &door)
{
    int offset = kSlidingDoors[door.subtype - kEntranceLeftDoor].offset[door.facing];
    for (size_t use : {1, 4}) {
        int cell = door.use_pos[use] + offset;
        if (cell < 0 || cell >= kMapCells)
            continue;
        for (int16_t i = world.map[size_t(cell)].first; i > 0; i = world.things[size_t(i)].cell_next) {
            uint8_t type = world.things[size_t(i)].type;
            // The game also opens for the mayor and the inspector.
            if (type == kThingPatient || type == kThingStaff)
                return true;
        }
    }
    return false;
}

} // namespace

bool load_person_complexes(const std::filesystem::path &dir, PersonComplexes &out, std::string *error)
{
    auto data = load_data_file(dir, "data/p_comp.lex", error);
    if (!data)
        return false;
    for (uint8_t op : *data) {
        if (op >= kPersonComplex.size()) {
            if (error)
                *error = "data/p_comp.lex: unknown step " + std::to_string(op);
            return false;
        }
    }
    out.group = std::move(*data);
    return true;
}

bool set_complex(World &world, uint16_t id, uint16_t action, uint16_t arg)
{
    Thing &w = world.things[id];
    if (w.complex_pc == 0) {
        if (w.complex_resume) {
            w.complex_pc = w.complex_resume;
            w.complex_action = action;
            if (w.complex_queued) {
                w.complex_queued2 = action;
                w.queued2_arg_x = uint8_t(arg % kMapSize);
                w.queued2_arg_y = uint8_t(arg / kMapSize);
            } else {
                w.complex_queued = action;
                w.queued_arg_x = uint8_t(arg % kMapSize);
                w.queued_arg_y = uint8_t(arg / kMapSize);
            }
            w.complex_resume = 0;
        } else {
            w.complex_pc = w.complex_action = action;
            if (arg)
                set_destination_coords(world, id, arg);
        }
        return true;
    }
    if (w.complex_queued) {
        w.complex_queued2 = action;
        if (arg) {
            w.queued2_arg_x = uint8_t(arg % kMapSize);
            w.queued2_arg_y = uint8_t(arg / kMapSize);
        }
        return false;
    }
    w.complex_queued = w.complex_action = action;
    if (arg) {
        w.queued_arg_x = uint8_t(arg % kMapSize);
        w.queued_arg_y = uint8_t(arg / kMapSize);
    }
    return false;
}

uint8_t process_person_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                               uint32_t game_turn)
{
    return process({world, anims, complexes, game_turn}, id);
}

void fail_person_complex(World &world, const AnimData &anims, uint16_t id)
{
    static const PersonComplexes none;
    fail({world, anims, none, 0}, id);
}

uint16_t find_nearest_manned_reception(const World &world, uint16_t id, uint8_t room, bool need_space)
{
    const Thing &person = world.things[id];
    const Balance::Gbv &gbv = settings_of(world).gbv;
    uint16_t best = 0;
    int32_t best_score = 9999;
    for (uint16_t d = person.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; d;
         d = world.things[d].type_obj_next) {
        const Thing &desk = world.things[d];
        if (!desk.user)
            continue;
        uint16_t staff_room = world.things[desk.user].room;
        if (!staff_room || !(room == 0 || staff_room == room))
            continue;
        const RoomInfo &reception = world.rooms[staff_room];
        int32_t free = reception.queue_capacity - reception.queue_length;
        if (free <= 0 && need_space)
            continue;
        int32_t score = reception.queue_length * gbv.q_points;
        score += (std::abs(desk.x.tile - person.x.tile) + std::abs(desk.y.tile - person.y.tile)) * gbv.dist_points;
        score -= free;
        if (score < best_score) {
            best_score = score;
            best = d;
        }
    }
    return best;
}

void start_person_going(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn)
{
    Thing &p = world.things[id];
    bool go = false;
    if (uint16_t d = find_nearest_manned_reception(world, id, 0, true)) {
        const Thing &desk = world.things[d];
        p.dest_room = world.things[desk.user].room;
        go = set_complex(world, id, kComplexToReception, desk.use_pos[1]);
    } else {
        uint16_t arg = world.entrances[p.owner & 3][old_newrand(world.seed) & 3];
        go = set_complex(world, id, kComplexLeave, arg);
    }
    if (go)
        process({world, anims, complexes, game_turn}, id);
}

void person_go_home(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                    uint32_t game_turn)
{
    Thing &p = world.things[id];
    p.next_state = 0;
    interrupt_person(world, anims, complexes, id, game_turn);
    leave_queue(world, id);
    if (set_complex(world, id, kComplexLeave, world.entrances[p.owner & 3][0]))
        process({world, anims, complexes, game_turn}, id);
}

bool person_ok_to_call(const World &world, uint16_t id)
{
    const Thing &p = world.things[id];
    if (p.room)
        return world.rooms[p.room].type != kRoomToilets;
    return p.next_state != kPsInBogs;
}

void interrupt_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                       uint32_t game_turn)
{
    Thing &w = world.things[id];
    w.complex_pc = w.complex_queued = w.complex_queued2 = w.complex_resume = 0;
    w.next_state = 0;
    w.resume_state = 0;
    w.x.frac = w.y.frac = 0x80;
    w.countdown = 0;
    if (w.object && world.things[w.object].type == kThingObject && world.things[w.object].user == id) {
        set_complex(world, id, kComplexGetOff, 0);
        process({world, anims, complexes, game_turn}, id);
        w.speed = kPeepSpeed;
        w.anim_speed = kPeepAnimSpeed;
    }
}

void remove_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                   uint32_t game_turn)
{
    remove({world, anims, complexes, game_turn}, id);
}

bool person_turn(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id, uint32_t game_turn)
{
    return turn({world, anims, complexes, game_turn}, id);
}

void sliding_door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &door = world.things[id];
    if (door.type != kThingObject)
        return;
    switch (door.state) {
    case kOsSlidingDoorClosed:
        // sliding_door_closed (0x39704).
        if (someone_at_door(world, door))
            door.state = kOsSlidingDoorOpens;
        break;
    case kOsSlidingDoorOpens:
        // sliding_door_opens (0x39734).
        if (game_turn % door.anim_speed == 0) {
            uint16_t next = anims.frames[door.sprite].next;
            if (anims.frames[next].flags & 1) {
                door.countdown = 50;
                door.state = kOsSlidingDoorOpen;
            } else {
                door.sprite = next;
            }
        }
        break;
    case kOsSlidingDoorOpen:
        // sliding_door_open (0x397b4).
        if (someone_at_door(world, door)) {
            door.countdown = 50;
        } else if (--door.countdown == 0) {
            play_frame(door, anims, kSlidingDoors[door.subtype - kEntranceLeftDoor].close_anim);
            door.state = kOsSlidingDoorCloses;
        }
        break;
    case kOsSlidingDoorCloses: {
        // sliding_door_closes (0x39880).
        const SlidingDoorInfo &info = kSlidingDoors[door.subtype - kEntranceLeftDoor];
        if (someone_at_door(world, door)) {
            // As far into the opening animation as the closing one had left
            // to go; uint8_t as in the game, so 0 left runs 255 frames.
            uint8_t steps = 0;
            while (!(anims.frames[door.sprite].flags & 1)) {
                door.sprite = anims.frames[door.sprite].next;
                steps++;
            }
            play_frame(door, anims, info.open_anim);
            for (steps--; steps != 0; steps--)
                door.sprite = anims.frames[door.sprite].next;
            door.state = kOsSlidingDoorOpens;
        } else if (game_turn % door.anim_speed == 0) {
            uint16_t next = anims.frames[door.sprite].next;
            if (anims.frames[next].flags & 1) {
                play_frame(door, anims, info.open_anim);
                door.state = kOsSlidingDoorClosed;
            } else {
                door.sprite = next;
            }
        }
        break;
    }
    default:
        break;
    }
}

} // namespace th
