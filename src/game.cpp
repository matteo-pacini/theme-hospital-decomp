#include "hosp.h"

extern "C" {

extern u32 menu_turn;
extern u8 new_game;
extern Event D_001549d0[];
extern u8 *palette;
extern u8 D_00156910;

void start_game(void);
void fn_000110b0(void);
int LbScreenLock(void);
void process_menus(void);
void ProcessWindows(Window **windows);
void fn_000a6ca4(void);
void draw_town_select(void);
void process_keys_town_select(void);
u8 fn_00029a54(RoomInfo *room);
void display_year_charts(void);
void insert_hall_details(void);
void display_hall(void);

}

// FUNCTION: 0x000112b0
void game(void)
{
    u8 run_things;
    u8 unused;
    int i;
    char path[128];

    unused = 0;
    sprintf(path, "%s%s", cd_install_path, animation_table[25].name);
    check_cd_available(path);
    get_cd_speed();
    start_game();
    if (debug.unknown1fa) {
        display_mode = 0;
        gamevar.unknown21 = 0;
        fn_000110b0();
    } else {
        display_mode = 2;
        mainmenu.unknown2 = 0x17;
    }
    process_turn();
    while (quit_game == 0) {
        othervar.f8a2 = LbTimerClock();
        get_input();
        if (othervar.f2c) {
            if (!change_res(othervar.f2c)) {
                break;
            }
            othervar.f2c = (TbScreenMode)0;
        }
        check_if_active();
        if (LbScreenLock() != 1) {
            run_things = display_mode_control[display_mode];
            if (display_mode == 13) {
                run_things = process_lost_focus_display(0);
            } else {
                set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
            }
        } else {
            if (display_mode_control[display_mode]) {
                start_turn();
            }
            run_things = display_mode_control[display_mode];
            menu_turn++;
            switch (display_mode) {
            case 0:
                draw_it();
                do_net_text();
                process_menus();
                process_keys(HospShift, lbInkey);
                ProcessWindows(open_windows);
                clear_object_drag_shape();
                process_mouse();
                display_advisor();
                play_next_midi();
                manage_sound();
                process_sound_heap();
                manage_background_sounds();
                do_ambients();
                fn_000a6ca4();
                DrawWindows(open_windows);
                process_turn();
                draw_pause_mode();
                draw_help_text();
                break;
            case 1:
                do_charts();
                process_keys(HospShift, lbInkey);
                process_sound_heap();
                draw_help_text();
                process_turn();
                break;
            case 2:
                draw_main_menu();
                process_keys_main_menu();
                process_mouse_main_menu();
                process_sound_heap();
                process_turn();
                break;
            case 3:
                if (!draw_area_select()) {
                    goto done;
                }
                process_keys_area_select();
                process_mouse_area_select();
                process_sound_heap();
                process_turn();
                break;
            case 4:
                draw_town_select();
                ProcessWindows(open_windows);
                process_keys_town_select();
                process_mouse_town_select();
                process_sound_heap();
                process_turn();
                break;
            case 5:
                draw_network_select();
                process_keys_network_select();
                process_mouse_network_select();
                process_sound_heap();
                break;
            case 6:
                draw_load_game();
                process_keys_load_game();
                process_mouse_load_game();
                process_sound_heap();
                manage_sound();
                break;
            case 7:
                if (othervar.f72 != 0) {
                    switch (othervar.f72) {
                    case 1:
                    case 2:
                        play_winlose();
                        break;
                    default:
                        othervar.f72 = 0;
                        break;
                    }
                }
                process_turn();
                break;
            case 8:
                do_year_end();
                manage_sound();
                process_sound_heap();
                draw_help_text();
                process_turn();
                break;
            case 9:
                display_choice();
                process_sound_heap();
                manage_sound();
                draw_help_text();
                process_turn();
                break;
            case 10:
                draw_nickname();
                process_keys_nickname();
                process_mouse_nickname();
                process_sound_heap();
                manage_sound();
                process_turn();
                break;
            case 11:
                draw_it();
                do_net_text();
                ProcessWindows(open_windows);
                req13_cancel_check();
                display_advisor();
                process_sound_heap();
                do_ambients();
                manage_sound();
                fn_000a6ca4();
                DrawWindows(open_windows);
                process_turn();
                draw_help_text();
                break;
            case 12:
                fn_000c21e6(&choice_backup, &year_end_backup, 10);
                play_winlose();
                process_turn();
                break;
            case 13:
                run_things = process_lost_focus_display(1);
                break;
            case 99:
                process_turn();
                break;
            case 14:
                draw_town_select();
                ProcessWindows(open_windows);
                process_keys_town_select();
                process_mouse_town_select();
                process_sound_heap();
                process_turn();
                if (town.unknown2 >= 0x45 && statvar.unknownfd > 1) {
                    run_things = 0;
                }
                break;
            default:
                break;
            }
            LbScreenUnlock();
        }
        if (debug.unknownd4 != 0) {
            if (debug.unknownd4 == 1) {
                debug.unknownd4 = 0;
            }
            LbMousePlace();
            while (LbScreenLock() != 1) {
            }
            screen_shot();
            LbScreenUnlock();
            LbMouseRemove();
        }
        limit_framerate();
        if ((display_mode == 0 || display_mode == 11) && resvars.f0 == SCREEN_MODE_13 && othervar.fa81 == 0) {
            special_refresh();
        } else {
            if (othervar.fa81) {
                othervar.fa81--;
            }
            LbScreenSwap();
        }
        if (run_things && !gamevar.unknown21 && gamevar.f12d == 0 && frame && quit_game != 1) {
            process_things(thing + 1, end_things);
            process_blueprint_zones();
            if (gamevar.f139 || othervar.f8b0 || othervar.fa63) {
                for (i = 0; othervar.fa63 + 2 > i; i++) {
                    process_things(thing + 1, end_things);
                    process_blueprint_zones();
                    start_turn();
                }
                othervar.f8b0 = 0;
                othervar.fa63 = 0;
            }
        }
        if (othervar.f1f) {
            hosp_fade_palette(othervar.f1f, 10, Lb_PALETTE_FADE_CLOSED);
            current_palette = othervar.f1f;
            othervar.f1f = 0;
            setup_colors();
        }
        if (othervar.f23) {
            hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
            othervar.f23 = 0;
        }
        if (othervar.f24) {
            LbPaletteSet(othervar.f24);
            current_palette = othervar.f24;
            othervar.f24 = 0;
            setup_colors();
        }
        if (othervar.fa5c || othervar.fa5d || othervar.fa5e) {
            if (display_mode == 0 && crequester.unknown2 == 0x63 && othervar.f9b8 == 0 && gamevar.f12e != MM_HELP && !othervar.fa94
                && gamevar.f12e != MM_PLACE_STAFF) {
                display_mission();
            }
        }
        switch (quit_game) {
        case 1:
            close_irequesters();
            if (crequester.unknown2 != 0x63) {
                close_crequester(crequester.unknown2);
            }
            mainmenu.unknown2 = 0x1f;
            display_mode = 2;
            fade_out_menu();
            LbScreenClear(0);
            LbScreenSwap();
            quit_game = 0;
            save_game(0x63, -1);
            stop_midi();
            if (debug.unknown1fa) {
                quit_game = 1;
            }
            break;
        case 2:
            new_game = 1;
            quit_game = 0;
            othervar.f10 = 1;
            break;
        default:
            break;
        }
    }
done:
    AIL_end_sequence(GetSongHandle());
}

