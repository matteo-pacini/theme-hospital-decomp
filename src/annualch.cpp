#include "hosp.h"

struct ChartList {
    signed int value : 27;
    u8 hosp : 5;
};

extern ChartList chart[][4];
extern Awards awards_won[];
extern Trophies trophies_won[];
extern u8 current_display;
extern u8 current_award;

extern "C" {
extern u16 choice;
extern char **chart_types;
extern char **trophy_text;
extern char **bad_awards_text;
extern char **good_awards_text;
extern char **trophy_bonus_messages;
extern Event D_001549d0[];

void conv_string(void *dst, char *src, int mode);

void qsort(void *base, u32 n, u32 size, int (comparp)(const void *, const void *));
}

void display_plaque(void);
void process_yearend_buttons(u8 button);
void check_stats_buttons(void);
u8 button_available(const u8 button);
void check_awards_buttons(void);
void display_awards(void);

// Switches the display to the year-end screens (display_mode 8), saving the
// current screen state to restore afterwards (from the choice screen's backup
// if one is showing). Places on the map each hospital's staff member held in
// allhosps.held_staff (at tile f11f6) and clears the awards and trophies to show.
// FUNCTION: 0x000b4980
void start_year_end(void)
{
    s32 i;

    finish_composite_help();
    if (display_mode == DM_CHOICE) {
        fn_000c21e6(year_end_backup.choice_state, &choice, 10);
        year_end_backup.mode = display_mode;
        year_end_backup.spare_data = choice_spare_data;
        year_end_backup.time_out = choice_time_out;
        choice_backup.mode = DM_YEAR_END;
    } else {
        fn_000c21e6(&year_end_backup, &choice_backup, 10);
    }
    display_mode = DM_YEAR_END;
    year_end_type = 0;
    for (i = 0; i < 4; i++) {
        if (allhosps[i].held_staff)
            place_staff_on_map(allhosps[i].held_staff_tile, allhosps[i].held_staff, 1, i);
    }
    LbMemorySet(awards_won, 0, 0x24);
    LbMemorySet(trophies_won, 0, 0xf);
}

// Fills the year-end league tables (chart) with each of the four hospitals'
// figures and sorts each table best first (table 4 ascending). The first
// table's running total is not reset between hospitals.
// FUNCTION: 0x000b4aac
void create_chart_data(void)
{
    u16 hosp_count;
    u16 i;
    s32 k;
    AllHosps *hospital;
    s32 j;
    s32 total;

    total = 0;
    i = 0;
    hosp_count = 4;
    for (i = 0; i < hosp_count; i++) {
        hospital = &allhosps[i];
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 12; k++)
                total += hospital->insurance_claims[j][k];
        }
        chart[0][i].hosp = i;
        chart[0][i].value = hospital->balance - hospital->loan + total;
        chart[1][i].hosp = i;
        chart[1][i].value = hospital->score;
        chart[3][i].hosp = i;
        chart[3][i].value = hospital->cures;
        chart[4][i].hosp = i;
        chart[4][i].value = hospital->deaths;
        chart[5][i].hosp = i;
        chart[5][i].value = hospital->cures * 100 / (hospital->cures + hospital->deaths + 1);
        chart[8][i].hosp = i;
        chart[8][i].value = hospital->staff_count;
        chart[9][i].hosp = i;
        chart[9][i].value = hospital->visitors;
        chart[10][i].hosp = i;
        chart[10][i].value = hospital->hospital_value;
    }
    qsort(chart[0], hosp_count, 5, descend);
    qsort(chart[1], hosp_count, 5, descend);
    qsort(chart[3], hosp_count, 5, descend);
    qsort(chart[4], hosp_count, 5, ascend);
    qsort(chart[5], hosp_count, 5, descend);
    qsort(chart[8], hosp_count, 5, descend);
    qsort(chart[9], hosp_count, 5, descend);
    qsort(chart[10], hosp_count, 5, descend);
}

