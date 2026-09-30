#include "hosp.h"

struct HighScore {
    char name[0x20];
    s32 score;
};

struct HospScores {
    HighScore fame[10];
    HighScore shame[10];
};

extern "C" {

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
int strcmp(const char *a, const char *b);
void fn_00094a18(void);
void fn_00094b28(void);
void fn_000951c8(void);
void fn_00094d34(void);
void fn_00094f5c(s8 slot);
void fn_00094fc8(void);
void fn_00095094(s8 slot);
void fn_00095100(void);
}

extern s32 TextPos[];
extern s32 DesiredTextPos[];
extern s32 TitlePos[];
extern s32 DesiredTitlePos[];
extern char *TitleText;
extern u8 TitleLen;
HospScores hosp_scores = {
    {
        {"WEBLEY", 250000},
        {"SAMUEL", 210000},
        {"JAYNE", 200000},
        {"GAZZA", 190000},
        {"CHIVERS", 175000},
        {"RAJ", 165000},
        {"WAZ", 145000},
        {"BEN", 99000},
        {"LORD RIDER", 79000},
        {"BASSEY", 40000},
    },
    {
        {"JAN SV.", 8000},
        {"COGGY", 10000},
        {"BRUTUS", 11000},
        {"NUTTERS", 12500},
        {"ROBBO", 16000},
        {"NATS", 16500},
        {"MR MAT", 17000},
        {"JON", 18000},
        {"ADIE", 19000},
        {"RUSS", 25000},
    },
};

void move_text(void);
void set_fame_pos(s32 *text, s32 *title);
void set_shame_pos(s32 *text, s32 *title);
void check_hall_buttons(void);
void save_highscores(void);
void load_highscores(void);

// FUNCTION: 0x00094500
void move_text(void)
{
    u8 old;
    s16 i;

    for (i = 0; i < 10; i++) {
        TextPos[i] += (DesiredTextPos[i] - TextPos[i] + 0x1000) >> 3;
        if (TextPos[i] > DesiredTextPos[i]) {
            if (TextPos[i] - DesiredTextPos[i] < 0x800)
                TextPos[i] = DesiredTextPos[i];
        }
        if (TextPos[i] < DesiredTextPos[i]) {
            if (DesiredTextPos[i] - TextPos[i] < 0x800)
                TextPos[i] = DesiredTextPos[i];
        }
    }
    old = othervar.f9e4;
    othervar.f9e4 = TextPos[0] < TextPos[5] ? (u8)1 : (u8)0;
    if (othervar.f9e4 != old) {
        if (othervar.f9e4) {
            TitleText = hall_text[4];
            TitleLen = fn_000cd716(TitleText);
        } else {
            TitleText = hall_text[5];
            TitleLen = fn_000cd716(TitleText);
        }
    }
    for (i = 0; i < 20; i++) {
        TitlePos[i] += (DesiredTitlePos[i] - TitlePos[i] + 0x1000) >> 3;
        if (TitlePos[i] > DesiredTitlePos[i]) {
            if (TitlePos[i] - DesiredTitlePos[i] < 0x800)
                TitlePos[i] = DesiredTitlePos[i];
        }
        if (TitlePos[i] < DesiredTitlePos[i]) {
            if (DesiredTitlePos[i] - TitlePos[i] < 0x800)
                TitlePos[i] = DesiredTitlePos[i];
        }
    }
}

// FUNCTION: 0x000947f0
void set_fame_pos(s32 *text, s32 *title)
{
    char *saved;
    s16 i;
    u8 c;

    for (i = 0; i < 10; i++)
        text[i] = resvars.f40[i + 191].y << 16;
    saved = TitleText;
    TitleText = hall_text[4];
    for (i = 0; i < 20; i++) {
        c = TitleText[i];
        title[i] = (lbDisplay.graphics_screen_width - LbTextStringWidth(TitleText)) / 2;
        TitleText[i] = 0;
        title[i] += LbTextStringWidth(TitleText);
        TitleText[i] = c;
        title[i] = title[i] << 16;
    }
    TitleText = saved;
}

// FUNCTION: 0x00094914
void set_shame_pos(s32 *text, s32 *title)
{
    char *saved;
    s16 i;

    for (i = 0; i < 10; i++)
        text[i] = resvars.f40[200 - i].y << 16;
    saved = TitleText;
    TitleText = hall_text[5];
    for (i = 0; i < 20; i++) {
        title[i] = (lbDisplay.graphics_screen_width + LbTextStringWidth(TitleText)) / 2;
        title[i] -= LbTextStringWidth(&TitleText[fn_000cd716(TitleText) - 1 - i]);
        title[i] = title[i] << 16;
    }
    TitleText = saved;
}

