#include "hosp.h"

struct SlistArea {
    u16 x0;
    u16 y0;
    u16 unknown4;
    u16 x1;
    u16 y1;
    u8 unknowna[8];
    u16 face_x;
    u16 face_y;
};

struct SlistMenuItem {
    u8 unknown0[8];
    u16 text;
    u16 row;
    u8 unknownc[4];
};

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

extern "C" {
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern Event D_001549d0[];
extern char **D_001602f4;
extern char D_001209c8[], D_001209cb[], D_001209d4[], D_001209d8[], D_001209db[];
extern char D_001209de[], D_001209e2[];
extern Slot D_001581b2[];
}

u16 get_slist_staff(const u8 dir);
void draw_slist_staff(const u16 selected);
void draw_slist_window(const u16 id);
void draw_slist_panel(void);
void draw_slist_overall(void);
void process_slist_panel(void);
u8 staff_on_page(const u8 page);
void draw_slist_nums(void);
void draw_slist_coverup(void);
void slist_keys(void);
void draw_slist_button_overlays(Thing *w);
void draw_slist_training_bar(u16 x, u16 y, u16 value, u16 max);
void cancel_helpmsg(void);
void draw_gauge(s32 value, const u16 x, const u16 y, const u16 w, TbSprite *spr, const s32 max, const s32 lo, const s32 hi, const u8 flag);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
void draw_window_not_using_bscreen(const u8 n);
void draw_face(Picture *pic, const u16 x, const u16 y);

// FUNCTION: 0x000914e0
void display_slist(void)
{
    u16 prev;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0;
    switch (slist.unknown2) {
    case 0:
        load_ingame_pic(8, 1, 0x21, 0, SCREEN_MODE_13);
        D_001290d0 = 0xff;
        D_001290d1 = 0xff;
        D_001290d2 = 0xff;
        set_mouse_mode(0x21);
        slist.unknown2 = 1;
        slist.unknown0 = 0;
        othervar.f49 = 1;
        close_irequesters();
        open_new_window(0, 0, 0, resvars.f44_SlistAreap->x0, resvars.f44_SlistAreap->y0, resvars.f44_SlistAreap->x1, resvars.f44_SlistAreap->y1);
        window.unknown2eb = window.unknown2ec - 1;
        othervar.f91 = 0;
        if (othervar.fa82) {
            switch ((thing + othervar.fa82)->subtype) {
            case 0:
                othervar.f49 = 2;
                break;
            case 1:
                othervar.f49 = 1;
                break;
            case 2:
                othervar.f49 = 4;
                break;
            case 3:
                othervar.f49 = 8;
                break;
            case 4:
                othervar.f49 = 1;
                break;
            default:
                break;
            }
            for (;;) {
                prev = slist.unknown0;
                slist.unknown0 = get_slist_staff(0);
                if (prev == slist.unknown0) {
                    slist.unknown0 = 0;
                    break;
                }
                if (slist.unknown0 == othervar.fa82)
                    break;
                if (othervar.fa60 / 11 > 0 && othervar.fa60 % 11 == 0) {
                    if (othervar.f91 < 150) {
                        if (staff_on_page(othervar.f91 + 1))
                            othervar.f91++;
                    }
                }
            }
        }
        copy_screen_to_wscreen(bscreen);
        draw_slist_staff(slist.unknown0);
        draw_slist_window(slist.unknown0);
        draw_slist_panel();
        draw_slist_overall();
        draw_slist_nums();
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        draw_slist_staff(slist.unknown0);
        draw_slist_window(slist.unknown0);
        draw_slist_panel();
        draw_slist_overall();
        draw_slist_nums();
        process_slist_panel();
        break;
    case 2:
        D_001549d0[player].type = 0x87;
        D_001549d0[player].value = 0;
        D_001549d0[player].arg = slist.unknown0;
        cancel_helpmsg();
        break;
    case 3:
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        cancel_helpmsg();
        break;
    default:
        break;
    }
    slist_keys();
    lbDisplay.draw_flags = 0;
    if (!staff_on_page(othervar.f91)) {
        if (othervar.f91 > 0)
            othervar.f91--;
    }
    lbDisplay.rleft_button = lbDisplay.left_button = 0;
    if (exit_quick_requester())
        slist.unknown2 = 3;
    compute_help_text(0, 0, 6);
}

