#include "core/world/complex.h"

#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/ui/charts.h"
#include "core/world/complex_tables.h"
#include "core/world/counts.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reaper.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/staff_complex.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

namespace
{

// Object states (the game's OS_*, include/enums.h).
constexpr uint8_t kOsStatic = 0x02;
constexpr uint8_t kOsInUse = 0x04;
constexpr uint8_t kOsDoubleDoorSwings = 0x10;
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

// The patient kind of an alien (PK_ALIEN).
constexpr uint8_t kPatientAlien = 0xe;

// RoomInfo.type of the toilets (the game's ROOM_TOILETS).
constexpr uint8_t kRoomToilets = 29;
// A ward, and its beds.
constexpr uint8_t kRoomWard = 9;
constexpr uint8_t kObjBed = 8;
// The complex that ends what a patient does where it stands (getting out
// of bed).
constexpr uint16_t kComplexStandStill = 240;

// The complexes of the toilets: to their door and in, to a toilet and to a
// sink.
constexpr uint16_t kComplexToToilets = 360;
constexpr uint16_t kComplexToilet = 330;
constexpr uint16_t kComplexSink = 350;
constexpr uint8_t kObjToilet = 51;
constexpr uint8_t kObjSink = 32;

// The complexes that get a patient off its object (interrupt_complex) and
// back to its queue (return_to_queue_area).
constexpr uint16_t kComplexGetOff = 160;
constexpr uint16_t kComplexReturnToQueue = 30;
// The complexes of rooms: at a single door, turned away from it to queue;
// out of the room by its door; getting off a door (interrupt_complex).
constexpr uint16_t kComplexAtDoor = 50;
constexpr uint16_t kComplexLeaveRoom = 90;
constexpr uint16_t kComplexOffDoor = 0x26c;

// Object kinds and the animation columns of kPatientAnimStarts doors use:
// through a single door (OBJ_DOOR) or a double one.
constexpr uint8_t kObjDoor = 3;
constexpr uint8_t kObjSwingDoor2 = 53;
constexpr int kAnimSingleDoor = 7;
constexpr int kAnimDoubleDoor = 67;
// The starts of a double door's swing as it opens (by the facing of who
// opens it) and as it closes, and its closed frame (OBJ_SWING_DOOR2 and the
// other half).
constexpr uint16_t kAnimSwingOpen = 0x3f8;
constexpr uint16_t kAnimSwingClose = 0x400;
constexpr uint16_t kAnimSwingDoor2Closed = 0x3eb;
constexpr uint16_t kAnimSwingDoorClosed = 0x3e6;
// MapCell.flags a door in use sets on its cell.
constexpr uint8_t kCellDoorInUse = 0x10;

// RoomInfo.state of a room being furnished or edited (RS_EDITED).
constexpr int8_t kRoomEdited = 4;

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

// listhead[owner].rooms[type]: hospital `owner`'s room of `type` once its
// doors are committed (World::room_of_type, hospital 0's), 0 if none.
uint16_t first_room(const World &world, uint8_t owner, uint8_t type)
{
    return owner == 0 ? world.room_of_type[type % world.room_of_type.size()] : 0;
}

// Whether the patient stands on a blueprint's floor and was sent off it
// (move_peep_from_blueprint), as the walking and waiting states check
// first.
bool off_blueprint(World &world, uint16_t id)
{
    const Thing &t = world.things[id];
    return block_flags(world.map[uint16_t(t.x.tile + (t.y.tile << 7))].floor_block) == 0x20 &&
           move_peep_from_blueprint(world, id);
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

uint16_t find_nearest_free_object(World &world, uint16_t id, uint8_t kind)
{
    const Thing &seeker = world.things[id];
    if (uint16_t room = which_room(world, cell_of(seeker))) {
        std::array<uint16_t, 30> found{};
        uint8_t count = 0;
        for (uint16_t o = world.rooms[room].first_object; o; o = world.things[o].room_obj_next) {
            const Thing &obj = world.things[o];
            if (obj.subtype != kind || obj.in_use != 0)
                continue;
            if (count >= found.size())
                return o;
            found[count++] = o;
        }
        return count ? found[old_newrand(world.seed) % count] : 0;
    }
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

uint8_t process(const Run &r, uint16_t id);
bool turn(const Run &r, uint16_t id);
void start_going(const Run &r, uint16_t id);

// person_nav_failure (0x1752c): heading from the corridor for a room built
// in the last 500 turns, sent on again (start_person_going); else back to
// queueing if it was to queue, or PS_STUCK, announced to the advisor
// (World::announce_stuck) for a patient of hospital 0's.
void person_nav_failure(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("nav_failure %u", unsigned(id));
    if (p.room == 0) {
        uint16_t room = which_room(world, uint16_t(p.dest_x + (p.dest_y << 7)));
        if (room && r.game_turn - 500 < world.rooms[room].created_turn) {
            start_going(r, id);
            return;
        }
    }
    if (p.next_state == kPsInQueue || p.next_state == kPsEntertainsThemself) {
        p.state = p.next_state;
        return;
    }
    p.saved_state = p.state;
    p.saved_dest_x = p.dest_x;
    p.saved_dest_y = p.dest_y;
    p.state = kPsStuck;
    p.countdown = 0;
    if (p.owner == 0)
        world.announce_stuck = true;
}

} // namespace

// complex_success (0x423c8) for a patient: the effect of a step of action
// 52 of `kind`. A drink counts a can for the drinks trophy and pays hospital
// 0's drinks income; the jukebox's takings are not kept; kind 24 counts a
// use of the patient's object (Thing.times_used). The staff's kinds do
// nothing here.
void complex_success(World &world, uint16_t id, uint16_t kind)
{
    Thing &w = world.things[id];
    const Balance::Gbv &gbv = settings_of(world).gbv;
    switch (kind) {
    case 2:
        reset_icon_value(w, kIconThirsty);
        w.refreshment = 100;
        w.happiness = uint16_t(w.happiness + gbv.drink_happy);
        add_litter_urge(w, 10);
        w.bladder = int16_t(w.bladder + 10);
        if (int16_t(w.warmth) > 60)
            w.warmth = 60;
        w.cash -= 20;
        create_numfx(world, kNumfxMoney, 20, uint16_t(w.x.tile + (w.y.tile << 7)), colour_red(world), 0x40);
        if (world.game)
            world.game->progress.award_counts.cans_of_coke++;
        if (w.owner == 0)
            world.drinks_income += 20;
        break;
    case 4:
        w.happiness++;
        w.cash -= 10;
        create_numfx(world, kNumfxMoney, 10, uint16_t(w.x.tile + (w.y.tile << 7)), colour_red(world), 0x40);
        if (w.owner == 0)
            world.jukebox_income += 10;
        break;
    case 6:
        trace_event("relieved %u bladder=%d", unsigned(id), int(w.bladder));
        reset_icon_value(w, kIconToilet);
        w.happiness = uint16_t(w.happiness + gbv.toilet_happy);
        w.bladder = w.illness == kIllTheSquits ? int16_t(gbv.bowel_full / 2) : int16_t(0);
        w.impatience = 0;
        w.waiting_time = 0;
        break;
    case 24:
        world.things[w.object].times_used++;
        break;
    default:
        break;
    }
    if (int16_t(w.happiness) < 0)
        w.happiness = 0;
    else if (int16_t(w.happiness) > 100)
        w.happiness = 100;
}

namespace
{

// The game's PLAY_FRAME of patient_stand after a failed or abandoned
// complex.
void stand(const Run &r, Thing &p)
{
    play_patient_anim(r.anims, p, kAnimStand);
}

void fail(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    trace_event("fail_complex %u", unsigned(id));
    if (p.resume_state) {
        p.state = p.resume_state;
        p.complex_pc = p.complex_queued = p.complex_queued2 = 0;
        p.complex_action = p.complex_resume = 0;
        stand(r, p);
    } else {
        p.state = kPsComplexRetry;
    }
}

// The other half of double door `door` swings open as `user` goes through,
// at their pace.
void swing_partner(const Run &r, const Thing &door, const Thing &user)
{
    Thing &other = r.world.things[door.door_partner];
    other.state = kOsDoubleDoorSwings;
    size_t start = kAnimSwingOpen + user.facing;
    if (start < r.anims.starts.size()) {
        other.sprite = uint16_t(r.anims.starts[start].frame);
        other.flip = uint8_t(r.anims.starts[start].flip);
    }
    other.anim_speed = user.anim_speed;
}

// Action 22 of a patient's complex: off its door onto the cell beyond
// (exit_tile), in that cell's room or none; a single door free again, a
// double door swinging shut.
void leave_door(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &door = world.things[p.object];
    MapPos x{0x80, uint8_t(p.exit_tile % kMapSize)};
    MapPos y{0x80, uint8_t(p.exit_tile / kMapSize)};
    p.dest_x = x.tile;
    p.dest_y = y.tile;
    move_node(world, id, x, y);
    door.user = 0;
    p.object = 0;
    move_person_in_room_list(world, id, p.exit_tile);
    MapCell &cell = world.map[cell_of(door)];
    if (!door.door_cell_flag)
        cell.flags &= uint8_t(~kCellDoorInUse);
    if (door.subtype == kObjDoor) {
        door.in_use = 0;
        door.state = kOsStatic;
    } else {
        door.state = kOsDoubleDoorSwings;
        size_t start = kAnimSwingClose + p.facing;
        if (start < r.anims.starts.size()) {
            door.sprite = uint16_t(r.anims.starts[start].frame);
            door.flip = uint8_t(r.anims.starts[start].flip);
        }
    }
}

// What a step of a complex returns when it carried on; any other value is
// what process returns at once.
constexpr uint8_t kStepDone = 1;

// Action 6: onto the object, from its use cell use_pos[1].
uint8_t process_use_object(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &obj = world.things[p.object];
    if (obj.in_use == 1 || obj.state == kOsRemoved || obj.type != kThingObject || obj.use_pos[1] != tile) {
        fail(r, id);
        return 3;
    }
    trace_event("use_object %u obj=%u kind=%u", unsigned(id), unsigned(p.object), unsigned(obj.subtype));
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
    return kStepDone;
}

// Action 8: off the object, back onto its use cell use_pos[1].
void process_leave_object(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    Thing &obj = world.things[p.object];
    obj.user = 0;
    p.object = 0;
    obj.in_use = 0;
    if (obj.use_style == 1 || obj.use_style == 3) {
        adjust_big_cell_flag(world, tile);
        move_node(world, id, {0x80, uint8_t(obj.use_pos[1] % kMapSize)}, {0x80, uint8_t(obj.use_pos[1] / kMapSize)});
        p.depth = 0;
        if (kObjectUse[obj.subtype].overlay_anim) {
            obj.depth = obj.saved_depth;
            obj.saved_depth = 0;
        }
        obj.state = kOsStatic;
    }
    stand(r, p);
    p.layers[10] = p.layers[11] = p.layers[12] = 0;
}

// Action 18: faces the way its object's kind faces its user.
void process_face_object(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object)
        return;
    const Thing &obj = world.things[p.object];
    int face = kObjectUse[obj.subtype].face_dirn;
    // Kinds whose face_dirn is -2 or -99 make the game read before
    // face_dirns; here they leave the facing.
    if (face < 0 || size_t(face) >= kFaceDirns.size())
        return;
    int8_t d = kFaceDirns[size_t(face)][size_t(obj.facing) + 4];
    if (d != -1) {
        p.facing = uint8_t(d);
        stand(r, p);
    }
}

// Action 20: onto the door of the cell it faces, unless the room beyond is
// being edited; a double door's other half swings open.
uint8_t process_enter_door(const Run &r, uint16_t id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint16_t door_id = find_door(world, id);
    if (!door_id) {
        fail(r, id);
        return 0;
    }
    Thing &door = world.things[door_id];
    bool double_door = door.subtype != kObjDoor;
    if (door.in_use == 1) {
        p.state = kPsComplexRetry;
        return 3;
    }
    const RoomInfo &room = world.rooms[door.room_id];
    if (p.room == 0 && (room.being_edited || room.state == kRoomEdited)) {
        fail(r, id);
        return 3;
    }
    door.in_use = 1;
    p.facing = p.room ? door.facing : kOppositeAngle[door.facing & 3];
    p.exit_tile = tile == door.use_pos[0] ? door.use_pos[3] : door.use_pos[0];
    play_patient_anim(r.anims, p, double_door ? kAnimDoubleDoor : kAnimSingleDoor);
    p.object = door_id;
    door.user = id;
    move_node(world, id, door.x, door.y);
    door.state = kOsInUse;
    MapCell &cell = world.map[cell_of(door)];
    door.door_cell_flag = cell.flags & kCellDoorInUse ? 1 : 0;
    cell.flags |= kCellDoorInUse;
    if (double_door)
        swing_partner(r, door, p);
    return kStepDone;
}

// Action 32, transform_peep_who (0x16b30).
void process_transform(const Run &r, Thing &p, const ComplexStep &entry)
{
    int8_t kind = kChangePersonWho[entry.anim][p.subtype];
    p.orig_subtype = p.subtype;
    if (kind != -1) {
        p.subtype = uint8_t(kind);
        play_patient_anim(r.anims, p, kAnimWalk);
    }
}

// Action 16: a random animation of the step's group.
void process_group_anim(const Run &r, Thing &p, const ComplexStep &entry)
{
    const GroupAnim &group = kGroupAnims[entry.anim];
    uint32_t choice = old_newrand(r.world.seed, "process_person_complex") % uint32_t(group.count);
    play_patient_anim(r.anims, p, group.anims[choice]);
}

// The step's actions that sequence the complex, animate or change the
// patient alone.
void process_sequence_action(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (entry.action) {
    case 4:
        play_patient_anim(r.anims, p, entry.anim);
        break;
    case 16:
        process_group_anim(r, p, entry);
        break;
    case 14:
        if (uint16_t door = find_door(world, id))
            p.facing = kOppositeAngle[world.things[door].facing & 3];
        break;
    case 24:
        p.complex_pc++;
        p.complex_resume = uint16_t(r.complexes.at(p.complex_pc) * 10);
        break;
    case 32:
        process_transform(r, p, entry);
        break;
    case 34:
        p.subtype = p.orig_subtype;
        break;
    case 46:
        head_off(r, id);
        break;
    case 52:
        complex_success(world, id, entry.anim);
        break;
    default:
        // 0, 12 and 26 do nothing, nor does 50 here (see process).
        break;
    }
}

// Runs the action of step `entry`: kStepDone, or the value process returns.
uint8_t process_action(const Run &r, uint16_t id, const ComplexStep &entry, uint16_t obj_id, uint16_t tile)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (entry.action) {
    case 2:
        p.object = obj_id;
        [[fallthrough]];
    case 44:
        set_destination_coords(world, id, world.things[p.object].use_pos[1]);
        head_off(r, id);
        p.countdown = 0;
        return kStepDone;
    case 6:
        return process_use_object(r, id, tile);
    case 8:
        process_leave_object(r, id, tile);
        return kStepDone;
    case 10:
        if (p.object)
            play_frame(world.things[p.object], r.anims, entry.anim);
        return kStepDone;
    case 18:
        process_face_object(r, id);
        return kStepDone;
    case 20:
        return process_enter_door(r, id, tile);
    case 22:
        leave_door(r, id);
        return kStepDone;
    default:
        process_sequence_action(r, id, entry);
        return kStepDone;
    }
}

// Moves the complex on past step `entry`: at once, or after it has repeated
// entry.repeat times (a random 1 to -entry.repeat when negative).
void process_advance(World &world, Thing &p, const ComplexStep &entry)
{
    if (!entry.repeat) {
        p.complex_pc++;
        return;
    }
    if (p.complex_repeat) {
        p.complex_repeat--;
        if (p.complex_repeat == 0)
            p.complex_pc++;
    } else if (entry.repeat < 0) {
        p.complex_repeat = uint8_t(old_newrand(world.seed, "process_person_complex") % uint32_t(-entry.repeat) + 1);
    } else {
        p.complex_repeat = uint8_t(entry.repeat);
    }
}

// At the end of the complex, starts the one queued behind it (running its
// first step) or moves on to next_state; otherwise runs a chained step.
void process_finish(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.complexes.at(p.complex_pc) != 0) {
        if (entry.chain)
            process(r, id);
        return;
    }
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
}