// FUNCTION: 0x00094a18
void fn_00094a18(void)
{
    s16 i;

    set_font_from_cache(0x10);
    othervar.f9d7 = lbDisplay.unknown4;
    othervar.f9db = lbDisplay.graphics_screen_width;
    othervar.f9df_u32 = lbDisplay.graphics_screen_height;
    lbDisplay.unknown4 = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.unknown18;
    lbDisplay.graphics_screen_height = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    for (i = 1; resvars.f40[i + 200].y != 0; i++)
        LbTextDraw(resvars.f40[i + 200].x, resvars.f40[i + 200].y, hall_text[i]);
    lbDisplay.unknown4 = othervar.f9d7;
    lbDisplay.graphics_screen_width = othervar.f9db;
    lbDisplay.graphics_screen_height = othervar.f9df_u32;
    LbScreenSetGraphicsWindow(0, 0, othervar.f9db, othervar.f9df_u32);
}

// FUNCTION: 0x00094b28
void fn_00094b28(void)
{
    s8 i;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x10);
    lbDisplay.draw_flags = 0;
    switch (hall.unknown2) {
    case 0:
        load_ingame_pic(0xe, 1, 0x16, 0, SCREEN_MODE_13);
        load_highscores();
        fn_00094a18();
        hall.unknown2 = 1;
        othervar.f9ed = 1200;
        for (i = 0; i < 20; i++)
            TitlePos[i] = DesiredTitlePos[i] = lbDisplay.graphics_screen_width / 2;
        if (othervar.f9e4) {
            set_fame_pos(DesiredTextPos, DesiredTitlePos);
            set_fame_pos(TextPos, TitlePos);
            TitleText = hall_text[4];
        } else {
            set_shame_pos(DesiredTextPos, DesiredTitlePos);
            set_shame_pos(TextPos, TitlePos);
            TitleText = hall_text[5];
        }
        TitleLen = fn_000cd716(TitleText);
        move_text();
        fn_00094d34();
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        move_text();
        fn_00094d34();
        break;
    case 2:
        set_font_from_cache(1);
        othervar.f10 = 1;
        year_end_type++;
        break;
    default:
        break;
    }
    check_hall_buttons();
    if (othervar.f9eb != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
        othervar.f9ed = 1200;
        othervar.f9eb = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
    }
    if (!othervar.f9ed--)
        hall.unknown2 = 2;
}

// FUNCTION: 0x00094d34
void fn_00094d34(void)
{
    u8 i;
    u8 c;
    u16 height;
    char *p;
    s32 x;
    char buf[100];

    set_font_from_cache(0x10);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    if (othervar.f9e4)
        p = TitleText;
    else
        p = &TitleText[TitleLen - 1];
    for (i = 0; i < TitleLen; i++) {
        x = (TitlePos[i] + 0x8000) >> 16;
        c = p[1];
        p[1] = 0;
        LbTextDraw(x, resvars.f40[205].y, p);
        x += LbTextStringWidth(p);
        p[1] = c;
        if (othervar.f9e4)
            p++;
        else
            p--;
    }
    for (i = 0; i < 10; i++) {
        x = (TextPos[i] + 0x8000) >> 16;
        sprintf(buf, "%2d", i + 1);
        LbTextDraw(resvars.f40[201].x, x, buf);
        if (othervar.f9e4) {
            LbTextDraw(resvars.f40[202].x, x, hosp_scores.fame[i].name);
            sprintf(buf, "%6d", hosp_scores.fame[i].score);
            LbTextDraw(resvars.f40[203].x, x, buf);
        } else {
            LbTextDraw(resvars.f40[202].x, x, hosp_scores.shame[9 - i].name);
            sprintf(buf, "%6d", hosp_scores.shame[9 - i].score);
            LbTextDraw(resvars.f40[203].x, x, buf);
        }
    }
}

// FUNCTION: 0x00094f5c
void fn_00094f5c(s8 slot)
{
    sprintf(hosp_scores.fame[slot].name, "%s", &allhosps[player]);
    hosp_scores.fame[slot].score = allhosps[player].f2d;
}

