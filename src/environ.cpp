#include "hosp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern FeelGoodies feely[];
extern ShowAnim show_anims[];
extern WinLose lose_criteria[];

extern "C" {

u8 shall_i_be_abducted(Thing *w);
}

u16 check_warmth(Thing *person);
s16 check_feelgood(Thing *person);
void check_around(Thing *person);
void setup_showanim(Thing *peep, u16 anim);
s16 staff_check_around(Thing *employee);
void spread_epidemic(Thing *carrier, u16 warmth);
void calculate_speed(Thing *staff);
void doctor_cracks_up(Thing *doctor);
s32 evaluate_criteria(const u8 owner);
u32 check_criteria_values(const u32 mode, const u8 owner);
s32 losing(void);

// Periodic update of patient person: an emergency patient dies when the
// emergency ends and otherwise only shows the emergency icon; a contagious one
// shows contagious or vaccinated. When due (Thing.modify_time) and inside the
// building (MAP_OUTDOOR clear in Map.owner_flags), applies warmth, plants,
// mess and contagion, makes them more impatient, thirsty and in need of the
// toilet, adjusts happiness and status icons, and costs 1 health.
// FUNCTION: 0x00070f80
void modify_person(Thing *person)
{
    if (person->flags & PF_EMERGENCY) {
        if (allhosps[person->owner].emergency_state_s == 2) {
            change_health(person, -1000);
            person->flags &= ~PF_EMERGENCY;
            reset_icon_value(person, ICON_EMERGENCY);
            return;
        }
        set_icon_value(person, ICON_EMERGENCY);
        if (person->health_delta)
            change_health(person, 0);
        return;
    }
    if (person->flags & PF_CONTAGIOUS) {
        if ((s8)person->contagion > 0)
            set_icon_value(person, ICON_CONTAGIOUS);
        else if ((s8)person->contagion < 0)
            set_icon_value(person, ICON_VACCINATED);
    }
    if (!(othervar.cur_turn > person->modify_time || person->modify_time - othervar.cur_turn > 0x7fff))
        return;
    person->modify_time = othervar.next_modify_turn;
    if (map[((s16)person->y.tile << 7) + (s16)person->x.tile].owner_flags & MAP_OUTDOOR)
        return;
    check_warmth(person);
    check_feelgood(person);
    check_around(person);
    if ((s8)person->contagion > 1)
        spread_epidemic(person, person->warmth);
    person->impatience++;
    person->waiting_time++;
    person->refreshment--;
    if (old_newrand() % 3 == 0)
        person->bladder++;
    if ((s16)person->warmth > 80) {
        person->nausea++;
        if ((s16)person->nausea < 0)
            person->nausea = 0;
        set_icon_value(person, ICON_TOO_HOT);
        person->happiness--;
    } else if ((s16)person->warmth < 20) {
        set_icon_value(person, ICON_TOO_COLD);
        person->happiness--;
    } else {
        reset_icon_value(person, ICON_TOO_HOT);
        reset_icon_value(person, ICON_TOO_COLD);
        person->happiness++;
    }
    if (person->happiness_s < gbv.unhappy) {
        set_icon_value(person, ICON_UNHAPPY);
        person->impatience++;
    } else {
        reset_icon_value(person, ICON_UNHAPPY);
    }
    if (person->bladder > gbv.bowel_full) {
        set_icon_value(person, ICON_TOILET);
        person->impatience++;
    }
    if (person->refreshment < 30) {
        set_icon_value(person, ICON_THIRSTY);
        person->impatience++;
    }
    if (person->happiness_s < 0)
        person->happiness = 0;
    else if (person->happiness_s > 100)
        person->happiness = 100;
    if (person->refreshment < 0)
        person->refreshment = 0;
    change_health(person, -1);
}