// Runs one frame of the year-end screens, by yearend.state: 0 draws the
// league tables, 2 shows them, 1 loads the awards screen, 3 shows awards and
// trophies, 4 moves year_end_type on. Escape, Space, or 0x4b0 frames without
// mouse movement ask to leave (an EV_PAUSE event). Clears the mouse buttons.
// FUNCTION: 0x000b4e88
extern "C" void display_year_charts(void)
{
    u8 i;
    s8 cat;
    u8 j;
    u16 count;
    u16 x;
    u16 y;
    u16 height;
    char buf[100];

    LbScreenClear(color.dark_blue);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x10);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    if (yearend.value == 0)
        yearend.state = 4;
    count = 4;
    switch (yearend.state) {
    case 1:
        crequester.arg3 = 0;
        current_display = 0;
        load_ingame_pic(SCR_AWARDS1, 1, 0x16, 0, SCREEN_MODE_13);
        current_award = 0;
        othervar.idle_countdown = 0x4b0;
        yearend.state = 3;
        break;
    case 0:
        crequester.arg3 = 0;
        load_ingame_pic(SCR_AWARDS2, 1, 0x16, 0, SCREEN_MODE_13);
        copy_screen_to_wscreen(bscreen);
        yearend.state = 2;
        othervar.idle_countdown = 0x4b0;
        LbTextSetWindow(0xe0, 0x24 - height / 2, 0xc0, 0x16);
        lbDisplay.draw_flags = 0x100;
        sprintf(buf, "%s %4.4d", help_text[20], mycontrol->year + 0x7cf);
        conv_string(buf, buf, 0);
        LbTextDraw(0, 0, buf);
        for (i = 0, cat = -1; i < 6; i++) {
            for (cat++; cat < 11 && !chart_in_use[cat]; cat++) {
            }
            if (cat == 11)
                break;
            LbTextSetWindow(resvars.screen_coords[i % 2 + 0x97].x, resvars.screen_coords[i / 2 + 0x97].y, resvars.screen_coords[0x99].x,
                            resvars.screen_coords[0x98].y - resvars.screen_coords[0x97].y);
            x = 0;
            y = 0;
            lbDisplay.draw_flags = 0x100;
            LbTextDraw(x, y, chart_types[cat]);
            y += height;
            for (j = 0; j < count; j++) {
                if (statvar.num_players > 1 && D_001609c4[chart[cat][j].hosp].active == 0)
                    continue;
                if (allhosps[chart[cat][j].hosp].player_type != 0) {
                    y += height;
                    lbDisplay.draw_flags = 0x20;
                    LbTextDraw(x, y, (char *)allhosps[chart[cat][j].hosp].name);
                    lbDisplay.draw_flags = 0x80;
                    sprintf(buf, "%d", chart[cat][j].value);
                    LbTextDraw(x, y, buf);
                }
            }
        }
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        x = 0;
        y = height;
        lbDisplay.draw_flags = 0x20;
        lbDisplay.draw_flags = 0;
        copy_screen_from_wscreen(bscreen);
        yearend.state = 2;
        break;
    case 2:
        copy_screen_to_wscreen(bscreen);
        check_stats_buttons();
        if (lbInkey >= 0x80)
            lbInkey = 0;
        if (lbInkey == KC_ESCAPE || lbInkey == KC_SPACE) {
            lbInkey = 0;
            if (!exit_the_yearend_screen) {
                D_001549d0[player].type = EV_PAUSE;
                exit_the_yearend_screen = 1;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
        }
        break;
    case 3:
        copy_screen_to_wscreen(bscreen);
        display_awards();
        check_awards_buttons();
        display_plaque();
        if (lbInkey == KC_ESCAPE || lbInkey == KC_SPACE) {
            lbInkey = 0;
            if (!exit_the_yearend_screen) {
                D_001549d0[player].type = EV_PAUSE;
                exit_the_yearend_screen = 1;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
        }
        break;
    case 4:
        year_end_type++;
        break;
    default:
        break;
    }
    if (othervar.last_mouse_sum != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
        othervar.idle_countdown = 0x4b0;
        othervar.last_mouse_sum = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
    }
    if (!(s16)othervar.idle_countdown--)
        process_yearend_buttons(0);
    lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
}

// Draws the plaque for the award or trophy selected (current_display 1 good
// award, 2 bad award, 3 trophy; current_award picks it), with its text and
// bonus.
// FUNCTION: 0x000b5460
void display_plaque(void)
{
    char text1[252];
    char text2[20];
    char text3[12];

    switch (current_display) {
    case 1:
        if (awards_won[current_award].value > 0) {
            sprintf(text1, "%s", good_awards_text[awards_won[current_award].text_index]);
            sprintf(text2, "%s\n+%d", trophy_bonus_messages[awards_won[current_award].reward_kind], awards_won[current_award].value);
        } else {
            sprintf(text1, "%s", good_awards_text[awards_won[current_award].text_index]);
            sprintf(text2, "%s\n%d", trophy_bonus_messages[awards_won[current_award].reward_kind], awards_won[current_award].value);
        }
    case 2:
        reqspr_draw(resvars.screen_coords[0xa0].x, resvars.screen_coords[0xa0].y, &screen_sprites_tab_start[16]);
        reqspr_draw(resvars.screen_coords[0xa1].x, resvars.screen_coords[0xa1].y, &screen_sprites_tab_start[17]);
        reqspr_draw(resvars.screen_coords[0xa2].x, resvars.screen_coords[0xa2].y, &screen_sprites_tab_start[18]);
        if (current_display == 2) {
            if (awards_won[current_award].value > 0) {
                sprintf(text1, bad_awards_text[awards_won[current_award].text_index]);
                sprintf(text2, "%s\n+%d", trophy_bonus_messages[awards_won[current_award].reward_kind], awards_won[current_award].value);
            } else {
                sprintf(text1, bad_awards_text[awards_won[current_award].text_index]);
                sprintf(text2, "%s\n%d", trophy_bonus_messages[awards_won[current_award].reward_kind], awards_won[current_award].value);
            }
            reqspr_draw(resvars.screen_coords[0xa6].x, resvars.screen_coords[0xa6].y, &screen_sprites_tab_start[15]);
        }
        set_font_from_cache(0x12);
        lbDisplay.draw_flags = 0x100;
        LbTextSetWindow(0xee, 0x73, 0x9a, 0xbb);
        LbTextDraw(0, 0, text1);
        LbTextSetWindow(0xee, 0x12e, 0x9a, 0x26);
        LbTextDraw(0, 0, text2);
        break;
    case 3:
        reqspr_draw(resvars.screen_coords[0xa0].x, resvars.screen_coords[0xa0].y, &screen_sprites_tab_start[19]);
        reqspr_draw(resvars.screen_coords[0xa3].x, resvars.screen_coords[0xa3].y, &screen_sprites_tab_start[20]);
        reqspr_draw(resvars.screen_coords[0xa4].x, resvars.screen_coords[0xa4].y, &screen_sprites_tab_start[21]);
        reqspr_draw(resvars.screen_coords[0xa5].x, resvars.screen_coords[0xa5].y, &screen_sprites_tab_start[22]);
        set_font_from_cache(0x11);
        lbDisplay.draw_flags = 0x100;
        if (trophies_won[current_award].id == 5) {
            sprintf(text1, trophy_text[trophies_won[current_award].text_index], othervar.plaque_rat_kills);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].reward_kind]);
            sprintf(text3, "+%d", trophies_won[current_award].value);
        } else if (trophies_won[current_award].id == 6 && award_counts.rat_shots > 0) {
            sprintf(text1, trophy_text[trophies_won[current_award].text_index], othervar.plaque_rat_kill_pct);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].reward_kind]);
            sprintf(text3, "+%d", trophies_won[current_award].value);
        } else {
            sprintf(text1, trophy_text[trophies_won[current_award].text_index]);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].reward_kind]);
            sprintf(text3, "+%d", trophies_won[current_award].value);
        }
        LbTextSetWindow(0xe1, 0x6a, 0xb4, 0xe0);
        LbTextDraw(0, 0, text1);
        LbTextSetWindow(0xd7, 0x150, 0xce, 0x14);
        LbTextDraw(0, 0, text2);
        LbTextSetWindow(0xd7, 0x172, 0xce, 0x14);
        LbTextDraw(0, 0, text3);
        break;
    default:
        break;
    }
    if (trophies_won[1].id)
        reqspr_draw(resvars.screen_buttons[0x56].sprite_x, resvars.screen_buttons[0x56].sprite_y, &screen_sprites_tab_start[13]);
}

