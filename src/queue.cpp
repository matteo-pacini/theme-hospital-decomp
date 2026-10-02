#include "hosp.h"

extern "C" {

extern Event player_events[];
extern u32 last_flag_turn;
extern RoomInfo *last_flag_room;

u16 nth_other_active_player(int nth);
}

// Moves the patient at position pos (1-based) of room room_id's queue to its
// front.
// FUNCTION: 0x0004ace0
void front_queue(const u16 room_id, const u16 pos)
{
    RoomInfo *room;
    u16 size;
    u16 i;
    u16 id;
    Thing *person;
    Thing *head;

    room = &room_info[room_id];
    if (room > room_info) {
        if (pos) {
            if (size = queue_size(room)) {
                i = 1;
                if (id = room->queue_head) {
                    person = &thing[id];
                    while (1) {
                        if (pos == i) {
                            leave_queue(person);
                            id = room->queue_head;
                            head = &thing[id];
                            room->queue_head = person - thing;
                            room->queue_length++;
                            person->queue_room = room - room_info;
                            person->queue_prev = 0;
                            person->queue_next = id;
                            head->queue_prev = person - thing;
                            return;
                        }
                        if (person->queue_next == 0) {
                            return;
                        }
                        id = person->queue_next;
                        person = &thing[id];
                        i++;
                    }
                }
            }
        }
    }
}

// Moves the patient at position pos (1-based) of room room_id's queue to its
// back.
// FUNCTION: 0x0004ae60
void back_queue(const u16 room_id, const u16 pos)
{
    RoomInfo *room;
    u16 size;
    u16 id;
    u16 i;
    Thing *person;

    room = &room_info[room_id];
    if (room > room_info) {
        if (pos) {
            i = 1;
            if (size = queue_size(room)) {
                if (id = room->queue_head) {
                    person = &thing[id];
                    while (1) {
                        if (pos == i) {
                            leave_queue(person);
                            join_queue(person, room);
                            return;
                        }
                        if (person->queue_next == 0) {
                            return;
                        }
                        id = person->queue_next;
                        person = &thing[id];
                        i++;
                    }
                }
            }
        }
    }
}

// Appends person to room's queue, leaving any queue they were in. Counts
// them in the queue length (queue_length) and no longer as on the way
// (queue_incoming).
// FUNCTION: 0x0004af50
void join_queue(Thing *person, RoomInfo *room)
{
    u16 id;
    Thing *tail;

    leave_queue(person);
    if (id = room->queue_head) {
        tail = &thing[id];
        while (tail->queue_next != 0) {
            id = tail->queue_next;
            tail = &thing[id];
        }
        tail->queue_next = person - thing;
        person->queue_prev = id;
    } else {
        room->queue_head = person - thing;
        person->queue_prev = 0;
    }
    person->queue_next = 0;
    person->queue_room = room - room_info;
    room->queue_length++;
    if (room->queue_incoming) {
        room->queue_incoming--;
    }
}

// Takes person out of their queue (adjusting its length), or, if not in one,
// cancels their trip to dest_room (no longer counted as on the way).
// FUNCTION: 0x0004b058
void leave_queue(Thing *person)
{
    u16 id;
    Thing *next;
    RoomInfo *room;

    room = &room_info[person->queue_room];
    if (room > room_info && room < end_room_info) {
        if (person->queue_prev == 0) {
            if ((u16)(person - thing) == room->queue_head) {
                id = person->queue_next;
                next = &thing[id];
                if (next > thing) {
                    next->queue_prev = 0;
                }
                room->queue_head = id;
            }
        } else if ((thing[person->queue_prev].queue_next = person->queue_next) > 0) {
            thing[person->queue_next].queue_prev = person->queue_prev;
        }
        if (room->queue_length > 0) {
            room->queue_length--;
        }
    } else if (person->dest_room) {
        if (room_info[person->dest_room].type != ROOM_RECEPTION) {
            if (room_info[person->dest_room].queue_incoming != 0) {
                room_info[person->dest_room].queue_incoming--;
            }
        }
        person->dest_room = 0;
    }
    person->queue_next = 0;
    person->queue_prev = 0;
    person->queue_room = 0;
}

// Returns the number of people in room's queue.
// FUNCTION: 0x0004b21c
u16 queue_size(RoomInfo *room)
{
    return room->queue_length;
}