// Moves person's warmth one step towards the warmth where they stand: a room's
// radiators * 1250 spread over its area, or in the corridor the nearby corridor
// radiators by distance and the heating setting (allhosps.heating_level); 0 with the
// heating off. Hot patients get thirsty faster. Returns the warmth at that
// spot.
// FUNCTION: 0x000712a8
u16 check_warmth(Thing *person)
{
    u16 heat;
    u16 dy;
    Thing *radiator;
    u16 dx;
    u16 dist;
    RoomInfo *room;
    u16 warmth;

    warmth = 0;
    heat = allhosps[person->owner].heating_level * 10;
    if (heat) {
        room = which_room((u16)person->x.tile + ((u16)person->y.tile << 7));
        if (room) {
            warmth = room->radiators * 1250 / (room->width * room->height);
        } else {
            radiator = &thing[listhead[person->owner].objects[OBJ_RADIATOR]];
            if (radiator > thing) {
                do {
                    if (!radiator->room_id) {
                        dx = ABS(person->x.tile - radiator->x.tile);
                        if (dx <= 8) {
                            dy = ABS(person->y.tile - radiator->y.tile);
                            if (dy <= 8) {
                                dist = dx + dy;
                                if (dist)
                                    warmth += heat / dist;
                            }
                        }
                    }
                    radiator = &thing[radiator->type_obj_next];
                } while (radiator > thing);
            }
        }
    }
    if (warmth > (s16)person->warmth)
        person->warmth++;
    else if (warmth < (s16)person->warmth)
        person->warmth--;
    if (person->type == THING_PATIENT && (s16)person->warmth > 60)
        person->refreshment -= 2;
    if ((s16)person->warmth < 0)
        person->warmth = 0;
    else if ((s16)person->warmth > 100)
        person->warmth = 100;
    return warmth;
}

// Changes person's nausea and happiness by the healthy and dead plants near
// them (feely), and makes a patient in PS_ENTERTAINS_THEMSELF whose kind has
// abilities[][1] (unless in complex 0x46) a little unhappier. Always returns 0.
// FUNCTION: 0x00071558
s16 check_feelgood(Thing *person)
{
    s8 mood;
    u16 i;
    Thing *obj;

    mood = 0;
    for (i = 0; feely[i].valid != 0; i++) {
        obj = &thing[listhead[person->owner].objects[feely[i].kind]];
        if (obj > thing) {
            do {
                if (!obj->room_id) {
                    if (ABS(person->x.tile - obj->x.tile) <= feely[i].range) {
                        if (ABS(person->y.tile - obj->y.tile) <= feely[i].range) {
                            if (obj->subtype == OBJ_PLANT) {
                                if ((s16)obj->plant_health < 0)
                                    mood--;
                                else
                                    mood++;
                            }
                        }
                    }
                }
                obj = &thing[obj->type_obj_next];
            } while (obj > thing);
        }
    }
    if (mood < 0) {
        person->nausea++;
        if ((s16)person->nausea < 0)
            person->nausea = 0;
        person->happiness--;
    } else if (mood > 0) {
        person->nausea--;
        if ((s16)person->nausea < 0)
            person->nausea = 0;
        person->happiness++;
    }
    if ((person->state == PS_ENTERTAINS_THEMSELF && abilities[person->subtype][1]) && person->complex_action != 0x46)
        person->happiness--;
    return 0;
}

// In the corridor, makes person sicker, unhappier and more likely to drop
// litter for each tile of mess (floor kinds other than 7 and 8) on and around
// their tile.
// FUNCTION: 0x000717f0
void check_around(Thing *person)
{
    u16 floor;
    u16 i;
    Map *m;
    Map *base;

    if (!which_room((u16)person->x.tile + ((u16)person->y.tile << 7))) {
        base = &map[((s16)person->y.tile << 7) + (s16)person->x.tile];
        for (i = 0; i < 5; i++) {
            m = &base[around[i]];
            floor = m->litter_shadow & 0xf;
            if (floor) {
                if (floor == 5 || floor == 6) {
                    person->nausea++;
                    if ((s16)person->nausea < 0)
                        person->nausea = 0;
                    person->happiness -= 2;
                    person->litter_urge += 2;
                } else if (!(floor == 0 || floor == 7 || floor == 8)) {
                    person->nausea += 2;
                    if ((s16)person->nausea < 0)
                        person->nausea = 0;
                    person->happiness--;
                    person->litter_urge++;
                }
            }
        }
    }
}

