#include "hosp.h"

struct TrackSlot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    u8 unknown4[0x7];
};

struct ScrollLayout {
    u8 unknown0[0x558];
    s32 unknown558;
    s32 unknown55c;
};

extern "C" {
extern TrackSlot D_001581b2[];
extern u8 D_001553ec[];
extern u8 D_001553f0;
extern TbScreenMode D_0015f3f4;
extern ScrollLayout *D_0015f410;
extern u16 D_001553e0;
extern u8 D_001553e1[];
}

extern s16 mapx;
extern s16 mapy;
extern s16 curx;
extern s16 cury;
extern s16 mcurx;
extern s16 mcury;

void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);

// NONMATCHING: register choice
// FUNCTION: 0x000ab610
void process_tracking_windows(void)
{
    u8 i;
    Thing *w;

    for (i = 0; i < 4; i++) {
        if (D_001553ec[i]) {
            w = &thing[D_001581b2[i].value];
            give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
            if (!check_if_point_beyond_scroll_limit(
                    info.unknown54 - D_0015f410->unknown558
                        + (D_0015f3f4 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16),
                    (D_0015f3f4 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                        + (info.unknown58 - D_0015f410->unknown55c),
                    1)) {
                window.f0[D_001553ec[i]] = (D_0015f3f4 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16)
                    + (info.unknown54 - D_0015f410->unknown558);
                window.fc[D_001553ec[i]] = (D_0015f3f4 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                    + (info.unknown58 - D_0015f410->unknown55c);
            } else {
                close_irequesters();
            }
            D_001553ec[i] = 0;
        }
    }
    if (D_001553f0) {
        w = &thing[D_001581b2[D_001553f0].value];
        give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
        if (!check_if_point_beyond_scroll_limit(
                info.unknown54 - D_0015f410->unknown558
                    + (D_0015f3f4 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16),
                (D_0015f3f4 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                    + (info.unknown58 - D_0015f410->unknown558),
                0)) {
            window.f0[0] = (D_0015f3f4 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 32 : ((w->x.frac - w->y.frac) >> 4) + 16)
                + (info.unknown54 - D_0015f410->unknown558);
            window.fc[0] = (D_0015f3f4 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5)
                + (info.unknown58 - D_0015f410->unknown55c);
        } else {
            close_irequesters();
        }
        D_001553f0 = 0;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000aba50
void set_map_cur_and_mcur_x_and_y(void)
{
    s32 x;
    s32 y;

    find_cell_at_point(window.unknown2eb, window.zone_x[window.unknown2eb], window.zone_y[window.unknown2eb]);
    mapx = info.unknown5c;
    mapy = info.unknown60;
    find_cell_at_point(window.unknown2eb, mouse_dx, mouse_dy);
    curx = info.unknown5c - mapx;
    cury = info.unknown60 - mapy;
    mcurx = info.unknown54;
    mcury = info.unknown58;
    if (info.unknown5c < 0)
        x = 0;
    else if (info.unknown5c >= 128)
        x = 127;
    else
        x = info.unknown5c;
    if (info.unknown60 >= 128)
        y = 127;
    else if (info.unknown60 < 0)
        y = 0;
    else
        y = info.unknown60;
    *(u16 *)&D_001553e1[1] = D_001553e0;
    D_001553e0 = x + (y << 7);
}

// FUNCTION: 0x000abb80
void goto_cell_in_window(const u8 a, const s32 b)
{
    goto_column_row_in_window(a, b % 128, b / 128);
}

// NONMATCHING: register choice
// FUNCTION: 0x000abbc8
void goto_column_row_in_window(const u8 a, const s32 x, const s32 y)
{
    if (x >= 0 && x < 128 && y >= 0 && y < 128) {
        give_absolute_xy_of_cell_at_column_row(x, y);
        window.f0[a] = info.unknown54;
        window.fc[a] = info.unknown58;
    }
}
