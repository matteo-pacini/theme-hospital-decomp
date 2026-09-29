#include "hosp.h"

struct SelectionState {
    u8 unknown0[0xd];
    u16 widget;
    u8 unknown15[0x1d];
};

extern "C" {

extern s16 D_0012cd00[];
extern SelectionState D_001581b2;
extern PlayerCall D_0015f5a4[][9];
extern u8 D_00158192;
extern char **D_00160370;
extern char **D_00160318;
extern char **D_001602a4;
extern char D_0012d8e8[];
extern char D_001201a8[];

// Initialiser data the compiler emits for a local char array in announce_new_illness_fax.
extern char D_0012d8ea[];
#pragma aux D_0012d8ea "W?$Wsi0dnew_line$n[]a$:.0$:?announce_new_illness_fax$n(xucpnpna)usnx[]a"

void fn_000444e0(Thing *w);
void fn_000b05f0(Thing *w);
void fn_0001673c(Thing *w);
void fn_000168a0(Thing *w);
void fn_00017cb0(Thing *w);
void fn_000c93bf(char *dst, char *src);

}

// FUNCTION: 0x00015c50
void person_waits_for_busy(Thing *w)
{
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
    }
    if (w->timer-- <= 0) {
        person_go_home(w);
    }
}

// FUNCTION: 0x00015cc0
void person_arrives_at_door(Thing *w)
{
    u16 tile;
    RoomInfo *info;

    tile = w->x.tile + (w->y.tile << 7);
    info = is_this_a_door(tile);
    if (info != 0 && rooms[info->type].unknown36 != 0) {
        set_complex(w, 0x32, 0);
        process_person_complex(w);
    } else {
        w->kind = 0x14;
    }
}

// NONMATCHING: frame layout: the u16 anim local and the ?: byte temporary swap slots
// FUNCTION: 0x00015d48
void person_in_queue(Thing *w)
{
    CompFrame *cur;
    RoomInfo *room;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        cur = &frame[w->sprite];
        if (cur->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                if (blocks[map[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(w)) {
                    return;
                }
                modify_person(w);
                if (w->unknown130 > 0) {
                    room = &room_info[which_queue(w)];
                    if (room > room_info) {
                        page_staff_i_need(w, room);
                    }
                    if (abilities[w->subtype][0] && w->unknown134 < 30 && w->icon != 60) {
                        if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 7, 1, 150, w->owner)) {
                            w->state_a = 0x18;
                            w->state_b = 0x18;
                            set_complex(w, 60, 0);
                            process_person_complex(w);
                            return;
                        }
                    }
                    if (w->icon != 70) {
                        u16 anim;

                        if (abilities[w->subtype][1]) {
                            if (find_nearest_free_object(w - thing, 4)) {
                                if (w->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                w->state_a = 0x18;
                                w->state_b = 0x18;
                                set_complex(w, 70, 0);
                                process_person_complex(w);
                                return;
                            }
                            if (w->owner == player) {
                                NRAdcounts.unknown6++;
                            }
                        }
                        anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->unknown17;
                        w->sprite = start[anim].frame;
                        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        if (find_standing_place(w)) {
                            return;
                        }
                    }
                    w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
                    w->kind = 0x18;
                }
            }
        }
    }
}

// NONMATCHING: frame layout: the u16 anim local and the ?: byte temporary swap slots
// FUNCTION: 0x000160a8
void person_entertains_themself(Thing *w)
{
    CompFrame *cur;
    RoomInfo *room;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        cur = &frame[w->sprite];
        if (cur->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                if (blocks[map[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(w)) {
                    return;
                }
                modify_person(w);
                if (w->unknown130 > 0) {
                    room = &room_info[which_queue(w)];
                    if (room > room_info) {
                        page_staff_i_need(w, room);
                    }
                    if (w->unknown124 > gbv.f108 || (w->flags & 0x100)) {
                        if (w->owner == player) {
                            othervar.f89++;
                        }
                        set_icon_value(w, 7);
                        person_go_home(w);
                        w->flags |= 0x4000;
                        return;
                    }
                    if (abilities[w->subtype][0] && w->unknown134 < 30 && w->icon != 60) {
                        if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 7, 1, 150, w->owner)) {
                            w->state_a = 0x18;
                            w->state_b = 0x18;
                            set_complex(w, 60, 0);
                            process_person_complex(w);
                            return;
                        }
                    }
                    if (abilities[w->subtype][2] && w->unknown136 > gbv.f116 && w->icon != 90) {
                        room = get_nearest_room_info(w, 0x1d);
                        if (room != 0) {
                            leave_queue(w);
                            w->state_a = 0x18;
                            w->state_b = 0x36;
                            w->unknown103 = 0;
                            set_complex(w, 360, room->unknown4);
                            process_person_complex(w);
                            return;
                        }
                    }
                    if (w->icon != 70) {
                        u16 anim;

                        if (abilities[w->subtype][1]) {
                            if (find_nearest_free_object(w - thing, 4)) {
                                if (w->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                w->state_a = 0x18;
                                w->state_b = 0x18;
                                set_complex(w, 70, 0);
                                process_person_complex(w);
                                return;
                            }
                            if (w->owner == player) {
                                NRAdcounts.unknown6++;
                            }
                        }
                        anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->unknown17;
                        w->sprite = start[anim].frame;
                        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        if (find_standing_place(w)) {
                            return;
                        }
                    }
                    w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
                }
            }
        }
    }
}