uint8_t process(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
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
    if (uint8_t result = process_action(r, id, entry, obj_id, tile); result != kStepDone)
        return result;
    process_advance(world, p, entry);
    process_finish(r, id, entry);
    // Action 50: the game runs process_things for the person alone.
    if (entry.action == 50) {
        process_things_alone_begin(world, id, r.game_turn);
        turn(r, id);
        final_count(world, r.game_turn);
    }
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
    trace_event("abandon_complex %u", unsigned(id));
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
    remove_person_from_room_list(r.world, id);
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(r.world.things[id].type));
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
    modify_person(world, r.anims, settings, heating_level_of(world), id, r.game_turn);
    if (world.things[id].health <= 0)
        return true;
    return do_reaction(world, r.anims, settings, id, r.game_turn);
}

// person_goto_destination (0x763d0).
void goto_destination(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (off_blueprint(world, id))
            return;
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
            person_nav_failure(r, id);
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
        if (off_blueprint(world, id) || needs_stop_walk(r, id))
            return;
        p.countdown = int16_t(0x100 / p.speed);
        int dir = person_head_for(world, id);
        if (dir >= 0 && dir <= 3 && std::abs(p.x.tile - p.dest_x) < world.object_near_dist &&
            std::abs(p.y.tile - p.dest_y) < world.object_near_dist) {
            const Thing &obj = world.things[p.object];
            if (obj.in_use == 1 || obj.state == kOsRemoved || (obj.type != kThingDoor && obj.type != kThingObject))
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
            person_nav_failure(r, id);
            return;
        case kNavNoPath:
            if (p.object) {
                Thing &obj = world.things[p.object];
                uint8_t in_use = obj.in_use;
                obj.in_use = 1;
                process(r, id);
                obj.in_use = in_use;
            } else {
                person_nav_failure(r, id);
            }
            return;
        default:
            face_step(r, p, dir);
            break;
        }
    }
    step_on(world, id);
}

