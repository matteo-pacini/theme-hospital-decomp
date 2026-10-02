#include "hosp.h"

extern "C" {
void clear_peep_icon(Thing *peep);
void remove_person_from_room_list(Thing *person);
}

s16 place_staff_in_struct(u16 kind, u16 extra_skills);
s16 get_state(const u16 type, const Thing *staff);
void change_lockholder(Thing *staff, RoomInfo *room);
void calculate_speed(Thing *staff);
void inc_staff_stats(Thing *staff, u8 owner);
void set_expertise_staff(Thing *staff);
void insert_skill(u16 mask, u8 owner);
void reinit_nurse(Thing *nurse);
void pager_arrival(Thing *staff);
u16 staffrand(void);
u32 normal_staffrand(u32 low, u32 high);
void appraise_staff(Thing *employee);
void clear_my_mutex(Thing *staff);
void staff_exit_room_check(Thing *staff);
void remove_expertise_staff(Thing *staff);
void rest_staff(Thing *staff, s16 amount);
u8 send_doctor_to_research(Thing *doctor);
u8 send_doctor_to_training(Thing *doctor);
u8 nurse_check_for_work(Thing *nurse);
u16 check_pager(Thing *staff);
extern "C" u8 check_room_mutex(Thing *w);
extern "C" u8 surgeon_screen_free(Thing *o);
void private_interrupt_staff(Thing *staff);
u16 thingpage_remind_me(ThingPager *p, Thing *handyman);
void thingpage_cancel_my_response(ThingPager *p, Thing *handyman);
void staff_abandon_pager_response(Thing *staff);
void receptionist_reset(Thing *receptionist);
void remove_staff_from_map(Thing *staff);
extern "C" {
void remove_node(Thing *node);
void remove_person_from_room_list(Thing *person);
}
void change_surgeon_to_doc(Thing *surgeon);
extern s16 staff_states[][5];
extern s16 mcurx;
extern s16 mcury;
extern "C" {
extern s16 D_001567b0[8];
}

// At each month end (and level start): takes this month's staff_levels entry
// (applicant numbers per kind and the odds of each doctor skill) and creates
// applicants until each kind has its number.
// FUNCTION: 0x00029ad0
void create_job_market(void)
{
    u32 i;
    u32 shortfall;
    u32 j;

    for (i = 0; i < 30; i++) {
        if (staff_levels[i].month == othervar.game_month) {
            gamevar.staff_seed = staff_levels[i].fund;
            gamevar.applicants_wanted[0] = staff_levels[i].counts[0];
            gamevar.applicants_wanted[1] = staff_levels[i].counts[1];
            gamevar.applicants_wanted[2] = staff_levels[i].counts[2];
            gamevar.applicants_wanted[3] = staff_levels[i].counts[3];
            gamevar.shrink_rate = staff_levels[i].rates[0];
            gamevar.surgeon_rate = staff_levels[i].rates[1];
            gamevar.researcher_rate = staff_levels[i].rates[2];
            gamevar.consultant_rate = staff_levels[i].rates[3];
            gamevar.junior_rate = staff_levels[i].rates[4];
            break;
        }
        if (staff_levels[i].month > othervar.game_month) {
            break;
        }
    }
    for (i = 0; i <= 3; i++) {
        if (gamevar.applicants[i] < gamevar.applicants_wanted[i]) {
            shortfall = gamevar.applicants_wanted[i] - gamevar.applicants[i];
            for (j = 0; j < shortfall; j++) {
                place_staff_in_struct(i, 0);
            }
        }
    }
}

// Puts staff member id (an applicant or a picked-up one) on tile for player
// `owner`, if the tile is theirs or force is set, entering the state for
// the room there. A new hire is counted in the hospital's statistics and
// skills. Returns 1 if placed, 0 otherwise.
// FUNCTION: 0x00029c4c
u16 place_staff_on_map(u16 tile, u16 id, u8 force, u8 owner)
{
    Thing *new_staff;

    new_staff = &thing[id];
    if (new_staff > thing) {
        if (force || map[tile].owner_flags == (1 << owner)) {
            new_staff->owner = owner;
            if (new_staff->facing > 3) {
                new_staff->facing = 0;
            }
            {
                u16 idx;

                idx = check_frame(staff_anim_starts[new_staff->subtype].anims[staff[new_staff->subtype].anim],
                                  new_staff) +
                      new_staff->facing;
                new_staff->sprite = start[idx].frame;
                new_staff->flip = start[idx].flags > 0 ? (u8)1 : (u8)0;
            }
            new_staff->x.tile = tile % 128;
            new_staff->y.tile = tile / 128;
            new_staff->x.frac = 0x80;
            new_staff->y.frac = 0x80;
            new_staff->path[2] = 0xff;
            RoomInfo *room = which_room(tile);
            if (room > 0) {
                insert_person_in_room_list(room, new_staff);
                new_staff->state = get_state((u16)room->type, new_staff);
                if (new_staff->state != SS_EVACUATE) {
                    if (new_staff->subtype != STAFF_HANDYMAN &&
                        (room->type != ROOM_TRAINING || new_staff->subtype != STAFF_DOCTOR ||
                         (new_staff->skills & SKILL_CONSULTANT)) &&
                        room->type != ROOM_RESEARCH) {
                        change_lockholder(new_staff, room);
                    }
                } else if (new_staff->subtype != STAFF_HANDYMAN) {
                    if (new_staff->owner == player) {
                        advisor.new_staff_pending = 1;
                        advisor.new_staff_kind = new_staff->subtype;
                        advisor.new_staff_room = room->type;
                    }
                }
            } else {
                new_staff->state = get_state(0, new_staff);
            }
            new_staff->substate = 0;
            new_staff->complex_action = 0;
            calculate_speed(new_staff);
            insert_node(new_staff, new_staff->x, new_staff->y);
            if (allhosps[new_staff->owner].held_staff == 0) {
                new_staff->resign_count = 0;
                new_staff->resign_time = 0;
                new_staff->orders = 0;
                new_staff->discontent = 0;
                clear_peep_icon(new_staff);
                new_staff->hire_expiry = 0;
                new_staff->hire_time = actual_game_turn;
                inc_staff_stats(new_staff, new_staff->owner);
                set_expertise_staff(new_staff);
                insert_skill(new_staff->skills, new_staff->owner);
                gamevar.applicant_count--;
                gamevar.applicants[new_staff->subtype]--;
            }
            if (new_staff->subtype == STAFF_NURSE) {
                reinit_nurse(new_staff);
            } else if (new_staff->subtype == STAFF_HANDYMAN) {
                new_staff->work_target2 = 0;
                new_staff->work_target = new_staff->work_target2;
            }
            pager_arrival(new_staff);
            if (new_staff->room) {
                new_staff->room->staff_free = 1;
            }
            if (othervar.cur_player == player) {
                gamevar.held_staff = 0;
            }
            allhosps[new_staff->owner].held_staff = 0;
            allhosps[new_staff->owner].held_staff_tile = 0;
            return 1;
        }
    }
    return 0;
}

