#include "hosp.h"

extern "C" {

extern Event D_001549d0[];
extern u32 D_001603d0;
extern RoomInfo *D_001603d4;

u16 nth_other_active_player(int n);

}

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
                            room->unknown31++;
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
    room->unknown31++;
    if (room->unknown32) {
        room->unknown32--;
    }
}

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
        if (room->unknown31 > 0) {
            room->unknown31--;
        }
    } else if (person->dest_room) {
        if (room_info[person->dest_room].type != ROOM_RECEPTION) {
            if (room_info[person->dest_room].unknown32 != 0) {
                room_info[person->dest_room].unknown32--;
            }
        }
        person->dest_room = 0;
    }
    person->queue_next = 0;
    person->queue_prev = 0;
    person->queue_room = 0;
}

// FUNCTION: 0x0004b21c
u16 queue_size(RoomInfo *room)
{
    return room->unknown31;
}

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

// FUNCTION: 0x0004b2f4
void process_queue_button(u8 button)
{
    debug.unknown8 = button;
    switch (button) {
    case 0:
        if (othervar.f63) {
            D_001549d0[player].type = EV_FRONT_QUEUE;
            D_001549d0[player].value = othervar.f61;
            D_001549d0[player].arg = othervar.f63;
        }
        break;
    case 1:
        if (othervar.f63) {
            D_001549d0[player].type = EV_BACK_QUEUE;
            D_001549d0[player].value = othervar.f61;
            D_001549d0[player].arg = othervar.f63;
        }
        break;
    case 2:
        D_001549d0[player].type = 0x53;
        D_001549d0[player].value = othervar.f61;
        break;
    case 3:
        D_001549d0[player].type = 0x54;
        D_001549d0[player].value = othervar.f61;
        break;
    case 4:
        if (othervar.f63) {
            D_001549d0[player].type = EV_QUEUE_PERSON_GO_HOME;
            D_001549d0[player].value = othervar.f61;
            D_001549d0[player].arg = othervar.f63;
        }
        break;
    case 5:
        if (othervar.f63) {
            D_001549d0[player].type = EV_QUEUE_PERSON_GO_RECEPTION;
            D_001549d0[player].value = othervar.f61;
            D_001549d0[player].arg = othervar.f63;
        }
        break;
    case 6:
        if (othervar.f63) {
            gamevar.f13a = get_real_person_from_room(othervar.f61, othervar.f63);
            que.unknown2 = 3;
        }
        break;
    case 7:
        que.unknown2 = 2;
        break;
    case 8:
        if (statvar.unknownfd > 1) {
            if (othervar.f63) {
                D_001549d0[player].type = EV_REFER_PATIENT;
                D_001549d0[player].value = get_real_person_from_room(othervar.f61, othervar.f63);
                D_001549d0[player].arg = nth_other_active_player(1);
                othervar.f63 = 0;
            }
        }
        break;
    case 9:
        if (statvar.unknownfd > 2) {
            if (othervar.f63) {
                D_001549d0[player].type = EV_REFER_PATIENT;
                D_001549d0[player].value = get_real_person_from_room(othervar.f61, othervar.f63);
                D_001549d0[player].arg = nth_other_active_player(2);
                othervar.f63 = 0;
            }
        }
        break;
    case 10:
        if (statvar.unknownfd > 3) {
            if (othervar.f63) {
                D_001549d0[player].type = EV_REFER_PATIENT;
                D_001549d0[player].value = get_real_person_from_room(othervar.f61, othervar.f63);
                D_001549d0[player].arg = nth_other_active_player(3);
                othervar.f63 = 0;
            }
        }
        break;
    }
}

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

// FUNCTION: 0x0004b7e0
void queue_person_to_new_queue(const u16 room_id, const u16 id)
{
    RoomInfo *room;
    Thing *person;
    u8 reception;

    room = &room_info[room_id];
    if (room > room_info) {
        person = &thing[id];
        if (person > thing) {
            if (person_ok_to_call(person)) {
                leave_queue(person);
                interrupt_person(person);
                if (room->type == ROOM_RECEPTION) {
                    reception = find_nearest_manned_reception(person, room - room_info, 0);
                    if (reception) {
                        set_destination_coords(person, thing[reception].use_pos[1]);
                    }
                } else {
                    room->unknown32++;
                    page_staff_i_need(person, room);
                    set_destination_coords(person, room->unknown4);
                }
                person->dest_room = room - room_info;
                {
                    u16 anim;

                    anim = check_frame(anim_starts[person->subtype].idle, person) + (u16)person->facing;
                    person->sprite = start[anim].frame;
                    person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                }
                person->state = PS_GOTO_QUEUE;
            }
        }
    }
}

// FUNCTION: 0x0004b988
s16 spare_queue_capacity(const u16 type, const u8 waiting, RoomInfo *room, const u8 owner)
{
    Expertise *exp;
    RoomInfo *cand;
    s16 spare;

    exp = &expertise[type];
    if (room == 0) {
        for (cand = room_info + 1; cand < end_room_info; cand++) {
            if (cand->f3 >= 0 && cand->owner == owner) {
                if (cand->type == exp->category) {
                    spare = cand->f1e - cand->unknown31;
                    if (waiting) {
                        spare -= cand->unknown32;
                    }
                    if (spare > 0) {
                        return spare;
                    }
                }
            }
        }
    } else {
        spare = room->f1e - room->unknown31;
        if (waiting) {
            spare -= room->unknown32;
        }
        if (spare > 0) {
            return spare;
        }
    }
    return 0;
}

// FUNCTION: 0x0004ba8c
s16 all_queue_capacity(const u16 type, const u8 waiting, const u8 owner)
{
    s32 total;
    Expertise *exp;
    RoomInfo *cand;
    s32 spare;

    total = 0;
    if (allhosps[owner].rooms[type].unknown1C == 0) {
        return 0;
    }
    exp = &expertise[type];
    for (cand = room_info + 1; cand < end_room_info; cand++) {
        if (cand->f3 >= 0 && cand->owner == owner) {
            if (cand->type == exp->category) {
                spare = cand->f1e - cand->unknown31;
                if (waiting) {
                    spare -= cand->unknown32;
                }
                if (spare > 0) {
                    total += spare;
                }
            }
        }
    }
    return total;
}

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
            if (game_turn - 1 != D_001603d0 || D_001603d4 != room)
                play_sound(10, SFX_HLIGHTP2, -1, -1);
        }
    }
    D_001603d4 = room;
    D_001603d0 = game_turn;
}
