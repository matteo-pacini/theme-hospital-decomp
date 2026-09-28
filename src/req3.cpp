#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct ReqLayout {
    u8 unknown0[0x230];
    s32 graph_x;
    s32 graph_bottom;
    s32 graph_width;
    s32 graph_height;
    u8 unknown240[0x120];
    s32 gauge2_x;
    s32 gauge2_y;
    s32 gauge3_x;
    s32 gauge3_y;
    u8 unknown370[0x70];
    Box box_a;
    Box box_b;
    u8 unknown400[0xf0];
    Box next;
    Box diag;
    Box box_c;
    Box box_d;
    u8 unknown530[0x20];
    s32 gauge_x;
    s32 gauge_y;
    s32 gauge_len;
};

extern "C" {
extern Slot D_001581b2[];
extern u8 *D_0015f402;
extern Event D_001549d0[];
extern u16 D_00155444;
extern ReqLayout *D_0015f40c;
extern ReqLayout *D_0015f410;
extern u16 *D_0015f406;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern u8 D_001553ed;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00155e19;
extern u8 D_001553f0;
extern u8 D_001553c8;
extern MenuItem *D_0015f420;
extern u16 D_00155c64;
extern char D_0015544e[];
extern u16 D_0012dd7c[];
extern TbScreenMode D_0015f3f5;
extern char **D_001602a4;
extern char D_001205e0[];
extern char D_001205e4[];
extern char D_001205e7[];
extern char D_001205eb[];
extern s32 D_00188c38;
void fn_0001b7e0(char *dst, char *src, u8 mode);
void fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
}

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);
void draw_health_graph(Thing *w, u16 x, u16 y, u16 colour);
void draw_diagnosis(Thing *w, u16 x, u16 y);
u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);

// FUNCTION: 0x00072d60
void req3(Window *win)
{
    WindowButton *b;
    Thing *w;
    s32 id;
    u16 i;

    b = win->buttons;
    w = &thing[D_001581b2[1].value];
    for (i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = i;
            debug.unknown8 = i;
            if (i == D_0015f402[0xd]) {
                D_001549d0[player].type = 0x43;
                D_001549d0[player].value = 1;
                play_sound(10, 0x42, -1, -1);
            } else if (i == D_0015f402[0xe]) {
                if (w->unknown146 >= D_0012dd7c[0]) {
                    display_mode = 1;
                    chart_type = 1;
                    drugs.unknown2 = 0;
                    set_mouse_mode(9);
                    D_00155444 = w->style;
                    play_sound(10, 0x43, -1, -1);
                }
            } else if (i == D_0015f402[0xf]) {
                if (!(w->flags & 0x2000 || w->flags & 0x8000)) {
                    if (w->unknown130 > 0) {
                        D_001549d0[player].type = 0x74;
                        D_001549d0[player].value = D_001581b2[1].value;
                        play_sound(10, 0xaa, -1, -1);
                    }
                }
            } else if (i == D_0015f402[0x10]) {
                if (w->unknown140) {
                    D_001549d0[player].type = 0x3b;
                    D_001549d0[player].value = w->unknown140;
                    D_001549d0[player].arg = 0;
                    play_sound(10, 0x43, -1, -1);
                }
            } else if (i == D_0015f402[0x11]) {
                if (!(w->unknown171 == 0 || w->flags & 0x1000 || w->flags & 0x4000 || w->style == w->unknown169)
                    && w->unknown146 >= D_0012dd7c[0]) {
                    if (!(w->unknown171 == 0 || w->flags & 0x1000)) {
                        if (w->unknown130 > 0) {
                            D_001549d0[player].type = 0x86;
                            D_001549d0[player].value = D_001581b2[1].value;
                            play_sound(10, 0x43, -1, -1);
                        }
                    }
                }
            }
        }
        i++;
    }
}