// Counts staff in player `owner`'s staff statistics (total, per kind and
// surgeons).
// FUNCTION: 0x0002a094
void inc_staff_stats(Thing *staff, u8 owner)
{
    allhosps[owner].staff_by_kind[staff->subtype == STAFF_SURGEON ? 1 : staff->subtype]++;
    if (staff->skills & SKILL_SURGEON) {
        allhosps[owner].surgeon_count++;
    }
    allhosps[owner].staff_count++;
}

// Deletes applicant staff (not hired), freeing the thing.
// FUNCTION: 0x0002a130
void remove_staff_in_struct(Thing *staff)
{
    gamevar.applicants[staff->subtype]--;
    gamevar.applicant_count--;
    memset(staff, 0, 0xaf);
}

// Creates an applicant of staff kind `kind` (STAFF_*) with skills
// extra_skills added: random looks, face, doctor skills by the job market's
// odds, ability, wage, name and references; expires after 2500-5000 turns.
// Returns the thing index, 0 if none was free.
// FUNCTION: 0x0002a178
s16 place_staff_in_struct(u16 kind, u16 extra_skills)
{
    Thing *new_staff;
    u32 mean;
    u32 spread;

    new_staff = allocate_thing(0x19);
    if (new_staff != 0) {
        new_staff->subtype = kind;
        new_staff->type = THING_STAFF;
        new_staff->facing = 0;
        new_staff->owner = 0x63;
        if (staff[kind].layer5_count) {
            new_staff->layers[5] = staffrand() % staff[kind].layer5_count + 1;
        } else {
            new_staff->layers[5] = 0;
        }
        new_staff->layers[6] = staffrand() & 3;
        new_staff->layers[7] = staffrand() & 3;
        new_staff->layers[8] = staffrand() & 3;
        new_staff->layers[9] = staffrand() & 3;
        if (kind == 0 || kind == 3) {
            new_staff->layers[5] = 1;
        }
        new_staff->hire_expiry = actual_game_turn + 2500 + staffrand() % 2500;
        new_staff->work_delay = normal_staffrand(1, 10);
        new_staff->happiness = 100;
        new_staff->warmth = 50;
        new_staff->state = SS_APPLICANT;
        gamevar.applicant_count++;
        gamevar.applicants[new_staff->subtype]++;
        switch (new_staff->subtype) {
        case STAFF_NURSE:
            mean = 30;
            spread = 1000;
            new_staff->skills = SKILL_NURSE;
            new_staff->female = 1;
            new_staff->face_head = 12;
            new_staff->face_body = staffrand() % 5 + 10;
            new_staff->face_legs = staffrand() % 5 + 10;
            break;
        case STAFF_DOCTOR:
            mean = gbv.doctor_threshold;
            spread = gbv.consultant_threshold;
            new_staff->skills |= SKILL_DOCTOR;
            if (gamevar.shrink_rate > 0 && staffrand() % gamevar.shrink_rate == 0) {
                new_staff->skills |= SKILL_PSYCHIATRIST;
            }
            if (gamevar.surgeon_rate > 0 && staffrand() % gamevar.surgeon_rate == 0) {
                new_staff->skills |= SKILL_SURGEON;
            }
            if (gamevar.researcher_rate > 0 && staffrand() % gamevar.researcher_rate == 0) {
                new_staff->skills |= SKILL_RESEARCHER;
            }
            if (gamevar.consultant_rate > 0 && staffrand() % gamevar.consultant_rate == 0) {
                new_staff->skills |= SKILL_CONSULTANT;
                mean = gbv.consultant_threshold;
                spread = 1000;
            } else if (gamevar.junior_rate > 0 && staffrand() % gamevar.junior_rate == 0) {
                new_staff->skills |= SKILL_JUNIOR;
                mean = 30;
                spread = gbv.doctor_threshold;
            }
            if (staffrand() % 100 < 53) {
                new_staff->female = 1;
            } else {
                new_staff->female = 0;
            }
            new_staff->face_head = staffrand() % 5;
            new_staff->face_body = staffrand() % 5;
            new_staff->face_legs = staffrand() % 5;
            switch (new_staff->layers[5]) {
            case 1:
                break;
            case 2:
                new_staff->face_legs += 5;
                new_staff->face_body += 5;
                new_staff->face_head += 5;
                break;
            }
            break;
        case STAFF_HANDYMAN:
            mean = 30;
            spread = 1000;
            new_staff->skills = SKILL_HANDYMAN;
            new_staff->female = 0;
            new_staff->prio_litter = 0x21;
            new_staff->prio_plants = 0x21;
            new_staff->prio_repair = 0x22;
            new_staff->face_head = staffrand() % 3 + 15;
            new_staff->face_body = staffrand() % 3 + 15;
            new_staff->face_legs = staffrand() % 3 + 15;
            break;
        case STAFF_RECEPTIONIST:
            mean = 30;
            spread = 1000;
            new_staff->skills = SKILL_RECEPTIONIST;
            new_staff->female = 1;
            new_staff->face_head = staffrand() % 4 + 10;
            new_staff->face_body = staffrand() % 5 + 10;
            new_staff->face_legs = staffrand() % 5 + 10;
            if (new_staff->face_head == 12) {
                new_staff->face_head = 14;
            }
            break;
        }
        new_staff->skills |= extra_skills;
        new_staff->ability = normal_staffrand(mean, spread);
        appraise_staff(new_staff);
        new_staff->salary = new_staff->wage_demand;
        new_staff->initial = staffrand() % 26 + 0x41;
        new_staff->surname_a = staffrand() % 33;
        new_staff->surname_b = staffrand() % 26;
        new_staff->ref_general = staffrand() % statvar.ref_general_count;
        new_staff->ref_good = staffrand() % statvar.ref_good_count;
        new_staff->ref_bad = staffrand() % statvar.ref_bad_count;
        return new_staff - thing;
    }
    return 0;
}

