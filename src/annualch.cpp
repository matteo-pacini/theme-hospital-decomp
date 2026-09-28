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
extern u16 D_00158190;
extern s32 D_00188c38;
extern u16 D_00188c74;
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c81, D_00188c83;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern u16 D_00155da5;
extern s16 D_00155da3;
extern u8 D_0012ce18[];
extern Point *D_0015f434;
extern Button *D_0015f43c;
extern char **D_001602fc;
extern char **D_00160364;
extern char **D_00160368;
extern char **D_0016036c;
extern char **D_00160390;
extern Event D_001549d0[];
extern u8 D_00155e11;
extern u8 D_00155e12;
extern s32 D_0015f8ed;
extern char D_00121b04[];
extern char D_00121b0d[];
extern char D_00121b10[];
extern char D_00121b13[];
extern char D_00121b1a[];
extern char D_00121b1d[];
extern char D_00121b23[];
extern char D_00121b2a[];
extern char D_00121b30[];
extern char D_00121b34[];
extern char D_00121b38[];

void fn_0001b7e0(void *dst, char *text, int flags);

void fs_qsort(void *base, u32 n, u32 size, int (*cmp)(const void *, const void *));
#pragma aux fs_qsort "W?fs_point_object$n(pn$rw_dtreg$$ui)pnv"
}

void display_plaque(void);
void process_yearend_buttons(u8 b);
void check_stats_buttons(void);
u8 button_available(const u8 b);
void check_awards_buttons(void);
void display_awards(void);

// FUNCTION: 0x000b4980
void start_year_end(void)
{
    s32 i;

    finish_composite_help();
    if (display_mode == 9) {
        LbDrawBoxCoords(year_end_backup.unknown2, &D_00158190, 10);
        year_end_backup.mode = display_mode;
        year_end_backup.f6 = choice_spare_data;
        year_end_backup.f8 = choice_time_out;
        choice_backup.mode = 8;
    } else {
        LbDrawBoxCoords(&year_end_backup, &choice_backup, 10);
    }
    display_mode = 8;
    year_end_type = 0;
    for (i = 0; i < 4; i++) {
        if (allhosps[i].f11f4)
            fn_00029c4c(allhosps[i].f11f6, allhosps[i].f11f4, 1, i);
    }
    LbMemorySet(awards_won, 0, 0x24);
    LbMemorySet(trophies_won, 0, 0xf);
}

// FUNCTION: 0x000b4aac
void create_chart_data(void)
{
    u16 n;
    u16 i;
    s32 k;
    AllHosps *h;
    s32 j;
    s32 total;

    total = 0;
    i = 0;
    n = 4;
    for (i = 0; i < n; i++) {
        h = &allhosps[i];
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 12; k++)
                total += h->f45[j][k];
        }
        chart[0][i].hosp = i;
        chart[0][i].value = h->f35 - h->f3d + total;
        chart[1][i].hosp = i;
        chart[1][i].value = h->f2d;
        chart[3][i].hosp = i;
        chart[3][i].value = h->f1e9;
        chart[4][i].hosp = i;
        chart[4][i].value = h->f1eb;
        chart[5][i].hosp = i;
        chart[5][i].value = h->f1e9 * 100 / (h->f1e9 + h->f1eb + 1);
        chart[8][i].hosp = i;
        chart[8][i].value = h->fc9c;
        chart[9][i].hosp = i;
        chart[9][i].value = h->f1ed;
        chart[10][i].hosp = i;
        chart[10][i].value = h->f1bb;
    }
    fs_qsort(chart[0], n, 5, descend);
    fs_qsort(chart[1], n, 5, descend);
    fs_qsort(chart[3], n, 5, descend);
    fs_qsort(chart[4], n, 5, ascend);
    fs_qsort(chart[5], n, 5, descend);
    fs_qsort(chart[8], n, 5, descend);
    fs_qsort(chart[9], n, 5, descend);
    fs_qsort(chart[10], n, 5, descend);
}