// FUNCTION: 0x0007308c
void req3_overdraw(Window *win)
{
    Thing *w;
    s32 unused;
    s32 x;
    s32 y;
    s32 n;

    w = &thing[D_001581b2[1].value];
    n = 0x3e;
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = win->x - D_00188c3c;
    y = win->y - D_00188c40;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_001553ed = window.unknown2ec - 1;
    if (D_001581b2[1].f6)
        D_001581b2[1].f6--;
    if (w->unknown146 < D_0012dd7c[0]) {
        LbDrawBox(x + D_0015f40c->box_a.x, y + D_0015f40c->box_a.y, D_0015f40c->box_a.w, D_0015f40c->box_a.h, 0x46);
        LbDrawBox(x + D_0015f40c->box_c.x, y + D_0015f40c->box_c.y, D_0015f40c->box_c.w, D_0015f40c->box_c.h, 0x46);
    }
    if (!w->unknown140)
        LbDrawBox(x + D_0015f40c->box_b.x, y + D_0015f40c->box_b.y, D_0015f40c->box_b.w, D_0015f40c->box_b.h, 0x46);
    if (w->unknown171 == 0 || w->flags & 0x1000 || w->flags & 0x4000 || w->style == w->unknown169
        || w->unknown146 < D_0012dd7c[0]) {
        LbDrawBox(x + D_0015f40c->box_c.x, y + D_0015f40c->box_c.y, D_0015f40c->box_c.w, D_0015f40c->box_c.h, 0x46);
        if (w->flags & 0x2000 || w->flags & 0x8000)
            LbDrawBox(x + D_0015f40c->box_d.x, y + D_0015f40c->box_d.y, D_0015f40c->box_d.w, D_0015f40c->box_d.h, 0x46);
    }
    draw_gauge(w->unknown81s, x + D_0015f40c->gauge_x, y + D_0015f40c->gauge_y, (u16)D_0015f40c->gauge_len,
               &button_sprite[D_0015f406[0]], 100, 0, 100, 1);
    draw_gauge(w->unknown134, x + D_0015f410->gauge2_x, y + D_0015f410->gauge2_y, (u16)D_0015f40c->gauge_len,
               &button_sprite[D_0015f406[0x10]], 100, 0, 100, 1);
    draw_gauge((s16)w->unknown83w, x + D_0015f410->gauge3_x, y + D_0015f410->gauge3_y, (u16)D_0015f40c->gauge_len,
               &button_sprite[D_0015f406[0x11]], 100, 0, 100, 1);
    if (D_001581b2[1].f4)
        draw_health_graph(w, x, y, color.unknownD);
    else
        draw_diagnosis(w, x, y);
    if (D_00188c7e) {
        if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c->next.x, y + D_0015f40c->next.y, D_0015f40c->next.w,
                         D_0015f40c->next.h)) {
            D_001553f0 = 1;
            D_001553c8 = 1;
        } else if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c->diag.x, y + D_0015f40c->diag.y,
                                D_0015f40c->diag.w, D_0015f40c->diag.h)) {
            if (!D_001581b2[1].f6) {
                D_001581b2[1].f4 = !D_001581b2[1].f4 ? 1 : 0;
                D_001581b2[1].f6 = 5;
            }
        }
    }
    if (D_00188c7f && !D_00155e19) {
        if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c->next.x, y + D_0015f40c->next.y, D_0015f40c->next.w,
                         D_0015f40c->next.h)) {
            u16 old;

            old = D_001581b2[1].value;
            D_001581b2[1].value = find_next_thing(w, 2, -1, -1);
            if (old != D_001581b2[1].value)
                play_sound(10, 0xbb, -1, -1);
        }
    }
    if (D_00188c7f)
        D_00155e19 = 1;
    else
        D_00155e19 = 0;
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0);
    compute_help_text(0, 0, 9);
}

// FUNCTION: 0x000736d4
void close_query(u8 i)
{
    close_irequesters();
    D_001581b2[i].kind = 0x63;
}

// NONMATCHING: register choice
// FUNCTION: 0x00073704
void draw_health_graph(Thing *w, u16 x, u16 y, u16 colour)
{
    u8 i;
    u8 step;
    u8 n;
    u16 y1;
    u16 x2;
    u16 y2;
    s16 h1;
    s16 h2;
    u16 x1;

    h1 = 0;
    h2 = 0;
    n = 0;
    step = D_0015f40c->graph_width / 10;
    for (i = 1; i < 9; i++) {
        h1 = w->unknown158[i] * 10;
        h2 = w->unknown158[i + 1] * 10;
        h1 = D_0015f40c->graph_height * h1 / 1000;
        h2 = h2 * D_0015f40c->graph_height / 1000;
        if (h1 < 1)
            h1 = 1;
        else if (h1 > D_0015f40c->graph_height)
            h1 = D_0015f40c->graph_height;
        if (h2 < 1)
            h2 = 1;
        else if (h2 > D_0015f40c->graph_height)
            h2 = D_0015f40c->graph_height;
        y1 = D_0015f40c->graph_bottom - h1;
        y2 = D_0015f40c->graph_bottom - h2;
        x1 = n++ * step + D_0015f40c->graph_x;
        x2 = D_0015f40c->graph_x + n * step;
        fn_000c9be0(x + x1, y + y1, x + x2, y + y2, colour);
    }
}

