#include "hosp.h"

struct Settings {
    u8 unknown0[3];
    TbScreenMode mode;
};

extern "C" {

extern char D_00120174[];
extern char D_0013f486[];
extern u32 D_00155c5a;
extern Settings D_001553e1;
extern u8 D_0012d8d8[];
extern u32 D_001606bc;
extern u16 D_0015542a;
extern u8 D_00155e39;
extern u8 D_00155c68;
extern u16 D_00155e1b;
extern u8 *D_001553d7;
extern u8 D_001553db;
extern u8 *D_001553dc;
extern u8 D_00155e14, D_00155e15, D_00155e16;
extern u8 D_00155d70;
extern u8 D_00155e4c;
extern u8 D_0015f99f;
extern u8 D_001553c8;
extern u16 D_00155e2a;
extern u8 D_001553ea;
extern u8 D_001553b8[];
extern u8 D_001553c6, D_001553e7;
extern u16 D_0015543a, D_0015543c;
extern u8 D_0015543e;
extern s16 D_00155c64[2];
extern u8 D_0012b7c8[];
extern Event D_001549d0[];
extern u8 *D_00153f34;
extern u8 D_00155e1d;
extern u8 D_001553e8;
extern u16 D_00188c74;
extern u32 D_00188c38;
extern u8 D_00156910;
extern u8 D_00155e33[];
extern u8 D_00155e07;
extern u16 D_00155e37;
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c7f;

void fn_00090610(void);
void fn_000110b0(void);
int fn_000bed50(void);
void fn_0008add8(void);
void fn_0004e47c(Window **windows);
void fn_000a6ca4(void);
void fn_00063680(void);
void fn_00063a64(void);
u8 fn_00029a54(RoomInfo *room);
void fn_000b4e88(void);
void fn_000951c8(void);
void fn_00094b28(void);

}