// FUNCTION: 0x0009187c
void slist_keys(void)
{
    u16 prev;
    int i;

    prev = 0;
    if (lbInkey == 0x4d || lbKeyOn[0xcd]) {
        if (othervar.f49 != 8) {
            othervar.f49 <<= 1;
            slist.unknown0 = 0;
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            othervar.fa60 = 0;
            play_sound(10, 0x43, -1, -1);
        }
        lbInkey = 0;
    } else if (lbInkey == 0x4b || lbKeyOn[0xcb]) {
        if (othervar.f49 != 1) {
            othervar.f49 >>= 1;
            slist.unknown0 = 0;
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            othervar.fa60 = 0;
            play_sound(10, 0x43, -1, -1);
        }
        lbInkey = 0;
    }
    if (lbInkey == 0x50 || lbKeyOn[0xd0]) {
        prev = slist.unknown0;
        slist.unknown0 = get_slist_staff(0);
        if (prev != slist.unknown0) {
            play_sound(10, 0x43, -1, -1);
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            lbInkey = 0;
            if (othervar.fa60 / 11 > 0 && othervar.fa60 % 11 == 0) {
                if (othervar.f91 < 150) {
                    if (staff_on_page(othervar.f91 + 1)) {
                        othervar.f91++;
                        play_sound(10, 0x43, -1, -1);
                    } else {
                        play_sound(10, 0x43, -1, -1);
                    }
                } else {
                    play_sound(10, 0x43, -1, -1);
                }
            }
        }
    } else if (lbInkey == 0x48 || lbKeyOn[0xc8]) {
        prev = slist.unknown0;
        slist.unknown0 = get_slist_staff(1);
        if (prev != slist.unknown0) {
            play_sound(10, 0x43, -1, -1);
            if (othervar.fa60 / 10 > 0 && othervar.fa60 % 10 == 0) {
                if (othervar.f91 > 0) {
                    play_sound(10, 0x43, -1, -1);
                    othervar.f91--;
                    for (i = 0; i < 10; i++)
                        slist.unknown0 = get_slist_staff(1);
                } else {
                    play_sound(10, 0x43, -1, -1);
                }
            }
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            lbInkey = 0;
        }
    }
}

// FUNCTION: 0x00091bac
u16 get_slist_staff(const u8 dir)
{
    Thing *w;

    w = &thing[slist.unknown0];
    if (dir == 0) {
        while (++w < end_things) {
            if (w->renderer == 4 && w->owner == player) {
                if (w->kind != 0x2e) {
                    if (((w->subtype == 1 || w->subtype == 4) && (othervar.f49 & 1))
                        || (w->subtype == 0 && (othervar.f49 & 2))
                        || (w->subtype == 3 && (othervar.f49 & 8))
                        || (w->subtype == 2 && (othervar.f49 & 4))) {
                        othervar.fa60++;
                        return w - thing;
                    }
                }
            }
        }
    } else {
        while (thing + 1 <= --w) {
            if (w->renderer == 4 && w->owner == player) {
                if (w->kind != 0x2e) {
                    if (((w->subtype == 1 || w->subtype == 4) && (othervar.f49 & 1))
                        || (w->subtype == 0 && (othervar.f49 & 2))
                        || (w->subtype == 3 && (othervar.f49 & 8))
                        || (w->subtype == 2 && (othervar.f49 & 4))) {
                        othervar.fa60--;
                        return w - thing;
                    }
                }
            }
        }
    }
    return slist.unknown0;
}

