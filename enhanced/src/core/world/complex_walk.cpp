// A patient walking and queueing: heading for an object or a queue, the
// queue's waiting states and the toilets (src/patient.cpp).

#include "core/world/complex_internal.h"

#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"

#include <cstdlib>

namespace th
{

namespace
{

// The complexes of the toilets: to their door and in, to a toilet and to a
// sink.
constexpr uint16_t kComplexToToilets = 360;
constexpr uint16_t kComplexToilet = 330;
constexpr uint16_t kComplexSink = 350;
constexpr uint8_t kObjToilet = 51;
constexpr uint8_t kObjSink = 32;

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

} // namespace

inline namespace complex_detail
{

// Advances a person's animation every anim_speed turns, as the waiting
// states do; returns whether it reached the end of the animation.
bool animate(const Run &r, Thing &p)
{
    if (r.game_turn % p.anim_speed != 0)
        return false;
    p.sprite = r.anims.frames[p.sprite].next;
    return r.anims.frames[p.sprite].flags & 1;
}
} // namespace complex_detail

namespace
{

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

} // namespace

inline namespace complex_detail
{

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
} // namespace complex_detail

namespace
{

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

} // namespace

inline namespace complex_detail
{

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
} // namespace complex_detail

inline namespace complex_detail
{

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
} // namespace complex_detail

namespace
{

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

} // namespace

inline namespace complex_detail
{

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
} // namespace complex_detail

inline namespace complex_detail
{

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
} // namespace complex_detail

namespace
{

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

} // namespace

inline namespace complex_detail
{

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
} // namespace complex_detail

inline namespace complex_detail
{

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
} // namespace complex_detail

inline namespace complex_detail
{

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
} // namespace complex_detail

inline namespace complex_detail
{

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
} // namespace complex_detail

} // namespace th
