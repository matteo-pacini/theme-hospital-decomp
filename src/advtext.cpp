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

/*
 * The advisor speech bubble's entries (0x93 to 0x9e) of the in-game position
 * table, read through resvars.coords.
 */
struct AdvisorLayout {
    u8 unknown0[0x49c];
    // 0x49c: Y of the bubble's left end and middle pieces.
    s32 bubble_y;
    u8 unknown4a0[0x14];
    // 0x4b4: Text y when the advice wraps to three lines.
    s32 text_y_3_lines;
    u8 unknown4b8[4];
    // 0x4bc: Text y for two lines.
    s32 text_y_2_lines;
    u8 unknown4c0[4];
    // 0x4c4: Text y for one line.
    s32 text_y_1_line;
    // 0x4c8: Text x offset from the left end piece; also taken off the wrap
    // width.
    s32 text_left_margin;
    u8 unknown4cc[4];
    // 0x4d0: Taken off the wrap width.
    s32 text_right_margin;
    u8 unknown4d4[4];
    // 0x4d8: Wrap width added per middle piece.
    s32 middle_text_width;
    u8 unknown4dc[4];
    // 0x4e0: Wrap width added for one end piece.
    s32 left_text_width;
    u8 unknown4e4[4];
    // 0x4e8: Wrap width added for the other end piece.
    s32 right_text_width;
    u8 unknown4ec[4];
    // 0x4f0: X of the bubble's right end (game_panel[39]); the other pieces
    // extend left from it.
    s32 right_piece_x;
    // 0x4f4: Y of the bubble's right end.
    s32 right_piece_y;
};

extern "C" {
extern u8 no_pieces;
extern u8 D_00156910;
extern FrameBox D_001567b0;
char *strncpy(char *dst, const char *src, u32 len);
char *strncat(char *dst, const char *t, u32 n);
void process_advisor_text(void);
void format_advisor_text(u8 segments);
}

extern const u16 advisor_anims[];

void get_advisor(void);

// Draws the advisor's speech bubble with the current advice (advisor.text),
// re-wrapping the text first when advisor.rewrap is set. The bubble grows to the
// left by as many middle pieces as the text needs (advisor.extra_segments), and its left
// edge is kept in advisor.box_left. Draws nothing if wrapping left no_pieces past
// resvars.advisor_max_segments (the text did not fit).
// FUNCTION: 0x000b1e10
void advisor_text_box(void)
{
    u32 i;
    s32 y;

    if (advisor.text[0]) {
        set_font_from_cache(0x1d);
        if (advisor.rewrap)
            process_advisor_text();
        if (advisor.line_count == 3)
            y = resvars.coords_AdvisorLayoutp->text_y_3_lines;
        else if (advisor.line_count == 2)
            y = resvars.coords_AdvisorLayoutp->text_y_2_lines;
        else if (advisor.line_count == 1)
            y = resvars.coords_AdvisorLayoutp->text_y_1_line;
        if (no_pieces <= resvars.advisor_max_segments) {
            reqspr_draw(resvars.coords_AdvisorLayoutp->right_piece_x, resvars.coords_AdvisorLayoutp->right_piece_y, &game_panel[39]);
            reqspr_draw(resvars.coords_AdvisorLayoutp->right_piece_x - game_panel[37].width - (advisor.extra_segments + resvars.advisor_min_segments) * game_panel[38].width,
                        resvars.coords_AdvisorLayoutp->bubble_y, &game_panel[37]);
            for (i = 0; i < resvars.advisor_min_segments + advisor.extra_segments; i++)
                reqspr_draw(resvars.coords_AdvisorLayoutp->right_piece_x - (advisor.extra_segments + resvars.advisor_min_segments - i) * game_panel[38].width, resvars.coords_AdvisorLayoutp->bubble_y,
                            &game_panel[38]);
            set_all_refresh_zones_in_window(
                resvars.coords_AdvisorLayoutp->right_piece_x - (advisor.extra_segments + resvars.advisor_min_segments + 1) * game_panel[38].width - game_panel[39].width,
                resvars.coords_AdvisorLayoutp->right_piece_y, resvars.coords_AdvisorLayoutp->right_piece_x + game_panel[39].width, resvars.coords_AdvisorLayoutp->right_piece_y + game_panel[39].height, 3);
            advisor.box_left = resvars.coords_AdvisorLayoutp->right_piece_x - game_panel[38].width * (u16)(advisor.extra_segments + resvars.advisor_min_segments + 1) - game_panel[39].width;
            LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
            LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
            lbDisplay.draw_flags = 0;
            LbTextDraw(resvars.coords_AdvisorLayoutp->right_piece_x - (advisor.extra_segments + resvars.advisor_min_segments) * game_panel[38].width + resvars.coords_AdvisorLayoutp->text_left_margin
                           - game_panel[37].width,
                       y, advisor.text);
        }
    }
}

