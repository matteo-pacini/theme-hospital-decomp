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
void spread_epidemic(Thing *w, u16 a);
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
        if (person->unknown132)
            change_health(person, 0);
        return;
    }
    if (person->flags & 0x20) {
        if ((s8)person->unknown174 > 0)
            set_icon_value(person, 0xd);
        else if ((s8)person->unknown174 < 0)
            set_icon_value(person, 0x10);
    }
    if (!(othervar.f82 > person->unknown87w || person->unknown87w - othervar.f82 > 0x7fff))
        return;
    person->unknown87w = othervar.f84;
    if (map[((s16)person->y.tile << 7) + (s16)person->x.tile].unknown7 & 0x10)
        return;
    check_warmth(person);
    check_feelgood(person);
    check_around(person);
    if ((s8)person->unknown174 > 1)
        spread_epidemic(person, person->unknown83w);
    person->unknown124++;
    person->unknown126++;
    person->unknown134--;
    if (old_newrand() % 3 == 0)
        person->unknown136++;
    if ((s16)person->unknown83w > 80) {
        person->unknown138w++;
        if ((s16)person->unknown138w < 0)
            person->unknown138w = 0;
        set_icon_value(person, 0x11);
        person->unknown81--;
    } else if ((s16)person->unknown83w < 20) {
        set_icon_value(person, 0x12);
        person->unknown81--;
    } else {
        reset_icon_value(person, 0x11);
        reset_icon_value(person, 0x12);
        person->unknown81++;
    }
    if (person->unknown81s < gbv.f10e) {
        set_icon_value(person, 8);
        person->unknown124++;
    } else {
        reset_icon_value(person, 8);
    }
    if (person->unknown136 > gbv.f116) {
        set_icon_value(person, 0xb);
        person->unknown124++;
    }
    if (person->unknown134 < 30) {
        set_icon_value(person, 0xc);
        person->unknown124++;
    }
    if (person->unknown81s < 0)
        person->unknown81 = 0;
    else if (person->unknown81s > 100)
        person->unknown81 = 100;
    if (person->unknown134 < 0)
        person->unknown134 = 0;
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
                    if (!radiator->unknown89) {
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
                    radiator = &thing[radiator->unknown75w];
                } while (radiator > thing);
            }
        }
    }
    if (warmth > (s16)person->unknown83w)
        person->unknown83w++;
    else if (warmth < (s16)person->unknown83w)
        person->unknown83w--;
    if (person->renderer == 2 && (s16)person->unknown83w > 60)
        person->unknown134 -= 2;
    if ((s16)person->unknown83w < 0)
        person->unknown83w = 0;
    else if ((s16)person->unknown83w > 100)
        person->unknown83w = 100;
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
                if (!obj->unknown89) {
                    if (ABS(person->x.tile - obj->x.tile) <= feely[i].range) {
                        if (ABS(person->y.tile - obj->y.tile) <= feely[i].range) {
                            if (obj->subtype == 0x2d) {
                                if ((s16)obj->unknown100 < 0)
                                    mood--;
                                else
                                    mood++;
                            }
                        }
                    }
                }
                obj = &thing[obj->unknown75w];
            } while (obj > thing);
        }
    }
    if (mood < 0) {
        person->unknown138w++;
        if ((s16)person->unknown138w < 0)
            person->unknown138w = 0;
        person->unknown81--;
    } else if (mood > 0) {
        person->unknown138w--;
        if ((s16)person->unknown138w < 0)
            person->unknown138w = 0;
        person->unknown81++;
    }
    if ((person->kind == 0x18 && abilities[person->subtype][1]) && person->icon != 0x46)
        person->unknown81--;
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
                    person->unknown138w++;
                    if ((s16)person->unknown138w < 0)
                        person->unknown138w = 0;
                    person->unknown81 -= 2;
                    person->unknown128 += 2;
                } else if (!(floor == 0 || floor == 7 || floor == 8)) {
                    person->unknown138w += 2;
                    if ((s16)person->unknown138w < 0)
                        person->unknown138w = 0;
                    person->unknown81--;
                    person->unknown128++;
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
        if ((s16)peep->unknown138w >= gbv.f11a && old_newrand() % 5 == 0) {
            setup_showanim(peep, 0x4b);
            peep->unknown138w = 5;
            peep->flags |= 4;
            change_health(peep, -10);
            return 1;
        }
        if (peep->unknown136 >= gbv.f118 && old_newrand() % 5 == 0) {
            reset_icon_value(peep, 0xb);
            setup_showanim(peep, 0x66);
            if (peep->style == 0x1d)
                peep->unknown136 = gbv.f116 / 2;
            else
                peep->unknown136 = 0;
            peep->flags |= 8;
            change_health(peep, -10);
            return 1;
        }
        if (fn_0007628c(peep))
            return 1;
        if (old_newrand() % 200 == 0) {
            for (i = 0; i < 5; i++) {
                illness = show_anims[i].illness;
                if (illness == peep->style || illness == 0) {
                    if (show_anims[i].ability == 0 || abilities[peep->subtype][show_anims[i].ability]) {
                        if (show_anims[i].any_subtype || show_anims[i].subtypes[peep->unknown17]) {
                            ok = 0;
                            switch (show_anims[i].condition) {
                            case 0:
                                ok = 1;
                                break;
                            case 1:
                                if (peep->unknown124 > gbv.f108 / 2)
                                    ok = 1;
                                break;
                            case 2:
                                if (peep->unknown124 > gbv.f108 * 3 / 4)
                                    ok = 1;
                                break;
                            case 3:
                                if (peep->unknown124 > gbv.f108 * 3 / 4)
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
        if (peep->unknown128 > gbv.f106 || chance == 0) {
            if (old_newrand() % 10 == 0) {
                place_litter(pos, (s8)peep->owner, old_newrand() % 3 + 1);
                peep->unknown128 = 0;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00071dfc
void setup_showanim(Thing *peep, u16 anim)
{
    peep->unknown103 = peep->kind;
    peep->unknown85 = peep->sprite;
    peep->unknown104 = peep->palette;
    peep->kind = 0x3a;
    peep->sprite = start[anim_starts[peep->subtype].anims[anim]].frame;
    peep->palette = start[anim_starts[peep->subtype].anims[anim]].flags;
}

// FUNCTION: 0x00071eb8
void modify_staff(Thing *employee)
{
    if (!(othervar.f82 > employee->unknown87w || employee->unknown87w - othervar.f82 > 0x7fff))
        return;
    employee->unknown87w = othervar.f84;
    if (employee->owner >= 4 || gamevar.f174)
        return;
    if (employee->subtype == 3)
        return;
    calculate_speed(employee);
    check_warmth(employee);
    staff_check_around(employee);
    if (employee->kind != 0x34) {
        if (employee->room == 0 || employee->room->type != 0x19)
            employee->f78 += gbv.fe2;
    }
    if (employee->f78 > gbv.fec) {
        if (old_newrand() % 1000 == 0)
            doctor_cracks_up(employee);
    }
    if (employee->f78 > gbv.fec) {
        set_icon_value(employee, 10);
        employee->unknown81--;
    } else if (employee->f78 < gbv.fe6) {
        reset_icon_value(employee, 10);
        employee->unknown81++;
    }
    if ((s16)employee->unknown83w > 80) {
        employee->unknown81--;
        set_icon_value(employee, 0x11);
    } else if ((s16)employee->unknown83w < 20) {
        employee->unknown81--;
        set_icon_value(employee, 0x12);
    }
    if (employee->unknown81s < gbv.f10e)
        employee->unknown128++;
    else if (employee->unknown81s > gbv.f10c)
        employee->unknown128--;
    if (employee->unknown81s < 0)
        employee->unknown81 = 0;
    else if (employee->unknown81s > 100)
        employee->unknown81 = 100;
    if (employee->f78 < 0)
        employee->f78 = 0;
    else if (employee->f78 > 1000)
        employee->f78 = 1000;
    if ((s16)employee->unknown128 < 0)
        employee->unknown128 = 0;
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
            if (!plant->unknown89) {
                if (ABS(employee->x.tile - plant->x.tile) <= feely[0].range) {
                    if (ABS(employee->y.tile - plant->y.tile) <= feely[0].range) {
                        if ((s16)plant->unknown100 < 0)
                            employee->unknown81--;
                        else
                            employee->unknown81++;
                    }
                }
            }
            plant = &thing[plant->unknown75w];
        } while (plant > thing);
    }
    if (!which_room((u16)employee->x.tile + ((u16)employee->y.tile << 7))) {
        base = &map[((s16)employee->y.tile << 7) + (s16)employee->x.tile];
        for (i = 0; i < 5; i++) {
            m = &base[around[i]];
            floor = m->floor & 0xf;
            if (floor) {
                if (floor == 5)
                    employee->unknown81--;
                else if (!(floor == 0 || floor == 7 || floor == 8))
                    employee->unknown81 -= 2;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00072374
void appraise_staff(Thing *employee)
{
    s32 value;

    value = employee->unknown126 / gbv.fc4 + *(u16 *)&staff[employee->subtype].unknown0[2];
    if (employee->unknown124 & 8)
        value += gbv.fb8[0];
    if (employee->unknown124 & 0x10)
        value += gbv.fb8[1];
    if (employee->unknown124 & 0x20)
        value += gbv.fb8[2];
    if (employee->unknown124 & 0x40)
        value += gbv.fb8[3];
    if (employee->unknown124 & 0x80)
        value += gbv.fb8[4];
    if (employee->unknown124 & 0x100)
        value += gbv.fb8[5];
    employee->f70 = value;
    if (employee->unknown124 & 0x10) {
        if ((employee->unknown124 & 8) && employee->unknown126 > gbv.f156) {
            if (actual_game_turn - (u16)employee->unknown130 >= gbv.f143 * 1500) {
                employee->unknown124 &= ~8;
                if (employee->owner == player)
                    announce_this(10, 1);
            }
        } else if (!(employee->unknown124 & 0x80) && employee->unknown126 > gbv.f158) {
            if (actual_game_turn - (u16)employee->unknown130 >= gbv.f144 * 1500) {
                employee->unknown124 |= 0x80;
                if (employee->owner == player)
                    announce_this(0xd, 1);
                if (employee->room != 0 && employee->room->type == 0x16) {
                    employee->state_b = 0x28;
                    set_complex(employee, 0x136, employee->room->unknown6);
                    fn_0003e724(employee);
                }
            }
        }
    }
    if (employee->unknown108 < value)
        employee->unknown81 += gbv.f11e;
    else
        employee->unknown81 += gbv.f120;
    if (employee->unknown81s < 0)
        employee->unknown81 = 0;
    else if (employee->unknown81s > 100)
        employee->unknown81 = 100;
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
        if (employee->kind != 0x2e && employee->owner == player) {
            switch (employee->subtype) {
            case 4:
                adcounts.f23++;
            case 1:
                adcounts.f6++;
                adcounts.f7a[1] += employee->f78;
                adcounts.f84[1] += employee->unknown81;
                if (employee->unknown124 & 0x80)
                    adcounts.f20++;
                if (employee->unknown124 & 0x100)
                    adcounts.f21++;
                if (employee->unknown124 & 0x40)
                    adcounts.f22++;
                break;
            case 0:
                adcounts.f6++;
                adcounts.f7a[0] += employee->f78;
                adcounts.f84[0] += employee->unknown81;
                break;
            case 2:
                adcounts.f7a[2] += employee->f78;
                adcounts.f84[2] += employee->unknown81;
                break;
            case 3:
                adcounts.f7a[3] += employee->f78;
                adcounts.f84[3] += employee->unknown81;
                break;
            }
            adcounts.f4 += employee->unknown81;
            adcounts.f2 += employee->f78;
            if (employee->f78 > adcounts.f8)
                adcounts.f8 = employee->f78;
            if (employee->kind == 0x34
                || (((employee->room != 0 && employee->room->type != 0x16) && employee->room->unknown31 == 0) && employee->room->unknown32 == 0))
                adcounts.f99_arr[employee->subtype]++;
            adcounts.f16 += employee->unknown83w;
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
                adcounts.f48 += person->unknown124;
                adcounts.f4a += person->unknown126;
                while (--k >= 0) {
                    adcounts.f4e_arr[k] += person->unknown126;
                    adcounts.f62_arr[k]++;
                }
                adcounts.fa += person->unknown81;
                if (person->unknown81s > adcounts.fc)
                    adcounts.fc = person->unknown81;
                if (person->unknown81s < adcounts.fe)
                    adcounts.fe = person->unknown81;
                adcounts.f10 += person->unknown134;
                if (person->unknown134 < adcounts.f12)
                    adcounts.f12 = person->unknown134;
                if ((s16)person->unknown83w > adcounts.f18)
                    adcounts.f18 = person->unknown83w;
                if ((s16)person->unknown83w < adcounts.f1a)
                    adcounts.f1a = person->unknown83w;
                adcounts.f14 += person->unknown83w;
                adcounts.f1c += person->unknown136;
                if (person->unknown136 > adcounts.f1e)
                    adcounts.f1e = person->unknown136;
                adcounts.f24[person->style]++;
            }
            if (person->flags & 0x40)
                allhosps[person->owner].f1e7++;
            if ((s8)person->unknown174 != 0)
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
                adcounts.f90 += (obj->mode << 9) / (obj->value * 3);
            }
            if (obj->mode > obj->value) {
                if ((obj->mode << 9) / (obj->value * 3) > adcounts.f94) {
                    if (obj->owner == player) {
                        adcounts.f94 = (obj->mode << 9) / (obj->value * 3);
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
