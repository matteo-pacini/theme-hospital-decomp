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

void conv_string(void *dst, char *text, int flags);

void qsort(void *base, u32 n, u32 size, int (*cmp)(const void *, const void *));
}

void display_plaque(void);
void process_yearend_buttons(u8 button);
void check_stats_buttons(void);
u8 button_available(const u8 button);
void check_awards_buttons(void);
void display_awards(void);

// FUNCTION: 0x000b4980
void start_year_end(void)
{
    s32 i;

    finish_composite_help();
    if (display_mode == 9) {
        fn_000c21e6(year_end_backup.unknown2, &choice, 10);
        year_end_backup.mode = display_mode;
        year_end_backup.f6 = choice_spare_data;
        year_end_backup.f8 = choice_time_out;
        choice_backup.mode = 8;
    } else {
        fn_000c21e6(&year_end_backup, &choice_backup, 10);
    }
    display_mode = 8;
    year_end_type = 0;
    for (i = 0; i < 4; i++) {
        if (allhosps[i].f11f4)
            place_staff_on_map(allhosps[i].f11f6, allhosps[i].f11f4, 1, i);
    }
    LbMemorySet(awards_won, 0, 0x24);
    LbMemorySet(trophies_won, 0, 0xf);
}

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
                total += hospital->f45[j][k];
        }
        chart[0][i].hosp = i;
        chart[0][i].value = hospital->f35 - hospital->f3d + total;
        chart[1][i].hosp = i;
        chart[1][i].value = hospital->f2d;
        chart[3][i].hosp = i;
        chart[3][i].value = hospital->f1e9;
        chart[4][i].hosp = i;
        chart[4][i].value = hospital->f1eb;
        chart[5][i].hosp = i;
        chart[5][i].value = hospital->f1e9 * 100 / (hospital->f1e9 + hospital->f1eb + 1);
        chart[8][i].hosp = i;
        chart[8][i].value = hospital->fc9c;
        chart[9][i].hosp = i;
        chart[9][i].value = hospital->f1ed;
        chart[10][i].hosp = i;
        chart[10][i].value = hospital->f1bb;
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

    LbScreenClear(color.unknown9);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x10);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    if (yearend.unknown0 == 0)
        yearend.unknown2 = 4;
    count = 4;
    switch (yearend.unknown2) {
    case 1:
        crequester.unknown9 = 0;
        current_display = 0;
        load_ingame_pic(15, 1, 0x16, 0, SCREEN_MODE_13);
        current_award = 0;
        othervar.f9ed = 0x4b0;
        yearend.unknown2 = 3;
        break;
    case 0:
        crequester.unknown9 = 0;
        load_ingame_pic(16, 1, 0x16, 0, SCREEN_MODE_13);
        copy_screen_to_wscreen(bscreen);
        yearend.unknown2 = 2;
        othervar.f9ed = 0x4b0;
        LbTextSetWindow(0xe0, 0x24 - height / 2, 0xc0, 0x16);
        lbDisplay.draw_flags = 0x100;
        sprintf(buf, "%s %4.4d", help_text[20], mycontrol->f648 + 0x7cf);
        conv_string(buf, buf, 0);
        LbTextDraw(0, 0, buf);
        for (i = 0, cat = -1; i < 6; i++) {
            for (cat++; cat < 11 && !chart_in_use[cat]; cat++) {
            }
            if (cat == 11)
                break;
            LbTextSetWindow(resvars.f40[i % 2 + 0x97].x, resvars.f40[i / 2 + 0x97].y, resvars.f40[0x99].x,
                            resvars.f40[0x98].y - resvars.f40[0x97].y);
            x = 0;
            y = 0;
            lbDisplay.draw_flags = 0x100;
            LbTextDraw(x, y, chart_types[cat]);
            y += height;
            for (j = 0; j < count; j++) {
                if (statvar.unknownfd > 1 && D_001609c4[chart[cat][j].hosp].active == 0)
                    continue;
                if (allhosps[chart[cat][j].hosp].f2a != 0) {
                    y += height;
                    lbDisplay.draw_flags = 0x20;
                    LbTextDraw(x, y, (char *)allhosps[chart[cat][j].hosp].unknown0);
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
        yearend.unknown2 = 2;
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
                play_sound(10, SFX_NO4, -1, -1);
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
                play_sound(10, SFX_NO4, -1, -1);
            }
        }
        break;
    case 4:
        year_end_type++;
        break;
    default:
        break;
    }
    if (othervar.f9eb != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
        othervar.f9ed = 0x4b0;
        othervar.f9eb = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
    }
    if (!(s16)othervar.f9ed--)
        process_yearend_buttons(0);
    lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
}

