#include "hosp.h"

struct AdvisorLayout {
    u8 unknown0[0x49c];
    s32 f49c;
    u8 unknown4a0[0x14];
    s32 f4b4;
    u8 unknown4b8[4];
    s32 f4bc;
    u8 unknown4c0[4];
    s32 f4c4;
    s32 f4c8;
    u8 unknown4cc[4];
    s32 f4d0;
    u8 unknown4d4[4];
    s32 f4d8;
    u8 unknown4dc[4];
    s32 f4e0;
    u8 unknown4e4[4];
    s32 f4e8;
    u8 unknown4ec[4];
    s32 f4f0;
    s32 f4f4;
};

extern "C" {
extern char D_0015820c[];
extern u8 D_0015831e;
extern u8 D_0015831f;
extern u8 D_00158320;
extern s16 D_00158321;
extern u8 D_00161230;
extern u8 D_0015f40a;
extern u8 D_0015f40b;
extern AdvisorLayout *D_0015f410;
extern u16 D_00188c74;
char *strncpy(char *dst, const char *src, u32 n);
char *strncat(char *dst, const char *src, u32 n);
void fn_000b2124(void);
void fn_000b2178(u8 n);
}

// NONMATCHING: register choice
// FUNCTION: 0x000b1e10
void advisor_text_box(void)
{
    u32 i;
    s32 y;

    if (D_0015820c[0]) {
        set_font_from_cache(0x1d);
        if (D_00158320)
            fn_000b2124();
        if (D_0015831e == 3)
            y = D_0015f410->f4b4;
        else if (D_0015831e == 2)
            y = D_0015f410->f4bc;
        else if (D_0015831e == 1)
            y = D_0015f410->f4c4;
        if (D_00161230 <= D_0015f40b) {
            reqspr_draw(D_0015f410->f4f0, D_0015f410->f4f4, &game_panel[39]);
            reqspr_draw(D_0015f410->f4f0 - game_panel[37].width - (D_0015831f + D_0015f40a) * game_panel[38].width,
                        D_0015f410->f49c, &game_panel[37]);
            for (i = 0; i < D_0015f40a + D_0015831f; i++)
                reqspr_draw(D_0015f410->f4f0 - game_panel[38].width * (D_0015831f + D_0015f40a - i), D_0015f410->f49c,
                            &game_panel[38]);
            set_all_refresh_zones_in_window(
                D_0015f410->f4f0 - game_panel[38].width * (D_0015831f + D_0015f40a + 1) - game_panel[39].width,
                D_0015f410->f4f4, D_0015f410->f4f0 + game_panel[39].width, D_0015f410->f4f4 + game_panel[39].height, 3);
            D_00158321 = D_0015f410->f4f0 - game_panel[38].width * (D_0015831f + D_0015f40a + 1) - game_panel[39].width;
            LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
            LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
            D_00188c74 = 0;
            LbTextDraw(D_0015f410->f4f0 - (D_0015831f + D_0015f40a) * game_panel[38].width + D_0015f410->f4c8
                           - game_panel[37].width,
                       y, D_0015820c);
        }
    }
}

// FUNCTION: 0x000b2124
void fn_000b2124(void)
{
    D_00161230 = D_0015f40a;
    do {
        fn_000b2178(D_00161230);
        D_00161230++;
    } while (D_00161230 <= D_0015f40b && !D_0015831e);
    D_00158320 = 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x000b2178
void fn_000b2178(u8 n)
{
    u32 max;
    u32 width;
    u8 lines;
    char buf2[0x10e];
    char *out;
    char buf1[0x118];
    char *p;
    char *brk;
    char *start;

    max = n * D_0015f410->f4d8 + D_0015f410->f4e8 + D_0015f410->f4e0 - D_0015f410->f4c8 - D_0015f410->f4d0;
    width = 0;
    lines = 0;
    out = buf2;
    LbMemorySet(buf1, 0, 0x118);
    LbMemorySet(buf2, 0, 0x10e);
    LbDataLoadSetModifyFilenameFunction(buf1, D_0015820c);
    p = buf1;
    brk = buf1;
    start = buf1;
    do {
        do {
            width += lbFontPtr ? lbFontPtr[(u8)*p - 0x1f].width : 0;
            if (*p == ' ' || *p == ',' || *p == '.' || *p == ';' || *p == ':')
                brk = p;
            p++;
        } while (width <= max && *p != '\0');
        if (!*p)
            brk = p;
        width = 0;
        lines++;
        if (lines == 1) {
            strncpy(buf2, start, brk - start);
            out += brk - start;
        } else {
            strncat(out, start, brk - start);
            out += brk - start + 1;
        }
        *out = '\n';
        if (*p) {
            p = brk + 1;
            brk = p;
            start = p;
        }
    } while (lines < 3 && *p != '\0');
    if (!*p) {
        LbDataLoadSetModifyFilenameFunction(D_0015820c, buf2);
        if (lines < 3)
            D_0015831e = lines;
        else
            D_0015831e = lines;
        D_0015831f = n - D_0015f40a;
    }
}