// FUNCTION: 0x000164f4
u16 find_standing_place(Thing *w)
{
    s16 base;
    u16 i;
    u16 tile;
    Map *target;
    Map *cell;

    cell = &map[((s16)w->y.tile << 7) + (s16)w->x.tile];
    if (w->prev != 0 || (cell->flags & 7)) {
        base = w->x.tile + (w->y.tile << 7);
        for (i = old_newrand() % 12 + 1; i <= D_0012cd00[0]; i++) {
            tile = base + D_0012cd00[i];
            target = &map[tile];
            if (target->first == 0 && !(target->flags & 7) && can_i_get_to(w, tile) > 0 && !(target->unknown7 & 0x10)) {
                break;
            }
        }
        if (D_0012cd00[i]) {
            w->state_a = w->kind;
            w->state_b = w->kind;
            set_complex(w, 430, tile);
            process_person_complex(w);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0001665c
void person_go_home(Thing *w)
{
    if (w->unknown91) {
        get_playercall(8, w->unknown91, w->owner);
        get_playercall(4, w->unknown91, w->owner);
        w->flags &= ~0x10;
        w->unknown91 = 0;
    }
    w->state_b = 0;
    record_person_as_patient(w);
    interrupt_person(w);
    leave_queue(w);
    if (set_complex(w, 20, get_room_info(1, 0x65, w->owner)->unknown4)) {
        process_person_complex(w);
    }
    set_icon_value(w, 7);
}

// FUNCTION: 0x0001673c
void fn_0001673c(Thing *w)
{
    remove_person(w);
}

// FUNCTION: 0x00016760
void remove_person(Thing *w)
{
    leave_queue(w);
    abandon_complex(w);
    fn_000168a0(w);
    cancel_vaccination_request(w);
    if (&thing[D_001581b2.widget] == w) {
        close_irequesters();
    }
    remove_thing(w);
}

// FUNCTION: 0x000167c4
void move_person_in_room_list(Thing *w, u16 id)
{
    RoomInfo *room;

    room = which_room(id);
    if (room != 0) {
        insert_person_in_room_list(room, w);
    } else {
        fn_000168a0(w);
    }
}

// FUNCTION: 0x00016808
void insert_person_in_room_list(RoomInfo *room, Thing *w)
{
    u16 first;
    int id;

    id = w - thing;
    if (room != 0) {
        first = room->first;
        if (first) {
            w->room_next = first;
            thing[first].room_prev = id;
        }
        room->first = id;
    }
    w->room = room;
    if (w->renderer == 2) {
        room->count++;
    }
}

// FUNCTION: 0x000168a0
void fn_000168a0(Thing *w)
{
    RoomInfo *room;
    Thing *prev;
    Thing *next;

    room = w->room;
    if (room != 0) {
        if (room->first) {
            prev = &thing[w->room_prev];
            if (prev > thing) {
                prev->room_next = w->room_next;
            } else {
                room->first = w->room_next;
            }
            next = &thing[w->room_next];
            if (next > thing) {
                next->room_prev = w->room_prev;
            }
        }
        w->room = 0;
        w->room_next = 0;
        w->room_prev = 0;
    }
}

// FUNCTION: 0x00016974
void nav_debug_state(Thing *w)
{
    if (w->renderer == 2) {
        if (actual_game_turn % 6 == 0) {
            u16 anim;

            w->unknown17 = *(u8 *)&next_angle[w->unknown17];
            anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->unknown17;
            w->sprite = start[anim].frame;
            w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(actual_game_turn & 0xf)) {
            w->kind = w->state_a;
            fn_000444e0(w);
        }
    } else {
        if (actual_game_turn % 6 == 0) {
            u16 anim;

            w->unknown17 = *(u8 *)&next_angle[w->unknown17];
            anim = check_frame(staff_anim_starts[w->subtype].unknownA, w) + (u16)w->unknown17;
            w->sprite = start[anim].frame;
            w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(actual_game_turn & 0xf)) {
            w->kind = w->state_a;
            fn_000444e0(w);
        }
    }
}

// FUNCTION: 0x00016b30
u8 transform_peep_who(Thing *w, u8 level)
{
    s8 subtype;

    subtype = change_person_who[level][w->subtype];
    if (w->renderer == 2) {
        w->unknown170 = w->subtype;
    }
    if (subtype != -1) {
        w->subtype = subtype;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00016ba8
u16 call_person_to_room(Thing *w, u16 id, u16 action, u16 arg, u8 follow)
{
    Thing *other;

    other = &thing[id];
    while (1) {
        if (other->unknown130 <= 0) {
            id = other->unknown122;
            if (id == 0) {
                break;
            }
            leave_queue(other);
            other = &thing[id];
        } else {
            if (person_ok_to_call(other)) {
                w->unknown73 = id;
                w->timer = 200;
                w->unknown122 = (old_newrand() & 3) + 3;
                leave_queue(other);
                if (set_complex(other, action, arg)) {
                    process_person_complex(other);
                }
                other->state_b = 0x46;
                return id;
            }
            if (follow == 0) {
                break;
            }
            id = other->unknown122;
            if (id == 0) {
                break;
            }
            other = &thing[id];
        }
    }
    return 0;
}

// FUNCTION: 0x00016cdc
u8 call_person_again(Thing *w, u16 action, u16 arg)
{
    Thing *other;
    u16 tile;

    other = &thing[w->unknown73];
    if (other > thing) {
        if (other->unknown130 <= 0) {
            return 0;
        }
        if (other->renderer != 2) {
            return 0;
        }
        if (other->unknown140) {
            return 0;
        }
        if (!person_ok_to_call(other)) {
            w->timer = 200;
            return 2;
        }
        if (other->unknown61 / 10 * 10 != action) {
            if (other->unknown61 == 0) {
                if (set_complex(other, action, arg)) {
                    process_person_complex(other);
                }
                other->state_b = 0x46;
            } else if (other->unknown63 != action) {
                if (other->unknown63 == 0) {
                    if (set_complex(other, action, arg)) {
                        process_person_complex(other);
                    }
                    other->state_b = 0x46;
                } else {
                    return 0;
                }
            }
        } else {
            tile = (u16)other->unknown19 + ((u16)other->unknown20 << 7);
            if (tile != arg) {
                return 0;
            }
        }
        if (other->state_b != 0x46) {
            other->state_b = 0x46;
        }
        w->timer = 200;
        w->unknown122--;
        if (w->unknown122 <= 0) {
            return 1;
        }
    } else {
        return 0;
    }
    return 2;
}

// NONMATCHING: frame layout: the u16 anim local and the ?: byte temporary swap slots
// FUNCTION: 0x00016eb0
void send_person_to_bed(Thing *w)
{
    RoomInfo *room;
    Thing *other;

    w->timer = old_newrand() % 300 + 300;
    for (room = room_info + 1; room < end_room_info; room++) {
        u16 anim;

        if (room->owner != w->owner || room->type != 9) {
            continue;
        }
        for (other = &thing[room->first_object]; other > thing; other = &thing[other->unknown71]) {
            if (other->subtype != 8 || other->unknown91 != 0) {
                continue;
            }
            page_member_of_staff(4, room - room_info, 1, 1, w->owner);
            w->unknown89 = room - room_info;
            fn_0004195c(w, room->unknown4);
            anim = check_frame(anim_starts[w->subtype].idle, w) + (u16)w->unknown17;
            w->sprite = start[anim].frame;
            w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
            w->kind = 0x24;
            room->unknown32++;
            return;
        }
    }
    room = get_expertise_dest(0x2a, w);
    room->unknown32++;
    page_member_of_staff(4, room - room_info, 1, 1, w->owner);
    w->unknown89 = room - room_info;
    {
        u16 anim;

        anim = check_frame(anim_starts[w->subtype].idle, w) + (u16)w->unknown17;
        w->sprite = start[anim].frame;
        w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
    }
    w->kind = 0x24;
    fn_0004195c(w, room->unknown4);
}

// FUNCTION: 0x00017194
void person_in_bed(Thing *w)
{
    if (w->timer != 0) {
        if (actual_game_turn % 30 == 0) {
            modify_person(w);
        }
        w->timer--;
        if (w->timer == 0) {
            set_complex(w, 240, 0);
            process_person_complex(w);
            setup_next_treatment(0, w);
        }
    }
}

// FUNCTION: 0x0001720c
u16 find_peep(u16 room_id, u8 random, u8 renderer, u8 kind, u8 owner)
{
    u16 count;
    RoomInfo *room;
    Thing *obj;

    count = 0;
    room = &room_info[room_id];
    if (random) {
        for (obj = &thing[room->first]; obj > thing; obj = &thing[obj->room_next]) {
            if (obj->owner == owner) {
                if (renderer && obj->renderer != renderer) {
                    continue;
                }
                if (kind && obj->kind != kind) {
                    continue;
                }
                count++;
            }
        }
        if (count == 0) {
            return 0;
        }
        count = old_newrand() % count + 1;
    } else {
        count = 1;
    }
    for (obj = &thing[room->first]; obj > thing; obj = &thing[obj->room_next]) {
        if (obj->owner == owner) {
            if (renderer && obj->renderer != renderer) {
                continue;
            }
            if (kind && obj->kind != kind) {
                continue;
            }
            if (--count == 0) {
                return obj - thing;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x000173c0
void start_person_going(Thing *w)
{
    u8 go;
    u16 done;
    u16 flag;
    u16 id;
    u16 arg;
    Thing *other;
    RoomInfo *room;

    done = 0;
    flag = 0;
    go = 0;
    if (w->room > room_info) {
        if (set_complex(w, 90, w->room->unknown6)) {
            go = 1;
        }
        flag = 1;
    }
    id = find_nearest_manned_reception(w, 0, 1);
    if (id) {
        other = &thing[id];
        arg = other->room_next;
        w->unknown89 = thing[other->unknown91].room - room_info;
        if (set_complex(w, 470, arg)) {
            go = 1;
        }
        done = 1;
    }
    if (done == 0) {
        room = &room_info[allhosps[w->owner].unknown1cf[old_newrand() & 3]];
        arg = room->unknown4;
        if (set_complex(w, 20, arg)) {
            go = 1;
        }
    }
    if (go) {
        process_person_complex(w);
    }
}

// FUNCTION: 0x0001752c
void person_nav_failure(Thing *w)
{
    RoomInfo *room;

    if (w->room == 0) {
        room = which_room(w->unknown19 + (w->unknown20 << 7));
        if (room != 0 && actual_game_turn - 500 < room->unknown22) {
            start_person_going(w);
            return;
        }
    }
    if (w->state_b == 0x14 || w->state_b == 0x18) {
        w->kind = w->state_b;
        return;
    }
    w->unknown105 = w->kind;
    w->unknown106 = w->unknown19;
    w->unknown107 = w->unknown20;
    w->kind = 0x3c;
    w->height = 0;
    if (w->owner == player) {
        announce_this(1, 1);
    }
}

// FUNCTION: 0x00017620
void person_fills(Thing *w)
{
    Thing *found;
    Thing *other;
    CompFrame *cur;

    other = &thing[w->unknown69];
    if (other > thing) {
        if (actual_game_turn % w->anim_speed == 0) {
            w->sprite = frame[w->sprite].next;
            other->unknown100 = frame[other->unknown100].next;
        }
        if (w->subtype == 14) {
            other->unknown103 += 2;
            cur = &frame[other->unknown100];
        } else {
            other->unknown103++;
            cur = &frame[w->sprite];
        }
        if (other->unknown103 > cur->length) {
            w->kind = 0x20;
            w->sprite = other->unknown100;
            w->palette = other->state_b;
            if (w->subtype == 14) {
                found = find_person_in_room(w->room, 4, 1);
                if (found != 0) {
                    process_things(found, found + 1);
                }
            }
        }
    }
}

// FUNCTION: 0x00017780
void person_energises(Thing *w)
{
    u16 next;
    Thing *other;

    other = &thing[w->unknown69];
    if (other > thing) {
        if (actual_game_turn % w->anim_speed == 0) {
            next = frame[w->sprite].next;
            if (!(frame[next].flags & 1)) {
                w->sprite = frame[w->sprite].next;
                other->unknown100 = frame[other->unknown100].next;
            }
        }
    }
}

// FUNCTION: 0x00017854
void person_in_bogs(Thing *w)
{
    if (w->unknown103 == 0) {
        if (find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x33, 1, 150, w->owner)) {
            w->state_b = 0x36;
            w->state_a = 0x38;
            w->unknown103 = 1;
            set_complex(w, 330, 0);
            process_person_complex(w);
        } else {
            w->kind = 0x38;
        }
    } else if (w->unknown103 == 1) {
        w->unknown124 = 0;
        if (w->style >= 0x10 && find_object(((u16)w->y.tile << 7) + (u16)w->x.tile, 0x20, 1, 150, w->owner)) {
            w->state_b = 0x38;
            w->state_a = 0x38;
            set_complex(w, 350, 0);
            process_person_complex(w);
        } else {
            w->kind = 0x38;
        }
    } else {
        w->kind = 0x38;
    }
}

// FUNCTION: 0x000179a8
void leave_toilets(Thing *w)
{
    w->state_b = 0x26;
    w->state_a = 0x38;
    set_complex(w, 90, w->room->unknown6);
    process_person_complex(w);
}

// FUNCTION: 0x000179f8
void return_to_queue_area(Thing *w)
{
    u16 id;
    u16 arg;
    RoomInfo *room;
    Thing *other;

    arg = 0;
    if (w->unknown140) {
        room = &room_info[w->unknown140];
        w->state_a = 0x18;
        w->state_b = 0x18;
        if (room->type == 4) {
            id = find_object_anywhere(11, 0, 0, w->owner);
            if (id) {
                other = &thing[id];
                arg = other->room_next;
            }
        } else {
            arg = room->unknown4;
        }
        if (arg) {
            set_complex(w, 30, arg);
            process_person_complex(w);
            return;
        }
    }
    w->kind = 0x18;
}

// FUNCTION: 0x00017ae0
void play_person_showanim(Thing *w)
{
    u16 next;
    CompFrame *cur;

    if (actual_game_turn % w->anim_speed == 0) {
        next = frame[w->sprite].next;
        cur = &frame[next];
        if (cur->flags & 1) {
            w->kind = w->unknown103;
            w->sprite = w->unknown85;
            w->palette = w->unknown104;
            w->unknown85 = w->unknown104 = w->unknown103 = 0;
            if (w->flags & 0x4) {
                place_litter(w->x.tile + (w->y.tile << 7), (s8)w->owner, 5);
                w->flags &= ~0x4;
            } else if (w->flags & 0x8) {
                place_litter(w->x.tile + (w->y.tile << 7), (s8)w->owner, 6);
                w->flags &= ~0x8;
            }
        } else {
            w->sprite = next;
        }
    }
}

// FUNCTION: 0x00017c28
void page_staff_i_need(Thing *w, RoomInfo *room)
{
    s8 count;

    if (room->type == 10) {
        count = 2;
    } else {
        count = 1;
    }
    page_member_of_staff(expertise[w->unknown171].unknown0, room - room_info, 1, count, w->owner);
}

// FUNCTION: 0x00017cb0
void fn_00017cb0(Thing *w)
{
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
    }
    if (w->height < 400) {
        w->height += w->unknown18;
    } else {
        remove_person(w);
    }
}

// FUNCTION: 0x00017d2c
void send_person_somewhere_else(Thing *w)
{
    w->state_b = 0x26;
    if (set_complex(w, 90, w->room->unknown6)) {
        process_person_complex(w);
    }
}

// FUNCTION: 0x00017d78
void refer_patient(Thing *w, u8 owner)
{
    u8 i;

    if (!(w->flags & 0x40)) {
        interrupt_person(w);
        leave_queue(w);
        w->owner = owner;
        w->unknown118 |= 1;
        if (w->flags & 0x20) {
            fn_000b05f0(w);
            w->unknown174 = 100;
            w->flags &= ~0x20;
            remove_person_from_vaccination_queue(w);
        }
        w->unknown169 = 1;
        w->unknown171 = 1;
        for (i = 0; i < 10; i++) {
            w->unknown148[i] = 0;
        }
        start_person_going(w);
    }
}

// NONMATCHING: frame layout: the u16 anim local and the ?: byte temporary swap slots
// FUNCTION: 0x00017e30
void person_waits_for_decision(Thing *w)
{
    u8 style;
    u8 category;
    u8 done;
    CompFrame *cur;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        cur = &frame[w->sprite];
        if (cur->flags & 1) {
            w->height--;
            if (w->height <= 0) {
                u16 anim;

                if (w->flags & 0x400) {
                    done = find_possible_diagnosis(w);
                    if (done || (u16)(othervar.f82 - w->timer) >= 1500) {
                        w->flags &= ~0x10;
                        reset_icon_value(w, 0x1b);
                        if (!(w->flags & 0x80)) {
                            get_playercall(8, w->unknown91, w->owner);
                            w->unknown91 = 0;
                        }
                        if (done) {
                            w->kind = 0x26;
                        } else {
                            person_go_home(w);
                            allhosps[w->owner].rooms[w->style].unknown24++;
                            w->unknown118 |= 8;
                        }
                        w->flags &= 0xfb7f;
                        return;
                    }
                } else {
                    if (w->unknown172[0]) {
                        style = w->unknown172[0];
                    } else {
                        style = w->unknown171;
                    }
                    category = expertise[style].category;
                    done = allhosps[w->owner].rooms[style].unknown1C == 1 && listhead[w->owner].rooms[category]
                        ? (u8)1 : (u8)0;
                    if (done || (u16)(othervar.f82 - w->timer) >= 1500) {
                        w->flags &= ~0x10;
                        reset_icon_value(w, 0x1b);
                        if (w->flags & 0x80) {
                            w->flags &= ~0x80;
                            if (!done) {
                                person_go_home(w);
                                allhosps[w->owner].rooms[w->style].unknown24++;
                                w->unknown118 |= 0x10;
                            } else {
                                get_playercall(4, w->unknown91, w->owner);
                                w->kind = 0x26;
                                w->unknown91 = 0;
                            }
                        } else {
                            person_go_home(w);
                            w->unknown118 |= 0x10;
                        }
                        return;
                    }
                }
                if (blocks[map[((s16)w->y.tile << 7) + (s16)w->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(w)) {
                    return;
                }
                anim = check_frame(anim_starts[w->subtype].unknown12, w) + (u16)w->unknown17;
                w->sprite = start[anim].frame;
                w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                if (find_standing_place(w)) {
                    return;
                }
                w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
            }
        }
    }
}

// FUNCTION: 0x0001824c
u16 make_a_playercall(const u16 type, const u16 x, const s16 y, u8 owner)
{
    PlayerCall *p;

    if (gamevar.f174) {
        return 0;
    }
    for (p = playercall + 1; p < end_playercall; p++) {
        if (p->type == 0) {
            switch (type) {
            case 3:
                if (owner == player) {
                    play_ambient(saferand() % 3 + 0xaf);
                }
                break;
            case 5:
                switch (x) {
                case 3:
                    if (owner == player) {
                        play_ambient(saferand() % 3 + 0xb2);
                    }
                    break;
                }
                break;
            }
            p->type = type;
            p->id = popup_id[0]++;
            p->active = 1;
            p->x = x;
            p->timer = y;
            p->owner = owner;
            add_panel_popup(p);
            if (listhead[owner].first[type] == 0) {
                listhead[owner].first[type] = p - playercall;
            }
            return p->id;
        }
    }
    return 0;
}

// FUNCTION: 0x00018438
void get_new_popup_head(u16 type, u8 owner)
{
    int unused = 0;
    u16 min = 0xffff;
    PlayerCall *p;

    for (p = playercall + 1; p < end_playercall; p++) {
        if (p->type != type || p->owner != owner) {
            continue;
        }
        if (min > p->id) {
            listhead[owner].first[type] = p - playercall;
            min = p->id;
        }
    }
}

// FUNCTION: 0x000184f4
void save_popup(u16 type)
{
    u8 i;
    int n = 1;
    PlayerCall *p;

    if (statvar.unknownfd > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        p = playercall + listhead[i].first[type];
        if (p->type == 0) {
            return;
        }
        memmove(&D_0015f5a4[i][type], p, 12);
        delete_panel_popup(p);
        p->type = 0;
        listhead[i].first[type] = 0;
        get_new_popup_head(type, i);
    }
}

// FUNCTION: 0x00018620
void restore_popup(u16 type)
{
    u8 i;
    u16 unused = 0xffff;
    int n = 1;
    PlayerCall *p;

    if (statvar.unknownfd > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        for (p = playercall + 1; p < end_playercall; p++) {
            if (p->type == 0) {
                memmove(p, &D_0015f5a4[i][type], 12);
                add_panel_popup(p);
                break;
            }
        }
        get_new_popup_head(type, i);
    }
}

// FUNCTION: 0x00018710
u16 get_playercall(const u16 type, u16 id, u8 owner)
{
    u16 result = 0;
    PlayerCall *p;

    for (p = playercall + 1; p < end_playercall; p++) {
        if ((int)p->type != (int)type || p->owner != owner) {
            continue;
        }
        if (id == 0 || id && id == p->id) {
            p->type = 0;
            result = p->x;
            delete_panel_popup(p);
            if (listhead[owner].first[type] == p - playercall) {
                listhead[owner].first[type] = 0;
                get_new_popup_head(type, owner);
            }
            break;
        }
    }
    return result;
}

// FUNCTION: 0x00018838
u16 check_playercall(u16 type, u16 x, u8 owner)
{
    int count = 0;
    PlayerCall *p;

    for (p = playercall + 1; p < end_playercall; p++) {
        if (p->type != type || p->owner != owner) {
            continue;
        }
        if (type == 4) {
            count++;
        } else if (type == 8) {
            count++;
        } else if (type == 2) {
            count++;
        } else if (type == 3) {
            count++;
        } else if (p->x == x) {
            count++;
        }
    }
    return count;
}

// FUNCTION: 0x00018908
void expire_playercalls(void)
{
    u8 owner1;
    u8 changed = 0;
    u8 owner2;
    PlayerCall *p;

    for (p = playercall + 1; p < end_playercall; p++) {
        if (p->timer) {
            p->timer--;
            if (p->timer > 0) {
                continue;
            }
            if (p->type == 6) {
                owner1 = p->owner;
                allhosps[owner1].f1f3 = actual_game_turn + 0x5dc;
                allhosps[owner1].f1fa = 2;
            } else if (p->type == 3) {
                choice_spare_data = 0x63;
                packet_choice_result(0xb);
            } else if (p->type == 5) {
                if (p->x == 1) {
                    allhosps[p->owner].f1e2 = 0;
                }
            }
            owner2 = p->owner;
            if (listhead[owner2].first[p->type] == p - playercall) {
                listhead[owner2].first[p->type] = 0;
                changed = 1;
            }
            delete_panel_popup(p);
            p->type = 0;
        }
    }
    if (changed) {
        for (p = playercall + 1; p < end_playercall; p++) {
            if (p->type && listhead[p->owner].first[p->type] == 0) {
                listhead[p->owner].first[p->type] = p - playercall;
            }
        }
    }
}

// FUNCTION: 0x00018b78
u16 call_no_treatment(Thing *e, char **strs)
{
    char buf3[0xc8];
    char buf2[0x7d0];
    char buf1[0xc8];
    int room;
    int unused30;
    int staff;
    int i;
    Expertise *info;
    u16 b;
    u16 unused1c;
    s16 pct;
    u16 v14;
    u16 a;
    u16 sep;

    room = 0;
    staff = 0;
    unused30 = 0;
    unused1c = 0;
    sep = *(u16 *)D_0012d8e8;
    if (e <= thing) {
        return 0;
    }
    v14 = lbFontPtr ? lbFontPtr[34].height : 0;
    info = &expertise[e->style];
    if (listhead[e->owner].rooms[info->category] == 0 && room_available((u16)info->category)) {
        room = info->category;
    }
    for (i = 0; i < 9; i++) {
        if ((info->unknown0 & (1 << i)) && listhead[e->owner].staff[i] == 0) {
            staff = i;
            break;
        }
    }
    if (room && staff) {
        sprintf(buf1, strs[0], D_00160370[rooms[room].id], D_00160318[staff]);
    } else if (room) {
        sprintf(buf1, strs[1], D_00160370[rooms[room].id]);
    } else if (staff) {
        sprintf(buf1, strs[2], D_00160318[staff]);
    } else {
        sprintf(buf1, strs[3]);
    }
    if (allhosps[e->owner].rooms[0x2c].unknown1C != 1) {
        available_buttons &= ~4;
    }
    a = 0;
    b = 0;
    if (e->flags32 & 0x1000) {
        pct = e->unknown146 * 100 / gbv.ffc;
        if (pct < 0) {
            pct = 0;
        } else if (pct > 99) {
            pct = 99;
        }
        sprintf(buf2, strs[6], pct, D_001602a4[e->style]);
    } else {
        sprintf(buf2, strs[4], D_001602a4[e->style]);
    }
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf1);
    sprintf(buf3, strs[5]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    LbTextDraw(a, b, buf2);
    return 0;
}

// FUNCTION: 0x00018f20
void process_no_treatment_buttons(u8 n)
{
    switch (n) {
    case 0:
        packet_choice_result(0);
        break;
    case 1:
        packet_choice_result(1);
        break;
    case 2:
        packet_choice_result(2);
        break;
    }
}

// FUNCTION: 0x00018f88
void person_arrived_from_call(Thing *e)
{
    Thing *q;

    if (e->room != 0) {
        for (q = thing + e->room->first; q > thing; q = thing + q->room_next) {
            if (q->renderer == 4 && (u16)(e - thing) == q->unknown73) {
                return;
            }
        }
        set_complex(e, 0x5a, e->room->unknown6);
        e->state_b = 0x26;
        process_person_complex(e);
    }
}

// FUNCTION: 0x0001905c
u32 person_ok_to_call(Thing *e)
{
    RoomInfo *r = e->room;

    if (r != 0) {
        if (r->type == 0x1d) {
            return 0;
        }
    } else if (e->state_b == 0x36) {
        return 0;
    }
    return 1;
}

// FUNCTION: 0x000190c4
u16 announce_new_illness_fax(const u8 kind, char **strs)
{
    char buf3[0xc8];
    char buf2[0x7d0];
    char buf1[0xc8];
    int i;
    Expertise *info;
    u16 v28;
    u16 sep;
    int unused18;
    int unused14;
    int staff;
    int room;
    u16 b;
    u16 a;

    room = 0;
    staff = 0;
    unused14 = 0;
    unused18 = 0;
    sep = *(u16 *)D_0012d8ea;
    if (kind <= 1) {
        return 0;
    }
    v28 = lbFontPtr ? lbFontPtr[34].height : 0;
    info = &expertise[kind];
    if (listhead[player].rooms[info->category] == 0 && room_available((u16)info->category)) {
        room = info->category;
    }
    for (i = 0; i < 9; i++) {
        if ((info->unknown0 & (1 << i)) && listhead[player].staff[i] == 0) {
            staff = i;
            break;
        }
    }
    if (allhosps[player].rooms[kind].unknown1C == 1) {
        sprintf(buf1, strs[0]);
    } else if (room && staff) {
        sprintf(buf1, strs[1], D_00160370[rooms[room].id], D_00160318[staff]);
    } else if (room) {
        sprintf(buf1, strs[2], D_00160370[rooms[room].id]);
    } else if (staff) {
        sprintf(buf1, strs[3], D_00160318[staff]);
    } else {
        sprintf(buf1, D_001201a8);
    }
    buf2[0] = 0;
    a = 0;
    b = 0;
    sprintf(buf3, strs[4], D_001602a4[kind]);
    fn_000c93bf(buf2, buf3);
    fn_000c93bf(buf2, (char *)&sep);
    sprintf(buf3, strs[kind * 3 - 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    sprintf(buf3, strs[kind * 3 - 1 + 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    sprintf(buf3, strs[kind * 3 + 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    LbTextDraw(a, b, buf2);
    return 0;
}

// FUNCTION: 0x00019464
void process_announce_new_illness_buttons(u8 n)
{
    switch (n) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000194a0
void force_peep_to_be_cured(Thing *e)
{
    e->state_b = 0;
    e->flags32 = 0;
    leave_queue(e);
    interrupt_person(e);
    best_guess_treatment(e);
    next_treatment_now_setup(e, 0);
}

// FUNCTION: 0x000194f0
void interrupt_person(Thing *e)
{
    u8 was44 = e->kind == 0x44 || e->state_b == 0x44;
    u8 state;
    Thing *q;

    if (e->unknown140) {
        state = 8;
    } else if (e->unknown89) {
        state = 0x24;
    } else {
        leave_queue(e);
        state = 0x26;
    }
    interrupt_complex(e);
    if (e->room) {
        for (q = thing + e->room->first; q > thing; q = thing + q->room_next) {
            if (q->renderer == 4 && (u16)(e - thing) == q->unknown73) {
                q->unknown73 = 0;
                interrupt_staff(q);
                break;
            }
        }
        switch (e->subtype) {
        case 4:
        case 5:
            set_complex(e, 0xb4, 0);
            break;
        case 9:
        case 10:
            e->state_a = 0;
            set_complex(e, 0x276, 0);
            break;
        }
        set_complex(e, 0x5a, e->room->unknown6);
        process_person_complex(e);
        e->state_b = state;
    } else if (was44) {
        e->kind = 0x44;
    } else {
        e->kind = state;
    }
}

// FUNCTION: 0x0002bb58
void remove_person_from_room_list(Thing *w)
{
    u16 *staff;

    if (w->room) {
        staff = w->room->staff;
        if (staff[0] == (u16)(w - thing)) {
            staff[0] = 0;
        } else if (staff[1] == (u16)(w - thing)) {
            staff[1] = 0;
        }
    }
}
