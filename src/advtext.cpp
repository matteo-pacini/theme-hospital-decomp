#include "hosp.h"

struct FrameBox {
    s16 x0;
    s16 y0;
    s16 x1;
    s16 y1;
    s16 x;
    s16 y;
    s16 w;
    s16 h;
};

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
extern u8 D_00161230;
extern u8 D_00156910;
extern FrameBox D_001567b0;
char *strncpy(char *dst, const char *src, u32 n);
char *strncat(char *dst, const char *src, u32 n);
void fn_000b2124(void);
void fn_000b2178(u8 segments);
}

extern const u16 advisor_anims[];

void get_advisor(void);

// FUNCTION: 0x000b1e10
void advisor_text_box(void)
{
    u32 i;
    s32 y;

    if (advisor.f10[0]) {
        set_font_from_cache(0x1d);
        if (advisor.f124)
            fn_000b2124();
        if (advisor.f122 == 3)
            y = resvars.f1c_AdvisorLayoutp->f4b4;
        else if (advisor.f122 == 2)
            y = resvars.f1c_AdvisorLayoutp->f4bc;
        else if (advisor.f122 == 1)
            y = resvars.f1c_AdvisorLayoutp->f4c4;
        if (D_00161230 <= resvars.f17) {
            reqspr_draw(resvars.f1c_AdvisorLayoutp->f4f0, resvars.f1c_AdvisorLayoutp->f4f4, &game_panel[39]);
            reqspr_draw(resvars.f1c_AdvisorLayoutp->f4f0 - game_panel[37].width - (advisor.f123 + resvars.f16) * game_panel[38].width,
                        resvars.f1c_AdvisorLayoutp->f49c, &game_panel[37]);
            for (i = 0; i < resvars.f16 + advisor.f123; i++)
                reqspr_draw(resvars.f1c_AdvisorLayoutp->f4f0 - (advisor.f123 + resvars.f16 - i) * game_panel[38].width, resvars.f1c_AdvisorLayoutp->f49c,
                            &game_panel[38]);
            set_all_refresh_zones_in_window(
                resvars.f1c_AdvisorLayoutp->f4f0 - (advisor.f123 + resvars.f16 + 1) * game_panel[38].width - game_panel[39].width,
                resvars.f1c_AdvisorLayoutp->f4f4, resvars.f1c_AdvisorLayoutp->f4f0 + game_panel[39].width, resvars.f1c_AdvisorLayoutp->f4f4 + game_panel[39].height, 3);
            advisor.f125 = resvars.f1c_AdvisorLayoutp->f4f0 - game_panel[38].width * (u16)(advisor.f123 + resvars.f16 + 1) - game_panel[39].width;
            LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
            LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
            lbDisplay.draw_flags = 0;
            LbTextDraw(resvars.f1c_AdvisorLayoutp->f4f0 - (advisor.f123 + resvars.f16) * game_panel[38].width + resvars.f1c_AdvisorLayoutp->f4c8
                           - game_panel[37].width,
                       y, advisor.f10);
        }
    }
}

// FUNCTION: 0x000b2124
void fn_000b2124(void)
{
    D_00161230 = resvars.f16;
    do {
        fn_000b2178(D_00161230);
        D_00161230++;
    } while (D_00161230 <= resvars.f17 && !advisor.f122);
    advisor.f124 = 0;
}

// FUNCTION: 0x000b2178
void fn_000b2178(u8 segments)
{
    u32 max;
    u32 width;
    u8 lines;
    char wrapped[0x10e];
    char *out;
    char text[0x118];
    char *p;
    char *brk;
    char *start;

    max = segments * resvars.f1c_AdvisorLayoutp->f4d8 + resvars.f1c_AdvisorLayoutp->f4e8 + resvars.f1c_AdvisorLayoutp->f4e0 - resvars.f1c_AdvisorLayoutp->f4c8 - resvars.f1c_AdvisorLayoutp->f4d0;
    width = 0;
    lines = 0;
    out = wrapped;
    LbMemorySet(text, 0, 0x118);
    LbMemorySet(wrapped, 0, 0x10e);
    fn_000bbff0(text, advisor.f10);
    p = text;
    brk = text;
    start = text;
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
            strncpy(wrapped, start, brk - start);
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
        fn_000bbff0(advisor.f10, wrapped);
        if (lines < 3)
            advisor.f122 = lines;
        else
            advisor.f122 = lines;
        advisor.f123 = segments - resvars.f16;
    }
}

// FUNCTION: 0x000b242c
void advisor_character(void)
{
    CompFrame *f;
    s32 loop;
    u32 speed;

    speed = 0;
    if (advisor.f0_u16)
        get_advisor();
    switch (advisor.f11e) {
    case 0:
        break;
    case 1:
    case 4:
    case 5:
    case 6:
    case 8:
        D_00156910 = 1;
        speed = 1;
        loop = 1;
        advisor.f12c = 0;
        break;
    case 9:
    case 10:
        speed = 4;
        loop = 1;
        break;
    case 2:
        advisor.f12c = 0;
    case 3:
        if (saferand() % 50 == 0) {
            advisor.f11e = advisor_specials[saferand() % 2];
            advisor.f6 = start[advisor_anims[advisor.f11e]].frame;
        }
    case 7:
        speed = 2;
        loop = 0;
        break;
    }
    if (speed) {
        draw_a_frame(advisor.f6, advisor.f8, advisor.fa, 0, 0);
        set_all_refresh_zones_in_window(D_001567b0.x, D_001567b0.y, D_001567b0.x + D_001567b0.w, D_001567b0.y + D_001567b0.h, 3);
        if (actual_game_turn % speed == 0 && !gamevar.unknown21) {
            advisor.f6 = frame[advisor.f6].next;
            f = &frame[advisor.f6];
            if (loop && (f->flags & 1)) {
                advisor.f11e = next_advisor_state[advisor.f11e];
                advisor.f6 = start[advisor_anims[advisor.f11e]].frame;
            }
        }
    }
}

// FUNCTION: 0x000b2678
void get_advisor(void)
{
    if (advisor.f11e == 0) {
        advisor.f11e = 1;
        advisor.f6 = start[advisor_anims[advisor.f11e]].frame;
    }
}
