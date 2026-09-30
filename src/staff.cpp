#include "hosp.h"

extern "C" {
void fn_000b05f0(Thing *w);
void fn_000168a0(Thing *w);
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
void rest_staff(Thing *w, s16 amount);
u8 send_doctor_to_research(Thing *w);
u8 send_doctor_to_training(Thing *w);
u8 nurse_check_for_work(Thing *w);
u16 check_pager(Thing *w);
extern "C" u8 fn_0002b400(Thing *w);
extern "C" u8 fn_0002d11c(Thing *o);
void private_interrupt_staff(Thing *w);
u16 thingpage_remind_me(ThingPager *p, Thing *w);
void thingpage_cancel_my_response(ThingPager *p, Thing *w);
void staff_abandon_pager_response(Thing *w);
void receptionist_reset(Thing *w);
void remove_node(Thing *w);
extern "C" {
void fn_00015b68(Thing *w);
void fn_000168a0(Thing *w);
}
void change_surgeon_to_doc(Thing *w);
extern s16 staff_states[][5];
extern s16 mcurx;
extern s16 mcury;
extern "C" {
extern s16 D_001567b0[8];
}

// FUNCTION: 0x00029ad0
void create_job_market(void)
{
    u32 i;
    u32 n;
    u32 j;

    for (i = 0; i < 30; i++) {
        if (staff_levels[i].month == othervar.f9d1) {
            gamevar.f0 = staff_levels[i].fund;
            gamevar.f10[0] = staff_levels[i].counts[0];
            gamevar.f10[1] = staff_levels[i].counts[1];
            gamevar.f10[2] = staff_levels[i].counts[2];
            gamevar.f10[3] = staff_levels[i].counts[3];
            gamevar.f16f = staff_levels[i].f0a[0];
            gamevar.f170 = staff_levels[i].f0a[1];
            gamevar.f171 = staff_levels[i].f0a[2];
            gamevar.f172 = staff_levels[i].f0a[3];
            gamevar.f173 = staff_levels[i].f0a[4];
            break;
        }
        if (staff_levels[i].month > othervar.f9d1) {
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
                        advisor.f12d = 1;
                        advisor.f12e = w->subtype;
                        advisor.f12f = room->type;
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
            }
            pager_arrival(w);
            if (w->room) {
                w->room->unknown29 = 1;
            }
            if (othervar.ff == player) {
                gamevar.unknown75 = 0;
            }
            allhosps[w->owner].f11f4 = 0;
            allhosps[w->owner].f11f6 = 0;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002a094
void inc_staff_stats(Thing *w, u8 owner)
{
    allhosps[owner].fc9e[w->subtype == 4 ? 1 : w->subtype]++;
    if (w->unknown124 & 0x20) {
        allhosps[owner].fca6++;
    }
    allhosps[owner].fc9c++;
}

// FUNCTION: 0x0002a130
void remove_staff_in_struct(Thing *w)
{
    gamevar.f6[w->subtype]--;
    gamevar.f4--;
    memset(w, 0, 0xaf);
}

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
            mean = gbv.f156;
            spread = gbv.f158;
            w->unknown124 |= 0x10;
            if (gamevar.f16f > 0 && staffrand() % gamevar.f16f == 0) {
                w->unknown124 |= 0x40;
            }
            if (gamevar.f170 > 0 && staffrand() % gamevar.f170 == 0) {
                w->unknown124 |= 0x20;
            }
            if (gamevar.f171 > 0 && staffrand() % gamevar.f171 == 0) {
                w->unknown124 |= 0x100;
            }
            if (gamevar.f172 > 0 && staffrand() % gamevar.f172 == 0) {
                w->unknown124 |= 0x80;
                mean = gbv.f158;
                spread = 1000;
            } else if (gamevar.f173 > 0 && staffrand() % gamevar.f173 == 0) {
                w->unknown124 |= 8;
                mean = 30;
                spread = gbv.f156;
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
        w->f8a = staffrand() % statvar.f5d9;
        w->f8b = staffrand() % statvar.f5da;
        w->f8c = staffrand() % statvar.f5db;
        return w - thing;
    }
    return 0;
}

// FUNCTION: 0x0002a804
void sack_staff(Thing *w, u16 mode)
{
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
    allhosps[w->owner].fc9e[w->subtype == 4 ? 1 : w->subtype]--;
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

// FUNCTION: 0x0002abb0
void staff_in_corridor(Thing *w)
{
    u16 anim;

    w->unknown19 = w->x.tile;
    w->unknown20 = w->y.tile;
    w->kind = 0x34;
    w->unknown103 = 6;
    w->height = 1;
    anim = check_frame(staff_anim_starts[w->subtype].unknownA, w) + (u16)w->unknown17;
    w->sprite = start[anim].frame;
    w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
}

// FUNCTION: 0x0002ac70
u8 obey_pager(Thing *w)
{
    RoomInfo *room;

    room = &room_info[check_pager(w)];
    if (room <= room_info || room == w->room) {
        return 0;
    }
    staff_exit_room_check(w);
    w->state_b = (u8)get_state((u16)room->type, w);
    w->unknown103 = 0;
    if (w->room) {
        set_complex(w, 0x136, w->room->unknown6);
        fn_0003e724(w);
        w->state_a = 0x36;
        set_complex(w, 0x14a, room->unknown4);
    } else {
        w->state_a = 0x36;
        set_complex(w, 0x14a, room->unknown4);
        fn_0003e724(w);
    }
    w->unknown89 = room - room_info;
    return 1;
}

// FUNCTION: 0x0002ad8c
u8 process_staff_complex(Thing *w)
{
    RoomInfo *room;

    room = w->room;
    if (room > room_info) {
        staff_exit_room_check(w);
        set_complex(w, 0x136, room->unknown6);
        w->state_b = 0x28;
        fn_0003e724(w);
    } else {
        w->kind = 0x28;
    }
    return 1;
}

// FUNCTION: 0x0002adfc
u8 take_a_break(Thing *w)
{
    RoomInfo *room;

    room = get_nearest_room_info(w, 0x19);
    if (room <= room_info) {
        return 0;
    }
    if (w->subtype == 3) {
        interrupt_staff(w);
    }
    staff_exit_room_check(w);
    w->state_b = 0x30;
    if (w->room > room_info) {
        set_complex(w, 0x136, w->room->unknown6);
        fn_0003e724(w);
        w->state_a = 0x36;
        set_complex(w, 0x96, room->unknown4);
    } else {
        set_complex(w, 0x96, room->unknown4);
        w->state_a = 0x36;
        fn_0003e724(w);
    }
    w->unknown89 = room - room_info;
    return 1;
}

// FUNCTION: 0x0002af04
void staff_in_staff_room(Thing *w)
{
    u16 r;
    CompFrame *f;

    if (actual_game_turn % w->anim_speed == 0) {
        w->sprite = frame[w->sprite].next;
        f = &frame[w->sprite];
        if (f->flags & 1) {
            if (--w->height <= 0) {
                if (w->icon == 0x2da) {
                    rest_staff(w, gbv.fdc);
                } else {
                    rest_staff(w, gbv.fda);
                }
                w->f8e &= ~1;
                if (w->f8e & 8) {
                    w->f8e &= 0xf0;
                    process_staff_complex(w);
                    return;
                }
                if (w->room->f22 || w->room->f3 == 4) {
                    process_staff_complex(w);
                    return;
                }
                r = old_newrand() % 10;
                if ((find_object(w->x.tile + (w->y.tile << 7), 10, 1, 0x96, w->owner) && w->icon != 0x2ee) && r == 1) {
                    if (((u8 (*)[2])&abilities[15][2])[w->subtype][1]) {
                        w->state_a = 0x30;
                        w->state_b = 0x30;
                        set_complex(w, 0x2ee, 0);
                        fn_0003e724(w);
                        return;
                    }
                }
                if ((find_object((u16)w->x.tile + ((u16)w->y.tile << 7), 0x39, 1, 0x96, w->owner) && w->icon != 0x2f8) && r == 2) {
                    if (((u8 (*)[2])&abilities[15][2])[w->subtype][0]) {
                        w->state_a = 0x30;
                        w->state_b = 0x30;
                        set_complex(w, 0x2f8, 0);
                        fn_0003e724(w);
                    }
                    return;
                }
                if (w->icon != 0x2da) {
                    if (find_object((u16)w->x.tile + ((u16)w->y.tile << 7), 0x13, 1, 0x96, w->owner)) {
                    w->state_a = 0x30;
                    w->state_b = 0x30;
                    set_complex(w, 0x2da, 0);
                    fn_0003e724(w);
                    return;
                    }
                }
                if (overlapping_staff_check(w, 0x30)) {
                    return;
                }
                w->unknown18 = 0x20;
                if (w->f78 <= 0) {
                    w->unknown128 = (s16)w->unknown128 / 2;
                    if (w->subtype == 1 && w->unknown148[2]) {
                        if (w->f78 <= 0) {
                            if (w->unknown30[4] == 3) {
                                w->unknown30[4] = 1;
                            } else if (w->unknown30[4] == 4) {
                                w->unknown30[4] = 2;
                            }
                            w->unknown148[2] = 0;
                        }
                    } else if (w->subtype == 1) {
                        if ((w->f8e & 0x10) && send_doctor_to_research(w)) {
                            return;
                        }
                        if ((w->f8e & 0x40) && send_doctor_to_training(w)) {
                            return;
                        }
                        if (!obey_pager(w)) {
                            set_complex(w, 0x136, w->room->unknown6);
                            w->state_b = 0x28;
                            fn_0003e724(w);
                        }
                    } else if (w->subtype == 0) {
                        set_complex(w, 0x136, w->room->unknown6);
                        if (!nurse_check_for_work(w)) {
                            w->state_b = 0x28;
                            fn_0003e724(w);
                        }
                    } else if (w->subtype == 2) {
                        w->kind = 0x36;
                        return;
                    } else if (w->subtype == 3) {
                        w->kind = 0x36;
                        return;
                    }
                    calculate_speed(w);
                }
                w->height = w->unknown18 == 0 ? div_err() : 0x100 / w->unknown18;
            }
        }
    }
}

// FUNCTION: 0x0002b400
u8 fn_0002b400(Thing *w)
{
    RoomInfo *room;
    u16 *staff;

    room = w->room;
    if (room <= room_info) {
        return 1;
    }
    staff = room->staff;
    if ((u16)(w - thing) == staff[0] || (u16)(w - thing) == staff[1]) {
        return 1;
    }
    if (w->room->type == 10) {
        if (!staff[0]) {
            staff[0] = w - thing;
            return 1;
        }
        if (!staff[1]) {
            staff[1] = w - thing;
            return 1;
        }
        return 0;
    }
    if (staff[0]) {
        return 0;
    }
    staff[0] = w - thing;
    return 1;
}

// FUNCTION: 0x0002b528
u8 am_i_lead_staff(Thing *w)
{
    u16 *staff;

    if (w->room) {
        staff = w->room->staff;
        if (w->room->type == 10) {
            if (staff[0] == 0 || staff[1] == 0) {
                return 0;
            }
        }
        return (u16)(w - thing) == staff[0] ? (u8)1 : (u8)0;
    }
    return 0;
}

// FUNCTION: 0x0002b5cc
void staff_nav_failure(Thing *w)
{
    RoomInfo *room;

    if (w->room == 0) {
        room = which_room((u16)w->unknown19 + ((u16)w->unknown20 << 7));
        if (room && actual_game_turn - 500 < room->unknown22) {
            process_staff_complex(w);
            return;
        }
    }
    w->unknown105 = w->kind;
    w->unknown106 = w->unknown19;
    w->unknown107 = w->unknown20;
    w->kind = 0x4e;
    w->height = 0;
    if (w->owner == player) {
        announce_this(1, 1);
    }
}

// FUNCTION: 0x0002b690
void draw_staff_cursor(void)
{
    u8 bad;
    s16 state;
    RoomInfo *room;
    Thing *w;
    u8 *remap;

    if (othervar.f28 != othervar.f2a || gamevar.unknown77 != gamevar.unknown79) {
        room = which_room(othervar.f28);
        if (room > 0) {
            state = staff_states[rooms[room->type].unknown2[1]][gamevar.unknown77];
            bad = 0;
            if (check_points_connected(othervar.f28, room->unknown6) == 0) {
                bad = 1;
            }
            if ((room->f3 != 0 && room->f3 != 1) && gamevar.unknown77 != 2) {
                bad = 1;
            }
        } else {
            state = staff_states[rooms[0].unknown2[1]][gamevar.unknown77];
            bad = !check_points_connected(othervar.f28, room_info[allhosps[player].unknown1cf[0]].unknown4) ? (u8)1 : (u8)0;
        }
        if (state == -1 || bad == 1) {
            gamevar.f72 = 0;
        } else {
            gamevar.f72 = 1;
        }
        gamevar.f73 = staff_anim_starts[gamevar.unknown77].anims[staff[gamevar.unknown77].anim] + 1;
        gamevar.unknown79 = gamevar.unknown77;
        if (map[othervar.f28].unknown7 != 1 << player) {
            gamevar.f72 = 0;
        }
    }
    if (gamevar.f72) {
        remap = 0;
    } else {
        remap = bw_remap;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    w = &thing[gamevar.unknown75];
    if (w > thing) {
        D_001567b0[0] = D_001567b0[1] = 999;
        D_001567b0[2] = D_001567b0[3] = 0;
        draw_from_special_id_frame(start[gamevar.f73].frame, mcurx + resvars.f4 / 2, mcury - resvars.f3 / 2, remap, w->layers, start[gamevar.f73].flags);
    }
}

// FUNCTION: 0x0002b998
s32 pay_staff_bonus(Thing *w, u16 amount, u16 percent, u16 morale, u8 owner)
{
    s32 total;
    Thing *e;

    total = 0;
    if (w > thing) {
        if (percent) {
            total = w->unknown108 * percent / 100;
        } else {
            total = amount;
        }
        if (morale) {
            w->unknown81 += 50;
            if ((s16)w->unknown81 < 0) {
                w->unknown81 = 0;
            } else if ((s16)w->unknown81 > 100) {
                w->unknown81 = 100;
            }
            change_balance(0x1b, -amount, w - thing, owner);
        }
    } else {
        for (e = thing + 1; e < end_things; e++) {
            if (e->renderer == 4 && e->owner == owner) {
                if (percent) {
                    total += percent * e->unknown108 / 100;
                } else {
                    total += amount;
                }
                if (morale) {
                    e->unknown81 += 50;
                    if ((s16)e->unknown81 < 0) {
                        e->unknown81 = 0;
                    } else if ((s16)e->unknown81 > 100) {
                        e->unknown81 = 100;
                    }
                }
            }
        }
        if (morale) {
            change_balance(0x19, -total, 0, owner);
        }
    }
    return total;
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

// FUNCTION: 0x0002bbdc
void staff_exit_room_check(Thing *w)
{
    Thing *obj;

    if (w->room) {
        if (w->subtype == 4) {
            change_surgeon_to_doc(w);
        }
        if (w->room->type == 0x16 && &thing[w->room->staff[0]] == w) {
            obj = &thing[find_object((u16)w->x.tile + ((u16)w->y.tile << 7), 0x25, 0, 0x97, w->owner)];
            if (obj > thing && obj->fuse) {
                w->unknown61 = 0;
                w->unknown63 = 0;
                w->unknown75w = 0;
                w->unknown65 = 0;
                if (&thing[w->unknown69 && (thing[w->unknown69].renderer == 6 || thing[w->unknown69].renderer == 8)
                           && (u16)(w - thing) == thing[w->unknown69].unknown91 ? w->unknown69 : 0] == obj) {
                    set_complex(w, 0x3b6, 0);
                } else {
                    set_complex(w, 0xa0, 0);
                }
                obj->fuse = 0;
            }
        }
        if (w->subtype == 0 && w->room->type == 9) {
            w->room->f3 == 4 ? 0 : (w->room->f3 = 0);
        }
    }
}

// FUNCTION: 0x0002be14
void rest_staff(Thing *w, s16 amount)
{
    if (w->f78 > gbv.fee / 2) {
        if ((amount = amount * gbv.fee / w->f78) < gbv.ff0) {
            amount = gbv.ff0;
        }
    }
    w->f78 -= amount;
    w->unknown81++;
    if ((s16)w->unknown81 < 0) {
        w->unknown81 = 0;
    } else if ((s16)w->unknown81 > 100) {
        w->unknown81 = 100;
    }
}

// FUNCTION: 0x0002becc
void staff_picked_up(Thing *w)
{
    staff_exit_room_check(w);
    abandon_complex(w);
    allhosps[w->owner].f11f6 = w->x.tile + (w->y.tile << 7);
    if (othervar.ff == player) {
        gamevar.unknown77 = w->subtype;
        gamevar.unknown75 = w - thing;
        gamevar.unknown79 = 0xffff;
        set_mouse_mode(3);
    }
    remove_person_from_room_list(w);
    remove_node(w);
    w->kind = 0x44;
}

// FUNCTION: 0x0002bf94
void remove_node(Thing *w)
{
    if (w->room) {
        fn_000168a0(w);
    }
    fn_00015b68(w);
}

// FUNCTION: 0x0002bfc8
u32 shall_i_resign(Thing *w)
{
    u32 limit;

    if (w->f8e & 0x20) {
        set_icon_value(w, 9);
        if (w->owner == player) {
            advisor.f128 |= 1;
        }
        if (actual_game_turn >= w->flags32) {
            get_playercall(1, w->unknown91, w->owner);
            sack_staff(w, 1);
            w->unknown91 = 0;
            return 1;
        }
    } else {
        limit = ((old_newrand() & 3) + 6) * 1500;
        if (actual_game_turn - (u16)w->unknown130 > limit) {
            if ((s16)w->unknown128 > gbv.f10a) {
        w->f8e |= 0x20;
        w->f8f++;
        w->flags32 = actual_game_turn + 1500;
        w->unknown91 = make_a_playercall(1, w - thing, 0, w->owner);
                return 1;
            }
        }
        if (w->f93) {
            if (w->unknown128s > gbv.f10a / 4) {
            w->f8e |= 0x20;
            w->flags32 = actual_game_turn + 1500;
            w->unknown91 = make_a_playercall(1, w - thing, 0, w->owner);
                return 1;
            }
            w->f93 = 0;
        }
    }
    return 0;
}

// FUNCTION: 0x0002c1cc
u16 count_emergency_staff(u8 owner)
{
    u32 count;
    Thing *w;

    count = 0;
    for (w = thing + 1; w < end_things; w++) {
        if ((w->renderer == 4 && w->subtype == 1) && w->owner == owner) {
            if (w->f78 < gbv.fe8) {
                count++;
            }
        }
    }
    return count;
}

// FUNCTION: 0x0002c268
void calculate_speed(Thing *w)
{
    u32 speed;

    speed = 3;
    if (w->x.frac == 0x80 && w->y.frac == 0x80) {
        if (w->f78 > gbv.fec) {
            speed = 1;
        } else if (w->f78 > gbv.fea) {
            speed = 2;
        }
        if (w->unknown126 >= gbv.f158) {
            speed += 30;
        } else if (w->unknown126 > gbv.f156) {
            speed += 20;
        } else {
            speed += 10;
        }
        if (w->room) {
            if (w->room->type == 0x16) {
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
            w->unknown18 = 0x10;
            w->anim_speed = 2;
            break;
        case 23:
        case 32:
            w->unknown18 = 0x20;
            w->anim_speed = 1;
            break;
        case 33:
            w->unknown18 = 0x40;
            w->anim_speed = 1;
            break;
        }
    }
}

// FUNCTION: 0x0002c3bc
void interrupt_staff(Thing *w)
{
    Thing *e;
    u16 *staff;

    private_interrupt_staff(w);
    if (w->subtype == 4) {
        if (w->room) {
            staff = w->room->staff;
            e = &thing[staff[0]];
            if (e <= thing || e == w) {
                e = &thing[staff[1]];
                if (e <= thing || e == w) {
                    return;
                }
            }
            if (e->owner != 0x63) {
                private_interrupt_staff(e);
            }
        }
    }
}

// FUNCTION: 0x0002c484
void private_interrupt_staff(Thing *w)
{
    Thing *patient;
    Thing *e;
    RoomInfo *room;

    patient = &thing[w->unknown73];
    if (w->subtype == 2) {
        e = &thing[thingpage_remind_me(&allhosps[w->owner].ff6d, w)];
        if (e > thing) {
            room = &room_info[e->unknown89];
            if (room > room_info) {
                if (room->f3 == 5 || room->f3 == 6) {
                    room->f3 = 0;
                }
            }
        }
        room = w->room;
        if ((room && (e = &thing[w->unknown69]) > thing) && objects[e->subtype].unknown24) {
            if (room->f3 == 5 || room->f3 == 6) {
                room->f3 = 0;
            }
        }
        thingpage_cancel_my_response(&allhosps[w->owner].ff6d, w);
        thingpage_cancel_my_response(&allhosps[w->owner].fd2b, w);
    }
    interrupt_complex(w);
    if ((w->subtype == 0 && patient > thing) && w->room == 0) {
        patient->flags &= ~2;
        w->unknown73 = 0;
    } else if ((patient > thing && patient->renderer == 2) && (w->room != 0 || w->subtype == 3)) {
        w->unknown73 = 0;
        interrupt_person(patient);
    }
    if (w->room) {
        w->kind = get_state((u16)w->room->type, w);
    } else {
        w->kind = get_state(0, w);
    }
    w->unknown103 = 0;
    staff_abandon_pager_response(w);
    if (w->subtype == 3) {
        receptionist_reset(w);
    }
}

// FUNCTION: 0x0002c700
void staff_join_new_employer(u16 id, u16 owner)
{
    Thing *w;

    w = &thing[id];
    if (w > thing) {
        interrupt_staff(w);
        if (w->owner == player) {
            announce_this(0xb, 1);
        }
        staff_exit_room_check(w);
        if (w->room) {
            set_complex(w, 0x136, w->room->unknown6);
        }
        set_complex(w, 0xbe, get_room_info(4, 0x65, owner)->unknown4);
        w->state_b = 0x28;
        fn_0003e724(w);
        remove_expertise_staff(w);
        w->f8d += w->f8d / 4;
        w->unknown81 = 100;
        w->f8f = 0;
        w->unknown130 = actual_game_turn;
        w->flags32 = 0;
        w->f8e = 0;
        w->unknown128 = 0;
        fn_000b05f0(w);
        allhosps[w->owner].fc9c--;
        allhosps[owner].fc9c++;
        allhosps[w->owner].fc9e[w->subtype == 4 ? 1 : w->subtype]--;
        allhosps[owner].fc9e[w->subtype == 4 ? 1 : w->subtype]++;
        if (w->unknown124 & 0x20) {
            allhosps[w->owner].fca6--;
            allhosps[owner].fca6++;
        }
        w->owner = owner;
        set_expertise_staff(w);
        insert_skill(w->unknown124, w->owner);
        w->unknown128 = 0;
        w->unknown108 *= 2;
        if (!listhead[owner].rooms[4]) {
            sack_staff(w, 0);
        }
    }
}

// FUNCTION: 0x0002c9cc
void change_lockholder(Thing *w, RoomInfo *room)
{
    Thing *e;
    u16 *slot;

    slot = 0;
    if (room->staff[0]) {
        slot = &room->staff[0];
    } else if (room->staff[1]) {
        slot = &room->staff[1];
    }
    if (slot) {
        if (room->type != 10 || room->staff[0] && room->staff[1]) {
            e = &thing[*slot];
            interrupt_staff(e);
            process_staff_complex(e);
        }
        *slot = w - thing;
    }
}

// FUNCTION: 0x0002ca88
s16 get_state(const u16 type, const Thing *w)
{
    s16 state;

    state = staff_states[rooms[type].unknown2[1]][w->subtype];
    if (w->subtype == 1 && rooms[type].unknown8) {
        if ((w->unknown124 & rooms[type].unknown8) == 0) {
            state = 0x36;
        }
    }
    return state;
}

// FUNCTION: 0x0002cb2c
void increase_staff_salaries(Thing *w, u16 amount, u16 percent, u8 owner)
{
    s32 total;
    Thing *e;

    total = 0;
    if (w > thing) {
        if (percent) {
            w->unknown108 += percent * w->unknown108 / 100;
        } else {
            w->unknown108 += amount;
        }
        if (w->unknown108 > 90000) {
            w->unknown108 = 90000;
        }
        w->unknown128 = 0;
        w->unknown81 += 50;
        if ((s16)w->unknown81 < 0) {
            w->unknown81 = 0;
        } else if ((s16)w->unknown81 > 100) {
            w->unknown81 = 100;
        }
    } else {
        for (e = thing + 1; e < end_things; e++) {
            if (e->renderer == 4 && e->owner == owner) {
                if (percent) {
                    e->unknown108 += percent * e->unknown108 / 100;
                } else {
                    e->unknown108 += amount;
                }
                if (w->unknown108 > 90000) {
                    w->unknown108 = 90000;
                }
                e->unknown128 = 0;
                e->unknown81 += 50;
                if ((s16)e->unknown81 < 0) {
                    e->unknown81 = 0;
                } else if ((s16)e->unknown81 > 100) {
                    e->unknown81 = 100;
                }
            }
        }
    }
}

// FUNCTION: 0x0002cccc
void training_builder(u8 level)
{
    u32 count;
    RatFestLevel *lvl;
    Map *m;
    s16 id;

    count = 0;
    lvl = &rat_fest_level[level];
    seed = safeseed = lvl->seed;
    for (m = map; m < end_map; m++) {
        if (m->unknown7 == 1 << player) {
            if (lvl->staff_max > count && saferand() % lvl->staff_chance == 0) {
                gamevar.f172 = 1;
                gamevar.f16f = 1;
                gamevar.f170 = 0;
                gamevar.f171 = 0;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(m - map, id, 0, player);
                }
                gamevar.f16f = 0;
                gamevar.f170 = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(m - map, id, 0, player);
                }
                gamevar.f170 = 0;
                gamevar.f171 = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(m - map, id, 0, player);
                }
                break;
            }
        }
    }
}

// FUNCTION: 0x0002ce9c
u8 is_staff_stepping_on_toes(Thing *w)
{
    Thing *e;

    for (e = &thing[w->next]; e > thing; e = &thing[e->next]) {
        if (e->renderer == 4) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002cf14
u8 overlapping_staff_check(Thing *w, s32 state)
{
    if (actual_game_turn % 47 == 0 || ((s16)w->y.tile << 7) + (s16)w->x.tile == w->room->unknown6
        || is_staff_stepping_on_toes(w)
        || (w->unknown69 && (thing[w->unknown69].renderer == 6 || thing[w->unknown69].renderer == 8)
                   && (u16)(w - thing) == thing[w->unknown69].unknown91 ? w->unknown69 : 0) == 0
            && (map[(s16)w->x.tile + ((s16)w->y.tile << 7)].flags & 7)) {
        RoomInfo *room;
        u16 tile;
        u16 dest;

        room = w->room;
        tile = w->x.tile + (w->y.tile << 7);
        dest = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
        if (tile != dest && can_i_get_to(w, dest)) {
            w->state_a = state;
            w->state_b = w->state_a;
            set_complex(w, 0xbe, dest);
            fn_0003e724(w);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002d11c
u8 fn_0002d11c(Thing *o)
{
    Thing *e;

    if (o->subtype == 4) {
        e = &thing[find_object(o->x.tile + (o->y.tile << 7), 0x23, 0, 0x97, o->owner)];
        if (e > thing) {
            if (e->unknown98 != 0) {
                return 0;
            }
        }
    }
    return 1;
}