// Gives a patient in the corridor (inside the building) a chance to react:
// plays the earthquake reaction (PF_QUAKE_SHAKEN), vomits when nauseous or wets
// when desperate (each costing health), may be abducted
// (shall_i_be_abducted), or rarely plays a fitting idle
// animation from show_anims; may also drop litter. Returns 1 if an animation
// started, else 0.
// FUNCTION: 0x00071960
u8 do_reaction(Thing *peep)
{
    u16 pos;
    u16 illness;
    u16 ok;
    u16 count;
    u16 pick;
    u16 i;
    u16 chance;
    u16 list[5];

    count = 0;
    if (map[((s16)peep->y.tile << 7) + (s16)peep->x.tile].owner_flags & MAP_OUTDOOR)
        return 0;
    pos = (u16)peep->x.tile + ((u16)peep->y.tile << 7);
    if (which_room(pos) == 0) {
        if (peep->flags & PF_QUAKE_SHAKEN) {
            peep->flags &= ~PF_QUAKE_SHAKEN;
            setup_showanim(peep, 0x2d);
            return 1;
        }
        if ((s16)peep->nausea >= gbv.vomit_limit && old_newrand() % 5 == 0) {
            setup_showanim(peep, 0x4b);
            peep->nausea = 5;
            peep->flags |= PF_VOMITS;
            change_health(peep, -10);
            return 1;
        }
        if (peep->bladder >= gbv.bowel_overflows && old_newrand() % 5 == 0) {
            reset_icon_value(peep, ICON_TOILET);
            setup_showanim(peep, 0x66);
            if (peep->illness == ILL_THE_SQUITS)
                peep->bladder = gbv.bowel_full / 2;
            else
                peep->bladder = 0;
            peep->flags |= PF_WETS;
            change_health(peep, -10);
            return 1;
        }
        if (shall_i_be_abducted(peep))
            return 1;
        if (old_newrand() % 200 == 0) {
            for (i = 0; i < 5; i++) {
                illness = show_anims[i].illness;
                if (illness == peep->illness || illness == 0) {
                    if (show_anims[i].ability == 0 || abilities[peep->subtype][show_anims[i].ability]) {
                        if (show_anims[i].any_facing || show_anims[i].facings[peep->facing]) {
                            ok = 0;
                            switch (show_anims[i].condition) {
                            case 0:
                                ok = 1;
                                break;
                            case 1:
                                if (peep->impatience > gbv.leave_max / 2)
                                    ok = 1;
                                break;
                            case 2:
                                if (peep->impatience > gbv.leave_max * 3 / 4)
                                    ok = 1;
                                break;
                            case 3:
                                if (peep->impatience > gbv.leave_max * 3 / 4)
                                    ok = 1;
                                break;
                            }
                            if (ok)
                                list[count++] = show_anims[i].anim;
                        }
                    }
                }
            }
            if (count > 0) {
                pick = old_newrand() % count;
                setup_showanim(peep, list[pick]);
                return 1;
            }
        }
        if (gbv.litter_random > 0)
            chance = old_newrand() % gbv.litter_random;
        else
            chance = 1;
        if (peep->litter_urge > gbv.litter_drop || chance == 0) {
            if (old_newrand() % 10 == 0) {
                place_litter(pos, (s8)peep->owner, old_newrand() % 3 + 1);
                peep->litter_urge = 0;
            }
        }
    }
    return 0;
}

// Plays reaction animation anim (anim_starts index) on peep in PS_SHOWANIM,
// saving its state, sprite and flip to restore afterwards.
// FUNCTION: 0x00071dfc
void setup_showanim(Thing *peep, u16 anim)
{
    peep->substate = peep->state;
    peep->saved_sprite = peep->sprite;
    peep->saved_flip = peep->flip;
    peep->state = PS_SHOWANIM;
    peep->sprite = start[anim_starts[peep->subtype].anims[anim]].frame;
    peep->flip = start[anim_starts[peep->subtype].anims[anim]].flags;
}