// person_goto_queue arriving within `near` cells of its destination, the
// queue of room `room_id`.
void goto_queue_arrive(const Run &r, uint16_t id, uint16_t room_id, int near)
{
    World &world = r.world;
    Thing &p = world.things[id];
    RoomInfo &room = world.rooms[room_id];
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
        page_staff_i_need(world, id, room_id);
        p.state = kPsArrivesAtDoor;
    } else {
        p.state = kPsInQueue;
        stand(r, p);
    }
    p.countdown = 0;
}

// person_goto_queue (0x76c44): walks to the queue of dest_room; within
// World::queue_near_dist of the destination (of a reception, or of a room
// with a queue already) it joins the queue: PS_IN_QUEUE at a reception,
// PS_ARRIVES_AT_DOOR at a room, paging the staff its treatment needs, first
// walking on to the room's door if that is further. Sent on again
// (start_person_going) if the room is not one for its treatment or has no
// door.
void goto_queue(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (--p.countdown <= 0) {
        if (off_blueprint(world, id) || needs_stop_walk(r, id))
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
            goto_queue_arrive(r, id, room_id, near);
            return;
        case kNavBlockedByPeople:
            stand(r, p);
            p.countdown = 0;
            return;
        case kNavWalledIn:
        case kNavNoPath:
            person_nav_failure(r, id);
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

// wait_in_queue sending the patient out of its queue to the nearest toilets
// in use (then PS_IN_BOGS, back to PS_ENTERTAINS_THEMSELF). Returns whether
// there were any.
bool wait_in_queue_to_toilets(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint16_t room = get_nearest_room_info(world, p, kRoomToilets);
    if (!room)
        return false;
    trace_event("to_toilets %u room=%u bladder=%d", unsigned(id), unsigned(room), int(p.bladder));
    leave_queue(world, id);
    p.resume_state = kPsEntertainsThemself;
    p.next_state = kPsInBogs;
    p.substate = 0;
    set_complex(world, id, kComplexToToilets, world.rooms[room].door_tile);
    process(r, id);
    return true;
}

// wait_in_queue for a patient not on a bench: sits on a free bench near by
// if it can (the bench shortage counted down 5, or up 1 with none), or
// stands, moving off a crowded cell. Returns whether it was sent anywhere.
bool wait_in_queue_sit_or_stand(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (kAbilities[p.subtype][1]) {
        if (find_nearest_free_object(world, id, kObjBench)) {
            if (p.owner == 0)
                world.bench_shortage -= 5;
            start_pastime(r, id, kComplexBench);
            return true;
        }
        if (p.owner == 0)
            world.bench_shortage++;
    }
    stand(r, p);
    return find_standing_place(r, id);
}

// person_in_queue (0x15d48) and, with `entertaining`, person_entertains_themself
// (0x160a8): at the end of each animation, once the countdown runs out, off
// a blueprint it stands on, then the patient's needs (modify_person); then,
// alive, it pages the staff its treatment needs to the room it queues for,
// goes home fed up past gbv.leave_max impatience (entertaining only),
// fetches a drink when thirsty, goes to the nearest toilets in use with a
// bladder past gbv.bowel_full (entertaining only; out of its queue, then
// PS_IN_BOGS), sits on a free bench near by (the bench shortage counted
// down 5, or up 1 with none), or stands, moving off a crowded cell. A
// patient in the queue moves on to PS_ENTERTAINS_THEMSELF.
void wait_in_queue(const Run &r, uint16_t id, bool entertaining)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!animate(r, p) || --p.countdown > 0)
        return;
    if (off_blueprint(world, id))
        return;
    const Balance &settings = settings_of(world);
    modify_person(world, r.anims, settings, heating_level_of(world), id, r.game_turn);
    if (p.health <= 0)
        return;
    if (uint16_t room = which_queue(world, id))
        page_staff_i_need(world, id, room);
    if (entertaining && (p.impatience > settings.gbv.leave_max || (p.person_flags & kPfSentHomeByInspector))) {
        if (p.owner == 0)
            world.patients_left_count++;
        set_icon_value(r.anims, p, kIconFedUp);
        go_home(r, id);
        p.person_flags |= kPfFedUp;
        return;
    }
    const auto &can = kAbilities[p.subtype];
    if (can[0] && p.refreshment < 30 && p.complex_action != kComplexDrink &&
        find_object(world, cell_of(p), kObjDrinksMachine, true, true, p.owner)) {
        start_pastime(r, id, kComplexDrink);
        return;
    }
    if (entertaining && can[2] && p.bladder > settings.gbv.bowel_full && p.complex_action != kComplexLeaveRoom &&
        wait_in_queue_to_toilets(r, id))
        return;
    if (p.complex_action != kComplexBench && wait_in_queue_sit_or_stand(r, id))
        return;
    p.countdown = int16_t(0x100 / p.speed);
    if (!entertaining)
        p.state = kPsEntertainsThemself;
}