// FUNCTION: 0x00091dec
void draw_slist_staff(const u16 selected)
{
    u16 row;
    char buf[100];
    Thing *w;
    u16 height;
    u16 n;
    int pages;

    n = 0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(1);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    for (row = 0; row < 3; row++) {
        lbDisplay.draw_flags = 0x100;
        LbTextSetWindow(resvars.f3c_Boxp[row + 61].x, resvars.f3c_Boxp[row + 61].y, resvars.f3c_Boxp[row + 61].w, resvars.f3c_Boxp[row + 61].h);
        LbTextDraw(0, (resvars.f3c_Boxp[row + 61].h - height) / 2, D_001602f4[row + 2]);
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    }
    row = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->owner == player) {
            if (w->kind == 0x2e)
                continue;
            if (row >= 10)
                break;
            if (((w->subtype == 1 || w->subtype == 4) && (othervar.f49 & 1))
                || (w->subtype == 0 && (othervar.f49 & 2))
                || (w->subtype == 3 && (othervar.f49 & 8))
                || (w->subtype == 2 && (othervar.f49 & 4))) {
                if (n >= othervar.f91 * 10 && n < (othervar.f91 + 1) * 10) {
                    set_font_from_cache(1);
                    if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_Boxp[row + 104].x, resvars.f3c_Boxp[row + 104].y, resvars.f3c_Boxp[row + 104].w, resvars.f3c_Boxp[row + 104].h)) {
                        if (othervar.fa60 != row + othervar.f91 * 10 + 1)
                            play_sound(10, 0x44, -1, -1);
                        set_font_from_cache(2);
                        othervar.fa60 = othervar.f91 * 10 + row + 1;
                    }
                    if (selected == (u16)(w - thing)) {
                        set_font_from_cache(2);
                        draw_frame(resvars.f3c_Boxp[row + 104].x, resvars.f3c_Boxp[row + 104].y - 1, resvars.f3c_Boxp[row + 104].w, resvars.f3c_Boxp[row + 104].h, (u16)color.unknownD);
                        draw_slist_button_overlays(w);
                    }
                    sprintf(buf, D_001209c8, n + 1);
                    LbTextDraw(resvars.f40[row + 114].x, resvars.f40[row + 114].y, buf);
                    sprintf(buf, D_001209cb, w->f84, staff_names_1[w->f85], staff_names_2[w->f86]);
                    LbTextDraw(resvars.f40[row + 124].x, resvars.f40[row + 124].y, buf);
                    sprintf(buf, D_001209d4, w->unknown108);
                    LbTextDraw(resvars.f40[row + 134].x, resvars.f40[row + 134].y, buf);
                    draw_gauge(w->unknown81s, resvars.f3c_Boxp[row + 71].x, resvars.f3c_Boxp[row + 71].y, resvars.f3c_Boxp[row + 71].w,
                               &screen_sprites_tab_start[resvars.f3c_Boxp[row + 71].h], 100, 0, 100, 1);
                    draw_gauge(w->f78, resvars.f3c_Boxp[row + 81].x, resvars.f3c_Boxp[row + 81].y, resvars.f3c_Boxp[row + 81].w,
                               &screen_sprites_tab_start[resvars.f3c_Boxp[row + 81].h], 1000, 0, 1000, 0);
                    draw_gauge(w->unknown126, resvars.f3c_Boxp[row + 91].x, resvars.f3c_Boxp[row + 91].y, resvars.f3c_Boxp[row + 91].w,
                               &screen_sprites_tab_start[resvars.f3c_Boxp[row + 91].h], 1000, 30, 970, 1);
                    if (lbDisplay.mleft_button) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f3c_Boxp[row + 104].x, resvars.f3c_Boxp[row + 104].y, resvars.f3c_Boxp[row + 104].w, resvars.f3c_Boxp[row + 104].h)) {
                            if ((u16)(w - thing) != slist.unknown0)
                                play_sound(10, 0x43, -1, -1);
                            slist.unknown0 = w - thing;
                        }
                    }
                    row++;
                }
                n++;
            }
        }
    }
    if (row < 10)
        LbDrawBox(resvars.f3c_Boxp[row + 104].x, resvars.f3c_Boxp[row + 104].y, resvars.f3c_Boxp[row + 104].w, (10 - row) * resvars.f3c_Boxp[row + 104].h, 0x77);
    othervar.fa5b = row;
    for (pages = 0; staff_on_page(pages); pages++)
        ;
    if (pages > 1) {
        u8 pos;

        pos = resvars.f3c_Boxp[114].h / (pages - 1) * othervar.f91;
        reqspr_draw(resvars.f3c_Boxp[114].x, pos + resvars.f3c_Boxp[114].y, &screen_sprites_tab_start[resvars.f3c_Boxp[114].w]);
    }
}