// NONMATCHING: register choice
// FUNCTION: 0x000b4e88
extern "C" void fn_000b4e88(void)
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
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
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
        D_00155da5 = 0x4b0;
        yearend.unknown2 = 3;
        break;
    case 0:
        crequester.unknown9 = 0;
        load_ingame_pic(16, 1, 0x16, 0, SCREEN_MODE_13);
        copy_screen_to_wscreen(bscreen);
        yearend.unknown2 = 2;
        D_00155da5 = 0x4b0;
        LbTextSetWindow(0xe0, 0x24 - height / 2, 0xc0, 0x16);
        D_00188c74 = 0x100;
        sprintf(buf, D_00121b04, help_text[20], mycontrol->f648 + 0x7cf);
        fn_0001b7e0(buf, buf, 0);
        LbTextDraw(0, 0, buf);
        for (i = 0, cat = -1; i < 6; i++) {
            for (cat++; cat < 11 && !D_0012ce18[cat]; cat++) {
            }
            if (cat == 11)
                break;
            LbTextSetWindow(D_0015f434[i % 2 + 0x97].x, D_0015f434[i / 2 + 0x97].y, D_0015f434[0x99].x,
                            D_0015f434[0x98].y - D_0015f434[0x97].y);
            x = 0;
            y = 0;
            D_00188c74 = 0x100;
            LbTextDraw(x, y, D_001602fc[cat]);
            y += height;
            for (j = 0; j < count; j++) {
                if (statvar.unknownfd > 1 && D_001609c4[chart[cat][j].hosp].active == 0)
                    continue;
                if (allhosps[chart[cat][j].hosp].f2a != 0) {
                    y += height;
                    D_00188c74 = 0x20;
                    LbTextDraw(x, y, (char *)allhosps[chart[cat][j].hosp].unknown0);
                    D_00188c74 = 0x80;
                    sprintf(buf, D_00121b0d, chart[cat][j].value);
                    LbTextDraw(x, y, buf);
                }
            }
        }
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        x = 0;
        y = height;
        D_00188c74 = 0x20;
        D_00188c74 = 0;
        copy_screen_from_wscreen(bscreen);
        yearend.unknown2 = 2;
        break;
    case 2:
        copy_screen_to_wscreen(bscreen);
        check_stats_buttons();
        if (lbInkey >= 0x80)
            lbInkey = 0;
        if (lbInkey == 1 || lbInkey == 0x39) {
            lbInkey = 0;
            if (!exit_the_yearend_screen) {
                D_001549d0[player].type = 0x22;
                exit_the_yearend_screen = 1;
                play_sound(10, 0x42, -1, -1);
            }
        }
        break;
    case 3:
        copy_screen_to_wscreen(bscreen);
        display_awards();
        check_awards_buttons();
        display_plaque();
        if (lbInkey == 1 || lbInkey == 0x39) {
            lbInkey = 0;
            if (!exit_the_yearend_screen) {
                D_001549d0[player].type = 0x22;
                exit_the_yearend_screen = 1;
                play_sound(10, 0x42, -1, -1);
            }
        }
        break;
    case 4:
        year_end_type++;
        break;
    default:
        break;
    }
    if (D_00155da3 != mouse_dx + mouse_dy) {
        D_00155da5 = 0x4b0;
        D_00155da3 = mouse_dx + mouse_dy;
    }
    if (!D_00155da5--)
        process_yearend_buttons(0);
    D_00188c7b = D_00188c7c = D_00188c81 = D_00188c83 = 0;
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
            sprintf(text1, D_00121b10, D_0016036c[awards_won[current_award].f2]);
            sprintf(text2, D_00121b13, D_00160390[awards_won[current_award].f5], awards_won[current_award].value);
        } else {
            sprintf(text1, D_00121b1a, D_0016036c[awards_won[current_award].f2]);
            sprintf(text2, D_00121b1d, D_00160390[awards_won[current_award].f5], awards_won[current_award].value);
        }
    case 2:
        reqspr_draw(D_0015f434[0xa0].x, D_0015f434[0xa0].y, &screen_sprites_tab_start[16]);
        reqspr_draw(D_0015f434[0xa1].x, D_0015f434[0xa1].y, &screen_sprites_tab_start[17]);
        reqspr_draw(D_0015f434[0xa2].x, D_0015f434[0xa2].y, &screen_sprites_tab_start[18]);
        if (current_display == 2) {
            if (awards_won[current_award].value > 0) {
                sprintf(text1, D_00160368[awards_won[current_award].f2]);
                sprintf(text2, D_00121b23, D_00160390[awards_won[current_award].f5], awards_won[current_award].value);
            } else {
                sprintf(text1, D_00160368[awards_won[current_award].f2]);
                sprintf(text2, D_00121b2a, D_00160390[awards_won[current_award].f5], awards_won[current_award].value);
            }
            reqspr_draw(D_0015f434[0xa6].x, D_0015f434[0xa6].y, &screen_sprites_tab_start[15]);
        }
        set_font_from_cache(0x12);
        D_00188c74 = 0x100;
        LbTextSetWindow(0xee, 0x73, 0x9a, 0xbb);
        LbTextDraw(0, 0, text1);
        LbTextSetWindow(0xee, 0x12e, 0x9a, 0x26);
        LbTextDraw(0, 0, text2);
        break;
    case 3:
        reqspr_draw(D_0015f434[0xa0].x, D_0015f434[0xa0].y, &screen_sprites_tab_start[19]);
        reqspr_draw(D_0015f434[0xa3].x, D_0015f434[0xa3].y, &screen_sprites_tab_start[20]);
        reqspr_draw(D_0015f434[0xa4].x, D_0015f434[0xa4].y, &screen_sprites_tab_start[21]);
        reqspr_draw(D_0015f434[0xa5].x, D_0015f434[0xa5].y, &screen_sprites_tab_start[22]);
        set_font_from_cache(0x11);
        D_00188c74 = 0x100;
        if (trophies_won[current_award].unknown0 == 5) {
            sprintf(text1, D_00160364[trophies_won[current_award].f1], D_00155e11);
            sprintf(text2, D_00160390[trophies_won[current_award].f4]);
            sprintf(text3, D_00121b30, trophies_won[current_award].value);
        } else if (trophies_won[current_award].unknown0 == 6 && D_0015f8ed > 0) {
            sprintf(text1, D_00160364[trophies_won[current_award].f1], D_00155e12);
            sprintf(text2, D_00160390[trophies_won[current_award].f4]);
            sprintf(text3, D_00121b34, trophies_won[current_award].value);
        } else {
            sprintf(text1, D_00160364[trophies_won[current_award].f1]);
            sprintf(text2, D_00160390[trophies_won[current_award].f4]);
            sprintf(text3, D_00121b38, trophies_won[current_award].value);
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
        reqspr_draw(D_0015f43c[0x57].unknownC, D_0015f43c[0x57].unknownE, &screen_sprites_tab_start[13]);
}

