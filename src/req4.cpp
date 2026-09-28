#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct Picture;

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

extern "C" {
extern Slot D_001581b2[];
extern u8 *D_0015f402;
extern Event D_001549d0[];
extern char D_00155ced[];
extern char D_00120770[];
extern char D_00120773[];
extern char D_00120776[];
extern char D_00120779[];
extern char D_0012077c[];
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c38;
extern u8 D_001553ee;
extern u8 D_001553f0;
extern u8 D_001553c8;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00155e19;
extern u16 D_00155e3a;
extern u16 *D_0015f406;
extern Box *D_0015f40c;
extern Point *D_0015f410;
extern Window *D_0015f414;
extern u8 D_00155d70;
extern char D_00120785[];
extern u8 D_00156429;
extern u8 D_0015642a;
extern u8 D_0015642b;
extern char D_0015544e[];
extern MenuItem *D_0015f420;
extern u16 D_00155c64;
}

void remove_window_from_list(const u8 n);

u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);
void draw_face(Picture *p, const u16 x, const u16 y);
void draw_staff_handyman_bars(const Thing *const w, const u16 x, const u16 y, TbSprite *spr);

// FUNCTION: 0x00086c10
void req4(Window *win)
{
    WindowButton *b;
    Thing *w;
    u16 i;

    b = win->buttons;
    w = &thing[D_001581b2[2].value];
    switch (w->subtype) {
    case 2:
        for (i = 1; b; b = b->next) {
            if (b->flags & 6) {
                debug.unknown4 = i;
                debug.unknown8 = b->flags;
                if (i == D_0015f402[0x26]) {
                    D_001549d0[player].type = 0x43;
                    D_001549d0[player].value = 2;
                    play_sound(10, 0x42, -1, -1);
                } else if (i == D_0015f402[0x27]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = D_0015f402[0x27];
                } else if (i == D_0015f402[0x28]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = D_0015f402[0x28];
                } else if (i == D_0015f402[0x29]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = D_0015f402[0x29];
                } else if (i == D_0015f402[0x2a]) {
                    close_query(2);
                    play_sound(10, 0x43, -1, -1);
                    sprintf(D_00155ced, D_00120770, yesno_text[5]);
                    open_yes_no_requester(0x47, D_001581b2[2].value, 0, 0x88, D_001581b2[2].value, 0);
                } else if (i == D_0015f402[0x2b]) {
                    D_001549d0[player].type = 0x68;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(10, 0x99, -1, -1);
                }
            }
            i++;
        }
        break;
    default:
        for (i = 1; b; b = b->next) {
            if (b->flags & 6) {
                debug.unknown4 = i;
                if (i == D_0015f402[0x2c]) {
                    D_001549d0[player].type = 0x43;
                    D_001549d0[player].value = 2;
                    play_sound(10, 0x42, -1, -1);
                } else if (i == D_0015f402[0x2d]) {
                    close_query(2);
                    play_sound(10, 0x43, -1, -1);
                    sprintf(D_00155ced, D_00120773, yesno_text[5]);
                    open_yes_no_requester(0x47, D_001581b2[2].value, 0, 0x88, D_001581b2[2].value, 0);
                } else if (i == D_0015f402[0x2e]) {
                    D_001549d0[player].type = 0x68;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(10, 0x99, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

// FUNCTION: 0x0008704c
void req4_overdraw(Window *win)
{
    u8 a;
    u8 b;
    u8 c;
    u8 n;
    s32 y;
    s32 x;
    Thing *w;
    char buf[100];

    a = 0;
    b = 0;
    c = 0;
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = win->x - D_00188c3c;
    y = win->y - D_00188c40;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    w = &thing[D_001581b2[2].value];
    switch (w->subtype) {
    case 2:
        compute_help_text(0, 0, 0xc);
        set_font_from_cache(1);
        D_001553ee = window.unknown2ec - 1;
        sprintf(buf, D_00120776, w->unknown108);
        LbTextDraw(x + D_0015f410[0x6b].x, y + D_0015f410[0x6b].y, buf);
        draw_staff_handyman_bars(w, x, y, &button_sprite[D_0015f406[3]]);
        if (D_00188c7e) {
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x4e].x, y + D_0015f40c[0x4e].y, D_0015f40c[0x4e].w,
                             D_0015f40c[0x4e].h)) {
                D_001553f0 = 2;
                D_001553c8 = 1;
            }
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x5d].x, y + D_0015f40c[0x5d].y, D_0015f40c[0x5d].w,
                             D_0015f40c[0x5d].h)) {
                D_00155e3a = w - thing;
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = 5;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (D_00188c7f && !D_00155e19) {
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x4e].x, y + D_0015f40c[0x4e].y, D_0015f40c[0x4e].w,
                             D_0015f40c[0x4e].h)) {
                D_001581b2[2].value = find_next_thing(w, 4, w->subtype, -1);
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (D_00188c7f)
            D_00155e19 = 1;
        else
            D_00155e19 = 0;
        break;
    default:
        compute_help_text(0, 0, 10);
        set_font_from_cache(1);
        D_001553ee = window.unknown2ec - 1;
        sprintf(buf, D_00120779, w->unknown108);
        LbTextDraw(x + D_0015f410[0x66].x, y + D_0015f410[0x66].y, buf);
        n = 0;
        n++;
        if (w->unknown124 & 8)
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[10]]);
        n++;
        if (w->unknown124 & 0x10 && !(w->unknown124 & 0x80) && !(w->unknown124 & 8))
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[10]]);
        n++;
        if (w->unknown124 & 0x80)
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[10]]);
        n++;
        if (w->unknown124 & 0x20)
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[11]]);
        n++;
        if (w->unknown124 & 0x40)
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[12]]);
        n++;
        if (w->unknown124 & 0x100)
            reqspr_draw(x + D_0015f40c[n + 0x19].x, y + D_0015f40c[n + 0x19].y, &button_sprite[D_0015f406[13]]);
        if (!(w->subtype == 1 || w->subtype == 4))
            LbDrawBox(x + D_0015f40c[0x40].x, y + D_0015f40c[0x40].y, D_0015f40c[0x40].w, D_0015f40c[0x40].h, 0x7a);
        if (D_00188c7e) {
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x4d].x, y + D_0015f40c[0x4d].y, D_0015f40c[0x4d].w,
                             D_0015f40c[0x4d].h)) {
                D_001553f0 = 2;
                D_001553c8 = 1;
            }
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x5d].x, y + D_0015f40c[0x5d].y, D_0015f40c[0x5d].w,
                             D_0015f40c[0x5d].h)) {
                D_00155e3a = w - thing;
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = 5;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (D_00188c7f && !D_00155e19) {
            if (over_hotspot(mouse_dx, mouse_dy, x + D_0015f40c[0x4d].x, y + D_0015f40c[0x4d].y, D_0015f40c[0x4d].w,
                             D_0015f40c[0x4d].h)) {
                if (w->subtype == 1 || w->subtype == 4)
                    D_001581b2[2].value = find_next_thing(w, 4, 1, 4);
                else
                    D_001581b2[2].value = find_next_thing(w, 4, w->subtype, -1);
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (D_00188c7f)
            D_00155e19 = 1;
        else
            D_00155e19 = 0;
        break;
    }
    draw_face((Picture *)&w->f87, x + D_0015f410[0x65].x, y + D_0015f410[0x65].y);
    draw_gauge(w->unknown81s, x + D_0015f410[0x67].x, y + D_0015f410[0x67].y, D_0015f410[0x69].x,
               &button_sprite[D_0015f406[0]], 100, 0, 100, 1);
    draw_gauge(w->f78, x + D_0015f410[0x68].x, y + D_0015f410[0x68].y, D_0015f410[0x69].x,
               &button_sprite[D_0015f406[1]], 1000, 0, 1000, 0);
    draw_gauge(w->unknown126, x + D_0015f410[0x6a].x, y + D_0015f410[0x6a].y, D_0015f410[0x76].x,
               &button_sprite[D_0015f406[2]], 1000, 30, 970, 1);
    sprintf(buf, D_0012077c, w->f84, staff_names_1[w->f85], staff_names_2[w->f86]);
    LbTextDraw(x + D_0015f410[0x64].x, y + D_0015f410[0x64].y, buf);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0);
}

