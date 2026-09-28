#include "hosp.h"

struct HighScore {
    char name[0x20];
    s32 score;
};

struct HospScores {
    HighScore fame[10];
    HighScore shame[10];
};

struct ScoreText {
    union {
        Point p[0xd0];
        struct {
            u8 unknown0[0x5f8];
            Point fame[10];
        };
        struct {
            u8 unknown0_[0x640];
            Point hall[10];
        };
    };
};

extern "C" {
extern u32 D_00188c38;
extern u16 D_00188c74;
extern ScoreText *D_0015f434;
extern Button *D_0015f43c;
extern u8 D_00155d9c;
extern u8 *D_00155d8f;
extern s32 D_00155d93;
extern u32 D_00155d97;
extern s16 D_00155da3;
extern u16 D_00155da5;
extern u8 D_001553c8;
extern u8 D_00188c7e, D_00188c81;
extern volatile s16 D_00188c6c, D_00188c70;
extern char D_001209e8[], D_001209ec[], D_001209f0[], D_001209f4[], D_001209f7[];
extern char D_001209fa[], D_00120a0d[];
extern char D_001563c5[];

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "W?GetCurrentPlayersCallback$n(pn$TbNetworkCallbackData$$pnv)v"
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
extern HospScores hosp_scores;

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
    old = D_00155d9c;
    D_00155d9c = TextPos[0] < TextPos[5] ? (u8)1 : (u8)0;
    if (D_00155d9c != old) {
        if (D_00155d9c) {
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
        text[i] = D_0015f434->fame[i].y << 16;
    saved = TitleText;
    TitleText = hall_text[4];
    for (i = 0; i < 20; i++) {
        c = TitleText[i];
        title[i] = (D_00188c34 - LbTextStringWidth(TitleText)) / 2;
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
        text[i] = D_0015f434->p[200 - i].y << 16;
    saved = TitleText;
    TitleText = hall_text[5];
    for (i = 0; i < 20; i++) {
        title[i] = (D_00188c34 + LbTextStringWidth(TitleText)) / 2;
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
    D_00155d8f = lbDisplay.unknown4;
    D_00155d93 = D_00188c34;
    D_00155d97 = D_00188c38;
    lbDisplay.unknown4 = bscreen;
    D_00188c34 = lbDisplay.unknown18;
    D_00188c38 = lbDisplay.unknown1c;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    for (i = 1; D_0015f434->hall[i].y != 0; i++)
        LbTextDraw(D_0015f434->hall[i].x, D_0015f434->hall[i].y, hall_text[i]);
    lbDisplay.unknown4 = D_00155d8f;
    D_00188c34 = D_00155d93;
    D_00188c38 = D_00155d97;
    LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
}

// NONMATCHING: register choice; post-decrement test (test ax,ax)
// FUNCTION: 0x00094b28
void fn_00094b28(void)
{
    s8 i;

    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0x10);
    D_00188c74 = 0;
    switch (hall.unknown2) {
    case 0:
        load_ingame_pic(0xe, 1, 0x16, 0, SCREEN_MODE_13);
        load_highscores();
        fn_00094a18();
        hall.unknown2 = 1;
        D_00155da5 = 1200;
        for (i = 0; i < 20; i++)
            TitlePos[i] = DesiredTitlePos[i] = D_00188c34 / 2;
        if (D_00155d9c) {
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
        D_001553c8 = 1;
        year_end_type++;
        break;
    default:
        break;
    }
    check_hall_buttons();
    if (D_00155da3 != mouse_dx + mouse_dy) {
        D_00155da5 = 1200;
        D_00155da3 = mouse_dx + mouse_dy;
    }
    if (!D_00155da5--)
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
    if (D_00155d9c)
        p = TitleText;
    else
        p = &TitleText[TitleLen - 1];
    for (i = 0; i < TitleLen; i++) {
        x = (TitlePos[i] + 0x8000) >> 16;
        c = p[1];
        p[1] = 0;
        LbTextDraw(x, D_0015f434->hall[5].y, p);
        x += LbTextStringWidth(p);
        p[1] = c;
        if (D_00155d9c)
            p++;
        else
            p--;
    }
    for (i = 0; i < 10; i++) {
        x = (TextPos[i] + 0x8000) >> 16;
        sprintf(buf, D_001209e8, i + 1);
        LbTextDraw(D_0015f434->hall[1].x, x, buf);
        if (D_00155d9c) {
            LbTextDraw(D_0015f434->hall[2].x, x, hosp_scores.fame[i].name);
            sprintf(buf, D_001209ec, hosp_scores.fame[i].score);
            LbTextDraw(D_0015f434->hall[3].x, x, buf);
        } else {
            LbTextDraw(D_0015f434->hall[2].x, x, hosp_scores.shame[9 - i].name);
            sprintf(buf, D_001209f0, hosp_scores.shame[9 - i].score);
            LbTextDraw(D_0015f434->hall[3].x, x, buf);
        }
    }
}

// FUNCTION: 0x00094f5c
void fn_00094f5c(s8 slot)
{
    sprintf(hosp_scores.fame[slot].name, D_001209f4, &allhosps[player]);
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
    sprintf(hosp_scores.shame[slot].name, D_001209f7, &allhosps[player]);
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
        D_00155d9c = 1;
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
        D_00155d9c = 0;
    }
}

// FUNCTION: 0x0009536c
void process_hall_buttons(u8 n)
{
    switch (n) {
    case 0:
    case 1:
        if (D_00155d9c) {
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

    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0xa9].width != 0 && D_0015f43c[i + 0xa9].height != 0; i++) {
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f43c[i + 0xa9].x, D_0015f43c[i + 0xa9].y, D_0015f43c[i + 0xa9].width, D_0015f43c[i + 0xa9].height)) {
                reqspr_draw(D_0015f43c[i + 0xa9].unknownC, D_0015f43c[i + 0xa9].unknownE, &screen_sprites_tab_start[D_0015f43c[i + 0xa9].sprite]);
                break;
            }
        }
    }
    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0xa9].width != 0 && D_0015f43c[i + 0xa9].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0xa9].x, D_0015f43c[i + 0xa9].y, D_0015f43c[i + 0xa9].width, D_0015f43c[i + 0xa9].height)) {
                D_0015f43c[i + 0xa9].callback(i);
                reqspr_draw(D_0015f43c[i + 0xa9].unknownC, D_0015f43c[i + 0xa9].unknownE, &screen_sprites_tab_start[D_0015f43c[i + 0xa9].sprite]);
                break;
            }
        }
        D_00188c81 = 0;
    }
}

// FUNCTION: 0x00095710
void save_highscores(void)
{
    int fh;
    char path[100];

    if (statvar.unknownfd <= 1 || (statvar.unknownfd > 1 && player == 0)) {
        sprintf(path, D_001209fa, D_001563c5);
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

    sprintf(path, D_00120a0d, D_001563c5);
    fh = LbFileOpen(path, 2);
    if (fh != -1) {
        LbFileRead(fh, &hosp_scores, 0x2d0);
        LbFileClose(fh);
    }
}