// FUNCTION: 0x000b5460
void display_plaque(void)
{
    char text1[252];
    char text2[20];
    char text3[12];

    switch (current_display) {
    case 1:
        if (awards_won[current_award].value > 0) {
            sprintf(text1, "%s", good_awards_text[awards_won[current_award].f2]);
            sprintf(text2, "%s\n+%d", trophy_bonus_messages[awards_won[current_award].f5], awards_won[current_award].value);
        } else {
            sprintf(text1, "%s", good_awards_text[awards_won[current_award].f2]);
            sprintf(text2, "%s\n%d", trophy_bonus_messages[awards_won[current_award].f5], awards_won[current_award].value);
        }
    case 2:
        reqspr_draw(resvars.f40[0xa0].x, resvars.f40[0xa0].y, &screen_sprites_tab_start[16]);
        reqspr_draw(resvars.f40[0xa1].x, resvars.f40[0xa1].y, &screen_sprites_tab_start[17]);
        reqspr_draw(resvars.f40[0xa2].x, resvars.f40[0xa2].y, &screen_sprites_tab_start[18]);
        if (current_display == 2) {
            if (awards_won[current_award].value > 0) {
                sprintf(text1, bad_awards_text[awards_won[current_award].f2]);
                sprintf(text2, "%s\n+%d", trophy_bonus_messages[awards_won[current_award].f5], awards_won[current_award].value);
            } else {
                sprintf(text1, bad_awards_text[awards_won[current_award].f2]);
                sprintf(text2, "%s\n%d", trophy_bonus_messages[awards_won[current_award].f5], awards_won[current_award].value);
            }
            reqspr_draw(resvars.f40[0xa6].x, resvars.f40[0xa6].y, &screen_sprites_tab_start[15]);
        }
        set_font_from_cache(0x12);
        lbDisplay.draw_flags = 0x100;
        LbTextSetWindow(0xee, 0x73, 0x9a, 0xbb);
        LbTextDraw(0, 0, text1);
        LbTextSetWindow(0xee, 0x12e, 0x9a, 0x26);
        LbTextDraw(0, 0, text2);
        break;
    case 3:
        reqspr_draw(resvars.f40[0xa0].x, resvars.f40[0xa0].y, &screen_sprites_tab_start[19]);
        reqspr_draw(resvars.f40[0xa3].x, resvars.f40[0xa3].y, &screen_sprites_tab_start[20]);
        reqspr_draw(resvars.f40[0xa4].x, resvars.f40[0xa4].y, &screen_sprites_tab_start[21]);
        reqspr_draw(resvars.f40[0xa5].x, resvars.f40[0xa5].y, &screen_sprites_tab_start[22]);
        set_font_from_cache(0x11);
        lbDisplay.draw_flags = 0x100;
        if (trophies_won[current_award].unknown0 == 5) {
            sprintf(text1, trophy_text[trophies_won[current_award].f1], othervar.fa59);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].f4]);
            sprintf(text3, "+%d", trophies_won[current_award].value);
        } else if (trophies_won[current_award].unknown0 == 6 && award_counts.f151 > 0) {
            sprintf(text1, trophy_text[trophies_won[current_award].f1], othervar.fa5a);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].f4]);
            sprintf(text3, "+%d", trophies_won[current_award].value);
        } else {
            sprintf(text1, trophy_text[trophies_won[current_award].f1]);
            sprintf(text2, trophy_bonus_messages[trophies_won[current_award].f4]);
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
    if (trophies_won[1].unknown0)
        reqspr_draw(resvars.f48[0x56].unknownC, resvars.f48[0x56].unknownE, &screen_sprites_tab_start[13]);
}