// person_in_bogs (0x17854), PS_IN_BOGS: in the toilets, the patient goes
// to a free toilet (substate 0, complex 330; PS_LEAVE_TOILETS if it fails),
// then (substate 1, its impatience gone) with an illness from 16 on (the
// non-visual ones) to a free sink (complex 350); without one free, or
// after, PS_LEAVE_TOILETS.
void person_in_bogs(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("in_bogs %u substate=%u", unsigned(id), unsigned(p.substate));
    if (p.substate == 0) {
        if (find_object(world, cell_of(p), kObjToilet, true, true, p.owner)) {
            p.next_state = kPsInBogs;
            p.resume_state = kPsLeaveToilets;
            p.substate = 1;
            set_complex(world, id, kComplexToilet, 0);
            process(r, id);
        } else {
            p.state = kPsLeaveToilets;
        }
    } else if (p.substate == 1) {
        p.impatience = 0;
        if (p.illness >= 0x10 && find_object(world, cell_of(p), kObjSink, true, true, p.owner)) {
            p.next_state = kPsLeaveToilets;
            p.resume_state = kPsLeaveToilets;
            set_complex(world, id, kComplexSink, 0);
            process(r, id);
        } else {
            p.state = kPsLeaveToilets;
        }
    } else {
        p.state = kPsLeaveToilets;
    }
}

// leave_toilets (0x179a8), PS_LEAVE_TOILETS: out of the toilets by their
// door to go for treatment again.
void leave_toilets(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("leave_toilets %u", unsigned(id));
    p.next_state = kPsGoForTreatment;
    p.resume_state = kPsLeaveToilets;
    set_complex(world, id, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile);
    process(r, id);
}