// FUNCTION: 0x00087b4c
void close_irequesters(void)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    remove_window(open_windows, &D_0015f414[2]);
    remove_window(open_windows, &D_0015f414[0]);
    remove_window(open_windows, &D_0015f414[1]);
    remove_window(open_windows, &D_0015f414[3]);
    while (window.unknown2ec > 1) {
        x = window.zone_x[window.unknown2ec - 1];
        y = window.zone_y[window.unknown2ec - 1];
        w = window.zone_w[window.unknown2ec - 1];
        h = window.zone_h[window.unknown2ec - 1];
        set_all_refresh_zones_in_window(x, y, w, h, 3);
        remove_window_from_list(window.unknown2ec - 1);
    }
    window.unknown2eb = 0;
    cancel_helpmsg();
    D_00155d70 = 0;
}

// FUNCTION: 0x00087c44
void adjust_handyman_values(const u16 id, const u8 button)
{
    Thing *w;
    u8 a;
    u8 b;
    u8 c;

    w = &thing[id];
    if (button == D_0015f402[0x27]) {
        a = w->f9e;
        b = w->f9f;
        c = w->fa0;
    } else if (button == D_0015f402[0x28]) {
        a = w->f9f;
        b = w->f9e;
        c = w->fa0;
    } else if (button == D_0015f402[0x29]) {
        a = w->fa0;
        b = w->f9f;
        c = w->f9e;
    }
    if (a < 0x5e) {
        a += 6;
        while (a + b + c > 100) {
            if (b > c)
                b--;
            else
                c--;
        }
        if (w->owner == player)
            play_sound(10, 0x43, -1, -1);
    } else {
        a = 100;
        b = 0;
        c = 0;
    }
    if (button == D_0015f402[0x27]) {
        w->f9e = a;
        w->f9f = b;
        w->fa0 = c;
    } else if (button == D_0015f402[0x28]) {
        w->f9f = a;
        w->f9e = b;
        w->fa0 = c;
    } else if (button == D_0015f402[0x29]) {
        w->fa0 = a;
        w->f9f = b;
        w->f9e = c;
    }
    w->f98 = 0;
    w->f9a = 0;
    w->f9c = 0;
}