// Returns the room (room_info index) whose queue person is in, or 0.
// FUNCTION: 0x0004b244
u16 which_queue(Thing *person)
{
    Thing *head;
    RoomInfo *room;

    head = person;
    while (head->queue_prev) {
        head = &thing[head->queue_prev];
    }
    for (room = room_info + 1; room < end_room_info; room++) {
        if ((u16)(head - thing) == room->queue_head) {
            return room - room_info;
        }
    }
    return 0;
}

// Queue window button `button` for queue position othervar.queue_pos of room
// othervar.queue_win_room: 0/1 to the front/back, 4 home, 5 to reception, 6 picks the
// patient up to move to another queue, 7 closes, 8-10 refer to the 1st-3rd
// other player (network game), 2/3 raise/lower the queue's limit
// (EV_QUEUE_CAPACITY_UP/DOWN).
// FUNCTION: 0x0004b2f4
void process_queue_button(u8 button)
{
    debug.last_action = button;
    switch (button) {
    case 0:
        if (othervar.queue_pos) {
            player_events[player].type = EV_FRONT_QUEUE;
            player_events[player].value = othervar.queue_win_room;
            player_events[player].arg = othervar.queue_pos;
        }
        break;
    case 1:
        if (othervar.queue_pos) {
            player_events[player].type = EV_BACK_QUEUE;
            player_events[player].value = othervar.queue_win_room;
            player_events[player].arg = othervar.queue_pos;
        }
        break;
    case 2:
        player_events[player].type = EV_QUEUE_CAPACITY_UP;
        player_events[player].value = othervar.queue_win_room;
        break;
    case 3:
        player_events[player].type = EV_QUEUE_CAPACITY_DOWN;
        player_events[player].value = othervar.queue_win_room;
        break;
    case 4:
        if (othervar.queue_pos) {
            player_events[player].type = EV_QUEUE_PERSON_GO_HOME;
            player_events[player].value = othervar.queue_win_room;
            player_events[player].arg = othervar.queue_pos;
        }
        break;
    case 5:
        if (othervar.queue_pos) {
            player_events[player].type = EV_QUEUE_PERSON_GO_RECEPTION;
            player_events[player].value = othervar.queue_win_room;
            player_events[player].arg = othervar.queue_pos;
        }
        break;
    case 6:
        if (othervar.queue_pos) {
            gamevar.queue_selected_person = get_real_person_from_room(othervar.queue_win_room, othervar.queue_pos);
            que.state = 3;
        }
        break;
    case 7:
        que.state = 2;
        break;
    case 8:
        if (statvar.num_players > 1) {
            if (othervar.queue_pos) {
                player_events[player].type = EV_REFER_PATIENT;
                player_events[player].value = get_real_person_from_room(othervar.queue_win_room, othervar.queue_pos);
                player_events[player].arg = nth_other_active_player(1);
                othervar.queue_pos = 0;
            }
        }
        break;
    case 9:
        if (statvar.num_players > 2) {
            if (othervar.queue_pos) {
                player_events[player].type = EV_REFER_PATIENT;
                player_events[player].value = get_real_person_from_room(othervar.queue_win_room, othervar.queue_pos);
                player_events[player].arg = nth_other_active_player(2);
                othervar.queue_pos = 0;
            }
        }
        break;
    case 10:
        if (statvar.num_players > 3) {
            if (othervar.queue_pos) {
                player_events[player].type = EV_REFER_PATIENT;
                player_events[player].value = get_real_person_from_room(othervar.queue_win_room, othervar.queue_pos);
                player_events[player].arg = nth_other_active_player(3);
                othervar.queue_pos = 0;
            }
        }
        break;
    }
}

// Sends the patient at position pos of room room_id's queue home.
// FUNCTION: 0x0004b654
void queue_person_go_home(const u16 room_id, const u16 pos)
{
    u16 id;
    Thing *person;

    id = get_real_person_from_room(room_id, pos);
    person = &thing[id];
    if (person > thing) {
        person_go_home(person);
    }
}

// Sends the patient at position pos of room room_id's queue back to
// reception.
// FUNCTION: 0x0004b6b0
void queue_person_go_reception(const u16 room_id, const u16 pos)
{
    u16 id;
    Thing *person;

    id = get_real_person_from_room(room_id, pos);
    person = &thing[id];
    if (person > thing) {
        leave_queue(person);
        interrupt_person(person);
        start_person_going(person);
    }
}

// Returns the patient at position pos (1-based) of room room_id's queue, or
// 0.
// FUNCTION: 0x0004b71c
u16 get_real_person_from_room(const u16 room_id, const u16 pos)
{
    u32 i;
    RoomInfo *room;
    Thing *person;

    i = 1;
    room = &room_info[room_id];
    person = &thing[room->queue_head];
    if (person > thing) {
        while (1) {
            if (pos == i) {
                return person - thing;
            }
            if (person->queue_next == 0) {
                break;
            }
            person = &thing[person->queue_next];
            i++;
        }
    }
    return 0;
}