// return_to_queue_area (0x179f8): back to the queue it is in (to its
// hospital's first reception desk, or the room's door), entertaining itself
// there.
void return_to_queue_area(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("return_to_queue_area %u", unsigned(id));
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

// send_person_to_bed (0x16eb0): to the first ward of its hospital with a
// bed no one uses, else the best ward for it (get_expertise_dest), counted
// on its way to its queue, a nurse paged there; it will stay in bed
// 300-599 turns (drawn from world.seed first).
void send_person_to_bed(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    p.timer = int16_t(old_newrand(world.seed) % 300 + 300);
    uint16_t ward = 0;
    for (uint16_t room = 1; room < kRoomCount && !ward; room++) {
        if (world.rooms[room].owner != p.owner || world.rooms[room].type != kRoomWard)
            continue;
        for (uint16_t bed = world.rooms[room].first_object; bed; bed = world.things[bed].room_obj_next) {
            if (world.things[bed].subtype == kObjBed && world.things[bed].user == 0) {
                ward = room;
                break;
            }
        }
    }
    if (ward) {
        page_member_of_staff(world, kSkillNurse, ward, true, 1, p.owner);
        p.dest_room = ward;
        set_destination_coords(world, id, world.rooms[ward].door_tile);
        play_patient_anim(r.anims, p, kAnimWalk);
        p.state = kPsGotoQueue;
        world.rooms[ward].queue_incoming++;
        return;
    }
    ward = get_expertise_dest(world, kIllDiagWard, p);
    world.rooms[ward].queue_incoming++;
    page_member_of_staff(world, kSkillNurse, ward, true, 1, p.owner);
    p.dest_room = ward;
    play_patient_anim(r.anims, p, kAnimWalk);
    p.state = kPsGotoQueue;
    set_destination_coords(world, id, world.rooms[ward].door_tile);
}

// The end of go_for_treatment and send_person_for_research: patient `id` is
// counted on its way to the queue of `room_id` (paging the staff the treatment
// needs) and walks to its door.
void go_to_queue(const Run &r, uint16_t id, uint16_t room_id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    RoomInfo &room = world.rooms[room_id];
    room.queue_incoming++;
    page_staff_i_need(world, id, room_id);
    p.dest_room = room_id;
    play_patient_anim(r.anims, p, kAnimWalk);
    p.state = kPsGotoQueue;
    set_destination_coords(world, id, room.door_tile);
}

// send_person_for_research (0x6db78): patient `id` goes for its autopsy to the
// research room get_expertise_dest picks.
void send_person_for_research(const Run &r, uint16_t id)
{
    uint16_t room_id = get_expertise_dest(r.world, kIllAutopsy, r.world.things[id]);
    trace_event("send_for_research %u room=%u", unsigned(id), unsigned(room_id));
    go_to_queue(r, id, room_id);
}

// person_go_for_treatment (0x3d91c): the patient pays or not
// (shall_peep_pay); without the treatment (or paying for it) it goes home
// through the first entrance; with no room for it, it waits for a decision;
// else it walks to the queue of the room for it (get_expertise_dest),
// counted on its way there, paging the staff the treatment needs. A
// treatment the hospital cannot give goes to reputation_affected
// (kRepGoForTreatment: no points, but its price moves). A ward's diagnosis
// goes to bed (send_person_to_bed); an autopsy to the research room
// (send_person_for_research).
void go_for_treatment(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("go_for_treatment %u treatment=%u", unsigned(id), unsigned(p.treatment));
    uint16_t treatment = p.treatment;
    if (!shall_peep_pay(world, r.anims, id))
        treatment = 0;
    uint8_t category = kExpertiseCategory[treatment];
    if (p.person_flags & kPfEmergency) {
        if (!first_room(world, p.owner, category)) {
            trace_event("emergency_leaves %u", unsigned(id));
            treatment = 0;
        }
    } else if (p.owner != 0 || world.cure_available[treatment] == 0) {
        if (p.owner == 0)
            reputation_affected(world, uint8_t(treatment), kRepGoForTreatment);
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
        set_complex(world, id, kComplexLeave,
                    world.rooms[get_room_info(world, kRoomEntrance, 0x65, p.owner)].door_tile);
        process(r, id);
        break;
    case kIllDiagWard:
        send_person_to_bed(r, id);
        break;
    case kIllAutopsy:
        send_person_for_research(r, id);
        break;
    default:
        go_to_queue(r, id,
                    world.cure_available[treatment] == 1 ? get_expertise_dest(world, treatment, p)
                                                         : first_room(world, p.owner, category));
        break;
    }
}

// allhosps[owner].rooms[illness].turned_away++, kept for hospital 0.
void count_turned_away(World &world, const Thing &p)
{
    if (world.hospital && p.owner == 0)
        world.hospital->illnesses[p.illness % kIllnessCount].turned_away++;
}

// person_waits_for_decision for a diagnosis room, `waited` turns on: goes
// for one once the hospital has one, or home turned away after 1500 turns.
// Returns whether it was decided.
bool waits_for_decision_diagnosis(const Run &r, uint16_t id, uint16_t waited)
{
    World &world = r.world;
    Thing &p = world.things[id];
    bool done = uint8_t(find_possible_diagnosis(world, id)) != 0;
    if (!done && waited < 1500)
        return false;
    p.person_flags &= ~kPfAwaitingDecision;
    if (!(p.person_flags & kPfWaitingForRoom)) {
        get_playercall(world, 8, p.playercall, p.owner);
        p.playercall = 0;
    }
    if (done) {
        p.state = kPsGoForTreatment;
    } else {
        go_home(r, id);
        count_turned_away(world, p);
        p.person_flags |= kPfNoDiagnosis;
    }
    p.person_flags &= ~(kPfNeedsDiagnosisRoom | kPfWaitingForRoom);
    return true;
}

// person_waits_for_decision for a room for its treatment, `waited` turns
// on: goes for it once the hospital can give it and has its room, or home
// after 1500 turns. Returns whether it was decided.
bool waits_for_decision_treatment(const Run &r, uint16_t id, uint16_t waited)
{
    World &world = r.world;
    Thing &p = world.things[id];
    uint8_t treatment = p.next_treatment ? p.next_treatment : p.treatment;
    bool done = p.owner == 0 && world.cure_available[treatment] == 1 &&
                first_room(world, p.owner, kExpertiseCategory[treatment]);
    if (!done && waited < 1500)
        return false;
    p.person_flags &= ~kPfAwaitingDecision;
    if (!(p.person_flags & kPfWaitingForRoom)) {
        go_home(r, id);
        p.person_flags |= kPfNoTreatment;
        return true;
    }
    p.person_flags &= ~kPfWaitingForRoom;
    if (!done) {
        go_home(r, id);
        count_turned_away(world, p);
        p.person_flags |= kPfNoTreatment;
    } else {
        get_playercall(world, 4, p.playercall, p.owner);
        p.state = kPsGoForTreatment;
        p.playercall = 0;
    }
    return true;
}

// person_waits_for_decision (0x17e30): a patient waiting for a diagnosis
// room goes once the hospital has one for it (find_possible_diagnosis,
// which draws from world.seed), one waiting for a room for its treatment
// once the hospital can give it and has its room; 1500 turns after it began
// waiting, it goes home, turned away. Its player call is answered then. The
// player's answers (EV_CHOICE_RESULT) and status icons are not ported.
// Meanwhile it stands, moving off a blueprint or a crowded cell.
void waits_for_decision(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!animate(r, p) || --p.countdown > 0)
        return;
    auto waited = uint16_t(uint16_t(r.game_turn) - uint16_t(p.timer));
    bool decided = p.person_flags & kPfNeedsDiagnosisRoom ? waits_for_decision_diagnosis(r, id, waited)
                                                          : waits_for_decision_treatment(r, id, waited);
    if (decided)
        return;
    if (off_blueprint(world, id))
        return;
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
            set_icon_value(r.anims, p, kIconFedUp);
            go_home(r, id);
            p.person_flags |= kPfFedUp;
            return;
        }
        auto dir = uint8_t(old_newrand(world.seed, "peep_stuck") & 3);
        if (!wall_in_way(world, p.x.tile, p.y.tile, dir)) {
            p.facing = dir;
            p.countdown = int16_t(0x100 / p.speed);
            play_patient_anim(r.anims, p, kAnimWalk);
        }
    }
    step_on(world, id);
}