// Handles year-end screen button button: 0 leaves the screen, 1 switches
// between the tables and awards pages, 2-3 close the plaque, 4-9 show
// awards_won[button - 4], 10-12 show trophies_won[button - 10].
// FUNCTION: 0x000b5b38
void process_yearend_buttons(u8 button)
{
    switch (button) {
    case 0:
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        if (!exit_the_yearend_screen) {
            D_001549d0[player].type = EV_PAUSE;
            exit_the_yearend_screen = 1;
        }
        break;
    case 1:
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        switch (yearend.state) {
        case 3:
            yearend.state = 0;
            break;
        case 2:
            yearend.state = 1;
            break;
        default:
            break;
        }
        break;
    case 3:
    case 2:
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        current_display = 0;
        break;
    case 4:
    case 6:
    case 8:
    case 5:
    case 7:
    case 9:
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        switch (awards_won[button - 4].direction) {
        case 0:
            current_display = 0;
            break;
        case 1:
            current_display = 1;
            break;
        case 0xff:
            current_display = 2;
            break;
        default:
            break;
        }
        current_award = button - 4;
        break;
    case 12:
    case 11:
    case 10:
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        current_display = 3;
        current_award = button - 10;
        break;
    default:
        break;
    }
}

// Draws pressed and runs the callbacks of the league table page's buttons
// (resvars.screen_buttons from index 0x48) under the mouse.
// FUNCTION: 0x000b5d10
void check_stats_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(resvars.screen_buttons[0x48].sprite_x, resvars.screen_buttons[0x48].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x48].sprite]);
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x48].width != 0 && resvars.screen_buttons[i + 0x48].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x48].x, resvars.screen_buttons[i + 0x48].y,
                             (s16)resvars.screen_buttons[i + 0x48].width, (s16)resvars.screen_buttons[i + 0x48].height)) {
                reqspr_draw(resvars.screen_buttons[i + 0x48].sprite_x, resvars.screen_buttons[i + 0x48].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x48].sprite]);
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x48].width != 0 && resvars.screen_buttons[i + 0x48].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x48].x, resvars.screen_buttons[i + 0x48].y,
                             (s16)resvars.screen_buttons[i + 0x48].width, (s16)resvars.screen_buttons[i + 0x48].height)) {
                resvars.screen_buttons[i + 0x48].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 0x48].sprite_x, resvars.screen_buttons[i + 0x48].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x48].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// Returns nonzero if awards page button button (see process_yearend_buttons)
