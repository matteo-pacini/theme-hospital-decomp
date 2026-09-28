#include "hosp.h"

extern "C" {

extern Event D_001549d0[];
extern u16 D_00155419, D_0015541b;
extern u16 D_0015537a;
extern u32 D_001603d0;
extern RoomInfo *D_001603d4;

u16 fn_00085660(int n);

}

// NONMATCHING: register choice
// FUNCTION: 0x0004ace0
void front_queue(const u16 room_id, const u16 pos)
{
    RoomInfo *room;
    u16 size;
    u16 i;
    u16 id;
    Thing *w;
    Thing *head;

    room = &room_info[room_id];
    if (room > room_info) {
        if (pos) {
            if (size = queue_size(room)) {
                i = 1;
                if (id = room->queue_head) {
                    w = &thing[id];
                    while (1) {
                        if (pos == i) {
                            leave_queue(w);
                            id = room->queue_head;
                            head = &thing[id];
                            room->queue_head = w - thing;
                            room->unknown31++;
                            w->unknown140 = room - room_info;
                            w->queue_prev = 0;
                            w->queue_next = id;
                            head->queue_prev = w - thing;
                            return;
                        }
                        if (w->queue_next == 0) {
                            return;
                        }
                        id = w->queue_next;
                        w = &thing[id];
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
    Thing *w;

    room = &room_info[room_id];
    if (room > room_info) {
        if (pos) {
            i = 1;
            if (size = queue_size(room)) {
                if (id = room->queue_head) {
                    w = &thing[id];
                    while (1) {
                        if (pos == i) {
                            leave_queue(w);
                            join_queue(w, room);
                            return;
                        }
                        if (w->queue_next == 0) {
                            return;
                        }
                        id = w->queue_next;
                        w = &thing[id];
                        i++;
                    }
                }
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004af50
void join_queue(Thing *w, RoomInfo *room)
{
    u16 id;
    Thing *tail;

    leave_queue(w);
    if (id = room->queue_head) {
        tail = &thing[id];
        while (tail->queue_next != 0) {
            id = tail->queue_next;
            tail = &thing[id];
        }
        tail->queue_next = w - thing;
        w->queue_prev = id;
    } else {
        room->queue_head = w - thing;
        w->queue_prev = 0;
    }
    w->queue_next = 0;
    w->unknown140 = room - room_info;
    room->unknown31++;
    if (room->unknown32) {
        room->unknown32--;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0004b058
void leave_queue(Thing *w)
{
    u16 id;
    Thing *next;
    RoomInfo *room;

    room = &room_info[w->unknown140];
    if (room > room_info && room < end_room_info) {
        if (w->queue_prev == 0) {
            if ((u16)(w - thing) == room->queue_head) {
                id = w->queue_next;
                next = &thing[id];
                if (next > thing) {
                    next->queue_prev = 0;
                }
                room->queue_head = id;
            }
        } else if ((thing[w->queue_prev].queue_next = w->queue_next) > 0) {
            thing[w->queue_next].queue_prev = w->queue_prev;
        }
        if (room->unknown31 > 0) {
            room->unknown31--;
        }
    } else if (w->unknown89) {
        if (room_info[w->unknown89].type != 4) {
            if (room_info[w->unknown89].unknown32 != 0) {
                room_info[w->unknown89].unknown32--;
            }
        }
        w->unknown89 = 0;
    }
    w->queue_next = 0;
    w->queue_prev = 0;
    w->unknown140 = 0;
}

// FUNCTION: 0x0004b21c
u16 queue_size(RoomInfo *room)
{
    return room->unknown31;
}

// FUNCTION: 0x0004b244
u16 which_queue(Thing *w)
{
    Thing *head;
    RoomInfo *room;

    head = w;
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

// NONMATCHING: register choice
// FUNCTION: 0x0004b2f4
void process_queue_button(u8 n)
{
    debug.unknown8 = n;
    switch (n) {
    case 0:
        if (D_0015541b) {
            D_001549d0[player].type = 0x3c;
            D_001549d0[player].value = D_00155419;
            D_001549d0[player].arg = D_0015541b;
        }
        break;
    case 1:
        if (D_0015541b) {
            D_001549d0[player].type = 0x3d;
            D_001549d0[player].value = D_00155419;
            D_001549d0[player].arg = D_0015541b;
        }
        break;
    case 2:
        D_001549d0[player].type = 0x53;
        D_001549d0[player].value = D_00155419;
        break;
    case 3:
        D_001549d0[player].type = 0x54;
        D_001549d0[player].value = D_00155419;
        break;
    case 4:
        if (D_0015541b) {
            D_001549d0[player].type = 0x51;
            D_001549d0[player].value = D_00155419;
            D_001549d0[player].arg = D_0015541b;
        }
        break;
    case 5:
        if (D_0015541b) {
            D_001549d0[player].type = 0x55;
            D_001549d0[player].value = D_00155419;
            D_001549d0[player].arg = D_0015541b;
        }
        break;
    case 6:
        if (D_0015541b) {
            D_0015537a = get_real_person_from_room(D_00155419, D_0015541b);
            que.unknown2 = 3;
        }
        break;
    case 7:
        que.unknown2 = 2;
        break;
    case 8:
        if (statvar.unknownfd > 1) {
            if (D_0015541b) {
                D_001549d0[player].type = 0x7c;
                D_001549d0[player].value = get_real_person_from_room(D_00155419, D_0015541b);
                D_001549d0[player].arg = fn_00085660(1);
                D_0015541b = 0;
            }
        }
        break;
    case 9:
        if (statvar.unknownfd > 2) {
            if (D_0015541b) {
                D_001549d0[player].type = 0x7c;
                D_001549d0[player].value = get_real_person_from_room(D_00155419, D_0015541b);
                D_001549d0[player].arg = fn_00085660(2);
                D_0015541b = 0;
            }
        }
        break;
    case 10:
        if (statvar.unknownfd > 3) {
            if (D_0015541b) {
                D_001549d0[player].type = 0x7c;
                D_001549d0[player].value = get_real_person_from_room(D_00155419, D_0015541b);
                D_001549d0[player].arg = fn_00085660(3);
                D_0015541b = 0;
            }
        }
        break;
    }
}

// FUNCTION: 0x0004b654
void queue_person_go_home(const u16 room_id, const u16 pos)
{
    u16 id;
    Thing *w;

    id = get_real_person_from_room(room_id, pos);
    w = &thing[id];
    if (w > thing) {
        person_go_home(w);
    }
}

// FUNCTION: 0x0004b6b0
void queue_person_go_reception(const u16 room_id, const u16 pos)
{
    u16 id;
    Thing *w;

    id = get_real_person_from_room(room_id, pos);
    w = &thing[id];
    if (w > thing) {
        leave_queue(w);
        interrupt_person(w);
        start_person_going(w);
    }
}

// FUNCTION: 0x0004b71c
u16 get_real_person_from_room(const u16 room_id, const u16 pos)
{
    u32 i;
    RoomInfo *room;
    Thing *w;

    i = 1;
    room = &room_info[room_id];
    w = &thing[room->queue_head];
    if (w > thing) {
        while (1) {
            if (pos == i) {
                return w - thing;
            }
            if (w->queue_next == 0) {
                break;
            }
            w = &thing[w->queue_next];
            i++;
        }
    }
    return 0;
}

// NONMATCHING: register choice and frame slot order
// FUNCTION: 0x0004b7e0
void queue_person_to_new_queue(const u16 room_id, const u16 id)
{
    RoomInfo *room;
    Thing *w;
    u16 frame;

    room = &room_info[room_id];
    if (room > room_info) {
        w = &thing[id];
        if (w > thing) {
            if (person_ok_to_call(w)) {
                leave_queue(w);
                interrupt_person(w);
                if (room->type == 4) {
                    u8 reception = find_nearest_manned_reception(w, room - room_info, 0);
                    if (reception) {
                        fn_0004195c(w, thing[reception].room_next);
                    }
                } else {
                    room->unknown32++;
                    page_staff_i_need(w, room);
                    fn_0004195c(w, room->unknown4);
                }
                w->unknown89 = room - room_info;
                frame = check_frame(anim_starts[w->subtype].idle, w) + (u16)w->unknown17;
                w->sprite = start[frame].frame;
                w->palette = start[frame].flags > 0 ? (u8)1 : (u8)0;
                w->kind = 0x24;
            }
        }
    }
}

// FUNCTION: 0x0004b988
s16 spare_queue_capacity(const u16 type, const u8 waiting, RoomInfo *room, const u8 owner)
{
    Expertise *e;
    RoomInfo *r;
    s16 spare;

    e = &expertise[type];
    if (room == 0) {
        for (r = room_info + 1; r < end_room_info; r++) {
            if (r->f3 >= 0 && r->owner == owner) {
                if (r->type == e->category) {
                    spare = r->f1e - r->unknown31;
                    if (waiting) {
                        spare -= r->unknown32;
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
    Expertise *e;
    RoomInfo *r;
    s32 spare;

    total = 0;
    if (allhosps[owner].rooms[type].unknown1C == 0) {
        return 0;
    }
    e = &expertise[type];
    for (r = room_info + 1; r < end_room_info; r++) {
        if (r->f3 >= 0 && r->owner == owner) {
            if (r->type == e->category) {
                spare = r->f1e - r->unknown31;
                if (waiting) {
                    spare -= r->unknown32;
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
    Thing *w;

    while ((w = &thing[room->queue_head]) > thing) {
        interrupt_person(w);
        leave_queue(w);
        start_person_going(w);
    }
}

// NONMATCHING: second jbe to the epilogue is near instead of short
// FUNCTION: 0x0004bbec
void flag_all_queue_members(RoomInfo *room)
{
    Thing *w;

    if (room > room_info) {
        w = &thing[room->queue_head];
        if (w > thing) {
            while (1) {
                set_icon_value(w, 0x1a);
                if (w->queue_next == 0) {
                    break;
                }
                w = &thing[w->queue_next];
            }
            if (game_turn - 1 != D_001603d0 || D_001603d4 != room) {
                play_sound(10, 1, -1, -1);
                D_001603d4 = room;
                D_001603d0 = game_turn;
            }
        }
    }
}