// person_fills (0x17620), PS_FILLS: the patient fills its machine (the
// machine's aux_sprite drawn split at split_height, which grows a pixel a
// turn, two for an alien), both animating every anim_speed turns; once past
// the frame's height it is ready (PS_SLAVE_READY) in the machine's frame,
// and an alien's doctor in the room takes his turn at once.
void person_fills(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object)
        return;
    Thing &machine = world.things[p.object];
    if (r.game_turn % p.anim_speed == 0) {
        p.sprite = r.anims.frames[p.sprite].next;
        machine.aux_sprite = r.anims.frames[machine.aux_sprite].next;
    }
    const AnimFrame *cur;
    if (p.subtype == kPatientAlien) {
        machine.split_height = uint8_t(machine.split_height + 2);
        cur = &r.anims.frames[machine.aux_sprite];
    } else {
        machine.split_height++;
        cur = &r.anims.frames[p.sprite];
    }
    if (machine.split_height <= cur->height)
        return;
    p.state = kPsSlaveReady;
    p.sprite = machine.aux_sprite;
    p.flip = machine.aux_flip;
    if (p.subtype != kPatientAlien || !p.room || !world.staff_complexes)
        return;
    // find_person_in_room (0x1b150).
    for (uint16_t o = world.rooms[p.room].first_person; o; o = world.things[o].room_next) {
        if (world.things[o].type == kThingStaff && world.things[o].subtype == kStaffDoctor) {
            process_things_alone_begin(world, o, r.game_turn);
            staff_turn(world, r.anims, r.complexes, *world.staff_complexes, o, r.game_turn);
            final_count(world, r.game_turn);
            break;
        }
    }
}

// person_energises (0x17780), PS_ENERGISES: the patient's and its machine's
// animations play to their last frame and hold there.
void person_energises(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.object || r.game_turn % p.anim_speed != 0)
        return;
    uint16_t next = r.anims.frames[p.sprite].next;
    if (r.anims.frames[next].flags & 1)
        return;
    p.sprite = next;
    Thing &machine = world.things[p.object];
    machine.aux_sprite = r.anims.frames[machine.aux_sprite].next;
}

// person_waits_for_busy (0x15c50).
void waits_for_busy(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    trace_event("waits_for_busy %u", unsigned(id));
    if (r.game_turn % p.anim_speed == 0)
        p.sprite = r.anims.frames[p.sprite].next;
    if (p.timer-- <= 0)
        go_home(r, id);
}

// play_complex_person_anim (0x418dc).
void play_complex_anim(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    if (r.game_turn % p.anim_speed != 0)
        return;
    uint16_t next = r.anims.frames[p.sprite].next;
    if (r.anims.frames[next].flags & 1)
        process(r, id);
    else
        p.sprite = next;
}

// person_arrives_at_door (0x15cc0): at the door of a room with a single
// door it turns away from it to queue (complex 50), else it queues at once.
void arrives_at_door(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    trace_event("at_door %u", unsigned(id));
    uint16_t room = is_this_a_door(world, cell_of(p));

    if (room && kRoomTypeInfo[world.rooms[room].type % kRoomTypes].door_kind != 0) {
        set_complex(world, id, kComplexAtDoor, 0);
        process(r, id);
    } else {
        p.state = kPsInQueue;
    }
}

// person_complex_retry (0x41a54).
void complex_retry(const Run &r, uint16_t id)
{
    Thing &p = r.world.things[id];
    if (r.game_turn % p.anim_speed == 0) {
        p.sprite = r.anims.frames[p.sprite].next;
        if (r.anims.frames[p.sprite].flags & 1)
            process(r, id);
    }
}

// person_in_bed (0x17194): in bed until the timer runs out (its needs
// looked at every 30 turns), then up and on to its next treatment.
void in_bed(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (p.timer == 0)
        return;
    if (r.game_turn % 30 == 0)
        modify_person(world, r.anims, settings_of(world), heating_level_of(world), id, r.game_turn);
    if (--p.timer == 0) {
        set_complex(world, id, kComplexStandStill, 0);
        process(r, id);
        setup_next_treatment(world, r.anims, 0, id, r.game_turn);
    }
}

