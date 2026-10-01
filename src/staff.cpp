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
    u32 shortfall;
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
            shortfall = gamevar.f10[i] - gamevar.f6[i];
            for (j = 0; j < shortfall; j++) {
                place_staff_in_struct(i, 0);
            }
        }
    }
}

// FUNCTION: 0x00029c4c
u16 fn_00029c4c(u16 tile, u16 id, u8 flag, u8 owner)
{
    Thing *new_staff;

    new_staff = &thing[id];
    if (new_staff > thing) {
        if (flag || map[tile].unknown7 == (1 << owner)) {
            new_staff->owner = owner;
            if (new_staff->unknown17 > 3) {
                new_staff->unknown17 = 0;
            }
            {
                u16 idx;

                idx = check_frame(staff_anim_starts[new_staff->subtype].anims[staff[new_staff->subtype].anim], new_staff) + new_staff->unknown17;
                new_staff->sprite = start[idx].frame;
                new_staff->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
            }
            new_staff->x.tile = tile % 128;
            new_staff->y.tile = tile / 128;
            new_staff->x.frac = 0x80;
            new_staff->y.frac = 0x80;
            new_staff->unknown30[14] = 0xff;
            RoomInfo *room = which_room(tile);
            if (room > 0) {
                insert_person_in_room_list(room, new_staff);
                new_staff->kind = get_state((u16)room->type, new_staff);
                if (new_staff->kind != 0x36) {
                    if (new_staff->subtype != 2 && (room->type != 0x16 || new_staff->subtype != 1 || (new_staff->unknown124 & 0x80)) && room->type != 0x1c) {
                        change_lockholder(new_staff, room);
                    }
                } else if (new_staff->subtype != 2) {
                    if (new_staff->owner == player) {
                        advisor.f12d = 1;
                        advisor.f12e = new_staff->subtype;
                        advisor.f12f = room->type;
                    }
                }
            } else {
                new_staff->kind = get_state(0, new_staff);
            }
            new_staff->unknown103 = 0;
            new_staff->icon = 0;
            calculate_speed(new_staff);
            insert_node(new_staff, new_staff->x, new_staff->y);
            if (allhosps[new_staff->owner].f11f4 == 0) {
                new_staff->f8f = 0;
                new_staff->flags32 = 0;
                new_staff->f8e = 0;
                new_staff->unknown128 = 0;
                fn_000b05f0(new_staff);
                new_staff->unknown148d = 0;
                new_staff->unknown130 = actual_game_turn;
                inc_staff_stats(new_staff, new_staff->owner);
                set_expertise_staff(new_staff);
                insert_skill(new_staff->unknown124, new_staff->owner);
                gamevar.f4--;
                gamevar.f6[new_staff->subtype]--;
            }
            if (new_staff->subtype == 0) {
                reinit_nurse(new_staff);
            } else if (new_staff->subtype == 2) {
                new_staff->unknown150 = 0;
                new_staff->unknown148w = new_staff->unknown150;
            }
            pager_arrival(new_staff);
            if (new_staff->room) {
                new_staff->room->unknown29 = 1;
            }
            if (othervar.ff == player) {
                gamevar.unknown75 = 0;
            }
            allhosps[new_staff->owner].f11f4 = 0;
            allhosps[new_staff->owner].f11f6 = 0;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002a094
void inc_staff_stats(Thing *staff, u8 owner)
{
    allhosps[owner].fc9e[staff->subtype == 4 ? 1 : staff->subtype]++;
    if (staff->unknown124 & 0x20) {
        allhosps[owner].fca6++;
    }
    allhosps[owner].fc9c++;
}

// FUNCTION: 0x0002a130
void remove_staff_in_struct(Thing *staff)
{
    gamevar.f6[staff->subtype]--;
    gamevar.f4--;
    memset(staff, 0, 0xaf);
}

// FUNCTION: 0x0002a178
s16 place_staff_in_struct(u16 kind, u16 flags)
{
    Thing *new_staff;
    u32 mean;
    u32 spread;

    new_staff = allocate_thing(0x19);
    if (new_staff != 0) {
        new_staff->subtype = kind;
        new_staff->renderer = 4;
        new_staff->unknown17 = 0;
        new_staff->owner = 0x63;
        if (staff[kind].unknown6[5]) {
            new_staff->unknown30[4] = staffrand() % staff[kind].unknown6[5] + 1;
        } else {
            new_staff->unknown30[4] = 0;
        }
        new_staff->unknown30[5] = staffrand() & 3;
        new_staff->unknown30[6] = staffrand() & 3;
        new_staff->unknown30[7] = staffrand() & 3;
        new_staff->unknown30[8] = staffrand() & 3;
        if (kind == 0 || kind == 3) {
            new_staff->unknown30[4] = 1;
        }
        new_staff->unknown148d = actual_game_turn + 2500 + staffrand() % 2500;
        new_staff->f8d = normal_staffrand(1, 10);
        new_staff->unknown81 = 100;
        new_staff->unknown83w = 50;
        new_staff->kind = 0x2e;
        gamevar.f4++;
        gamevar.f6[new_staff->subtype]++;
        switch (new_staff->subtype) {
        case 0:
            mean = 30;
            spread = 1000;
            new_staff->unknown124 = 4;
            new_staff->f90 = 1;
            new_staff->f87 = 12;
            new_staff->f88 = staffrand() % 5 + 10;
            new_staff->f89 = staffrand() % 5 + 10;
            break;
        case 1:
            mean = gbv.f156;
            spread = gbv.f158;
            new_staff->unknown124 |= 0x10;
            if (gamevar.f16f > 0 && staffrand() % gamevar.f16f == 0) {
                new_staff->unknown124 |= 0x40;
            }
            if (gamevar.f170 > 0 && staffrand() % gamevar.f170 == 0) {
                new_staff->unknown124 |= 0x20;
            }
            if (gamevar.f171 > 0 && staffrand() % gamevar.f171 == 0) {
                new_staff->unknown124 |= 0x100;
            }
            if (gamevar.f172 > 0 && staffrand() % gamevar.f172 == 0) {
                new_staff->unknown124 |= 0x80;
                mean = gbv.f158;
                spread = 1000;
            } else if (gamevar.f173 > 0 && staffrand() % gamevar.f173 == 0) {
                new_staff->unknown124 |= 8;
                mean = 30;
                spread = gbv.f156;
            }
            if (staffrand() % 100 < 53) {
                new_staff->f90 = 1;
            } else {
                new_staff->f90 = 0;
            }
            new_staff->f87 = staffrand() % 5;
            new_staff->f88 = staffrand() % 5;
            new_staff->f89 = staffrand() % 5;
            switch (new_staff->unknown30[4]) {
            case 1:
                break;
            case 2:
                new_staff->f89 += 5;
                new_staff->f88 += 5;
                new_staff->f87 += 5;
                break;
            }
            break;
        case 2:
            mean = 30;
            spread = 1000;
            new_staff->unknown124 = 2;
            new_staff->f90 = 0;
            new_staff->f9e = 0x21;
            new_staff->f9f = 0x21;
            new_staff->fa0 = 0x22;
            new_staff->f87 = staffrand() % 3 + 15;
            new_staff->f88 = staffrand() % 3 + 15;
            new_staff->f89 = staffrand() % 3 + 15;
            break;
        case 3:
            mean = 30;
            spread = 1000;
            new_staff->unknown124 = 1;
            new_staff->f90 = 1;
            new_staff->f87 = staffrand() % 4 + 10;
            new_staff->f88 = staffrand() % 5 + 10;
            new_staff->f89 = staffrand() % 5 + 10;
            if (new_staff->f87 == 12) {
                new_staff->f87 = 14;
            }
            break;
        }
        new_staff->unknown124 |= flags;
        new_staff->unknown126 = normal_staffrand(mean, spread);
        appraise_staff(new_staff);
        new_staff->unknown108 = new_staff->f70;
        new_staff->f84 = staffrand() % 26 + 0x41;
        new_staff->f85 = staffrand() % 33;
        new_staff->f86 = staffrand() % 26;
        new_staff->f8a = staffrand() % statvar.f5d9;
        new_staff->f8b = staffrand() % statvar.f5da;
        new_staff->f8c = staffrand() % statvar.f5db;
        return new_staff - thing;
    }
    return 0;
}

// FUNCTION: 0x0002a804
void sack_staff(Thing *staff, u16 mode)
{
    remove_person_from_room_list(staff);
    set_icon_value(staff, 9);
    if (staff->owner == 0x63) {
        return;
    }
    interrupt_staff(staff);
    switch (mode) {
    case 0:
        change_balance(0x18, -staff->unknown108, 0, staff->owner);
        break;
    case 1:
        staff->f8e &= 0xdf;
        break;
    }
    if (mode != 2) {
        staff_exit_room_check(staff);
        if (staff->room) {
            set_complex(staff, 0x136, staff->room->unknown6);
        }
        set_complex(staff, 0xbe, get_room_info(1, 0x65, staff->owner)->unknown4);
        staff->state_b = 0x38;
        staff->unknown94 = 0;
        fn_0003e724(staff);
    }
    remove_expertise_staff(staff);
    staff->f8d += staff->f8d / 4;
    staff->unknown81 = 100;
    staff->f8f = 0;
    staff->flags32 = 0;
    staff->f8e = 0;
    staff->unknown128 = 0;
    fn_000b05f0(staff);
    allhosps[staff->owner].fc9c--;
    allhosps[staff->owner].fc9e[staff->subtype == 4 ? 1 : staff->subtype]--;
    if (staff->unknown124 & 0x20) {
        allhosps[staff->owner].fca6--;
    }
    staff->owner = 0x63;
    if (mode == 2) {
        if (staff->room) {
            fn_000168a0(staff);
        }
        remove_thing(staff);
    }
}

// FUNCTION: 0x0002aa54
s16 staff_waits(Thing *worker)
{
    if (actual_game_turn % worker->anim_speed == 0) {
        worker->sprite = frame[worker->sprite].next;
    }
    if (--worker->height <= 0) {
        worker->height = worker->unknown18 == 0 ? div_err() : 0x100 / worker->unknown18;
        worker->unknown17 = old_newrand() & 3;
        {
            u16 idx;

            idx = check_frame(staff_anim_starts[worker->subtype].anims[staff[worker->subtype].anim], worker) + worker->unknown17;
            worker->sprite = start[idx].frame;
            worker->palette = start[idx].flags > 0 ? (u8)1 : (u8)0;
        }
    }
    return 0;
}

// FUNCTION: 0x0002abb0
void staff_in_corridor(Thing *staff)
{
    u16 anim;

    staff->unknown19 = staff->x.tile;
    staff->unknown20 = staff->y.tile;
    staff->kind = 0x34;
    staff->unknown103 = 6;
    staff->height = 1;
    anim = check_frame(staff_anim_starts[staff->subtype].unknownA, staff) + (u16)staff->unknown17;
    staff->sprite = start[anim].frame;
    staff->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
}

// FUNCTION: 0x0002ac70
u8 obey_pager(Thing *staff)
{
    RoomInfo *room;

    room = &room_info[check_pager(staff)];
    if (room <= room_info || room == staff->room) {
        return 0;
    }
    staff_exit_room_check(staff);
    staff->state_b = (u8)get_state((u16)room->type, staff);
    staff->unknown103 = 0;
    if (staff->room) {
        set_complex(staff, 0x136, staff->room->unknown6);
        fn_0003e724(staff);
        staff->state_a = 0x36;
        set_complex(staff, 0x14a, room->unknown4);
    } else {
        staff->state_a = 0x36;
        set_complex(staff, 0x14a, room->unknown4);
        fn_0003e724(staff);
    }
    staff->unknown89 = room - room_info;
    return 1;
}

// FUNCTION: 0x0002ad8c
u8 process_staff_complex(Thing *staff)
{
    RoomInfo *room;

    room = staff->room;
    if (room > room_info) {
        staff_exit_room_check(staff);
        set_complex(staff, 0x136, room->unknown6);
        staff->state_b = 0x28;
        fn_0003e724(staff);
    } else {
        staff->kind = 0x28;
    }
    return 1;
}

// FUNCTION: 0x0002adfc
u8 take_a_break(Thing *staff)
{
    RoomInfo *room;

    room = get_nearest_room_info(staff, 0x19);
    if (room <= room_info) {
        return 0;
    }
    if (staff->subtype == 3) {
        interrupt_staff(staff);
    }
    staff_exit_room_check(staff);
    staff->state_b = 0x30;
    if (staff->room > room_info) {
        set_complex(staff, 0x136, staff->room->unknown6);
        fn_0003e724(staff);
        staff->state_a = 0x36;
        set_complex(staff, 0x96, room->unknown4);
    } else {
        set_complex(staff, 0x96, room->unknown4);
        staff->state_a = 0x36;
        fn_0003e724(staff);
    }
    staff->unknown89 = room - room_info;
    return 1;
}

// FUNCTION: 0x0002af04
void staff_in_staff_room(Thing *staff)
{
    u16 roll;
    CompFrame *cur_frame;

    if (actual_game_turn % staff->anim_speed == 0) {
        staff->sprite = frame[staff->sprite].next;
        cur_frame = &frame[staff->sprite];
        if (cur_frame->flags & 1) {
            if (--staff->height <= 0) {
                if (staff->icon == 0x2da) {
                    rest_staff(staff, gbv.fdc);
                } else {
                    rest_staff(staff, gbv.fda);
                }
                staff->f8e &= ~1;
                if (staff->f8e & 8) {
                    staff->f8e &= 0xf0;
                    process_staff_complex(staff);
                    return;
                }
                if (staff->room->f22 || staff->room->f3 == 4) {
                    process_staff_complex(staff);
                    return;
                }
                roll = old_newrand() % 10;
                if ((find_object(staff->x.tile + (staff->y.tile << 7), 10, 1, 0x96, staff->owner) && staff->icon != 0x2ee) && roll == 1) {
                    if (staff_abilities[staff->subtype][1]) {
                        staff->state_a = 0x30;
                        staff->state_b = 0x30;
                        set_complex(staff, 0x2ee, 0);
                        fn_0003e724(staff);
                        return;
                    }
                }
                if ((find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), 0x39, 1, 0x96, staff->owner) && staff->icon != 0x2f8) && roll == 2) {
                    if (staff_abilities[staff->subtype][0]) {
                        staff->state_a = 0x30;
                        staff->state_b = 0x30;
                        set_complex(staff, 0x2f8, 0);
                        fn_0003e724(staff);
                    }
                    return;
                }
                if (staff->icon != 0x2da) {
                    if (find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), 0x13, 1, 0x96, staff->owner)) {
                    staff->state_a = 0x30;
                    staff->state_b = 0x30;
                    set_complex(staff, 0x2da, 0);
                    fn_0003e724(staff);
                    return;
                    }
                }
                if (overlapping_staff_check(staff, 0x30)) {
                    return;
                }
                staff->unknown18 = 0x20;
                if (staff->f78 <= 0) {
                    staff->unknown128 = (s16)staff->unknown128 / 2;
                    if (staff->subtype == 1 && staff->unknown148[2]) {
                        if (staff->f78 <= 0) {
                            if (staff->unknown30[4] == 3) {
                                staff->unknown30[4] = 1;
                            } else if (staff->unknown30[4] == 4) {
                                staff->unknown30[4] = 2;
                            }
                            staff->unknown148[2] = 0;
                        }
                    } else if (staff->subtype == 1) {
                        if ((staff->f8e & 0x10) && send_doctor_to_research(staff)) {
                            return;
                        }
                        if ((staff->f8e & 0x40) && send_doctor_to_training(staff)) {
                            return;
                        }
                        if (!obey_pager(staff)) {
                            set_complex(staff, 0x136, staff->room->unknown6);
                            staff->state_b = 0x28;
                            fn_0003e724(staff);
                        }
                    } else if (staff->subtype == 0) {
                        set_complex(staff, 0x136, staff->room->unknown6);
                        if (!nurse_check_for_work(staff)) {
                            staff->state_b = 0x28;
                            fn_0003e724(staff);
                        }
                    } else if (staff->subtype == 2) {
                        staff->kind = 0x36;
                        return;
                    } else if (staff->subtype == 3) {
                        staff->kind = 0x36;
                        return;
                    }
                    calculate_speed(staff);
                }
                staff->height = staff->unknown18 == 0 ? div_err() : 0x100 / staff->unknown18;
            }
        }
    }
}

