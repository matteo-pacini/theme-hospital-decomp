#include "hosp.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

struct Req2Layout {
    u8 unknown0[0x170];
    s32 win_x;
    s32 win_y;
    s32 win_w;
    s32 win_h;
    s32 msg_x;
    s32 msg_y;
    s32 msg_w;
    s32 msg_h;
    u8 unknown190[0x220];
    s32 cover0_x;
    s32 cover0_y;
    s32 cover0_w;
    s32 cover0_h;
    s32 cover1_x;
    s32 cover1_y;
    s32 cover1_w;
    s32 cover1_h;
    s32 box_x;
    s32 box_y;
    s32 box_w;
    s32 box_h;
    u8 unknown3e0[0x30];
    s32 text_x;
    s32 text_y;
    s32 text_w;
    s32 text_h;
    u8 unknown420[0x140];
    u16 gauge_x;
    u8 unknown562[2];
    u16 gauge_y;
    u8 unknown566[2];
    u16 gauge_w;
    u8 unknown56a[6];
    s32 cover2_x;
    s32 cover2_y;
    s32 cover2_w;
    s32 cover2_h;
    s32 cover3_x;
    s32 cover3_y;
    s32 cover3_w;
    s32 cover3_h;
};

extern "C" {
extern Event D_001549d0[];
extern s32 D_001290cc;
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 *D_0015479c;
void fn_0001bee4(void);
}

u32 safe_normal_rand(u32 a, u32 b);
void draw_face(Picture *p, const u16 x, const u16 y);
void draw_req2_coverup(const u16 x, const u16 y);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long mid, const u8 flag);

extern "C" {
}

// FUNCTION: 0x000697e0
void req2(Window *w)
{
    u16 i;
    u16 id;
    u16 old;
    WindowButton *b;

    for (b = w->buttons, i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = i;
            if (i == resvars.fe[2]) {
                D_001549d0[player].type = 0x2f;
                D_001549d0[player].value = 1;
                if (crequester.unknown4 != 1 || crequester.unknown0 != 2)
                    play_sound(10, 0x43, -1, -1);
            } else if (i == resvars.fe[3]) {
                D_001549d0[player].type = 0x2f;
                D_001549d0[player].value = 0;
                if (crequester.unknown4 != 0 || crequester.unknown0 != 2)
                    play_sound(10, 0x43, -1, -1);
            } else if (i == resvars.fe[5]) {
                D_001549d0[player].type = 0x2f;
                D_001549d0[player].value = 3;
                if (crequester.unknown4 != 3 || crequester.unknown0 != 2)
                    play_sound(10, 0x43, -1, -1);
            } else if (i == resvars.fe[4]) {
                D_001549d0[player].type = 0x2f;
                D_001549d0[player].value = 2;
                if (crequester.unknown4 != 2 || crequester.unknown0 != 2)
                    play_sound(10, 0x43, -1, -1);
            } else if (i == resvars.fe[8]) {
                if (crequester.unknown6) {
                    if (crequester.unknown0 == 2) {
                        if (allhosps[player].f35 >= thing[crequester.unknown6].unknown108 && allhosps[player].f35 > 0) {
                            D_001549d0[player].type = 0x30;
                            D_001549d0[player].value = crequester.unknown4;
                            D_001549d0[player].arg = crequester.unknown6;
                            play_sound(10, 0x41, -1, -1);
                        } else {
                            play_sound(10, 8, -1, -1);
                        }
                    } else {
                        play_sound(10, 8, -1, -1);
                    }
                } else {
                    play_sound(10, 8, -1, -1);
                }
            } else if (i == resvars.fe[6]) {
                if (crequester.unknown6) {
                    old = crequester.unknown6;
                    id = req2_alter_staff(0, crequester.unknown6, crequester.unknown4);
                    if (id) {
                        if (id == old) {
                            play_sound(10, 8, -1, -1);
                        } else {
                            D_001549d0[player].type = 0x32;
                            D_001549d0[player].value = 0;
                            play_sound(10, 0xa8, -1, -1);
                        }
                    } else {
                        play_sound(10, 8, -1, -1);
                    }
                } else {
                    play_sound(10, 8, -1, -1);
                }
            } else if (i == resvars.fe[7]) {
                if (crequester.unknown6) {
                    old = crequester.unknown6;
                    id = req2_alter_staff(1, crequester.unknown6, crequester.unknown4);
                    if (id) {
                        if (id == old) {
                            play_sound(10, 8, -1, -1);
                        } else {
                            D_001549d0[player].type = 0x32;
                            D_001549d0[player].value = 1;
                            play_sound(10, 0xa8, -1, -1);
                        }
                    } else {
                        play_sound(10, 8, -1, -1);
                    }
                } else {
                    play_sound(10, 8, -1, -1);
                }
            } else if (i == resvars.fe[9]) {
                window.unknown2eb = 0;
                setup_abort_all_requesters();
                play_sound(10, 0x42, -1, -1);
            }
        }
        i++;
    }
}