// FUNCTION: 0x000b5b38
void process_yearend_buttons(u8 b)
{
    switch (b) {
    case 0:
        play_sound(10, 0x42, -1, -1);
        if (!exit_the_yearend_screen) {
            D_001549d0[player].type = 0x22;
            exit_the_yearend_screen = 1;
        }
        break;
    case 1:
        play_sound(10, 0x41, -1, -1);
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
        play_sound(10, 0x41, -1, -1);
        current_display = 0;
        break;
    case 4:
    case 6:
    case 8:
    case 5:
    case 7:
    case 9:
        play_sound(10, 0x41, -1, -1);
        switch (awards_won[b - 4].unknown0) {
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
        current_award = b - 4;
        break;
    case 12:
    case 11:
    case 10:
        play_sound(10, 0x41, -1, -1);
        current_display = 3;
        current_award = b - 10;
        break;
    default:
        break;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000b5d10
void check_stats_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(D_0015f43c[0x48].unknownC, D_0015f43c[0x48].unknownE,
                    (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[0x48].sprite * 6));
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x48].width != 0 && D_0015f43c[i + 0x48].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x48].x, D_0015f43c[i + 0x48].y,
                             (s16)D_0015f43c[i + 0x48].width, (s16)D_0015f43c[i + 0x48].height)) {
                reqspr_draw(D_0015f43c[i + 0x48].unknownC, D_0015f43c[i + 0x48].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x48].sprite * 6));
                break;
            }
        }
    }
    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0x48].width != 0 && D_0015f43c[i + 0x48].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x48].x, D_0015f43c[i + 0x48].y,
                             (s16)D_0015f43c[i + 0x48].width, (s16)D_0015f43c[i + 0x48].height)) {
                D_0015f43c[i + 0x48].callback(i);
                reqspr_draw(D_0015f43c[i + 0x48].unknownC, D_0015f43c[i + 0x48].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x48].sprite * 6));
                break;
            }
        }
        D_00188c81 = 0;
    }
}

