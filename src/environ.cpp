#include "hosp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern FeelGoodies feely[];
extern ShowAnim show_anims[];
extern WinLose lose_criteria[];

extern "C" {


u8 fn_0007628c(Thing *w);

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

// FUNCTION: 0x00070f80
void modify_person(Thing *person)
{
    if (person->flags & 0x40) {
        if (allhosps[person->owner].f1e2s == 2) {
            change_health(person, -1000);
            person->flags &= ~0x40;
            reset_icon_value(person, 6);
            return;
        }
        set_icon_value(person, 6);
        if (person->health_delta)
            change_health(person, 0);
        return;
    }
    if (person->flags & 0x20) {
        if ((s8)person->contagion > 0)
            set_icon_value(person, 0xd);
        else if ((s8)person->contagion < 0)
            set_icon_value(person, 0x10);
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
        set_icon_value(person, 0x11);
        person->happiness--;
    } else if ((s16)person->warmth < 20) {
        set_icon_value(person, 0x12);
        person->happiness--;
    } else {
        reset_icon_value(person, 0x11);
        reset_icon_value(person, 0x12);
        person->happiness++;
    }
    if (person->happiness_s < gbv.f10e) {
        set_icon_value(person, 8);
        person->impatience++;
    } else {
        reset_icon_value(person, 8);
    }
    if (person->bladder > gbv.f116) {
        set_icon_value(person, 0xb);
        person->impatience++;
    }
    if (person->refreshment < 30) {
        set_icon_value(person, 0xc);
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
            radiator = &thing[listhead[person->owner].objects[0x2c]];
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
    if (person->type == 2 && (s16)person->warmth > 60)
        person->refreshment -= 2;
    if ((s16)person->warmth < 0)
        person->warmth = 0;
    else if ((s16)person->warmth > 100)
        person->warmth = 100;
    return warmth;
}

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
                            if (obj->subtype == 0x2d) {
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
    if ((person->state == 0x18 && abilities[person->subtype][1]) && person->complex_action != 0x46)
        person->happiness--;
    return 0;
}

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
            peep->flags |= 4;
            change_health(peep, -10);
            return 1;
        }
        if (peep->bladder >= gbv.f118 && old_newrand() % 5 == 0) {
            reset_icon_value(peep, 0xb);
            setup_showanim(peep, 0x66);
            if (peep->illness == 0x1d)
                peep->bladder = gbv.f116 / 2;
            else
                peep->bladder = 0;
            peep->flags |= 8;
            change_health(peep, -10);
            return 1;
        }
        if (fn_0007628c(peep))
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

// FUNCTION: 0x00071dfc
void setup_showanim(Thing *peep, u16 anim)
{
    peep->substate = peep->state;
    peep->saved_sprite = peep->sprite;
    peep->saved_flip = peep->flip;
    peep->state = 0x3a;
    peep->sprite = start[anim_starts[peep->subtype].anims[anim]].frame;
    peep->flip = start[anim_starts[peep->subtype].anims[anim]].flags;
}

// FUNCTION: 0x00071eb8
void modify_staff(Thing *employee)
{
    if (!(othervar.f82 > employee->modify_time || employee->modify_time - othervar.f82 > 0x7fff))
        return;
    employee->modify_time = othervar.f84;
    if (employee->owner >= 4 || gamevar.f174)
        return;
    if (employee->subtype == 3)
        return;
    calculate_speed(employee);
    check_warmth(employee);
    staff_check_around(employee);
    if (employee->state != 0x34) {
        if (employee->room == 0 || employee->room->type != 0x19)
            employee->tiredness += gbv.fe2;
    }
    if (employee->tiredness > gbv.fec) {
        if (old_newrand() % 1000 == 0)
            doctor_cracks_up(employee);
    }
    if (employee->tiredness > gbv.fec) {
        set_icon_value(employee, 10);
        employee->happiness--;
    } else if (employee->tiredness < gbv.fe6) {
        reset_icon_value(employee, 10);
        employee->happiness++;
    }
    if ((s16)employee->warmth > 80) {
        employee->happiness--;
        set_icon_value(employee, 0x11);
    } else if ((s16)employee->warmth < 20) {
        employee->happiness--;
        set_icon_value(employee, 0x12);
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

// FUNCTION: 0x00072374
void appraise_staff(Thing *employee)
{
    s32 value;

    value = employee->ability / gbv.fc4 + *(u16 *)&staff[employee->subtype].unknown0[2];
    if (employee->skills & 8)
        value += gbv.fb8[0];
    if (employee->skills & 0x10)
        value += gbv.fb8[1];
    if (employee->skills & 0x20)
        value += gbv.fb8[2];
    if (employee->skills & 0x40)
        value += gbv.fb8[3];
    if (employee->skills & 0x80)
        value += gbv.fb8[4];
    if (employee->skills & 0x100)
        value += gbv.fb8[5];
    employee->wage_demand = value;
    if (employee->skills & 0x10) {
        if ((employee->skills & 8) && employee->ability > gbv.f156) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.f143 * 1500) {
                employee->skills &= ~8;
                if (employee->owner == player)
                    announce_this(10, 1);
            }
        } else if (!(employee->skills & 0x80) && employee->ability > gbv.f158) {
            if (actual_game_turn - (u16)employee->hire_time >= gbv.f144 * 1500) {
                employee->skills |= 0x80;
                if (employee->owner == player)
                    announce_this(0xd, 1);
                if (employee->room != 0 && employee->room->type == 0x16) {
                    employee->next_state = 0x28;
                    set_complex(employee, 0x136, employee->room->unknown6);
                    fn_0003e724(employee);
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

// FUNCTION: 0x00072684
void staff_count(Thing *employee)
{
    if (othervar.f86) {
        if (employee->state != 0x2e && employee->owner == player) {
            switch (employee->subtype) {
            case 4:
                adcounts.f23++;
            case 1:
                adcounts.f6++;
                adcounts.f7a[1] += employee->tiredness;
                adcounts.f84[1] += employee->happiness;
                if (employee->skills & 0x80)
                    adcounts.f20++;
                if (employee->skills & 0x100)
                    adcounts.f21++;
                if (employee->skills & 0x40)
                    adcounts.f22++;
                break;
            case 0:
                adcounts.f6++;
                adcounts.f7a[0] += employee->tiredness;
                adcounts.f84[0] += employee->happiness;
                break;
            case 2:
                adcounts.f7a[2] += employee->tiredness;
                adcounts.f84[2] += employee->happiness;
                break;
            case 3:
                adcounts.f7a[3] += employee->tiredness;
                adcounts.f84[3] += employee->happiness;
                break;
            }
            adcounts.f4 += employee->happiness;
            adcounts.f2 += employee->tiredness;
            if (employee->tiredness > adcounts.f8)
                adcounts.f8 = employee->tiredness;
            if (employee->state == 0x34
                || (((employee->room != 0 && employee->room->type != 0x16) && employee->room->unknown31 == 0) && employee->room->unknown32 == 0))
                adcounts.f99_arr[employee->subtype]++;
            adcounts.f16 += employee->warmth;
        }
    }
}

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
            if (person->flags & 0x40)
                allhosps[person->owner].f1e7++;
            if ((s8)person->contagion != 0)
                allhosps[person->owner].f205++;
            if (~person->flags32 & 0x200000) {
                for (k = 3; --k >= 0;) {
                    mycontrol->history[4][k][11]++;
                    person->flags32 |= 0x200000;
                }
            }
        } else if (person->flags & 0x20) {
            allhosps[person->owner].f207 = 0;
        }
    }
}

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

// FUNCTION: 0x00072c8c
void check_win_status(void)
{
    if (othervar.f86) {
        evaluate_criteria(player);
        adcounts.fa2 = losing();
    }
}

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