// applies: the plaque buttons while a plaque is shown, the award and trophy
// buttons when there is one in that slot.
// FUNCTION: 0x000b601c
u8 button_available(const u8 button)
{
    switch (button) {
    case 0:
    case 1:
        return 1;
    case 2:
        return current_display == 1 || current_display == 2;
    case 3:
        return current_display == 3;
    case 4:
    case 6:
    case 8:
    case 5:
    case 7:
    case 9:
        return awards_won[button - 4].text_index != 0;
    case 10:
    case 11:
    case 12:
        return trophies_won[button - 10].text_index != 0;
    default:
        return 0;
    }
}

// Draws pressed and runs the callbacks of the awards page's available buttons
// (resvars.screen_buttons from index 0x4b) under the mouse.
// FUNCTION: 0x000b614c
void check_awards_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(resvars.screen_buttons[0x4b].sprite_x, resvars.screen_buttons[0x4b].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x4b].sprite]);
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x4b].width != 0 && resvars.screen_buttons[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x4b].x, resvars.screen_buttons[i + 0x4b].y,
                                 (s16)resvars.screen_buttons[i + 0x4b].width, (s16)resvars.screen_buttons[i + 0x4b].height)) {
                    reqspr_draw(resvars.screen_buttons[i + 0x4b].sprite_x, resvars.screen_buttons[i + 0x4b].sprite_y,
                                &screen_sprites_tab_start[resvars.screen_buttons[i + 0x4b].sprite]);
                    break;
                }
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x4b].width != 0 && resvars.screen_buttons[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x4b].x, resvars.screen_buttons[i + 0x4b].y,
                                 (s16)resvars.screen_buttons[i + 0x4b].width, (s16)resvars.screen_buttons[i + 0x4b].height)) {
                    resvars.screen_buttons[i + 0x4b].callback(i);
                    reqspr_draw(resvars.screen_buttons[i + 0x4b].sprite_x, resvars.screen_buttons[i + 0x4b].sprite_y,
                                &screen_sprites_tab_start[resvars.screen_buttons[i + 0x4b].sprite]);
                    break;
                }
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// Draws the trophies (up to 3) and awards (up to 6) won this year and copies
// them into trophies_won and awards_won, which the plaques and buttons use.
// FUNCTION: 0x000b647c
void display_awards(void)
{
    u8 i;
    u8 trophy_count;
    u8 award_count;

    for (i = 0, trophy_count = 0, award_count = 0; i < 9 && trophy_count < 3; i++) {
        if (trophy[i].id) {
            reqspr_draw(resvars.screen_buttons[trophy_count + 0x55].sprite_x, resvars.screen_buttons[trophy_count + 0x55].sprite_y,
                        &screen_sprites_tab_start[trophy_count + 12]);
            trophies_won[trophy_count].id = trophy[i].id;
            trophies_won[trophy_count].text_index = trophy[i].text_index;
            trophies_won[trophy_count].value = trophy[i].value;
            trophies_won[trophy_count++].reward_kind = trophy[i].reward_kind;
        }
    }
    for (i = 0; i < 13 && award_count < 6; i++) {
        if (all_awards[i].text_index) {
            if (trophy_count == 3 && award_count == 5)
                reqspr_draw(resvars.screen_buttons[award_count + 0x4f].sprite_x, resvars.screen_buttons[award_count + 0x4f].sprite_y, &screen_sprites_tab_start[24]);
            else
                reqspr_draw(resvars.screen_buttons[award_count + 0x4f].sprite_x, resvars.screen_buttons[award_count + 0x4f].sprite_y,
                            (TbSprite *)((u32)screen_sprites_tab_start + (award_count + 4) * 6));
            reqspr_draw(resvars.screen_buttons[award_count + 0x4f].sprite_x, resvars.screen_buttons[award_count + 0x4f].sprite_y, &screen_sprites_tab_start[23]);
            if ((s8)all_awards[i].direction == 1)
                reqspr_draw(resvars.screen_coords[award_count + 0x9a].x, resvars.screen_coords[award_count + 0x9a].y, &screen_sprites_tab_start[10]);
            else
                reqspr_draw(resvars.screen_coords[award_count + 0x9a].x, resvars.screen_coords[award_count + 0x9a].y, &screen_sprites_tab_start[11]);
            awards_won[award_count].id = all_awards[i].id;
            awards_won[award_count].text_index = all_awards[i].text_index;
            awards_won[award_count].direction = all_awards[i].direction;
            awards_won[award_count].value = all_awards[i].value;
            awards_won[award_count].reward_kind = all_awards[i].reward_kind;
            award_count++;
        }
    }
}
