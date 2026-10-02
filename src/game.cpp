#include "hosp.h"

extern "C" {

extern u32 menu_turn;
extern u8 new_game;
extern Event player_events[];
extern u8 *palette;
extern u8 redraw_panel;

void start_game(void);
void use_hires_layout(void);
int LbScreenLock(void);
void process_menus(void);
void ProcessWindows(Window **windows);
void draw_other_windows(void);
void draw_town_select(void);
void process_keys_town_select(void);
u8 is_room_empty(RoomInfo *room);
void display_year_charts(void);
void insert_hall_details(void);
void display_hall(void);
}

// The game's main loop, until quit_game asks to leave or a mode change or the
// briefing fails: reads input, then runs the screen for display_mode (game
// view, full-screen charts, menus, briefing, town select, network, load, year
// end, fax, ...) and the turn's events, limits the frame rate, swaps the
// screen, runs the things for the turn (several times when fast forwarding),
// applies pending palette changes, shows the mission text when due, and
// handles quitting to the main menu or a new game.
// FUNCTION: 0x000112b0
void game(void)
{
    u8 run_things;
    u8 unused;
    int i;
    char path[128];

    unused = 0;
    sprintf(path, "%s%s", cd_install_path, animation_table[ANIM_INTRO_SM2].name);
    check_cd_available(path);
    get_cd_speed();
    start_game();
    if (debug.direct_start) {
        display_mode = DM_GAME;
        gamevar.paused = 0;
        use_hires_layout();
    } else {
        display_mode = DM_MAIN_MENU;
        mainmenu.state = MMS_INTRO;
    }
    process_turn();
    while (quit_game == 0) {
        othervar.clock_ms = LbTimerClock();
        get_input();
        if (othervar.pending_screen_mode) {
            if (!change_res(othervar.pending_screen_mode)) {
                break;
            }
            othervar.pending_screen_mode = (TbScreenMode)0;
        }
        check_if_active();
        if (LbScreenLock() != 1) {
            run_things = display_mode_control[display_mode];
            if (display_mode == DM_LOST_FOCUS) {
                run_things = process_lost_focus_display(0);
            } else {
                set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height,
                                                3);
            }
        } else {
            if (display_mode_control[display_mode]) {
                start_turn();
            }
            run_things = display_mode_control[display_mode];
            menu_turn++;
            switch (display_mode) {
            case DM_GAME:
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
                draw_other_windows();
                DrawWindows(open_windows);
                process_turn();
                draw_pause_mode();
                draw_help_text();
                break;
            case DM_CHARTS:
                do_charts();
                process_keys(HospShift, lbInkey);
                process_sound_heap();
                draw_help_text();
                process_turn();
                break;
            case DM_MAIN_MENU:
                draw_main_menu();
                process_keys_main_menu();
                process_mouse_main_menu();
                process_sound_heap();
                process_turn();
                break;
            case DM_AREA_SELECT:
                if (!draw_area_select()) {
                    goto done;
                }
                process_keys_area_select();
                process_mouse_area_select();
                process_sound_heap();
                process_turn();
                break;
            case DM_TOWN_MAP:
                draw_town_select();
                ProcessWindows(open_windows);
                process_keys_town_select();
                process_mouse_town_select();
                process_sound_heap();
                process_turn();
                break;
            case DM_NETWORK_SELECT:
                draw_network_select();
                process_keys_network_select();
                process_mouse_network_select();
                process_sound_heap();
                break;
            case DM_LOAD_GAME:
                draw_load_game();
                process_keys_load_game();
                process_mouse_load_game();
                process_sound_heap();
                manage_sound();
                break;
            case DM_WINLOSE:
                if (othervar.winlose_result != 0) {
                    switch (othervar.winlose_result) {
                    case 1:
                    case 2:
                        play_winlose();
                        break;
                    default:
                        othervar.winlose_result = 0;
                        break;
                    }
                }
                process_turn();
                break;
            case DM_YEAR_END:
                do_year_end();
                manage_sound();
                process_sound_heap();
                draw_help_text();
                process_turn();
                break;
            case DM_CHOICE:
                display_choice();
                process_sound_heap();
                manage_sound();
                draw_help_text();
                process_turn();
                break;
            case DM_NICKNAME:
                draw_nickname();
                process_keys_nickname();
                process_mouse_nickname();
                process_sound_heap();
                manage_sound();
                process_turn();
                break;
            case DM_MODAL:
                draw_it();
                do_net_text();
                ProcessWindows(open_windows);
                req13_cancel_check();
                display_advisor();
                process_sound_heap();
                do_ambients();
                manage_sound();
                draw_other_windows();
                DrawWindows(open_windows);
                process_turn();
                draw_help_text();
                break;
            case DM_YEAR_END_WINLOSE:
                lib_memcpy(&choice_backup, &year_end_backup, 10);
                play_winlose();
                process_turn();
                break;
            case DM_LOST_FOCUS:
                run_things = process_lost_focus_display(1);
                break;
            case 99:
                process_turn();
                break;
            case DM_TOWN_BID:
                draw_town_select();
                ProcessWindows(open_windows);
                process_keys_town_select();
                process_mouse_town_select();
                process_sound_heap();
                process_turn();
                if (town.state >= 0x45 && statvar.num_players > 1) {
                    run_things = 0;
                }
                break;
            default:
                break;
            }
            LbScreenUnlock();
        }
        if (debug.screenshot_request != 0) {
            if (debug.screenshot_request == 1) {
                debug.screenshot_request = 0;
            }
            LbMousePlace();
            while (LbScreenLock() != 1) {
            }
            screen_shot();
            LbScreenUnlock();
            LbMouseRemove();
        }
        limit_framerate();
        if ((display_mode == DM_GAME || display_mode == DM_MODAL) && resvars.screen_mode == SCREEN_MODE_13 &&
            othervar.full_swap_frames == 0) {
            special_refresh();
        } else {
            if (othervar.full_swap_frames) {
                othervar.full_swap_frames--;
            }
            LbScreenSwap();
        }
        if (run_things && !gamevar.paused && gamevar.turn_skipped == 0 && frame && quit_game != 1) {
            process_things(thing + 1, end_things);
            process_blueprint_zones();
            if (gamevar.turbo || othervar.turbo_key || othervar.extra_turns) {
                for (i = 0; othervar.extra_turns + 2 > i; i++) {
                    process_things(thing + 1, end_things);
                    process_blueprint_zones();
                    start_turn();
                }
                othervar.turbo_key = 0;
                othervar.extra_turns = 0;
            }
        }
        if (othervar.fade_palette) {
            hosp_fade_palette(othervar.fade_palette, 10, Lb_PALETTE_FADE_CLOSED);
            current_palette = othervar.fade_palette;
            othervar.fade_palette = 0;
            setup_colors();
        }
        if (othervar.fade_to_black) {
            hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
            othervar.fade_to_black = 0;
        }
        if (othervar.pending_palette) {
            LbPaletteSet(othervar.pending_palette);
            current_palette = othervar.pending_palette;
            othervar.pending_palette = 0;
            setup_colors();
        }
        if (othervar.show_level_brief || othervar.show_net_criteria || othervar.net_result_page) {
            if (display_mode == DM_GAME && crequester.id == REQ_NONE && othervar.irequester_slot == 0 &&
                gamevar.mouse_mode != MM_HELP && !othervar.net_result && gamevar.mouse_mode != MM_PLACE_STAFF) {
                display_mission();
            }
        }
        switch (quit_game) {
        case 1:
            close_irequesters();
            if (crequester.id != REQ_NONE) {
                close_crequester(crequester.id);
            }
            mainmenu.state = MMS_UNLOAD_GAME;
            display_mode = DM_MAIN_MENU;
            fade_out_menu();
            LbScreenClear(0);
            LbScreenSwap();
            quit_game = 0;
            save_game(0x63, -1);
            stop_midi();
            if (debug.direct_start) {
                quit_game = 1;
            }
            break;
        case 2:
            new_game = 1;
            quit_game = 0;
            othervar.full_redraw = 1;
            break;
        default:
            break;
        }
    }
