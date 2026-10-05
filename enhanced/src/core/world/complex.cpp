#include "core/world/complex.h"

#include "core/data/gamedata.h"
#include "core/world/complex_tables.h"
#include "core/world/nav.h"

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

// find_nearest_free_object (0x37d28), outside rooms (there are none yet):
// the nearest free object of `kind` of the seeker's hospital within 7 cells
// on each axis whose use cell a path reaches, as can_i_get_to with the
// object as the mover (a path from its own use cell 1). 0 if none.
uint16_t find_nearest_free_object(const World &world, uint16_t id, uint8_t kind)
{
    const Thing &seeker = world.things[id];
    // Only player 0's objects exist (World::first_of_kind).
    if (seeker.owner != 0)
        return 0;
    // Patients use cell 1 (staff 0, handymen 2).
    constexpr size_t side = 1;
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

// adjust_big_cell_flag (0xa8f78): sets kCellBig on `cell` if a person on it
// uses its object or an object on it covers more than one cell, else clears
// it.
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
    default:
        // 0, 12 and 26 do nothing; 22 (leaving a door) and 52
        // (complex_success) are not ported, see process_person_complex.
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
    abandon_complex(r, id);
    remove_node(r.world, id);
    r.world.things[id] = Thing{};
}

bool turn(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &p = world.things[id];
    switch (p.state) {
    case kPsGotoDestination:
        switch (walk_turn(world, r.anims, id, r.game_turn)) {
        case WalkResult::Arrived:
            if (process(r, id) == 2) {
                p.state = kPsWaitsForBusy;
                p.timer = 200;
            }
            p.countdown = 0;
            break;
        case WalkResult::WalledIn:
        case WalkResult::NoPath:
            person_nav_failure(world, id);
            break;
        default:
            break;
        }
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
    case kPsComplexRetry:
        // person_complex_retry (0x41a54).
        if (r.game_turn % p.anim_speed == 0) {
            p.sprite = r.anims.frames[p.sprite].next;
            if (r.anims.frames[p.sprite].flags & 1)
                process(r, id);
        }
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

void start_person_going(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn)
{
    Thing &p = world.things[id];
    // A patient in a room would first leave it (complex 90), and one with a
    // manned reception desk would go there (complex 470): there are no rooms
    // or staff yet.
    uint16_t arg = world.entrances[p.owner & 3][old_newrand(world.seed) & 3];
    if (set_complex(world, id, kComplexLeave, arg))
        process({world, anims, complexes, game_turn}, id);
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
