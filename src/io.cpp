#include "hosp.h"

/*
 * Info bar entries of the in-game position table, read through resvars.coords.
 */
struct Struct_0015f410 {
    u8 unknown0[0x570];
    // 0x570: Info bar text x.
    int info_text_x;
    u8 unknown574[0x10];
    // 0x584: Info bar first line y.
    int info_text_y;
    u8 unknown588[0xc];
    // 0x594: Extra y of a one-line message.
    int info_message_dy;
};

/*
 * The misc_text string table (language section 2) read as a struct.
 */
struct Struct_001602a0 {
    u8 unknown0[0x6c];
    // 0x6c: String 27, "Game Out Of Sync".
    int out_of_sync;
};

/*
 * A tracked-window slot (D_001581b2[]): which requester and thing it shows,
 * and the patient window's view state.
 */
struct Slot {
    // 0x0: Requester shown (REQ_).
    u8 kind;
    u8 unknown1[0x1];
    // 0x2: Thing shown.
    u16 value;
    // 0x4: Patient window: nonzero shows the health graph, 0 the diagnosis.
    u16 show_graph;
    // 0x6: Patient window: frames before the graph/diagnosis toggle works
    // again.
    u8 toggle_delay;
    u8 unknown7[0x4];
};

struct Rect {
    int x0;
    int y0;
    int x1;
    int y1;
};

/*
 * A player's popup state (D_0015f5c2[player]), as emerg.cpp's PlayerPopup.
 */
struct Stride6c {
    // 0x0: The emergency call the player's popup is about (matched against
    // gamevar.emergency_calls[].call).
    u16 call;
    u8 unknown2[0x6a];
};

extern "C" {
extern u8 D_00160508;

extern Event D_001549d0[];
extern Event D_00154a0c[];
extern MemRegion D_001290d4[];
extern u32 D_00160288;
extern u16 D_0015fe86[];
extern u32 D_0015fe78, D_0015fe7c, D_0015fe80;
extern u32 D_0016028c, D_00160290;
extern u8 D_00160294;
extern Struct_001602a0 *misc_text;
extern u8 D_001581ee;
extern s32 D_001581e8;
extern u8 D_001581ef, D_001581f0;
extern u8 *palette;
extern u8 D_00156910;
extern u8 last_saved;
extern u8 need_a_save;
extern u8 D_00158166;
extern Slot D_001581b2[];
extern Stride6c D_0015f5c2[];

int LbScreenLock(void);
u32 check_sum(void);
u8 fn_00086b60(void);
void fn_0008debc(char *s);
u8 is_player_building(void);
void check_autosave(void);
void restart_level(void);
void find_other_player(int me);
void fn_0002994c(u8 a);
u8 fn_0002d11c(Thing *staff);
int is_checksummed_region(int region);
void draw_sync_progress(int percent);
u16 checksum16(u16 *p, int len);
void checksum_memory_regions(void);
int sync_memory_region(s8 port, u32 base, int len);
void resync_network_game(void);
void *fn_00022b60(TbCacheUsedBlock *block);

}

// Returns 1 if save region region (D_001290d4 index) is checksummed and
// synchronised in network games, else 0.
// FUNCTION: 0x0001c030
int is_checksummed_region(int region)
{
    if (!(region >= 0x11 || region == 0xc || region == 0xd || region == 7 || region == 9)) {
        return 1;
    }
    return 0;
}

// Shows the network resynchronisation message with percent done in the
// panel's info bar (game view only).
// FUNCTION: 0x0001c084
void draw_sync_progress(int percent)
{
    u8 was_locked = lbDisplay.wscreen != 0;

    if (display_mode == DM_GAME) {
        char buf[32];

        if (!was_locked) {
            while (LbScreenLock() != 1) {
            }
        }
        sprintf(buf, "%s (%d%%)", misc_text->out_of_sync, percent);
        clear_panel_info_bar();
        LbTextSetWindow(panel->x, panel->y, lbDisplay.physical_screen_width - panel->x, lbDisplay.physical_screen_height - panel->y);
        set_font_from_cache(0x2c);
        LbTextDraw(resvars.coords_Struct_0015f410p->info_text_x, resvars.coords_Struct_0015f410p->info_text_y + resvars.coords_Struct_0015f410p->info_message_dy, buf);
        set_all_refresh_zones_in_window(0, lbDisplay.physical_screen_height - 0x5c, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        LbScreenUnlock();
        special_refresh();
        if (was_locked) {
            while (LbScreenLock() != 1) {
            }
        }
    }
}

// Returns the ones' complement 16-bit checksum of the len bytes at p.
// FUNCTION: 0x0001c1b4
u16 checksum16(u16 *p, int len)
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

// Records the checksum of each 1 KB block of the checksummed save regions in
// D_0015fe86 (at most 512), for resynchronising network games.
// FUNCTION: 0x0001c234
void checksum_memory_regions(void)
{
    int i;
    u32 p;
    u32 base;
    u32 chunk;
    u32 size;

    D_00160288 = 0;
    for (i = 0; D_001290d4[i].addr != 0; i++) {
        if (is_checksummed_region(i)) {
            p = base = D_001290d4[i].addr;
            size = D_001290d4[i].size;
            while (base + size > p) {
                chunk = 0x400;
                if (p + chunk > base + size) {
                    chunk = base + size - p;
                }
                if (D_00160288 < 0x200) {
                    D_0015fe86[D_00160288++] = checksum16((u16 *)p, chunk);
                }
                p += chunk;
            }
        }
    }
    D_00160288 <<= 1;
}

// Resynchronises save region base (len bytes) with a network player. With
// port negative, this machine receives: it asks the host (D_00160508) for
// each 1 KB block whose checksum differs from the host's and reads it in.
// Otherwise it sends the blocks port asks for. Shows progress. Returns 1, or
// 0 if the network failed.
// FUNCTION: 0x0001c318
int sync_memory_region(s8 port, u32 base, int len)
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
            if (D_0015fe86[D_0015fe78++] != checksum16((u16 *)(base + off), chunk)) {
                tries = 0;
                do {
                } while (LbNetwork_Send(D_00160508, &off, 4) == -1 && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                tries = 0;
                do {
                } while (LbNetwork_Receive(D_00160508, (void *)(base + off), chunk) != chunk && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                if ((D_0015fe7c++ & 0xf) == 0) {
                    draw_sync_progress((D_00160290 + chunk) * 100 / D_0016028c);
                }
            }
            off += chunk;
            D_00160290 += chunk;
        }
        tries = 0;
        off = -1;
        do {
        } while (LbNetwork_Send(D_00160508, &off, 4) == -1 && tries++ < 8);
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
                draw_sync_progress((D_00160290 + end) * 100 / D_0016028c);
            }
        }
        D_00160290 += len;
        draw_sync_progress(D_00160290 * 100 / D_0016028c);
    }
    D_0015fe80++;
    return 1;
}