// FUNCTION: 0x0002b400
u8 fn_0002b400(Thing *staff)
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
    if (staff->room->type == 10) {
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

// FUNCTION: 0x0002b528
u8 am_i_lead_staff(Thing *staff)
{
    u16 *holders;

    if (staff->room) {
        holders = staff->room->staff;
        if (staff->room->type == 10) {
            if (holders[0] == 0 || holders[1] == 0) {
                return 0;
            }
        }
        return (u16)(staff - thing) == holders[0] ? (u8)1 : (u8)0;
    }
    return 0;
}

// FUNCTION: 0x0002b5cc
void staff_nav_failure(Thing *staff)
{
    RoomInfo *room;

    if (staff->room == 0) {
        room = which_room((u16)staff->unknown19 + ((u16)staff->unknown20 << 7));
        if (room && actual_game_turn - 500 < room->unknown22) {
            process_staff_complex(staff);
            return;
        }
    }
    staff->unknown105 = staff->kind;
    staff->unknown106 = staff->unknown19;
    staff->unknown107 = staff->unknown20;
    staff->kind = 0x4e;
    staff->height = 0;
    if (staff->owner == player) {
        announce_this(1, 1);
    }
}

// FUNCTION: 0x0002b690
void draw_staff_cursor(void)
{
    u8 bad;
    s16 state;
    RoomInfo *room;
    Thing *held;
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
    held = &thing[gamevar.unknown75];
    if (held > thing) {
        D_001567b0[0] = D_001567b0[1] = 999;
        D_001567b0[2] = D_001567b0[3] = 0;
        draw_from_special_id_frame(start[gamevar.f73].frame, mcurx + resvars.f4 / 2, mcury - resvars.f3 / 2, remap, held->layers, start[gamevar.f73].flags);
    }
}

// FUNCTION: 0x0002b998
s32 pay_staff_bonus(Thing *staff, u16 amount, u16 percent, u16 morale, u8 owner)
{
    s32 total;
    Thing *member;

    total = 0;
    if (staff > thing) {
        if (percent) {
            total = staff->unknown108 * percent / 100;
        } else {
            total = amount;
        }
        if (morale) {
            staff->unknown81 += 50;
            if ((s16)staff->unknown81 < 0) {
                staff->unknown81 = 0;
            } else if ((s16)staff->unknown81 > 100) {
                staff->unknown81 = 100;
            }
            change_balance(0x1b, -amount, staff - thing, owner);
        }
    } else {
        for (member = thing + 1; member < end_things; member++) {
            if (member->renderer == 4 && member->owner == owner) {
                if (percent) {
                    total += percent * member->unknown108 / 100;
                } else {
                    total += amount;
                }
                if (morale) {
                    member->unknown81 += 50;
                    if ((s16)member->unknown81 < 0) {
                        member->unknown81 = 0;
                    } else if ((s16)member->unknown81 > 100) {
                        member->unknown81 = 100;
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
void remove_person_from_room_list(Thing *staff)
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

// FUNCTION: 0x0002bbdc
void staff_exit_room_check(Thing *staff)
{
    Thing *obj;

    if (staff->room) {
        if (staff->subtype == 4) {
            change_surgeon_to_doc(staff);
        }
        if (staff->room->type == 0x16 && &thing[staff->room->staff[0]] == staff) {
            obj = &thing[find_object((u16)staff->x.tile + ((u16)staff->y.tile << 7), 0x25, 0, 0x97, staff->owner)];
            if (obj > thing && obj->fuse) {
                staff->unknown61 = 0;
                staff->unknown63 = 0;
                staff->unknown75w = 0;
                staff->unknown65 = 0;
                if (&thing[staff->unknown69 && (thing[staff->unknown69].renderer == 6 || thing[staff->unknown69].renderer == 8)
                           && (u16)(staff - thing) == thing[staff->unknown69].unknown91 ? staff->unknown69 : 0] == obj) {
                    set_complex(staff, 0x3b6, 0);
                } else {
                    set_complex(staff, 0xa0, 0);
                }
                obj->fuse = 0;
            }
        }
        if (staff->subtype == 0 && staff->room->type == 9) {
            staff->room->f3 == 4 ? 0 : (staff->room->f3 = 0);
        }
    }
}

// FUNCTION: 0x0002be14
void rest_staff(Thing *staff, s16 amount)
{
    if (staff->f78 > gbv.fee / 2) {
        if ((amount = amount * gbv.fee / staff->f78) < gbv.ff0) {
            amount = gbv.ff0;
        }
    }
    staff->f78 -= amount;
    staff->unknown81++;
    if ((s16)staff->unknown81 < 0) {
        staff->unknown81 = 0;
    } else if ((s16)staff->unknown81 > 100) {
        staff->unknown81 = 100;
    }
}

// FUNCTION: 0x0002becc
void staff_picked_up(Thing *staff)
{
    staff_exit_room_check(staff);
    abandon_complex(staff);
    allhosps[staff->owner].f11f6 = staff->x.tile + (staff->y.tile << 7);
    if (othervar.ff == player) {
        gamevar.unknown77 = staff->subtype;
        gamevar.unknown75 = staff - thing;
        gamevar.unknown79 = 0xffff;
        set_mouse_mode(3);
    }
    remove_person_from_room_list(staff);
    remove_node(staff);
    staff->kind = 0x44;
}

// FUNCTION: 0x0002bf94
void remove_node(Thing *staff)
{
    if (staff->room) {
        fn_000168a0(staff);
    }
    fn_00015b68(staff);
}

// FUNCTION: 0x0002bfc8
u32 shall_i_resign(Thing *staff)
{
    u32 limit;

    if (staff->f8e & 0x20) {
        set_icon_value(staff, 9);
        if (staff->owner == player) {
            advisor.f128 |= 1;
        }
        if (actual_game_turn >= staff->flags32) {
            get_playercall(1, staff->unknown91, staff->owner);
            sack_staff(staff, 1);
            staff->unknown91 = 0;
            return 1;
        }
    } else {
        limit = ((old_newrand() & 3) + 6) * 1500;
        if (actual_game_turn - (u16)staff->unknown130 > limit) {
            if ((s16)staff->unknown128 > gbv.f10a) {
        staff->f8e |= 0x20;
        staff->f8f++;
        staff->flags32 = actual_game_turn + 1500;
        staff->unknown91 = make_a_playercall(1, staff - thing, 0, staff->owner);
                return 1;
            }
        }
        if (staff->f93) {
            if (staff->unknown128s > gbv.f10a / 4) {
            staff->f8e |= 0x20;
            staff->flags32 = actual_game_turn + 1500;
            staff->unknown91 = make_a_playercall(1, staff - thing, 0, staff->owner);
                return 1;
            }
            staff->f93 = 0;
        }
    }
    return 0;
}

// FUNCTION: 0x0002c1cc
u16 count_emergency_staff(u8 owner)
{
    u32 count;
    Thing *doctor;

    count = 0;
    for (doctor = thing + 1; doctor < end_things; doctor++) {
        if ((doctor->renderer == 4 && doctor->subtype == 1) && doctor->owner == owner) {
            if (doctor->f78 < gbv.fe8) {
                count++;
            }
        }
    }
    return count;
}

// FUNCTION: 0x0002c268
void calculate_speed(Thing *staff)
{
    u32 speed;

    speed = 3;
    if (staff->x.frac == 0x80 && staff->y.frac == 0x80) {
        if (staff->f78 > gbv.fec) {
            speed = 1;
        } else if (staff->f78 > gbv.fea) {
            speed = 2;
        }
        if (staff->unknown126 >= gbv.f158) {
            speed += 30;
        } else if (staff->unknown126 > gbv.f156) {
            speed += 20;
        } else {
            speed += 10;
        }
        if (staff->room) {
            if (staff->room->type == 0x16) {
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
            staff->unknown18 = 0x10;
            staff->anim_speed = 2;
            break;
        case 23:
        case 32:
            staff->unknown18 = 0x20;
            staff->anim_speed = 1;
            break;
        case 33:
            staff->unknown18 = 0x40;
            staff->anim_speed = 1;
            break;
        }
    }
}

// FUNCTION: 0x0002c3bc
void interrupt_staff(Thing *staff)
{
    Thing *partner;
    u16 *slots;

    private_interrupt_staff(staff);
    if (staff->subtype == 4) {
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

// FUNCTION: 0x0002c484
void private_interrupt_staff(Thing *staff)
{
    Thing *patient;
    Thing *target_obj;
    RoomInfo *room;

    patient = &thing[staff->unknown73];
    if (staff->subtype == 2) {
        target_obj = &thing[thingpage_remind_me(&allhosps[staff->owner].ff6d, staff)];
        if (target_obj > thing) {
            room = &room_info[target_obj->unknown89];
            if (room > room_info) {
                if (room->f3 == 5 || room->f3 == 6) {
                    room->f3 = 0;
                }
            }
        }
        room = staff->room;
        if ((room && (target_obj = &thing[staff->unknown69]) > thing) && objects[target_obj->subtype].unknown24) {
            if (room->f3 == 5 || room->f3 == 6) {
                room->f3 = 0;
            }
        }
        thingpage_cancel_my_response(&allhosps[staff->owner].ff6d, staff);
        thingpage_cancel_my_response(&allhosps[staff->owner].fd2b, staff);
    }
    interrupt_complex(staff);
    if ((staff->subtype == 0 && patient > thing) && staff->room == 0) {
        patient->flags &= ~2;
        staff->unknown73 = 0;
    } else if ((patient > thing && patient->renderer == 2) && (staff->room != 0 || staff->subtype == 3)) {
        staff->unknown73 = 0;
        interrupt_person(patient);
    }
    if (staff->room) {
        staff->kind = get_state((u16)staff->room->type, staff);
    } else {
        staff->kind = get_state(0, staff);
    }
    staff->unknown103 = 0;
    staff_abandon_pager_response(staff);
    if (staff->subtype == 3) {
        receptionist_reset(staff);
    }
}

// FUNCTION: 0x0002c700
void staff_join_new_employer(u16 id, u16 owner)
{
    Thing *staff;

    staff = &thing[id];
    if (staff > thing) {
        interrupt_staff(staff);
        if (staff->owner == player) {
            announce_this(0xb, 1);
        }
        staff_exit_room_check(staff);
        if (staff->room) {
            set_complex(staff, 0x136, staff->room->unknown6);
        }
        set_complex(staff, 0xbe, get_room_info(4, 0x65, owner)->unknown4);
        staff->state_b = 0x28;
        fn_0003e724(staff);
        remove_expertise_staff(staff);
        staff->f8d += staff->f8d / 4;
        staff->unknown81 = 100;
        staff->f8f = 0;
        staff->unknown130 = actual_game_turn;
        staff->flags32 = 0;
        staff->f8e = 0;
        staff->unknown128 = 0;
        fn_000b05f0(staff);
        allhosps[staff->owner].fc9c--;
        allhosps[owner].fc9c++;
        allhosps[staff->owner].fc9e[staff->subtype == 4 ? 1 : staff->subtype]--;
        allhosps[owner].fc9e[staff->subtype == 4 ? 1 : staff->subtype]++;
        if (staff->unknown124 & 0x20) {
            allhosps[staff->owner].fca6--;
            allhosps[owner].fca6++;
        }
        staff->owner = owner;
        set_expertise_staff(staff);
        insert_skill(staff->unknown124, staff->owner);
        staff->unknown128 = 0;
        staff->unknown108 *= 2;
        if (!listhead[owner].rooms[4]) {
            sack_staff(staff, 0);
        }
    }
}

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
        if (room->type != 10 || room->staff[0] && room->staff[1]) {
            old_holder = &thing[*slot];
            interrupt_staff(old_holder);
            process_staff_complex(old_holder);
        }
        *slot = staff - thing;
    }
}

// FUNCTION: 0x0002ca88
s16 get_state(const u16 type, const Thing *staff)
{
    s16 state;

    state = staff_states[rooms[type].unknown2[1]][staff->subtype];
    if (staff->subtype == 1 && rooms[type].unknown8) {
        if ((staff->unknown124 & rooms[type].unknown8) == 0) {
            state = 0x36;
        }
    }
    return state;
}

// FUNCTION: 0x0002cb2c
void increase_staff_salaries(Thing *staff, u16 amount, u16 percent, u8 owner)
{
    s32 total;
    Thing *member;

    total = 0;
    if (staff > thing) {
        if (percent) {
            staff->unknown108 += percent * staff->unknown108 / 100;
        } else {
            staff->unknown108 += amount;
        }
        if (staff->unknown108 > 90000) {
            staff->unknown108 = 90000;
        }
        staff->unknown128 = 0;
        staff->unknown81 += 50;
        if ((s16)staff->unknown81 < 0) {
            staff->unknown81 = 0;
        } else if ((s16)staff->unknown81 > 100) {
            staff->unknown81 = 100;
        }
    } else {
        for (member = thing + 1; member < end_things; member++) {
            if (member->renderer == 4 && member->owner == owner) {
                if (percent) {
                    member->unknown108 += percent * member->unknown108 / 100;
                } else {
                    member->unknown108 += amount;
                }
                if (staff->unknown108 > 90000) {
                    staff->unknown108 = 90000;
                }
                member->unknown128 = 0;
                member->unknown81 += 50;
                if ((s16)member->unknown81 < 0) {
                    member->unknown81 = 0;
                } else if ((s16)member->unknown81 > 100) {
                    member->unknown81 = 100;
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
    Map *cell;
    s16 id;

    count = 0;
    lvl = &rat_fest_level[level];
    seed = safeseed = lvl->seed;
    for (cell = map; cell < end_map; cell++) {
        if (cell->unknown7 == 1 << player) {
            if (lvl->staff_max > count && saferand() % lvl->staff_chance == 0) {
                gamevar.f172 = 1;
                gamevar.f16f = 1;
                gamevar.f170 = 0;
                gamevar.f171 = 0;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(cell - map, id, 0, player);
                }
                gamevar.f16f = 0;
                gamevar.f170 = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(cell - map, id, 0, player);
                }
                gamevar.f170 = 0;
                gamevar.f171 = 1;
                id = place_staff_in_struct(1, 0);
                if (id) {
                    fn_00029c4c(cell - map, id, 0, player);
                }
                break;
            }
        }
    }
}

// FUNCTION: 0x0002ce9c
u8 is_staff_stepping_on_toes(Thing *staff)
{
    Thing *other;

    for (other = &thing[staff->next]; other > thing; other = &thing[other->next]) {
        if (other->renderer == 4) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002cf14
u8 overlapping_staff_check(Thing *staff, s32 state)
{
    if (actual_game_turn % 47 == 0 || ((s16)staff->y.tile << 7) + (s16)staff->x.tile == staff->room->unknown6
        || is_staff_stepping_on_toes(staff)
        || (staff->unknown69 && (thing[staff->unknown69].renderer == 6 || thing[staff->unknown69].renderer == 8)
                   && (u16)(staff - thing) == thing[staff->unknown69].unknown91 ? staff->unknown69 : 0) == 0
            && (map[(s16)staff->x.tile + ((s16)staff->y.tile << 7)].flags & 7)) {
        RoomInfo *room;
        u16 tile;
        u16 dest;

        room = staff->room;
        tile = staff->x.tile + (staff->y.tile << 7);
        dest = room->pos + (((s16)(old_newrand() % room->f2) << 7) + (s16)(old_newrand() % room->f1));
        if (tile != dest && can_i_get_to(staff, dest)) {
            staff->state_a = state;
            staff->state_b = staff->state_a;
            set_complex(staff, 0xbe, dest);
            fn_0003e724(staff);
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0002d11c
u8 fn_0002d11c(Thing *staff)
{
    Thing *obj;

    if (staff->subtype == 4) {
        obj = &thing[find_object(staff->x.tile + (staff->y.tile << 7), 0x23, 0, 0x97, staff->owner)];
        if (obj > thing) {
            if (obj->unknown98 != 0) {
                return 0;
            }
        }
    }
    return 1;
}