// FUNCTION: 0x00011b24
void limit_framerate(void)
{
    u32 i;
    u32 now;

    while ((now = LbTimerClock()) - gamecounts.unknown18 < 0x28) {
        for (i = 0; i < 1000; i++) {
            wasteOfTime = wasteOfTime * 11 % (wasteOfTime - 10);
        }
    }
    gamecounts.unknown18 = now;
}

// FUNCTION: 0x00011b90
void start_turn(void)
{
    u8 hosp_count;
    u8 i;
    u32 now;
    RoomInfo *room;

    now = LbTimerClock();
    game_turn++;
    othervar.fa72 = 0;
    if (gamevar.f138) {
        myplayer.f1es = 0x7d00;
    }
    if (now - gamecounts.unknown14 < 1000 / myplayer.f1es && othervar.f8b0 == 0 && statvar.unknownfd <= 1) {
        gamevar.f12d = 1;
        return;
    }
    gamecounts.unknown14 = now;
    gamevar.f12d = 0;
    if (gamevar.unknown21) {
        return;
    }
    actual_game_turn++;
    othervar.f32 = (actual_game_turn & 0xf) == 0;
    othervar.f0 = old_newrand() % 100;
    othervar.fe = old_newrand() % 6;
    othervar.f2f = (old_newrand() & 3) + 1;
    othervar.f82 = actual_game_turn;
    othervar.f84 = actual_game_turn + gbv.fe4;
    othervar.f86 = (actual_game_turn & 0x1f) == 0;
    if (othervar.f8ac_s16a[1] > 0) {
        othervar.f8ac_s16a[1]--;
    }
    if (actual_game_turn % 25 == 0) {
        twice_daily_admin();
        if (actual_game_turn % 50 == 0) {
            daily_admin();
            if ((int)mycontrol->unknown64f > (int)calendar[mycontrol->unknown650]) {
                month_end();
                if (mycontrol->unknown650 >= 12) {
                    year_end();
                } else if (mycontrol->unknown650 % 3 == 0) {
                    check_quarterly_game_over();
                }
            }
        }
    }
    hosp_count = 1;
    if (statvar.unknownfd > 1) {
        hosp_count = 4;
    }
    for (i = 0; i < hosp_count; i++) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        allhosps[i].unknown11ee = allhosps[i].unknown11f0;
        allhosps[i].unknown11f0 = 0;
        if ((statvar.unknownfd > 1 ? 1 : allhosps[i].unknown1256) != 0 || (advisor.f11f & 1)) {
            continue;
        }
        allhosps[i].unknown1257--;
        if (!allhosps[i].unknown1257) {
            if (i != player) {
                announce_this(0xc, i);
            } else {
                play_sound(10, SFX_FANFARE, -1, -1);
            }
            if (statvar.unknownfd <= 1) {
                allhosps[i].unknown1256 = 1;
            }
        }
    }
    compute_whether_user_not_busy();
    if (gamevar.ff0s) {
        earthquake(gamevar.ff0s, gamevar.ff1);
    }
    room = &room_info[allhosps[player].unknown11b7];
    if (room > room_info) {
        if (room->f22 && fn_00029a54(&room_info[allhosps[player].unknown11b7])) {
            if (gamevar.f12e == 0xe) {
                if (room_info[allhosps[player].unknown11b7].first == 0) {
                    D_001549d0[player].type = 0x83;
                }
            } else {
                D_001549d0[player].type = 0x7b;
            }
        }
    }
    if (statvar.unknownfd > 1) {
        switch (othervar.fa94) {
        case 1:
            D_001549d0[player].type = 0x7e;
            D_001549d0[player].value = 1;
            break;
        case 2:
            D_001549d0[player].type = 0x90;
            break;
        default:
            break;
        }
    }
}