// FUNCTION: 0x000738e8
void draw_diagnosis(Thing *w, u16 x, u16 y)
{
    u8 i;
    u16 dy;
    char buf[100];

    LbDrawBox(x + D_0015f40c->diag.x, y + D_0015f40c->diag.y, D_0015f40c->diag.w, D_0015f40c->diag.h, 0xa9);
    LbTextSetClipWindow(x + D_0015f40c->diag.x, y + D_0015f40c->diag.y, D_0015f40c->diag.w, D_0015f40c->diag.h);
    set_font_from_cache(3);
    dy = 0;
    for (i = 0; i < 10; i++) {
        if (!w->unknown148[i])
            break;
        if (D_0015f3f5 == SCREEN_MODE_13) {
            sprintf(buf, D_001205e0, D_001602a4[w->unknown148[i]]);
            fn_0001b7e0(buf, buf, 0);
        } else {
            sprintf(buf, D_001205e4, D_001602a4[w->unknown148[i]]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + D_0015f40c->diag.x, dy + y + D_0015f40c->diag.y, buf);
    }
    if (w->flags & 0x2000) {
        if (D_0015f3f5 == SCREEN_MODE_13) {
            sprintf(buf, D_001205e7, info_bar_text[8]);
            fn_0001b7e0(buf, buf, 0);
        } else {
            sprintf(buf, D_001205eb, info_bar_text[8]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + D_0015f40c->diag.x, dy + y + D_0015f40c->diag.y, buf);
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
}

// FUNCTION: 0x00073bb0
u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2)
{
    u8 found;

    found = 0;
    w++;
    if (w >= end_things)
        w = thing;
    if (sub1 >= 0) {
        for (; w < end_things; w++) {
            if (w->renderer == kind) {
                if (w->subtype == sub1 || w->subtype == sub2) {
                    if (w->owner == player) {
                        if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0)) {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (w = thing; w < end_things; w++) {
                if (w->renderer == kind) {
                    if (w->subtype == sub1 || w->subtype == sub2) {
                        if (w->owner == player) {
                            if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0))
                                break;
                        }
                    }
                }
            }
        }
    } else {
        for (; w < end_things; w++) {
            if (w->renderer == kind) {
                if (w->owner == player) {
                    if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0)) {
                        if (w->renderer == 6) {
                            if (objects[w->subtype].f20) {
                                found = 1;
                                break;
                            }
                        } else {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (w = thing; w < end_things; w++) {
                if (w->renderer == kind) {
                    if (w->owner == player) {
                        if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0)) {
                            if (w->renderer == 6) {
                                if (objects[w->subtype].f20)
                                    break;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return w - thing;
}

// FUNCTION: 0x00073fbc
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up)
{
    u32 n;
    u32 i;

    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    if (up)
        n = len * value / range;
    else
        n = len - len * value / range;
    for (i = 0; i < n; i++)
        reqspr_draw(x + i, y, spr);
}

// FUNCTION: 0x00074064
extern "C" void fn_00074064(void)
{
    Thing *w;

    w = &thing[D_001581b2[1].value];
    if (w->unknown146 >= D_0012dd7c[0])
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x000740e4
extern "C" void fn_000740e4(void)
{
    Thing *w;

    w = &thing[D_001581b2[1].value];
    if (w->unknown171 != 0 && !(w->flags & 0x1000) && !(w->flags & 0x4000) && w->style != w->unknown169
        && w->unknown146 >= D_0012dd7c[0])
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x000741a4
extern "C" void fn_000741a4(void)
{
    Thing *w;

    w = &thing[D_001581b2[1].value];
    if (w->unknown140)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x00073e64
u16 fn_00073e64(u16 id, u32 flags)
{
    Thing *w;
    u8 found;

    w = &thing[id];
    found = 0;
    w++;
    if (w >= end_things)
        w = thing;
    for (; w < end_things; w++) {
        if (w->renderer == 2) {
            if (w->owner == player) {
                if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0)) {
                    if (w->flags32 & flags) {
                        found = 1;
                        break;
                    }
                }
            }
        }
    }
    if (!found) {
        for (w = thing; w < end_things; w++) {
            if (w->renderer == 2) {
                if (w->owner == player) {
                    if (check_if_point_beyond_scroll_limit((u8)w->x.tile, w->y.tile, 0)) {
                        if (w->flags32 & flags)
                            break;
                    }
                }
            }
        }
    }
    return w - thing;
}
