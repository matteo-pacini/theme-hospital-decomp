#include "hosp.h"

struct Struct_0015f410 {
    u8 unknown0[0x570];
    int f570;
    u8 unknown574[0x10];
    int f584;
    u8 unknown588[0xc];
    int f594;
};

struct Struct_001602a0 {
    u8 unknown0[0x6c];
    int f6c;
};

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    u16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct Rect {
    int x0;
    int y0;
    int x1;
    int y1;
};

struct Stride6c {
    u16 f0;
    u8 unknown2[0x6a];
};

extern "C" {

extern Event D_001549d0[];
extern Event D_00154a0c[];
extern MemRegion D_001290d4[];
extern u32 D_00160288;
extern u16 D_0015fe86[];
extern u32 D_0015fe78, D_0015fe7c, D_0015fe80;
extern u32 D_0016028c, D_00160290;
extern u8 D_00160294;
extern Struct_001602a0 *D_001602a0;
extern Struct_0015f410 *D_0015f410;
extern char D_00120260[], D_0012026a[], D_0012027f[], D_0012028f[], D_001202a4[], D_001202aa[];
extern u8 D_001581ee;
extern s32 D_001581e8;
extern u8 D_001581ef, D_001581f0;
extern u16 D_0015536e;
extern u8 D_001553c7, D_001553c8;
extern u8 D_00155c69;
extern u16 D_0012ddae;
extern u8 D_00155dc6[];
extern u8 D_00188c7b, D_00188c7c, D_00188c81, D_00188c83;
extern u8 D_00155e07;
extern u8 D_00155e14;
extern u16 D_00155e21;
extern u8 D_00155d71;
extern u8 D_001553e9;
extern u8 *D_00153f34;
extern u8 D_00156910;
extern u8 *D_001553d7;
extern u8 D_00158166;
extern u16 D_00155419, D_0015541b, D_0015541d;
extern Slot D_001581b2[];
extern u8 D_00155d70;
extern Window *D_0015f414;
extern Rect *D_0015f40c;
extern u8 D_001553ec[];
extern u8 D_00155d7d;
extern char D_001563c5[];
extern u8 D_00155e09;
extern u8 D_00155402;
extern u8 D_00155430;
extern Blob63 D_00155c6f;
extern u8 D_0012b7c8[];
extern u32 D_0015f8e9, D_0015f8ed;
extern u8 D_00158324;
extern u16 D_00155376;
extern u8 D_00155e1d;
extern u8 D_00155d7b, D_00155d7c;
extern u16 D_00155d69;
extern u16 D_0015543a;
extern Stride6c D_0015f5c2[];
extern u32 D_0015542c;
extern u8 D_00155e4d;
extern u16 D_0015542a;
extern u16 D_001553fd;
extern u16 D_00155d57;
extern u8 D_00155e4c;
extern char D_001202ba[], D_001202c1[], D_001202c8[], D_001202cf[], D_001202e2[], D_001202f3[];
extern TbScreenMode D_0015f3f5;
extern u8 D_0015f3f7, D_0015f3f8;
extern u16 D_00155444, D_00155372, D_00155374;
extern u8 D_00155e1e;
extern u16 D_00155e1f;
extern u8 D_00155db1;
extern TbCacheUsedBlock *D_00155dbe;
extern u8 D_0015532d[5];
extern u8 D_00155333, D_00155334;
extern u8 *D_001553dc;

int fn_000bed50(void);
u32 fn_00022790(void);
u8 fn_00086b60(void);
void fn_0008debc(char *s);
void fn_000228e0(void);
void fn_00022a58(void);
void fn_0007fbb8(int a);
void fn_0002994c(u8 a);
u8 fn_0002d11c(Thing *o);
int fn_0001c030(int n);
void fn_0001c084(int n);
u16 fn_0001c1b4(u16 *p, int len);
void fn_0001c234(void);
int fn_0001c318(s8 port, u32 base, int len);
void fn_0001c5e4(void);
void *fn_00022b60(TbCacheUsedBlock *block);

}

// FUNCTION: 0x0001c030
int fn_0001c030(int n)
{
    if (!(n >= 0x11 || n == 0xc || n == 0xd || n == 7 || n == 9)) {
        return 1;
    }
    return 0;
}

// FUNCTION: 0x0001c084
void fn_0001c084(int n)
{
    u8 flag = lbDisplay.unknown4 != 0;

    if (display_mode == 0) {
        char buf[32];

        if (!flag) {
            while (fn_000bed50() != 1) {
            }
        }
        sprintf(buf, D_00120260, D_001602a0->f6c, n);
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.unknown18 - panel->x, lbDisplay.unknown1c - panel->y);
        set_font_from_cache(0x2c);
        LbTextDraw(D_0015f410->f570, D_0015f410->f584 + D_0015f410->f594, buf);
        set_all_refresh_zones_in_window(0, lbDisplay.unknown1c - 0x5c, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        LbIsActive();
        special_refresh();
        if (flag) {
            while (fn_000bed50() != 1) {
            }
        }
    }
}

