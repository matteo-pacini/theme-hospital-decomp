#include "hosp.h"

struct SelectionState {
    u8 unknown0[0xd];
    u16 widget;
    u8 unknown15[0x1d];
};

extern "C" {

extern SelectionState D_001581b2;
extern PlayerCall D_0015f5a4[][9];
extern u8 D_00158192;
extern char **D_00160370;
extern char **D_00160318;
extern char **D_001602a4;

void fn_000444e0(Thing *w);
void fn_000b05f0(Thing *w);
void fn_0001673c(Thing *w);
void fn_000168a0(Thing *w);
void fn_00017cb0(Thing *w);
void fn_000c93bf(char *dst, char *src);

}

// FUNCTION: 0x00015c50
void person_waits_for_busy(Thing *person)
{
    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
    }
    if (person->timer-- <= 0) {
        person_go_home(person);
    }
}

// FUNCTION: 0x00015cc0
void person_arrives_at_door(Thing *person)
{
    u16 tile;
    RoomInfo *info;

    tile = person->x.tile + (person->y.tile << 7);
    info = is_this_a_door(tile);
    if (info != 0 && rooms[info->type].unknown36 != 0) {
        set_complex(person, 0x32, 0);
        process_person_complex(person);
    } else {
        person->kind = 0x14;
    }
}