// person_arrived_from_call (0x18f88): in a room, it waits for the member of
// staff who called it, and with none there leaves for its treatment; at a
// reception desk the receptionist takes it from here. Returns whether it
// waits.
bool arrived_from_call(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    if (!p.room)
        return false;
    for (const Thing &t : world.things)
        if (t.type == kThingStaff && t.room == p.room && t.slave == id)
            return true;
    set_complex(world, id, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile);
    p.next_state = kPsGoForTreatment;
    process(r, id);
    return false;
}

// The turn of a patient in state `state`, other than PS_ARRIVED_FROM_CALL.
void turn_state(const Run &r, uint16_t id, uint8_t state)
{
    World &world = r.world;
    switch (state) {
    case kPsWaitsForBusy:
        return waits_for_busy(r, id);
    case kPsGotoDestination:
        return goto_destination(r, id);
    case kPsReturnToQueueArea:
        return return_to_queue_area(r, id);
    case kPsGoHome:
        return go_home(r, id);
    case kPsGone:
        // person_gone (0x1673c).
        return remove(r, id);
    case kPsPlayComplexAnim:
        return play_complex_anim(r, id);
    case kPsArrivesAtDoor:
        return arrives_at_door(r, id);
    case kPsInQueue:
        return wait_in_queue(r, id, false);
    case kPsGotoObject:
        return goto_object(r, id);
    case kPsEntertainsThemself:
        return wait_in_queue(r, id, true);
    case kPsGotoQueue:
        return goto_queue(r, id);
    case kPsGoForTreatment:
        return go_for_treatment(r, id);
    case kPsComplexRetry:
        return complex_retry(r, id);
    case kPsShowanim:
        return person_showanim_turn(world, r.anims, id, r.game_turn);
    case kPsStuck:
        return stuck(r, id);
    case kPsInBed:
        return in_bed(r, id);
    case kPsWaitsForDecision:
        return waits_for_decision(r, id);
    case kPsBpGoto:
        return peep_bp_goto_turn(world, r.anims, id, r.game_turn);
    case kPsInBogs:
        return person_in_bogs(r, id);
    case kPsLeaveToilets:
        return leave_toilets(r, id);
    case kPsFills:
        return person_fills(r, id);
    case kPsEnergises:
        return person_energises(r, id);
    case kPsDead:
        return person_dead(world, r.anims, id, r.game_turn);
    case kPsFlys:
        return person_flys(world, r.anims, id, r.game_turn);
    default:
        return;
    }
}

bool turn(const Run &r, uint16_t id)
{
    World &world = r.world;
    uint8_t state = world.things[id].state;
    if (state != kPsArrivedFromCall)
        turn_state(r, id, state);
    else if (arrived_from_call(r, id))
        return true;
    return world.things[id].type == kThingPatient;
}

// double_door_swings (0x395b0): the swing plays a frame every anim_speed
// turns; at its end the door is free again, closed.
void double_door_swings(Thing &door, const AnimData &anims, uint32_t game_turn)
{
    if (game_turn % door.anim_speed != 0)
        return;
    door.sprite = anims.frames[door.sprite].next;
    if (!(anims.frames[door.sprite].flags & 1))
        return;
    door.in_use = 0;
    door.state = kOsStatic;
    play_frame(door, anims, door.subtype == kObjSwingDoor2 ? kAnimSwingDoor2Closed : kAnimSwingDoorClosed);
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
            if (type == kThingPatient || type == kThingStaff || type == kThingMayor || type == kThingInspector)
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
    trace_event("set_complex %u action=%u arg=%u", unsigned(id), unsigned(action), unsigned(arg));

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
    trace_event("start_going %u", unsigned(id));
    Thing &p = world.things[id];
    bool go = false;
    if (p.room && set_complex(world, id, kComplexLeaveRoom, world.rooms[p.room].door_inside_tile))
        go = true;
    if (uint16_t d = find_nearest_manned_reception(world, id, 0, true)) {
        const Thing &desk = world.things[d];
        p.dest_room = world.things[desk.user].room;
        go |= set_complex(world, id, kComplexToReception, desk.use_pos[1]);
    } else {
        uint16_t arg = world.rooms[world.entrances[p.owner & 3][old_newrand(world.seed) & 3]].door_tile;
        go |= set_complex(world, id, kComplexLeave, arg);
    }
    if (go)
        process({world, anims, complexes, game_turn}, id);
}

void person_go_home(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                    uint32_t game_turn)
{
    Thing &p = world.things[id];
    trace_event("go_home %u state=%u", unsigned(id), unsigned(p.state));
    if (p.playercall) {
        get_playercall(world, 8, p.playercall, p.owner);
        get_playercall(world, 4, p.playercall, p.owner);
        p.person_flags &= ~kPfAwaitingDecision;
        p.playercall = 0;
    }
    p.next_state = 0;
    record_person_as_patient(world, id);
    interrupt_person(world, anims, complexes, id, game_turn);
    leave_queue(world, id);
    if (set_complex(world, id, kComplexLeave,
                    world.rooms[get_room_info(world, kRoomEntrance, 0x65, p.owner)].door_tile))
        process({world, anims, complexes, game_turn}, id);
    set_icon_value(anims, p, kIconFedUp);
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
    if (!w.object || world.things[w.object].user != id)
        return;
    uint8_t type = world.things[w.object].type;
    if (type == kThingDoor) {
        set_complex(world, id, kComplexOffDoor, 0);
        process({world, anims, complexes, game_turn}, id);
    } else if (type == kThingObject) {
        set_complex(world, id, kComplexGetOff, 0);
        process({world, anims, complexes, game_turn}, id);
        w.speed = kPeepSpeed;
        w.anim_speed = kPeepAnimSpeed;
    }
}

void interrupt_peep(World &world, uint16_t id)
{
    if (!world.game)
        return;
    uint32_t game_turn = world.game->turn_count;
    const AnimData &anims = world.game->anims();
    if (world.things[id].type == kThingStaff)
        interrupt_staff(world, anims, id, game_turn);
    else if (world.things[id].type == kThingPatient && world.person_complexes)
        interrupt_person(world, anims, *world.person_complexes, id, game_turn);
}