// Dismisses staff: mode 0 sacks them with severance pay, 1 lets them resign,
// both walking them off the map; mode 2 removes them at once. Takes them
// out of the hospital's statistics and skills (owner 0x63 after).
// FUNCTION: 0x0002a804
void sack_staff(Thing *staff, u16 mode)
{
    clear_my_mutex(staff);
    set_icon_value(staff, ICON_LEAVING);
    if (staff->owner == 0x63) {
        return;
    }
    interrupt_staff(staff);
    switch (mode) {
    case 0:
        change_balance(TX_SEVERANCE_PAY, -staff->salary, 0, staff->owner);
        break;
    case 1:
        staff->orders &= ~ORD_RESIGN;
        break;
    }
    if (mode != 2) {
        staff_exit_room_check(staff);
        if (staff->room) {
            set_complex(staff, 0x136, staff->room->door_inside_tile);
        }
        set_complex(staff, 0xbe, get_room_info(ROOM_ENTRANCE, 0x65, staff->owner)->door_tile);
        staff->next_state = SS_BACK_TO_APPLICANTS;
        staff->resume_state = 0;
        process_staff_complex(staff);
    }
    remove_expertise_staff(staff);
    staff->work_delay += staff->work_delay / 4;
    staff->happiness = 100;
    staff->resign_count = 0;
    staff->resign_time = 0;
    staff->orders = 0;
    staff->discontent = 0;
    clear_peep_icon(staff);
    allhosps[staff->owner].staff_count--;
    allhosps[staff->owner].staff_by_kind[staff->subtype == STAFF_SURGEON ? 1 : staff->subtype]--;
    if (staff->skills & SKILL_SURGEON) {
        allhosps[staff->owner].surgeon_count--;
    }
    staff->owner = 0x63;
    if (mode == 2) {
        if (staff->room) {
            remove_person_from_room_list(staff);
        }
        remove_thing(staff);
    }
}

// Staff waiting state: animates and turns randomly at each step. Returns 0.
// FUNCTION: 0x0002aa54
s16 staff_waits(Thing *worker)
{
    if (actual_game_turn % worker->anim_speed == 0) {
        worker->sprite = frame[worker->sprite].next;
    }
    if (--worker->countdown <= 0) {
        worker->countdown = worker->speed == 0 ? div_err() : 0x100 / worker->speed;
        worker->facing = old_newrand() & 3;
        {
            u16 idx;

            idx = check_frame(staff_anim_starts[worker->subtype].anims[staff[worker->subtype].anim], worker) +
                  worker->facing;
            worker->sprite = start[idx].frame;
            worker->flip = start[idx].flags > 0 ? (u8)1 : (u8)0;
        }
    }
    return 0;
}

// Sends staff wandering the corridor from where they stand (SS_WANDERS).
// FUNCTION: 0x0002abb0
void staff_in_corridor(Thing *staff)
{
    u16 anim;

    staff->dest_x = staff->x.tile;
    staff->dest_y = staff->y.tile;
    staff->state = SS_WANDERS;
    staff->substate = 6;
    staff->countdown = 1;
    anim = check_frame(staff_anim_starts[staff->subtype].staff_stand, staff) + (u16)staff->facing;
    staff->sprite = start[anim].frame;
    staff->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
}

// If a page for staff's skills is waiting in another room (check_pager),
// sends them there. Returns 1 if they went.
// FUNCTION: 0x0002ac70
u8 obey_pager(Thing *staff)
{
    RoomInfo *room;

    room = &room_info[check_pager(staff)];
    if (room <= room_info || room == staff->room) {
        return 0;
    }
    staff_exit_room_check(staff);
    staff->next_state = (u8)get_state((u16)room->type, staff);
    staff->substate = 0;
    if (staff->room) {
        set_complex(staff, 0x136, staff->room->door_inside_tile);
        process_staff_complex(staff);
        staff->resume_state = SS_EVACUATE;
        set_complex(staff, 0x14a, room->door_tile);
    } else {
        staff->resume_state = SS_EVACUATE;
        set_complex(staff, 0x14a, room->door_tile);
        process_staff_complex(staff);
    }
    staff->dest_room = room - room_info;
    return 1;
}

// Walks staff out of their room into the corridor (SS_IN_CORRIDOR).
// Returns 1.
// FUNCTION: 0x0002ad8c
u8 evacuate_staff(Thing *staff)
{
    RoomInfo *room;

    room = staff->room;
    if (room > room_info) {
        staff_exit_room_check(staff);
        set_complex(staff, 0x136, room->door_inside_tile);
        staff->next_state = SS_IN_CORRIDOR;
        process_staff_complex(staff);
    } else {
        staff->state = SS_IN_CORRIDOR;
    }
    return 1;
}