// FUNCTION: 0x00015d48
void person_in_queue(Thing *person)
{
    CompFrame *cur;
    RoomInfo *room;

    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
        cur = &frame[person->sprite];
        if (cur->flags & 1) {
            person->height--;
            if (person->height <= 0) {
                if (blocks[map[((s16)person->y.tile << 7) + (s16)person->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(person)) {
                    return;
                }
                modify_person(person);
                if (person->unknown130 > 0) {
                    room = &room_info[which_queue(person)];
                    if (room > room_info) {
                        page_staff_i_need(person, room);
                    }
                    if (abilities[person->subtype][0] && person->unknown134 < 30 && person->icon != 60) {
                        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, 7, 1, 150, person->owner)) {
                            person->state_a = 0x18;
                            person->state_b = 0x18;
                            set_complex(person, 60, 0);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (person->icon != 70) {
                        if (abilities[person->subtype][1]) {
                            if (find_nearest_free_object(person - thing, 4)) {
                                if (person->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                person->state_a = 0x18;
                                person->state_b = 0x18;
                                set_complex(person, 70, 0);
                                process_person_complex(person);
                                return;
                            }
                            if (person->owner == player) {
                                NRAdcounts.unknown6++;
                            }
                        }
                        {
                            u16 anim;

                            anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->unknown17;
                            person->sprite = start[anim].frame;
                            person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        }
                        if (find_standing_place(person)) {
                            return;
                        }
                    }
                    person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
                    person->kind = 0x18;
                }
            }
        }
    }
}

// FUNCTION: 0x000160a8
void person_entertains_themself(Thing *person)
{
    CompFrame *cur;
    RoomInfo *room;

    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
        cur = &frame[person->sprite];
        if (cur->flags & 1) {
            person->height--;
            if (person->height <= 0) {
                if (blocks[map[((s16)person->y.tile << 7) + (s16)person->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(person)) {
                    return;
                }
                modify_person(person);
                if (person->unknown130 > 0) {
                    room = &room_info[which_queue(person)];
                    if (room > room_info) {
                        page_staff_i_need(person, room);
                    }
                    if (person->unknown124 > gbv.f108 || (person->flags & 0x100)) {
                        if (person->owner == player) {
                            othervar.f89++;
                        }
                        set_icon_value(person, 7);
                        person_go_home(person);
                        person->flags |= 0x4000;
                        return;
                    }
                    if (abilities[person->subtype][0] && person->unknown134 < 30 && person->icon != 60) {
                        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, 7, 1, 150, person->owner)) {
                            person->state_a = 0x18;
                            person->state_b = 0x18;
                            set_complex(person, 60, 0);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (abilities[person->subtype][2] && person->unknown136 > gbv.f116 && person->icon != 90) {
                        room = get_nearest_room_info(person, 0x1d);
                        if (room != 0) {
                            leave_queue(person);
                            person->state_a = 0x18;
                            person->state_b = 0x36;
                            person->unknown103 = 0;
                            set_complex(person, 360, room->unknown4);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (person->icon != 70) {
                        if (abilities[person->subtype][1]) {
                            if (find_nearest_free_object(person - thing, 4)) {
                                if (person->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                person->state_a = 0x18;
                                person->state_b = 0x18;
                                set_complex(person, 70, 0);
                                process_person_complex(person);
                                return;
                            }
                            if (person->owner == player) {
                                NRAdcounts.unknown6++;
                            }
                        }
                        {
                            u16 anim;

                            anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->unknown17;
                            person->sprite = start[anim].frame;
                            person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        }
                        if (find_standing_place(person)) {
                            return;
                        }
                    }
                    person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
                }
            }
        }
    }
}

// FUNCTION: 0x000164f4
u16 find_standing_place(Thing *person)
{
    s16 base;
    u16 i;
    u16 tile;
    Map *target;
    Map *cell;

    cell = &map[((s16)person->y.tile << 7) + (s16)person->x.tile];
    if (person->prev != 0 || (cell->flags & 7)) {
        base = person->x.tile + (person->y.tile << 7);
        for (i = old_newrand() % 12 + 1; i <= standing_place[0]; i++) {
            tile = base + standing_place[i];
            target = &map[tile];
            if (target->first == 0 && !(target->flags & 7) && can_i_get_to(person, tile) > 0 && !(target->unknown7 & 0x10)) {
                break;
            }
        }
        if (standing_place[i]) {
            person->state_a = person->kind;
            person->state_b = person->kind;
            set_complex(person, 430, tile);
            process_person_complex(person);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0001665c
void person_go_home(Thing *person)
{
    if (person->unknown91) {
        get_playercall(8, person->unknown91, person->owner);
        get_playercall(4, person->unknown91, person->owner);
        person->flags &= ~0x10;
        person->unknown91 = 0;
    }
    person->state_b = 0;
    record_person_as_patient(person);
    interrupt_person(person);
    leave_queue(person);
    if (set_complex(person, 20, get_room_info(1, 0x65, person->owner)->unknown4)) {
        process_person_complex(person);
    }
    set_icon_value(person, 7);
}

// FUNCTION: 0x0001673c
void fn_0001673c(Thing *person)
{
    remove_person(person);
}

// FUNCTION: 0x00016760
void remove_person(Thing *person)
{
    leave_queue(person);
    abandon_complex(person);
    fn_000168a0(person);
    cancel_vaccination_request(person);
    if (&thing[D_001581b2.widget] == person) {
        close_irequesters();
    }
    remove_thing(person);
}

// FUNCTION: 0x000167c4
void move_person_in_room_list(Thing *person, u16 id)
{
    RoomInfo *room;

    room = which_room(id);
    if (room != 0) {
        insert_person_in_room_list(room, person);
    } else {
        fn_000168a0(person);
    }
}

// FUNCTION: 0x00016808
void insert_person_in_room_list(RoomInfo *room, Thing *person)
{
    u16 first;
    int id;

    id = person - thing;
    if (room != 0) {
        first = room->first;
        if (first) {
            person->room_next = first;
            thing[first].room_prev = id;
        }
        room->first = id;
    }
    person->room = room;
    if (person->renderer == 2) {
        room->count++;
    }
}

// FUNCTION: 0x000168a0
void fn_000168a0(Thing *person)
{
    RoomInfo *room;
    Thing *prev;
    Thing *next;

    room = person->room;
    if (room != 0) {
        if (room->first) {
            prev = &thing[person->room_prev];
            if (prev > thing) {
                prev->room_next = person->room_next;
            } else {
                room->first = person->room_next;
            }
            next = &thing[person->room_next];
            if (next > thing) {
                next->room_prev = person->room_prev;
            }
        }
        person->room = 0;
        person->room_next = 0;
        person->room_prev = 0;
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
u8 transform_peep_who(Thing *person, u8 level)
{
    s8 subtype;

    subtype = change_person_who[level][person->subtype];
    if (person->renderer == 2) {
        person->unknown170 = person->subtype;
    }
    if (subtype != -1) {
        person->subtype = subtype;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00016ba8
u16 call_person_to_room(Thing *caller, u16 id, u16 action, u16 arg, u8 follow)
{
    Thing *callee;

    callee = &thing[id];
    while (1) {
        if (callee->unknown130 <= 0) {
            id = callee->unknown122;
            if (id == 0) {
                break;
            }
            leave_queue(callee);
            callee = &thing[id];
        } else {
            if (person_ok_to_call(callee)) {
                caller->unknown73 = id;
                caller->timer = 200;
                caller->unknown122 = (old_newrand() & 3) + 3;
                leave_queue(callee);
                if (set_complex(callee, action, arg)) {
                    process_person_complex(callee);
                }
                callee->state_b = 0x46;
                return id;
            }
            if (follow == 0) {
                break;
            }
            id = callee->unknown122;
            if (id == 0) {
                break;
            }
            callee = &thing[id];
        }
    }
    return 0;
}

// FUNCTION: 0x00016cdc
u8 call_person_again(Thing *caller, u16 action, u16 arg)
{
    Thing *callee;
    u16 tile;

    callee = &thing[caller->unknown73];
    if (callee > thing) {
        if (callee->unknown130 <= 0) {
            return 0;
        }
        if (callee->renderer != 2) {
            return 0;
        }
        if (callee->unknown140) {
            return 0;
        }
        if (!person_ok_to_call(callee)) {
            caller->timer = 200;
            return 2;
        }
        if (callee->unknown61 / 10 * 10 != action) {
            if (callee->unknown61 == 0) {
                if (set_complex(callee, action, arg)) {
                    process_person_complex(callee);
                }
                callee->state_b = 0x46;
            } else if (callee->unknown63 != action) {
                if (callee->unknown63 == 0) {
                    if (set_complex(callee, action, arg)) {
                        process_person_complex(callee);
                    }
                    callee->state_b = 0x46;
                } else {
                    return 0;
                }
            }
        } else {
            tile = (u16)callee->unknown19 + ((u16)callee->unknown20 << 7);
            if (tile != arg) {
                return 0;
            }
        }
        if (callee->state_b != 0x46) {
            callee->state_b = 0x46;
        }
        caller->timer = 200;
        caller->unknown122--;
        if (caller->unknown122 <= 0) {
            return 1;
        }
    } else {
        return 0;
    }
    return 2;
}

// FUNCTION: 0x00016eb0
void send_person_to_bed(Thing *person)
{
    RoomInfo *room;
    Thing *other;

    person->timer = old_newrand() % 300 + 300;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->owner != person->owner || room->type != 9) {
            continue;
        }
        for (other = &thing[room->first_object]; other > thing; other = &thing[other->unknown71]) {
            if (other->subtype != 8 || other->unknown91 != 0) {
                continue;
            }
            page_member_of_staff(4, room - room_info, 1, 1, person->owner);
            person->unknown89 = room - room_info;
            fn_0004195c(person, room->unknown4);
            {
                u16 anim;

                anim = check_frame(anim_starts[person->subtype].idle, person) + (u16)person->unknown17;
                person->sprite = start[anim].frame;
                person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
            }
            person->kind = 0x24;
            room->unknown32++;
            return;
        }
    }
    room = get_expertise_dest(0x2a, person);
    room->unknown32++;
    page_member_of_staff(4, room - room_info, 1, 1, person->owner);
    person->unknown89 = room - room_info;
    {
        u16 anim;

        anim = check_frame(anim_starts[person->subtype].idle, person) + (u16)person->unknown17;
        person->sprite = start[anim].frame;
        person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
    }
    person->kind = 0x24;
    fn_0004195c(person, room->unknown4);
}

// FUNCTION: 0x00017194
void person_in_bed(Thing *person)
{
    if (person->timer != 0) {
        if (actual_game_turn % 30 == 0) {
            modify_person(person);
        }
        person->timer--;
        if (person->timer == 0) {
            set_complex(person, 240, 0);
            process_person_complex(person);
            setup_next_treatment(0, person);
        }
    }
}

// FUNCTION: 0x0001720c
u16 find_peep(u16 room_id, u8 random, u8 renderer, u8 kind, u8 owner)
{
    u16 count;
    RoomInfo *room;
    Thing *person;

    count = 0;
    room = &room_info[room_id];
    if (random) {
        for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
            if (person->owner == owner) {
                if (renderer && person->renderer != renderer) {
                    continue;
                }
                if (kind && person->kind != kind) {
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
    for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
        if (person->owner == owner) {
            if (renderer && person->renderer != renderer) {
                continue;
            }
            if (kind && person->kind != kind) {
                continue;
            }
            if (--count == 0) {
                return person - thing;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x000173c0
void start_person_going(Thing *person)
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
    if (person->room > room_info) {
        if (set_complex(person, 90, person->room->unknown6)) {
            go = 1;
        }
        flag = 1;
    }
    id = find_nearest_manned_reception(person, 0, 1);
    if (id) {
        other = &thing[id];
        arg = other->room_next;
        person->unknown89 = thing[other->unknown91].room - room_info;
        if (set_complex(person, 470, arg)) {
            go = 1;
        }
        done = 1;
    }
    if (done == 0) {
        room = &room_info[allhosps[person->owner].unknown1cf[old_newrand() & 3]];
        arg = room->unknown4;
        if (set_complex(person, 20, arg)) {
            go = 1;
        }
    }
    if (go) {
        process_person_complex(person);
    }
}

// FUNCTION: 0x0001752c
void person_nav_failure(Thing *person)
{
    RoomInfo *room;

    if (person->room == 0) {
        room = which_room(person->unknown19 + (person->unknown20 << 7));
        if (room != 0 && actual_game_turn - 500 < room->unknown22) {
            start_person_going(person);
            return;
        }
    }
    if (person->state_b == 0x14 || person->state_b == 0x18) {
        person->kind = person->state_b;
        return;
    }
    person->unknown105 = person->kind;
    person->unknown106 = person->unknown19;
    person->unknown107 = person->unknown20;
    person->kind = 0x3c;
    person->height = 0;
    if (person->owner == player) {
        announce_this(1, 1);
    }
}

// FUNCTION: 0x00017620
void person_fills(Thing *person)
{
    Thing *found;
    Thing *other;
    CompFrame *cur;

    other = &thing[person->unknown69];
    if (other > thing) {
        if (actual_game_turn % person->anim_speed == 0) {
            person->sprite = frame[person->sprite].next;
            other->unknown100 = frame[other->unknown100].next;
        }
        if (person->subtype == 14) {
            other->unknown103 += 2;
            cur = &frame[other->unknown100];
        } else {
            other->unknown103++;
            cur = &frame[person->sprite];
        }
        if (other->unknown103 > cur->length) {
            person->kind = 0x20;
            person->sprite = other->unknown100;
            person->palette = other->state_b;
            if (person->subtype == 14) {
                found = find_person_in_room(person->room, 4, 1);
                if (found != 0) {
                    process_things(found, found + 1);
                }
            }
        }
    }
}

// FUNCTION: 0x00017780
void person_energises(Thing *person)
{
    u16 next;
    Thing *other;

    other = &thing[person->unknown69];
    if (other > thing) {
        if (actual_game_turn % person->anim_speed == 0) {
            next = frame[person->sprite].next;
            if (!(frame[next].flags & 1)) {
                person->sprite = frame[person->sprite].next;
                other->unknown100 = frame[other->unknown100].next;
            }
        }
    }
}

// FUNCTION: 0x00017854
void person_in_bogs(Thing *person)
{
    if (person->unknown103 == 0) {
        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, 0x33, 1, 150, person->owner)) {
            person->state_b = 0x36;
            person->state_a = 0x38;
            person->unknown103 = 1;
            set_complex(person, 330, 0);
            process_person_complex(person);
        } else {
            person->kind = 0x38;
        }
    } else if (person->unknown103 == 1) {
        person->unknown124 = 0;
        if (person->style >= 0x10 && find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, 0x20, 1, 150, person->owner)) {
            person->state_b = 0x38;
            person->state_a = 0x38;
            set_complex(person, 350, 0);
            process_person_complex(person);
        } else {
            person->kind = 0x38;
        }
    } else {
        person->kind = 0x38;
    }
}

// FUNCTION: 0x000179a8
void leave_toilets(Thing *person)
{
    person->state_b = 0x26;
    person->state_a = 0x38;
    set_complex(person, 90, person->room->unknown6);
    process_person_complex(person);
}

// FUNCTION: 0x000179f8
void return_to_queue_area(Thing *person)
{
    u16 id;
    u16 arg;
    RoomInfo *room;
    Thing *other;

    arg = 0;
    if (person->unknown140) {
        room = &room_info[person->unknown140];
        person->state_a = 0x18;
        person->state_b = 0x18;
        if (room->type == 4) {
            id = find_object_anywhere(11, 0, 0, person->owner);
            if (id) {
                other = &thing[id];
                arg = other->room_next;
            }
        } else {
            arg = room->unknown4;
        }
        if (arg) {
            set_complex(person, 30, arg);
            process_person_complex(person);
            return;
        }
    }
    person->kind = 0x18;
}

// FUNCTION: 0x00017ae0
void play_person_showanim(Thing *person)
{
    u16 next;
    CompFrame *cur;

    if (actual_game_turn % person->anim_speed == 0) {
        next = frame[person->sprite].next;
        cur = &frame[next];
        if (cur->flags & 1) {
            person->kind = person->unknown103;
            person->sprite = person->unknown85;
            person->palette = person->unknown104;
            person->unknown85 = person->unknown104 = person->unknown103 = 0;
            if (person->flags & 0x4) {
                place_litter(person->x.tile + (person->y.tile << 7), (s8)person->owner, 5);
                person->flags &= ~0x4;
            } else if (person->flags & 0x8) {
                place_litter(person->x.tile + (person->y.tile << 7), (s8)person->owner, 6);
                person->flags &= ~0x8;
            }
        } else {
            person->sprite = next;
        }
    }
}

// FUNCTION: 0x00017c28
void page_staff_i_need(Thing *person, RoomInfo *room)
{
    s8 count;

    if (room->type == 10) {
        count = 2;
    } else {
        count = 1;
    }
    page_member_of_staff(expertise[person->unknown171].unknown0, room - room_info, 1, count, person->owner);
}

// FUNCTION: 0x00017cb0
void fn_00017cb0(Thing *person)
{
    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
    }
    if (person->height < 400) {
        person->height += person->unknown18;
    } else {
        remove_person(person);
    }
}

// FUNCTION: 0x00017d2c
void send_person_somewhere_else(Thing *person)
{
    person->state_b = 0x26;
    if (set_complex(person, 90, person->room->unknown6)) {
        process_person_complex(person);
    }
}

// FUNCTION: 0x00017d78
void refer_patient(Thing *person, u8 owner)
{
    u8 i;

    if (!(person->flags & 0x40)) {
        interrupt_person(person);
        leave_queue(person);
        person->owner = owner;
        person->unknown118 |= 1;
        if (person->flags & 0x20) {
            fn_000b05f0(person);
            person->unknown174 = 100;
            person->flags &= ~0x20;
            remove_person_from_vaccination_queue(person);
        }
        person->unknown169 = 1;
        person->unknown171 = 1;
        for (i = 0; i < 10; i++) {
            person->unknown148[i] = 0;
        }
        start_person_going(person);
    }
}

// FUNCTION: 0x00017e30
void person_waits_for_decision(Thing *person)
{
    u8 style;
    u8 category;
    u8 done;
    CompFrame *cur;

    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
        cur = &frame[person->sprite];
        if (cur->flags & 1) {
            person->height--;
            if (person->height <= 0) {
                if (person->flags & 0x400) {
                    done = find_possible_diagnosis(person);
                    if (done || (u16)(othervar.f82 - person->timer) >= 1500) {
                        person->flags &= ~0x10;
                        reset_icon_value(person, 0x1b);
                        if (!(person->flags & 0x80)) {
                            get_playercall(8, person->unknown91, person->owner);
                            person->unknown91 = 0;
                        }
                        if (done) {
                            person->kind = 0x26;
                        } else {
                            person_go_home(person);
                            allhosps[person->owner].rooms[person->style].unknown24++;
                            person->unknown118 |= 8;
                        }
                        person->flags &= 0xfb7f;
                        return;
                    }
                } else {
                    if (person->unknown172[0]) {
                        style = person->unknown172[0];
                    } else {
                        style = person->unknown171;
                    }
                    category = expertise[style].category;
                    done = allhosps[person->owner].rooms[style].unknown1C == 1 && listhead[person->owner].rooms[category]
                        ? (u8)1 : (u8)0;
                    if (done || (u16)(othervar.f82 - person->timer) >= 1500) {
                        person->flags &= ~0x10;
                        reset_icon_value(person, 0x1b);
                        if (person->flags & 0x80) {
                            person->flags &= ~0x80;
                            if (!done) {
                                person_go_home(person);
                                allhosps[person->owner].rooms[person->style].unknown24++;
                                person->unknown118 |= 0x10;
                            } else {
                                get_playercall(4, person->unknown91, person->owner);
                                person->kind = 0x26;
                                person->unknown91 = 0;
                            }
                        } else {
                            person_go_home(person);
                            person->unknown118 |= 0x10;
                        }
                        return;
                    }
                }
                if (blocks[map[((s16)person->y.tile << 7) + (s16)person->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(person)) {
                    return;
                }
                {
                    u16 anim;

                    anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->unknown17;
                    person->sprite = start[anim].frame;
                    person->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                }
                if (find_standing_place(person)) {
                    return;
                }
                person->height = person->unknown18 == 0 ? div_err() : 0x100 / person->unknown18;
            }
        }
    }
}

// FUNCTION: 0x0001824c
u16 make_a_playercall(const u16 type, const u16 subject, const s16 timeout, u8 owner)
{
    PlayerCall *call;

    if (gamevar.f174) {
        return 0;
    }
    for (call = playercall + 1; call < end_playercall; call++) {
        if (call->type == 0) {
            switch (type) {
            case 3:
                if (owner == player) {
                    play_ambient(saferand() % 3 + 0xaf);
                }
                break;
            case 5:
                switch (subject) {
                case 3:
                    if (owner == player) {
                        play_ambient(saferand() % 3 + 0xb2);
                    }
                    break;
                }
                break;
            }
            call->type = type;
            call->id = popup_id[0]++;
            call->active = 1;
            call->x = subject;
            call->timer = timeout;
            call->owner = owner;
            add_panel_popup(call);
            if (listhead[owner].first[type] == 0) {
                listhead[owner].first[type] = call - playercall;
            }
            return call->id;
        }
    }
    return 0;
}

// FUNCTION: 0x00018438
void get_new_popup_head(u16 type, u8 owner)
{
    int unused = 0;
    u16 min = 0xffff;
    PlayerCall *call;

    for (call = playercall + 1; call < end_playercall; call++) {
        if (call->type != type || call->owner != owner) {
            continue;
        }
        if (min > call->id) {
            listhead[owner].first[type] = call - playercall;
            min = call->id;
        }
    }
}

// FUNCTION: 0x000184f4
void save_popup(u16 type)
{
    u8 i;
    int n = 1;
    PlayerCall *call;

    if (statvar.unknownfd > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        call = playercall + listhead[i].first[type];
        if (call->type == 0) {
            return;
        }
        memmove(&D_0015f5a4[i][type], call, 12);
        delete_panel_popup(call);
        call->type = 0;
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
    PlayerCall *call;

    if (statvar.unknownfd > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        for (call = playercall + 1; call < end_playercall; call++) {
            if (call->type == 0) {
                memmove(call, &D_0015f5a4[i][type], 12);
                add_panel_popup(call);
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
    PlayerCall *call;

    for (call = playercall + 1; call < end_playercall; call++) {
        if ((int)call->type != (int)type || call->owner != owner) {
            continue;
        }
        if (id == 0 || id && id == call->id) {
            call->type = 0;
            result = call->x;
            delete_panel_popup(call);
            if (listhead[owner].first[type] == call - playercall) {
                listhead[owner].first[type] = 0;
                get_new_popup_head(type, owner);
            }
            break;
        }
    }
    return result;
}

// FUNCTION: 0x00018838
u16 check_playercall(u16 type, u16 subject, u8 owner)
{
    int count = 0;
    PlayerCall *call;

    for (call = playercall + 1; call < end_playercall; call++) {
        if (call->type != type || call->owner != owner) {
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
        } else if (call->x == subject) {
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
    PlayerCall *call;

    for (call = playercall + 1; call < end_playercall; call++) {
        if (call->timer) {
            call->timer--;
            if (call->timer > 0) {
                continue;
            }
            if (call->type == 6) {
                owner1 = call->owner;
                allhosps[owner1].f1f3 = actual_game_turn + 0x5dc;
                allhosps[owner1].f1fa = 2;
            } else if (call->type == 3) {
                choice_spare_data = 0x63;
                packet_choice_result(0xb);
            } else if (call->type == 5) {
                if (call->x == 1) {
                    allhosps[call->owner].f1e2 = 0;
                }
            }
            owner2 = call->owner;
            if (listhead[owner2].first[call->type] == call - playercall) {
                listhead[owner2].first[call->type] = 0;
                changed = 1;
            }
            delete_panel_popup(call);
            call->type = 0;
        }
    }
    if (changed) {
        for (call = playercall + 1; call < end_playercall; call++) {
            if (call->type && listhead[call->owner].first[call->type] == 0) {
                listhead[call->owner].first[call->type] = call - playercall;
            }
        }
    }
}

// FUNCTION: 0x00018b78
u16 call_no_treatment(Thing *person, char **strs)
{
    char buf3[0xc8];
    char buf2[0x7d0];
    char buf1[0xc8];
    int room;
    int unused30;
    int staff;
    int i;
    Expertise *info;
    u16 text_y;
    u16 unused1c;
    s16 pct;
    u16 v14;
    u16 text_x;

    room = 0;
    staff = 0;
    unused30 = 0;
    unused1c = 0;
    char new_line[] = "\n";
    if (person <= thing) {
        return 0;
    }
    v14 = lbFontPtr ? lbFontPtr[34].height : 0;
    info = &expertise[person->style];
    if (listhead[person->owner].rooms[info->category] == 0 && room_available((u16)info->category)) {
        room = info->category;
    }
    for (i = 0; i < 9; i++) {
        if ((info->unknown0 & (1 << i)) && listhead[person->owner].staff[i] == 0) {
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
    if (allhosps[person->owner].rooms[0x2c].unknown1C != 1) {
        available_buttons &= ~4;
    }
    text_x = 0;
    text_y = 0;
    if (person->flags32 & 0x1000) {
        pct = person->unknown146 * 100 / gbv.ffc;
        if (pct < 0) {
            pct = 0;
        } else if (pct > 99) {
            pct = 99;
        }
        sprintf(buf2, strs[6], pct, D_001602a4[person->style]);
    } else {
        sprintf(buf2, strs[4], D_001602a4[person->style]);
    }
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, buf1);
    sprintf(buf3, strs[5]);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, buf3);
    LbTextDraw(text_x, text_y, buf2);
    return 0;
}

// FUNCTION: 0x00018f20
void process_no_treatment_buttons(u8 button)
{
    switch (button) {
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
void person_arrived_from_call(Thing *person)
{
    Thing *staff;

    if (person->room != 0) {
        for (staff = thing + person->room->first; staff > thing; staff = thing + staff->room_next) {
            if (staff->renderer == 4 && (u16)(person - thing) == staff->unknown73) {
                return;
            }
        }
        set_complex(person, 0x5a, person->room->unknown6);
        person->state_b = 0x26;
        process_person_complex(person);
    }
}

// FUNCTION: 0x0001905c
u32 person_ok_to_call(Thing *person)
{
    RoomInfo *room = person->room;

    if (room != 0) {
        if (room->type == 0x1d) {
            return 0;
        }
    } else if (person->state_b == 0x36) {
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
    int unused18;
    int unused14;
    int staff;
    int room;
    u16 text_y;
    u16 text_x;

    room = 0;
    staff = 0;
    unused14 = 0;
    unused18 = 0;
    char new_line[] = "\n";
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
        sprintf(buf1, "");
    }
    buf2[0] = 0;
    text_x = 0;
    text_y = 0;
    sprintf(buf3, strs[4], D_001602a4[kind]);
    fn_000c93bf(buf2, buf3);
    fn_000c93bf(buf2, new_line);
    sprintf(buf3, strs[kind * 3 - 1]);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, buf3);
    sprintf(buf3, strs[kind * 3 - 1 + 1]);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, buf3);
    sprintf(buf3, strs[kind * 3 + 1]);
    fn_000c93bf(buf2, new_line);
    fn_000c93bf(buf2, buf3);
    LbTextDraw(text_x, text_y, buf2);
    return 0;
}

// FUNCTION: 0x00019464
void process_announce_new_illness_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000194a0
void force_peep_to_be_cured(Thing *person)
{
    person->state_b = 0;
    person->flags32 = 0;
    leave_queue(person);
    interrupt_person(person);
    best_guess_treatment(person);
    next_treatment_now_setup(person, 0);
}

// FUNCTION: 0x000194f0
void interrupt_person(Thing *person)
{
    u8 was44 = person->kind == 0x44 || person->state_b == 0x44;
    u8 state;
    Thing *staff;

    if (person->unknown140) {
        state = 8;
    } else if (person->unknown89) {
        state = 0x24;
    } else {
        leave_queue(person);
        state = 0x26;
    }
    interrupt_complex(person);
    if (person->room) {
        for (staff = thing + person->room->first; staff > thing; staff = thing + staff->room_next) {
            if (staff->renderer == 4 && (u16)(person - thing) == staff->unknown73) {
                staff->unknown73 = 0;
                interrupt_staff(staff);
                break;
            }
        }
        switch (person->subtype) {
        case 4:
        case 5:
            set_complex(person, 0xb4, 0);
            break;
        case 9:
        case 10:
            person->state_a = 0;
            set_complex(person, 0x276, 0);
            break;
        }
        set_complex(person, 0x5a, person->room->unknown6);
        process_person_complex(person);
        person->state_b = state;
    } else if (was44) {
        person->kind = 0x44;
    } else {
        person->kind = state;
    }
}