// FUNCTION: 0x00094fc8
void fn_00094fc8(void)
{
    s8 i;
    s8 j;
    u8 swapped;
    HighScore tmp;

    for (i = 0; i < 9; i++) {
        swapped = 0;
        for (j = 9; j > i; j--) {
            if (hosp_scores.fame[j].score > hosp_scores.fame[j - 1].score) {
                tmp = hosp_scores.fame[j];
                hosp_scores.fame[j] = hosp_scores.fame[j - 1];
                hosp_scores.fame[j - 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

// FUNCTION: 0x00095094
void fn_00095094(s8 slot)
{
    sprintf(hosp_scores.shame[slot].name, "%s", &allhosps[player]);
    hosp_scores.shame[slot].score = allhosps[player].f2d;
}

// FUNCTION: 0x00095100
void fn_00095100(void)
{
    s8 i;
    s8 j;
    u8 swapped;
    HighScore tmp;

    for (i = 9; i > 0; i--) {
        swapped = 0;
        for (j = 0; j < i; j++) {
            if (hosp_scores.shame[j].score > hosp_scores.shame[j + 1].score) {
                tmp = hosp_scores.shame[j];
                hosp_scores.shame[j] = hosp_scores.shame[j + 1];
                hosp_scores.shame[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

// FUNCTION: 0x000951c8
void fn_000951c8(void)
{
    s8 i;

    load_highscores();
    if (allhosps[player].f2d > hosp_scores.fame[9].score) {
        for (i = 0; i < 10; i++) {
            if (strcmp((char *)allhosps[player].unknown0, hosp_scores.fame[i].name) == 0)
                break;
        }
        if (i == 10) {
            i--;
            fn_00094f5c(i);
            fn_00094fc8();
            save_highscores();
        } else if (allhosps[player].f2d > hosp_scores.fame[i].score) {
            fn_00094f5c(i);
            fn_00094fc8();
            save_highscores();
        }
        othervar.f9e4 = 1;
    } else {
        if (allhosps[player].f2d > hosp_scores.shame[0].score) {
            for (i = 0; i < 10; i++) {
                if (strcmp((char *)allhosps[player].unknown0, hosp_scores.shame[i].name) == 0)
                    break;
            }
            if (i == 10) {
                i--;
                fn_00095094(i);
                fn_00095100();
                save_highscores();
            } else if (allhosps[player].f2d > hosp_scores.shame[i].score) {
                fn_00095094(i);
                fn_00095100();
                save_highscores();
            }
        }
        othervar.f9e4 = 0;
    }
}

// FUNCTION: 0x0009536c
void process_hall_buttons(u8 n)
{
    switch (n) {
    case 0:
    case 1:
        if (othervar.f9e4) {
            play_sound(10, 0x47, -1, -1);
            set_shame_pos(DesiredTextPos, DesiredTitlePos);
        } else {
            play_sound(10, 0x47, -1, -1);
            set_fame_pos(DesiredTextPos, DesiredTitlePos);
        }
        break;
    case 2:
        hall.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    default:
        break;
    }
}

// FUNCTION: 0x0009542c
void check_hall_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0xa9].width != 0 && resvars.f48[i + 0xa9].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f48[i + 0xa9].x, resvars.f48[i + 0xa9].y, resvars.f48[i + 0xa9].width, resvars.f48[i + 0xa9].height)) {
                reqspr_draw(resvars.f48[i + 0xa9].unknownC, resvars.f48[i + 0xa9].unknownE, &screen_sprites_tab_start[resvars.f48[i + 0xa9].sprite]);
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0xa9].width != 0 && resvars.f48[i + 0xa9].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0xa9].x, resvars.f48[i + 0xa9].y, resvars.f48[i + 0xa9].width, resvars.f48[i + 0xa9].height)) {
                resvars.f48[i + 0xa9].callback(i);
                reqspr_draw(resvars.f48[i + 0xa9].unknownC, resvars.f48[i + 0xa9].unknownE, &screen_sprites_tab_start[resvars.f48[i + 0xa9].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// FUNCTION: 0x00095710
void save_highscores(void)
{
    int fh;
    char path[100];

    if (statvar.unknownfd <= 1 || (statvar.unknownfd > 1 && player == 0)) {
        sprintf(path, "%ssave/hiscore.dat", statvar.f575);
        fh = LbFileOpen(path, 0);
        if (fh != -1) {
            LbFileWrite(fh, &hosp_scores, 0x2d0);
            LbFileClose(fh);
        }
    }
}

// FUNCTION: 0x00095790
void load_highscores(void)
{
    int fh;
    char path[100];

    sprintf(path, "%ssave/hiscore.dat", statvar.f575);
    fh = LbFileOpen(path, 2);
    if (fh != -1) {
        LbFileRead(fh, &hosp_scores, 0x2d0);
        LbFileClose(fh);
    }
}