// Periodic update of a staff member (not receptionists): when due, applies
// speed, warmth and surroundings, adds tiredness (except while wandering or in
// the staff room), may make an exhausted doctor crack up, adjusts happiness,
// status icons and discontent.
// FUNCTION: 0x00071eb8
void modify_staff(Thing *employee)
{
    if (!(othervar.cur_turn > employee->modify_time || employee->modify_time - othervar.cur_turn > 0x7fff))
        return;
    employee->modify_time = othervar.next_modify_turn;
    if (employee->owner >= 4 || gamevar.rat_fest_level)
        return;
    if (employee->subtype == STAFF_RECEPTIONIST)
        return;
    calculate_speed(employee);
    check_warmth(employee);
    staff_check_around(employee);
    if (employee->state != SS_WANDERS) {
        if (employee->room == 0 || employee->room->type != ROOM_STAFF_ROOM)
            employee->tiredness += gbv.work_light;
    }
    if (employee->tiredness > gbv.crack_up_tired) {
        if (old_newrand() % 1000 == 0)
            doctor_cracks_up(employee);
    }
    if (employee->tiredness > gbv.crack_up_tired) {
        set_icon_value(employee, ICON_TIRED);
        employee->happiness--;
    } else if (employee->tiredness < gbv.not_tired) {
        reset_icon_value(employee, ICON_TIRED);
        employee->happiness++;
    }
    if ((s16)employee->warmth > 80) {
        employee->happiness--;
        set_icon_value(employee, ICON_TOO_HOT);
    } else if ((s16)employee->warmth < 20) {
        employee->happiness--;
        set_icon_value(employee, ICON_TOO_COLD);
    }
    if (employee->happiness_s < gbv.unhappy)
        employee->discontent++;
    else if (employee->happiness_s > gbv.happy)
        employee->discontent--;
    if (employee->happiness_s < 0)
        employee->happiness = 0;
    else if (employee->happiness_s > 100)
        employee->happiness = 100;
    if (employee->tiredness < 0)
        employee->tiredness = 0;
    else if (employee->tiredness > 1000)
        employee->tiredness = 1000;
    if ((s16)employee->discontent < 0)
        employee->discontent = 0;
}

// Changes employee's happiness by the healthy and dead plants near them and,
// in the corridor, the mess around them. Always returns 0.
// FUNCTION: 0x00072110
s16 staff_check_around(Thing *employee)
{
    u16 floor;
    u16 i;
    Thing *plant;
    Map *base;
    Map *m;

    plant = &thing[listhead[employee->owner].objects[feely[0].kind]];
    if (plant > thing) {
        do {
            if (!plant->room_id) {
                if (ABS(employee->x.tile - plant->x.tile) <= feely[0].range) {
                    if (ABS(employee->y.tile - plant->y.tile) <= feely[0].range) {
                        if ((s16)plant->plant_health < 0)
                            employee->happiness--;
                        else
                            employee->happiness++;
                    }
                }
            }
            plant = &thing[plant->type_obj_next];
        } while (plant > thing);
    }
    if (!which_room((u16)employee->x.tile + ((u16)employee->y.tile << 7))) {
        base = &map[((s16)employee->y.tile << 7) + (s16)employee->x.tile];
        for (i = 0; i < 5; i++) {
            m = &base[around[i]];
            floor = m->litter_shadow & 0xf;
            if (floor) {
                if (floor == 5)
                    employee->happiness--;
                else if (!(floor == 0 || floor == 7 || floor == 8))
                    employee->happiness -= 2;
            }
        }
    }
    return 0;
}

// Sets employee's wage demand from their ability and skills, promotes a
// junior or doctor (to doctor or consultant) whose ability and time served
// allow it, and makes them happier or less happy by whether their salary
// meets the demand.
// FUNCTION: 0x00072374
void appraise_staff(Thing *employee)
{
    s32 value;

    value = employee->ability / gbv.salary_ability_divisor + staff[employee->subtype].min_salary;
    if (employee->skills & SKILL_JUNIOR)
        value += gbv.salary_add[0];
    if (employee->skills & SKILL_DOCTOR)
        value += gbv.salary_add[1];
    if (employee->skills & SKILL_SURGEON)
        value += gbv.salary_add[2];
    if (employee->skills & SKILL_PSYCHIATRIST)
        value += gbv.salary_add[3];
    if (employee->skills & SKILL_CONSULTANT)
        value += gbv.salary_add[4];
    if (employee->skills & SKILL_RESEARCHER)
        value += gbv.salary_add[5];
    employee->wage_demand = value;
    if (employee->skills & SKILL_DOCTOR) {
        if ((employee->skills & SKILL_JUNIOR) && employee->ability > gbv.doctor_threshold) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.promo_doc * 1500) {
                employee->skills &= ~8;
                if (employee->owner == player)
                    announce_this(ANN_JUNIOR_TO_DOCTOR, 1);
            }
        } else if (!(employee->skills & SKILL_CONSULTANT) && employee->ability > gbv.consultant_threshold) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.promo_con * 1500) {
                employee->skills |= SKILL_CONSULTANT;
                if (employee->owner == player)
                    announce_this(ANN_DOCTOR_TO_CONSULTANT, 1);
                if (employee->room != 0 && employee->room->type == ROOM_TRAINING) {
                    employee->next_state = SS_IN_CORRIDOR;
                    set_complex(employee, 0x136, employee->room->door_inside_tile);
                    process_staff_complex(employee);
                }
            }
        }
    }
    if (employee->salary < value)
        employee->happiness += gbv.salary_too_low;
    else
        employee->happiness += gbv.salary_too_high;
    if (employee->happiness_s < 0)
        employee->happiness = 0;
    else if (employee->happiness_s > 100)
        employee->happiness = 100;
}

