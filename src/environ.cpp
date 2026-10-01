#include "hosp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern FeelGoodies feely[];
extern ShowAnim show_anims[];
extern WinLose lose_criteria[];

extern "C" {


u8 shall_i_be_abducted(Thing *w);

}

u16 check_warmth(Thing *w);
s16 check_feelgood(Thing *w);
void check_around(Thing *w);
void setup_showanim(Thing *w, u16 anim);
s16 staff_check_around(Thing *w);
void spread_epidemic(Thing *carrier, u16 warmth);
void calculate_speed(Thing *w);
void doctor_cracks_up(Thing *w);
s32 evaluate_criteria(const u8 player);
u32 check_criteria_values(const u32 a, const u8 player);
s32 losing(void);

// Periodic update of patient person: an emergency patient dies when the
// emergency ends and otherwise only shows the emergency icon; a contagious one
// shows contagious or vaccinated. When due (Thing.modify_time) and inside the
// building (map unknown7 flag 0x10 clear), applies warmth, plants, mess and
// contagion, makes them more impatient, thirsty and in need of the toilet,
// adjusts happiness and status icons, and costs 1 health.
// FUNCTION: 0x00070f80
void modify_person(Thing *person)
{
    if (person->flags & PF_EMERGENCY) {
        if (allhosps[person->owner].f1e2s == 2) {
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
    if (!(othervar.f82 > person->modify_time || person->modify_time - othervar.f82 > 0x7fff))
        return;
    person->modify_time = othervar.f84;
    if (map[((s16)person->y.tile << 7) + (s16)person->x.tile].unknown7 & 0x10)
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
    if (person->happiness_s < gbv.f10e) {
        set_icon_value(person, ICON_UNHAPPY);
        person->impatience++;
    } else {
        reset_icon_value(person, ICON_UNHAPPY);
    }
    if (person->bladder > gbv.f116) {
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
// unknown26 * 1250 spread over its area, or in the corridor the nearby corridor
// radiators by distance and the heating setting (allhosps.fc94); 0 with the
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
    heat = allhosps[person->owner].fc94 * 10;
    if (heat) {
        room = which_room((u16)person->x.tile + ((u16)person->y.tile << 7));
        if (room) {
            warmth = room->unknown26 * 1250 / (room->f1 * room->f2);
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
            floor = m->floor & 0xf;
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
// plays a pending animation (flags bit 0x200), vomits when nauseous or wets
// when desperate (each costing health), or rarely plays a fitting idle
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
    if (map[((s16)peep->y.tile << 7) + (s16)peep->x.tile].unknown7 & 0x10)
        return 0;
    pos = (u16)peep->x.tile + ((u16)peep->y.tile << 7);
    if (which_room(pos) == 0) {
        if (peep->flags & 0x200) {
            peep->flags &= ~0x200;
            setup_showanim(peep, 0x2d);
            return 1;
        }
        if ((s16)peep->nausea >= gbv.f11a && old_newrand() % 5 == 0) {
            setup_showanim(peep, 0x4b);
            peep->nausea = 5;
            peep->flags |= PF_VOMITS;
            change_health(peep, -10);
            return 1;
        }
        if (peep->bladder >= gbv.f118 && old_newrand() % 5 == 0) {
            reset_icon_value(peep, ICON_TOILET);
            setup_showanim(peep, 0x66);
            if (peep->illness == ILL_THE_SQUITS)
                peep->bladder = gbv.f116 / 2;
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
                        if (show_anims[i].any_subtype || show_anims[i].subtypes[peep->facing]) {
                            ok = 0;
                            switch (show_anims[i].condition) {
                            case 0:
                                ok = 1;
                                break;
                            case 1:
                                if (peep->impatience > gbv.f108 / 2)
                                    ok = 1;
                                break;
                            case 2:
                                if (peep->impatience > gbv.f108 * 3 / 4)
                                    ok = 1;
                                break;
                            case 3:
                                if (peep->impatience > gbv.f108 * 3 / 4)
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
        if (gbv.f11c > 0)
            chance = old_newrand() % gbv.f11c;
        else
            chance = 1;
        if (peep->litter_urge > gbv.f106 || chance == 0) {
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
    if (!(othervar.f82 > employee->modify_time || employee->modify_time - othervar.f82 > 0x7fff))
        return;
    employee->modify_time = othervar.f84;
    if (employee->owner >= 4 || gamevar.f174)
        return;
    if (employee->subtype == STAFF_RECEPTIONIST)
        return;
    calculate_speed(employee);
    check_warmth(employee);
    staff_check_around(employee);
    if (employee->state != SS_WANDERS) {
        if (employee->room == 0 || employee->room->type != ROOM_STAFF_ROOM)
            employee->tiredness += gbv.fe2;
    }
    if (employee->tiredness > gbv.fec) {
        if (old_newrand() % 1000 == 0)
            doctor_cracks_up(employee);
    }
    if (employee->tiredness > gbv.fec) {
        set_icon_value(employee, ICON_TIRED);
        employee->happiness--;
    } else if (employee->tiredness < gbv.fe6) {
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
    if (employee->happiness_s < gbv.f10e)
        employee->discontent++;
    else if (employee->happiness_s > gbv.f10c)
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
            floor = m->floor & 0xf;
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

    value = employee->ability / gbv.fc4 + *(u16 *)&staff[employee->subtype].unknown0[2];
    if (employee->skills & SKILL_JUNIOR)
        value += gbv.fb8[0];
    if (employee->skills & SKILL_DOCTOR)
        value += gbv.fb8[1];
    if (employee->skills & SKILL_SURGEON)
        value += gbv.fb8[2];
    if (employee->skills & SKILL_PSYCHIATRIST)
        value += gbv.fb8[3];
    if (employee->skills & SKILL_CONSULTANT)
        value += gbv.fb8[4];
    if (employee->skills & SKILL_RESEARCHER)
        value += gbv.fb8[5];
    employee->wage_demand = value;
    if (employee->skills & SKILL_DOCTOR) {
        if ((employee->skills & SKILL_JUNIOR) && employee->ability > gbv.f156) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.f143 * 1500) {
                employee->skills &= ~8;
                if (employee->owner == player)
                    announce_this(10, 1);
            }
        } else if (!(employee->skills & SKILL_CONSULTANT) && employee->ability > gbv.f158) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.f144 * 1500) {
                employee->skills |= SKILL_CONSULTANT;
                if (employee->owner == player)
                    announce_this(0xd, 1);
                if (employee->room != 0 && employee->room->type == ROOM_TRAINING) {
                    employee->next_state = SS_IN_CORRIDOR;
                    set_complex(employee, 0x136, employee->room->unknown6);
                    process_staff_complex(employee);
                }
            }
        }
    }
    if (employee->salary < value)
        employee->happiness += gbv.f11e;
    else
        employee->happiness += gbv.f120;
    if (employee->happiness_s < 0)
        employee->happiness = 0;
    else if (employee->happiness_s > 100)
        employee->happiness = 100;
}

// Starts a statistics pass (when othervar.f86 is set): clears the advisor
// counts and each hospital's emergency and contagion counts.
// FUNCTION: 0x000725dc
void reset_all_count(void)
{
    u8 i;

    if (othervar.f86) {
        memset(&adcounts, 0, 0xa6);
        adcounts.f0 = 1;
        adcounts.fe = 999;
        adcounts.f1a = 999;
        adcounts.f12 = 999;
        for (i = 0; i < 4; i++) {
            allhosps[i].f1e7 = 0;
            allhosps[i].f205 = 0;
        }
    }
}

// Adds employee (the local player's, not applicants) to the statistics pass:
// staff counts, tiredness, happiness and warmth totals, and idle staff.
// FUNCTION: 0x00072684
void staff_count(Thing *employee)
{
    if (othervar.f86) {
        if (employee->state != SS_APPLICANT && employee->owner == player) {
            switch (employee->subtype) {
            case STAFF_SURGEON:
                adcounts.f23++;
            case STAFF_DOCTOR:
                adcounts.f6++;
                adcounts.f7a[1] += employee->tiredness;
                adcounts.f84[1] += employee->happiness;
                if (employee->skills & SKILL_CONSULTANT)
                    adcounts.f20++;
                if (employee->skills & SKILL_RESEARCHER)
                    adcounts.f21++;
                if (employee->skills & SKILL_PSYCHIATRIST)
                    adcounts.f22++;
                break;
            case STAFF_NURSE:
                adcounts.f6++;
                adcounts.f7a[0] += employee->tiredness;
                adcounts.f84[0] += employee->happiness;
                break;
            case STAFF_HANDYMAN:
                adcounts.f7a[2] += employee->tiredness;
                adcounts.f84[2] += employee->happiness;
                break;
            case STAFF_RECEPTIONIST:
                adcounts.f7a[3] += employee->tiredness;
                adcounts.f84[3] += employee->happiness;
                break;
            }
            adcounts.f4 += employee->happiness;
            adcounts.f2 += employee->tiredness;
            if (employee->tiredness > adcounts.f8)
                adcounts.f8 = employee->tiredness;
            if (employee->state == SS_WANDERS
                || (((employee->room != 0 && employee->room->type != ROOM_TRAINING) && employee->room->unknown31 == 0) && employee->room->unknown32 == 0))
                adcounts.f99_arr[employee->subtype]++;
            adcounts.f16 += employee->warmth;
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

    if (othervar.f86) {
        advisor.f130 = 1;
        if (!(map[((s16)person->y.tile << 7) + (s16)person->x.tile].unknown7 & 0x10)) {
            if (person->owner == player) {
                adcounts.f0++;
                adcounts.f48 += person->impatience;
                adcounts.f4a += person->waiting_time;
                while (--k >= 0) {
                    adcounts.f4e_arr[k] += person->waiting_time;
                    adcounts.f62_arr[k]++;
                }
                adcounts.fa += person->happiness;
                if (person->happiness_s > adcounts.fc)
                    adcounts.fc = person->happiness;
                if (person->happiness_s < adcounts.fe)
                    adcounts.fe = person->happiness;
                adcounts.f10 += person->refreshment;
                if (person->refreshment < adcounts.f12)
                    adcounts.f12 = person->refreshment;
                if ((s16)person->warmth > adcounts.f18)
                    adcounts.f18 = person->warmth;
                if ((s16)person->warmth < adcounts.f1a)
                    adcounts.f1a = person->warmth;
                adcounts.f14 += person->warmth;
                adcounts.f1c += person->bladder;
                if (person->bladder > adcounts.f1e)
                    adcounts.f1e = person->bladder;
                adcounts.f24[person->illness]++;
            }
            if (person->flags & PF_EMERGENCY)
                allhosps[person->owner].f1e7++;
            if ((s8)person->contagion != 0)
                allhosps[person->owner].f205++;
            if (~person->flags32 & 0x200000) {
                for (k = 3; --k >= 0;) {
                    mycontrol->history[4][k][11]++;
                    person->flags32 |= 0x200000;
                }
            }
        } else if (person->flags & PF_CONTAGIOUS) {
            allhosps[person->owner].f207 = 0;
        }
    }
}

// Ends a statistics pass: stores the local player's average patient and staff
// happiness.
// FUNCTION: 0x00072b20
void final_count(void)
{
    if (othervar.f86) {
        myhosp->fc8a = adcounts.fa / adcounts.f0;
        if (adcounts.f6)
            myhosp->fc8c = (s16)adcounts.f4 / (s16)adcounts.f6;
        else
            myhosp->fc8c = 0;
    }
}

// Adds machine obj to the statistics pass: the local player's machine count
// and wear, and the most worn machine past its strength.
// FUNCTION: 0x00072ba4
void object_count(const Thing *obj)
{
    if (othervar.f86) {
        if (objects[obj->subtype].unknown24) {
            if (obj->owner == player) {
                adcounts.f8f++;
                adcounts.f90 += (obj->times_used << 9) / (obj->strength * 3);
            }
            if (obj->times_used > obj->strength) {
                if ((obj->times_used << 9) / (obj->strength * 3) > adcounts.f94) {
                    if (obj->owner == player) {
                        adcounts.f94 = (obj->times_used << 9) / (obj->strength * 3);
                        adcounts.f98 = obj->subtype;
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
    if (othervar.f86) {
        evaluate_criteria(player);
        adcounts.fa2 = losing();
    }
}

// Returns how close the local player is to losing: the average progress
// (lose_criteria[].f14) of the lose criteria in use, times 256.
// FUNCTION: 0x00072cc0
s32 losing(void)
{
    s8 i = 10;
    s32 total = 0;
    u8 count = 0;

    check_criteria_values(1, player);
    while (--i >= 0) {
        if (lose_criteria[i].criterion) {
            total += lose_criteria[i].f14;
            count++;
        }
    }
    if (count > 0)
        return (total << 8) / count;
    return 0;
}