// FUNCTION: 0x000926ec
void draw_slist_window(const u16 id)
{
    u8 count;
    u8 i;
    u8 j;
    u16 save_y;
    Thing *w;
    u16 save_w;
    u16 save_h;
    u16 save_x;
    int dx;
    int dy;
    int ability;

    w = &thing[id];
    if (!id)
        return;
    if (resvars.f1 == SCREEN_MODE_13) {
        give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
        if (resvars.f0 == SCREEN_MODE_13)
            dx = ((w->x.frac - w->y.frac) >> 3) + 32;
        else
            dx = ((w->x.frac - w->y.frac) >> 4) + 16;
        window.f0[window.unknown2ec - 1] = dx + (info.unknown54 - *(s32 *)&resvars.f1c_u8p[0x558]);
        if (resvars.f0 == SCREEN_MODE_13)
            dy = (w->y.frac + w->x.frac) >> 4;
        else
            dy = (w->y.frac + w->x.frac) >> 5;
        window.fc[window.unknown2ec - 1] = info.unknown58 - *(s32 *)&resvars.f1c_u8p[0x55c] + dy;
        save_x = window.unknown2c3;
        save_y = window.unknown2c7;
        save_w = window.unknown2cb;
        save_h = window.unknown2cf;
        window.unknown2c3 = resvars.f44_SlistAreap->x0;
        window.unknown2c7 = resvars.f44_SlistAreap->y0;
        window.unknown2cb = resvars.f44_SlistAreap->x1;
        window.unknown2cf = resvars.f44_SlistAreap->y1;
        draw_window_not_using_bscreen(window.unknown2ec - 1);
        window.unknown2c3 = save_x;
        window.unknown2c7 = save_y;
        window.unknown2cb = save_w;
        window.unknown2cf = save_h;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    draw_face((Picture *)&thing[id].f87, resvars.f44_SlistAreap->face_x, resvars.f44_SlistAreap->face_y);
    if (w->subtype == 1 || w->subtype == 4) {
        i = 0;
        if ((w->unknown124 & 8) && !(w->unknown124 & 0x80))
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        i++;
        if ((w->unknown124 & 0x10) && !(w->unknown124 & 0x80) && !(w->unknown124 & 8))
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        i++;
        if (w->unknown124 & 0x80)
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        i++;
        if (w->unknown124 & 0x20) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        } else if (w->unknown148[3]) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, w->unknown148[3], gbv.f146);
        }
        i++;
        if (w->unknown124 & 0x40) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        } else if (w->unknown148[4]) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, w->unknown148[4], gbv.f148);
        }
        i++;
        if (w->unknown124 & 0x100) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h]);
        } else if (w->unknown148[5]) {
            reqspr_draw(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, &screen_sprites_tab_start[resvars.f3c_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.f3c_Boxp[i + 64].x, resvars.f3c_Boxp[i + 64].y, w->unknown148[5], gbv.f14a);
        }
    }
    ability = w->f8d;
    if (ability > 10)
        ability = 10;
    if (ability < 0)
        ability = 0;
    count = ability * 4200 / 1000;
    for (j = 0; j < count; j++)
        reqspr_draw(resvars.f3c_Boxp[70].x + j, resvars.f3c_Boxp[70].y, &screen_sprites_tab_start[resvars.f3c_Boxp[70].h]);
}

// FUNCTION: 0x00092e88
void draw_slist_panel(void)
{
}