done:
    AIL_end_sequence(GetSongHandle());
}

// Waits until at least 40 timer ticks (LbTimerClock) have passed since the
// previous frame.
// FUNCTION: 0x00011b24
void limit_framerate(void)
{
    u32 i;
    u32 now;

    while ((now = LbTimerClock()) - gamecounts.last_frame_clock < 0x28) {
        for (i = 0; i < 1000; i++) {
            wasteOfTime = wasteOfTime * 11 % (wasteOfTime - 10);
        }
    }
    gamecounts.last_frame_clock = now;
}

// Starts a game turn if the game speed (myplayer.game_speed, turns per second)
// allows one now (always in a network game); otherwise sets
// gamevar.turn_skipped (frame without a turn) and returns. Unless paused,
// advances actual_game_turn and the per-turn random values, runs the
// twice-daily, daily, month-end, quarterly and year-end administration as they
// fall due, counts down each hospital's start delay (opening_countdown), runs
// earthquakes, and once the room being edited is empty queues its
// place-object requester (in MM_EDIT_ROOM_WAIT, once room->first is 0) or the
// end of the edit. In a network game, queues the pending net result event.
// FUNCTION: 0x00011b90
void start_turn(void)
{
    u8 hosp_count;
    u8 i;
    u32 now;
    RoomInfo *room;

    now = LbTimerClock();
    game_turn++;
    othervar.visible_people = 0;
    if (gamevar.max_speed) {
        myplayer.game_speed_s = 0x7d00;
    }
    if (now - gamecounts.last_turn_clock < 1000 / myplayer.game_speed_s && othervar.turbo_key == 0 &&
        statvar.num_players <= 1) {
        gamevar.turn_skipped = 1;
        return;
    }
    gamecounts.last_turn_clock = now;
    gamevar.turn_skipped = 0;
    if (gamevar.paused) {
        return;
    }
    actual_game_turn++;
    othervar.every_16_turns = (actual_game_turn & 0xf) == 0;
    othervar.rand_pct = old_newrand() % 100;
    othervar.queue_near_dist = old_newrand() % 6;
    othervar.object_near_dist = (old_newrand() & 3) + 1;
    othervar.cur_turn = actual_game_turn;
    othervar.next_modify_turn = actual_game_turn + gbv.modify_freq;
    othervar.every_32_turns = (actual_game_turn & 0x1f) == 0;
    if (othervar.recept_call_timer > 0) {
        othervar.recept_call_timer--;
    }
    if (actual_game_turn % 25 == 0) {
        twice_daily_admin();
        if (actual_game_turn % 50 == 0) {
            daily_admin();
            if ((int)mycontrol->day > (int)calendar[mycontrol->month]) {
                month_end();
                if (mycontrol->month >= 12) {
                    year_end();
                } else if (mycontrol->month % 3 == 0) {
                    check_quarterly_game_over();
                }
            }
        }
    }
    hosp_count = 1;
    if (statvar.num_players > 1) {
        hosp_count = 4;
    }
    for (i = 0; i < hosp_count; i++) {
        if (statvar.num_players > 1 && player_info[i].active == 0) {
            continue;
        }
        allhosps[i].idle_nurses_prev = allhosps[i].idle_nurses;
        allhosps[i].idle_nurses = 0;
        if ((statvar.num_players > 1 ? 1 : allhosps[i].hospital_open) != 0 || (advisor.flags & 1)) {
            continue;
        }
        allhosps[i].opening_countdown--;
        if (!allhosps[i].opening_countdown) {
            if (i != player) {
                announce_this(ANN_RIVAL_OPENED, i);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_FANFARE, -1, -1);
            }
            if (statvar.num_players <= 1) {
                allhosps[i].hospital_open = 1;
            }
        }
    }
    compute_whether_user_not_busy();
    if (gamevar.quake_strength_s) {
        earthquake(gamevar.quake_strength_s, gamevar.quake_step_chance);
    }
    room = &room_info[allhosps[player].edited_room];
    if (room > room_info) {
        if (room->being_edited && is_room_empty(&room_info[allhosps[player].edited_room])) {
            if (gamevar.mouse_mode == MM_EDIT_ROOM_WAIT) {
                if (room_info[allhosps[player].edited_room].first_person == 0) {
                    player_events[player].type = EV_SETUP_PLACE_OBJECT_REQUESTER;
                }
            } else {
                player_events[player].type = EV_CANCEL_EDIT_ROOM;
            }
        }
    }
    if (statvar.num_players > 1) {
        switch (othervar.net_result) {
        case 1:
            player_events[player].type = EV_LEAVE_NET_GAME;
            player_events[player].value = 1;
            break;
        case 2:
            player_events[player].type = EV_NET_WIN;
            break;
        default:
            break;
        }
    }
}