// Sends staff to the nearest staff room for a rest. Returns 0 if there is
// none.
// FUNCTION: 0x0002adfc
u8 take_a_break(Thing *staff)
{
    RoomInfo *room;

    room = get_nearest_room_info(staff, ROOM_STAFF_ROOM);
    if (room <= room_info) {
        return 0;
    }
    if (staff->subtype == STAFF_RECEPTIONIST) {
        interrupt_staff(staff);
    }
    staff_exit_room_check(staff);
    staff->next_state = SS_IN_STAFF_ROOM;
    if (staff->room > room_info) {
        set_complex(staff, 0x136, staff->room->door_inside_tile);
        process_staff_complex(staff);
        staff->resume_state = SS_EVACUATE;
        set_complex(staff, 0x96, room->door_tile);
    } else {
        set_complex(staff, 0x96, room->door_tile);
        staff->resume_state = SS_EVACUATE;
        process_staff_complex(staff);
    }
    staff->dest_room = room - room_info;
    return 1;
}

// Staff state SS_IN_STAFF_ROOM: rests them, plays pool, video games or sits
// on a sofa, and once fully rested cures a crazy doctor and sends everyone
// back to work (research, training, the pager or the corridor).
// FUNCTION: 0x0002af04
void staff_in_staff_room(Thing *staff)
{
    u16 roll;
    CompFrame *cur_frame;

    if (actual_game_turn % staff->anim_speed == 0) {
        staff->sprite = frame[staff->sprite].next;
        cur_frame = &frame[staff->sprite];
        if (cur_frame->flags & 1) {
            if (--staff->countdown <= 0) {
                if (staff->complex_action == 0x2da) {
                    rest_staff(staff, gbv.rest_sofa);
                } else {
                    rest_staff(staff, gbv.rest_standing);
                }
                staff->orders &= ~ORD_REST;
                if (staff->orders & ORD_LEAVE_ROOM) {
                    staff->orders &= 0xf0;
                    evacuate_staff(staff);
                    return;
                }
                if (staff->room->being_edited || staff->room->state == RS_EDITED) {
                    evacuate_staff(staff);
                    return;
                }
                roll = old_newrand() % 10;
                if ((find_object(staff->x.tile + (staff->y.tile << 7), OBJ_POOL_TABLE, 1, 0x96, staff->owner) &&
                     staff->complex_action != 0x2ee) &&
                    roll == 1) {
                    if (staff_abilities[staff->subtype][1]) {
                        staff->resume_state = SS_IN_STAFF_ROOM;
                        staff->next_state = SS_IN_STAFF_ROOM;
                        set_complex(staff, 0x2ee, 0);
                        process_staff_complex(staff);
                        return;
                    }
                }
                if ((find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), OBJ_VIDEO_GAME, 1, 0x96,
                                 staff->owner) &&
                     staff->complex_action != 0x2f8) &&
                    roll == 2) {
                    if (staff_abilities[staff->subtype][0]) {
                        staff->resume_state = SS_IN_STAFF_ROOM;
                        staff->next_state = SS_IN_STAFF_ROOM;
                        set_complex(staff, 0x2f8, 0);
                        process_staff_complex(staff);
                    }
                    return;
                }
                if (staff->complex_action != 0x2da) {
                    if (find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), OBJ_SOFA, 1, 0x96, staff->owner)) {
                        staff->resume_state = SS_IN_STAFF_ROOM;
                        staff->next_state = SS_IN_STAFF_ROOM;
                        set_complex(staff, 0x2da, 0);
                        process_staff_complex(staff);
                        return;
                    }
                }
                if (overlapping_staff_check(staff, 0x30)) {
                    return;
                }
                staff->speed = 0x20;
                if (staff->tiredness <= 0) {
                    staff->discontent = (s16)staff->discontent / 2;
                    if (staff->subtype == STAFF_DOCTOR && staff->crazy) {
                        if (staff->tiredness <= 0) {
                            if (staff->layers[5] == 3) {
                                staff->layers[5] = 1;
                            } else if (staff->layers[5] == 4) {
                                staff->layers[5] = 2;
                            }
                            staff->crazy = 0;
                        }
                    } else if (staff->subtype == STAFF_DOCTOR) {
                        if ((staff->orders & ORD_RETURN_TO_RESEARCH) && send_doctor_to_research(staff)) {
                            return;
                        }
                        if ((staff->orders & ORD_RETURN_TO_TRAINING) && send_doctor_to_training(staff)) {
                            return;
                        }
                        if (!obey_pager(staff)) {
                            set_complex(staff, 0x136, staff->room->door_inside_tile);
                            staff->next_state = SS_IN_CORRIDOR;
                            process_staff_complex(staff);
                        }
                    } else if (staff->subtype == STAFF_NURSE) {
                        set_complex(staff, 0x136, staff->room->door_inside_tile);
                        if (!nurse_check_for_work(staff)) {
                            staff->next_state = SS_IN_CORRIDOR;
                            process_staff_complex(staff);
                        }
                    } else if (staff->subtype == STAFF_HANDYMAN) {
                        staff->state = SS_EVACUATE;
                        return;
                    } else if (staff->subtype == STAFF_RECEPTIONIST) {
                        staff->state = SS_EVACUATE;
                        return;
                    }
                    calculate_speed(staff);
                }
                staff->countdown = staff->speed == 0 ? div_err() : 0x100 / staff->speed;
            }
        }
    }
}