// FUNCTION: 0x00092ea0
void draw_slist_overall(void)
{
    Thing *w;
    s32 happiness;
    s32 tiredness;
    s32 ability;
    u8 count;

    happiness = tiredness = ability = 0;
    count = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->owner == player) {
            if (w->kind != 0x2e) {
                if (((w->subtype == 1 || w->subtype == 4) && (othervar.f49 & 1))
                    || (w->subtype == 0 && (othervar.f49 & 2))
                    || (w->subtype == 3 && (othervar.f49 & 8))
                    || (w->subtype == 2 && (othervar.f49 & 4))) {
                    happiness += w->unknown81s;
                    tiredness += w->f78;
                    ability += w->unknown126;
                    count++;
                }
            }
        }
    }
    if (!(othervar.f49 & 1))
        draw_slist_coverup();
    if (count) {
        ability /= count;
        happiness /= count;
        tiredness /= count;
        count = 0;
        draw_gauge(happiness, resvars.f3c_Boxp[count + 101].x, resvars.f3c_Boxp[count + 101].y, resvars.f3c_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.f3c_Boxp[count + 101].h], 100, 0, 100, 1);
        count++;
        draw_gauge(tiredness, resvars.f3c_Boxp[count + 101].x, resvars.f3c_Boxp[count + 101].y, resvars.f3c_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.f3c_Boxp[count + 101].h], 1000, 0, 1000, 0);
        count++;
        draw_gauge(ability, resvars.f3c_Boxp[count + 101].x, resvars.f3c_Boxp[count + 101].y, resvars.f3c_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.f3c_Boxp[count + 101].h], 1000, 30, 970, 1);
    }
}

// FUNCTION: 0x000931d4
void process_slist_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0x30].width != 0 && resvars.f48[i + 0x30].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x30].x, resvars.f48[i + 0x30].y, resvars.f48[i + 0x30].width, resvars.f48[i + 0x30].height)) {
                resvars.f48[i + 0x30].callback(i);
                reqspr_draw(resvars.f48[i + 0x30].unknownC, resvars.f48[i + 0x30].unknownE, &screen_sprites_tab_start[resvars.f48[i + 0x30].sprite]);
            }
        }
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f44_SlistAreap->x0, resvars.f44_SlistAreap->y0, resvars.f44_SlistAreap->x1 - resvars.f44_SlistAreap->x0, resvars.f44_SlistAreap->y1 - resvars.f44_SlistAreap->y0)) {
            if (slist.unknown0) {
                othervar.f38 = 2;
                D_001581b2[2].value = slist.unknown0;
                othervar.f10 = 1;
                slist.unknown2 = 2;
                close_irequesters();
                window.unknown2eb = 0;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (slist.unknown0)
            draw_slist_button_overlays(&thing[slist.unknown0]);
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0x30].width != 0 && resvars.f48[i + 0x30].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f48[i + 0x30].x, resvars.f48[i + 0x30].y, resvars.f48[i + 0x30].width, resvars.f48[i + 0x30].height)) {
                reqspr_draw(resvars.f48[i + 0x30].unknownC, resvars.f48[i + 0x30].unknownE, &screen_sprites_tab_start[resvars.f48[i + 0x30].sprite]);
                break;
            }
        }
        if (slist.unknown0)
            draw_slist_button_overlays(&thing[slist.unknown0]);
    }
}