// FUNCTION: 0x00069d34
void req2_overdraw(Window *w)
{
    s32 x0;
    WindowButton *b;
    Thing *t;
    s32 y0;
    s32 spr;
    u8 i;
    u16 counts[4];
    char buf[200];

    spr = 0;
    if (resvars.f1 == SCREEN_MODE_13 && loaded_requester != 0xb)
        return;
    compute_help_text(w->x, w->y, 4);
    if (crequester.unknown6 > end_things - thing)
        crequester.unknown6 = 0;
    if (crequester.unknown6) {
        t = &thing[crequester.unknown6];
        if (t->kind != 0x2e)
            crequester.unknown6 = req2_alter_staff(1, 0, crequester.unknown4);
    }
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x0 = w->x - lbDisplay.graphics_window_x;
    y0 = w->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (crequester.unknown6 && crequester.unknown0 == 2) {
        for (b = w->buttons, i = 1; b; b = b->next, i++) {
            if (i == resvars.fe[2] && crequester.unknown4 == 1) {
                spr = resvars.f12[0x35];
                break;
            }
            if (i == resvars.fe[3] && crequester.unknown4 == 0) {
                spr = 0x10b;
                break;
            }
            if (i == resvars.fe[5] && crequester.unknown4 == 3) {
                spr = resvars.f12[0x31];
                break;
            }
            if (i == resvars.fe[4] && crequester.unknown4 == 2) {
                spr = 0x10d;
                break;
            }
        }
    }
    if (spr)
        reqspr_draw(b->x + x0, b->y + y0, &button_sprite[spr]);
    for (i = 0; i < 4; i++)
        counts[i] = 0;
    for (t = &thing[1]; t < end_things; t++) {
        if (t->renderer == 4) {
            if (t->kind == 0x2e) {
                if (t->subtype == 1)
                    counts[0]++;
                else if (t->subtype == 0)
                    counts[1]++;
                else if (t->subtype == 2)
                    counts[2]++;
                else if (t->subtype == 3)
                    counts[3]++;
            }
        }
    }
    set_font_from_cache(0xd);
    for (i = 0; i < 4; i++) {
        sprintf(buf, "%d", counts[i]);
        LbTextDraw(x0 + resvars.f1c[i + 78].x, y0 + resvars.f1c[i + 78].y, buf);
    }
    if (advisor.f11f & 1) {
        i = 1;
        othervar.f9bc++;
        if (othervar.f9bc % 10 == 0) {
            switch (advisor.f120) {
            case 3:
                for (b = w->buttons; b; b = b->next) {
                    if (i == resvars.fe[5])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + b->x, (s16)y0 + b->y, &button_sprite[resvars.f12[0x31]]);
                break;
            case 5:
                for (b = w->buttons; b; b = b->next) {
                    if (i == resvars.fe[8])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + b->x, (s16)y0 + b->y, &button_sprite[resvars.f12[0x32]]);
                break;
            case 6:
                for (b = w->buttons; b; b = b->next) {
                    if (i == resvars.fe[6])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + b->x, (s16)y0 + b->y, &button_sprite[resvars.f12[0x33]]);
                break;
            case 7:
                for (b = w->buttons; b; b = b->next) {
                    if (i == resvars.fe[7])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + b->x, (s16)y0 + b->y, &button_sprite[resvars.f12[0x34]]);
                break;
            case 4:
                for (b = w->buttons; b; b = b->next) {
                    if (i == resvars.fe[2])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + b->x, (s16)y0 + b->y, &button_sprite[resvars.f12[0x35]]);
                break;
            }
            othervar.f9bc = 0;
        }
    }
    set_font_from_cache(1);
    switch (crequester.unknown0) {
    case 1:
        LbDrawBox(x0 + resvars.f18_Req2Layoutp->box_x, y0 + resvars.f18_Req2Layoutp->box_y, resvars.f18_Req2Layoutp->box_w, resvars.f18_Req2Layoutp->box_h, 0x77);
        break;
    case 2:
        if (crequester.unknown6) {
            t = &thing[crequester.unknown6];
            sprintf(buf, "%c. %s%s", t->f84, staff_names_1[t->f85], staff_names_2[t->f86]);
            LbTextDraw(x0 + resvars.f1c[82].x, y0 + resvars.f1c[82].y, buf);
            sprintf(buf, "$%d", t->unknown108);
            LbTextDraw(x0 + resvars.f1c[84].x, y0 + resvars.f1c[84].y, buf);
            draw_face((Picture *)&t->f87, x0 + resvars.f1c[83].x, y0 + resvars.f1c[83].y);
            i = 0;
            if ((t->unknown124 & 8) && !(t->unknown124 & 0x80))
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if ((t->unknown124 & 0x10) && !(t->unknown124 & 0x80) && !(t->unknown124 & 8))
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if (t->unknown124 & 0x80)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if (t->unknown124 & 0x20)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[7]]);
            i++;
            if (t->unknown124 & 0x40)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[8]]);
            i++;
            if (t->unknown124 & 0x100)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[9]]);
            if (t->subtype != 1)
                draw_req2_coverup(x0, y0);
            draw_gauge(t->unknown126, x0 + resvars.f18_Req2Layoutp->gauge_x + i, y0 + resvars.f18_Req2Layoutp->gauge_y, resvars.f18_Req2Layoutp->gauge_w,
                       &button_sprite[resvars.f12[2]], 1000, 30, 970, 1);
            lbDisplay.draw_flags = 0x20;
            LbTextSetWindow(x0 + resvars.f18_Req2Layoutp->text_x, y0 + resvars.f18_Req2Layoutp->text_y, resvars.f18_Req2Layoutp->text_w, resvars.f18_Req2Layoutp->text_h);
            if (resvars.f1 == SCREEN_MODE_13) {
                if (t->unknown126 > 500)
                    sprintf(buf, "%s%s", ref_general_text[t->f8a], ref_good_text[t->f8b]);
                else
                    sprintf(buf, "%s%s", ref_general_text[t->f8a], ref_bad_text[t->f8c]);
            } else if (resvars.f1 == SCREEN_MODE_1) {
                if (t->unknown126 > 500)
                    sprintf(buf, "%s", ref_good_text[t->f8b]);
                else
                    sprintf(buf, "%s", ref_bad_text[t->f8c]);
            }
            LbTextDraw(0, 0, buf);
            lbDisplay.draw_flags = 0;
        } else {
            LbDrawBox(x0 + resvars.f18_Req2Layoutp->box_x, y0 + resvars.f18_Req2Layoutp->box_y, resvars.f18_Req2Layoutp->box_w, resvars.f18_Req2Layoutp->box_h, 0x77);
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(0);
        break;
    default:
        LbDrawBox(x0 + resvars.f18_Req2Layoutp->box_x, y0 + resvars.f18_Req2Layoutp->box_y, resvars.f18_Req2Layoutp->box_w, resvars.f18_Req2Layoutp->box_h, 0x77);
        break;
    }
}

