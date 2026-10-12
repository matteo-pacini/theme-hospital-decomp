#include "core/world/complex_internal.h"

#include "core/data/blocks.h"
#include "core/data/gamedata.h"
#include "core/world/complex_tables.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

inline namespace complex_detail
{

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}
} // namespace complex_detail

inline namespace complex_detail
{

// The settings the patients' states read: World::settings, or the game's
// static ones before start_queues.
const Balance &settings_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}
} // namespace complex_detail

inline namespace complex_detail
{

int heating_level_of(const World &world)
{
    return world.finance ? world.finance->heating_level : kStartHeatingLevel;
}
} // namespace complex_detail

namespace
{

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

} // namespace

inline namespace complex_detail
{

// can_i_get_to (0x19918): whether a path leads from (x, y) to `tile`.
bool path_exists(const World &world, int x, int y, uint16_t tile)
{
    OrthonavPath path;
    orthonav_init(world, path, x, y, tile % kMapSize, tile / kMapSize);
    return path.count != 0;
}
} // namespace complex_detail

inline namespace complex_detail
{

// listhead[owner].rooms[type]: hospital `owner`'s room of `type` once its
// doors are committed (World::room_of_type, hospital 0's), 0 if none.
uint16_t first_room(const World &world, uint8_t owner, uint8_t type)
{
    return owner == 0 ? world.room_of_type[type % world.room_of_type.size()] : 0;
}
} // namespace complex_detail

inline namespace complex_detail
{

// Whether the patient stands on a blueprint's floor and was sent off it
// (move_peep_from_blueprint), as the walking and waiting states check
// first.
bool off_blueprint(World &world, uint16_t id)
{
    const Thing &t = world.things[id];
    return block_flags(world.map[uint16_t(t.x.tile + (t.y.tile << 7))].floor_block) == 0x20 &&
           move_peep_from_blueprint(world, id);
}
} // namespace complex_detail

inline namespace complex_detail
{

void play_patient_anim(const AnimData &anims, Thing &p, int column)
{
    play_frame(p, anims, kPatientAnimStarts[p.subtype][size_t(column)]);
}
} // namespace complex_detail

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

inline namespace complex_detail
{

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
} // namespace complex_detail

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