void remove_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                   uint32_t game_turn)
{
    remove({world, anims, complexes, game_turn}, id);
}

void person_nav_failure(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn)
{
    person_nav_failure({world, anims, complexes, game_turn}, id);
}

bool person_turn(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id, uint32_t game_turn)
{
    return turn({world, anims, complexes, game_turn}, id);
}

namespace
{

// sliding_door_opens (0x39734).
void sliding_door_opens(Thing &door, const AnimData &anims, uint32_t game_turn)
{
    if (game_turn % door.anim_speed != 0)
        return;
    uint16_t next = anims.frames[door.sprite].next;
    if (anims.frames[next].flags & 1) {
        door.countdown = 50;
        door.state = kOsSlidingDoorOpen;
    } else {
        door.sprite = next;
    }
}

// sliding_door_closes (0x39880).
void sliding_door_closes(const World &world, Thing &door, const AnimData &anims, uint32_t game_turn)
{
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
}

} // namespace

void sliding_door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &door = world.things[id];
    if (door.type != kThingObject)
        return;
    switch (door.state) {
    case kOsDoubleDoorSwings:
        double_door_swings(door, anims, game_turn);
        break;
    case kOsSlidingDoorClosed:
        // sliding_door_closed (0x39704).
        if (someone_at_door(world, door))
            door.state = kOsSlidingDoorOpens;
        break;
    case kOsSlidingDoorOpens:
        sliding_door_opens(door, anims, game_turn);
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
    case kOsSlidingDoorCloses:
        sliding_door_closes(world, door, anims, game_turn);
        break;
    default:
        break;
    }
}

void door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &door = world.things[id];
    if (door.type != kThingDoor)
        return;
    if (door.state == kOsDoubleDoorSwings)
        double_door_swings(door, anims, game_turn);
    else if (door.anim_speed && game_turn % door.anim_speed == 0)
        door.sprite = anims.frames[door.sprite].next;
}

void move_person_in_room_list(World &world, uint16_t id, uint16_t tile)
{
    trace_event("room_move %u from=%u to=%u", unsigned(id), unsigned(world.things[id].room),
                unsigned(which_room(world, tile)));
    if (uint16_t room = which_room(world, tile))
        insert_person_in_room_list(world, room, id);
    else
        remove_person_from_room_list(world, id);
}

void insert_person_in_room_list(World &world, uint16_t room, uint16_t id)
{
    Thing &person = world.things[id];
    if (uint16_t first = world.rooms[room].first_person) {
        person.room_next = first;
        world.things[first].room_prev = id;
    }
    world.rooms[room].first_person = id;
    person.room = room;
    if (person.type == kThingPatient)
        world.rooms[room].patients_seen++;
}

void remove_person_from_room_list(World &world, uint16_t id)
{
    Thing &person = world.things[id];
    if (!person.room)
        return;
    RoomInfo &room = world.rooms[person.room];
    if (room.first_person) {
        if (person.room_prev)
            world.things[person.room_prev].room_next = person.room_next;
        else
            room.first_person = person.room_next;
        if (person.room_next)
            world.things[person.room_next].room_prev = person.room_prev;
    }
    person.room = 0;
    person.room_next = 0;
    person.room_prev = 0;
}

namespace
{

// Whether map cell `cell` (which may be off the map) is a blueprint's floor.
bool peep_bp_goto_on_blueprint(const World &world, int cell)
{
    return cell >= 0 && cell < kMapCells && block_flags(world.map[size_t(cell)].floor_block) == 0x20;
}

// The direction peep_bp_goto_turn steps from blueprint cell `cell` towards
// the saved destination: along the axis it is already lined up on, else
// along a random one, or the other if that leaves the blueprint.
uint8_t peep_bp_goto_direction(World &world, const Thing &peep, int cell)
{
    bool vertical = false;
    bool retry = false;
    if (peep.x.tile == peep.saved_dest_x) {
        vertical = true;
    } else if (peep.y.tile != peep.saved_dest_y) {
        vertical = old_newrand(world.seed, "peep_bp_goto") % 1000 < 500;
        retry = true;
    }
    static constexpr std::array<int16_t, 4> around = {-128, 1, 128, -1};
    uint8_t dir = 0;
    for (;;) {
        if (vertical)
            dir = peep.y.tile < peep.saved_dest_y ? 2 : 0;
        else
            dir = peep.x.tile < peep.saved_dest_x ? 1 : 3;
        if (!retry || peep_bp_goto_on_blueprint(world, cell + around[dir]))
            break;
        vertical = !vertical;
        retry = false;
    }
    return dir;
}

// A step of peep_bp_goto_turn once the countdown runs out: off the
// blueprint (or at the saved destination) back to saved_state, else on
// towards the destination. Returns whether it is still walking.
bool peep_bp_goto_step(World &world, const AnimData &anims, uint16_t id)
{
    Thing &peep = world.things[id];
    int cell = peep.x.tile + (peep.y.tile << 7);
    if (!peep_bp_goto_on_blueprint(world, cell) ||
        (peep.x.tile == peep.saved_dest_x && peep.y.tile == peep.saved_dest_y)) {
        peep.state = peep.saved_state;
        return false;
    }
    uint8_t dir = peep_bp_goto_direction(world, peep, cell);
    peep.countdown = int16_t(0x100 / peep.speed);
    const PersonAnims &kind = peep.type == kThingStaff ? kStaffAnims[peep.subtype] : kPatientAnims[peep.subtype];
    size_t stand = size_t(kind.stand) + peep.facing;
    if (peep.facing != dir || (stand < anims.starts.size() && anims.starts[stand].frame == peep.sprite)) {
        peep.facing = dir;
        play_frame(peep, anims, kind.walk);
    }
    return true;
}

} // namespace

void peep_bp_goto_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &peep = world.things[id];
    if (game_turn % peep.anim_speed == 0)
        peep.sprite = anims.frames[peep.sprite].next;
    if (--peep.countdown <= 0 && !peep_bp_goto_step(world, anims, id))
        return;
    step_on(world, id);
}

void patient_abandon_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                             uint32_t game_turn)
{
    abandon_complex(Run{world, anims, complexes, game_turn}, id);
    world.things[id].complex_action = 0;
}

} // namespace th