// Switches to screen mode mode, keeping the mouse at the same place on the
// screen and the cursor sprite. Returns 1, or 0 if the mode could not be set.
// FUNCTION: 0x00012068
u8 change_res(TbScreenMode mode)
{
    s32 mouse_x = lbDisplay.mmouse_x;
    s32 mouse_y = lbDisplay.mmouse_y;

    if (resvars.screen_mode == SCREEN_MODE_13 && mode != SCREEN_MODE_13) {
        mouse_x = mouse_x / 2;
        mouse_y = mouse_y * 5 / 12;
    } else if (resvars.screen_mode != SCREEN_MODE_13 && mode == SCREEN_MODE_13) {
        mouse_x <<= 1;
        mouse_y = mouse_y * 12 / 5;
    }
    if (!setup_any_screen(palette, mode)) {
        return 0;
    } else {
        TbSprite *cursor_sprites;

        if (othervar.screen_pic_loaded) {
            cursor_sprites = screen_pointers_tab_start;
        } else {
            cursor_sprites = pointer;
        }
        LbMouseSetup(cursor_sprites + othervar.pointer_sprite);
        LbMouseSetPosition(mouse_x, mouse_y);
        return 1;
    }
}

// Runs one frame of the full-screen window chart_type (bank statement,
// casebook, policy, research, graphs, staff list, manager, patient
// overview, year charts); others ask to close the screen.
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
        draw_graphs((u16)resvars.screen_boxes->graph_x, (u16)resvars.screen_boxes->graph_y,
                    (u16)resvars.screen_boxes->graph_h, (u16)resvars.screen_boxes->graph_step);
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
        player_events[player].type = EV_CLOSE_SCREEN;
        break;
    }
}