// FUNCTION: 0x00093574
void process_slist_button(u8 n)
{
    u16 sound;
    u16 i;

    switch (n) {
    case 0:
        if (othervar.f49 != 1)
            play_sound(10, 0x43, -1, -1);
        othervar.f49 = 1;
        slist.unknown0 = 0;
        othervar.fa60 = 0;
        break;
    case 1:
        if (othervar.f49 != 2)
            play_sound(10, 0x43, -1, -1);
        othervar.f49 = 2;
        slist.unknown0 = 0;
        othervar.fa60 = 0;
        break;
    case 2:
        if (othervar.f49 != 8)
            play_sound(10, 0x43, -1, -1);
        othervar.f49 = 8;
        slist.unknown0 = 0;
        othervar.fa60 = 0;
        break;
    case 3:
        if (othervar.f49 != 4)
            play_sound(10, 0x43, -1, -1);
        othervar.f49 = 4;
        slist.unknown0 = 0;
        othervar.fa60 = 0;
        break;
    case 4:
        if (slist.unknown0) {
            play_sound(10, 0x43, -1, -1);
            close_irequesters();
            lbDisplay.rleft_button = lbDisplay.rright_button = lbDisplay.left_button = lbDisplay.right_button = 0;
            set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
            sprintf(othervar.f935, D_001209d8, yesno_text[5]);
            open_yes_no_requester(0x47, slist.unknown0, 0, 0x88, slist.unknown0, 0);
            slist.unknown0 = 0;
        }
        break;
    case 5:
        if (slist.unknown0) {
            D_001549d0[player].type = 0x5a;
            D_001549d0[player].value = thing[slist.unknown0].unknown108 / 10;
            D_001549d0[player].arg = slist.unknown0;
            play_sound(10, 0xb4, -1, -1);
        }
        break;
    case 6:
        if (slist.unknown0) {
            D_001549d0[player].type = 0x48;
            D_001549d0[player].value = thing[slist.unknown0].unknown108 / 10;
            D_001549d0[player].arg = slist.unknown0;
            play_sound(10, 0xb8, -1, -1);
        }
        break;
    case 7:
        slist.unknown2 = 3;
        close_irequesters();
        window.unknown2eb = 0;
        play_sound(10, 0x42, -1, -1);
        break;
    case 8:
        sound = 8;
        if (othervar.f91 > 0) {
            if (staff_on_page(othervar.f91 - 1)) {
                othervar.f91--;
                for (i = 0; i < 10; i++)
                    slist.unknown0 = get_slist_staff(1);
                sound = 0x43;
            }
        }
        play_sound(10, sound, -1, -1);
        break;
    case 9:
        sound = 8;
        if (othervar.f91 < 150) {
            if (staff_on_page(othervar.f91 + 1)) {
                othervar.f91++;
                for (i = 0; i < 10; i++)
                    slist.unknown0 = get_slist_staff(0);
                sound = 0x43;
            }
        }
        play_sound(10, sound, -1, -1);
        break;
    case 10:
        D_001549d0[player].type = 0x5a;
        D_001549d0[player].value = 500;
        D_001549d0[player].arg = 0;
        play_sound(10, 0x43, -1, -1);
        break;
    case 11:
        if (slist.unknown0) {
            D_001549d0[player].type = 0x5a;
            D_001549d0[player].value = 500;
            D_001549d0[player].arg = slist.unknown0;
        }
        play_sound(10, 0x43, -1, -1);
        break;
    default:
        break;
    }
}

// FUNCTION: 0x00093a74
u8 staff_on_page(const u8 page)
{
    u16 n;
    Thing *w;

    n = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4) {
            if (w->kind != 0x2e && w->owner == player) {
                if (((w->subtype == 1 || w->subtype == 4) && (othervar.f49 & 1))
                    || (w->subtype == 0 && (othervar.f49 & 2))
                    || (w->subtype == 3 && (othervar.f49 & 8))
                    || (w->subtype == 2 && (othervar.f49 & 4))) {
                    if (n >= page * 10 && n < (page + 1) * 10)
                        return 1;
                    n++;
                }
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00093bb8
void draw_slist_nums(void)
{
    u8 i;
    Thing *w;
    u16 counts[4];
    char buf[100];

    if (othervar.f49 & 1)
        reqspr_draw(resvars.f48[0x30].unknownC, resvars.f48[0x30].unknownE, &screen_sprites_tab_start[resvars.f48[0x30].sprite]);
    else if (othervar.f49 & 2)
        reqspr_draw(resvars.f48[0x31].unknownC, resvars.f48[0x31].unknownE, &screen_sprites_tab_start[resvars.f48[0x31].sprite]);
    else if (othervar.f49 & 8)
        reqspr_draw(resvars.f48[0x32].unknownC, resvars.f48[0x32].unknownE, &screen_sprites_tab_start[resvars.f48[0x32].sprite]);
    else if (othervar.f49 & 4)
        reqspr_draw(resvars.f48[0x33].unknownC, resvars.f48[0x33].unknownE, &screen_sprites_tab_start[resvars.f48[0x33].sprite]);
    for (i = 0; i < 4; i++)
        counts[i] = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->owner == player) {
            if (w->kind == 0x2e)
                continue;
            if (w->subtype == 1 || w->subtype == 4)
                counts[0]++;
            else if (w->subtype == 0)
                counts[1]++;
            else if (w->subtype == 2)
                counts[2]++;
            else if (w->subtype == 3)
                counts[3]++;
        }
    }
    set_font_from_cache(0xd);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 4; i++) {
        sprintf(buf, D_001209db, counts[i]);
        LbTextDraw(resvars.f40[i + 144].x, resvars.f40[i + 144].y, buf);
    }
}