// Claims staff's room for them (the lock: one holder, two in an operating
// theatre). Returns 1 if they hold it or are not in a room, 0 if it is
// taken.
// FUNCTION: 0x0002b400
u8 check_room_mutex(Thing *staff)
{
    RoomInfo *room;
    u16 *holders;

    room = staff->room;
    if (room <= room_info) {
        return 1;
    }
    holders = room->staff;
    if ((u16)(staff - thing) == holders[0] || (u16)(staff - thing) == holders[1]) {
        return 1;
    }
    if (staff->room->type == ROOM_THEATRE) {
        if (!holders[0]) {
            holders[0] = staff - thing;
            return 1;
        }
        if (!holders[1]) {
            holders[1] = staff - thing;
            return 1;
        }
        return 0;
    }
    if (holders[0]) {
        return 0;
    }
    holders[0] = staff - thing;
    return 1;
}

// Returns 1 if staff holds their room's first lock (in an operating
// theatre, only once both surgeons are there).
// FUNCTION: 0x0002b528
u8 am_i_lead_staff(Thing *staff)
{
    u16 *holders;

    if (staff->room) {
        holders = staff->room->staff;
        if (staff->room->type == ROOM_THEATRE) {
            if (holders[0] == 0 || holders[1] == 0) {
                return 0;
            }
        }
        return (u16)(staff - thing) == holders[0] ? (u8)1 : (u8)0;
    }
    return 0;
}

// A staff member whose path failed: leaves a recently built destination
// room, or gets stuck (SS_STUCK), announced for this player.
// FUNCTION: 0x0002b5cc
void staff_nav_failure(Thing *staff)
{
    RoomInfo *room;

    if (staff->room == 0) {
        room = which_room((u16)staff->dest_x + ((u16)staff->dest_y << 7));
        if (room && actual_game_turn - 500 < room->created_turn) {
            evacuate_staff(staff);
            return;
        }
    }
    staff->saved_state = staff->state;
    staff->saved_dest_x = staff->dest_x;
    staff->saved_dest_y = staff->dest_y;
    staff->state = SS_STUCK;
    staff->countdown = 0;
    if (staff->owner == player) {
        announce_this(ANN_STUCK, 1);
    }
}

// Draws the staff member being placed under the mouse, greyed where they
// cannot be dropped (gamevar.staff_place_ok 0).
// FUNCTION: 0x0002b690
void draw_staff_cursor(void)
{
    u8 bad;
    s16 state;
    RoomInfo *room;
    Thing *held;
    u8 *remap;

    if (othervar.mouse_cell != othervar.prev_mouse_cell || gamevar.held_staff_kind != gamevar.drawn_staff_kind) {
        room = which_room(othervar.mouse_cell);
        if (room > 0) {
            state = staff_states[rooms[room->type].staff_state_row][gamevar.held_staff_kind];
            bad = 0;
            if (check_points_connected(othervar.mouse_cell, room->door_inside_tile) == 0) {
                bad = 1;
            }
            if ((room->state != 0 && room->state != 1) && gamevar.held_staff_kind != 2) {
                bad = 1;
            }
        } else {
            state = staff_states[rooms[0].staff_state_row][gamevar.held_staff_kind];
            bad = !check_points_connected(othervar.mouse_cell, room_info[allhosps[player].entrances[0]].door_tile)
                      ? (u8)1
                      : (u8)0;
        }
        if (state == -1 || bad == 1) {
            gamevar.staff_place_ok = 0;
        } else {
            gamevar.staff_place_ok = 1;
        }
        gamevar.staff_cursor_anim =
            staff_anim_starts[gamevar.held_staff_kind].anims[staff[gamevar.held_staff_kind].anim] + 1;
        gamevar.drawn_staff_kind = gamevar.held_staff_kind;
        if (map[othervar.mouse_cell].owner_flags != 1 << player) {
            gamevar.staff_place_ok = 0;
        }
    }
    if (gamevar.staff_place_ok) {
        remap = 0;
    } else {
        remap = bw_remap;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    held = &thing[gamevar.held_staff];
    if (held > thing) {
        D_001567b0[0] = D_001567b0[1] = 999;
        D_001567b0[2] = D_001567b0[3] = 0;
        draw_from_special_id_frame(start[gamevar.staff_cursor_anim].frame, mcurx + resvars.tile_width / 2,
                                   mcury - resvars.tile_height / 2, remap, held->layers,
                                   start[gamevar.staff_cursor_anim].flags);
    }
}

// Pays staff (or, if staff is not a thing, every staff member of player
// `owner`) a bonus of percent of their wage or of amount, raising their
// happiness by 50 when morale is set (the money is only charged then).
// Returns the total. For one staff member the charge is amount, even with
// a percentage.
// FUNCTION: 0x0002b998
s32 pay_staff_bonus(Thing *staff, u16 amount, u16 percent, u16 morale, u8 owner)
{
    s32 total;
    Thing *member;

    total = 0;
    if (staff > thing) {
        if (percent) {
            total = staff->salary * percent / 100;
        } else {
            total = amount;
        }
        if (morale) {
            staff->happiness += 50;
            if ((s16)staff->happiness < 0) {
                staff->happiness = 0;
            } else if ((s16)staff->happiness > 100) {
                staff->happiness = 100;
            }
            change_balance(TX_PERSONAL_BONUS, -amount, staff - thing, owner);
        }
    } else {
        for (member = thing + 1; member < end_things; member++) {
            if (member->type == THING_STAFF && member->owner == owner) {
                if (percent) {
                    total += percent * member->salary / 100;
                } else {
                    total += amount;
                }
                if (morale) {
                    member->happiness += 50;
                    if ((s16)member->happiness < 0) {
                        member->happiness = 0;
                    } else if ((s16)member->happiness > 100) {
                        member->happiness = 100;
                    }
                }
            }
        }
        if (morale) {
            change_balance(TX_GENERAL_BONUS, -total, 0, owner);
        }
    }
    return total;
}

// Releases staff's lock on their room.
// FUNCTION: 0x0002bb58
void clear_my_mutex(Thing *staff)
{
    u16 *holders;

    if (staff->room) {
        holders = staff->room->staff;
        if (holders[0] == (u16)(staff - thing)) {
            holders[0] = 0;
        } else if (holders[1] == (u16)(staff - thing)) {
            holders[1] = 0;
        }
    }
}

// Tidies up as staff leave their room: a surgeon becomes a doctor again,
// the training room's consultant stops the projector, and a nurse frees
// her ward.
// FUNCTION: 0x0002bbdc
void staff_exit_room_check(Thing *staff)
{
    Thing *projector;

    if (staff->room) {
        if (staff->subtype == STAFF_SURGEON) {
            change_surgeon_to_doc(staff);
        }
        if (staff->room->type == ROOM_TRAINING && &thing[staff->room->staff[0]] == staff) {
            projector = &thing[find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), OBJ_PROJECTOR, 0, 0x97,
                                           staff->owner)];
            if (projector > thing && projector->lecture_started) {
                staff->complex_pc = 0;
                staff->complex_queued = 0;
                staff->complex_queued2 = 0;
                staff->complex_resume = 0;
                if (&thing[staff->object &&
                                   (thing[staff->object].type == THING_OBJECT ||
                                    thing[staff->object].type == THING_DOOR) &&
                                   (u16)(staff - thing) == thing[staff->object].user
                               ? staff->object
                               : 0] == projector) {
                    set_complex(staff, 0x3b6, 0);
                } else {
                    set_complex(staff, 0xa0, 0);
                }
                projector->lecture_started = 0;
            }
        }
        if (staff->subtype == STAFF_NURSE && staff->room->type == ROOM_WARD) {
            staff->room->state == RS_EDITED ? 0 : (staff->room->state = 0);
        }
    }
}