// Runs one frame of the year end, by year_end_type: 0 sets it up and pauses
// the game, 1 the hall of fame, 2 the year charts, 3 moves on to 4, 4 checks for winning or
// losing and returns to the game, 5-7 the win or lose sequence.
// FUNCTION: 0x00012274
void do_year_end(void)
{
    switch (year_end_type) {
    case 0:
        if (statvar.num_players > 1) {
            year_end_type = 2;
            yearend.value = network_num_players();
        } else {
            hall.state = 0;
            year_end_type++;
            yearend.value = 1;
            insert_hall_details();
        }
        yearend.state = 0;
        exit_the_yearend_screen = 0;
        gamevar.paused = 1;
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
        lib_memcpy(&choice_backup, &year_end_backup, 10);
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

// While the game is paused, shades the whole screen.
// FUNCTION: 0x00012390
void draw_pause_mode(void)
{
    if (gamevar.paused) {
        lbDisplay.draw_flags = 4;
        LbDrawBox(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height, color.dark_blue);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        lbDisplay.draw_flags = 0;
        redraw_panel = 1;
    }
}

// Network game: tells the others when this player's game window gains or
// loses focus, switching to the lost focus display (DM_LOST_FOCUS) and
// pausing or resuming the music.
// FUNCTION: 0x00012400
void check_if_active(void)
{
    if (statvar.num_players > 1) {
        if (LbIsActive() && othervar.lost_focus[player] != 0) {
            othervar.full_swap_frames = 0xf;
            player_events[player].type = EV_NETWORK_GAINED_FOCUS;
            if (othervar.music_playing) {
                AIL_resume_sequence(GetSongHandle());
            }
        } else if (!LbIsActive() && othervar.lost_focus[player] == 0) {
            player_events[player].type = EV_NETWORK_LOST_FOCUS;
            if (has_any_player_lost_focus()) {
                othervar.lost_focus[player] = 2;
            } else {
                othervar.lost_focus[player] = 1;
            }
            if (display_mode != DM_LOST_FOCUS) {
                othervar.lost_focus_display_mode = display_mode;
                display_mode = DM_LOST_FOCUS;
            }
            if (othervar.music_playing) {
                AIL_end_sequence(GetSongHandle());
            }
        }
    }
}

// Lost focus display: keeps turns running if only this player lost focus,
// and with draw set draws the game view (or a blank screen) and the lost
// focus message. Returns 1 if the turn's things should run.
// FUNCTION: 0x00012500
u8 process_lost_focus_display(u8 draw)
{
    u8 result;

    if (othervar.lost_focus[player] == 1) {
        start_turn();
        result = 1;
    } else {
        result = 0;
    }
    if (draw) {
        switch (othervar.lost_focus_display_mode) {
        case DM_GAME:
        case DM_MODAL:
            draw_it();
            break;
        default:
            LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
            LbDrawBox(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, color.black);
            break;
        }
        draw_lost_focus_screen();
    }
    process_turn();
    return result;
}

// Shows the mission briefing help page (single player) or the network
// criteria or result page, clearing the request; not in the tutorial.
// FUNCTION: 0x000125b8
void display_mission(void)
{
    if (advisor.flags & 1) {
        return;
    }
    lbDisplay.left_button = lbDisplay.right_button = 0;
    lbDisplay.mleft_button = lbDisplay.mright_button = 0;
    if (statvar.num_players <= 1) {
        show_help_page(1, level_number);
        othervar.show_level_brief = 0;
    } else if (othervar.net_result_page) {
        if (othervar.net_result_page == 3) {
            play_sound(SNDSRC_GENERAL, SFX_CHEER, -1, -1);
        }
        show_help_page(3, othervar.net_result_page);
        othervar.net_result_page = 0;
    } else {
        show_help_page(2, current_net_criteria.criterion);
        othervar.show_net_criteria = 0;
    }
}