// Starts a statistics pass (when othervar.every_32_turns is set): clears the advisor
// counts and each hospital's emergency and contagion counts.
// FUNCTION: 0x000725dc
void reset_all_count(void)
{
    u8 i;

    if (othervar.every_32_turns) {
        memset(&adcounts, 0, 0xa6);
        adcounts.patients = 1;
        adcounts.min_happiness = 999;
        adcounts.min_warmth = 999;
        adcounts.min_refreshment = 999;
        for (i = 0; i < 4; i++) {
            allhosps[i].emergency_present = 0;
            allhosps[i].contagious_count = 0;
        }
    }
}

// Adds employee (the local player's, not applicants) to the statistics pass:
// staff counts, tiredness, happiness and warmth totals, and idle staff.
// An idle surgeon (subtype 4) is counted in idle_staff[4], past the array.
// FUNCTION: 0x00072684
void staff_count(Thing *employee)
{
    if (othervar.every_32_turns) {
        if (employee->state != SS_APPLICANT && employee->owner == player) {
            switch (employee->subtype) {
            case STAFF_SURGEON:
                adcounts.surgeons++;
            case STAFF_DOCTOR:
                adcounts.doctors_and_nurses++;
                adcounts.staff_tiredness_by_kind[STAFF_DOCTOR] += employee->tiredness;
                adcounts.staff_happiness_by_kind[STAFF_DOCTOR] += employee->happiness;
                if (employee->skills & SKILL_CONSULTANT)
                    adcounts.consultants++;
                if (employee->skills & SKILL_RESEARCHER)
                    adcounts.researchers++;
                if (employee->skills & SKILL_PSYCHIATRIST)
                    adcounts.psychiatrists++;
                break;
            case STAFF_NURSE:
                adcounts.doctors_and_nurses++;
                adcounts.staff_tiredness_by_kind[STAFF_NURSE] += employee->tiredness;
                adcounts.staff_happiness_by_kind[STAFF_NURSE] += employee->happiness;
                break;
            case STAFF_HANDYMAN:
                adcounts.staff_tiredness_by_kind[STAFF_HANDYMAN] += employee->tiredness;
                adcounts.staff_happiness_by_kind[STAFF_HANDYMAN] += employee->happiness;
                break;
            case STAFF_RECEPTIONIST:
                adcounts.staff_tiredness_by_kind[STAFF_RECEPTIONIST] += employee->tiredness;
                adcounts.staff_happiness_by_kind[STAFF_RECEPTIONIST] += employee->happiness;
                break;
            }
            adcounts.staff_happiness_sum += employee->happiness;
            adcounts.staff_tiredness_sum += employee->tiredness;
            if (employee->tiredness > adcounts.max_staff_tiredness)
                adcounts.max_staff_tiredness = employee->tiredness;
            if (employee->state == SS_WANDERS || (((employee->room != 0 && employee->room->type != ROOM_TRAINING) &&
                                                   employee->room->queue_length == 0) &&
                                                  employee->room->queue_incoming == 0))
                adcounts.idle_staff[employee->subtype]++;
            adcounts.staff_warmth_sum += employee->warmth;
        }
    }
}