// FUNCTION: 0x00093eac
void draw_slist_coverup(void)
{
    LbDrawBox(resvars.f3c_Boxp[116].x, resvars.f3c_Boxp[116].y, resvars.f3c_Boxp[116].w, resvars.f3c_Boxp[116].h, 0x76);
    LbDrawBox(resvars.f3c_Boxp[117].x, resvars.f3c_Boxp[117].y, resvars.f3c_Boxp[117].w, resvars.f3c_Boxp[117].h, 0x76);
}

// FUNCTION: 0x00093f38
void slist_salary_help_if_present(void)
{
    if (resvars.f2c_SlistMenuItemp[othervar.f8ac].row < othervar.fa5b)
        fn_000bbff0(othervar.f96, help_text[50]);
}

// FUNCTION: 0x00093f88
void slist_happiness_help_if_present(void)
{
    if (resvars.f2c_SlistMenuItemp[othervar.f8ac].row < othervar.fa5b)
        fn_000bbff0(othervar.f96, help_text[51]);
}

// FUNCTION: 0x00093fd8
void slist_tiredness_help_if_present(void)
{
    if (resvars.f2c_SlistMenuItemp[othervar.f8ac].row < othervar.fa5b)
        fn_000bbff0(othervar.f96, help_text[52]);
}

// FUNCTION: 0x00094028
void slist_ability_help_if_present(void)
{
    if (resvars.f2c_SlistMenuItemp[othervar.f8ac].row < othervar.fa5b)
        fn_000bbff0(othervar.f96, help_text[53]);
}

// FUNCTION: 0x00094078
void slist_help_if_doctor(void)
{
    if (othervar.f49 & 1)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c_SlistMenuItemp[othervar.f8ac].text]);
}

// FUNCTION: 0x000940cc
void slist_help_if_surgeon(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((othervar.f49 & 1) && w > thing && (w->unknown124 & 0x20)) {
        fn_000bbff0(othervar.f96, help_text[resvars.f2c_SlistMenuItemp[othervar.f8ac].text]);
    } else {
        pct = w->unknown148[3];
        if (pct != 0) {
            pct = pct * 100 / gbv.f146;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.f96, help_text[66], pct);
        }
    }
}

// FUNCTION: 0x000941bc
void slist_help_if_shrink(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((othervar.f49 & 1) && w > thing && (w->unknown124 & 0x40)) {
        fn_000bbff0(othervar.f96, help_text[resvars.f2c_SlistMenuItemp[othervar.f8ac].text]);
    } else {
        pct = w->unknown148[4];
        if (pct != 0) {
            pct = pct * 100 / gbv.f148;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.f96, help_text[67], pct);
        }
    }
}

// FUNCTION: 0x000942ac
void slist_help_if_researcher(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((othervar.f49 & 1) && w > thing && (w->unknown124 & 0x100)) {
        fn_000bbff0(othervar.f96, help_text[resvars.f2c_SlistMenuItemp[othervar.f8ac].text]);
    } else {
        pct = w->unknown148[5];
        if (pct != 0) {
            pct = pct * 100 / gbv.f14a;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.f96, help_text[68], pct);
        }
    }
}

// FUNCTION: 0x0009439c
void draw_slist_button_overlays(Thing *w)
{
    char buf[100];

    set_font_from_cache(1);
    sprintf(buf, D_001209de, w->unknown108 / 10 + w->unknown108);
    LbTextDraw(resvars.f40[149].x, resvars.f40[149].y, buf);
    sprintf(buf, D_001209e2, w->unknown108 / 10);
    LbTextDraw(resvars.f40[150].x, resvars.f40[150].y, buf);
}

// FUNCTION: 0x00094458
void draw_slist_training_bar(u16 x, u16 y, u16 value, u16 max)
{
    u8 i;

    for (i = 0; i < value / (max / resvars.f3c_Boxp[115].w); i++) {
        if (i < resvars.f3c_Boxp[115].w)
            reqspr_draw(x + resvars.f3c_Boxp[115].x + i, y + resvars.f3c_Boxp[115].y, &screen_sprites_tab_start[13]);
    }
}