// FUNCTION: 0x00012068
u8 change_res(TbScreenMode mode)
{
    s32 mouse_x = lbDisplay.mmouse_x;
    s32 mouse_y = lbDisplay.mmouse_y;

    if (resvars.f0 == SCREEN_MODE_13 && mode != SCREEN_MODE_13) {
        mouse_x = mouse_x / 2;
        mouse_y = mouse_y * 5 / 12;
    } else if (resvars.f0 != SCREEN_MODE_13 && mode == SCREEN_MODE_13) {
        mouse_x <<= 1;
        mouse_y = mouse_y * 12 / 5;
    }
    if (!setup_any_screen(palette, mode)) {
        return 0;
    } else {
        TbSprite *cursor_sprites;

        if (othervar.fa65) {
            cursor_sprites = screen_pointers_tab_start;
        } else {
            cursor_sprites = pointer;
        }
        LbMouseSetup(cursor_sprites + othervar.f30);
        LbMouseSetPosition(mouse_x, mouse_y);
        return 1;
    }
}

// FUNCTION: 0x00012148
void do_charts(void)
{
    switch (chart_type) {
    case 0:
        bank_statement();
        break;
    case 1:
        display_drugs();
        break;
    case 2:
        display_policy();
        break;
    case 3:
        display_research();
        break;
    case 4:
        draw_graphs((u16)resvars.f3c->unknown2e0, (u16)resvars.f3c->unknown2e4, (u16)resvars.f3c->unknown2ec,
                    (u16)resvars.f3c->unknown2e8);
        break;
    case 5:
        display_slist();
        break;
    case 6:
        display_manager();
        break;
    case 7:
        display_peep_overview();
        break;
    case 13:
        display_year_charts();
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    default:
        D_001549d0[player].type = EV_CLOSE_SCREEN;
        break;
    }
}

