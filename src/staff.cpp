#include "hosp.h"

extern "C" {
extern u16 D_00155d89;
extern u8 D_001553af;
extern u8 D_001553b0;
extern u8 D_001553b1;
extern u8 D_001553b2;
extern u8 D_001553b3;
extern u8 D_00158329;
extern u8 D_0015832a;
extern u8 D_0015832b;
extern u8 D_001553c7;
void fn_000b05f0(Thing *w);
void fn_000168a0(Thing *w);
extern u16 D_0012ddd6;
extern u16 D_0012ddd8;
extern u8 D_00156429;
extern u8 D_0015642a;
extern u8 D_0015642b;
}

s16 place_staff_in_struct(u16 a, u16 b);
s16 get_state(const u16 type, const Thing *w);
void change_lockholder(Thing *w, RoomInfo *room);
void calculate_speed(Thing *w);
void inc_staff_stats(Thing *w, u8 owner);
void set_expertise_staff(Thing *w);
void insert_skill(u16 skill, u8 owner);
void reinit_nurse(Thing *w);
void pager_arrival(Thing *w);
u16 staffrand(void);
u32 normal_staffrand(u32 mean, u32 spread);
void appraise_staff(Thing *w);
void remove_person_from_room_list(Thing *w);
void staff_exit_room_check(Thing *w);
void remove_expertise_staff(Thing *w);

// NONMATCHING: register choice
// FUNCTION: 0x00029ad0
void create_job_market(void)
{
    u32 i;
    u32 n;
    u32 j;

    for (i = 0; i < 30; i++) {
        if (staff_levels[i].month == D_00155d89) {
            gamevar.f0 = staff_levels[i].fund;
            gamevar.f10[0] = staff_levels[i].counts[0];
            gamevar.f10[1] = staff_levels[i].counts[1];
            gamevar.f10[2] = staff_levels[i].counts[2];
            gamevar.f10[3] = staff_levels[i].counts[3];
            D_001553af = staff_levels[i].f0a[0];
            D_001553b0 = staff_levels[i].f0a[1];
            D_001553b1 = staff_levels[i].f0a[2];
            D_001553b2 = staff_levels[i].f0a[3];
            D_001553b3 = staff_levels[i].f0a[4];
            break;
        }
        if (staff_levels[i].month > D_00155d89) {
            break;
        }
    }
    for (i = 0; i <= 3; i++) {
        if (gamevar.f6[i] < gamevar.f10[i]) {
            n = gamevar.f10[i] - gamevar.f6[i];
            for (j = 0; j < n; j++) {
                place_staff_in_struct(i, 0);
            }
        }
    }
}