// Adds patient person to the statistics pass: the local player's waiting,
// happiness, thirst, warmth and toilet figures and illness counts, each
// hospital's emergency and contagious patients, and counts them once as a
// visitor this month. Patients outside the building are not counted.
// FUNCTION: 0x00072880
void person_count(Thing *person)
{
    s8 k = 5;

    if (othervar.every_32_turns) {
        advisor.has_patients = 1;
        if (!(map[((s16)person->y.tile << 7) + (s16)person->x.tile].owner_flags & MAP_OUTDOOR)) {
            if (person->owner == player) {
                adcounts.patients++;
                adcounts.impatience_sum += person->impatience;
                adcounts.waiting_sum += person->waiting_time;
                while (--k >= 0) {
                    adcounts.wait_sums[k] += person->waiting_time;
                    adcounts.wait_counts[k]++;
                }
                adcounts.happiness_sum += person->happiness;
                if (person->happiness_s > adcounts.max_happiness)
                    adcounts.max_happiness = person->happiness;
                if (person->happiness_s < adcounts.min_happiness)
                    adcounts.min_happiness = person->happiness;
                adcounts.refreshment_sum += person->refreshment;
                if (person->refreshment < adcounts.min_refreshment)
                    adcounts.min_refreshment = person->refreshment;
                if ((s16)person->warmth > adcounts.max_warmth)
                    adcounts.max_warmth = person->warmth;
                if ((s16)person->warmth < adcounts.min_warmth)
                    adcounts.min_warmth = person->warmth;
                adcounts.warmth_sum += person->warmth;
                adcounts.bladder_sum += person->bladder;
                if (person->bladder > adcounts.max_bladder)
                    adcounts.max_bladder = person->bladder;
                adcounts.illness_counts[person->illness]++;
            }
            if (person->flags & PF_EMERGENCY)
                allhosps[person->owner].emergency_present++;
            if ((s8)person->contagion != 0)
                allhosps[person->owner].contagious_count++;
            if (~person->flags32 & PF_COUNTED_VISITOR) {
                for (k = 3; --k >= 0;) {
                    mycontrol->history[4][k][11]++;
                    person->flags32 |= PF_COUNTED_VISITOR;
                }
            }
        } else if (person->flags & PF_CONTAGIOUS) {
            allhosps[person->owner].epidemic_timer = 0;
        }
    }
}

// Ends a statistics pass: stores the local player's average patient and staff
// happiness.
// FUNCTION: 0x00072b20
void final_count(void)
{
    if (othervar.every_32_turns) {
        myhosp->avg_patient_happiness = adcounts.happiness_sum / adcounts.patients;
        if (adcounts.doctors_and_nurses)
            myhosp->avg_staff_happiness = (s16)adcounts.staff_happiness_sum / (s16)adcounts.doctors_and_nurses;
        else
            myhosp->avg_staff_happiness = 0;
    }
}

// Adds machine obj to the statistics pass: the local player's machine count
// and wear, and the most worn machine past its strength.
// FUNCTION: 0x00072ba4
void object_count(const Thing *obj)
{
    if (othervar.every_32_turns) {
        if (objects[obj->subtype].is_machine) {
            if (obj->owner == player) {
                adcounts.machines++;
                adcounts.machine_wear_sum += (obj->times_used << 9) / (obj->strength * 3);
            }
            if (obj->times_used > obj->strength) {
                if ((obj->times_used << 9) / (obj->strength * 3) > adcounts.max_machine_wear) {
                    if (obj->owner == player) {
                        adcounts.max_machine_wear = (obj->times_used << 9) / (obj->strength * 3);
                        adcounts.worst_machine = obj->subtype;
                    }
                }
            }
        }
    }
}

// During a statistics pass, re-evaluates the local player's win criteria and
// how close they are to losing.
// FUNCTION: 0x00072c8c
void check_win_status(void)
{
    if (othervar.every_32_turns) {
        evaluate_criteria(player);
        adcounts.lose_progress = losing();
    }
}

// Returns how close the local player is to losing: the average progress
// (lose_criteria[].progress) of the lose criteria in use, times 256.
// FUNCTION: 0x00072cc0
s32 losing(void)
{
    s8 i = 10;
    s32 total = 0;
    u8 count = 0;

    check_criteria_values(1, player);
    while (--i >= 0) {
        if (lose_criteria[i].criterion) {
            total += lose_criteria[i].progress;
            count++;
        }
    }
    if (count > 0)
        return (total << 8) / count;
    return 0;
}
