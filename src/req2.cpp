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
void req2(Window *win)
{
    u16 i;
    u16 id;
    u16 old;
    WindowButton *button;

    for (button = win->buttons, i = 1; button; button = button->next) {
        if (button->flags & 6) {
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
void req2_overdraw(Window *win)
{
    s32 x0;
    WindowButton *button;
    Thing *candidate;
    s32 y0;
    s32 spr;
    u8 i;
    u16 counts[4];
    char buf[200];

    spr = 0;
    if (resvars.f1 == SCREEN_MODE_13 && loaded_requester != 0xb)
        return;
    compute_help_text(win->x, win->y, 4);
    if (crequester.unknown6 > end_things - thing)
        crequester.unknown6 = 0;
    if (crequester.unknown6) {
        candidate = &thing[crequester.unknown6];
        if (candidate->kind != 0x2e)
            crequester.unknown6 = req2_alter_staff(1, 0, crequester.unknown4);
    }
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x0 = win->x - lbDisplay.graphics_window_x;
    y0 = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (crequester.unknown6 && crequester.unknown0 == 2) {
        for (button = win->buttons, i = 1; button; button = button->next, i++) {
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
        reqspr_draw(button->x + x0, button->y + y0, &button_sprite[spr]);
    for (i = 0; i < 4; i++)
        counts[i] = 0;
    for (candidate = &thing[1]; candidate < end_things; candidate++) {
        if (candidate->renderer == 4) {
            if (candidate->kind == 0x2e) {
                if (candidate->subtype == 1)
                    counts[0]++;
                else if (candidate->subtype == 0)
                    counts[1]++;
                else if (candidate->subtype == 2)
                    counts[2]++;
                else if (candidate->subtype == 3)
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
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[5])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.f12[0x31]]);
                break;
            case 5:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[8])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.f12[0x32]]);
                break;
            case 6:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[6])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.f12[0x33]]);
                break;
            case 7:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[7])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.f12[0x34]]);
                break;
            case 4:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[2])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.f12[0x35]]);
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
            candidate = &thing[crequester.unknown6];
            sprintf(buf, "%c. %s%s", candidate->f84, staff_names_1[candidate->f85], staff_names_2[candidate->f86]);
            LbTextDraw(x0 + resvars.f1c[82].x, y0 + resvars.f1c[82].y, buf);
            sprintf(buf, "$%d", candidate->unknown108);
            LbTextDraw(x0 + resvars.f1c[84].x, y0 + resvars.f1c[84].y, buf);
            draw_face((Picture *)&candidate->f87, x0 + resvars.f1c[83].x, y0 + resvars.f1c[83].y);
            i = 0;
            if ((candidate->unknown124 & 8) && !(candidate->unknown124 & 0x80))
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if ((candidate->unknown124 & 0x10) && !(candidate->unknown124 & 0x80) && !(candidate->unknown124 & 8))
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if (candidate->unknown124 & 0x80)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[6]]);
            i++;
            if (candidate->unknown124 & 0x20)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[7]]);
            i++;
            if (candidate->unknown124 & 0x40)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[8]]);
            i++;
            if (candidate->unknown124 & 0x100)
                reqspr_draw(x0 + resvars.f1c[i + 85].x, y0 + resvars.f1c[i + 85].y, &button_sprite[resvars.f12[9]]);
            if (candidate->subtype != 1)
                draw_req2_coverup(x0, y0);
            draw_gauge(candidate->unknown126, x0 + resvars.f18_Req2Layoutp->gauge_x + i, y0 + resvars.f18_Req2Layoutp->gauge_y, resvars.f18_Req2Layoutp->gauge_w,
                       &button_sprite[resvars.f12[2]], 1000, 30, 970, 1);
            lbDisplay.draw_flags = 0x20;
            LbTextSetWindow(x0 + resvars.f18_Req2Layoutp->text_x, y0 + resvars.f18_Req2Layoutp->text_y, resvars.f18_Req2Layoutp->text_w, resvars.f18_Req2Layoutp->text_h);
            if (resvars.f1 == SCREEN_MODE_13) {
                if (candidate->unknown126 > 500)
                    sprintf(buf, "%s%s", ref_general_text[candidate->f8a], ref_good_text[candidate->f8b]);
                else
                    sprintf(buf, "%s%s", ref_general_text[candidate->f8a], ref_bad_text[candidate->f8c]);
            } else if (resvars.f1 == SCREEN_MODE_1) {
                if (candidate->unknown126 > 500)
                    sprintf(buf, "%s", ref_good_text[candidate->f8b]);
                else
                    sprintf(buf, "%s", ref_bad_text[candidate->f8c]);
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
    Thing *candidate;

    found = 0;
    switch (dir) {
    case 1:
        if (cur == 0) {
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if (candidate->renderer == 4) {
                    if (candidate->subtype == kind) {
                        if (candidate->kind == 0x2e)
                            return candidate - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if ((u16)(candidate - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (candidate++; candidate < end_things; candidate++) {
                    if (candidate->renderer == 4) {
                        if (candidate->subtype == kind) {
                            if (candidate->kind == 0x2e)
                                return candidate - thing;
                        }
                    }
                }
                return cur;
            }
        }
        break;
    case 0:
        if (cur == 0) {
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if (candidate->renderer == 4) {
                    if (candidate->subtype == kind) {
                        if (candidate->kind == 0x2e)
                            return candidate - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if ((u16)(candidate - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (candidate--; &thing[1] <= candidate; candidate--) {
                    if (candidate->renderer == 4) {
                        if (candidate->subtype == kind) {
                            if (candidate->kind == 0x2e)
                                return candidate - thing;
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
void draw_face(Picture *face, const u16 x, const u16 y)
{
    u8 *src;
    u8 *dst;
    s32 width;
    s32 rows;

    fn_0001bee4();
    if (face->head != D_001290d0) {
        LbFileSeek(face_file, face->head * (resvars.f60 * resvars.f64), 0);
        LbFileRead(face_file, D_0015479c, resvars.f60 * resvars.f64);
        D_001290d0 = face->head;
    }
    if (face->body != D_001290d1) {
        LbFileSeek(face_file, resvars.f60 * resvars.f64 * 18 + face->body * (resvars.f60 * resvars.f68), 0);
        LbFileRead(face_file, D_0015479c + resvars.f60 * resvars.f64, resvars.f60 * resvars.f68);
        D_001290d1 = face->body;
    }
    if (face->legs != D_001290d2) {
        LbFileSeek(face_file, resvars.f60 * resvars.f64 * 18 + resvars.f60 * resvars.f68 * 18 + (u8)face->legs * (resvars.f60 * resvars.f6c), 0);
        LbFileRead(face_file, resvars.f60 * resvars.f64 + resvars.f60 * resvars.f68 + D_0015479c, resvars.f60 * resvars.f6c);
        D_001290d2 = face->legs;
    }
    src = D_0015479c;
    dst = x + resvars.f58 + lbDisplay.unknown4 + lbDisplay.graphics_screen_width * (y - (resvars.f64 + resvars.f68 + resvars.f6c) - resvars.f5c);
    width = resvars.f60;
    rows = resvars.f64 + resvars.f68 + resvars.f6c;
    while (--rows >= 0) {
        fn_000c21e6(dst, src, width);
        src += width;
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
    Thing *candidate;

    candidate = &thing[id];
    candidate->kind = 0x44;
    adcounts.f4 += candidate->unknown81;
}

// FUNCTION: 0x0006b1bc
void req2_exit_employing(const u16 id)
{
    Thing *candidate;

    candidate = &thing[id];
    candidate->kind = 0x2e;
}

// FUNCTION: 0x0006b1f4
void req12(Window *win)
{
    u16 kind;
    u16 i;
    u16 sub;
    WindowButton *button;

    button = win->buttons;
    kind = crequester.unknown8;
    sub = crequester.unknown3;
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
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
void req12_overdraw(Window *win)
{
    u8 i;
    u16 old_h;
    u16 old_w;
    u16 sub;
    u16 old_x;
    u16 old_y;
    Thing *staff;
    s32 y0;
    s32 x0;
    char buf[200];

    sub = crequester.unknown3;
    othervar.f10 = 1;
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x0 = win->x - lbDisplay.graphics_window_x;
    y0 = win->y - lbDisplay.graphics_window_y;
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
        staff = &thing[crequester.unknown4];
        sprintf(buf, "%c. %s%s", staff->f84, staff_names_1[staff->f85], staff_names_2[staff->f86]);
        LbTextDraw(x0 + resvars.f1c[2].x, y0 + resvars.f1c[2].y, buf);
        sprintf(buf, "$%d", staff->unknown108);
        LbTextDraw(x0 + resvars.f1c[4].x, y0 + resvars.f1c[4].y, buf);
        draw_face((Picture *)&staff->f87, x0 + resvars.f1c[3].x, y0 + resvars.f1c[3].y);
        i = 0;
        if (staff->unknown124 & 8)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if ((staff->unknown124 & 0x10) && !(staff->unknown124 & 0x80) && !(staff->unknown124 & 8))
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if (staff->unknown124 & 0x80)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[6]]);
        i++;
        if (staff->unknown124 & 0x20)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[7]]);
        i++;
        if (staff->unknown124 & 0x40)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[8]]);
        i++;
        if (staff->unknown124 & 0x100)
            reqspr_draw(x0 + resvars.f1c[i + 91].x, y0 + resvars.f1c[i + 91].y, &button_sprite[resvars.f12[9]]);
        draw_gauge(staff->unknown126, x0 + resvars.f1c[0].x, y0 + resvars.f1c[0].y, resvars.f18_Req2Layoutp->gauge_w,
                   &button_sprite[resvars.f12[3]], 1000, 30, 970, 1);
        if (staff->subtype != 1 && staff->subtype != 4) {
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
                    crequester.unknown6 + staff->unknown108);
            break;
        case 9:
            sprintf(buf, D_001602a0[crequester.unknown8], staff->unknown108 * 2, &allhosps[crequester.unknown6]);
            break;
        }
        lbDisplay.draw_flags = 0x20;
        LbTextSetWindow(x0 + resvars.f18_Req2Layoutp->msg_x, y0 + resvars.f18_Req2Layoutp->msg_y, resvars.f18_Req2Layoutp->msg_w, resvars.f18_Req2Layoutp->msg_h);
        LbTextDraw(0, 0, buf);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        give_absolute_xy_of_cell_at_column_row(staff->x.tile, staff->y.tile);
        window.f0[window.unknown2ec - 1] = (resvars.f0 == SCREEN_MODE_13 ? ((staff->x.frac - staff->y.frac) >> 3) + 0x20
                                                                        : ((staff->x.frac - staff->y.frac) >> 4) + 0x10)
                                           + (info.unknown54 - resvars.f1c[171].x);
        window.fc[window.unknown2ec - 1] = info.unknown58 - resvars.f1c[171].y
                                           + (resvars.f0 == SCREEN_MODE_13 ? (staff->y.frac + staff->x.frac) >> 4
                                                                           : (staff->y.frac + staff->x.frac) >> 5);
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
void open_staff_resign_requester(Thing *staff)
{
    u32 i;
    s32 pay_gap;
    u32 nplayers;

    nplayers = 1;
    if (statvar.unknownfd > 1)
        nplayers = 4;
    play_sound(10, 0x9a, -1, -1);
    display_mode = 0xb;
    close_irequesters();
    crequester.unknown2 = 0xc;
    fn_0001bc94(crequester.unknown2, 1);
    crequester.unknown4 = staff - thing;
    crequester.unknown9 = 0;
    if (staff->f93) {
        crequester.unknown6 = 0;
        for (i = 0; i < nplayers; i++) {
            if (statvar.unknownfd > 1 && D_001609c4[i].active == 0)
                continue;
            if (staff->f93 == 1 << i) {
                crequester.unknown6 = i;
                break;
            }
        }
        crequester.unknown8 = 9;
    } else if (staff->f8f > 5) {
        crequester.unknown6 = 0;
        crequester.unknown8 = 2;
    } else if (staff->f70 > staff->unknown108) {
        pay_gap = staff->f70 - staff->unknown108;
        crequester.unknown6 = staff->f70 * 10 / 100 + pay_gap;
        crequester.unknown8 = 7;
    } else if (staff->f78 > gbv.fea && saferand() % 2) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.unknown8 = 3;
        else
            crequester.unknown8 = 4;
        crequester.unknown6 = staff->unknown108 * i / 100;
    } else if (staff->unknown81s > gbv.fea) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.unknown8 = 5;
        else
            crequester.unknown8 = 6;
        crequester.unknown6 = staff->unknown108 * i / 100;
    } else {
        i = safe_normal_rand(10, 30);
        crequester.unknown8 = 8;
        crequester.unknown6 = staff->unknown108 * i / 100;
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
    Thing *candidate;

    candidate = &thing[crequester.unknown6];
    if (candidate > thing && (candidate->unknown124 & 0x10))
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c184
void req2_help_if_surgeon(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.unknown6];
    mask = 0x30;
    if (candidate > thing && (candidate->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c214
void req2_help_if_psycho(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.unknown6];
    mask = 0x50;
    if (candidate > thing && (candidate->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c2a4
void req2_help_if_researcher(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.unknown6];
    mask = 0x110;
    if (candidate > thing && (candidate->unknown124 & mask) == mask)
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
    Thing *staff;

    staff = &thing[crequester.unknown4];
    if (staff > thing && (staff->unknown124 & 0x10))
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c408
void req12_help_if_surgeon(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.unknown4];
    mask = 0x30;
    if (staff > thing && (staff->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c498
void req12_help_if_shrink(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.unknown4];
    mask = 0x50;
    if (staff > thing && (staff->unknown124 & mask) == mask)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x0006c528
void req12_help_if_research(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.unknown4];
    mask = 0x110;
    if (staff > thing && (staff->unknown124 & mask) == mask)
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