// NONMATCHING: register choice, u8 temp slot order
// FUNCTION: 0x00029c4c
u16 fn_00029c4c(u16 tile, u16 id, u8 flag, u8 owner)
{
    Thing *w;

    w = &thing[id];
    if (w > thing) {
        if (flag || map[tile].unknown7 == (1 << owner)) {
            w->owner = owner;
            if (w->unknown17 > 3) {
                w->unknown17 = 0;
            }
            {
                u16 idx;

                idx = check_frame(staff_anim_starts[w->subtype].anims[staff[w->subtype].anim], w) + w->unknown17;
                w->sprite = start[idx].frame;
                w->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
            }
            w->x.tile = tile % 128;
            w->y.tile = tile / 128;
            w->x.frac = 0x80;
            w->y.frac = 0x80;
            w->unknown30[14] = 0xff;
            RoomInfo *room = which_room(tile);
            if (room > 0) {
                insert_person_in_room_list(room, w);
                w->kind = get_state((u16)room->type, w);
                if (w->kind != 0x36) {
                    if (w->subtype != 2 && (room->type != 0x16 || w->subtype != 1 || (w->unknown124 & 0x80)) && room->type != 0x1c) {
                        change_lockholder(w, room);
                    }
                } else if (w->subtype != 2) {
                    if (w->owner == player) {
                        D_00158329 = 1;
                        D_0015832a = w->subtype;
                        D_0015832b = room->type;
                    }
                }
            } else {
                w->kind = get_state(0, w);
            }
            w->unknown103 = 0;
            w->icon = 0;
            calculate_speed(w);
            insert_node(w, w->x, w->y);
            if (allhosps[w->owner].f11f4 == 0) {
                w->f8f = 0;
                w->flags32 = 0;
                w->f8e = 0;
                w->unknown128 = 0;
                fn_000b05f0(w);
                w->unknown148d = 0;
                w->unknown130 = actual_game_turn;
                inc_staff_stats(w, w->owner);
                set_expertise_staff(w);
                insert_skill(w->unknown124, w->owner);
                gamevar.f4--;
                gamevar.f6[w->subtype]--;
            }
            if (w->subtype == 0) {
                reinit_nurse(w);
            } else if (w->subtype == 2) {
                w->unknown150 = 0;
                w->unknown148w = w->unknown150;
                pager_arrival(w);
            }
            if (w->room) {
                w->room->unknown29 = 1;
            }
            if (D_001553c7 == player) {
                gamevar.unknown75 = 0;
            }
            allhosps[w->owner].f11f4 = 0;
            allhosps[w->owner].f11f6 = 0;
            return 1;
        }
    }
    return 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x0002a094
void inc_staff_stats(Thing *w, u8 owner)
{
    int kind;

    if (w->subtype == 4) {
        kind = 1;
    } else {
        kind = w->subtype;
    }
    allhosps[owner].fc9e[kind]++;
    if (w->unknown124 & 0x20) {
        allhosps[owner].fca6++;
    }
    allhosps[owner].fc9c++;
}

// NONMATCHING: register choice
// FUNCTION: 0x0002a130
void remove_staff_in_struct(Thing *w)
{
    gamevar.f6[w->subtype]--;
    gamevar.f4--;
    memset(w, 0, 0xaf);
}

// NONMATCHING: register choice
// FUNCTION: 0x0002a178
s16 place_staff_in_struct(u16 kind, u16 flags)
{
    Thing *w;
    u32 mean;
    u32 spread;

    w = allocate_thing(0x19);
    if (w != 0) {
        w->subtype = kind;
        w->renderer = 4;
        w->unknown17 = 0;
        w->owner = 0x63;
        if (staff[kind].unknown6[5]) {
            w->unknown30[4] = staffrand() % staff[kind].unknown6[5] + 1;
        } else {
            w->unknown30[4] = 0;
        }
        w->unknown30[5] = staffrand() & 3;
        w->unknown30[6] = staffrand() & 3;
        w->unknown30[7] = staffrand() & 3;
        w->unknown30[8] = staffrand() & 3;
        if (kind == 0 || kind == 3) {
            w->unknown30[4] = 1;
        }
        w->unknown148d = actual_game_turn + 2500 + staffrand() % 2500;
        w->f8d = normal_staffrand(1, 10);
        w->unknown81 = 100;
        w->unknown83w = 50;
        w->kind = 0x2e;
        gamevar.f4++;
        gamevar.f6[w->subtype]++;
        switch (w->subtype) {
        case 0:
            mean = 30;
            spread = 1000;
            w->unknown124 = 4;
            w->f90 = 1;
            w->f87 = 12;
            w->f88 = staffrand() % 5 + 10;
            w->f89 = staffrand() % 5 + 10;
            break;
        case 1:
            mean = D_0012ddd6;
            spread = D_0012ddd8;
            w->unknown124 |= 0x10;
            if (D_001553af > 0 && staffrand() % D_001553af == 0) {
                w->unknown124 |= 0x40;
            }
            if (D_001553b0 > 0 && staffrand() % D_001553b0 == 0) {
                w->unknown124 |= 0x20;
            }
            if (D_001553b1 > 0 && staffrand() % D_001553b1 == 0) {
                w->unknown124 |= 0x100;
            }
            if (D_001553b2 > 0 && staffrand() % D_001553b2 == 0) {
                w->unknown124 |= 0x80;
                mean = D_0012ddd8;
                spread = 1000;
            } else if (D_001553b3 > 0 && staffrand() % D_001553b3 == 0) {
                w->unknown124 |= 8;
                mean = 30;
                spread = D_0012ddd6;
            }
            if (staffrand() % 100 < 53) {
                w->f90 = 1;
            } else {
                w->f90 = 0;
            }
            w->f87 = staffrand() % 5;
            w->f88 = staffrand() % 5;
            w->f89 = staffrand() % 5;
            switch (w->unknown30[4]) {
            case 1:
                break;
            case 2:
                w->f89 += 5;
                w->f88 += 5;
                w->f87 += 5;
                break;
            }
            break;
        case 2:
            mean = 30;
            spread = 1000;
            w->unknown124 = 2;
            w->f90 = 0;
            w->f9e = 0x21;
            w->f9f = 0x21;
            w->fa0 = 0x22;
            w->f87 = staffrand() % 3 + 15;
            w->f88 = staffrand() % 3 + 15;
            w->f89 = staffrand() % 3 + 15;
            break;
        case 3:
            mean = 30;
            spread = 1000;
            w->unknown124 = 1;
            w->f90 = 1;
            w->f87 = staffrand() % 4 + 10;
            w->f88 = staffrand() % 5 + 10;
            w->f89 = staffrand() % 5 + 10;
            if (w->f87 == 12) {
                w->f87 = 14;
            }
            break;
        }
        w->unknown124 |= flags;
        w->unknown126 = normal_staffrand(mean, spread);
        appraise_staff(w);
        w->unknown108 = w->f70;
        w->f84 = staffrand() % 26 + 0x41;
        w->f85 = staffrand() % 33;
        w->f86 = staffrand() % 26;
        w->f8a = staffrand() % D_00156429;
        w->f8b = staffrand() % D_0015642a;
        w->f8c = staffrand() % D_0015642b;
        return w - thing;
    }
    return 0;
}

// FUNCTION: 0x0002a804
void sack_staff(Thing *w, u16 mode)
{
    int kind;

    remove_person_from_room_list(w);
    set_icon_value(w, 9);
    if (w->owner == 0x63) {
        return;
    }
    interrupt_staff(w);
    switch (mode) {
    case 0:
        change_balance(0x18, -w->unknown108, 0, w->owner);
        break;
    case 1:
        w->f8e &= 0xdf;
        break;
    }
    if (mode != 2) {
        staff_exit_room_check(w);
        if (w->room) {
            set_complex(w, 0x136, w->room->unknown6);
        }
        set_complex(w, 0xbe, get_room_info(1, 0x65, w->owner)->unknown4);
        w->state_b = 0x38;
        w->unknown94 = 0;
        fn_0003e724(w);
    }
    remove_expertise_staff(w);
    w->f8d += w->f8d / 4;
    w->unknown81 = 100;
    w->f8f = 0;
    w->flags32 = 0;
    w->f8e = 0;
    w->unknown128 = 0;
    fn_000b05f0(w);
    allhosps[w->owner].fc9c--;
    if (w->subtype == 4) {
        kind = 1;
    } else {
        kind = w->subtype;
    }
    allhosps[w->owner].fc9e[kind]--;
    if (w->unknown124 & 0x20) {
        allhosps[w->owner].fca6--;
    }
    w->owner = 0x63;
    if (mode == 2) {
        if (w->room) {
            fn_000168a0(w);
        }
        remove_thing(w);
    }
}

// FUNCTION: 0x0002aa54
s16 staff_waits(Thing *w)
{
    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
    }
    if (--w->height <= 0) {
        w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
        w->unknown17 = old_newrand() & 3;
        {
            u16 idx;

            idx = check_frame(staff_anim_starts[w->subtype].anims[staff[w->subtype].anim], w) + w->unknown17;
            w->sprite = start[idx].frame;
            w->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
        }
    }
    return 0;
}
