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
    RoomInfo *room;

    tile = person->x.tile + (person->y.tile << 7);
    room = is_this_a_door(tile);
    if (room != 0 && rooms[room->type].unknown36 != 0) {
        set_complex(person, 0x32, 0);
        process_person_complex(person);
    } else {
        person->state = PS_IN_QUEUE;
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
            person->countdown--;
            if (person->countdown <= 0) {
                if (blocks[map[((s16)person->y.tile << 7) + (s16)person->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(person)) {
                    return;
                }
                modify_person(person);
                if (person->health > 0) {
                    room = &room_info[which_queue(person)];
                    if (room > room_info) {
                        page_staff_i_need(person, room);
                    }
                    if (abilities[person->subtype][0] && person->refreshment < 30 && person->complex_action != 60) {
                        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, OBJ_DRINKS_MACHINE, 1, 150, person->owner)) {
                            person->resume_state = PS_ENTERTAINS_THEMSELF;
                            person->next_state = PS_ENTERTAINS_THEMSELF;
                            set_complex(person, 60, 0);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (person->complex_action != 70) {
                        if (abilities[person->subtype][1]) {
                            if (find_nearest_free_object(person - thing, OBJ_BENCH)) {
                                if (person->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                person->resume_state = PS_ENTERTAINS_THEMSELF;
                                person->next_state = PS_ENTERTAINS_THEMSELF;
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

                            anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->facing;
                            person->sprite = start[anim].frame;
                            person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        }
                        if (find_standing_place(person)) {
                            return;
                        }
                    }
                    person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
                    person->state = PS_ENTERTAINS_THEMSELF;
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
            person->countdown--;
            if (person->countdown <= 0) {
                if (blocks[map[((s16)person->y.tile << 7) + (s16)person->x.tile].overlay].width == 0x20
                    && move_peep_from_blueprint(person)) {
                    return;
                }
                modify_person(person);
                if (person->health > 0) {
                    room = &room_info[which_queue(person)];
                    if (room > room_info) {
                        page_staff_i_need(person, room);
                    }
                    if (person->impatience > gbv.f108 || (person->flags & PF_SENT_HOME_BY_INSPECTOR)) {
                        if (person->owner == player) {
                            othervar.f89++;
                        }
                        set_icon_value(person, ICON_FED_UP);
                        person_go_home(person);
                        person->flags |= PF_FED_UP;
                        return;
                    }
                    if (abilities[person->subtype][0] && person->refreshment < 30 && person->complex_action != 60) {
                        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, OBJ_DRINKS_MACHINE, 1, 150, person->owner)) {
                            person->resume_state = PS_ENTERTAINS_THEMSELF;
                            person->next_state = PS_ENTERTAINS_THEMSELF;
                            set_complex(person, 60, 0);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (abilities[person->subtype][2] && person->bladder > gbv.f116 && person->complex_action != 90) {
                        room = get_nearest_room_info(person, ROOM_TOILETS);
                        if (room != 0) {
                            leave_queue(person);
                            person->resume_state = PS_ENTERTAINS_THEMSELF;
                            person->next_state = PS_IN_BOGS;
                            person->substate = 0;
                            set_complex(person, 360, room->unknown4);
                            process_person_complex(person);
                            return;
                        }
                    }
                    if (person->complex_action != 70) {
                        if (abilities[person->subtype][1]) {
                            if (find_nearest_free_object(person - thing, OBJ_BENCH)) {
                                if (person->owner == player) {
                                    NRAdcounts.unknown6 -= 5;
                                }
                                person->resume_state = PS_ENTERTAINS_THEMSELF;
                                person->next_state = PS_ENTERTAINS_THEMSELF;
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

                            anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->facing;
                            person->sprite = start[anim].frame;
                            person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                        }
                        if (find_standing_place(person)) {
                            return;
                        }
                    }
                    person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
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
    if (person->cell_prev != 0 || (cell->flags & 7)) {
        base = person->x.tile + (person->y.tile << 7);
        for (i = old_newrand() % 12 + 1; i <= standing_place[0]; i++) {
            tile = base + standing_place[i];
            target = &map[tile];
            if (target->first == 0 && !(target->flags & 7) && can_i_get_to(person, tile) > 0 && !(target->unknown7 & 0x10)) {
                break;
            }
        }
        if (standing_place[i]) {
            person->resume_state = person->state;
            person->next_state = person->state;
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
    if (person->playercall) {
        get_playercall(8, person->playercall, person->owner);
        get_playercall(4, person->playercall, person->owner);
        person->flags &= ~PF_AWAITING_DECISION;
        person->playercall = 0;
    }
    person->next_state = 0;
    record_person_as_patient(person);
    interrupt_person(person);
    leave_queue(person);
    if (set_complex(person, 20, get_room_info(1, 0x65, person->owner)->unknown4)) {
        process_person_complex(person);
    }
    set_icon_value(person, ICON_FED_UP);
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
    if (person->type == THING_PATIENT) {
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
void nav_debug_state(Thing *person)
{
    if (person->type == THING_PATIENT) {
        if (actual_game_turn % 6 == 0) {
            u16 anim;

            person->facing = *(u8 *)&next_angle[person->facing];
            anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->facing;
            person->sprite = start[anim].frame;
            person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(actual_game_turn & 0xf)) {
            person->state = person->resume_state;
            fn_000444e0(person);
        }
    } else {
        if (actual_game_turn % 6 == 0) {
            u16 anim;

            person->facing = *(u8 *)&next_angle[person->facing];
            anim = check_frame(staff_anim_starts[person->subtype].unknownA, person) + (u16)person->facing;
            person->sprite = start[anim].frame;
            person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
        }
        if (!(actual_game_turn & 0xf)) {
            person->state = person->resume_state;
            fn_000444e0(person);
        }
    }
}

// FUNCTION: 0x00016b30
u8 transform_peep_who(Thing *person, u8 level)
{
    s8 subtype;

    subtype = change_person_who[level][person->subtype];
    if (person->type == THING_PATIENT) {
        person->orig_subtype = person->subtype;
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
        if (callee->health <= 0) {
            id = callee->queue_next;
            if (id == 0) {
                break;
            }
            leave_queue(callee);
            callee = &thing[id];
        } else {
            if (person_ok_to_call(callee)) {
                caller->slave = id;
                caller->timer = 200;
                caller->call_retries = (old_newrand() & 3) + 3;
                leave_queue(callee);
                if (set_complex(callee, action, arg)) {
                    process_person_complex(callee);
                }
                callee->next_state = PS_ARRIVED_FROM_CALL;
                return id;
            }
            if (follow == 0) {
                break;
            }
            id = callee->queue_next;
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

    callee = &thing[caller->slave];
    if (callee > thing) {
        if (callee->health <= 0) {
            return 0;
        }
        if (callee->type != THING_PATIENT) {
            return 0;
        }
        if (callee->queue_room) {
            return 0;
        }
        if (!person_ok_to_call(callee)) {
            caller->timer = 200;
            return 2;
        }
        if (callee->complex_pc / 10 * 10 != action) {
            if (callee->complex_pc == 0) {
                if (set_complex(callee, action, arg)) {
                    process_person_complex(callee);
                }
                callee->next_state = PS_ARRIVED_FROM_CALL;
            } else if (callee->complex_queued != action) {
                if (callee->complex_queued == 0) {
                    if (set_complex(callee, action, arg)) {
                        process_person_complex(callee);
                    }
                    callee->next_state = PS_ARRIVED_FROM_CALL;
                } else {
                    return 0;
                }
            }
        } else {
            tile = (u16)callee->dest_x + ((u16)callee->dest_y << 7);
            if (tile != arg) {
                return 0;
            }
        }
        if (callee->next_state != PS_ARRIVED_FROM_CALL) {
            callee->next_state = PS_ARRIVED_FROM_CALL;
        }
        caller->timer = 200;
        caller->call_retries--;
        if (caller->call_retries <= 0) {
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
    Thing *bed;

    person->timer = old_newrand() % 300 + 300;
    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->owner != person->owner || room->type != ROOM_WARD) {
            continue;
        }
        for (bed = &thing[room->first_object]; bed > thing; bed = &thing[bed->room_obj_next]) {
            if (bed->subtype != OBJ_BED || bed->user != 0) {
                continue;
            }
            page_member_of_staff(4, room - room_info, 1, 1, person->owner);
            person->dest_room = room - room_info;
            fn_0004195c(person, room->unknown4);
            {
                u16 anim;

                anim = check_frame(anim_starts[person->subtype].idle, person) + (u16)person->facing;
                person->sprite = start[anim].frame;
                person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
            }
            person->state = PS_GOTO_QUEUE;
            room->unknown32++;
            return;
        }
    }
    room = get_expertise_dest(0x2a, person);
    room->unknown32++;
    page_member_of_staff(4, room - room_info, 1, 1, person->owner);
    person->dest_room = room - room_info;
    {
        u16 anim;

        anim = check_frame(anim_starts[person->subtype].idle, person) + (u16)person->facing;
        person->sprite = start[anim].frame;
        person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
    }
    person->state = PS_GOTO_QUEUE;
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
u16 find_peep(u16 room_id, u8 random, u8 type, u8 state, u8 owner)
{
    u16 count;
    RoomInfo *room;
    Thing *person;

    count = 0;
    room = &room_info[room_id];
    if (random) {
        for (person = &thing[room->first]; person > thing; person = &thing[person->room_next]) {
            if (person->owner == owner) {
                if (type && person->type != type) {
                    continue;
                }
                if (state && person->state != state) {
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
            if (type && person->type != type) {
                continue;
            }
            if (state && person->state != state) {
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
    u16 found_desk;
    u16 in_room;
    u16 id;
    u16 arg;
    Thing *desk;
    RoomInfo *room;

    found_desk = 0;
    in_room = 0;
    go = 0;
    if (person->room > room_info) {
        if (set_complex(person, 90, person->room->unknown6)) {
            go = 1;
        }
        in_room = 1;
    }
    id = find_nearest_manned_reception(person, 0, 1);
    if (id) {
        desk = &thing[id];
        arg = desk->use_pos[1];
        person->dest_room = thing[desk->user].room - room_info;
        if (set_complex(person, 470, arg)) {
            go = 1;
        }
        found_desk = 1;
    }
    if (found_desk == 0) {
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
        room = which_room(person->dest_x + (person->dest_y << 7));
        if (room != 0 && actual_game_turn - 500 < room->unknown22) {
            start_person_going(person);
            return;
        }
    }
    if (person->next_state == PS_IN_QUEUE || person->next_state == PS_ENTERTAINS_THEMSELF) {
        person->state = person->next_state;
        return;
    }
    person->saved_state = person->state;
    person->saved_dest_x = person->dest_x;
    person->saved_dest_y = person->dest_y;
    person->state = PS_STUCK;
    person->countdown = 0;
    if (person->owner == player) {
        announce_this(1, 1);
    }
}

// FUNCTION: 0x00017620
void person_fills(Thing *person)
{
    Thing *found;
    Thing *machine;
    CompFrame *cur;

    machine = &thing[person->object];
    if (machine > thing) {
        if (actual_game_turn % person->anim_speed == 0) {
            person->sprite = frame[person->sprite].next;
            machine->aux_sprite = frame[machine->aux_sprite].next;
        }
        if (person->subtype == 14) {
            machine->split_height += 2;
            cur = &frame[machine->aux_sprite];
        } else {
            machine->split_height++;
            cur = &frame[person->sprite];
        }
        if (machine->split_height > cur->length) {
            person->state = 0x20;
            person->sprite = machine->aux_sprite;
            person->flip = machine->aux_flip;
            if (person->subtype == 14) {
                found = find_person_in_room(person->room, THING_STAFF, STAFF_DOCTOR);
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
    Thing *machine;

    machine = &thing[person->object];
    if (machine > thing) {
        if (actual_game_turn % person->anim_speed == 0) {
            next = frame[person->sprite].next;
            if (!(frame[next].flags & 1)) {
                person->sprite = frame[person->sprite].next;
                machine->aux_sprite = frame[machine->aux_sprite].next;
            }
        }
    }
}

// FUNCTION: 0x00017854
void person_in_bogs(Thing *person)
{
    if (person->substate == 0) {
        if (find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, OBJ_TOILET, 1, 150, person->owner)) {
            person->next_state = PS_IN_BOGS;
            person->resume_state = PS_LEAVE_TOILETS;
            person->substate = 1;
            set_complex(person, 330, 0);
            process_person_complex(person);
        } else {
            person->state = PS_LEAVE_TOILETS;
        }
    } else if (person->substate == 1) {
        person->impatience = 0;
        if (person->illness >= 0x10 && find_object(((u16)person->y.tile << 7) + (u16)person->x.tile, OBJ_SINK, 1, 150, person->owner)) {
            person->next_state = PS_LEAVE_TOILETS;
            person->resume_state = PS_LEAVE_TOILETS;
            set_complex(person, 350, 0);
            process_person_complex(person);
        } else {
            person->state = PS_LEAVE_TOILETS;
        }
    } else {
        person->state = PS_LEAVE_TOILETS;
    }
}

// FUNCTION: 0x000179a8
void leave_toilets(Thing *person)
{
    person->next_state = PS_GO_FOR_TREATMENT;
    person->resume_state = PS_LEAVE_TOILETS;
    set_complex(person, 90, person->room->unknown6);
    process_person_complex(person);
}

// FUNCTION: 0x000179f8
void return_to_queue_area(Thing *person)
{
    u16 id;
    u16 arg;
    RoomInfo *room;
    Thing *desk;

    arg = 0;
    if (person->queue_room) {
        room = &room_info[person->queue_room];
        person->resume_state = PS_ENTERTAINS_THEMSELF;
        person->next_state = PS_ENTERTAINS_THEMSELF;
        if (room->type == ROOM_RECEPTION) {
            id = find_object_anywhere(OBJ_RECEPTION_DESK, 0, 0, person->owner);
            if (id) {
                desk = &thing[id];
                arg = desk->use_pos[1];
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
    person->state = PS_ENTERTAINS_THEMSELF;
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
            person->state = person->substate;
            person->sprite = person->saved_sprite;
            person->flip = person->saved_flip;
            person->saved_sprite = person->saved_flip = person->substate = 0;
            if (person->flags & PF_VOMITS) {
                place_litter(person->x.tile + (person->y.tile << 7), (s8)person->owner, 5);
                person->flags &= ~PF_VOMITS;
            } else if (person->flags & PF_WETS) {
                place_litter(person->x.tile + (person->y.tile << 7), (s8)person->owner, 6);
                person->flags &= ~PF_WETS;
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

    if (room->type == ROOM_THEATRE) {
        count = 2;
    } else {
        count = 1;
    }
    page_member_of_staff(expertise[person->treatment].unknown0, room - room_info, 1, count, person->owner);
}

// FUNCTION: 0x00017cb0
void fn_00017cb0(Thing *person)
{
    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
    }
    if (person->fly_height < 400) {
        person->fly_height += person->speed;
    } else {
        remove_person(person);
    }
}

// FUNCTION: 0x00017d2c
void send_person_somewhere_else(Thing *person)
{
    person->next_state = PS_GO_FOR_TREATMENT;
    if (set_complex(person, 90, person->room->unknown6)) {
        process_person_complex(person);
    }
}

// FUNCTION: 0x00017d78
void refer_patient(Thing *person, u8 owner)
{
    u8 i;

    if (!(person->flags & PF_EMERGENCY)) {
        interrupt_person(person);
        leave_queue(person);
        person->owner = owner;
        person->flags2 |= (PF_REFERRED >> 16);
        if (person->flags & PF_CONTAGIOUS) {
            fn_000b05f0(person);
            person->contagion = 100;
            person->flags &= ~PF_CONTAGIOUS;
            remove_person_from_vaccination_queue(person);
        }
        person->known_illness = 1;
        person->treatment = ILL_GENERAL_PRACTICE;
        for (i = 0; i < 10; i++) {
            person->treatments[i] = 0;
        }
        start_person_going(person);
    }
}

// FUNCTION: 0x00017e30
void person_waits_for_decision(Thing *person)
{
    u8 treatment;
    u8 category;
    u8 done;
    CompFrame *cur;

    if (actual_game_turn % person->anim_speed == 0) {
        person->sprite = frame[person->sprite].next;
        cur = &frame[person->sprite];
        if (cur->flags & 1) {
            person->countdown--;
            if (person->countdown <= 0) {
                if (person->flags & 0x400) {
                    done = find_possible_diagnosis(person);
                    if (done || (u16)(othervar.f82 - person->timer) >= 1500) {
                        person->flags &= ~PF_AWAITING_DECISION;
                        reset_icon_value(person, ICON_AWAITING_DECISION);
                        if (!(person->flags & PF_WAITING_FOR_ROOM)) {
                            get_playercall(8, person->playercall, person->owner);
                            person->playercall = 0;
                        }
                        if (done) {
                            person->state = PS_GO_FOR_TREATMENT;
                        } else {
                            person_go_home(person);
                            allhosps[person->owner].rooms[person->illness].unknown24++;
                            person->flags2 |= (PF_NO_DIAGNOSIS >> 16);
                        }
                        person->flags &= 0xfb7f;
                        return;
                    }
                } else {
                    if (person->next_treatment) {
                        treatment = person->next_treatment;
                    } else {
                        treatment = person->treatment;
                    }
                    category = expertise[treatment].category;
                    done = allhosps[person->owner].rooms[treatment].unknown1C == 1 && listhead[person->owner].rooms[category]
                        ? (u8)1 : (u8)0;
                    if (done || (u16)(othervar.f82 - person->timer) >= 1500) {
                        person->flags &= ~PF_AWAITING_DECISION;
                        reset_icon_value(person, ICON_AWAITING_DECISION);
                        if (person->flags & PF_WAITING_FOR_ROOM) {
                            person->flags &= ~PF_WAITING_FOR_ROOM;
                            if (!done) {
                                person_go_home(person);
                                allhosps[person->owner].rooms[person->illness].unknown24++;
                                person->flags2 |= (PF_NO_TREATMENT >> 16);
                            } else {
                                get_playercall(4, person->playercall, person->owner);
                                person->state = PS_GO_FOR_TREATMENT;
                                person->playercall = 0;
                            }
                        } else {
                            person_go_home(person);
                            person->flags2 |= (PF_NO_TREATMENT >> 16);
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

                    anim = check_frame(anim_starts[person->subtype].unknown12, person) + (u16)person->facing;
                    person->sprite = start[anim].frame;
                    person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                }
                if (find_standing_place(person)) {
                    return;
                }
                person->countdown = person->speed == 0 ? div_err() : 0x100 / person->speed;
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
    Expertise *exp;
    u16 text_y;
    u16 unused1c;
    s16 pct;
    u16 line_height;
    u16 text_x;

    room = 0;
    staff = 0;
    unused30 = 0;
    unused1c = 0;
    char new_line[] = "\n";
    if (person <= thing) {
        return 0;
    }
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    exp = &expertise[person->illness];
    if (listhead[person->owner].rooms[exp->category] == 0 && room_available((u16)exp->category)) {
        room = exp->category;
    }
    for (i = 0; i < 9; i++) {
        if ((exp->unknown0 & (1 << i)) && listhead[person->owner].staff[i] == 0) {
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
    if (allhosps[person->owner].rooms[ILL_AUTOPSY].unknown1C != 1) {
        available_buttons &= ~4;
    }
    text_x = 0;
    text_y = 0;
    if (person->flags32 & PF_GUESSED) {
        pct = person->diagnosis * 100 / gbv.ffc;
        if (pct < 0) {
            pct = 0;
        } else if (pct > 99) {
            pct = 99;
        }
        sprintf(buf2, strs[6], pct, D_001602a4[person->illness]);
    } else {
        sprintf(buf2, strs[4], D_001602a4[person->illness]);
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
            if (staff->type == THING_STAFF && (u16)(person - thing) == staff->slave) {
                return;
            }
        }
        set_complex(person, 0x5a, person->room->unknown6);
        person->next_state = PS_GO_FOR_TREATMENT;
        process_person_complex(person);
    }
}

// FUNCTION: 0x0001905c
u32 person_ok_to_call(Thing *person)
{
    RoomInfo *room = person->room;

    if (room != 0) {
        if (room->type == ROOM_TOILETS) {
            return 0;
        }
    } else if (person->next_state == PS_IN_BOGS) {
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
    Expertise *exp;
    u16 line_height;
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
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    exp = &expertise[kind];
    if (listhead[player].rooms[exp->category] == 0 && room_available((u16)exp->category)) {
        room = exp->category;
    }
    for (i = 0; i < 9; i++) {
        if ((exp->unknown0 & (1 << i)) && listhead[player].staff[i] == 0) {
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
    person->next_state = 0;
    person->flags32 = 0;
    leave_queue(person);
    interrupt_person(person);
    best_guess_treatment(person);
    next_treatment_now_setup(person, 0);
}

// FUNCTION: 0x000194f0
void interrupt_person(Thing *person)
{
    u8 was_deciding = person->state == PS_WAITS_FOR_DECISION || person->next_state == PS_WAITS_FOR_DECISION;
    u8 state;
    Thing *staff;

    if (person->queue_room) {
        state = 8;
    } else if (person->dest_room) {
        state = 0x24;
    } else {
        leave_queue(person);
        state = 0x26;
    }
    interrupt_complex(person);
    if (person->room) {
        for (staff = thing + person->room->first; staff > thing; staff = thing + staff->room_next) {
            if (staff->type == THING_STAFF && (u16)(person - thing) == staff->slave) {
                staff->slave = 0;
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
            person->resume_state = 0;
            set_complex(person, 0x276, 0);
            break;
        }
        set_complex(person, 0x5a, person->room->unknown6);
        process_person_complex(person);
        person->next_state = state;
    } else if (was_deciding) {
        person->state = PS_WAITS_FOR_DECISION;
    } else {
        person->state = state;
    }
}