// Wraps the advice text into the narrowest bubble that holds it, trying widths
// from resvars.advisor_min_segments middle pieces up to resvars.advisor_max_segments, until advisor.line_count is set.
// Leaves no_pieces one past the width tried last and clears advisor.rewrap.
// FUNCTION: 0x000b2124
void process_advisor_text(void)
{
    no_pieces = resvars.advisor_min_segments;
    do {
        format_advisor_text(no_pieces);
        no_pieces++;
    } while (no_pieces <= resvars.advisor_max_segments && !advisor.line_count);
    advisor.rewrap = 0;
}

// Tries to wrap advisor.text into at most three lines for a bubble segments
// middle pieces wide, breaking after spaces and punctuation. If the whole text
// fits, it replaces advisor.text with the wrapped text, sets advisor.line_count to the
// line count and advisor.extra_segments to the extra pieces used; otherwise it leaves
// everything unchanged.
// FUNCTION: 0x000b2178
void format_advisor_text(u8 segments)
{
    u32 max_width;
    u32 width;
    u8 lines;
    char wrapped[0x10e];
    char *out;
    char text[0x118];
    char *p;
    char *brk;
    char *line_start;

    max_width = segments * resvars.coords_AdvisorLayoutp->middle_text_width + resvars.coords_AdvisorLayoutp->right_text_width + resvars.coords_AdvisorLayoutp->left_text_width - resvars.coords_AdvisorLayoutp->text_left_margin - resvars.coords_AdvisorLayoutp->text_right_margin;
    width = 0;
    lines = 0;
    out = wrapped;
    LbMemorySet(text, 0, 0x118);
    LbMemorySet(wrapped, 0, 0x10e);
    fn_000bbff0(text, advisor.text);
    p = text;
    brk = text;
    line_start = text;
    do {
        do {
            width += lbFontPtr ? lbFontPtr[(u8)*p - 0x1f].width : 0;
            if (*p == ' ' || *p == ',' || *p == '.' || *p == ';' || *p == ':')
                brk = p;
            p++;
        } while (width <= max_width && *p != '\0');
        if (!*p)
            brk = p;
        width = 0;
        lines++;
        if (lines == 1) {
            strncpy(wrapped, line_start, brk - line_start);
            out += brk - line_start;
        } else {
            strncat(out, line_start, brk - line_start);
            out += brk - line_start + 1;
        }
        *out = '\n';
        if (*p) {
            p = brk + 1;
            brk = p;
            line_start = p;
        }
    } while (lines < 3 && *p != '\0');
    if (!*p) {
        fn_000bbff0(advisor.text, wrapped);
        if (lines < 3)
            advisor.line_count = lines;
        else
            advisor.line_count = lines;
        advisor.extra_segments = segments - resvars.advisor_min_segments;
    }
}

// Draws and animates the advisor character for advisor.anim_state (its animation
// state), first bringing it on screen if advice is pending (advisor.advice_id). Idle
// states occasionally switch to a special animation; looping states move to
// next_advisor_state when their animation ends.
// FUNCTION: 0x000b242c
void advisor_character(void)
{
    CompFrame *f;
    s32 loop;
    u32 speed;

    speed = 0;
    if (advisor.advice_id_u)
        get_advisor();
    switch (advisor.anim_state) {
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
        advisor.state_advanced = 0;
        break;
    case 9:
    case 10:
        speed = 4;
        loop = 1;
        break;
    case 2:
        advisor.state_advanced = 0;
    case 3:
        if (saferand() % 50 == 0) {
            advisor.anim_state = advisor_specials[saferand() % 2];
            advisor.anim_frame = start[advisor_anims[advisor.anim_state]].frame;
        }
    case 7:
        speed = 2;
        loop = 0;
        break;
    }
    if (speed) {
        draw_a_frame(advisor.anim_frame, advisor.x, advisor.y, 0, 0);
        set_all_refresh_zones_in_window(D_001567b0.x, D_001567b0.y, D_001567b0.x + D_001567b0.w, D_001567b0.y + D_001567b0.h, 3);
        if (actual_game_turn % speed == 0 && !gamevar.paused) {
            advisor.anim_frame = frame[advisor.anim_frame].next;
            f = &frame[advisor.anim_frame];
            if (loop && (f->flags & 1)) {
                advisor.anim_state = next_advisor_state[advisor.anim_state];
                advisor.anim_frame = start[advisor_anims[advisor.anim_state]].frame;
            }
        }
    }
}

// Starts the advisor's entry animation if the advisor is not on screen.
// FUNCTION: 0x000b2678
void get_advisor(void)
{
    if (advisor.anim_state == 0) {
        advisor.anim_state = 1;
        advisor.anim_frame = start[advisor_anims[advisor.anim_state]].frame;
    }
}