// FUNCTION: 0x000b601c
u8 button_available(const u8 b)
{
    switch (b) {
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
        return awards_won[b - 4].f2 != 0;
    case 10:
    case 11:
    case 12:
        return trophies_won[b - 10].f1 != 0;
    default:
        return 0;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000b614c
void check_awards_buttons(void)
{
    u8 i;

    if (exit_the_yearend_screen)
        reqspr_draw(D_0015f43c[0x4b].unknownC, D_0015f43c[0x4b].unknownE,
                    (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[0x4b].sprite * 6));
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x4b].width != 0 && D_0015f43c[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0x4b].x, D_0015f43c[i + 0x4b].y,
                                 (s16)D_0015f43c[i + 0x4b].width, (s16)D_0015f43c[i + 0x4b].height)) {
                    reqspr_draw(D_0015f43c[i + 0x4b].unknownC, D_0015f43c[i + 0x4b].unknownE,
                                (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x4b].sprite * 6));
                    break;
                }
            }
        }
    }
    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0x4b].width != 0 && D_0015f43c[i + 0x4b].height != 0; i++) {
            if (button_available(i)) {
                if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x4b].x, D_0015f43c[i + 0x4b].y,
                                 (s16)D_0015f43c[i + 0x4b].width, (s16)D_0015f43c[i + 0x4b].height)) {
                    D_0015f43c[i + 0x4b].callback(i);
                    reqspr_draw(D_0015f43c[i + 0x4b].unknownC, D_0015f43c[i + 0x4b].unknownE,
                                (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0x4b].sprite * 6));
                    break;
                }
            }
        }
        D_00188c81 = 0;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000b647c
void display_awards(void)
{
    u8 i;
    u8 t;
    u8 a;

    for (i = 0, t = 0, a = 0; i < 9 && t < 3; i++) {
        if (trophy[i].unknown0) {
            reqspr_draw(D_0015f43c[t + 0x55].unknownC, D_0015f43c[t + 0x55].unknownE,
                        (TbSprite *)((u32)screen_sprites_tab_start + (t + 12) * 6));
            trophies_won[t].unknown0 = trophy[i].unknown0;
            trophies_won[t].f1 = trophy[i].f1;
            trophies_won[t].value = trophy[i].value;
            trophies_won[t++].f4 = trophy[i].f4;
        }
    }
    for (i = 0; i < 13 && a < 6; i++) {
        if (all_awards[i].f2) {
            if (t == 3 && a == 5)
                reqspr_draw(D_0015f43c[a + 0x4f].unknownC, D_0015f43c[a + 0x4f].unknownE, &screen_sprites_tab_start[24]);
            else
                reqspr_draw(D_0015f43c[a + 0x4f].unknownC, D_0015f43c[a + 0x4f].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + (a + 4) * 6));
            reqspr_draw(D_0015f43c[a + 0x4f].unknownC, D_0015f43c[a + 0x4f].unknownE, &screen_sprites_tab_start[23]);
            if ((s8)all_awards[i].unknown0 == 1)
                reqspr_draw(D_0015f434[a + 0x9a].x, D_0015f434[a + 0x9a].y, &screen_sprites_tab_start[10]);
            else
                reqspr_draw(D_0015f434[a + 0x9a].x, D_0015f434[a + 0x9a].y, &screen_sprites_tab_start[11]);
            awards_won[a].f1 = all_awards[i].f1;
            awards_won[a].f2 = all_awards[i].f2;
            awards_won[a].unknown0 = all_awards[i].unknown0;
            awards_won[a].value = all_awards[i].value;
            awards_won[a].f5 = all_awards[i].f5;
            a++;
        }
    }
}