// Lowers staff's tiredness by amount (by less when very tired, but at least
// gbv.recovery_minimum) and raises their happiness by 1.
// FUNCTION: 0x0002be14
void rest_staff(Thing *staff, s16 amount)
{
    if (staff->tiredness > gbv.recovery_factor / 2) {
        if ((amount = amount * gbv.recovery_factor / staff->tiredness) < gbv.recovery_minimum) {
            amount = gbv.recovery_minimum;
        }
    }
    staff->tiredness -= amount;
    staff->happiness++;
    if ((s16)staff->happiness < 0) {
        staff->happiness = 0;
    } else if ((s16)staff->happiness > 100) {
        staff->happiness = 100;
    }
}

// Staff picked up by the player: takes them off the map and, on this
// player's machine, starts placing them (MM_PLACE_STAFF).
// FUNCTION: 0x0002becc
void staff_picked_up(Thing *staff)
{
    staff_exit_room_check(staff);
    abandon_complex(staff);
    allhosps[staff->owner].held_staff_tile = staff->x.tile + (staff->y.tile << 7);
    if (othervar.cur_player == player) {
        gamevar.held_staff_kind = staff->subtype;
        gamevar.held_staff = staff - thing;
        gamevar.drawn_staff_kind = 0xffff;
        set_mouse_mode(MM_PLACE_STAFF);
    }
    clear_my_mutex(staff);
    remove_staff_from_map(staff);
    staff->state = SS_HELD;
}

// Takes staff off the map and out of their room's list.
// FUNCTION: 0x0002bf94
void remove_staff_from_map(Thing *staff)
{
    if (staff->room) {
        remove_person_from_room_list(staff);
    }
    remove_node(staff);
}

// Handles a staff member's resignation: once given notice, leaves when it
// runs out; otherwise hands in notice (with a call to the player) after six
// to nine months if discontented, or when poached. Returns 1 if they
// resigned or handed in notice this turn.
// FUNCTION: 0x0002bfc8
u32 shall_i_resign(Thing *staff)
{
    u32 limit;

    if (staff->orders & ORD_RESIGN) {
        set_icon_value(staff, ICON_LEAVING);
        if (staff->owner == player) {
            advisor.event_flags |= 1;
        }
        if (actual_game_turn >= staff->resign_time) {
            get_playercall(1, staff->playercall, staff->owner);
            sack_staff(staff, 1);
            staff->playercall = 0;
            return 1;
        }
    } else {
        limit = ((old_newrand() & 3) + 6) * 1500;
        if (actual_game_turn - (u16)staff->hire_time > limit) {
            if ((s16)staff->discontent > gbv.resign_max) {
                staff->orders |= ORD_RESIGN;
                staff->resign_count++;
                staff->resign_time = actual_game_turn + 1500;
                staff->playercall = make_a_playercall(1, staff - thing, 0, staff->owner);
                return 1;
            }
        }
        if (staff->poach_offers) {
            if (staff->discontent_s > gbv.resign_max / 4) {
                staff->orders |= ORD_RESIGN;
                staff->resign_time = actual_game_turn + 1500;
                staff->playercall = make_a_playercall(1, staff - thing, 0, staff->owner);
                return 1;
            }
            staff->poach_offers = 0;
        }
    }
    return 0;
}

// Returns the number of player `owner`'s doctors who are not tired
// (tiredness below gbv.tired).
// FUNCTION: 0x0002c1cc
u16 count_emergency_staff(u8 owner)
{
    u32 count;
    Thing *doctor;

    count = 0;
    for (doctor = thing + 1; doctor < end_things; doctor++) {
        if ((doctor->type == THING_STAFF && doctor->subtype == STAFF_DOCTOR) && doctor->owner == owner) {
            if (doctor->tiredness < gbv.tired) {
                count++;
            }
        }
    }
    return count;
}