// FUNCTION: 0x000112b0
void game(void)
{
    u8 redraw;
    u8 quit;
    int i;
    char path[128];

    quit = 0;
    sprintf(path, D_00120174, cd_install_path, D_0013f486);
    check_cd_available(path);
    get_cd_speed();
    fn_00090610();
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
        D_00155c5a = LbTimerClock();
        get_input();
        if (D_001553e1.mode) {
            if (!change_res(D_001553e1.mode)) {
                break;
            }
            D_001553e1.mode = (TbScreenMode)0;
        }
        check_if_active();
        if (fn_000bed50() != 1) {
            redraw = D_0012d8d8[display_mode];
            if (display_mode == 13) {
                redraw = process_lost_focus_display(0);
            } else {
                set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
            }
        } else {
            if (D_0012d8d8[display_mode]) {
                start_turn();
            }
            redraw = D_0012d8d8[display_mode];
            D_001606bc++;
            switch (display_mode) {
            case 0:
                draw_it();
                do_net_text();
                fn_0008add8();
                fn_00074220(HospShift, lbInkey);
                fn_0004e47c(open_windows);
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
                fn_00074220(HospShift, lbInkey);
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
                fn_00063680();
                fn_0004e47c(open_windows);
                fn_00063a64();
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
                if (D_0015542a != 0) {
                    switch (D_0015542a) {
                    case 1:
                    case 2:
                        play_winlose();
                        break;
                    default:
                        D_0015542a = 0;
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
                fn_0004e47c(open_windows);
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
                LbDrawBoxCoords(&choice_backup, &year_end_backup, 10);
                play_winlose();
                process_turn();
                break;
            case 13:
                redraw = process_lost_focus_display(1);
                break;
            case 99:
                process_turn();
                break;
            case 14:
                fn_00063680();
                fn_0004e47c(open_windows);
                fn_00063a64();
                process_mouse_town_select();
                process_sound_heap();
                process_turn();
                if (town.unknown2 >= 0x45 && statvar.unknownfd > 1) {
                    redraw = 0;
                }
                break;
            default:
                break;
            }
            LbIsActive();
        }
        if (debug.unknownd4 != 0) {
            if (debug.unknownd4 == 1) {
                debug.unknownd4 = 0;
            }
            LbMousePlace();
            while (fn_000bed50() != 1) {
            }
            screen_shot();
            LbIsActive();
            LbMouseRemove();
        }
        limit_framerate();
        if ((display_mode == 0 || display_mode == 11) && resvars.f0 == SCREEN_MODE_13 && D_00155e39 == 0) {
            special_refresh();
        } else {
            if (D_00155e39) {
                D_00155e39--;
            }
            LbScreenSwap();
        }
        if (redraw && !gamevar.unknown21 && gamevar.f12d == 0 && frame && quit_game != 1) {
            process_things(thing + 1, end_things);
            process_blueprint_zones();
            if (gamevar.f139 || D_00155c68 || D_00155e1b) {
                for (i = 0; D_00155e1b + 2 > i; i++) {
                    process_things(thing + 1, end_things);
                    process_blueprint_zones();
                    start_turn();
                }
                D_00155c68 = 0;
                D_00155e1b = 0;
            }
        }
        if (D_001553d7) {
            hosp_fade_palette(D_001553d7, 10, Lb_PALETTE_FADE_CLOSED);
            current_palette = D_001553d7;
            D_001553d7 = 0;
            setup_colors();
        }
        if (D_001553db) {
            hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
            D_001553db = 0;
        }
        if (D_001553dc) {
            LbPaletteSet(D_001553dc);
            current_palette = D_001553dc;
            D_001553dc = 0;
            setup_colors();
        }
        if (D_00155e14 || D_00155e15 || D_00155e16) {
            if (display_mode == 0 && crequester.unknown2 == 0x63 && D_00155d70 == 0 && gamevar.f12e != 0x25 && !D_00155e4c
                && gamevar.f12e != 3) {
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
            fn_00021818(0x63, -1);
            stop_midi();
            if (debug.unknown1fa) {
                quit_game = 1;
            }
            break;
        case 2:
            D_0015f99f = 1;
            quit_game = 0;
            D_001553c8 = 1;
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
    u8 count;
    u8 i;
    u32 now;
    RoomInfo *obj;

    now = LbTimerClock();
    game_turn++;
    D_00155e2a = 0;
    if (gamevar.f138) {
        myplayer.f1es = 0x7d00;
    }
    if (now - gamecounts.unknown14 < 1000 / myplayer.f1es && D_00155c68 == 0 && statvar.unknownfd <= 1) {
        gamevar.f12d = 1;
        return;
    }
    gamecounts.unknown14 = now;
    gamevar.f12d = 0;
    if (gamevar.unknown21) {
        return;
    }
    actual_game_turn++;
    D_001553ea = (actual_game_turn & 0xf) == 0;
    *(u16 *)D_001553b8 = old_newrand() % 100;
    D_001553c6 = old_newrand() % 6;
    D_001553e7 = (old_newrand() & 3) + 1;
    D_0015543a = actual_game_turn;
    D_0015543c = actual_game_turn + gbv.fe4;
    D_0015543e = (actual_game_turn & 0x1f) == 0;
    if (D_00155c64[1] > 0) {
        D_00155c64[1]--;
    }
    if (actual_game_turn % 25 == 0) {
        twice_daily_admin();
        if (actual_game_turn % 50 == 0) {
            daily_admin();
            if ((int)mycontrol->unknown64f > (int)D_0012b7c8[mycontrol->unknown650]) {
                month_end();
                if (mycontrol->unknown650 >= 12) {
                    year_end();
                } else if (mycontrol->unknown650 % 3 == 0) {
                    check_quarterly_game_over();
                }
            }
        }
    }
    count = 1;
    if (statvar.unknownfd > 1) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
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
                play_sound(10, 0xd9, -1, -1);
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
    obj = &room_info[allhosps[player].unknown11b7];
    if (obj > room_info) {
        if (obj->f22 && fn_00029a54(&room_info[allhosps[player].unknown11b7])) {
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
        switch (D_00155e4c) {
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
    s32 width = mouse_dx;
    s32 height = mouse_dy;

    if (resvars.f0 == SCREEN_MODE_13 && mode != SCREEN_MODE_13) {
        width = width / 2;
        height = height * 5 / 12;
    } else if (resvars.f0 != SCREEN_MODE_13 && mode == SCREEN_MODE_13) {
        width <<= 1;
        height = height * 12 / 5;
    }
    if (!setup_any_screen(D_00153f34, mode)) {
        return 0;
    } else {
        TbSprite *font;

        if (D_00155e1d) {
            font = screen_pointers_tab_start;
        } else {
            font = pointer;
        }
        LbMouseSetup(font + D_001553e8);
        LbMouseSetPosition(width, height);
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
        fn_000b4e88();
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    default:
        D_001549d0[player].type = 0x34;
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
            fn_000951c8();
        }
        yearend.unknown2 = 0;
        exit_the_yearend_screen = 0;
        gamevar.unknown21 = 1;
        StopAllSamples();
        break;
    case 1:
        fn_00094b28();
        break;
    case 2:
        fn_000b4e88();
        break;
    case 3:
        year_end_type = 4;
        break;
    case 4:
        LbDrawBoxCoords(&choice_backup, &year_end_backup, 10);
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
        D_00188c74 = 4;
        LbDrawBox(0, 0, D_00188c34, D_00188c38, color.unknown9);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        D_00188c74 = 0;
        D_00156910 = 1;
    }
}

// FUNCTION: 0x00012400
void check_if_active(void)
{
    if (statvar.unknownfd > 1) {
        if (LbDate() && D_00155e33[player] != 0) {
            D_00155e39 = 0xf;
            D_001549d0[player].type = 0x8e;
            if (D_00155e07) {
                AIL_resume_sequence(GetSongHandle());
            }
        } else if (!LbDate() && D_00155e33[player] == 0) {
            D_001549d0[player].type = 0x8d;
            if (has_any_player_lost_focus()) {
                D_00155e33[player] = 2;
            } else {
                D_00155e33[player] = 1;
            }
            if (display_mode != 0xd) {
                D_00155e37 = display_mode;
                display_mode = 0xd;
            }
            if (D_00155e07) {
                AIL_end_sequence(GetSongHandle());
            }
        }
    }
}

// FUNCTION: 0x00012500
u8 process_lost_focus_display(u8 flag)
{
    u8 result;

    if (D_00155e33[player] == 1) {
        start_turn();
        result = 1;
    } else {
        result = 0;
    }
    if (flag) {
        switch (D_00155e37) {
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
    D_00188c7b = D_00188c7c = 0;
    D_00188c7e = D_00188c7f = 0;
    if (statvar.unknownfd <= 1) {
        show_help_page(1, level_number);
        D_00155e14 = 0;
    } else if (D_00155e16) {
        if (D_00155e16 == 3) {
            play_sound(10, 0xaf, -1, -1);
        }
        show_help_page(3, D_00155e16);
        D_00155e16 = 0;
    } else {
        show_help_page(2, current_net_criteria.unknown0);
        D_00155e15 = 0;
    }
}