// FUNCTION: 0x00012274
void do_year_end(void)
{
    switch (year_end_type) {
    case 0:
        if (statvar.unknownfd > 1) {
            year_end_type = 2;
            yearend.unknown0 = network_num_players();
        } else {
            hall.unknown2 = 0;
            year_end_type++;
            yearend.unknown0 = 1;
            insert_hall_details();
        }
        yearend.unknown2 = 0;
        exit_the_yearend_screen = 0;
        gamevar.unknown21 = 1;
        StopAllSamples();
        break;
    case 1:
        display_hall();
        break;
    case 2:
        display_year_charts();
        break;
    case 3:
        year_end_type = 4;
        break;
    case 4:
        fn_000c21e6(&choice_backup, &year_end_backup, 10);
        check_game_over();
        if (year_end_type == 4) {
            restore_display_screen();
        }
        break;
    case 5:
    case 6:
    case 7:
        play_winlose();
        break;
    }
}

// FUNCTION: 0x00012390
void draw_pause_mode(void)
{
    if (gamevar.unknown21) {
        lbDisplay.draw_flags = 4;
        LbDrawBox(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height, color.unknown9);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        lbDisplay.draw_flags = 0;
        D_00156910 = 1;
    }
}

// FUNCTION: 0x00012400
void check_if_active(void)
{
    if (statvar.unknownfd > 1) {
        if (LbIsActive() && othervar.fa7b[player] != 0) {
            othervar.fa81 = 0xf;
            D_001549d0[player].type = EV_NETWORK_GAINED_FOCUS;
            if (othervar.fa4f) {
                AIL_resume_sequence(GetSongHandle());
            }
        } else if (!LbIsActive() && othervar.fa7b[player] == 0) {
            D_001549d0[player].type = EV_NETWORK_LOST_FOCUS;
            if (has_any_player_lost_focus()) {
                othervar.fa7b[player] = 2;
            } else {
                othervar.fa7b[player] = 1;
            }
            if (display_mode != 0xd) {
                othervar.fa7f = display_mode;
                display_mode = 0xd;
            }
            if (othervar.fa4f) {
                AIL_end_sequence(GetSongHandle());
            }
        }
    }
}

// FUNCTION: 0x00012500
u8 process_lost_focus_display(u8 draw)
{
    u8 result;

    if (othervar.fa7b[player] == 1) {
        start_turn();
        result = 1;
    } else {
        result = 0;
    }
    if (draw) {
        switch (othervar.fa7f) {
        case 0:
        case 11:
            draw_it();
            break;
        default:
            LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
            LbDrawBox(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, color.unknown8);
            break;
        }
        draw_lost_focus_screen();
    }
    process_turn();
    return result;
}

// FUNCTION: 0x000125b8
void display_mission(void)
{
    if (advisor.f11f & 1) {
        return;
    }
    lbDisplay.left_button = lbDisplay.right_button = 0;
    lbDisplay.mleft_button = lbDisplay.mright_button = 0;
    if (statvar.unknownfd <= 1) {
        show_help_page(1, level_number);
        othervar.fa5c = 0;
    } else if (othervar.fa5e) {
        if (othervar.fa5e == 3) {
            play_sound(10, SFX_CHEER, -1, -1);
        }
        show_help_page(3, othervar.fa5e);
        othervar.fa5e = 0;
    } else {
        show_help_page(2, current_net_criteria.unknown0);
        othervar.fa5d = 0;
    }
}