// Sets staff's walking speed and animation speed from their tiredness and
// ability (slow in the training room). Only when at a tile's centre.
// FUNCTION: 0x0002c268
void calculate_speed(Thing *staff)
{
    u32 speed;

    speed = 3;
    if (staff->x.frac == 0x80 && staff->y.frac == 0x80) {
        if (staff->tiredness > gbv.crack_up_tired) {
            speed = 1;
        } else if (staff->tiredness > gbv.very_tired) {
            speed = 2;
        }
        if (staff->ability >= gbv.consultant_threshold) {
            speed += 30;
        } else if (staff->ability > gbv.doctor_threshold) {
            speed += 20;
        } else {
            speed += 10;
        }
        if (staff->room) {
            if (staff->room->type == ROOM_TRAINING) {
                speed = 31;
            }
        }
        switch (speed) {
        case 11:
        case 12:
        case 13:
        case 21:
        case 22:
        case 31:
            staff->speed = 0x10;
            staff->anim_speed = 2;
            break;
        case 23:
        case 32:
            staff->speed = 0x20;
            staff->anim_speed = 1;
            break;
        case 33:
            staff->speed = 0x40;
            staff->anim_speed = 1;
            break;
        }
    }
}

// Interrupts staff (see private_interrupt_staff), and a surgeon's partner
// too.
// FUNCTION: 0x0002c3bc
void interrupt_staff(Thing *staff)
{
    Thing *partner;
    u16 *slots;

    private_interrupt_staff(staff);
    if (staff->subtype == STAFF_SURGEON) {
        if (staff->room) {
            slots = staff->room->staff;
            partner = &thing[slots[0]];
            if (partner <= thing || partner == staff) {
                partner = &thing[slots[1]];
                if (partner <= thing || partner == staff) {
                    return;
                }
            }
            if (partner->owner != 0x63) {
                private_interrupt_staff(partner);
            }
        }
    }
}

// Stops what staff is doing: drops their pages and repair job, frees the
// patient they were dealing with, and returns them to the state for their
// room.
// FUNCTION: 0x0002c484
void private_interrupt_staff(Thing *staff)
{
    Thing *patient;
    Thing *target_obj;
    RoomInfo *room;

    patient = &thing[staff->slave];
    if (staff->subtype == STAFF_HANDYMAN) {
        target_obj = &thing[thingpage_remind_me(&allhosps[staff->owner].repair_pager, staff)];
        if (target_obj > thing) {
            room = &room_info[target_obj->room_id];
            if (room > room_info) {
                if (room->state == RS_REPAIR_DUE || room->state == RS_CLOSED) {
                    room->state = 0;
                }
            }
        }
        room = staff->room;
        if ((room && (target_obj = &thing[staff->object]) > thing) && objects[target_obj->subtype].is_machine) {
            if (room->state == RS_REPAIR_DUE || room->state == RS_CLOSED) {
                room->state = 0;
            }
        }
        thingpage_cancel_my_response(&allhosps[staff->owner].repair_pager, staff);
        thingpage_cancel_my_response(&allhosps[staff->owner].plant_pager, staff);
    }
    interrupt_complex(staff);
    if ((staff->subtype == STAFF_NURSE && patient > thing) && staff->room == 0) {
        patient->flags &= ~PF_VACCINE_NURSE_SENT;
        staff->slave = 0;
    } else if ((patient > thing && patient->type == THING_PATIENT) &&
               (staff->room != 0 || staff->subtype == STAFF_RECEPTIONIST)) {
        staff->slave = 0;
        interrupt_person(patient);
    }
    if (staff->room) {
        staff->state = get_state((u16)staff->room->type, staff);
    } else {
        staff->state = get_state(0, staff);
    }
    staff->substate = 0;
    staff_abandon_pager_response(staff);
    if (staff->subtype == STAFF_RECEPTIONIST) {
        receptionist_reset(staff);
    }
}

// Network game: staff id leaves for player `owner`'s hospital at double
// wage, walking to its reception; they are sacked at once if it has none.
// FUNCTION: 0x0002c700
void staff_join_new_employer(u16 id, u16 owner)
{
    Thing *staff;

    staff = &thing[id];
    if (staff > thing) {
        interrupt_staff(staff);
        if (staff->owner == player) {
            announce_this(ANN_STAFF_POACHED, 1);
        }
        staff_exit_room_check(staff);
        if (staff->room) {
            set_complex(staff, 0x136, staff->room->door_inside_tile);
        }
        set_complex(staff, 0xbe, get_room_info(ROOM_RECEPTION, 0x65, owner)->door_tile);
        staff->next_state = SS_IN_CORRIDOR;
        process_staff_complex(staff);
        remove_expertise_staff(staff);
        staff->work_delay += staff->work_delay / 4;
        staff->happiness = 100;
        staff->resign_count = 0;
        staff->hire_time = actual_game_turn;
        staff->resign_time = 0;
        staff->orders = 0;
        staff->discontent = 0;
        clear_peep_icon(staff);
        allhosps[staff->owner].staff_count--;
        allhosps[owner].staff_count++;
        allhosps[staff->owner].staff_by_kind[staff->subtype == STAFF_SURGEON ? 1 : staff->subtype]--;
        allhosps[owner].staff_by_kind[staff->subtype == STAFF_SURGEON ? 1 : staff->subtype]++;
        if (staff->skills & SKILL_SURGEON) {
            allhosps[staff->owner].surgeon_count--;
            allhosps[owner].surgeon_count++;
        }
        staff->owner = owner;
        set_expertise_staff(staff);
        insert_skill(staff->skills, staff->owner);
        staff->discontent = 0;
        staff->salary *= 2;
        if (!listhead[owner].rooms[ROOM_RECEPTION]) {
            sack_staff(staff, 0);
        }
    }
}

// Makes staff room's lock holder in place of the first current holder,
// interrupting and evacuating that one (except in an operating theatre with
// a free slot). Does nothing if no one holds the room.
// FUNCTION: 0x0002c9cc
void change_lockholder(Thing *staff, RoomInfo *room)
{
    Thing *old_holder;
    u16 *slot;

    slot = 0;
    if (room->staff[0]) {
        slot = &room->staff[0];
    } else if (room->staff[1]) {
        slot = &room->staff[1];
    }
    if (slot) {
        if (room->type != ROOM_THEATRE || room->staff[0] && room->staff[1]) {
            old_holder = &thing[*slot];
            interrupt_staff(old_holder);
            evacuate_staff(old_holder);
        }
        *slot = staff - thing;
    }
}

