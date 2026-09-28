#include "hosp.h"

struct SlistRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct SlistLayout {
    u8 unknown0[0x3d0];
    SlistRect titles[3];
    SlistRect icons[7];
    SlistRect gauge_a[10];
    SlistRect gauge_b[10];
    SlistRect gauge_c[10];
    SlistRect overall[3];
    SlistRect rows[10];
    SlistRect scroll;
    SlistRect bar;
    SlistRect cover[2];
};

struct SlistText {
    u8 unknown0[0x390];
    Point num[10];
    Point name[10];
    Point salary[10];
    Point counts[4];
    Point unknown4a0;
    Point raise;
    Point bonus;
};

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
extern u32 D_00188c38;
extern u16 D_00188c74;
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 D_00155401;
extern SlistArea *D_0015f438;
extern SlistLayout *D_0015f430;
extern SlistText *D_0015f434;
extern Button *D_0015f43c;
extern u8 D_00155449;
extern u16 D_00155e3a;
extern u8 D_00155e18;
extern u8 D_00155e13;
extern Event D_001549d0[];
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c81, D_00188c83;
extern s32 D_00188c5c, D_00188c60;
extern volatile s16 D_00188c6c, D_00188c70;
extern char **D_001602f4;
extern char D_001209c8[], D_001209cb[], D_001209d4[], D_001209d8[], D_001209db[];
extern char D_001209de[], D_001209e2[];
extern TbScreenMode D_0015f3f4, D_0015f3f5;
extern u8 *D_0015f410;
extern u16 D_0012ddc6, D_0012ddc8, D_0012ddca;
extern u8 D_001553f0;
extern u8 D_001553c8;
extern Slot D_001581b2[];
extern char D_00155ced[];
extern u16 D_00155c64;
extern char D_0015544e[];
extern SlistMenuItem *D_0015f420;
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

    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(1);
    D_00188c74 = 0;
    switch (slist.unknown2) {
    case 0:
        load_ingame_pic(8, 1, 0x21, 0, SCREEN_MODE_13);
        D_001290d0 = 0xff;
        D_001290d1 = 0xff;
        D_001290d2 = 0xff;
        set_mouse_mode(0x21);
        slist.unknown2 = 1;
        slist.unknown0 = 0;
        D_00155401 = 1;
        close_irequesters();
        open_new_window(0, 0, 0, D_0015f438->x0, D_0015f438->y0, D_0015f438->x1, D_0015f438->y1);
        window.unknown2eb = window.unknown2ec - 1;
        D_00155449 = 0;
        if (D_00155e3a) {
            switch ((thing + D_00155e3a)->subtype) {
            case 0:
                D_00155401 = 2;
                break;
            case 1:
                D_00155401 = 1;
                break;
            case 2:
                D_00155401 = 4;
                break;
            case 3:
                D_00155401 = 8;
                break;
            case 4:
                D_00155401 = 1;
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
                if (slist.unknown0 == D_00155e3a)
                    break;
                if (D_00155e18 / 11 > 0 && D_00155e18 % 11 == 0) {
                    if (D_00155449 < 150) {
                        if (staff_on_page(D_00155449 + 1))
                            D_00155449++;
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
    D_00188c74 = 0;
    if (!staff_on_page(D_00155449)) {
        if (D_00155449 > 0)
            D_00155449--;
    }
    D_00188c81 = D_00188c7b = 0;
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
        if (D_00155401 != 8) {
            D_00155401 <<= 1;
            slist.unknown0 = 0;
            D_00188c5c = 0;
            D_00188c60 = 0;
            D_00155e18 = 0;
            play_sound(10, 0x43, -1, -1);
        }
        lbInkey = 0;
    } else if (lbInkey == 0x4b || lbKeyOn[0xcb]) {
        if (D_00155401 != 1) {
            D_00155401 >>= 1;
            slist.unknown0 = 0;
            D_00188c5c = 0;
            D_00188c60 = 0;
            D_00155e18 = 0;
            play_sound(10, 0x43, -1, -1);
        }
        lbInkey = 0;
    }
    if (lbInkey == 0x50 || lbKeyOn[0xd0]) {
        prev = slist.unknown0;
        slist.unknown0 = get_slist_staff(0);
        if (prev != slist.unknown0) {
            play_sound(10, 0x43, -1, -1);
            D_00188c5c = 0;
            D_00188c60 = 0;
            lbInkey = 0;
            if (D_00155e18 / 11 > 0 && D_00155e18 % 11 == 0) {
                if (D_00155449 < 150) {
                    if (staff_on_page(D_00155449 + 1)) {
                        D_00155449++;
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
            if (D_00155e18 / 10 > 0 && D_00155e18 % 10 == 0) {
                if (D_00155449 > 0) {
                    play_sound(10, 0x43, -1, -1);
                    D_00155449--;
                    for (i = 0; i < 10; i++)
                        slist.unknown0 = get_slist_staff(1);
                } else {
                    play_sound(10, 0x43, -1, -1);
                }
            }
            D_00188c5c = 0;
            D_00188c60 = 0;
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
                    if (((w->subtype == 1 || w->subtype == 4) && (D_00155401 & 1))
                        || (w->subtype == 0 && (D_00155401 & 2))
                        || (w->subtype == 3 && (D_00155401 & 8))
                        || (w->subtype == 2 && (D_00155401 & 4))) {
                        D_00155e18++;
                        return w - thing;
                    }
                }
            }
        }
    } else {
        while (thing + 1 <= --w) {
            if (w->renderer == 4 && w->owner == player) {
                if (w->kind != 0x2e) {
                    if (((w->subtype == 1 || w->subtype == 4) && (D_00155401 & 1))
                        || (w->subtype == 0 && (D_00155401 & 2))
                        || (w->subtype == 3 && (D_00155401 & 8))
                        || (w->subtype == 2 && (D_00155401 & 4))) {
                        D_00155e18--;
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
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(1);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    for (row = 0; row < 3; row++) {
        D_00188c74 = 0x100;
        LbTextSetWindow(D_0015f430->titles[row].x, D_0015f430->titles[row].y, D_0015f430->titles[row].w, D_0015f430->titles[row].h);
        LbTextDraw(0, (D_0015f430->titles[row].h - height) / 2, D_001602f4[row + 2]);
        D_00188c74 = 0;
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    }
    row = 0;
    for (w = thing + 1; w < end_things; w++) {
        if (w->renderer == 4 && w->owner == player) {
            if (w->kind == 0x2e)
                continue;
            if (row >= 10)
                break;
            if (((w->subtype == 1 || w->subtype == 4) && (D_00155401 & 1))
                || (w->subtype == 0 && (D_00155401 & 2))
                || (w->subtype == 3 && (D_00155401 & 8))
                || (w->subtype == 2 && (D_00155401 & 4))) {
                if (n >= D_00155449 * 10 && n < (D_00155449 + 1) * 10) {
                    set_font_from_cache(1);
                    if (over_hotspot(mouse_dx, mouse_dy, D_0015f430->rows[row].x, D_0015f430->rows[row].y, D_0015f430->rows[row].w, D_0015f430->rows[row].h)) {
                        if (D_00155e18 != row + D_00155449 * 10 + 1)
                            play_sound(10, 0x44, -1, -1);
                        set_font_from_cache(2);
                        D_00155e18 = D_00155449 * 10 + row + 1;
                    }
                    if (selected == (u16)(w - thing)) {
                        set_font_from_cache(2);
                        draw_frame(D_0015f430->rows[row].x, D_0015f430->rows[row].y - 1, D_0015f430->rows[row].w, D_0015f430->rows[row].h, (u16)color.unknownD);
                        draw_slist_button_overlays(w);
                    }
                    sprintf(buf, D_001209c8, n + 1);
                    LbTextDraw(D_0015f434->num[row].x, D_0015f434->num[row].y, buf);
                    sprintf(buf, D_001209cb, w->f84, staff_names_1[w->f85], staff_names_2[w->f86]);
                    LbTextDraw(D_0015f434->name[row].x, D_0015f434->name[row].y, buf);
                    sprintf(buf, D_001209d4, w->unknown108);
                    LbTextDraw(D_0015f434->salary[row].x, D_0015f434->salary[row].y, buf);
                    draw_gauge(w->unknown81s, D_0015f430->gauge_a[row].x, D_0015f430->gauge_a[row].y, D_0015f430->gauge_a[row].w,
                               &screen_sprites_tab_start[D_0015f430->gauge_a[row].h], 100, 0, 100, 1);
                    draw_gauge(w->f78, D_0015f430->gauge_b[row].x, D_0015f430->gauge_b[row].y, D_0015f430->gauge_b[row].w,
                               &screen_sprites_tab_start[D_0015f430->gauge_b[row].h], 1000, 0, 1000, 0);
                    draw_gauge(w->unknown126, D_0015f430->gauge_c[row].x, D_0015f430->gauge_c[row].y, D_0015f430->gauge_c[row].w,
                               &screen_sprites_tab_start[D_0015f430->gauge_c[row].h], 1000, 30, 970, 1);
                    if (D_00188c7e) {
                        if (over_hotspot(mouse_dx, mouse_dy, D_0015f430->rows[row].x, D_0015f430->rows[row].y, D_0015f430->rows[row].w, D_0015f430->rows[row].h)) {
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
        LbDrawBox(D_0015f430->rows[row].x, D_0015f430->rows[row].y, D_0015f430->rows[row].w, (10 - row) * D_0015f430->rows[row].h, 0x77);
    D_00155e13 = row;
    for (pages = 0; staff_on_page(pages); pages++)
        ;
    if (pages > 1) {
        u8 pos;

        pos = D_0015f430->scroll.h / (pages - 1) * D_00155449;
        reqspr_draw(D_0015f430->scroll.x, pos + D_0015f430->scroll.y, &screen_sprites_tab_start[D_0015f430->scroll.w]);
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
    if (D_0015f3f5 == SCREEN_MODE_13) {
        give_absolute_xy_of_cell_at_column_row(w->x.tile, w->y.tile);
        if (D_0015f3f4 == SCREEN_MODE_13)
            dx = ((w->x.frac - w->y.frac) >> 3) + 32;
        else
            dx = ((w->x.frac - w->y.frac) >> 4) + 16;
        window.f0[window.unknown2ec - 1] = dx + (info.unknown54 - *(s32 *)&D_0015f410[0x558]);
        if (D_0015f3f4 == SCREEN_MODE_13)
            dy = (w->y.frac + w->x.frac) >> 4;
        else
            dy = (w->y.frac + w->x.frac) >> 5;
        window.fc[window.unknown2ec - 1] = info.unknown58 - *(s32 *)&D_0015f410[0x55c] + dy;
        save_x = window.unknown2c3;
        save_y = window.unknown2c7;
        save_w = window.unknown2cb;
        save_h = window.unknown2cf;
        window.unknown2c3 = D_0015f438->x0;
        window.unknown2c7 = D_0015f438->y0;
        window.unknown2cb = D_0015f438->x1;
        window.unknown2cf = D_0015f438->y1;
        draw_window_not_using_bscreen(window.unknown2ec - 1);
        window.unknown2c3 = save_x;
        window.unknown2c7 = save_y;
        window.unknown2cb = save_w;
        window.unknown2cf = save_h;
    }
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    draw_face((Picture *)&thing[id].f87, D_0015f438->face_x, D_0015f438->face_y);
    if (w->subtype == 1 || w->subtype == 4) {
        i = 0;
        if ((w->unknown124 & 8) && !(w->unknown124 & 0x80))
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        i++;
        if ((w->unknown124 & 0x10) && !(w->unknown124 & 0x80) && !(w->unknown124 & 8))
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        i++;
        if (w->unknown124 & 0x80)
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        i++;
        if (w->unknown124 & 0x20) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        } else if (w->unknown148[3]) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h - 3]);
            draw_slist_training_bar(D_0015f430->icons[i].x, D_0015f430->icons[i].y, w->unknown148[3], D_0012ddc6);
        }
        i++;
        if (w->unknown124 & 0x40) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        } else if (w->unknown148[4]) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h - 3]);
            draw_slist_training_bar(D_0015f430->icons[i].x, D_0015f430->icons[i].y, w->unknown148[4], D_0012ddc8);
        }
        i++;
        if (w->unknown124 & 0x100) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h]);
        } else if (w->unknown148[5]) {
            reqspr_draw(D_0015f430->icons[i].x, D_0015f430->icons[i].y, &screen_sprites_tab_start[D_0015f430->icons[i].h - 3]);
            draw_slist_training_bar(D_0015f430->icons[i].x, D_0015f430->icons[i].y, w->unknown148[5], D_0012ddca);
        }
    }
    ability = w->f8d;
    if (ability > 10)
        ability = 10;
    if (ability < 0)
        ability = 0;
    count = ability * 4200 / 1000;
    for (j = 0; j < count; j++)
        reqspr_draw(D_0015f430->icons[6].x + j, D_0015f430->icons[6].y, &screen_sprites_tab_start[D_0015f430->icons[6].h]);
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
                if (((w->subtype == 1 || w->subtype == 4) && (D_00155401 & 1))
                    || (w->subtype == 0 && (D_00155401 & 2))
                    || (w->subtype == 3 && (D_00155401 & 8))
                    || (w->subtype == 2 && (D_00155401 & 4))) {
                    happiness += w->unknown81s;
                    tiredness += w->f78;
                    ability += w->unknown126;
                    count++;
                }
            }
        }
    }
    if (!(D_00155401 & 1))
        draw_slist_coverup();
    if (count) {
        ability /= count;
        happiness /= count;
        tiredness /= count;
        count = 0;
        draw_gauge(happiness, D_0015f430->overall[count].x, D_0015f430->overall[count].y, D_0015f430->overall[count].w,
                   &screen_sprites_tab_start[D_0015f430->overall[count].h], 100, 0, 100, 1);
        count++;
        draw_gauge(tiredness, D_0015f430->overall[count].x, D_0015f430->overall[count].y, D_0015f430->overall[count].w,
                   &screen_sprites_tab_start[D_0015f430->overall[count].h], 1000, 0, 1000, 0);
        count++;
        draw_gauge(ability, D_0015f430->overall[count].x, D_0015f430->overall[count].y, D_0015f430->overall[count].w,
                   &screen_sprites_tab_start[D_0015f430->overall[count].h], 1000, 30, 970, 1);
    }
}

// FUNCTION: 0x000931d4
void process_slist_panel(void)
{
    u8 i;

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0x30].width != 0 && D_0015f43c[i + 0x30].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0x30].x, D_0015f43c[i + 0x30].y, D_0015f43c[i + 0x30].width, D_0015f43c[i + 0x30].height)) {
                D_0015f43c[i + 0x30].callback(i);
                reqspr_draw(D_0015f43c[i + 0x30].unknownC, D_0015f43c[i + 0x30].unknownE, &screen_sprites_tab_start[D_0015f43c[i + 0x30].sprite]);
            }
        }
        if (over_hotspot(D_00188c6c, D_00188c70, D_0015f438->x0, D_0015f438->y0, D_0015f438->x1 - D_0015f438->x0, D_0015f438->y1 - D_0015f438->y0)) {
            if (slist.unknown0) {
                D_001553f0 = 2;
                D_001581b2[2].value = slist.unknown0;
                D_001553c8 = 1;
                slist.unknown2 = 2;
                close_irequesters();
                window.unknown2eb = 0;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (slist.unknown0)
            draw_slist_button_overlays(&thing[slist.unknown0]);
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x30].width != 0 && D_0015f43c[i + 0x30].height != 0; i++) {
            if (over_hotspot(mouse_dx, mouse_dy, D_0015f43c[i + 0x30].x, D_0015f43c[i + 0x30].y, D_0015f43c[i + 0x30].width, D_0015f43c[i + 0x30].height)) {
                reqspr_draw(D_0015f43c[i + 0x30].unknownC, D_0015f43c[i + 0x30].unknownE, &screen_sprites_tab_start[D_0015f43c[i + 0x30].sprite]);
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
        if (D_00155401 != 1)
            play_sound(10, 0x43, -1, -1);
        D_00155401 = 1;
        slist.unknown0 = 0;
        D_00155e18 = 0;
        break;
    case 1:
        if (D_00155401 != 2)
            play_sound(10, 0x43, -1, -1);
        D_00155401 = 2;
        slist.unknown0 = 0;
        D_00155e18 = 0;
        break;
    case 2:
        if (D_00155401 != 8)
            play_sound(10, 0x43, -1, -1);
        D_00155401 = 8;
        slist.unknown0 = 0;
        D_00155e18 = 0;
        break;
    case 3:
        if (D_00155401 != 4)
            play_sound(10, 0x43, -1, -1);
        D_00155401 = 4;
        slist.unknown0 = 0;
        D_00155e18 = 0;
        break;
    case 4:
        if (slist.unknown0) {
            play_sound(10, 0x43, -1, -1);
            close_irequesters();
            D_00188c81 = D_00188c83 = D_00188c7b = D_00188c7c = 0;
            set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
            sprintf(D_00155ced, D_001209d8, yesno_text[5]);
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
        if (D_00155449 > 0) {
            if (staff_on_page(D_00155449 - 1)) {
                D_00155449--;
                for (i = 0; i < 10; i++)
                    slist.unknown0 = get_slist_staff(1);
                sound = 0x43;
            }
        }
        play_sound(10, sound, -1, -1);
        break;
    case 9:
        sound = 8;
        if (D_00155449 < 150) {
            if (staff_on_page(D_00155449 + 1)) {
                D_00155449++;
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
                if (((w->subtype == 1 || w->subtype == 4) && (D_00155401 & 1))
                    || (w->subtype == 0 && (D_00155401 & 2))
                    || (w->subtype == 3 && (D_00155401 & 8))
                    || (w->subtype == 2 && (D_00155401 & 4))) {
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

    if (D_00155401 & 1)
        reqspr_draw(D_0015f43c[0x30].unknownC, D_0015f43c[0x30].unknownE, &screen_sprites_tab_start[D_0015f43c[0x30].sprite]);
    else if (D_00155401 & 2)
        reqspr_draw(D_0015f43c[0x31].unknownC, D_0015f43c[0x31].unknownE, &screen_sprites_tab_start[D_0015f43c[0x31].sprite]);
    else if (D_00155401 & 8)
        reqspr_draw(D_0015f43c[0x32].unknownC, D_0015f43c[0x32].unknownE, &screen_sprites_tab_start[D_0015f43c[0x32].sprite]);
    else if (D_00155401 & 4)
        reqspr_draw(D_0015f43c[0x33].unknownC, D_0015f43c[0x33].unknownE, &screen_sprites_tab_start[D_0015f43c[0x33].sprite]);
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
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    for (i = 0; i < 4; i++) {
        sprintf(buf, D_001209db, counts[i]);
        LbTextDraw(D_0015f434->counts[i].x, D_0015f434->counts[i].y, buf);
    }
}

// FUNCTION: 0x00093eac
void draw_slist_coverup(void)
{
    LbDrawBox(D_0015f430->cover[0].x, D_0015f430->cover[0].y, D_0015f430->cover[0].w, D_0015f430->cover[0].h, 0x76);
    LbDrawBox(D_0015f430->cover[1].x, D_0015f430->cover[1].y, D_0015f430->cover[1].w, D_0015f430->cover[1].h, 0x76);
}

// FUNCTION: 0x00093f38
void slist_salary_help_if_present(void)
{
    if (D_0015f420[D_00155c64].row < D_00155e13)
        fn_000bbff0(D_0015544e, help_text[50]);
}

// FUNCTION: 0x00093f88
void slist_happiness_help_if_present(void)
{
    if (D_0015f420[D_00155c64].row < D_00155e13)
        fn_000bbff0(D_0015544e, help_text[51]);
}

// FUNCTION: 0x00093fd8
void slist_tiredness_help_if_present(void)
{
    if (D_0015f420[D_00155c64].row < D_00155e13)
        fn_000bbff0(D_0015544e, help_text[52]);
}

// FUNCTION: 0x00094028
void slist_ability_help_if_present(void)
{
    if (D_0015f420[D_00155c64].row < D_00155e13)
        fn_000bbff0(D_0015544e, help_text[53]);
}

// FUNCTION: 0x00094078
void slist_help_if_doctor(void)
{
    if (D_00155401 & 1)
        fn_000bbff0(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
}

// NONMATCHING: pct * 100 is computed before the divisor global is loaded in the target; here after
// FUNCTION: 0x000940cc
void slist_help_if_surgeon(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((D_00155401 & 1) && w > thing && (w->unknown124 & 0x20)) {
        fn_000bbff0(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
    } else {
        pct = w->unknown148[3];
        if (pct != 0) {
            pct = pct * 100 / D_0012ddc6;
            pct > 100 ? pct = 100 : 0;
            sprintf(D_0015544e, help_text[66], pct);
        }
    }
}

// NONMATCHING: pct * 100 is computed before the divisor global is loaded in the target; here after
// FUNCTION: 0x000941bc
void slist_help_if_shrink(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((D_00155401 & 1) && w > thing && (w->unknown124 & 0x40)) {
        fn_000bbff0(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
    } else {
        pct = w->unknown148[4];
        if (pct != 0) {
            pct = pct * 100 / D_0012ddc8;
            pct > 100 ? pct = 100 : 0;
            sprintf(D_0015544e, help_text[67], pct);
        }
    }
}

// NONMATCHING: pct * 100 is computed before the divisor global is loaded in the target; here after
// FUNCTION: 0x000942ac
void slist_help_if_researcher(void)
{
    Thing *w;
    s32 pct;

    w = &thing[slist.unknown0];
    if ((D_00155401 & 1) && w > thing && (w->unknown124 & 0x100)) {
        fn_000bbff0(D_0015544e, help_text[D_0015f420[D_00155c64].text]);
    } else {
        pct = w->unknown148[5];
        if (pct != 0) {
            pct = pct * 100 / D_0012ddca;
            pct > 100 ? pct = 100 : 0;
            sprintf(D_0015544e, help_text[68], pct);
        }
    }
}

// FUNCTION: 0x0009439c
void draw_slist_button_overlays(Thing *w)
{
    char buf[100];

    set_font_from_cache(1);
    sprintf(buf, D_001209de, w->unknown108 / 10 + w->unknown108);
    LbTextDraw(D_0015f434->raise.x, D_0015f434->raise.y, buf);
    sprintf(buf, D_001209e2, w->unknown108 / 10);
    LbTextDraw(D_0015f434->bonus.x, D_0015f434->bonus.y, buf);
}

// FUNCTION: 0x00094458
void draw_slist_training_bar(u16 x, u16 y, u16 value, u16 max)
{
    u8 i;

    for (i = 0; i < value / (max / D_0015f430->bar.w); i++) {
        if (i < D_0015f430->bar.w)
            reqspr_draw(x + D_0015f430->bar.x + i, y + D_0015f430->bar.y, &screen_sprites_tab_start[13]);
    }
}