// FUNCTION: 0x00087e7c
void draw_staff_handyman_bars(const Thing *const w, const u16 x, const u16 y, TbSprite *spr)
{
    draw_gauge(w->f9e, x + D_0015f410[0x72].x, y + D_0015f410[0x72].y, D_0015f410[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(w->f9f, x + D_0015f410[0x73].x, y + D_0015f410[0x72].y, D_0015f410[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(w->fa0, x + D_0015f410[0x74].x, y + D_0015f410[0x72].y, D_0015f410[0xaa].x, spr, 100, 0, 100, 1);
}

// FUNCTION: 0x00087fb8
void setup_req4_references(void)
{
    u8 i;
    char buf[4];

    sprintf(buf, D_00120785);
    for (i = 0; ref_general_text[i][0] != buf[0]; i++) {}
    D_00156429 = i;
    for (i = 0; ref_good_text[i][0] != buf[0]; i++) {}
    D_0015642a = i;
    for (i = 0; ref_bad_text[i][0] != buf[0]; i++) {}
    D_0015642b = i;
}

// FUNCTION: 0x00088068
void req4_help_if_doctor(void)
{
    Thing *w;

    w = &thing[D_001581b2[2].value];
    if (w > thing && w->unknown124 & 0x10)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x000880e4
void req4_help_if_surgeon(void)
{
    Thing *w;

    w = &thing[D_001581b2[2].value];
    if (w > thing && w->unknown124 & 0x20)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x00088160
void req4_help_if_shrink(void)
{
    Thing *w;

    w = &thing[D_001581b2[2].value];
    if (w > thing && w->unknown124 & 0x40)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// FUNCTION: 0x000881dc
void req4_help_if_research(void)
{
    Thing *w;

    w = &thing[D_001581b2[2].value];
    if (w > thing && w->unknown124 & 0x100)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}