// Returns the state for staff entering a room of type `type` (0 for the
// corridor), or SS_EVACUATE for a doctor lacking the room's skill.
// FUNCTION: 0x0002ca88
s16 get_state(const u16 type, const Thing *staff)
{
    s16 state;

    state = staff_states[rooms[type].staff_state_row][staff->subtype];
    if (staff->subtype == STAFF_DOCTOR && rooms[type].doctor_skills) {
        if ((staff->skills & rooms[type].doctor_skills) == 0) {
            state = 0x36;
        }
    }
    return state;
}

// Raises staff's wage (or, if staff is not a thing, every staff member's of
// player `owner`) by percent or amount, up to 90,000, clearing discontent
// and raising happiness by 50. In the all-staff case the cap is applied to
// staff, not to each member.
// FUNCTION: 0x0002cb2c
void increase_staff_salaries(Thing *staff, u16 amount, u16 percent, u8 owner)
{
    s32 total;
    Thing *member;

    total = 0;
    if (staff > thing) {
        if (percent) {
            staff->salary += percent * staff->salary / 100;
        } else {
            staff->salary += amount;
        }
        if (staff->salary > 90000) {
            staff->salary = 90000;
        }
        staff->discontent = 0;
        staff->happiness += 50;
        if ((s16)staff->happiness < 0) {
            staff->happiness = 0;
        } else if ((s16)staff->happiness > 100) {
            staff->happiness = 100;
        }
    } else {
        for (member = thing + 1; member < end_things; member++) {
            if (member->type == THING_STAFF && member->owner == owner) {
                if (percent) {
                    member->salary += percent * member->salary / 100;
                } else {
                    member->salary += amount;
                }
                if (staff->salary > 90000) {
                    staff->salary = 90000;
                }
                member->discontent = 0;
                member->happiness += 50;
                if ((s16)member->happiness < 0) {
                    member->happiness = 0;
                } else if ((s16)member->happiness > 100) {
                    member->happiness = 100;
                }
            }
        }
    }
}

// Sets up training level `level` from rat_fest_level: places a consultant
// psychiatrist, a surgeon and a researcher on the first chosen tile of the
// player's land.
// FUNCTION: 0x0002cccc
void training_builder(u8 level)
{
    u32 count;
    RatFestLevel *lvl;
    Map *cell;
    s16 id;

    count = 0;
    lvl = &rat_fest_level[level];
    seed = safeseed = lvl->seed;
    for (cell = map; cell < end_map; cell++) {
        if (cell->owner_flags == 1 << player) {
            if (lvl->staff_max > count && saferand() % lvl->staff_chance == 0) {
                gamevar.consultant_rate = 1;
                gamevar.shrink_rate = 1;
                gamevar.surgeon_rate = 0;
                gamevar.researcher_rate = 0;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    place_staff_on_map(cell - map, id, 0, player);
                }
                gamevar.shrink_rate = 0;
                gamevar.surgeon_rate = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    place_staff_on_map(cell - map, id, 0, player);
                }
                gamevar.surgeon_rate = 0;
                gamevar.researcher_rate = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    place_staff_on_map(cell - map, id, 0, player);
                }
                break;
            }
        }
    }
}

// Returns 1 if another staff member shares staff's tile (later in the
// tile's list), else 0.
// FUNCTION: 0x0002ce9c
u8 is_staff_stepping_on_toes(Thing *staff)
{
    Thing *other;

    for (other = &thing[staff->cell_next]; other > thing; other = &thing[other->cell_next]) {
        if (other->type == THING_STAFF) {
            return 1;
        }
    }
    return 0;
}

// Moves staff to a random spot in their room if they block the door, share
// a tile, stand on an object they are not using, or now and then; resumes
// state afterwards. Returns 1 if they moved.
// FUNCTION: 0x0002cf14
u8 overlapping_staff_check(Thing *staff, s32 state)
{
    if (actual_game_turn % 47 == 0 || ((s16)staff->y.tile << 7) + (s16)staff->x.tile == staff->room->door_inside_tile ||
        is_staff_stepping_on_toes(staff) ||
        (staff->object && (thing[staff->object].type == THING_OBJECT || thing[staff->object].type == THING_DOOR) &&
                 (u16)(staff - thing) == thing[staff->object].user
             ? staff->object
             : 0) == 0 &&
            (map[(s16)staff->x.tile + ((s16)staff->y.tile << 7)].flags & 7)) {
        RoomInfo *room;
        u16 tile;
        u16 dest;

        room = staff->room;
        tile = staff->x.tile + (staff->y.tile << 7);
        dest = room->pos + (((s16)(old_newrand() % room->height) << 7) + (s16)(old_newrand() % room->width));
        if (tile != dest && can_i_get_to(staff, dest)) {
            staff->resume_state = state;
            staff->next_state = staff->resume_state;
            set_complex(staff, 0xbe, dest);
            process_staff_complex(staff);
            return 1;
        }
    }
    return 0;
}

// Returns 0 if staff is a surgeon whose room's surgeon screen is in use,
// else 1.
// FUNCTION: 0x0002d11c
u8 surgeon_screen_free(Thing *staff)
{
    Thing *obj;

    if (staff->subtype == STAFF_SURGEON) {
        obj = &thing[find_object(staff->x.tile + (staff->y.tile << 7), OBJ_SURGEON_SCREEN, 0, 0x97, staff->owner)];
        if (obj > thing) {
            if (obj->in_use != 0) {
                return 0;
            }
        }
    }
    return 1;
}