// After the players' checksums disagree: the host sends its game state to
// each player whose checksum differs and then releases everyone; the others
// receive it if theirs differs and wait for the host.
// FUNCTION: 0x0001c5e4
void resync_network_game(void)
{
    int value;
    int i;
    int tries;

    if (D_00160508 == player) {
        value = 1;
        for (i = 0; i < 4; i++) {
            if (D_001549d0[i].checksum != 0) {
                if (D_001549d0[player].checksum != D_001549d0[i].checksum) {
                    draw_sync_progress(0);
                    D_00160290 = 0;
                    tries = 0;
                    do {
                    } while (LbNetwork_Send(i, &value, 4) == -1 && tries++ < 8);
                    if (tries >= 8) {
                        return;
                    }
                    if (!save_game(0, i)) {
                        return;
                    }
                    draw_sync_progress(100);
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
        if (D_001549d0[player].checksum != D_001549d0[D_00160508].checksum) {
            draw_sync_progress(0);
            D_00160290 = 0;
            value = 0;
            while (value == 0) {
                LbNetwork_Receive(D_00160508, &value, 4);
            }
            load_game(0, 1);
            draw_sync_progress(100);
        }
        value = 0;
        while (value == 0) {
            LbNetwork_Receive(D_00160508, &value, 4);
        }
    }
}

// Carries out a turn's player commands: records or plays back the input,
// applies the local player's scroll requests, exchanges the turn's packets in
// network games (resynchronising if the checksums disagree), then runs each
// player's command (Event.type, EV_*) and moves it to the previous turn's
// copy. Also checks for an autosave.
// FUNCTION: 0x0001c7c0
void process_turn(void)
{
    char buf[100];
    Thing *employee;
    int mouse_x;
    Thing *patient;
    int mouse_y;
    u16 j;
    u16 i;
    u8 changed;

    if (D_001581ee & 2) {
        u32 recorded_seed;

        LbFileRead(D_001581e8, packet, 0xf);
        LbFileRead(D_001581e8, &mouse_x, 4);
        LbFileRead(D_001581e8, &mouse_y, 4);
        LbFileRead(D_001581e8, &gamevar.mouse_mode, 2);
        LbFileRead(D_001581e8, &gamevar, 0x178);
        LbFileRead(D_001581e8, &recorded_seed, 4);
        if (seed != recorded_seed) {
            fn_000bbff0(debug.msg_text, "PLAYBACK OUT OF SYNC");
            debug.msg_timer = 200;
        }
        LbMouseSetPosition(mouse_x, mouse_y);
    } else if (D_001581ee & 1) {
        mouse_x = lbDisplay.mmouse_x;
        mouse_y = lbDisplay.mmouse_y;
        LbFileWrite(D_001581e8, packet, 0xf);
        LbFileWrite(D_001581e8, &mouse_x, 4);
        LbFileWrite(D_001581e8, &mouse_y, 4);
        LbFileWrite(D_001581e8, &gamevar.mouse_mode, 2);
        LbFileWrite(D_001581e8, &gamevar, 0x178);
        LbFileWrite(D_001581e8, &seed, 4);
    }
    if (D_001549d0[player].flags && !pmenu.open_pulldown) {
        if (D_001549d0[player].flags & 1) {
            scroll.up = 1;
        } else if (D_001549d0[player].flags & 2) {
            scroll.down = 1;
        }
        if (D_001549d0[player].flags & 4) {
            scroll.left = 1;
        } else if (D_001549d0[player].flags & 8) {
            scroll.right = 1;
        }
        if (D_001549d0[player].flags & 0x10) {
            goto_column_row_in_window(window.current_window, level->camera_start[player] % 128, level->camera_start[player] / 128);
            othervar.full_redraw = 1;
        }
        D_00154a0c[player].flags = D_001549d0[player].flags;
        D_001549d0[player].flags = 0;
    }
    if (statvar.num_players > 1) {
        D_001549d0[player].checksum = check_sum();
        sprintf(buf, "size of pac %ld", 0xf);
        if (!fn_00086b60()) {
            LbNetwork_ChangeExchangeBuffer(packet, 0xf);
            fn_0008debc("Entering net xchg...");
            if (LbNetwork_Exchange((u8 *)packet + player * 0xf) == -1) {
            }
            fn_0008debc("done\n");
            if (game_turn == 4) {
                LbNetwork_ChangeExchangeTimeout(0);
            }
            changed = 0;
            for (j = 0; j < 4; j++) {
                if (D_001609c4[j].active && D_001549d0[j].checksum) {
                    if (D_001549d0[D_00160508].checksum != D_001549d0[j].checksum) {
                        if (D_001549d0[j].type != EV_NETWORK_GAINED_FOCUS && D_001549d0[j].type != EV_NETWORK_LOST_FOCUS) {
                            D_001549d0[j].type = EV_NONE;
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
                resync_network_game();
                D_00160294 = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            u8 handled = 0;

            switch (D_001549d0[i].type) {
            case EV_NETWORK_LOST_FOCUS:
                network_lost_focus(i);
                handled = 1;
                break;
            case EV_NETWORK_GAINED_FOCUS:
                network_gained_focus(i);
                handled = 1;
                break;
            }
            if (handled) {
                D_001549d0[i].type = EV_NONE;
                D_001549d0[i].value = 0;
                D_001549d0[i].arg = 0;
                return;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        othervar.cur_player = i;
        if (allhosps[i].playing != 2) {
            switch (D_001549d0[i].type) {
            case EV_QUIT_GAME:
            case EV_LEAVE_NET_GAME:
                goto dispatch;
            }
            goto copy;
        }
    dispatch:
        switch (D_001549d0[i].type) {
        case EV_NONE:
            break;
        case EV_OPEN_ROOM:
            open_room(i);
            break;
        case EV_MODIFY_BLUEPRINT:
            modify_blueprint(i, 1, D_001549d0[i].value);
            break;
        case EV_COMMIT_BLUEPRINT:
            if (commit_blueprint(i)) {
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                }
            } else {
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            }
            break;
        case EV_UNCOMMIT_BLUEPRINT:
            uncommit_blueprint(i);
            if (i == player) {
                set_mouse_mode(MM_BLUEPRINT);
            }
            break;
        case 0x14:
            if (i == player) {
                crequester.arg = 1;
                set_buyable_items(3);
            }
            break;
        case EV_PLACE_BP_DOOR:
            place_bp_door(i, (u8)D_001549d0[i].value);
            break;
        case EV_COMMIT_DOORS:
            if (i == player) {
                set_mouse_mode(0x10);
            }
            commit_doors(i);
            break;
        case EV_PLACE_STAFF: {
            u8 empty = !allhosps[i].held_staff;

            if (place_staff_on_map(D_001549d0[i].value, D_001549d0[i].arg, 0, i)) {
                if (empty) {
                    change_balance(TX_HIRE_STAFF, -(thing[D_001549d0[i].arg].salary * 12) / 10, 0, othervar.cur_player);
                }
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_PLAC_ST2, -1, -1);
                    set_mouse_mode(MM_NORMAL);
                }
            }
            break;
        }
        case EV_PLACE_OBJECT:
            if (place_object(D_001549d0[i].value, D_001549d0[i].argb[0], D_001549d0[i].argb[1], i) != 0) {
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_PLACE_R, -1, -1);
                    object_placed();
                }
            }
            break;
        case EV_HOVER_BLUEPRINT:
            modify_blueprint(i, 0, D_001549d0[i].value);
            break;
        case EV_QUEUE_PERSON_TO_NEW_QUEUE:
            queue_person_to_new_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case EV_ANSWER_PLAYERCALL:
            if (i == player) {
                compute_whether_user_not_busy();
            }
            switch (D_001549d0[i].value) {
            case 1:
                j = get_playercall(1, D_001549d0[i].arg, i);
                if (j) {
                    if (i == player && othervar.user_idle) {
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
                if (allhosps[i].contagious_count) {
                    allhosps[i].epidemic_fine = allhosps[i].contagious_count * gbv.epidemic_fine;
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
                    allhosps[i].emergency_state = 0;
                    if (i == player) {
                        display_choice_screen(i, 3, 0, 400);
                    }
                    break;
                case 2:
                    allhosps[i].mayor_state = 0;
                    if (i == player) {
                        display_choice_screen(i, 9, 0, 400);
                    }
                    break;
                case 3:
                    allhosps[i].epidemic_over = 0;
                    display_choice_screen(i, 6, allhosps[i].inspected_illness, 400);
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
        case EV_PLACE_SMALL_THING:
            place_small_thing(D_001549d0[i].value, D_001549d0[i].arg, 0);
            if (i == player) {
                play_sound(SNDSRC_GENERAL, SFX_PLACE_R, -1, -1);
            }
            break;
        case EV_SAVE_GAME:
            if (i == player) {
                if (othervar.save_slot_used[D_001549d0[i].value] == 1) {
                    if (allhosps[i].held_staff) {
                        place_staff_on_map(allhosps[i].held_staff_tile, allhosps[i].held_staff, 1, i);
                    } else if (gamevar.held_staff) {
                        req2_exit_employing(gamevar.held_staff);
                    }
                    close_yes_no_requester(0x63);
                    set_mouse_mode(MM_NORMAL);
                    lbDisplay.left_button = 0;
                }
                save_game((s8)D_001549d0[i].value, -1);
                check_save_games();
            }
            break;
        case EV_LOAD_GAME:
            if (i == player) {
                load_game((s8)D_001549d0[i].value, -1);
            }
            break;
        case EV_VARY_SPEED:
            vary_speed((s16)D_001549d0[i].value);
            break;
        case 0x17:
            break;
        case 0x18:
            break;
        case EV_TEMP_PLACE_DOOR:
            temp_place_door(D_001549d0[i].value, i);
            break;
        case EV_REQ1_RESET_ROOMS:
            if (i == player) {
                crequester.arg = D_001549d0[i].value;
                crequester.mode = 4;
                req1_reset_rooms(crequester.arg);
            }
            break;
        case EV_INIT_BLUEPRINT:
            init_blueprint(i, D_001549d0[i].value);
            if (i == player) {
                crequester.kind = D_001549d0[i].value;
                set_buyable_items(crequester.kind);
                buy_compulsory_items(crequester.kind);
                close_crequester(crequester.id);
                set_mouse_mode(MM_BLUEPRINT);
                open_edit_room_requester();
                set_edit_room_requester_mode(0);
                pmenu.disabled = 0;
            }
            allocate_roominfo(i, D_001549d0[i].value);
            break;
        case EV_QUIT_GAME:
            if (i == player) {
                if (crequester.id == REQ_YES_NO) {
                    close_crequester(REQ_YES_NO);
                    pmenu.disabled = 0;
                }
            }
            if (statvar.num_players > 1) {
                LbNetwork_Stop();
                if (othervar.music_playing) {
                    stop_midi();
                }
                statvar.num_players = 1;
                player = 0;
                find_other_player(-1);
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
                        sprintf(name, "%ssave/hosp.r%d", statvar.save_root, D_001581ef);
                        D_001581e8 = LbFileOpen(name, 2);
                        if (D_001581e8 > 0) {
                            D_001581ee |= 2;
                            break;
                        }
                    }
                }
            }
            break;
        case EV_LOAD_GAME_AT_START:
            if (i == player) {
                if (load_game((s8)D_001549d0[i].value, -1) == 0) {
                    goto_cell_in_window(0, get_room_info(ROOM_ENTRANCE, 0x65, othervar.cur_player)->door_tile);
                    othervar.full_redraw = 1;
                }
            }
            break;
        case EV_GOTO_COLUMN_ROW:
            goto_column_row_in_window(window.current_window, level->camera_start[player] % 128, level->camera_start[player] / 128);
            othervar.show_level_brief = 1;
            othervar.full_redraw = 1;
            break;
        case 0x21:
            break;
        case EV_PAUSE:
            yearend.value--;
            if (display_mode != DM_YEAR_END || yearend.value == 0) {
                gamevar.paused = !gamevar.paused;
                if (i == player) {
                    int handle;

                    if (gamevar.paused) {
                        StopAllSamples();
                        othervar.panel_info_hold = 0x32;
                        if (othervar.music_playing) {
                            handle = GetSongHandle();
                            AIL_stop_sequence(handle);
                            othervar.music_playing = 0;
                        }
                    } else {
                        othervar.panel_info_hold = 0x32;
                        if (myplayer.music_on && statvar.music_ok && othervar.midi_playing) {
                            handle = GetSongHandle();
                            AIL_resume_sequence(handle);
                            othervar.music_playing = 1;
                        }
                    }
                }
            }
            break;
        case EV_CLOSE_REQUESTERS:
            if (i == player) {
                close_crequester(crequester.id);
                close_irequesters();
                set_mouse_sprite(othervar.prev_pointer_sprite);
                pmenu.disabled = 0;
            }
            break;
        case 0x27:
            if (i == player) {
                close_crequester(crequester.id);
                set_mouse_sprite(othervar.prev_pointer_sprite);
                pmenu.disabled = 0;
            }
            break;
        case EV_BUY_OBJECT:
            if (i == player) {
                req1_buy_object(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case EV_PLACE_NEXT_OBJECT:
            if (i == player) {
                u16 k;

                close_crequester(crequester.id);
                k = next_object_to_build();
                if (k) {
                    init_place_object(k, 2, 0, 0);
                }
                pmenu.disabled = 0;
            }
            break;
        case EV_FADE_TO_BLACK:
            hosp_fade_palette(0, 0x32, Lb_PALETTE_FADE_CLOSED);
            break;
        case EV_FADE_IN:
            if (i == player) {
                hosp_fade_palette(palette, 0x32, Lb_PALETTE_FADE_CLOSED);
            }
            break;
        case EV_OPEN_HIRE_STAFF:
            if (i == player) {
                cancel_top_level_requesters(REQ_HIRE_STAFF);
                if (crequester.id == REQ_NONE) {
                    req2_reset();
                    close_irequesters();
                    open_requester(crequester.id, 1);
                    set_mouse_sprite(PTR_DEFAULT);
                    pmenu.disabled = 1;
                }
            }
            break;
        case EV_HIRE_SELECT_KIND:
            if (i == player) {
                crequester.mode = 2;
                crequester.arg = D_001549d0[i].value;
                crequester.arg2 = req2_alter_staff(1, 0, crequester.arg);
            }
            break;
        case EV_MARK_EMPLOYING:
            if (i == player) {
                if (thing[D_001549d0[i].arg].state != PS_WAITS_FOR_DECISION) {
                    close_crequester(crequester.id);
                    gamevar.held_staff_kind = D_001549d0[i].value;
                    gamevar.held_staff = D_001549d0[i].arg;
                    gamevar.drawn_staff_kind = 0xffff;
                    set_mouse_mode(MM_PLACE_STAFF);
                    pmenu.disabled = 0;
                }
            }
            req2_mark_employing(D_001549d0[i].arg);
            break;
        case EV_HIRE_NEXT_APPLICANT:
            if (i == player) {
                crequester.arg2 = req2_alter_staff((u8)D_001549d0[i].value, crequester.arg2, crequester.arg);
            }
            break;
        case EV_CLOSE_SCREEN:
            if (display_mode != DM_GAME) {
                display_mode = DM_GAME;
                othervar.full_redraw = 1;
                D_00156910 = 1;
                othervar.fade_palette = palette;
            }
            break;
        case EV_OPEN_SCREEN:
            if (i == player && gamevar.mouse_mode == MM_EDIT_ROOM_WAIT) {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                break;
            }
            if (allhosps[i].held_staff) {
                place_staff_on_map(allhosps[i].held_staff_tile, allhosps[i].held_staff, 1, i);
            }
            update_cure_costs();
            if (i == player) {
                display_mode = DM_CHARTS;
                chart_type = D_001549d0[i].value;
                switch (chart_type) {
                case 0:
                    bank.state = 0;
                    break;
                case 1:
                    drugs.state = 0;
                    break;
                case 2:
                    pol.state = 0;
                    break;
                case 3:
                    research.state = 0;
                    break;
                case 4:
                    graphs.state = 0;
                    break;
                case 5:
                    slist.state = 0;
                    break;
                case 6:
                    calculate_value(player);
                    manager.state = 0;
                    break;
                case 7:
                    D_00158166 = 0;
                    calculate_value(player);
                    break;
                case 8:
                    reputation.state = 0;
                    break;
                case 9:
                    variables.state = 0;
                    break;
                case 0xd:
                    yearend.state = 0;
                    break;
                case 0xf:
                    hall.state = 0;
                    break;
                }
                othervar.full_redraw = 1;
                D_00156910 = 1;
            }
            break;
        case EV_BUY_LAND:
            if (statvar.num_players > 1) {
                town.state = 1;
            }
            buy_land(D_001549d0[i].value);
            satisfied_net_criteria(3, i);
            break;
        case EV_OPEN_BUILD_ROOM:
            if (i == player) {
                cancel_top_level_requesters(REQ_BUILD_ROOM);
                if (crequester.id == REQ_NONE) {
                    crequester.id = REQ_BUILD_ROOM;
                    crequester.mode = 1;
                    close_irequesters();
                    open_requester(crequester.id, 1);
                    crequester.arg2 = 0;
                    set_mouse_sprite(PTR_DEFAULT);
                    crequester.kind = 0;
                    crequester.arg3 = 0;
                    clear_req_indices();
                    pmenu.disabled = 1;
                }
            }
            break;
        case EV_OPEN_QUEUE_WINDOW:
            if (i == player) {
                if (crequester.id == REQ_NONE && &room_info[D_001549d0[i].value] > room_info) {
                    crequester.id = REQ_QUEUE;
                    crequester.mode = 0;
                    crequester.arg = 0;
                    crequester.arg2 = 0;
                    crequester.kind = 0;
                    crequester.arg3 = 0;
                    crequester.unknownA = 0;
                    close_irequesters();
                    open_requester(crequester.id, 1);
                    set_mouse_sprite(PTR_DEFAULT);
                    othervar.queue_pos = 0;
                    othervar.queue_win_room = D_001549d0[i].value;
                    othervar.queue_win_cell = D_001549d0[i].arg;
                    pmenu.disabled = 1;
                }
            }
            break;
        case EV_FRONT_QUEUE:
            front_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case EV_BACK_QUEUE:
            back_queue(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case EV_CANCEL_BUILD:
            if (allhosps[i].held_staff) {
                place_staff_on_map(allhosps[i].held_staff_tile, allhosps[i].held_staff, 1, i);
            } else if (D_001549d0[i].value) {
                req2_exit_employing(D_001549d0[i].value);
            }
            clear_build_objects(i);
            if (i == player) {
                if (!gamevar.litter_bombs_to_place) {
                    close_crequester(REQ_STAFF);
                    close_crequester(REQ_HIRE_STAFF);
                    close_crequester(REQ_JUKEBOX);
                    close_crequester(REQ_QUEUE);
                    close_crequester(REQ_BUILD_ROOM);
                    close_crequester(REQ_BUY_OBJECTS);
                    close_crequester(REQ_STAFF_DEMAND);
                    close_crequester(REQ_EDIT_ROOM);
                    close_irequesters();
                    gamevar.held_staff = 0;
                    gamevar.held_staff_kind = 0;
                    gamevar.place_kind = 0;
                    gamevar.place_blueprint = 0;
                    set_mouse_mode(MM_NORMAL);
                    pmenu.disabled = 0;
                } else {
                    init_place_object(0x11, gamevar.place_mode, gamevar.place_room, 0);
                }
            }
            break;
        case EV_OPEN_THING_WINDOW:
            if (i == player) {
                Thing *obj;
                int kind = 0;
                u8 slot = 0;

                obj = &thing[D_001549d0[i].value];
                if (obj->owner == player) {
                    switch (obj->type) {
                    case THING_PATIENT:
                        slot = 1;
                        D_001581b2[slot].kind = REQ_PATIENT;
                        D_001581b2[slot].show_graph = 1;
                        D_001581b2[slot].toggle_delay = 0;
                        kind = 0x22;
                        break;
                    case THING_STAFF:
                        switch (obj->subtype) {
                        case STAFF_HANDYMAN:
                            slot = 2;
                            D_001581b2[slot].kind = REQ_HANDYMAN;
                            kind = 0x21;
                            break;
                        default:
                            slot = 2;
                            D_001581b2[slot].kind = REQ_STAFF;
                            kind = 0x20;
                            break;
                        }
                        break;
                    case THING_OBJECT:
                        slot = 3;
                        D_001581b2[slot].kind = REQ_MACHINE;
                        break;
                    }
                }
                if (slot) {
                    close_irequesters();
                    othervar.irequester_slot = slot;
                    open_requester(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    set_mouse_sprite(PTR_DEFAULT);
                }
                if (kind) {
                    int x = resvars.window_list[slot].x;
                    int y = resvars.window_list[slot].y;

                    open_new_window(0, 0, 0, x + resvars.boxes_Rectp[kind].x0, y + resvars.boxes_Rectp[kind].y0, x + resvars.boxes_Rectp[kind].x1, y + resvars.boxes_Rectp[kind].y1);
                    othervar.track_window[slot] = window.window_count - 1;
                    window.current_window = 0;
                }
            }
            break;
        case EV_CLOSE_QUERY:
            if (i == player) {
                close_query((u8)D_001549d0[i].value);
            }
            break;
        case EV_PICK_UP_OBJECT:
            pick_up_object(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                play_sound(SNDSRC_GENERAL, SFX_PICKUPSM, -1, -1);
            }
            break;
        case EV_SACK_STAFF:
            sack_staff(&thing[D_001549d0[i].value], D_001549d0[i].arg);
            if (i == player) {
                close_yes_no_requester(1);
                close_irequesters();
                set_mouse_mode(MM_NORMAL);
                lbDisplay.left_button = 0;
                play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
            }
            break;
        case EV_EDIT_BLUEPRINT:
            edit_blueprint(i);
            break;
        case EV_PAY_STAFF_BONUS:
            if (D_001549d0[i].arg) {
                if (allhosps[i].balance >= D_001549d0[i].value && allhosps[i].balance > 0) {
                    pay_staff_bonus(&thing[D_001549d0[i].arg], D_001549d0[i].value, 0, 1, i);
                }
            } else {
                if (allhosps[i].balance >= pay_staff_bonus(0, D_001549d0[i].value, 0, 0, i) && allhosps[i].balance > 0) {
                    pay_staff_bonus(0, D_001549d0[i].value, 0, 1, i);
                }
            }
            break;
        case EV_INIT_EDIT_ROOM:
            othervar.editing_room = 1;
            init_edit_room(&room_info[D_001549d0[i].value]);
            break;
        case EV_OPEN_JUKEBOX:
            if (i == player) {
                if (crequester.id == REQ_NONE) {
                    crequester.id = REQ_JUKEBOX;
                    crequester.mode = 1;
                    crequester.arg = 0;
                    crequester.arg2 = 0;
                    crequester.kind = 0;
                    crequester.arg3 = 0;
                    crequester.unknownA = 0;
                    close_irequesters();
                    open_requester(crequester.id, 1);
                    set_mouse_sprite(PTR_DEFAULT);
                    pmenu.disabled = 1;
                }
            }
            break;
        case EV_START_EMERGENCY:
            allhosps[i].emergency_state = 10;
            allhosps[i].emergency_timer = 100;
            allhosps[i].emergency_cured = 0;
            othervar.emergency_timer_start = allhosps[i].emergency_timer;
            if (i == player) {
                return_from_screen(0, 1, 1);
            }
            break;
        case EV_NET_TEXT_DISPLAY:
            if (D_001549d0[i].value & (1 << player)) {
                net_text_display(i);
            }
            net_text[i].text = 0;
            break;
        case EV_NET_TEXT_DELETE:
            net_text_delete(i);
            break;
        case EV_NET_TEXT_KEYPRESS:
            net_text_keypress(D_001549d0[i].value, i);
            break;
        case EV_NET_TEXT_START:
            net_text[i].text = 0;
            if (i == player) {
                othervar.net_text_entering = 1;
            }
            break;
        case EV_SET_HEATING:
            if (allhosps[i].heating_broken == 0) {
                allhosps[i].heating_level = D_001549d0[i].value;
            }
            break;
        case EV_ALTER_RESEARCH:
            alter_research(D_001549d0[i].value, D_001549d0[i].arg, i);
            break;
        case EV_QUEUE_PERSON_GO_HOME:
            queue_person_go_home(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
            }
            break;
        case EV_QUEUE_CAPACITY_UP:
            if (room_info[D_001549d0[i].value].queue_capacity < 0x1e) {
                room_info[D_001549d0[i].value].queue_capacity++;
            }
            break;
        case EV_QUEUE_CAPACITY_DOWN:
            if (room_info[D_001549d0[i].value].queue_capacity) {
                room_info[D_001549d0[i].value].queue_capacity--;
            }
            break;
        case EV_QUEUE_PERSON_GO_RECEPTION:
            queue_person_go_reception(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case EV_STAFF_JOIN_NEW_EMPLOYER:
            staff_join_new_employer(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == player) {
                display_mode = DM_GAME;
                close_crequester(REQ_STAFF_DEMAND);
                close_irequesters();
                window.current_window = 0;
                pmenu.disabled = 0;
            }
            break;
        case EV_SETUP_TOWN_BID:
            finish_composite_help();
            setup_town_bid();
            display_mode = DM_TOWN_BID;
            town.state = 0xb;
            othervar.bid_plot = D_001549d0[i].value;
            break;
        case EV_PLOT_BID_MONEY:
            plot_bid_money(i);
            break;
        case EV_INCREASE_STAFF_SALARIES:
            if (D_001549d0[i].arg) {
                increase_staff_salaries(&thing[D_001549d0[i].arg], D_001549d0[i].value, 0, i);
            } else {
                increase_staff_salaries(0, D_001549d0[i].value, 0, i);
            }
            break;
        case EV_FURNISH_EDITED_ROOM:
            if (i == player) {
                close_irequesters();
                close_crequester(REQ_EDIT_ROOM);
                othervar.saved_build_palette = gamevar.buy_list;
                adjust_build_palette_for_shop(&room_info[allhosps[i].edited_room]);
                set_buyable_items(room_info[allhosps[i].edited_room].type);
                crequester.id = REQ_BUY_OBJECTS;
                open_requester(crequester.id, 1);
                crequester.kind = allhosps[i].build.room_type;
                crequester.arg3 = room_info[allhosps[i].edited_room].pos_lo;
                crequester.mode = 3;
                crequester.item = req_indices[0];
                pmenu.disabled = 0;
            }
            break;
        case EV_BACK_TO_PLACE_OBJECT:
            if (i == player) {
                go_back_to_place_object_requester(D_001549d0[i].value);
            }
            break;
        case EV_ORDER_REST:
            employee = &thing[D_001549d0[i].value];
            employee->orders &= 0xf0;
            employee->orders |= ORD_REST;
            break;
        case EV_ORDER_TRAIN:
            employee = &thing[D_001549d0[i].value];
            employee->orders &= 0xf0;
            employee->orders |= ORD_TRAIN;
            break;
        case 0x60:
            employee = &thing[D_001549d0[i].value];
            employee->orders &= 0xf0;
            employee->orders |= 2;
            break;
        case EV_ORDER_LEAVE_ROOM:
            employee = &thing[D_001549d0[i].value];
            employee->orders &= 0xf0;
            employee->orders |= ORD_LEAVE_ROOM;
            break;
        case EV_OPEN_FURNISH_CORRIDOR:
            if (i == player) {
                cancel_top_level_requesters(REQ_BUY_OBJECTS);
                if (crequester.id == REQ_NONE) {
                    crequester.arg = 1;
                    set_buyable_items(3);
                    close_irequesters();
                    crequester.id = REQ_BUY_OBJECTS;
                    open_requester(crequester.id, 1);
                    allhosps[i].edited_room = 0;
                    crequester.kind = 3;
                    crequester.arg3 = 0;
                    crequester.mode = 3;
                    crequester.item = req_indices[0];
                    pmenu.disabled = 1;
                }
            }
            break;
        case EV_SELL_OBJECT:
            if (i == player) {
                req11_sell_object(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case EV_CANCEL_BUY_OBJECTS:
            if (i == player) {
                close_crequester(REQ_BUY_OBJECTS);
                close_irequesters();
                set_mouse_sprite(othervar.prev_pointer_sprite);
                pmenu.disabled = 0;
            }
            break;
        case EV_CHEAT_END_MONTH:
            mycontrol->day = calendar[mycontrol->month] - 1;
            break;
        case EV_TAKE_LOAN:
            change_balance(TX_BANK_LOAN, 5000, 0, i);
            allhosps[i].loan += 5000;
            break;
        case EV_REPAY_LOAN:
            if (allhosps[i].balance >= 5000 && allhosps[i].balance > 0) {
                change_balance(TX_LOAN_REPAYMENT, -5000, 0, i);
                allhosps[i].loan -= 5000;
            }
            break;
        case EV_REFER_TO_PLAYER:
            patient = &thing[D_001549d0[i].value];
            allhosps[patient->owner].rooms[patient->illness].turned_away++;
            if (patient->flags0 & PF_CONTAGIOUS) {
                allhosps[patient->owner].epidemic_timer = 0;
            }
            refer_patient(patient, D_001549d0[i].arg);
            if (i == player) {
                close_query(1);
                play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
            }
            break;
        case EV_PICK_UP_STAFF:
            employee = &thing[D_001549d0[i].value];
            allhosps[i].held_staff = D_001549d0[i].value;
            if (!(employee->orders & ORD_RESIGN)) {
                if (fn_0002d11c(employee)) {
                    interrupt_staff(employee);
                    staff_picked_up(&thing[D_001549d0[i].value]);
                }
            }
            if (i == player) {
                close_query(2);
            }
            break;
        case EV_STAFF_PICKED_UP:
            staff_picked_up(&thing[D_001549d0[i].value]);
            break;
        case EV_SHOOT_RAT: {
            Thing *rat = &thing[D_001549d0[i].value];

            switch (D_001549d0[i].argb[0]) {
            case 1:
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_UZIBRST, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
                }
                break;
            case 2:
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_ROCKET, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
                }
                break;
            case 3:
                create_explosion(rat->x.tile + (rat->y.tile << 7), 0x902);
                break;
            default:
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_SHOTGUN, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
                }
                break;
            }
            award_counts.rat_shots++;
            allhosps[i].rat_shots++;
            if (D_001549d0[i].argb[1]) {
                award_counts.rats_killed++;
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_DEADRAT2, lbDisplay.mmouse_x, lbDisplay.mmouse_y);
                }
                create_numfx(NUMFX_NUMBER, ++allhosps[i].rats_killed, rat->x.tile + (rat->y.tile << 7), color.dark_blue, 0x20);
                place_litter(rat->x.tile + (rat->y.tile << 7), (s8)rat->owner, 4);
                remove_thing(rat);
            }
            break;
        }
        case EV_REFUSE_STAFF_DEMAND:
            employee = &thing[D_001549d0[i].value];
            if (employee > thing) {
                if (old_newrand() % (employee->resign_count + 1)) {
                    sack_staff(&thing[D_001549d0[i].value], 1);
                } else {
                    employee->orders &= ~ORD_RESIGN;
                    advisor.event_flags &= 0xfe;
                    employee->discontent = 0;
                    reset_icon_value(&thing[D_001549d0[i].value], ICON_LEAVING);
                }
                if (i == player) {
                    display_mode = DM_GAME;
                    close_crequester(REQ_STAFF_DEMAND);
                    close_irequesters();
                    window.current_window = 0;
                }
                pmenu.disabled = 0;
            }
            break;
        case EV_RAISE_STAFF_SALARY:
            employee = &thing[D_001549d0[i].value];
            if (employee > thing) {
                employee->orders &= ~ORD_RESIGN;
                advisor.event_flags &= 0xfe;
                employee->salary += D_001549d0[i].arg;
                employee->happiness = 100;
                employee->discontent = 0;
                reset_icon_value(&thing[D_001549d0[i].value], ICON_LEAVING);
            }
            if (i == player) {
                display_mode = DM_GAME;
                close_crequester(REQ_STAFF_DEMAND);
                close_irequesters();
                window.current_window = 0;
                pmenu.disabled = 0;
            }
            break;
        case EV_GIVE_STAFF_BONUS:
            employee = &thing[D_001549d0[i].value];
            if (employee > thing) {
                employee->orders &= ~ORD_RESIGN;
                advisor.event_flags &= 0xfe;
                pay_staff_bonus(employee, D_001549d0[i].arg, 0, 1, i);
                employee->happiness = 100;
                employee->discontent = 0;
                reset_icon_value(&thing[D_001549d0[i].value], ICON_LEAVING);
            }
            if (i == player) {
                display_mode = DM_GAME;
                close_crequester(REQ_STAFF_DEMAND);
                close_irequesters();
                window.current_window = 0;
                pmenu.disabled = 0;
            }
            break;
        case EV_PRICE_UP: {
            int illness = D_001549d0[i].arg;
            int max_price = max_price_for_treatment(illness, i);

            if (allhosps[i].rooms[illness].price_scale < 20000) {
                allhosps[i].rooms[illness].price_scale += D_001549d0[i].value * 100;
                if (allhosps[i].rooms[illness].price_scale < 5000) {
                    allhosps[i].rooms[illness].price_scale = 5000;
                } else if (allhosps[i].rooms[illness].price_scale > 20000) {
                    allhosps[i].rooms[illness].price_scale = 20000;
                }
                allhosps[i].rooms[illness].price = allhosps[i].rooms[illness].price_scale * max_price / 10000;
            }
            break;
        }
        case EV_PRICE_DOWN: {
            int illness = D_001549d0[i].arg;
            int max_price = max_price_for_treatment(illness, i);

            if (allhosps[i].rooms[illness].price_scale > 5000) {
                allhosps[i].rooms[illness].price_scale -= D_001549d0[i].value * 100;
                if (allhosps[i].rooms[illness].price_scale < 5000) {
                    allhosps[i].rooms[illness].price_scale = 5000;
                } else if (allhosps[i].rooms[illness].price_scale > 20000) {
                    allhosps[i].rooms[illness].price_scale = 20000;
                }
                allhosps[i].rooms[illness].price = allhosps[i].rooms[illness].price_scale * max_price / 10000;
            }
            break;
        }
        case EV_RETURN_FROM_SCREEN:
            if (othervar.cur_player == player) {
                lbDisplay.rleft_button = lbDisplay.rright_button = lbDisplay.left_button = lbDisplay.right_button = 0;
                return_from_screen(D_001549d0[i].value, 1, 1);
            }
            break;
        case EV_SET_POLICY:
            switch (D_001549d0[i].value) {
            case 1:
                if (D_001549d0[i].arg <= allhosps[i].guess_cure_threshold) {
                    allhosps[i].send_home_threshold = D_001549d0[i].arg;
                } else {
                    allhosps[i].send_home_threshold = allhosps[i].guess_cure_threshold;
                }
                break;
            case 2:
                if (D_001549d0[i].arg >= allhosps[i].send_home_threshold) {
                    allhosps[i].guess_cure_threshold = D_001549d0[i].arg;
                } else {
                    allhosps[i].guess_cure_threshold = allhosps[i].send_home_threshold;
                }
                break;
            case 3:
                allhosps[i].diag_threshold = D_001549d0[i].arg;
                break;
            case 4:
                allhosps[i].rest_threshold = D_001549d0[i].arg;
                break;
            case 5:
                allhosps[i].staff_leave_rooms = D_001549d0[i].arg;
                break;
            }
            break;
        case EV_CONCENTRATE_RESEARCH:
            if (D_001549d0[i].value == (statvar.num_players > 1 ? allhosps[i].hospital_open : gamevar.research_illness)) {
                if (statvar.num_players > 1) {
                    allhosps[i].hospital_open = 0;
                } else {
                    gamevar.research_illness = 0;
                }
            } else {
                if (statvar.num_players > 1) {
                    allhosps[i].hospital_open = D_001549d0[i].value;
                } else {
                    gamevar.research_illness = D_001549d0[i].value;
                }
            }
            break;
        case EV_ADJUST_HANDYMAN_VALUES:
            adjust_handyman_values(D_001549d0[i].value, D_001549d0[i].argb[0]);
            break;
        case EV_PERSON_GO_HOME:
            patient = &thing[D_001549d0[i].value];
            patient->treatment = 0;
            allhosps[patient->owner].rooms[patient->illness].turned_away++;
            person_go_home(patient);
            patient->flags1 |= (PF_SENT_HOME >> 8);
            if (patient->flags0 & PF_CONTAGIOUS) {
                allhosps[patient->owner].epidemic_timer = 0;
            }
            break;
        case EV_THINGPAGE_REQUEST:
            thingpage_request(&allhosps[i].repair_pager, D_001549d0[i].value, 0xff);
            set_icon_value(&thing[D_001549d0[i].value], ICON_NEEDS_REPAIR);
            break;
        case EV_OPEN_YES_NO:
            if (i == player) {
                if (othervar.screen_pic_loaded) {
                    return_from_screen(0, 1, 1);
                }
                othervar.yesno_saved_mouse_mode = gamevar.mouse_mode;
                display_mode = DM_MODAL;
                close_irequesters();
                othervar.yesno_saved_requester = crequester.id;
                if (crequester.id != REQ_NONE) {
                    close_crequester(crequester.id);
                }
                crequester.id = REQ_YES_NO;
                open_requester(crequester.id, 1);
                pmenu.disabled = 1;
            }
            break;
        case EV_CLOSE_YES_NO:
            if (i == player) {
                close_yes_no_requester((u8)D_001549d0[i].value);
                if (othervar.no_arg) {
                    set_mouse_mode((u8)othervar.no_arg);
                    lbDisplay.left_button = 0;
                }
            }
            break;
        case EV_DELETE_ROOM:
            close_yes_no_requester(0);
            clear_build_objects(i);
            delete_blueprint_room(i);
            D_001549d0[i].type = EV_CANCEL_BUILD;
            D_001549d0[i].arg = gamevar.mouse_mode;
            goto dispatch;
        case EV_CANCEL_EDIT_ROOM:
            fn_0002994c(i);
            D_001549d0[i].type = EV_CANCEL_BUILD;
            D_001549d0[i].value = 0;
            goto dispatch;
        case EV_REFER_PATIENT:
            refer_patient(&thing[D_001549d0[i].value], D_001549d0[i].arg);
            break;
        case EV_CHOICE_RESULT: {
            u8 quiet = 0;

            switch (D_001549d0[i].value) {
            case CH_TREAT_SEND_HOME:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->treatment = 0;
                    allhosps[patient->owner].rooms[patient->illness].turned_away++;
                    person_go_home(patient);
                    patient->flags2 |= (PF_NO_TREATMENT >> 16);
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            case CH_TREAT_WAIT:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->flags0 &= ~PF_AWAITING_DECISION;
                    reset_icon_value(patient, ICON_AWAITING_DECISION);
                    patient->flags0 |= PF_WAITING_FOR_ROOM;
                    patient->timer = othervar.cur_turn;
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            case CH_TREAT_RESEARCH:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->treatment = ILL_AUTOPSY;
                    patient->flags0 &= ~PF_AWAITING_DECISION;
                    reset_icon_value(patient, ICON_AWAITING_DECISION);
                    patient->state = PS_GO_FOR_TREATMENT;
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            case CH_EPIDEMIC_DECLARE:
                count_and_flag_contagious_peeps(allhosps[i].epidemic_illness, 2, i);
                allhosps[i].epidemic_illness = 0;
                allhosps[i].epidemic_active = 0;
                change_balance(TX_EPIDEMIC_FINE, -allhosps[i].epidemic_fine, 0, i);
                reputation_affected(0, REP_EPIDEMIC_DECLARED, 0, i, 0);
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_EPIDEMIC_END, -1, -1);
                }
                break;
            case CH_EPIDEMIC_COVER_UP:
                count_and_flag_contagious_peeps(allhosps[i].epidemic_illness, 1, i);
                allhosps[i].epidemic_active = 1;
                allhosps[i].epidemic_timer = 100;
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_EPIDEMIC_START, -1, -1);
                }
                quiet = D_001549d0[i].arg;
                break;
            case CH_VIP_REFUSE:
                allhosps[i].mayor_state = 0;
                break;
            case CH_VIP_ACCEPT:
                allhosps[i].mayor_arrival_turn = actual_game_turn + 1500;
                allhosps[i].mayor_state = 2;
                break;
            case CH_EMERGENCY_REFUSE:
                if (statvar.num_players > 1) {
                    restore_popup(2);
                }
                break;
            case CH_EMERGENCY_ACCEPT:
                give_emergency_stats(D_0015f5c2[player].call, i);
                D_001549d0[i].type = EV_START_EMERGENCY;
                goto dispatch;
            case CH_DIAG_SEND_HOME:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->treatment = 0;
                    allhosps[patient->owner].rooms[patient->illness].turned_away++;
                    person_go_home(patient);
                    patient->flags2 |= (PF_NO_DIAGNOSIS >> 16);
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            case CH_DIAG_GUESS:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->flags0 &= ~PF_AWAITING_DECISION;
                    reset_icon_value(patient, ICON_AWAITING_DECISION);
                    interrupt_complex(patient);
                    best_guess_treatment(patient);
                    next_treatment_now_setup(patient, 0);
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            case CH_DIAG_WAIT:
                patient = &thing[D_001549d0[i].arg];
                if (patient->type == THING_PATIENT && (patient->flags0 & PF_AWAITING_DECISION)) {
                    patient->flags0 &= ~PF_AWAITING_DECISION;
                    reset_icon_value(patient, ICON_AWAITING_DECISION);
                    patient->flags0 |= PF_WAITING_FOR_ROOM;
                    patient->timer = othervar.cur_turn;
                }
                if (patient->type == THING_PATIENT) {
                    patient->playercall = 0;
                }
                break;
            }
            if (i == player && quiet == 0) {
                restore_display_screen();
                othervar.fade_palette = palette;
                othervar.full_redraw = 1;
                D_00156910 = 1;
            }
            break;
        }
        case EV_LEAVE_NET_GAME:
            finish_composite_help();
            switch (D_001549d0[i].value) {
            case 0:
                break;
            case 1:
                if (allhosps[i].playing == 2) {
                    Thing *o;

                    for (o = thing + 1; o < end_things; o++) {
                        if (o->owner == i) {
                            switch (o->type) {
                            case THING_OBJECT:
                                if (objects[o->subtype].overlay_anim) {
                                    o->times_used = o->strength * 2;
                                    machine_disaster(o);
                                }
                                break;
                            case THING_STAFF:
                                sack_staff(o, 0);
                                break;
                            case THING_PATIENT:
                                if (!(map[(s16)o->x.tile + ((s16)o->y.tile << 7)].owner_flags & MAP_OUTDOOR)) {
                                    o->flags0 &= ~PF_CONTAGIOUS;
                                    o->contagion = 0;
                                    o->flags1 |= (PF_SENT_HOME_BY_INSPECTOR >> 8);
                                }
                                break;
                            }
                        }
                    }
                    allhosps[i].playing = 0;
                    D_001549d0[i].checksum = 0;
                    if (i == player) {
                        close_yes_no_requester(0);
                        LbNetwork_Stop();
                        quit_game = 1;
                        othervar.next_level_offer = 100000;
                        if (othervar.net_won) {
                            allhosps[player].playing = 0;
                            level_number = 0xc;
                            othervar.winlose_result = 6;
                            winlose.state = othervar.winlose_result;
                        } else {
                            othervar.winlose_result = 2;
                            winlose.state = othervar.winlose_result;
                        }
                        play_winlose();
                        statvar.num_players = 1;
                        player = 0;
                    }
                    find_other_player((s8)i);
                }
                break;
            case 2:
                break;
            default:
                break;
            }
            break;
        case EV_PATIENT_WAITS_FOR_ROOM:
            patient = &thing[D_001549d0[i].value];
            patient->flags0 &= ~PF_AWAITING_DECISION;
            reset_icon_value(patient, ICON_AWAITING_DECISION);
            patient->flags0 |= PF_WAITING_FOR_ROOM;
            patient->timer = othervar.cur_turn;
            break;
        case EV_PATIENT_TO_RESEARCH:
            patient = &thing[D_001549d0[i].value];
            patient->treatment = ILL_AUTOPSY;
            patient->flags0 &= ~PF_AWAITING_DECISION;
            reset_icon_value(patient, ICON_AWAITING_DECISION);
            patient->state = PS_GO_FOR_TREATMENT;
            break;
        case EV_POACH_STAFF:
            employee = &thing[D_001549d0[i].value];
            if (employee > thing) {
                employee->poach_offers = 1 << (u8)i;
            }
            break;
        case EV_TOGGLE_VACCINATION:
            patient = &thing[D_001549d0[i].value];
            if (patient->flags0 & PF_VACCINE_REQUESTED) {
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_DEVACCIN, -1, -1);
                }
                cancel_vaccination_request(patient);
                if (patient->flags0 & PF_CONTAGIOUS) {
                    set_icon_value(patient, ICON_CONTAGIOUS);
                }
            } else {
                if (i == player) {
                    play_sound(SNDSRC_GENERAL, SFX_VACCIN, -1, -1);
                }
                request_vaccination(patient);
            }
            break;
        case EV_SETUP_PLACE_OBJECT_REQUESTER:
            room_info[allhosps[i].edited_room].state = RS_EDITED;
            room_info[allhosps[i].edited_room].being_edited = 0;
            if (i == player) {
                setup_place_object_requester();
                set_mouse_mode(MM_NORMAL);
            }
            break;
        case EV_CLOSE_QUEUE_WINDOW:
            if (i == player) {
                othervar.queue_pos = 0;
                othervar.queue_hilight_thing = 0;
                display_mode = DM_GAME;
                close_irequesters();
                close_crequester(REQ_QUEUE);
                window.current_window = 0;
                set_mouse_mode(MM_NORMAL);
                pmenu.disabled = 0;
            }
            break;
        case 0x85:
            break;
        case EV_FORCE_CURE:
            force_peep_to_be_cured(&thing[D_001549d0[i].value]);
            break;
        case EV_RETURN_TO_THING_WINDOW:
            if (othervar.cur_player == player) {
                lbDisplay.rleft_button = lbDisplay.rright_button = lbDisplay.left_button = lbDisplay.right_button = 0;
                return_from_screen(D_001549d0[i].value, 1, 1);
                if (i == player) {
                    Thing *obj;
                    int kind = 0;
                    u8 slot = 0;

                    obj = &thing[D_001549d0[i].arg];
                    if (obj->owner == player) {
                        switch (obj->type) {
                        case THING_PATIENT:
                            slot = 1;
                            D_001581b2[slot].kind = REQ_PATIENT;
                            D_001581b2[slot].show_graph = 1;
                            D_001581b2[slot].toggle_delay = 0;
                            kind = 0x22;
                            break;
                        case THING_STAFF:
                            switch (obj->subtype) {
                            case STAFF_HANDYMAN:
                                slot = 2;
                                D_001581b2[slot].kind = REQ_HANDYMAN;
                                kind = 0x21;
                                break;
                            default:
                                slot = 2;
                                D_001581b2[slot].kind = REQ_STAFF;
                                kind = 0x20;
                                break;
                            }
                            break;
                        case THING_OBJECT:
                            slot = 3;
                            D_001581b2[slot].kind = REQ_MACHINE;
                            break;
                        }
                    }
                    if (slot) {
                        close_irequesters();
                        cancel_top_level_requesters(REQ_NONE);
                        if (crequester.id != REQ_NONE) {
                            break;
                        }
                        othervar.irequester_slot = slot;
                        open_requester(D_001581b2[slot].kind, 1);
                        D_001581b2[slot].value = D_001549d0[i].arg;
                        set_mouse_sprite(PTR_DEFAULT);
                    }
                    if (kind) {
                        int x = resvars.window_list[slot].x;
                        int y = resvars.window_list[slot].y;

                        open_new_window(0, 0, 0, x + resvars.boxes_Rectp[kind].x0, y + resvars.boxes_Rectp[kind].y0, x + resvars.boxes_Rectp[kind].x1, y + resvars.boxes_Rectp[kind].y1);
                        othervar.track_window[slot] = window.window_count - 1;
                        window.current_window = 0;
                    }
                }
            }
            break;
        case EV_CANCEL_SACK:
            if (i == player) {
                Thing *obj;
                int kind = 0;
                u8 slot = 0;

                close_yes_no_requester(1);
                close_irequesters();
                if (othervar.no_arg) {
                    set_mouse_mode((u8)othervar.no_arg);
                    lbDisplay.left_button = 0;
                }
                obj = &thing[D_001549d0[i].value];
                if (obj->owner == player) {
                    switch (obj->subtype) {
                    case STAFF_HANDYMAN:
                        slot = 2;
                        D_001581b2[slot].kind = REQ_HANDYMAN;
                        kind = 0x21;
                        break;
                    default:
                        slot = 2;
                        D_001581b2[slot].kind = REQ_STAFF;
                        kind = 0x20;
                        break;
                    }
                }
                if (slot) {
                    close_irequesters();
                    othervar.irequester_slot = slot;
                    open_requester(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    set_mouse_sprite(PTR_DEFAULT);
                }
                if (kind) {
                    int x = resvars.window_list[slot].x;
                    int y = resvars.window_list[slot].y;

                    open_new_window(0, 0, 0, x + resvars.boxes_Rectp[kind].x0, y + resvars.boxes_Rectp[kind].y0, x + resvars.boxes_Rectp[kind].x1, y + resvars.boxes_Rectp[kind].y1);
                    othervar.track_window[slot] = window.window_count - 1;
                    window.current_window = 0;
                }
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
            break;
        case EV_CHEAT_END_YEAR:
            mycontrol->month = 0xb;
            mycontrol->day = calendar[mycontrol->month] - 2;
            break;
        case EV_REPLACE_MACHINE:
            if (i == player) {
                close_yes_no_requester(0);
            }
            replace_machine(&thing[D_001549d0[i].value]);
            break;
        case EV_ANNOUNCE:
            allhosps[i].opening_countdown = 0;
            if (statvar.num_players <= 1) {
                allhosps[i].hospital_open = 1;
            }
            if (i != player) {
                announce_this(ANN_RIVAL_OPENED, i);
            }
            break;
        case EV_CLOSE_SAVE_YES_NO:
            if (i == player) {
                close_yes_no_requester((u8)D_001549d0[i].value);
                if (othervar.no_arg) {
                    set_mouse_mode((u8)othervar.no_arg);
                    lbDisplay.left_button = 0;
                }
                if (othervar.yes_value < 10) {
                    othervar.save_slot_used[othervar.yes_value] = 1;
                }
                check_save_games();
            }
            break;
        case EV_NET_WIN:
            if (i == player) {
                othervar.net_result = 0;
            } else {
                othervar.net_result = 1;
            }
            break;
        case EV_RESTART_LEVEL:
            close_yes_no_requester(0);
            restart_level();
            break;
        default:
            break;
        }
    copy:
        D_00154a0c[i].type = D_001549d0[i].type;
        D_00154a0c[i].value = D_001549d0[i].value;
        D_00154a0c[i].arg = D_001549d0[i].arg;
        D_00154a0c[i].checksum = D_001549d0[i].checksum;
        D_001549d0[i].type = EV_NONE;
        D_001549d0[i].value = 0;
        D_001549d0[i].arg = 0;
    }
    othervar.cur_player = player;
    check_autosave();
}

// Returns 1 if save slot slot holds a valid save (the right length and
// version 3), loading its level number and balance settings; else sets the
// error message to show and returns 0.
// FUNCTION: 0x00021700
s16 save_game_valid(const s8 slot)
{
    char buf[64];
    int handle;

    sprintf(buf, "%s%s%d", statvar.save_root, statvar.save_prefix, slot);
    if (LbFileLengthRnc(buf) == statvar.save_game_size) {
        handle = LbFileOpen(buf, 2);
        if (handle != -1) {
            LbFileRead(handle, &version, 2);
            if (version == 3) {
                LbFileRead(handle, &level_number, 1);
                LbFileRead(handle, balance_id, 10);
                LbFileClose(handle);
                load_balance((u16)level_number);
                return 1;
            } else {
                LbFileClose(handle);
                othervar.panel_message = 0x1c;
                othervar.panel_message_time = 0x46;
            }
        } else {
            othervar.panel_message = 0x1c;
            othervar.panel_message_time = 0x46;
        }
    } else {
        othervar.panel_message = 0x1c;
        othervar.panel_message_time = 0x46;
    }
    return 0;
}

// Saves the game regions to save slot slot (port -1), or sends them to network
// player port for resynchronisation; in low resolution the view is saved as
// if in high resolution. Returns 1 on success, else 0 (with a message for a
// local save). Not while building or editing in single player.
// FUNCTION: 0x00021818
u16 save_game(s8 slot, s8 port)
{
    s32 cell;
    Thing *w;
    char buf[64];
    int handle;
    u16 i;
    u16 ok;
    int tries;

    if (statvar.num_players > 1 && port == -1) {
        return 0;
    }
    if (statvar.num_players <= 1) {
        if (!check_okay_to_save(1)) {
            return 0;
        }
    }
    if (resvars.game_mode == SCREEN_MODE_1) {
        find_cell_at_point(window.current_window, window.center_x[window.current_window], window.center_y[window.current_window]);
        cell = info.cell;
        window.zone0_x = 0;
        window.zone0_x1 = lbScreenModeInfo[SCREEN_MODE_13].width;
        window.zone0_y = 0;
        window.zone0_y1 = lbScreenModeInfo[SCREEN_MODE_13].height;
        resvars.tile_width = 0x40;
        resvars.tile_height = 0x20;
        window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
        window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
        goto_cell_in_window(window.current_window, cell);
    }
    setup_addresses();
    for (w = thing + 1; w < end_things; w++) {
        if ((w->type == THING_STAFF || w->type == THING_PATIENT) && w->room > 0) {
            w->room_index = w->room - room_info;
        }
    }
    if (port == -1) {
        sprintf(buf, "%s%s%d", statvar.save_root, statvar.save_prefix, slot);
        handle = LbFileOpen(buf, 0);
        if (handle != -1) {
            for (i = 0; D_001290d4[i].addr; i++) {
                LbFileWrite(handle, (void *)D_001290d4[i].addr, D_001290d4[i].size);
            }
            LbFileClose(handle);
            othervar.panel_message = 0x18;
            othervar.panel_message_time = 3;
            save_player_details();
            ok = 1;
        } else {
            othervar.panel_message = 0x19;
            othervar.panel_message_time = 0x14;
            ok = 0;
        }
    } else {
        ok = 0;
        tries = 0;
        do {
        } while (LbNetwork_Send(port, D_0015fe86, 0x400) == -1 && tries++ < 8);
        if (tries < 8) {
            ok = 1;
            for (i = 0; D_001290d4[i].addr; i++) {
                if (is_checksummed_region(i)) {
                    if (!sync_memory_region(port, D_001290d4[i].addr, D_001290d4[i].size)) {
                        ok = 0;
                        break;
                    }
                }
            }
        }
    }
    for (w = thing + 1; w < end_things; w++) {
        if ((w->type == THING_STAFF || w->type == THING_PATIENT) && w->room > 0) {
            w->room = &room_info[w->room_index];
        }
    }
    if (resvars.game_mode == SCREEN_MODE_1) {
        window.zone0_x = 0;
        window.zone0_x1 = lbScreenModeInfo[SCREEN_MODE_1].width;
        window.zone0_y = 0;
        window.zone0_y1 = lbScreenModeInfo[SCREEN_MODE_1].height;
        resvars.tile_width = 0x20;
        resvars.tile_height = 0x10;
        window.anim_mode0 = 0;
        window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
        window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
        window.current_window = 0;
        goto_cell_in_window(window.current_window, cell);
    }
    return ok;
}

// Loads save slot slot (port -1) over a freshly loaded level and rebuilds the
// map, or receives the game regions from the network host. Returns 1 on
// success, else 0.
// FUNCTION: 0x00021c38
s16 load_game(s8 slot, s8 port)
{
    u8 saved_level;
    u16 ok;
    char buf[64];
    int handle;
    u16 i;
    s32 cell;
    Thing *w;
    int tries;

    saved_level = myhosp->level;
    if (statvar.num_players > 1 && port == -1) {
        return 0;
    }
    if (statvar.num_players <= 1) {
        if (save_game_valid(slot) == 0) {
            return 0;
        }
    }
    if (port == -1) {
        reset_earthquake_variables();
        close_irequesters();
        if (crequester.id != REQ_NONE) {
            close_crequester(crequester.id);
        }
        if (load_level(level_number, 1)) {
            ok = 1;
            setup_addresses();
            sprintf(buf, "%s%s%d", statvar.save_root, statvar.save_prefix, slot);
            handle = LbFileOpen(buf, 2);
            if (handle != -1) {
                for (i = 0; D_001290d4[i].addr; i++) {
                    LbFileRead(handle, (void *)D_001290d4[i].addr, D_001290d4[i].size);
                }
                LbFileClose(handle);
            }
            if (resvars.game_mode == SCREEN_MODE_1) {
                window.zone0_x = 0;
                window.zone0_x1 = lbScreenModeInfo[SCREEN_MODE_13].width;
                window.zone0_y = 0;
                window.zone0_y1 = lbScreenModeInfo[SCREEN_MODE_13].height;
                resvars.tile_width = 0x40;
                resvars.tile_height = 0x20;
                window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
                window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
                find_cell_at_point(window.current_window, window.center_x[window.current_window], window.center_y[window.current_window]);
                cell = info.cell;
                window.zone0_x = 0;
                window.zone0_x1 = lbScreenModeInfo[SCREEN_MODE_1].width;
                window.zone0_y = 0;
                window.zone0_y1 = lbScreenModeInfo[SCREEN_MODE_1].height;
                window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
                window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
                resvars.tile_width = 0x20;
                resvars.tile_height = 0x10;
                goto_cell_in_window(window.current_window, cell);
            }
            for (w = thing + 1; w < end_things; w++) {
                if ((w->type == THING_PATIENT || w->type == THING_STAFF) && w->room > 0) {
                    w->room = &room_info[w->room_index];
                }
            }
            recreate_map(1);
            othervar.full_redraw = 1;
            reset_advisor();
            myhosp = &allhosps[player];
            if (statvar.num_players > 1) {
                myhosp->level = saved_level;
            }
            othervar.selected_illness = get_next_illness(1, 0);
            free_level();
            return 1;
        }
    } else {
        ok = 0;
        tries = 0;
        do {
        } while (LbNetwork_Receive(D_00160508, D_0015fe86, 0x400) != 0x400 && tries++ < 8);
        if (tries < 8) {
            u8 found;

            found = 0;
            ok = 1;
            setup_addresses();
            for (i = 0; D_001290d4[i].addr; i++) {
                if (is_checksummed_region(i)) {
                    if (!sync_memory_region(-1, D_001290d4[i].addr, D_001290d4[i].size)) {
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
                    if ((w->type == THING_PATIENT || w->type == THING_STAFF) && w->room > 0) {
                        w->room = &room_info[w->room_index];
                    }
                }
            }
            othervar.full_redraw = 1;
            reset_advisor();
            myhosp = &allhosps[player];
            if (statvar.num_players > 1) {
                myhosp->level = saved_level;
            }
        }
        return ok;
    }
    return 0;
}

// Points the save regions (D_001290d4) at the current map, things, rooms and
// other dynamic tables; in network games also totals their size and, on the
// host, checksums them.
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
    if (statvar.num_players > 1) {
        D_0016028c = 0;
        for (i = 0; D_001290d4[i].addr; i++) {
            if (is_checksummed_region(i)) {
                D_0016028c += D_001290d4[i].size;
            }
        }
        if (D_00160508 == player) {
            checksum_memory_regions();
        }
    }
}

// Loads level file lvl (level.l<lvl>) into the cache. With full set, also
// creates each player's entrance rooms and sets the start view, helipad and
// scroll limits from it and rebuilds the map. A level already loaded is kept.
// Returns 1, or 0 on failure.
// FUNCTION: 0x000221fc
s8 load_level(const u8 lvl, const u8 full)
{
    char buf[144];
    RoomInfo *room;
    u16 i;
    u16 j;

    if (lvl) {
        if (othervar.loaded_level == lvl) {
            return 1;
        }
        if (othervar.loaded_level > 0) {
            free_level();
        }
    }
    clear_sound_heap();
    othervar.level_block = cache.Allocate(0x2806c, 0x63);
    if (othervar.level_block) {
        level = (Level *)fn_00022b60(othervar.level_block);
    } else {
        return 0;
    }
    if (install_type < 3) {
        sprintf(buf, "%slevels/level.l%d", cd_install_path, lvl);
        check_cd_available(buf);
    } else {
        sprintf(buf, "levels/level.l%d", lvl);
    }
    if (LbFileLoadAt(buf, level) == LbFileLengthRnc(buf)) {
        if (!full) {
            othervar.loaded_level = lvl;
            recreate_map(1);
            return 1;
        }
        if (level->player_count < 1 || level->player_count > 4) {
            level->player_count = 1;
        }
        room = room_info + 1;
        for (i = 0; i < 4; i++) {
            if (D_001609c4[i].active || (statvar.num_players <= 1 && i == player)) {
                for (j = 0; j < 4; j++) {
                    for (; room < end_room_info; room++) {
                        if (room->type == ROOM_FREE) {
                            room->type = ROOM_ENTRANCE;
                            switch (j) {
                            case 0:
                                room->pos = room->door_tile = room->door_inside_tile = level->entrances[i][0];
                                break;
                            case 1:
                                room->pos = room->door_tile = room->door_inside_tile = level->entrances[i][1];
                                break;
                            case 2:
                                room->pos = room->door_tile = room->door_inside_tile = level->entrances[i][2];
                                break;
                            case 3:
                                room->pos = room->door_tile = room->door_inside_tile = level->entrances[i][3];
                                break;
                            }
                            room->owner = i;
                            allhosps[i].entrances[j] = room - room_info;
                            break;
                        }
                    }
                }
                allhosps[i].heli_pad_tile = level->emergency_tile[i];
            }
            goto_column_row_in_window(window.current_window, level->camera_start[player] % 128, level->camera_start[player] / 128);
            gamevar.scroll_min_tile = level->scroll_min;
            gamevar.scroll_max_tile = level->scroll_max;
        }
        for (i = 0; i < 4; i++) {
            fn_000c21e6(&allhosps[i].entrances, &allhosps[D_00160508].entrances, 8);
        }
        recreate_map(0);
        add_static_shadows();
        othervar.full_redraw = 1;
        othervar.loaded_level = lvl;
        return 1;
    }
    return 0;
}

// Frees the loaded level file.
// FUNCTION: 0x00022764
void free_level(void)
{
    othervar.loaded_level = 0;
    cache.Free(othervar.level_block);
}

// Returns the game state checksum compared between network players: the
// random seed plus each active hospital's allhosps.build.objects entries.
// FUNCTION: 0x00022790
u32 check_sum(void)
{
    u32 sum;
    int j;
    int i;

    sum = 0;
    sum = seed;
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active) {
            for (j = 0; j < 20; j++) {
                sum += allhosps[i].build.objects[j];
            }
        }
    }
    return sum;
}

// Clears the earthquake state.
// FUNCTION: 0x00022820
void reset_earthquake_variables(void)
{
    gamevar.quake_strength = 0;
    gamevar.quake_shake_index = 0;
    gamevar.quake_ramp_u = 1;
    gamevar.quake_phase = 0;
}

// Returns 1 if the game can be saved now (normal mouse mode, no building or
// room editing in progress); else 0, setting the error message if warn.
// FUNCTION: 0x00022854
u8 check_okay_to_save(u8 warn)
{
    if (gamevar.mouse_mode != MM_NORMAL) {
        if (warn) {
            othervar.panel_message = 0x19;
            othervar.panel_message_time = 0x14;
        }
        return 0;
    }
    if (is_player_building() || allhosps[player].edited_room) {
        if (warn) {
            othervar.panel_message = 0x19;
            othervar.panel_message_time = 0x14;
        }
        return 0;
    }
    return 1;
}

// Autosaves to slot 8 on day 28 of each month (or as soon as possible after,
// if saving was not possible then), when autosave is on (myplayer.autosave).
// Single player only.
// FUNCTION: 0x000228e0
void check_autosave(void)
{
    if (myplayer.autosave) {
        if (statvar.num_players <= 1) {
            if (mycontrol->day == 0x1c || need_a_save) {
                if (mycontrol->month + mycontrol->day != last_saved || need_a_save) {
                    if (check_okay_to_save(0)) {
                        save_game(8, -1);
                        last_saved = mycontrol->month + mycontrol->day;
                        need_a_save = 0;
                    } else {
                        need_a_save = 1;
                    }
                }
            }
        }
    }
}

// Returns the level number stored in save slot slot, or -1 if it cannot be
// read. Reads two bytes into a one-byte local.
// FUNCTION: 0x000229b0
s32 read_level_number(s32 slot)
{
    int handle;
    char buf[64];
    u8 level_no;

    sprintf(buf, "%s%s%d", statvar.save_root, statvar.save_prefix, slot);
    handle = LbFileOpen(buf, 2);
    if (handle == -1) {
        return -1;
    }
    if (LbFileSeek(handle, 2, 0) != -1) {
        if (LbFileRead(handle, &level_no, 2) != -1) {
            LbFileClose(handle);
            return level_no;
        }
    }
    LbFileClose(handle);
    return -1;
}

// Restarts the current level from the beginning (single player only).
// FUNCTION: 0x00022a58
void restart_level(void)
{
    if (statvar.num_players <= 1) {
        reset_earthquake_variables();
        reset_advisor();
        close_irequesters();
        if (crequester.id != REQ_NONE) {
            close_crequester(crequester.id);
        }
        D_00156910 = 1;
        setup_new_level((u16)level_number, 1);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        goto_column_row_in_window(window.current_window, level->camera_start[player] % 128, level->camera_start[player] / 128);
        othervar.show_level_brief = 1;
        draw_it();
        othervar.pending_palette = palette;
        init_ambients();
    }
}

// Returns the data of cache block block (just past its 0x14-byte header).
// FUNCTION: 0x00022b60 COMDAT
inline void *fn_00022b60(TbCacheUsedBlock *block)
{
    return (u8 *)block + 0x14;
}