// Moves patient id to room room_id's queue (to the nearest manned desk for
// a reception), unless they are in or going to the toilets. The desk's thing
// index is kept in a u8, so a desk past index 255 is looked up wrongly.
// FUNCTION: 0x0004b7e0
void queue_person_to_new_queue(const u16 room_id, const u16 id)
{
    RoomInfo *room;
    Thing *person;
    u8 desk;

    room = &room_info[room_id];
    if (room > room_info) {
        person = &thing[id];
        if (person > thing) {
            if (person_ok_to_call(person)) {
                leave_queue(person);
                interrupt_person(person);
                if (room->type == ROOM_RECEPTION) {
                    desk = find_nearest_manned_reception(person, room - room_info, 0);
                    if (desk) {
                        set_destination_coords(person, thing[desk].use_pos[1]);
                    }
                } else {
                    room->queue_incoming++;
                    page_staff_i_need(person, room);
                    set_destination_coords(person, room->door_tile);
                }
                person->dest_room = room - room_info;
                {
                    u16 anim;

                    anim = check_frame(anim_starts[person->subtype].walk, person) + (u16)person->facing;
                    person->sprite = start[anim].frame;
                    person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                }
                person->state = PS_GOTO_QUEUE;
            }
        }
    }
}

// Returns the free places (queue_capacity minus queue length, and minus those
// on the way if waiting is set) in room, or, with room 0, in the first of
// player `owner`'s rooms for illness type that has any. 0 if none.
// FUNCTION: 0x0004b988
s16 spare_queue_capacity(const u16 type, const u8 waiting, RoomInfo *room, const u8 owner)
{
    Expertise *exp;
    RoomInfo *cand;
    s16 spare;

    exp = &expertise[type];
    if (room == 0) {
        for (cand = room_info + 1; cand < end_room_info; cand++) {
            if (cand->state >= 0 && cand->owner == owner) {
                if (cand->type == exp->category) {
                    spare = cand->queue_capacity - cand->queue_length;
                    if (waiting) {
                        spare -= cand->queue_incoming;
                    }
                    if (spare > 0) {
                        return spare;
                    }
                }
            }
        }
    } else {
        spare = room->queue_capacity - room->queue_length;
        if (waiting) {
            spare -= room->queue_incoming;
        }
        if (spare > 0) {
            return spare;
        }
    }
    return 0;
}

// Returns the free places in all of player `owner`'s rooms for illness type
// (as spare_queue_capacity), or 0 if the hospital cannot treat it.
// FUNCTION: 0x0004ba8c
s16 all_queue_capacity(const u16 type, const u8 waiting, const u8 owner)
{
    s32 total;
    Expertise *exp;
    RoomInfo *cand;
    s32 spare;

    total = 0;
    if (allhosps[owner].rooms[type].cure_available == 0) {
        return 0;
    }
    exp = &expertise[type];
    for (cand = room_info + 1; cand < end_room_info; cand++) {
        if (cand->state >= 0 && cand->owner == owner) {
            if (cand->type == exp->category) {
                spare = cand->queue_capacity - cand->queue_length;
                if (waiting) {
                    spare -= cand->queue_incoming;
                }
                if (spare > 0) {
                    total += spare;
                }
            }
        }
    }
    return total;
}

// Empties room's queue, sending everyone in it back to reception.
// FUNCTION: 0x0004bb90
void restart_all_queue_peeps(RoomInfo *room)
{
    Thing *person;

    while ((person = &thing[room->queue_head]) > thing) {
        interrupt_person(person);
        leave_queue(person);
        start_person_going(person);
    }
}

// Marks everyone in room's queue with ICON_QUEUE, with a sound when a new
// queue is highlighted.
// FUNCTION: 0x0004bbec
void flag_all_queue_members(RoomInfo *room)
{
    Thing *person;

    if (room > room_info) {
        person = &thing[room->queue_head];
        if (person > thing) {
            while (1) {
                set_icon_value(person, ICON_QUEUE);
                if (person->queue_next == 0) {
                    break;
                }
                person = &thing[person->queue_next];
            }
            if (game_turn - 1 != last_flag_turn || last_flag_room != room)
                play_sound(SNDSRC_GENERAL, SFX_HLIGHTP2, -1, -1);
        }
    }
    last_flag_room = room;
    last_flag_turn = game_turn;
}