// FUNCTION: 0x0001c1b4
u16 fn_0001c1b4(u16 *p, int len)
{
    u32 sum = 0;

    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    if (len > 0) {
        sum += *(u8 *)p;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}

// FUNCTION: 0x0001c234
void fn_0001c234(void)
{
    int i;
    u32 p;
    u32 base;
    u32 chunk;
    u32 size;

    D_00160288 = 0;
    for (i = 0; D_001290d4[i].addr != 0; i++) {
        if (fn_0001c030(i)) {
            p = base = D_001290d4[i].addr;
            size = D_001290d4[i].size;
            while (base + size > p) {
                chunk = 0x400;
                if (p + chunk > base + size) {
                    chunk = base + size - p;
                }
                if (D_00160288 < 0x200) {
                    D_0015fe86[D_00160288++] = fn_0001c1b4((u16 *)p, chunk);
                }
                p += chunk;
            }
        }
    }
    D_00160288 <<= 1;
}

// FUNCTION: 0x0001c318
int fn_0001c318(s8 port, u32 base, int len)
{
    int tries;
    int chunk;
    int off;

    if (base == D_001290d4[0].addr) {
        D_0015fe78 = D_0015fe80 = 0;
    }
    if (port < 0) {
        off = 0;
        while (off < len) {
            chunk = 0x400;
            if (off + chunk > len) {
                chunk = len - off;
            }
            if (D_0015fe86[D_0015fe78++] != fn_0001c1b4((u16 *)(base + off), chunk)) {
                tries = 0;
                do {
                } while (LbNetwork_Send(modem_dev.unknown1c, &off, 4) == -1 && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                tries = 0;
                do {
                } while (LbNetwork_Receive(modem_dev.unknown1c, (void *)(base + off), chunk) != chunk && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                if ((D_0015fe7c++ & 0xf) == 0) {
                    fn_0001c084((D_00160290 + chunk) * 100 / D_0016028c);
                }
            }
            off += chunk;
            D_00160290 += chunk;
        }
        tries = 0;
        off = -1;
        do {
        } while (LbNetwork_Send(modem_dev.unknown1c, &off, 4) == -1 && tries++ < 8);
        if (tries >= 8) {
            return 0;
        }
    } else {
        int end = 0;

        off = 0;
        while (off >= 0) {
            tries = 0;
            do {
            } while (LbNetwork_Receive(port, &off, 4) != 4 && tries++ < 8);
            if (tries >= 8) {
                return 0;
            }
            if (off >= 0) {
                chunk = 0x400;
                if (off + chunk > len) {
                    chunk = len - off;
                }
                tries = 0;
                do {
                } while (LbNetwork_Send(port, (void *)(base + off), chunk) == -1 && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                end = off + chunk;
            }
            if ((D_0015fe7c++ & 0xf) == 0) {
                fn_0001c084((D_00160290 + end) * 100 / D_0016028c);
            }
        }
        D_00160290 += len;
        fn_0001c084(D_00160290 * 100 / D_0016028c);
    }
    D_0015fe80++;
    return 1;
}

// NONMATCHING: eax/edx swapped when comparing two table entries (register allocation); the
// modem_dev.unknown1c index is also evaluated before the player index, where the target
// orders it as it would a scalar global
// FUNCTION: 0x0001c5e4
void fn_0001c5e4(void)
{
    int value;
    int i;
    int tries;

    if (modem_dev.unknown1c == player) {
        value = 1;
        for (i = 0; i < 4; i++) {
            if (D_001549d0[i].checksum != 0) {
                if (D_001549d0[player].checksum != D_001549d0[i].checksum) {
                    fn_0001c084(0);
                    D_00160290 = 0;
                    tries = 0;
                    do {
                    } while (LbNetwork_Send(i, &value, 4) == -1 && tries++ < 8);
                    if (tries >= 8) {
                        return;
                    }
                    if (!fn_00021818(0, i)) {
                        return;
                    }
                    fn_0001c084(100);
                }
            }
        }
        for (i = 0; i < 4; i++) {
            if (i == player || D_001549d0[i].checksum == 0) {
                continue;
            }
            tries = 0;
            do {
            } while (LbNetwork_Send(i, &value, 4) == -1 && tries++ < 8);
            if (tries >= 8) {
                return;
            }
        }
    } else {
        if (D_001549d0[player].checksum != D_001549d0[modem_dev.unknown1c].checksum) {
            fn_0001c084(0);
            D_00160290 = 0;
            value = 0;
            while (value == 0) {
                LbNetwork_Receive(modem_dev.unknown1c, &value, 4);
            }
            fn_00021c38(0, 1);
            fn_0001c084(100);
        }
        value = 0;
        while (value == 0) {
            LbNetwork_Receive(modem_dev.unknown1c, &value, 4);
        }
    }
}

// NONMATCHING: eax/edx/ebx choice differs in many indexed loads and stores
// FUNCTION: 0x0001c7c0
void process_turn(void)
{
    char buf[100];
    Thing *obj2;
    int w;
    Thing *obj1;
    int h;
    u16 j;
    u16 i;
    u8 changed;

    if (D_001581ee & 2) {
        u32 version;

        LbFileRead(D_001581e8, packet, 0xf);
        LbFileRead(D_001581e8, &w, 4);
        LbFileRead(D_001581e8, &h, 4);
        LbFileRead(D_001581e8, &D_0015536e, 2);
        LbFileRead(D_001581e8, &gamevar, 0x178);
        LbFileRead(D_001581e8, &version, 4);
        if (seed != version) {
            LbDataLoadSetModifyFilenameFunction(debug.unknown116, D_0012026a);
            debug.unknown1de = 200;
        }
        LbMouseSetPosition(w, h);
    } else if (D_001581ee & 1) {
        w = mouse_dx;
        h = mouse_dy;
        LbFileWrite(D_001581e8, packet, 0xf);
        LbFileWrite(D_001581e8, &w, 4);
        LbFileWrite(D_001581e8, &h, 4);
        LbFileWrite(D_001581e8, &D_0015536e, 2);
        LbFileWrite(D_001581e8, &gamevar, 0x178);
        LbFileWrite(D_001581e8, &seed, 4);
    }
    if (D_001549d0[player].flags && !pmenu.unknown0) {
        if (D_001549d0[player].flags & 1) {
            scroll.unknown0 = 1;
        } else if (D_001549d0[player].flags & 2) {
            scroll.unknown1 = 1;
        }
        if (D_001549d0[player].flags & 4) {
            scroll.unknown2 = 1;
        } else if (D_001549d0[player].flags & 8) {
            scroll.unknown3 = 1;
        }
        if (D_001549d0[player].flags & 0x10) {
            goto_column_row_in_window(window.unknown2eb, level->unknown28024[player] % 128, level->unknown28024[player] / 128);
            D_001553c8 = 1;
        }
        D_00154a0c[player].flags = D_001549d0[player].flags;
        D_001549d0[player].flags = 0;
    }
    if (statvar.unknownfd > 1) {
        D_001549d0[player].checksum = fn_00022790();
        sprintf(buf, D_0012027f, 0xf);
        if (!fn_00086b60()) {
            LbNetwork_ChangeExchangeBuffer(packet, 0xf);
            fn_0008debc(D_0012028f);
            if (LbNetwork_Exchange((u8 *)packet + player * 0xf) == -1) {
            }
            fn_0008debc(D_001202a4);
            if (game_turn == 4) {
                LbNetwork_LibraryVersion(0);
            }
            changed = 0;
            for (j = 0; j < 4; j++) {
                if (D_001609c4[j].active && D_001549d0[j].checksum) {
                    if (D_001549d0[modem_dev.unknown1c].checksum != D_001549d0[j].checksum) {
                        if (D_001549d0[j].type != 0x8e && D_001549d0[j].type != 0x8d) {
                            D_001549d0[j].type = 0;
                            D_001549d0[j].value = 0;
                            D_001549d0[j].arg = 0;
                        }
                        changed = 1;
                    }
                }
            }
            if (changed) {
                finish_composite_help();
                D_00160294 = 1;
                fn_0001c5e4();
                D_00160294 = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            u8 handled = 0;

            switch (D_001549d0[i].type) {
            case 0x8d:
                network_lost_focus(i);
                handled = 1;
                break;
            case 0x8e:
                network_gained_focus(i);
                handled = 1;
                break;
            }
            if (handled) {
                D_001549d0[i].type = 0;
                D_001549d0[i].value = 0;
                D_001549d0[i].arg = 0;
                return;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        D_001553c7 = i;
        if (allhosps[i].f2b != 2) {
            switch (D_001549d0[i].type) {
            case 0x1c:
            case 0x7e:
                goto dispatch;
            }
            continue;
        }
    dispatch:
        switch (D_001549d0[i].type) {
        case 0:
            break;
        case 1:
            open_room(i);
            break;
        case 2:
            modify_blueprint(i, 1, D_001549d0[i].value);
            break;
        case 0x12:
            if (commit_blueprint(i)) {
                if (i == player) {
                    play_sound(10, 0x41, -1, -1);
                }
            } else {
                if (i == player) {
                    play_sound(10, 8, -1, -1);
                }
            }
            break;
        case 0x13:
            uncommit_blueprint(i);
            if (i == player) {
                set_mouse_mode(1);
            }
            break;
        case 0x14:
            if (i == player) {
                crequester.unknown4 = 1;
                set_buyable_items(3);
            }
            break;
        case 3:
            place_bp_door(i, (u8)D_001549d0[i].value);
            break;
        case 4:
            if (i == player) {
                set_mouse_mode(0x10);
            }
            commit_doors(i);
            break;
        case 5: {
            u8 empty = !allhosps[i].f11f4;

            if (fn_00029c4c(D_001549d0[i].value, D_001549d0[i].arg, 0, i)) {
                if (empty) {
                    change_balance(2, -(thing[D_001549d0[i].arg].unknown108 * 12) / 10, 0, D_001553c7);
                }
                if (i == player) {
                    play_sound(10, 7, -1, -1);
                    set_mouse_mode(0);
                }
            }
            break;
        }
        case 7:
            if (fn_00037098(D_001549d0[i].value, D_001549d0[i].argb[0], D_001549d0[i].argb[1], i) != 0) {
                if (i == player) {
                    play_sound(10, 0x46, -1, -1);
                    object_placed();
                }
            }
            break;
        case 8:
            modify_blueprint(i, 0, D_001549d0[i].value);
            break;
        case 9:
            queue_person_to_new_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0xa:
            if (i == player) {
                compute_whether_user_not_busy();
            }
            switch (D_001549d0[i].value) {
            case 1:
                j = get_playercall(1, D_001549d0[i].arg, i);
                if (j) {
                    if (i == player && D_00155c69) {
                        open_staff_resign_requester(&thing[j]);
                    }
                }
                break;
            case 2:
                if (check_playercall(2, 0, i)) {
                    save_popup(2);
                    if (i == player) {
                        display_choice_screen(i, 2, 0, 0);
                    }
                }
                break;
            case 3:
                get_playercall(3, 0, i);
                if (allhosps[i].f205) {
                    allhosps[i].f203 = allhosps[i].f205 * D_0012ddae;
                    display_choice_screen(i, 5, 0, 0);
                }
                break;
            case 4:
                j = get_playercall(4, D_001549d0[i].arg, i);
                if (j) {
                    display_choice_screen(i, 7, j, 0);
                }
                break;
            case 5:
                j = get_playercall(5, 0, i);
                switch (j) {
                case 1:
                    allhosps[i].f1e2 = 0;
                    if (i == player) {
                        display_choice_screen(i, 3, 0, 400);
                    }
                    break;
                case 2:
                    allhosps[i].f1fa = 0;
                    if (i == player) {
                        display_choice_screen(i, 9, 0, 400);
                    }
                    break;
                case 3:
                    allhosps[i].f20e = 0;
                    display_choice_screen(i, 6, allhosps[i].f20c, 400);
                    break;
                }
                break;
            case 6:
                j = get_playercall(6, 0, i);
                display_choice_screen(i, 8, j, 0);
                break;
            case 7:
                j = get_playercall(7, 0, i);
                display_choice_screen(i, 4, j, 0);
                break;
            case 8:
                j = get_playercall(8, D_001549d0[i].arg, i);
                if (j) {
                    display_choice_screen(i, 10, j, 0);
                }
                break;
            }
            break;
        case 0xb:
            place_small_thing(D_001549d0[i].value, D_001549d0[i].arg, 0);
            if (i == player) {
                play_sound(10, 0x46, -1, -1);
            }
            break;
        case 0xc:
            if (i == player) {
                if (D_00155dc6[D_001549d0[i].value] == 1) {
                    if (allhosps[i].f11f4) {
                        fn_00029c4c(allhosps[i].f11f6, allhosps[i].f11f4, 1, i);
                    } else if (gamevar.unknown75) {
                        req2_exit_employing(gamevar.unknown75);
                    }
                    close_yes_no_requester(0x63);
                    set_mouse_mode(0);
                    D_00188c7b = 0;
                }
                fn_00021818((s8)D_001549d0[i].value, -1);
                check_save_games();
            }
            break;
        case 0xd:
            if (i == player) {
                fn_00021c38((s8)D_001549d0[i].value, -1);
            }
            break;
        case 0xe:
            vary_speed((s16)D_001549d0[i].value);
            break;
        case 0x17:
            break;
        case 0x18:
            break;
        case 0x1a:
            temp_place_door(D_001549d0[i].value, i);
            break;
        case 0x20:
            if (i == player) {
                crequester.unknown4 = D_001549d0[i].value;
                crequester.unknown0 = 4;
                req1_reset_rooms(crequester.unknown4);
            }
            break;
        case 0x35:
            init_blueprint(i, D_001549d0[i].value);
            if (i == player) {
                crequester.unknown8 = D_001549d0[i].value;
                set_buyable_items(crequester.unknown8);
                buy_compulsory_items(crequester.unknown8);
                close_crequester(crequester.unknown2);
                set_mouse_mode(1);
                open_edit_room_requester();
                set_edit_room_requester_mode(0);
                pmenu.unknown7 = 0;
            }
            allocate_roominfo(i, D_001549d0[i].value);
            break;
        case 0x1c:
            if (i == player) {
                if (crequester.unknown2 == 4) {
                    close_crequester(4);
                    pmenu.unknown7 = 0;
                }
            }
            if (statvar.unknownfd > 1) {
                LbNetwork_Stop();
                if (D_00155e07) {
                    stop_midi();
                }
                statvar.unknownfd = 1;
                player = 0;
                fn_0007fbb8(-1);
            }
            quit_game = 1;
            if (debug.unknowne2 == 1) {
            }
            if (D_001581ee & 2) {
                if (D_001581f0) {
                    quit_game = 2;
                    for (;;) {
                        char name[50];

                        if (D_001581ef < 10) {
                            D_001581ef++;
                        } else {
                            D_001581ef = 1;
                        }
                        LbFileClose(D_001581e8);
                        sprintf(name, D_001202aa, D_001563c5, D_001581ef);
                        D_001581e8 = LbFileOpen(name, 2);
                        if (D_001581e8 > 0) {
                            D_001581ee |= 2;
                            break;
                        }
                    }
                }
            }
            break;
        case 0x1e:
            if (i == player) {
                if (fn_00021c38((s8)D_001549d0[i].value, -1) == 0) {
                    goto_cell_in_window(0, get_room_info(1, 0x65, D_001553c7)->unknown4);
                    D_001553c8 = 1;
                }
            }
            break;
        case 0x1d:
            goto_column_row_in_window(window.unknown2eb, level->unknown28024[player] % 128, level->unknown28024[player] / 128);
            D_00155e14 = 1;
            D_001553c8 = 1;
            break;
        case 0x21:
            break;
        case 0x22:
            yearend.unknown0--;
            if (display_mode != 8 || yearend.unknown0 == 0) {
                gamevar.unknown21 = !gamevar.unknown21;
                if (i == player) {
                    int handle;

                    if (gamevar.unknown21) {
                        StopAllSamples();
                        D_00155e21 = 0x32;
                        if (D_00155e07) {
                            handle = GetSongHandle();
                            AIL_stop_sequence(handle);
                            D_00155e07 = 0;
                        }
                    } else {
                        D_00155e21 = 0x32;
                        if (myplayer.unknown11 && statvar.unknowne5 && D_00155d71) {
                            handle = GetSongHandle();
                            AIL_resume_sequence(handle);
                            D_00155e07 = 1;
                        }
                    }
                }
            }
            break;
        case 0x26:
            if (i == player) {
                close_crequester(crequester.unknown2);
                close_irequesters();
                set_mouse_sprite(D_001553e9);
                pmenu.unknown7 = 0;
            }
            break;
        case 0x27:
            if (i == player) {
                close_crequester(crequester.unknown2);
                set_mouse_sprite(D_001553e9);
                pmenu.unknown7 = 0;
            }
            break;
        case 0x25:
            if (i == player) {
                req1_buy_object(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case 0x2b:
            if (i == player) {
                u16 k;

                close_crequester(crequester.unknown2);
                k = next_object_to_build();
                if (k) {
                    init_place_object(k, 2, 0, 0);
                }
                pmenu.unknown7 = 0;
            }
            break;
        case 0x29:
            hosp_fade_palette(0, 0x32, Lb_PALETTE_FADE_CLOSED);
            break;
        case 0x2a:
            if (i == player) {
                hosp_fade_palette(D_00153f34, 0x32, Lb_PALETTE_FADE_CLOSED);
            }
            break;
        case 0x2e:
            if (i == player) {
                cancel_top_level_requesters(0xb);
                if (crequester.unknown2 == 0x63) {
                    req2_reset();
                    close_irequesters();
                    fn_0001bc94(crequester.unknown2, 1);
                    set_mouse_sprite(1);
                    pmenu.unknown7 = 1;
                }
            }
            break;
        case 0x2f:
            if (i == player) {
                crequester.unknown0 = 2;
                crequester.unknown4 = D_001549d0[i].value;
                crequester.unknown6 = req2_alter_staff(1, 0, crequester.unknown4);
            }
            break;
        case 0x30:
            if (i == player) {
                if (thing[D_001549d0[i].arg].kind != 0x44) {
                    close_crequester(crequester.unknown2);
                    gamevar.unknown77 = D_001549d0[i].value;
                    gamevar.unknown75 = D_001549d0[i].arg;
                    gamevar.unknown79 = 0xffff;
                    set_mouse_mode(3);
                    pmenu.unknown7 = 0;
                }
            }
            req2_mark_employing(D_001549d0[i].arg);
            break;
        case 0x32:
            if (i == player) {
                crequester.unknown6 = req2_alter_staff((u8)D_001549d0[i].value, crequester.unknown6, crequester.unknown4);
            }
            break;
        case 0x34:
            if (display_mode != 0) {
                display_mode = 0;
                D_001553c8 = 1;
                D_00156910 = 1;
                D_001553d7 = D_00153f34;
            }
            break;
        case 0x33:
            if (i == player && D_0015536e == 0xe) {
                play_sound(10, 8, -1, -1);
                break;
            }
            if (allhosps[i].f11f4) {
                fn_00029c4c(allhosps[i].f11f6, allhosps[i].f11f4, 1, i);
            }
            update_cure_costs();
            if (i == player) {
                display_mode = 1;
                chart_type = D_001549d0[i].value;
                switch (chart_type) {
                case 0:
                    bank.unknown2 = 0;
                    break;
                case 1:
                    drugs.unknown2 = 0;
                    break;
                case 2:
                    pol.unknown2 = 0;
                    break;
                case 3:
                    research.unknown2 = 0;
                    break;
                case 4:
                    graphs.unknown2 = 0;
                    break;
                case 5:
                    slist.unknown2 = 0;
                    break;
                case 6:
                    calculate_value(player);
                    manager.unknown2 = 0;
                    break;
                case 7:
                    D_00158166 = 0;
                    calculate_value(player);
                    break;
                case 8:
                    reputation.unknown2 = 0;
                    break;
                case 9:
                    variables.unknown2 = 0;
                    break;
                case 0xd:
                    yearend.unknown2 = 0;
                    break;
                case 0xf:
                    hall.unknown2 = 0;
                    break;
                }
                D_001553c8 = 1;
                D_00156910 = 1;
            }
            break;
        case 0x36:
            if (statvar.unknownfd > 1) {
                town.unknown2 = 1;
            }
            buy_land(D_001549d0[i].value);
            satisfied_net_criteria(3, i);
            break;
        case 0x37:
            if (i == player) {
                cancel_top_level_requesters(9);
                if (crequester.unknown2 == 0x63) {
                    crequester.unknown2 = 9;
                    crequester.unknown0 = 1;
                    close_irequesters();
                    fn_0001bc94(crequester.unknown2, 1);
                    crequester.unknown6 = 0;
                    set_mouse_sprite(1);
                    crequester.unknown8 = 0;
                    crequester.unknown9 = 0;
                    clear_req_indices();
                    pmenu.unknown7 = 1;
                }
            }
            break;
        case 0x3b:
            if (i == player) {
                if (crequester.unknown2 == 0x63 && &room_info[D_001549d0[i].value] > room_info) {
                    crequester.unknown2 = 6;
                    crequester.unknown0 = 0;
                    crequester.unknown4 = 0;
                    crequester.unknown6 = 0;
                    crequester.unknown8 = 0;
                    crequester.unknown9 = 0;
                    crequester.unknownA = 0;
                    close_irequesters();
                    fn_0001bc94(crequester.unknown2, 1);
                    set_mouse_sprite(1);
                    D_0015541b = 0;
                    D_00155419 = D_001549d0[i].value;
                    D_0015541d = D_001549d0[i].arg;
                    pmenu.unknown7 = 1;
                }
            }
            break;
        case 0x3c:
            front_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x3d:
            back_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x38:
            if (allhosps[i].f11f4) {
                fn_00029c4c(allhosps[i].f11f6, allhosps[i].f11f4, 1, i);
            } else if (D_001549d0[i].value) {
                req2_exit_employing(D_001549d0[i].value);
            }
            clear_build_objects(i);
            if (i == player) {
                if (!gamevar.unknown44) {
                    close_crequester(0);
                    close_crequester(0xb);
                    close_crequester(0xd);
                    close_crequester(6);
                    close_crequester(9);
                    close_crequester(0xa);
                    close_crequester(0xc);
                    close_crequester(5);
                    close_irequesters();
                    gamevar.unknown75 = 0;
                    gamevar.unknown77 = 0;
                    gamevar.unknown25 = 0;
                    gamevar.unknown1e = 0;
                    set_mouse_mode(0);
                    pmenu.unknown7 = 0;
                } else {
                    init_place_object(0x11, gamevar.unknown26, gamevar.unknown2f, 0);
                }
            }
            break;
        case 0x42:
            if (i == player) {
                Thing *obj;
                int kind = 0;
                u8 slot = 0;

                obj = &thing[D_001549d0[i].value];
                if (obj->owner == player) {
                    switch (obj->renderer) {
                    case 2:
                        slot = 1;
                        D_001581b2[slot].kind = 2;
                        D_001581b2[slot].f4 = 1;
                        D_001581b2[slot].f6 = 0;
                        kind = 0x22;
                        break;
                    case 4:
                        switch (obj->subtype) {
                        case 2:
                            slot = 2;
                            D_001581b2[slot].kind = 1;
                            kind = 0x21;
                            break;
                        default:
                            slot = 2;
                            D_001581b2[slot].kind = 0;
                            kind = 0x20;
                            break;
                        }
                        break;
                    case 6:
                        slot = 3;
                        D_001581b2[slot].kind = 3;
                        break;
                    }
                }
                if (slot) {
                    close_irequesters();
                    D_00155d70 = slot;
                    fn_0001bc94(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    set_mouse_sprite(1);
                }
                if (kind) {
                    int x = D_0015f414[slot].x;
                    int y = D_0015f414[slot].y;

                    open_new_window(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                    D_001553ec[slot] = window.unknown2ec - 1;
                    window.unknown2eb = 0;
                }
            }
            break;
        case 0x43:
            if (i == player) {
                close_query((u8)D_001549d0[i].value);
            }
            break;
        case 0x44:
            pick_up_object(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                play_sound(10, 0xba, -1, -1);
            }
            break;
        case 0x47:
            sack_staff(&thing[D_001549d0[i].value], D_001549d0[i].arg);
            if (i == player) {
                close_yes_no_requester(1);
                close_irequesters();
                set_mouse_mode(0);
                D_00188c7b = 0;
                play_sound(10, 0xaa, -1, -1);
            }
            break;
        case 0x45:
            edit_blueprint(i);
            break;
        case 0x48:
            if (D_001549d0[i].arg) {
                if (allhosps[i].f35 >= D_001549d0[i].value && allhosps[i].f35 > 0) {
                    pay_staff_bonus(&thing[D_001549d0[i].arg], D_001549d0[i].value, 0, 1, i);
                }
            } else {
                if (allhosps[i].f35 >= pay_staff_bonus(0, D_001549d0[i].value, 0, 0, i) && allhosps[i].f35 > 0) {
                    pay_staff_bonus(0, D_001549d0[i].value, 0, 1, i);
                }
            }
            break;
        case 0x46:
            D_00155d7d = 1;
            init_edit_room(&room_info[D_001549d0[i].value]);
            break;
        case 0x49:
            if (i == player) {
                if (crequester.unknown2 == 0x63) {
                    crequester.unknown2 = 0xd;
                    crequester.unknown0 = 1;
                    crequester.unknown4 = 0;
                    crequester.unknown6 = 0;
                    crequester.unknown8 = 0;
                    crequester.unknown9 = 0;
                    crequester.unknownA = 0;
                    close_irequesters();
                    fn_0001bc94(crequester.unknown2, 1);
                    set_mouse_sprite(1);
                    pmenu.unknown7 = 1;
                }
            }
            break;
        case 0x4a:
            allhosps[i].f1e2 = 10;
            allhosps[i].f1d9 = 100;
            allhosps[i].f1e5 = 0;
            D_00155e09 = allhosps[i].f1d9;
            if (i == player) {
                return_from_screen(0, 1, 1);
            }
            break;
        case 0x4b:
            if (D_001549d0[i].value & (1 << player)) {
                net_text_display(i);
            }
            net_text[i].unknown0 = 0;
            break;
        case 0x4c:
            net_text_delete(i);
            break;
        case 0x4d:
            net_text_keypress(D_001549d0[i].value, i);
            break;
        case 0x4e:
            net_text[i].unknown0 = 0;
            if (i == player) {
                D_00155402 = 1;
            }
            break;
        case 0x4f:
            if (allhosps[i].fc95 == 0) {
                allhosps[i].fc94 = D_001549d0[i].value;
            }
            break;
        case 0x50:
            alter_research(D_001549d0[i].value, D_001549d0[i].arg, i);
            break;
        case 0x51:
            queue_person_go_home(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                play_sound(10, 0xaa, -1, -1);
            }
            break;
        case 0x53:
            if (room_info[D_001549d0[i].value].f1e < 0x1e) {
                room_info[D_001549d0[i].value].f1e++;
            }
            break;
        case 0x54:
            if (room_info[D_001549d0[i].value].f1e) {
                room_info[D_001549d0[i].value].f1e--;
            }
            break;
        case 0x55:
            queue_person_go_reception(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x56:
            staff_join_new_employer(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                display_mode = 0;
                close_crequester(0xc);
                close_irequesters();
                window.unknown2eb = 0;
                pmenu.unknown7 = 0;
            }
            break;
        case 0x58:
            finish_composite_help();
            setup_town_bid();
            display_mode = 0xe;
            town.unknown2 = 0xb;
            D_00155430 = D_001549d0[i].value;
            break;
        case 0x59:
            plot_bid_money(i);
            break;
        case 0x5a:
            if (D_001549d0[i].arg) {
                increase_staff_salaries(&thing[D_001549d0[i].arg], D_001549d0[i].value, 0, i);
            } else {
                increase_staff_salaries(0, D_001549d0[i].value, 0, i);
            }
            break;
        case 0x5b:
            if (i == player) {
                close_irequesters();
                close_crequester(5);
                D_00155c6f = gamevar.unknown33;
                adjust_build_palette_for_shop(&room_info[allhosps[i].unknown11b7]);
                set_buyable_items(room_info[allhosps[i].unknown11b7].type);
                crequester.unknown2 = 10;
                fn_0001bc94(crequester.unknown2, 1);
                crequester.unknown8 = allhosps[i].f11bc;
                crequester.unknown9 = room_info[allhosps[i].unknown11b7].f8;
                crequester.unknown0 = 3;
                crequester.unknown3 = req_indices[0];
                pmenu.unknown7 = 0;
            }
            break;
        case 0x5c:
            if (i == player) {
                go_back_to_place_object_requester(D_001549d0[i].value);
            }
            break;
        case 0x5e:
            obj2 = &thing[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 1;
            break;
        case 0x5f:
            obj2 = &thing[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 4;
            break;
        case 0x60:
            obj2 = &thing[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 2;
            break;
        case 0x61:
            obj2 = &thing[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 8;
            break;
        case 0x62:
            if (i == player) {
                cancel_top_level_requesters(10);
                if (crequester.unknown2 == 0x63) {
                    crequester.unknown4 = 1;
                    set_buyable_items(3);
                    close_irequesters();
                    crequester.unknown2 = 10;
                    fn_0001bc94(crequester.unknown2, 1);
                    allhosps[i].unknown11b7 = 0;
                    crequester.unknown8 = 3;
                    crequester.unknown9 = 0;
                    crequester.unknown0 = 3;
                    crequester.unknown3 = req_indices[0];
                    pmenu.unknown7 = 1;
                }
            }
            break;
        case 0x63:
            if (i == player) {
                req11_sell_object(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case 0x64:
            if (i == player) {
                close_crequester(10);
                close_irequesters();
                set_mouse_sprite(D_001553e9);
                pmenu.unknown7 = 0;
            }
            break;
        case 0x65:
            mycontrol->unknown64f = D_0012b7c8[mycontrol->unknown650] - 1;
            break;
        case 0x66:
            change_balance(0x12, 5000, 0, i);
            allhosps[i].f3d += 5000;
            break;
        case 0x67:
            if (allhosps[i].f35 >= 5000 && allhosps[i].f35 > 0) {
                change_balance(0x13, -5000, 0, i);
                allhosps[i].f3d -= 5000;
            }
            break;
        case 0x57:
            obj1 = &thing[D_001549d0[i].value];
            allhosps[obj1->owner].rooms[obj1->style].unknown24++;
            if (obj1->f74 & 0x20) {
                allhosps[obj1->owner].f207 = 0;
            }
            refer_patient(obj1, D_001549d0[i].arg);
            if (i == player) {
                close_query(1);
                play_sound(10, 0xaa, -1, -1);
            }
            break;
        case 0x68:
            obj2 = &thing[D_001549d0[i].value];
            allhosps[i].f11f4 = D_001549d0[i].value;
            if (!(obj2->f8e & 0x20)) {
                if (fn_0002d11c(obj2)) {
                    interrupt_staff(obj2);
                    staff_picked_up(&thing[D_001549d0[i].value]);
                }
            }
            if (i == player) {
                close_query(2);
            }
            break;
        case 0x69:
            staff_picked_up(&thing[D_001549d0[i].value]);
            break;
        case 0x6a: {
            Thing *o = &thing[D_001549d0[i].value];

            switch (D_001549d0[i].argb[0]) {
            case 1:
                if (i == player) {
                    play_sound(10, 0xd3, mouse_dx, mouse_dy);
                }
                break;
            case 2:
                if (i == player) {
                    play_sound(10, 0xd4, mouse_dx, mouse_dy);
                }
                break;
            case 3:
                create_explosion(o->x.tile + (o->y.tile << 7), 0x902);
                break;
            default:
                if (i == player) {
                    play_sound(10, 0x3f, mouse_dx, mouse_dy);
                }
                break;
            }
            D_0015f8ed++;
            allhosps[i].fd29++;
            if (D_001549d0[i].argb[1]) {
                D_0015f8e9++;
                if (i == player) {
                    play_sound(10, 0x40, mouse_dx, mouse_dy);
                }
                create_numfx(0, ++allhosps[i].fd27, (o->y.tile << 7) + o->x.tile, color.unknown9, 0x20);
                place_litter(o->x.tile + (o->y.tile << 7), (s8)o->owner, 4);
                remove_thing(o);
            }
            break;
        }
        case 0x6b:
            obj2 = &thing[D_001549d0[i].value];
            if (obj2 > thing) {
                if (old_newrand() % (obj2->f8f + 1)) {
                    sack_staff(&thing[D_001549d0[i].value], 1);
                } else {
                    obj2->f8e &= 0xdf;
                    D_00158324 &= 0xfe;
                    obj2->unknown128 = 0;
                    reset_icon_value(&thing[D_001549d0[i].value], 9);
                }
                if (i == player) {
                    display_mode = 0;
                    close_crequester(0xc);
                    close_irequesters();
                    window.unknown2eb = 0;
                }
                pmenu.unknown7 = 0;
            }
            break;
        case 0x6c:
            obj2 = &thing[D_001549d0[i].value];
            if (obj2 > thing) {
                obj2->f8e &= 0xdf;
                D_00158324 &= 0xfe;
                obj2->unknown108 += D_001549d0[i].arg;
                obj2->unknown81 = 100;
                obj2->unknown128 = 0;
                reset_icon_value(&thing[D_001549d0[i].value], 9);
            }
            if (i == player) {
                display_mode = 0;
                close_crequester(0xc);
                close_irequesters();
                window.unknown2eb = 0;
                pmenu.unknown7 = 0;
            }
            break;
        case 0x6d:
            obj2 = &thing[D_001549d0[i].value];
            if (obj2 > thing) {
                obj2->f8e &= 0xdf;
                D_00158324 &= 0xfe;
                pay_staff_bonus(obj2, D_001549d0[i].arg, 0, 1, i);
                obj2->unknown81 = 100;
                obj2->unknown128 = 0;
                reset_icon_value(&thing[D_001549d0[i].value], 9);
            }
            if (i == player) {
                display_mode = 0;
                close_crequester(0xc);
                close_irequesters();
                window.unknown2eb = 0;
                pmenu.unknown7 = 0;
            }
            break;
        case 0x6e: {
            int dept = D_001549d0[i].arg;
            int rate = max_price_for_treatment(dept, i);

            if (allhosps[i].rooms[dept].unknown10 < 20000) {
                allhosps[i].rooms[dept].unknown10 += D_001549d0[i].value * 100;
                if (allhosps[i].rooms[dept].unknown10 < 5000) {
                    allhosps[i].rooms[dept].unknown10 = 5000;
                } else if (allhosps[i].rooms[dept].unknown10 > 20000) {
                    allhosps[i].rooms[dept].unknown10 = 20000;
                }
                allhosps[i].rooms[dept].unknownC = allhosps[i].rooms[dept].unknown10 * rate / 10000;
            }
            break;
        }
        case 0x6f: {
            int dept = D_001549d0[i].arg;
            int rate = max_price_for_treatment(dept, i);

            if (allhosps[i].rooms[dept].unknown10 > 5000) {
                allhosps[i].rooms[dept].unknown10 -= D_001549d0[i].value * 100;
                if (allhosps[i].rooms[dept].unknown10 < 5000) {
                    allhosps[i].rooms[dept].unknown10 = 5000;
                } else if (allhosps[i].rooms[dept].unknown10 > 20000) {
                    allhosps[i].rooms[dept].unknown10 = 20000;
                }
                allhosps[i].rooms[dept].unknownC = allhosps[i].rooms[dept].unknown10 * rate / 10000;
            }
            break;
        }
        case 0x70:
            if (D_001553c7 == player) {
                D_00188c81 = D_00188c83 = D_00188c7b = D_00188c7c = 0;
                return_from_screen(D_001549d0[i].value, 1, 1);
            }
            break;
        case 0x71:
            switch (D_001549d0[i].value) {
            case 1:
                if (D_001549d0[i].arg <= allhosps[i].f219) {
                    allhosps[i].f21b = D_001549d0[i].arg;
                } else {
                    allhosps[i].f21b = allhosps[i].f219;
                }
                break;
            case 2:
                if (D_001549d0[i].arg >= allhosps[i].f21b) {
                    allhosps[i].f219 = D_001549d0[i].arg;
                } else {
                    allhosps[i].f219 = allhosps[i].f21b;
                }
                break;
            case 3:
                allhosps[i].f217 = D_001549d0[i].arg;
                break;
            case 4:
                allhosps[i].f21d = D_001549d0[i].arg;
                break;
            case 5:
                allhosps[i].f21f = D_001549d0[i].arg;
                break;
            }
            break;
        case 0x72:
            if (D_001549d0[i].value == (statvar.unknownfd > 1 ? allhosps[i].unknown1256 : D_00155376)) {
                if (statvar.unknownfd > 1) {
                    allhosps[i].unknown1256 = 0;
                } else {
                    D_00155376 = 0;
                }
            } else {
                if (statvar.unknownfd > 1) {
                    allhosps[i].unknown1256 = D_001549d0[i].value;
                } else {
                    D_00155376 = D_001549d0[i].value;
                }
            }
            break;
        case 0x73:
            adjust_handyman_values(D_001549d0[i].value, D_001549d0[i].argb[0]);
            break;
        case 0x74:
            obj1 = &thing[D_001549d0[i].value];
            obj1->unknown171 = 0;
            allhosps[obj1->owner].rooms[obj1->style].unknown24++;
            person_go_home(obj1);
            obj1->f75 |= 0x80;
            if (obj1->f74 & 0x20) {
                allhosps[obj1->owner].f207 = 0;
            }
            break;
        case 0x76:
            thingpage_request(&allhosps[i].ff6d, D_001549d0[i].value, 0xff);
            set_icon_value(&thing[D_001549d0[i].value], 0x1c);
            break;
        case 0x77:
            if (i == player) {
                if (D_00155e1d) {
                    return_from_screen(0, 1, 1);
                }
                D_00155d7c = D_0015536e;
                display_mode = 0xb;
                close_irequesters();
                D_00155d7b = crequester.unknown2;
                if (crequester.unknown2 != 0x63) {
                    close_crequester(crequester.unknown2);
                }
                crequester.unknown2 = 4;
                fn_0001bc94(crequester.unknown2, 1);
                pmenu.unknown7 = 1;
            }
            break;
        case 0x78:
            if (i == player) {
                close_yes_no_requester((u8)D_001549d0[i].value);
                if (D_00155d69) {
                    set_mouse_mode((u8)D_00155d69);
                    D_00188c7b = 0;
                }
            }
            break;
        case 0x79:
            close_yes_no_requester(0);
            clear_build_objects(i);
            delete_blueprint_room(i);
            D_001549d0[i].type = 0x38;
            D_001549d0[i].arg = D_0015536e;
            goto dispatch;
        case 0x7b:
            fn_0002994c(i);
            D_001549d0[i].type = 0x38;
            D_001549d0[i].value = 0;
            goto dispatch;
        case 0x7c:
            refer_patient(&thing[D_001549d0[i].value], D_001549d0[i].arg);
            break;
        case 0x7d: {
            u8 quiet = 0;

            switch (D_001549d0[i].value) {
            case 0:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->unknown171 = 0;
                    allhosps[obj1->owner].rooms[obj1->style].unknown24++;
                    person_go_home(obj1);
                    obj1->unknown118 |= 0x10;
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            case 1:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    reset_icon_value(obj1, 0x1b);
                    obj1->f74 |= 0x80;
                    obj1->timer = D_0015543a;
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            case 2:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->unknown171 = 0x2c;
                    obj1->f74 &= 0xef;
                    reset_icon_value(obj1, 0x1b);
                    obj1->kind = 0x26;
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            case 0xa:
                count_and_flag_contagious_peeps(allhosps[i].f20b, 2, i);
                allhosps[i].f20b = 0;
                allhosps[i].f20d = 0;
                change_balance(0x21, -allhosps[i].f203, 0, i);
                reputation_affected(0, 0xe, 0, i, 0);
                if (i == player) {
                    play_sound(10, 0xce, -1, -1);
                }
                break;
            case 0xb:
                count_and_flag_contagious_peeps(allhosps[i].f20b, 1, i);
                allhosps[i].f20d = 1;
                allhosps[i].f207 = 100;
                if (i == player) {
                    play_sound(10, 0xcd, -1, -1);
                }
                quiet = D_001549d0[i].arg;
                break;
            case 0x15:
                allhosps[i].f1fa = 0;
                break;
            case 0x14:
                allhosps[i].f1f3 = actual_game_turn + 1500;
                allhosps[i].f1fa = 2;
                break;
            case 0x1e:
                if (statvar.unknownfd > 1) {
                    restore_popup(2);
                }
                break;
            case 0x1f:
                give_emergency_stats(D_0015f5c2[player].f0, i);
                D_001549d0[i].type = 0x4a;
                goto dispatch;
            case 0x28:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->unknown171 = 0;
                    allhosps[obj1->owner].rooms[obj1->style].unknown24++;
                    person_go_home(obj1);
                    obj1->unknown118 |= 8;
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            case 0x29:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    reset_icon_value(obj1, 0x1b);
                    interrupt_complex(obj1);
                    best_guess_treatment(obj1);
                    next_treatment_now_setup(obj1, 0);
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            case 0x2a:
                obj1 = &thing[D_001549d0[i].arg];
                if (obj1->renderer == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    reset_icon_value(obj1, 0x1b);
                    obj1->f74 |= 0x80;
                    obj1->timer = D_0015543a;
                }
                if (obj1->renderer == 2) {
                    obj1->unknown91 = 0;
                }
                break;
            }
            if (i == player && quiet == 0) {
                restore_display_screen();
                D_001553d7 = D_00153f34;
                D_001553c8 = 1;
                D_00156910 = 1;
            }
            break;
        }
        case 0x7e:
            finish_composite_help();
            switch (D_001549d0[i].value) {
            case 0:
                break;
            case 1:
                if (allhosps[i].f2b == 2) {
                    Thing *o;

                    for (o = thing + 1; o < end_things; o++) {
                        if (o->owner == i) {
                            switch (o->renderer) {
                            case 6:
                                if (objects[o->subtype].unknown1e) {
                                    o->mode = o->value * 2;
                                    machine_disaster(o);
                                }
                                break;
                            case 4:
                                sack_staff(o, 0);
                                break;
                            case 2:
                                if (!(map[(s16)o->x.tile + ((s16)o->y.tile << 7)].unknown7 & 0x10)) {
                                    o->f74 &= 0xdf;
                                    o->unknown174 = 0;
                                    o->f75 |= 1;
                                }
                                break;
                            }
                        }
                    }
                    allhosps[i].f2b = 0;
                    D_001549d0[i].checksum = 0;
                    if (i == player) {
                        close_yes_no_requester(0);
                        LbNetwork_Stop();
                        quit_game = 1;
                        D_0015542c = 100000;
                        if (D_00155e4d) {
                            allhosps[player].f2b = 0;
                            level_number = 0xc;
                            D_0015542a = 6;
                            winlose.unknown2 = D_0015542a;
                        } else {
                            D_0015542a = 2;
                            winlose.unknown2 = D_0015542a;
                        }
                        play_winlose();
                        statvar.unknownfd = 1;
                        player = 0;
                    }
                    fn_0007fbb8((s8)i);
                }
                break;
            case 2:
                break;
            default:
                break;
            }
            break;
        case 0x7f:
            obj1 = &thing[D_001549d0[i].value];
            obj1->f74 &= 0xef;
            reset_icon_value(obj1, 0x1b);
            obj1->f74 |= 0x80;
            obj1->timer = D_0015543a;
            break;
        case 0x80:
            obj1 = &thing[D_001549d0[i].value];
            obj1->unknown171 = 0x2c;
            obj1->f74 &= 0xef;
            reset_icon_value(obj1, 0x1b);
            obj1->kind = 0x26;
            break;
        case 0x81:
            obj2 = &thing[D_001549d0[i].value];
            if (obj2 > thing) {
                obj2->f93 = 1 << (u8)i;
            }
            break;
        case 0x82:
            obj1 = &thing[D_001549d0[i].value];
            if (obj1->f74 & 1) {
                if (i == player) {
                    play_sound(10, 0xdf, -1, -1);
                }
                cancel_vaccination_request(obj1);
                if (obj1->f74 & 0x20) {
                    set_icon_value(obj1, 0xd);
                }
            } else {
                if (i == player) {
                    play_sound(10, 0xde, -1, -1);
                }
                request_vaccination(obj1);
            }
            break;
        case 0x83:
            room_info[allhosps[i].unknown11b7].f3 = 4;
            room_info[allhosps[i].unknown11b7].f22 = 0;
            if (i == player) {
                setup_place_object_requester();
                set_mouse_mode(0);
            }
            break;
        case 0x84:
            if (i == player) {
                D_0015541b = 0;
                D_001553fd = 0;
                display_mode = 0;
                close_irequesters();
                close_crequester(6);
                window.unknown2eb = 0;
                set_mouse_mode(0);
                pmenu.unknown7 = 0;
            }
            break;
        case 0x85:
            break;
        case 0x86:
            force_peep_to_be_cured(&thing[D_001549d0[i].value]);
            break;
        case 0x87:
            if (D_001553c7 == player) {
                D_00188c81 = D_00188c83 = D_00188c7b = D_00188c7c = 0;
                return_from_screen(D_001549d0[i].value, 1, 1);
                if (i == player) {
                    Thing *obj;
                    int kind = 0;
                    u8 slot = 0;

                    obj = &thing[D_001549d0[i].arg];
                    if (obj->owner == player) {
                        switch (obj->renderer) {
                        case 2:
                            slot = 1;
                            D_001581b2[slot].kind = 2;
                            D_001581b2[slot].f4 = 1;
                            D_001581b2[slot].f6 = 0;
                            kind = 0x22;
                            break;
                        case 4:
                            switch (obj->subtype) {
                            case 2:
                                slot = 2;
                                D_001581b2[slot].kind = 1;
                                kind = 0x21;
                                break;
                            default:
                                slot = 2;
                                D_001581b2[slot].kind = 0;
                                kind = 0x20;
                                break;
                            }
                            break;
                        case 6:
                            slot = 3;
                            D_001581b2[slot].kind = 3;
                            break;
                        }
                    }
                    if (slot) {
                        close_irequesters();
                        cancel_top_level_requesters(0x63);
                        if (crequester.unknown2 != 0x63) {
                            break;
                        }
                        D_00155d70 = slot;
                        fn_0001bc94(D_001581b2[slot].kind, 1);
                        D_001581b2[slot].value = D_001549d0[i].arg;
                        set_mouse_sprite(1);
                    }
                    if (kind) {
                        int x = D_0015f414[slot].x;
                        int y = D_0015f414[slot].y;

                        open_new_window(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                        D_001553ec[slot] = window.unknown2ec - 1;
                        window.unknown2eb = 0;
                    }
                }
            }
            break;
        case 0x88:
            if (i == player) {
                Thing *obj;
                int kind = 0;
                u8 slot = 0;

                close_yes_no_requester(1);
                close_irequesters();
                if (D_00155d69) {
                    set_mouse_mode((u8)D_00155d69);
                    D_00188c7b = 0;
                }
                obj = &thing[D_001549d0[i].value];
                if (obj->owner == player) {
                    switch (obj->subtype) {
                    case 2:
                        slot = 2;
                        D_001581b2[slot].kind = 1;
                        kind = 0x21;
                        break;
                    default:
                        slot = 2;
                        D_001581b2[slot].kind = 0;
                        kind = 0x20;
                        break;
                    }
                }
                if (slot) {
                    close_irequesters();
                    D_00155d70 = slot;
                    fn_0001bc94(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    set_mouse_sprite(1);
                }
                if (kind) {
                    int x = D_0015f414[slot].x;
                    int y = D_0015f414[slot].y;

                    open_new_window(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                    D_001553ec[slot] = window.unknown2ec - 1;
                    window.unknown2eb = 0;
                }
                play_sound(10, 0x42, -1, -1);
            }
            break;
        case 0x89:
            mycontrol->unknown650 = 0xb;
            mycontrol->unknown64f = D_0012b7c8[mycontrol->unknown650] - 2;
            break;
        case 0x8a:
            if (i == player) {
                close_yes_no_requester(0);
            }
            replace_machine(&thing[D_001549d0[i].value]);
            break;
        case 0x8b:
            allhosps[i].unknown1257 = 0;
            if (statvar.unknownfd <= 1) {
                allhosps[i].unknown1256 = 1;
            }
            if (i != player) {
                announce_this(0xc, i);
            }
            break;
        case 0x8c:
            if (i == player) {
                close_yes_no_requester((u8)D_001549d0[i].value);
                if (D_00155d69) {
                    set_mouse_mode((u8)D_00155d69);
                    D_00188c7b = 0;
                }
                if (D_00155d57 < 10) {
                    D_00155dc6[D_00155d57] = 1;
                }
                check_save_games();
            }
            break;
        case 0x90:
            if (i == player) {
                D_00155e4c = 0;
            } else {
                D_00155e4c = 1;
            }
            break;
        case 0x91:
            close_yes_no_requester(0);
            fn_00022a58();
            break;
        default:
            break;
        }
        D_00154a0c[i].type = D_001549d0[i].type;
        D_00154a0c[i].value = D_001549d0[i].value;
        D_00154a0c[i].arg = D_001549d0[i].arg;
        D_00154a0c[i].checksum = D_001549d0[i].checksum;
        D_001549d0[i].type = 0;
        D_001549d0[i].value = 0;
        D_001549d0[i].arg = 0;
    }
    D_001553c7 = player;
    fn_000228e0();
}

// FUNCTION: 0x00021700
s16 save_game_valid(const s8 slot)
{
    char buf[64];
    int h;

    sprintf(buf, D_001202ba, D_001563c5, statvar.f6c, slot);
    if (LbFileLengthRnc(buf) == statvar.unknown0) {
        h = LbFileOpen(buf, 2);
        if (h != -1) {
            LbFileRead(h, &version, 2);
            if (version == 3) {
                LbFileRead(h, &level_number, 1);
                LbFileRead(h, balance_id, 10);
                LbFileClose(h);
                load_balance((u16)level_number);
                return 1;
            } else {
                LbFileClose(h);
                D_00155e1e = 0x1c;
                D_00155e1f = 0x46;
            }
        } else {
            D_00155e1e = 0x1c;
            D_00155e1f = 0x46;
        }
    } else {
        D_00155e1e = 0x1c;
        D_00155e1f = 0x46;
    }
    return 0;
}

// FUNCTION: 0x00021818
u16 fn_00021818(s8 slot, s8 player)
{
    s32 cell;
    Thing *w;
    char buf[64];
    int h;
    u16 i;
    u16 ok;
    int tries;

    if (statvar.unknownfd > 1 && player == -1) {
        return 0;
    }
    if (statvar.unknownfd <= 1) {
        if (!fn_00022854(1)) {
            return 0;
        }
    }
    if (D_0015f3f5 == SCREEN_MODE_1) {
        find_cell_at_point(window.unknown2eb, window.f90[window.unknown2eb], window.f9c[window.unknown2eb]);
        cell = info.unknown64;
        window.unknown30 = 0;
        window.unknown48 = lbScreenModeInfo[SCREEN_MODE_13].width;
        window.unknown3c = 0;
        window.unknown54 = lbScreenModeInfo[SCREEN_MODE_13].height;
        D_0015f3f8 = 0x40;
        D_0015f3f7 = 0x20;
        window.unknown90 = (window.unknown30 + window.unknown48) / 2;
        window.unknown9c = (window.unknown3c + window.unknown54) / 2;
        goto_cell_in_window(window.unknown2eb, cell);
    }
    setup_addresses();
    for (w = thing + 1; w < end_things; w++) {
        if ((w->renderer == 4 || w->renderer == 2) && w->room > 0) {
            w->mode = w->room - room_info;
        }
    }
    if (player == -1) {
        sprintf(buf, D_001202c1, D_001563c5, statvar.f6c, slot);
        h = LbFileOpen(buf, 0);
        if (h != -1) {
            for (i = 0; D_001290d4[i].addr; i++) {
                LbFileWrite(h, (void *)D_001290d4[i].addr, D_001290d4[i].size);
            }
            LbFileClose(h);
            D_00155e1e = 0x18;
            D_00155e1f = 3;
            save_player_details();
            ok = 1;
        } else {
            D_00155e1e = 0x19;
            D_00155e1f = 0x14;
            ok = 0;
        }
    } else {
        ok = 0;
        tries = 0;
        do {
        } while (LbNetwork_Send(player, D_0015fe86, 0x400) == -1 && tries++ < 8);
        if (tries < 8) {
            ok = 1;
            for (i = 0; D_001290d4[i].addr; i++) {
                if (fn_0001c030(i)) {
                    if (!fn_0001c318(player, D_001290d4[i].addr, D_001290d4[i].size)) {
                        ok = 0;
                        break;
                    }
                }
            }
        }
    }
    for (w = thing + 1; w < end_things; w++) {
        if ((w->renderer == 4 || w->renderer == 2) && w->room > 0) {
            w->room = &room_info[w->mode];
        }
    }
    if (D_0015f3f5 == SCREEN_MODE_1) {
        window.unknown30 = 0;
        window.unknown48 = lbScreenModeInfo[SCREEN_MODE_1].width;
        window.unknown3c = 0;
        window.unknown54 = lbScreenModeInfo[SCREEN_MODE_1].height;
        D_0015f3f8 = 0x20;
        D_0015f3f7 = 0x10;
        window.unknown2c0 = 0;
        window.unknown90 = (window.unknown30 + window.unknown48) / 2;
        window.unknown9c = (window.unknown3c + window.unknown54) / 2;
        window.unknown2eb = 0;
        goto_cell_in_window(window.unknown2eb, cell);
    }
    return ok;
}

// FUNCTION: 0x00021c38
s16 fn_00021c38(s8 slot, s8 port)
{
    u8 owner;
    u16 ok;
    char buf[64];
    int h;
    u16 i;
    s32 cell;
    Thing *w;
    int tries;

    owner = myhosp->f29;
    if (statvar.unknownfd > 1 && port == -1) {
        return 0;
    }
    if (statvar.unknownfd <= 1) {
        if (save_game_valid(slot) == 0) {
            return 0;
        }
    }
    if (port == -1) {
        check_autosave();
        close_irequesters();
        if (crequester.unknown2 != 0x63) {
            close_crequester(crequester.unknown2);
        }
        if (load_level(level_number, 1)) {
            ok = 1;
            setup_addresses();
            sprintf(buf, D_001202c8, D_001563c5, statvar.f6c, slot);
            h = LbFileOpen(buf, 2);
            if (h != -1) {
                for (i = 0; D_001290d4[i].addr; i++) {
                    LbFileRead(h, (void *)D_001290d4[i].addr, D_001290d4[i].size);
                }
                LbFileClose(h);
            }
            if (D_0015f3f5 == SCREEN_MODE_1) {
                window.unknown30 = 0;
                window.unknown48 = lbScreenModeInfo[SCREEN_MODE_13].width;
                window.unknown3c = 0;
                window.unknown54 = lbScreenModeInfo[SCREEN_MODE_13].height;
                D_0015f3f8 = 0x40;
                D_0015f3f7 = 0x20;
                window.unknown90 = (window.unknown30 + window.unknown48) / 2;
                window.unknown9c = (window.unknown3c + window.unknown54) / 2;
                find_cell_at_point(window.unknown2eb, window.f90[window.unknown2eb], window.f9c[window.unknown2eb]);
                cell = info.unknown64;
                window.unknown30 = 0;
                window.unknown48 = lbScreenModeInfo[SCREEN_MODE_1].width;
                window.unknown3c = 0;
                window.unknown54 = lbScreenModeInfo[SCREEN_MODE_1].height;
                window.unknown90 = (window.unknown30 + window.unknown48) / 2;
                window.unknown9c = (window.unknown3c + window.unknown54) / 2;
                D_0015f3f8 = 0x20;
                D_0015f3f7 = 0x10;
                goto_cell_in_window(window.unknown2eb, cell);
            }
            for (w = thing + 1; w < end_things; w++) {
                if ((w->renderer == 2 || w->renderer == 4) && w->room > 0) {
                    w->room = &room_info[w->mode];
                }
            }
            recreate_map(1);
            D_001553c8 = 1;
            reset_advisor();
            myhosp = &allhosps[player];
            if (statvar.unknownfd > 1) {
                myhosp->f29 = owner;
            }
            D_00155444 = get_next_illness(1, 0);
            free_level();
            return 1;
        }
    } else {
        ok = 0;
        tries = 0;
        do {
        } while (LbNetwork_Receive(modem_dev.unknown1c, D_0015fe86, 0x400) != 0x400 && tries++ < 8);
        if (tries < 8) {
            u8 found;

            found = 0;
            ok = 1;
            setup_addresses();
            for (i = 0; D_001290d4[i].addr; i++) {
                if (fn_0001c030(i)) {
                    if (!fn_0001c318(-1, D_001290d4[i].addr, D_001290d4[i].size)) {
                        ok = 0;
                        break;
                    }
                    if (D_001290d4[i].addr == (u32)thing) {
                        found = 1;
                    }
                }
            }
            if (found) {
                for (w = thing + 1; w < end_things; w++) {
                    if ((w->renderer == 2 || w->renderer == 4) && w->room > 0) {
                        w->room = &room_info[w->mode];
                    }
                }
            }
            D_001553c8 = 1;
            reset_advisor();
            myhosp = &allhosps[player];
            if (statvar.unknownfd > 1) {
                myhosp->f29 = owner;
            }
        }
        return ok;
    }
    return 0;
}

// FUNCTION: 0x00022120
void setup_addresses(void)
{
    int i;

    D_001290d4[3].addr = (u32)map;
    D_001290d4[4].addr = (u32)thing;
    D_001290d4[5].addr = (u32)room_info;
    D_001290d4[6].addr = (u32)pagers;
    D_001290d4[7].addr = (u32)mycontrol;
    D_001290d4[8].addr = (u32)listhead;
    D_001290d4[9].addr = (u32)allhosps;
    D_001290d4[10].addr = (u32)playercall;
    D_001290d4[11].addr = (u32)for_sale;
    if (statvar.unknownfd > 1) {
        D_0016028c = 0;
        for (i = 0; D_001290d4[i].addr; i++) {
            if (fn_0001c030(i)) {
                D_0016028c += D_001290d4[i].size;
            }
        }
        if (modem_dev.unknown1c == player) {
            fn_0001c234();
        }
    }
}

// FUNCTION: 0x000221fc
s8 load_level(const u8 lvl, const u8 full)
{
    char buf[144];
    RoomInfo *r;
    u16 i;
    u16 j;

    if (lvl) {
        if (D_00155db1 == lvl) {
            return 1;
        }
        if (D_00155db1 > 0) {
            free_level();
        }
    }
    clear_sound_heap();
    D_00155dbe = cache.Allocate(0x2806c, 0x63);
    if (D_00155dbe) {
        level = (Level *)fn_00022b60(D_00155dbe);
    } else {
        return 0;
    }
    if (install_type < 3) {
        sprintf(buf, D_001202cf, cd_install_path, lvl);
        check_cd_available(buf);
    } else {
        sprintf(buf, D_001202e2, lvl);
    }
    if (LbFileLoadAt(buf, level) == LbFileLengthRnc(buf)) {
        if (!full) {
            D_00155db1 = lvl;
            recreate_map(1);
            return 1;
        }
        if (level->f0 < 1 || level->f0 > 4) {
            level->f0 = 1;
        }
        r = room_info + 1;
        for (i = 0; i < 4; i++) {
            if (D_001609c4[i].active || (statvar.unknownfd <= 1 && i == player)) {
                for (j = 0; j < 4; j++) {
                    for (; r < end_room_info; r++) {
                        if (r->type == 0) {
                            r->type = 1;
                            switch (j) {
                            case 0:
                                r->pos = r->unknown4 = r->unknown6 = level->f2[i][0];
                                break;
                            case 1:
                                r->pos = r->unknown4 = r->unknown6 = level->f2[i][1];
                                break;
                            case 2:
                                r->pos = r->unknown4 = r->unknown6 = level->f2[i][2];
                                break;
                            case 3:
                                r->pos = r->unknown4 = r->unknown6 = level->f2[i][3];
                                break;
                            }
                            r->owner = i;
                            allhosps[i].unknown1cf[j] = r - room_info;
                            break;
                        }
                    }
                }
                allhosps[i].f1d7 = level->f2802c[i];
            }
            goto_column_row_in_window(window.unknown2eb, level->unknown28024[player] % 128, level->unknown28024[player] / 128);
            D_00155372 = level->f28034;
            D_00155374 = level->f28036;
        }
        for (i = 0; i < 4; i++) {
            LbDrawBoxCoords(&allhosps[i].unknown1cf, &allhosps[modem_dev.unknown1c].unknown1cf, 8);
        }
        recreate_map(0);
        add_static_shadows();
        D_001553c8 = 1;
        D_00155db1 = lvl;
        return 1;
    }
    return 0;
}

// FUNCTION: 0x00022764
void free_level(void)
{
    D_00155db1 = 0;
    cache.Free(D_00155dbe);
}

// FUNCTION: 0x00022790
u32 fn_00022790(void)
{
    u32 sum;
    int j;
    int i;

    sum = 0;
    sum = seed;
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active) {
            for (j = 0; j < 20; j++) {
                sum += allhosps[i].f11c2[j];
            }
        }
    }
    return sum;
}

// FUNCTION: 0x00022820
void check_autosave(void)
{
    D_0015532d[3] = 0;
    D_00155333 = 0;
    D_00155334 = 1;
    D_0015532d[2] = 0;
}

// NONMATCHING: frame layout (target: level, return value, h, then buf; ours puts h and the return value below buf)
// FUNCTION: 0x000229b0
s32 read_level_number(s32 n)
{
    int h;
    u8 level;
    char buf[64];

    sprintf(buf, D_001202f3, D_001563c5, statvar.f6c, n);
    h = LbFileOpen(buf, 2);
    if (h == -1) {
        return -1;
    }
    if (LbFileSeek(h, 2, 0) != -1) {
        if (LbFileRead(h, &level, 2) != -1) {
            LbFileClose(h);
            return level;
        }
    }
    LbFileClose(h);
    return -1;
}

// FUNCTION: 0x00022a58
void fn_00022a58(void)
{
    if (statvar.unknownfd <= 1) {
        check_autosave();
        reset_advisor();
        close_irequesters();
        if (crequester.unknown2 != 0x63) {
            close_crequester(crequester.unknown2);
        }
        D_00156910 = 1;
        fn_00090804((u16)level_number, 1);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        goto_column_row_in_window(window.unknown2eb, level->unknown28024[player] % 128, level->unknown28024[player] / 128);
        D_00155e14 = 1;
        draw_it();
        D_001553dc = D_00153f34;
        init_ambients();
    }
}

// FUNCTION: 0x00022b60
void *fn_00022b60(TbCacheUsedBlock *block)
{
    return (u8 *)block + 0x14;
}
