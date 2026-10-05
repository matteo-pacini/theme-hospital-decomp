#include "core/world/queue.h"

#include "core/world/complex_tables.h"
#include "core/world/nav.h"
#include "core/world/reception.h"

namespace th
{

void start_queues(World &world, const Balance &settings, HospitalMarket &market, const int &reputation,
                  const Finance &finance)
{
    world.settings = &settings;
    world.market = &market;
    world.reputation = &reputation;
    world.finance = &finance;
    world.queue_near_dist = 0;
    world.object_near_dist = 0;
    world.cure_available = {};
    world.cure_available[0] = 1;
    world.ins_control = {1, 2, 3, 0, 0};
    world.patients_left_count = 0;
    world.bench_shortage = 0;
    world.unpaid_treatment = 0;
    world.too_expensive = false;
    world.drinks_income = 0;
}

void draw_turn_randoms(World &world)
{
    old_newrand(world.seed);
    world.queue_near_dist = uint8_t(old_newrand(world.seed) % 6);
    world.object_near_dist = uint8_t((old_newrand(world.seed) & 3) + 1);
}

void join_queue(World &world, uint16_t id, uint16_t room_id)
{
    leave_queue(world, id);
    Thing &p = world.things[id];
    RoomInfo &room = world.rooms[room_id];
    if (uint16_t tail = room.queue_head) {
        while (world.things[tail].queue_next)
            tail = world.things[tail].queue_next;
        world.things[tail].queue_next = id;
        p.queue_prev = tail;
    } else {
        room.queue_head = id;
        p.queue_prev = 0;
    }
    p.queue_next = 0;
    p.queue_room = room_id;
    room.queue_length++;
    if (room.queue_incoming)
        room.queue_incoming--;
}

void leave_queue(World &world, uint16_t id)
{
    Thing &p = world.things[id];
    if (p.queue_room > 0 && p.queue_room < kRoomCount) {
        RoomInfo &room = world.rooms[p.queue_room];
        if (p.queue_prev == 0) {
            if (id == room.queue_head) {
                if (p.queue_next)
                    world.things[p.queue_next].queue_prev = 0;
                room.queue_head = p.queue_next;
            }
        } else if ((world.things[p.queue_prev].queue_next = p.queue_next) > 0) {
            world.things[p.queue_next].queue_prev = p.queue_prev;
        }
        if (room.queue_length > 0)
            room.queue_length--;
    } else if (p.dest_room) {
        RoomInfo &dest = world.rooms[p.dest_room];
        if (dest.type != kRoomReception && dest.queue_incoming != 0)
            dest.queue_incoming--;
        p.dest_room = 0;
    }
    p.queue_next = 0;
    p.queue_prev = 0;
    p.queue_room = 0;
}

uint16_t queue_size(const World &world, uint16_t room)
{
    return world.rooms[room].queue_length;
}

uint16_t which_queue(const World &world, uint16_t id)
{
    uint16_t head = id;
    while (world.things[head].queue_prev)
        head = world.things[head].queue_prev;
    for (uint16_t r = 1; r < kRoomCount; r++)
        if (world.rooms[r].queue_head == head)
            return r;
    return 0;
}

uint16_t get_real_person_from_room(const World &world, uint16_t room, uint16_t pos)
{
    uint32_t i = 1;
    for (uint16_t id = world.rooms[room].queue_head; id; id = world.things[id].queue_next, i++)
        if (pos == i)
            return id;
    return 0;
}

void front_queue(World &world, uint16_t room_id, uint16_t pos)
{
    if (room_id == 0 || pos == 0 || queue_size(world, room_id) == 0)
        return;
    RoomInfo &room = world.rooms[room_id];
    uint16_t i = 1;
    for (uint16_t id = room.queue_head; id; id = world.things[id].queue_next, i++) {
        if (pos != i)
            continue;
        leave_queue(world, id);
        Thing &p = world.things[id];
        uint16_t head = room.queue_head;
        room.queue_head = id;
        room.queue_length++;
        p.queue_room = room_id;
        p.queue_prev = 0;
        p.queue_next = head;
        // The game links the old head back even when the queue was empty,
        // writing thing 0's queue_prev.
        world.things[head].queue_prev = id;
        return;
    }
}

void back_queue(World &world, uint16_t room_id, uint16_t pos)
{
    if (room_id == 0 || pos == 0 || queue_size(world, room_id) == 0)
        return;
    uint16_t i = 1;
    for (uint16_t id = world.rooms[room_id].queue_head; id; id = world.things[id].queue_next, i++) {
        if (pos == i) {
            leave_queue(world, id);
            join_queue(world, id, room_id);
            return;
        }
    }
}

int16_t spare_queue_capacity(const World &world, uint16_t type, bool waiting, uint16_t room_id, uint8_t owner)
{
    auto spare = [&](const RoomInfo &room) {
        int16_t n = int16_t(room.queue_capacity - room.queue_length);
        if (waiting)
            n = int16_t(n - room.queue_incoming);
        return n;
    };
    if (room_id == 0) {
        for (uint16_t r = 1; r < kRoomCount; r++) {
            const RoomInfo &cand = world.rooms[r];
            if (cand.state >= 0 && cand.owner == owner && cand.type == kExpertiseCategory[type]) {
                int16_t n = spare(cand);
                if (n > 0)
                    return n;
            }
        }
    } else if (int16_t n = spare(world.rooms[room_id]); n > 0) {
        return n;
    }
    return 0;
}

int16_t all_queue_capacity(const World &world, uint16_t type, bool waiting, uint8_t owner)
{
    if (owner != 0 || world.cure_available[type] == 0)
        return 0;
    int32_t total = 0;
    for (uint16_t r = 1; r < kRoomCount; r++) {
        const RoomInfo &cand = world.rooms[r];
        if (cand.state >= 0 && cand.owner == owner && cand.type == kExpertiseCategory[type]) {
            int32_t spare = cand.queue_capacity - cand.queue_length;
            if (waiting)
                spare -= cand.queue_incoming;
            if (spare > 0)
                total += spare;
        }
    }
    return int16_t(total);
}

void queue_person_to_new_queue(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room_id,
                               uint16_t id, uint32_t game_turn)
{
    if (room_id == 0 || id == 0 || !person_ok_to_call(world, id))
        return;
    leave_queue(world, id);
    interrupt_person(world, anims, complexes, id, game_turn);
    RoomInfo &room = world.rooms[room_id];
    if (room.type == kRoomReception) {
        auto desk = uint8_t(find_nearest_manned_reception(world, id, uint8_t(room_id), false));
        if (desk)
            set_destination_coords(world, id, world.things[desk].use_pos[1]);
    } else {
        room.queue_incoming++;
        set_destination_coords(world, id, room.door_tile);
    }
    Thing &p = world.things[id];
    p.dest_room = room_id;
    play_frame(p, anims, kPatientAnimStarts[p.subtype][0]);
    p.state = kPsGotoQueue;
}

void queue_person_go_home(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                          uint16_t pos, uint32_t game_turn)
{
    if (uint16_t id = get_real_person_from_room(world, room, pos))
        person_go_home(world, anims, complexes, id, game_turn);
}

void queue_person_go_reception(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                               uint16_t pos, uint32_t game_turn)
{
    if (uint16_t id = get_real_person_from_room(world, room, pos)) {
        leave_queue(world, id);
        interrupt_person(world, anims, complexes, id, game_turn);
        start_person_going(world, anims, complexes, id, game_turn);
    }
}

void restart_all_queue_peeps(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                             uint32_t game_turn)
{
    while (uint16_t id = world.rooms[room].queue_head) {
        interrupt_person(world, anims, complexes, id, game_turn);
        leave_queue(world, id);
        start_person_going(world, anims, complexes, id, game_turn);
    }
}

} // namespace th