// FUNCTION: 0x000b5b38
void process_yearend_buttons(u8 button)
{
    switch (button) {
    case 0:
        play_sound(10, SFX_NO4, -1, -1);
        if (!exit_the_yearend_screen) {
            D_001549d0[player].type = EV_PAUSE;
            exit_the_yearend_screen = 1;
        }
        break;
    case 1:
        play_sound(10, SFX_YESX, -1, -1);
        switch (yearend.unknown2) {
        case 3:
            yearend.unknown2 = 0;
            break;
        case 2:
            yearend.unknown2 = 1;
            break;
        default:
            break;
        }
        break;
    case 3:
    case 2:
        play_sound(10, SFX_YESX, -1, -1);
        current_display = 0;
        break;
    case 4:
    case 6:
    case 8:
    case 5:
    case 7:
    case 9:
        play_sound(10, SFX_YESX, -1, -1);
        switch (awards_won[button - 4].unknown0) {
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
        play_sound(10, SFX_YESX, -1, -1);
        current_display = 3;
        current_award = button - 10;
        break;
    default:
        break;
    }
}

// FUNCTION: 0x000b5d10
void check_stats_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(resvars.f48[0x48].unknownC, resvars.f48[0x48].unknownE,
                    &screen_sprites_tab_start[resvars.f48[0x48].sprite]);
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0x48].width != 0 && resvars.f48[i + 0x48].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0x48].x, resvars.f48[i + 0x48].y,
                             (s16)resvars.f48[i + 0x48].width, (s16)resvars.f48[i + 0x48].height)) {
                reqspr_draw(resvars.f48[i + 0x48].unknownC, resvars.f48[i + 0x48].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x48].sprite]);
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0x48].width != 0 && resvars.f48[i + 0x48].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x48].x, resvars.f48[i + 0x48].y,
                             (s16)resvars.f48[i + 0x48].width, (s16)resvars.f48[i + 0x48].height)) {
                resvars.f48[i + 0x48].callback(i);
                reqspr_draw(resvars.f48[i + 0x48].unknownC, resvars.f48[i + 0x48].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x48].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

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
        return awards_won[button - 4].f2 != 0;
    case 10:
    case 11:
    case 12:
        return trophies_won[button - 10].f1 != 0;
    default:
        return 0;
    }
}

// FUNCTION: 0x000b614c
void check_awards_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(resvars.f48[0x4b].unknownC, resvars.f48[0x4b].unknownE,
                    &screen_sprites_tab_start[resvars.f48[0x4b].sprite]);
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0x4b].width != 0 && resvars.f48[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0x4b].x, resvars.f48[i + 0x4b].y,
                                 (s16)resvars.f48[i + 0x4b].width, (s16)resvars.f48[i + 0x4b].height)) {
                    reqspr_draw(resvars.f48[i + 0x4b].unknownC, resvars.f48[i + 0x4b].unknownE,
                                &screen_sprites_tab_start[resvars.f48[i + 0x4b].sprite]);
                    break;
                }
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0x4b].width != 0 && resvars.f48[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x4b].x, resvars.f48[i + 0x4b].y,
                                 (s16)resvars.f48[i + 0x4b].width, (s16)resvars.f48[i + 0x4b].height)) {
                    resvars.f48[i + 0x4b].callback(i);
                    reqspr_draw(resvars.f48[i + 0x4b].unknownC, resvars.f48[i + 0x4b].unknownE,
                                &screen_sprites_tab_start[resvars.f48[i + 0x4b].sprite]);
                    break;
                }
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// FUNCTION: 0x000b647c
void display_awards(void)
{
    u8 i;
    u8 trophy_count;
    u8 award_count;

    for (i = 0, trophy_count = 0, award_count = 0; i < 9 && trophy_count < 3; i++) {
        if (trophy[i].unknown0) {
            reqspr_draw(resvars.f48[trophy_count + 0x55].unknownC, resvars.f48[trophy_count + 0x55].unknownE,
                        &screen_sprites_tab_start[trophy_count + 12]);
            trophies_won[trophy_count].unknown0 = trophy[i].unknown0;
            trophies_won[trophy_count].f1 = trophy[i].f1;
            trophies_won[trophy_count].value = trophy[i].value;
            trophies_won[trophy_count++].f4 = trophy[i].f4;
        }
    }
    for (i = 0; i < 13 && award_count < 6; i++) {
        if (all_awards[i].f2) {
            if (trophy_count == 3 && award_count == 5)
                reqspr_draw(resvars.f48[award_count + 0x4f].unknownC, resvars.f48[award_count + 0x4f].unknownE, &screen_sprites_tab_start[24]);
            else
                reqspr_draw(resvars.f48[award_count + 0x4f].unknownC, resvars.f48[award_count + 0x4f].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + (award_count + 4) * 6));
            reqspr_draw(resvars.f48[award_count + 0x4f].unknownC, resvars.f48[award_count + 0x4f].unknownE, &screen_sprites_tab_start[23]);
            if ((s8)all_awards[i].unknown0 == 1)
                reqspr_draw(resvars.f40[award_count + 0x9a].x, resvars.f40[award_count + 0x9a].y, &screen_sprites_tab_start[10]);
            else
                reqspr_draw(resvars.f40[award_count + 0x9a].x, resvars.f40[award_count + 0x9a].y, &screen_sprites_tab_start[11]);
            awards_won[award_count].f1 = all_awards[i].f1;
            awards_won[award_count].f2 = all_awards[i].f2;
            awards_won[award_count].unknown0 = all_awards[i].unknown0;
            awards_won[award_count].value = all_awards[i].value;
            awards_won[award_count].f5 = all_awards[i].f5;
            award_count++;
        }
    }
}