// FUNCTION: 0x0006aab0
void req2_reset(void)
{
    crequester.unknown2 = 0xb;
    crequester.unknown0 = 1;
    crequester.unknown4 = 0;
}

// FUNCTION: 0x0006aae0
u16 req2_alter_staff(u8 dir, u16 cur, u16 kind)
{
    u8 found;
    Thing *w;

    found = 0;
    switch (dir) {
    case 1:
        if (cur == 0) {
            for (w = &thing[1]; w < end_things; w++) {
                if (w->renderer == 4) {
                    if (w->subtype == kind) {
                        if (w->kind == 0x2e)
                            return w - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (w = &thing[1]; w < end_things; w++) {
                if ((u16)(w - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (w++; w < end_things; w++) {
                    if (w->renderer == 4) {
                        if (w->subtype == kind) {
                            if (w->kind == 0x2e)
                                return w - thing;
                        }
                    }
                }
                return cur;
            }
        }
        break;
    case 0:
        if (cur == 0) {
            for (w = &thing[1]; w < end_things; w++) {
                if (w->renderer == 4) {
                    if (w->subtype == kind) {
                        if (w->kind == 0x2e)
                            return w - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (w = &thing[1]; w < end_things; w++) {
                if ((u16)(w - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (w--; &thing[1] <= w; w--) {
                    if (w->renderer == 4) {
                        if (w->subtype == kind) {
                            if (w->kind == 0x2e)
                                return w - thing;
                        }
                    }
                }
                return cur;
            }
        }
        break;
    }
    return 0;
}

// FUNCTION: 0x0006ae00
void draw_face(Picture *p, const u16 x, const u16 y)
{
    u8 *src;
    u8 *dst;
    s32 w;
    s32 n;

    fn_0001bee4();
    if (p->head != D_001290d0) {
        LbFileSeek(D_001290cc, p->head * (resvars.f60 * resvars.f64), 0);
        LbFileRead(D_001290cc, D_0015479c, resvars.f60 * resvars.f64);
        D_001290d0 = p->head;
    }
    if (p->body != D_001290d1) {
        LbFileSeek(D_001290cc, resvars.f60 * resvars.f64 * 18 + p->body * (resvars.f60 * resvars.f68), 0);
        LbFileRead(D_001290cc, D_0015479c + resvars.f60 * resvars.f64, resvars.f60 * resvars.f68);
        D_001290d1 = p->body;
    }
    if (p->legs != D_001290d2) {
        LbFileSeek(D_001290cc, resvars.f60 * resvars.f64 * 18 + resvars.f60 * resvars.f68 * 18 + (u8)p->legs * (resvars.f60 * resvars.f6c), 0);
        LbFileRead(D_001290cc, resvars.f60 * resvars.f64 + resvars.f60 * resvars.f68 + D_0015479c, resvars.f60 * resvars.f6c);
        D_001290d2 = p->legs;
    }
    src = D_0015479c;
    dst = x + resvars.f58 + lbDisplay.unknown4 + lbDisplay.graphics_screen_width * (y - (resvars.f64 + resvars.f68 + resvars.f6c) - resvars.f5c);
    w = resvars.f60;
    n = resvars.f64 + resvars.f68 + resvars.f6c;
    while (--n >= 0) {
        fn_000c21e6(dst, src, w);
        src += w;
        dst += lbDisplay.graphics_screen_width;
    }
    lbDisplay.draw_flags = 0;
    LbDrawBox(x, y - (resvars.f64 + resvars.f68 + resvars.f6c) - resvars.f5c * 2, resvars.f58 * 2 + resvars.f60, resvars.f5c, 0x9c);
    LbDrawBox(x, y - (resvars.f64 + resvars.f68 + resvars.f6c) - resvars.f5c * 2, resvars.f58, resvars.f64 + resvars.f68 + resvars.f6c + resvars.f5c * 2, 0x9c);
    LbDrawBox(x, y - resvars.f5c, resvars.f58 * 2 + resvars.f60, resvars.f5c, 0x9c);
    LbDrawBox(x + resvars.f60 + resvars.f58, y - (resvars.f64 + resvars.f68 + resvars.f6c) - resvars.f5c * 2, resvars.f58, resvars.f64 + resvars.f68 + resvars.f6c + resvars.f5c * 2, 0x9c);
}

// FUNCTION: 0x0006b174
void req2_mark_employing(const u16 id)
{
    Thing *w;

    w = &thing[id];
    w->kind = 0x44;
    adcounts.f4 += w->unknown81;
}

// FUNCTION: 0x0006b1bc
void req2_exit_employing(const u16 id)
{
    Thing *w;

    w = &thing[id];
    w->kind = 0x2e;
}

// FUNCTION: 0x0006b1f4
void req12(Window *w)
{
    u16 kind;
    u16 i;
    u16 sub;
    WindowButton *b;

    b = w->buttons;
    kind = crequester.unknown8;
    sub = crequester.unknown3;
    for (i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = i;
            if (i == resvars.fe[0x15]) {
                play_sound(10, 0xb8, -1, -1);
                switch (crequester.unknown8) {
                case 2:
                    D_001549d0[player].type = 0x6b;
                    D_001549d0[player].value = crequester.unknown4;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 3:
                case 5:
                case 8:
                    D_001549d0[player].type = 0x6d;
                    D_001549d0[player].value = crequester.unknown4;
                    D_001549d0[player].arg = crequester.unknown6;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 4:
                case 6:
                case 7:
                    D_001549d0[player].type = 0x6c;
                    D_001549d0[player].value = crequester.unknown4;
                    D_001549d0[player].arg = crequester.unknown6;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 9:
                    D_001549d0[player].type = 0x6c;
                    D_001549d0[player].value = crequester.unknown4;
                    D_001549d0[player].arg = thing[crequester.unknown4].unknown108 * 2;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                }
            } else if (i == resvars.fe[0x16]) {
                play_sound(10, 0xaa, -1, -1);
                req12_clicked_sack();
                lbDisplay.left_button = lbDisplay.rleft_button = 0;
            }
            break;
        }
        i++;
    }
}

extern "C" {
extern char **D_001602a0;
}

void draw_window_not_using_bscreen(const u8 idx);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);

// FUNCTION: 0x0006b464
void req12_overdraw(Window *w)
{
    u8 i;
    u16 old_h;
    u16 old_w;
    u16 sub;
    u16 old_x;
    u16 old_y;
    Thing *t;
    s32 y0;
    s32 x0;
    char buf[200];

    sub = crequester.unknown3;
    othervar.f10 = 1;
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x0 = w->x - lbDisplay.graphics_window_x;
    y0 = w->y - lbDisplay.graphics_window_y;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (!crequester.unknown9) {
        close_irequesters();
        open_new_window(0, 0, 0, x0 + resvars.f18_Req2Layoutp->win_x, y0 + resvars.f18_Req2Layoutp->win_y,
                        x0 + resvars.f18_Req2Layoutp->win_x + resvars.f18_Req2Layoutp->win_w, y0 + resvars.f18_Req2Layoutp->win_y + resvars.f18_Req2Layoutp->win_h);
        window.unknown2eb = 0;
        crequester.unknown9 = 1;
    }
    if (crequester.unknown4) {
        t = &thing[crequester.unknown4];
        sprintf(buf, "%c. %s%s", t->f84, staff_names_1[t->f85], staff_names_2[t->f86]);
        LbTextDraw(x0 + resvars.f1c[2].x, y0 + resvars.f1c[2].y, buf);
        sprintf(buf, "$%d", t->unknown108);
        LbTextDraw(x0 + resvars.f1c[4].x, y0 + resvars.f1c[4].y, buf);
        draw_face((Picture *)&t->f87, x0 + resvars.f1c[3].x, y0 + resvars.f1c[3].y);
        i = 0;
        if (t->unknown124 & 8)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if ((t->unknown124 & 0x10) && !(t->unknown124 & 0x80) && !(t->unknown124 & 8))
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if (t->unknown124 & 0x80)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if (t->unknown124 & 0x20)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[7]]);
        i++;
        if (t->unknown124 & 0x40)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[8]]);
        i++;
        if (t->unknown124 & 0x100)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[9]]);
        draw_gauge(t->unknown126, x0 + resvars.f1c[0].x, y0 + resvars.f1c[0].y, resvars.f18_Req2Layoutp->gauge_w,
                   &button_sprite[resvars.f12[3]], 1000, 30, 970, 1);
        if (t->subtype != 1 && t->subtype != 4) {
            LbDrawBox(x0 + resvars.f18_Req2Layoutp->cover2_x, y0 + resvars.f18_Req2Layoutp->cover2_y, resvars.f18_Req2Layoutp->cover2_w,
                      resvars.f18_Req2Layoutp->cover2_h, 0x77);
            LbDrawBox(x0 + resvars.f18_Req2Layoutp->cover3_x, y0 + resvars.f18_Req2Layoutp->cover3_y, resvars.f18_Req2Layoutp->cover3_w,
                      resvars.f18_Req2Layoutp->cover3_h, 0x77);
        }
        switch (crequester.unknown8) {
        case 2:
            sprintf(buf, D_001602a0[crequester.unknown8]);
            break;
        case 3:
        case 5:
        case 7:
        case 8:
            sprintf(buf, D_001602a0[crequester.unknown8], crequester.unknown6);
            break;
        case 4:
        case 6:
            sprintf(buf, D_001602a0[crequester.unknown8], crequester.unknown6,
                    crequester.unknown6 + t->unknown108);
            break;
        case 9:
            sprintf(buf, D_001602a0[crequester.unknown8], t->unknown108 * 2, &allhosps[crequester.unknown6]);
            break;
        }
        lbDisplay.draw_flags = 0x20;
        LbTextSetWindow(x0 + resvars.f18_Req2Layoutp->msg_x, y0 + resvars.f18_Req2Layoutp->msg_y, resvars.f18_Req2Layoutp->msg_w, resvars.f18_Req2Layoutp->msg_h);
        LbTextDraw(0, 0, buf);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        give_absolute_xy_of_cell_at_column_row(t->x.tile, t->y.tile);
        window.f0[window.unknown2ec - 1] = (resvars.f0 == SCREEN_MODE_13 ? ((t->x.frac - t->y.frac) >> 3) + 0x20
                                                                        : ((t->x.frac - t->y.frac) >> 4) + 0x10)
                                           + (info.unknown54 - resvars.f1c[171].x);
        window.fc[window.unknown2ec - 1] = info.unknown58 - resvars.f1c[171].y
                                           + (resvars.f0 == SCREEN_MODE_13 ? (t->y.frac + t->x.frac) >> 4
                                                                           : (t->y.frac + t->x.frac) >> 5);
        old_x = window.unknown2c3;
        old_y = window.unknown2c7;
        old_w = window.unknown2cb;
        old_h = window.unknown2cf;
        window.unknown2c3 = resvars.f18_Req2Layoutp->win_x;
        window.unknown2c7 = resvars.f18_Req2Layoutp->win_y;
        window.unknown2cb = resvars.f18_Req2Layoutp->win_x + resvars.f18_Req2Layoutp->win_w;
        window.unknown2cf = resvars.f18_Req2Layoutp->win_y + resvars.f18_Req2Layoutp->win_h;
        draw_window_not_using_bscreen(window.unknown2ec - 1);
        window.unknown2c3 = old_x;
        window.unknown2c7 = old_y;
        window.unknown2cb = old_w;
        window.unknown2cf = old_h;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
    compute_help_text(0, 0, 0x1a);
}

// FUNCTION: 0x0006bda8
void open_staff_resign_requester(Thing *t)
{
    u32 i;
    s32 d;
    u32 n;

    n = 1;
    if (statvar.unknownfd > 1)
        n = 4;
    play_sound(10, 0x9a, -1, -1);
    display_mode = 0xb;
    close_irequesters();
    crequester.unknown2 = 0xc;
    fn_0001bc94(crequester.unknown2, 1);
    crequester.unknown4 = t - thing;
    crequester.unknown9 = 0;
    if (t->f93) {
        crequester.unknown6 = 0;
        for (i = 0; i < n; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            if (t->f93 == 1 << i) {
                crequester.unknown6 = i;
                break;
            }
        }
        crequester.unknown8 = 9;
    } else if (t->f8f > 5) {
        crequester.unknown6 = 0;
        crequester.unknown8 = 2;
    } else if (t->f70 > t->unknown108) {
        d = t->f70 - t->unknown108;
        crequester.unknown6 = t->f70 * 10 / 100 + d;
        crequester.unknown8 = 7;
    } else if (t->f78 > gbv.fea && saferand() % 2) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.unknown8 = 3;
        else
            crequester.unknown8 = 4;
        crequester.unknown6 = t->unknown108 * i / 100;
    } else if (t->unknown81s > gbv.fea) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.unknown8 = 5;
        else
            crequester.unknown8 = 6;
        crequester.unknown6 = t->unknown108 * i / 100;
    } else {
        i = safe_normal_rand(10, 30);
        crequester.unknown8 = 8;
        crequester.unknown6 = t->unknown108 * i / 100;
    }
    crequester.unknown0 = 0;
}

// FUNCTION: 0x0006c05c
void draw_req2_coverup(const u16 x, const u16 y)
{
    LbDrawBox(x + resvars.f18_Req2Layoutp->cover0_x, y + resvars.f18_Req2Layoutp->cover0_y, resvars.f18_Req2Layoutp->cover0_w, resvars.f18_Req2Layoutp->cover0_h, 0x77);
    LbDrawBox(x + resvars.f18_Req2Layoutp->cover1_x, y + resvars.f18_Req2Layoutp->cover1_y, resvars.f18_Req2Layoutp->cover1_w, resvars.f18_Req2Layoutp->cover1_h, 0x77);
}

// FUNCTION: 0x0006c108
void req2_help_if_doctor(void)
{
    Thing *w;

    w = &thing[crequester.unknown6];
    if (w > thing && (w->unknown124 & 0x10))
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c184
void req2_help_if_surgeon(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown6];
    mask = 0x30;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c214
void req2_help_if_psycho(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown6];
    mask = 0x50;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c2a4
void req2_help_if_researcher(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown6];
    mask = 0x110;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c334
void req2_help_if_selected(void)
{
    if (crequester.unknown6)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c38c
void req12_help_if_doctor(void)
{
    Thing *w;

    w = &thing[crequester.unknown4];
    if (w > thing && (w->unknown124 & 0x10))
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c408
void req12_help_if_surgeon(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown4];
    mask = 0x30;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c498
void req12_help_if_shrink(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown4];
    mask = 0x50;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c528
void req12_help_if_research(void)
{
    Thing *w;
    u32 mask;

    w = &thing[crequester.unknown4];
    mask = 0x110;
    if (w > thing && (w->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c5b8
void req12_clicked_sack(void)
{
    if (crequester.unknown8 == 9) {
        D_001549d0[player].type = 0x56;
        D_001549d0[player].value = crequester.unknown4;
        D_001549d0[player].arg = crequester.unknown6;
    } else {
        D_001549d0[player].type = 0x6b;
        D_001549d0[player].value = crequester.unknown4;
    }
}
