#include "hosp.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct FeelGoodies {
    u16 kind;
    u8 range;
    u8 valid;
    u16 unknown4;
};

struct ShowAnim {
    u16 anim;
    u8 subtypes[4];
    u8 any_subtype;
    u8 illness;
    u16 condition;
    u8 ability;
};

extern FeelGoodies feely[];
extern ShowAnim show_anims[];
extern WinLose lose_criteria[];

extern "C" {

extern u16 D_0015543a;
extern u16 D_0015543c;
extern u8 D_0015543e;
extern u8 D_001553b4;
extern u8 D_0015832c;
extern s16 D_0012dd7c[];
extern s16 D_0012dd98;
extern s16 D_0012dd9a;
extern u16 D_0012dd9c;
extern u16 D_0012dd9e;
extern u16 D_0012dda0;
extern u8 D_0012ddc3;
extern u8 D_0012ddc4;
extern u16 D_0012ddd6;
extern u16 D_0012ddd8;

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
void modify_person(Thing *w)
{
    if (w->flags & 0x40) {
        if (allhosps[w->owner].f1e2s == 2) {
            change_health(w, -1000);
            w->flags &= ~0x40;
            reset_icon_value(w, 6);
            return;
        }
        set_icon_value(w, 6);
        if (w->unknown132)
            change_health(w, 0);
        return;
    }
    if (w->flags & 0x20) {
        if ((s8)w->unknown174 > 0)
            set_icon_value(w, 0xd);
        else if ((s8)w->unknown174 < 0)
            set_icon_value(w, 0x10);
    }
    if (!(D_0015543a > w->unknown87w || w->unknown87w - D_0015543a > 0x7fff))
        return;
    w->unknown87w = D_0015543c;
    if (map[((s16)w->y.tile << 7) + (s16)w->x.tile].unknown7 & 0x10)
        return;
    check_warmth(w);
    check_feelgood(w);
    check_around(w);
    if ((s8)w->unknown174 > 1)
        spread_epidemic(w, w->unknown83w);
    w->unknown124++;
    w->unknown126++;
    w->unknown134--;
    if (old_newrand() % 3 == 0)
        w->unknown136++;
    if ((s16)w->unknown83w > 80) {
        w->unknown138w++;
        if ((s16)w->unknown138w < 0)
            w->unknown138w = 0;
        set_icon_value(w, 0x11);
        w->unknown81--;
    } else if ((s16)w->unknown83w < 20) {
        set_icon_value(w, 0x12);
        w->unknown81--;
    } else {
        reset_icon_value(w, 0x11);
        reset_icon_value(w, 0x12);
        w->unknown81++;
    }
    if (w->unknown81s < D_0012dd7c[9]) {
        set_icon_value(w, 8);
        w->unknown124++;
    } else {
        reset_icon_value(w, 8);
    }
    if (w->unknown136 > D_0012dd7c[13]) {
        set_icon_value(w, 0xb);
        w->unknown124++;
    }
    if (w->unknown134 < 30) {
        set_icon_value(w, 0xc);
        w->unknown124++;
    }
    if (w->unknown81s < 0)
        w->unknown81 = 0;
    else if (w->unknown81s > 100)
        w->unknown81 = 100;
    if (w->unknown134 < 0)
        w->unknown134 = 0;
    change_health(w, -1);
}

// NONMATCHING: warmth and the return-value slot trade places ([ebp-0x18] / [ebp-0x14])
// FUNCTION: 0x000712a8
u16 check_warmth(Thing *w)
{
    u16 warmth;
    u16 dy;
    u16 heat;
    u16 dx;
    u16 dist;
    RoomInfo *room;
    Thing *t;

    warmth = 0;
    heat = allhosps[w->owner].fc94 * 10;
    if (heat) {
        room = which_room((u16)w->x.tile + ((u16)w->y.tile << 7));
        if (room) {
            warmth = room->unknown26 * 1250 / (room->f1 * room->f2);
        } else {
            t = &thing[listhead[w->owner].objects[0x2c]];
            if (t > thing) {
                do {
                    if (!t->unknown89) {
                        dx = ABS(w->x.tile - t->x.tile);
                        if (dx <= 8) {
                            dy = ABS(w->y.tile - t->y.tile);
                            if (dy <= 8) {
                                dist = dx + dy;
                                if (dist)
                                    warmth += heat / dist;
                            }
                        }
                    }
                    t = &thing[t->unknown75w];
                } while (t > thing);
            }
        }
    }
    if (warmth > (s16)w->unknown83w)
        w->unknown83w++;
    else if (warmth < (s16)w->unknown83w)
        w->unknown83w--;
    if (w->renderer == 2 && (s16)w->unknown83w > 60)
        w->unknown134 -= 2;
    if ((s16)w->unknown83w < 0)
        w->unknown83w = 0;
    else if ((s16)w->unknown83w > 100)
        w->unknown83w = 100;
    return warmth;
}

// FUNCTION: 0x00071558
s16 check_feelgood(Thing *w)
{
    s8 mood;
    u16 i;
    Thing *t;

    mood = 0;
    for (i = 0; feely[i].valid != 0; i++) {
        t = &thing[listhead[w->owner].objects[feely[i].kind]];
        if (t > thing) {
            do {
                if (!t->unknown89) {
                    if (ABS(w->x.tile - t->x.tile) <= feely[i].range) {
                        if (ABS(w->y.tile - t->y.tile) <= feely[i].range) {
                            if (t->subtype == 0x2d) {
                                if ((s16)t->unknown100 < 0)
                                    mood--;
                                else
                                    mood++;
                            }
                        }
                    }
                }
                t = &thing[t->unknown75w];
            } while (t > thing);
        }
    }
    if (mood < 0) {
        w->unknown138w++;
        if ((s16)w->unknown138w < 0)
            w->unknown138w = 0;
        w->unknown81--;
    } else if (mood > 0) {
        w->unknown138w--;
        if ((s16)w->unknown138w < 0)
            w->unknown138w = 0;
        w->unknown81++;
    }
    if ((w->kind == 0x18 && abilities[w->subtype][1]) && w->icon != 0x46)
        w->unknown81--;
    return 0;
}

// FUNCTION: 0x000717f0
void check_around(Thing *w)
{
    u16 floor;
    u16 i;
    Map *m;
    Map *base;

    if (!which_room((u16)w->x.tile + ((u16)w->y.tile << 7))) {
        base = &map[((s16)w->y.tile << 7) + (s16)w->x.tile];
        for (i = 0; i < 5; i++) {
            m = &base[around[i]];
            floor = m->floor & 0xf;
            if (floor) {
                if (floor == 5 || floor == 6) {
                    w->unknown138w++;
                    if ((s16)w->unknown138w < 0)
                        w->unknown138w = 0;
                    w->unknown81 -= 2;
                    w->unknown128 += 2;
                } else if (!(floor == 0 || floor == 7 || floor == 8)) {
                    w->unknown138w += 2;
                    if ((s16)w->unknown138w < 0)
                        w->unknown138w = 0;
                    w->unknown81--;
                    w->unknown128++;
                }
            }
        }
    }
}

// NONMATCHING: the target loads D_0012dd9c before calling old_newrand() for chance
// FUNCTION: 0x00071960
u8 do_reaction(Thing *w)
{
    u16 pos;
    u16 illness;
    u16 ok;
    u16 n;
    u16 pick;
    u16 i;
    u16 chance;
    u16 list[5];

    n = 0;
    if (map[((s16)w->y.tile << 7) + (s16)w->x.tile].unknown7 & 0x10)
        return 0;
    pos = (u16)w->x.tile + ((u16)w->y.tile << 7);
    if (which_room(pos) == 0) {
        if (w->flags & 0x200) {
            w->flags &= ~0x200;
            setup_showanim(w, 0x2d);
            return 1;
        }
        if ((s16)w->unknown138w >= D_0012dd9a && old_newrand() % 5 == 0) {
            setup_showanim(w, 0x4b);
            w->unknown138w = 5;
            w->flags |= 4;
            change_health(w, -10);
            return 1;
        }
        if (w->unknown136 >= D_0012dd98 && old_newrand() % 5 == 0) {
            reset_icon_value(w, 0xb);
            setup_showanim(w, 0x66);
            if (w->style == 0x1d)
                w->unknown136 = D_0012dd7c[13] / 2;
            else
                w->unknown136 = 0;
            w->flags |= 8;
            change_health(w, -10);
            return 1;
        }
        if (fn_0007628c(w))
            return 1;
        if (old_newrand() % 200 == 0) {
            for (i = 0; i < 5; i++) {
                illness = show_anims[i].illness;
                if (illness == w->style || illness == 0) {
                    if (show_anims[i].ability == 0 || abilities[w->subtype][show_anims[i].ability]) {
                        if (show_anims[i].any_subtype || show_anims[i].subtypes[w->unknown17]) {
                            ok = 0;
                            switch (show_anims[i].condition) {
                            case 0:
                                ok = 1;
                                break;
                            case 1:
                                if (w->unknown124 > D_0012dd7c[6] / 2)
                                    ok = 1;
                                break;
                            case 2:
                                if (w->unknown124 > D_0012dd7c[6] * 3 / 4)
                                    ok = 1;
                                break;
                            case 3:
                                if (w->unknown124 > D_0012dd7c[6] * 3 / 4)
                                    ok = 1;
                                break;
                            }
                            if (ok)
                                list[n++] = show_anims[i].anim;
                        }
                    }
                }
            }
            if (n > 0) {
                pick = old_newrand() % n;
                setup_showanim(w, list[pick]);
                return 1;
            }
        }
        if (D_0012dd9c > 0)
            chance = old_newrand() % D_0012dd9c;
        else
            chance = 1;
        if (w->unknown128 > D_0012dd7c[5] || chance == 0) {
            if (old_newrand() % 10 == 0) {
                place_litter(pos, (s8)w->owner, old_newrand() % 3 + 1);
                w->unknown128 = 0;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00071dfc
void setup_showanim(Thing *w, u16 anim)
{
    w->unknown103 = w->kind;
    w->unknown85 = w->sprite;
    w->unknown104 = w->palette;
    w->kind = 0x3a;
    w->sprite = start[anim_starts[w->subtype].anims[anim]].frame;
    w->palette = start[anim_starts[w->subtype].anims[anim]].flags;
}

// FUNCTION: 0x00071eb8
void modify_staff(Thing *w)
{
    if (!(D_0015543a > w->unknown87w || w->unknown87w - D_0015543a > 0x7fff))
        return;
    w->unknown87w = D_0015543c;
    if (w->owner >= 4 || D_001553b4)
        return;
    if (w->subtype == 3)
        return;
    calculate_speed(w);
    check_warmth(w);
    staff_check_around(w);
    if (w->kind != 0x34) {
        if (w->room == 0 || w->room->type != 0x19)
            w->f78 += *(u16 *)((u8 *)&expertise[59] + 1);
    }
    if (w->f78 > *(u16 *)&expertise[59].unknown10[1]) {
        if (old_newrand() % 1000 == 0)
            doctor_cracks_up(w);
    }
    if (w->f78 > *(u16 *)&expertise[59].unknown10[1]) {
        set_icon_value(w, 10);
        w->unknown81--;
    } else if (w->f78 < *(u16 *)&expertise[59].unknown5[0]) {
        reset_icon_value(w, 10);
        w->unknown81++;
    }
    if ((s16)w->unknown83w > 80) {
        w->unknown81--;
        set_icon_value(w, 0x11);
    } else if ((s16)w->unknown83w < 20) {
        w->unknown81--;
        set_icon_value(w, 0x12);
    }
    if (w->unknown81s < D_0012dd7c[9])
        w->unknown128++;
    else if (w->unknown81s > D_0012dd7c[8])
        w->unknown128--;
    if (w->unknown81s < 0)
        w->unknown81 = 0;
    else if (w->unknown81s > 100)
        w->unknown81 = 100;
    if (w->f78 < 0)
        w->f78 = 0;
    else if (w->f78 > 1000)
        w->f78 = 1000;
    if ((s16)w->unknown128 < 0)
        w->unknown128 = 0;
}

// FUNCTION: 0x00072110
s16 staff_check_around(Thing *w)
{
    u16 floor;
    u16 i;
    Thing *t;
    Map *base;
    Map *m;

    t = &thing[listhead[w->owner].objects[feely[0].kind]];
    if (t > thing) {
        do {
            if (!t->unknown89) {
                if (ABS(w->x.tile - t->x.tile) <= feely[0].range) {
                    if (ABS(w->y.tile - t->y.tile) <= feely[0].range) {
                        if ((s16)t->unknown100 < 0)
                            w->unknown81--;
                        else
                            w->unknown81++;
                    }
                }
            }
            t = &thing[t->unknown75w];
        } while (t > thing);
    }
    if (!which_room((u16)w->x.tile + ((u16)w->y.tile << 7))) {
        base = &map[((s16)w->y.tile << 7) + (s16)w->x.tile];
        for (i = 0; i < 5; i++) {
            m = &base[around[i]];
            floor = m->floor & 0xf;
            if (floor) {
                if (floor == 5)
                    w->unknown81--;
                else if (!(floor == 0 || floor == 7 || floor == 8))
                    w->unknown81 -= 2;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00072374
void appraise_staff(Thing *w)
{
    s32 value;

    value = w->unknown126 / *(u16 *)&expertise[57].category + *(u16 *)&staff[w->subtype].unknown0[2];
    if (w->unknown124 & 8)
        value += ((s16 *)&expertise[56].unknown10[6])[0];
    if (w->unknown124 & 0x10)
        value += ((s16 *)&expertise[56].unknown10[6])[1];
    if (w->unknown124 & 0x20)
        value += ((s16 *)&expertise[56].unknown10[6])[2];
    if (w->unknown124 & 0x40)
        value += ((s16 *)&expertise[56].unknown10[6])[3];
    if (w->unknown124 & 0x80)
        value += ((s16 *)&expertise[56].unknown10[6])[4];
    if (w->unknown124 & 0x100)
        value += ((s16 *)&expertise[56].unknown10[6])[5];
    w->f70 = value;
    if (w->unknown124 & 0x10) {
        if ((w->unknown124 & 8) && w->unknown126 > D_0012ddd6) {
            if (actual_game_turn - (u16)w->unknown130 >= D_0012ddc3 * 1500) {
                w->unknown124 &= ~8;
                if (w->owner == player)
                    announce_this(10, 1);
            }
        } else if (!(w->unknown124 & 0x80) && w->unknown126 > D_0012ddd8) {
            if (actual_game_turn - (u16)w->unknown130 >= D_0012ddc4 * 1500) {
                w->unknown124 |= 0x80;
                if (w->owner == player)
                    announce_this(0xd, 1);
                if (w->room != 0 && w->room->type == 0x16) {
                    w->state_b = 0x28;
                    set_complex(w, 0x136, w->room->unknown6);
                    fn_0003e724(w);
                }
            }
        }
    }
    if (w->unknown108 < value)
        w->unknown81 += D_0012dd9e;
    else
        w->unknown81 += D_0012dda0;
    if (w->unknown81s < 0)
        w->unknown81 = 0;
    else if (w->unknown81s > 100)
        w->unknown81 = 100;
}

// FUNCTION: 0x000725dc
void reset_all_count(void)
{
    u8 i;

    if (D_0015543e) {
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
void staff_count(Thing *w)
{
    if (D_0015543e) {
        if (w->kind != 0x2e && w->owner == player) {
            switch (w->subtype) {
            case 4:
                adcounts.f23++;
            case 1:
                adcounts.f6++;
                adcounts.f7a[1] += w->f78;
                adcounts.f84[1] += w->unknown81;
                if (w->unknown124 & 0x80)
                    adcounts.f20++;
                if (w->unknown124 & 0x100)
                    adcounts.f21++;
                if (w->unknown124 & 0x40)
                    adcounts.f22++;
                break;
            case 0:
                adcounts.f6++;
                adcounts.f7a[0] += w->f78;
                adcounts.f84[0] += w->unknown81;
                break;
            case 2:
                adcounts.f7a[2] += w->f78;
                adcounts.f84[2] += w->unknown81;
                break;
            case 3:
                adcounts.f7a[3] += w->f78;
                adcounts.f84[3] += w->unknown81;
                break;
            }
            adcounts.f4 += w->unknown81;
            adcounts.f2 += w->f78;
            if (w->f78 > adcounts.f8)
                adcounts.f8 = w->f78;
            if (w->kind == 0x34
                || (((w->room != 0 && w->room->type != 0x16) && w->room->unknown31 == 0) && w->room->unknown32 == 0))
                adcounts.f99_arr[w->subtype]++;
            adcounts.f16 += w->unknown83w;
        }
    }
}

// FUNCTION: 0x00072880
void person_count(Thing *w)
{
    s8 k = 5;

    if (D_0015543e) {
        D_0015832c = 1;
        if (!(map[((s16)w->y.tile << 7) + (s16)w->x.tile].unknown7 & 0x10)) {
            if (w->owner == player) {
                adcounts.f0++;
                adcounts.f48 += w->unknown124;
                adcounts.f4a += w->unknown126;
                while (--k >= 0) {
                    adcounts.f4e_arr[k] += w->unknown126;
                    adcounts.f62_arr[k]++;
                }
                adcounts.fa += w->unknown81;
                if (w->unknown81s > adcounts.fc)
                    adcounts.fc = w->unknown81;
                if (w->unknown81s < adcounts.fe)
                    adcounts.fe = w->unknown81;
                adcounts.f10 += w->unknown134;
                if (w->unknown134 < adcounts.f12)
                    adcounts.f12 = w->unknown134;
                if ((s16)w->unknown83w > adcounts.f18)
                    adcounts.f18 = w->unknown83w;
                if ((s16)w->unknown83w < adcounts.f1a)
                    adcounts.f1a = w->unknown83w;
                adcounts.f14 += w->unknown83w;
                adcounts.f1c += w->unknown136;
                if (w->unknown136 > adcounts.f1e)
                    adcounts.f1e = w->unknown136;
                adcounts.f24[w->style]++;
            }
            if (w->flags & 0x40)
                allhosps[w->owner].f1e7++;
            if ((s8)w->unknown174 != 0)
                allhosps[w->owner].f205++;
            if (~w->flags32 & 0x200000) {
                for (k = 3; --k >= 0;) {
                    mycontrol->history[4][k][11]++;
                    w->flags32 |= 0x200000;
                }
            }
        } else if (w->flags & 0x20) {
            allhosps[w->owner].f207 = 0;
        }
    }
}

// FUNCTION: 0x00072b20
void final_count(void)
{
    if (D_0015543e) {
        myhosp->fc8a = adcounts.fa / adcounts.f0;
        if (adcounts.f6)
            myhosp->fc8c = (s16)adcounts.f4 / (s16)adcounts.f6;
        else
            myhosp->fc8c = 0;
    }
}

// FUNCTION: 0x00072ba4
void object_count(const Thing *w)
{
    if (D_0015543e) {
        if (objects[w->subtype].unknown24) {
            if (w->owner == player) {
                adcounts.f8f++;
                adcounts.f90 += (w->mode << 9) / (w->value * 3);
            }
            if (w->mode > w->value && (w->mode << 9) / (w->value * 3) > adcounts.f94 && w->owner == player) {
                adcounts.f94 = (w->mode << 9) / (w->value * 3);
                adcounts.f98 = w->subtype;
            }
        }
    }
}

// FUNCTION: 0x00072c8c
void check_win_status(void)
{
    if (D_0015543e) {
        evaluate_criteria(player);
        adcounts.fa2 = losing();
    }
}

// FUNCTION: 0x00072cc0
s32 losing(void)
{
    s8 i = 10;
    s32 total = 0;
    u8 n = 0;

    check_criteria_values(1, player);
    while (--i >= 0) {
        if (lose_criteria[i].criterion) {
            total += lose_criteria[i].f14;
            n++;
        }
    }
    if (n > 0)
        return (total << 8) / n;
    return 0;
}
