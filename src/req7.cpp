#include "hosp.h"

struct QueueRect {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct QueueLayout {
    u8 unknown0[0x420];
    QueueRect r[27];
};

struct QueuePanel {
    u8 unknown0[0x48];
    s32 queue_x;
    s32 queue_y;
    s32 queue_w;
    u8 unknown54[0x364];
    Point labels[8];
    u8 unknown3f8[0x1a8];
    s32 f5a0;
};

struct QueueButtons {
    u8 unknown0[0xa];
    u8 fa;
    u8 fb;
    u8 fc;
};

extern "C" {
extern u32 D_00188c38;
extern u16 D_00188c74;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern char **D_00160354;
extern QueueButtons *D_0015f402;
extern u16 *D_0015f406;
extern QueueLayout *D_0015f40c;
extern QueuePanel *D_0015f410;
extern TbScreenMode D_0015f3f4, D_0015f3f5;
extern Event D_001549d0[];
extern u16 D_00155419;
extern u16 D_0015541b;
extern u8 D_0015541f;
extern u16 D_001553e0;
extern u16 D_001553fb, D_001553fd;
extern u16 D_0015537a;
extern u16 D_00155436;
extern u8 D_001553d4;
extern u8 D_00155e1a, D_00155e19;
extern u8 D_00188c7e, D_00188c7f;
extern s16 D_001567b0[8];
extern char D_00120a20[], D_00120a23[], D_00120a26[], D_00120a29[];
extern char D_00120a2c[], D_00120a2f[];

void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void fn_0002f47c(Thing *w, s16 x, s16 y);
void fn_0002e804(Thing *w, s16 x, s16 y);
u32 fn_000856c8(u32 n);
}

u8 over_queue_person(u16 x, u16 y, u16 room_id, u8 sel);
void display_queue_people(u16 x, u16 y, u16 room_id, u8 sel);
void display_queue_person(u16 x, u16 y, u16 room_id, u8 sel);
void display_queue_bar(u16 x, u16 y);
u16 queue_person_x(u16 x, u16 y, u16 room_id, u8 sel);
u8 queue_person_valid(u16 room_id, u8 sel);
u16 get_queue_person(u16 room_id, u8 sel);
void display_panel_person_info(void);
void compute_room_pointed(u16 tile);

// FUNCTION: 0x000957f0
void req7(Window *w)
{
    u16 n;
    WindowButton *b;

    for (b = w->buttons, n = 1; b != 0; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = n;
            if (n == D_0015f402->fb) {
                D_001549d0[player].type = 0x54;
                D_001549d0[player].value = D_00155419;
                if (room_info[D_001549d0[player].value].f1e)
                    play_sound(10, 0x43, -1, -1);
                else
                    play_sound(10, 8, -1, -1);
            } else if (n == D_0015f402->fa) {
                D_001549d0[player].type = 0x53;
                D_001549d0[player].value = D_00155419;
                if (room_info[D_001549d0[player].value].f1e < 30)
                    play_sound(10, 0x43, -1, -1);
                else
                    play_sound(10, 8, -1, -1);
            } else if (n == D_0015f402->fc) {
                D_001549d0[player].type = 0x84;
                play_sound(10, 0x42, -1, -1);
            }
        }
        n++;
    }
}

// FUNCTION: 0x000959c8
void req7_overdraw(Window *w)
{
    u8 unusedc;
    u8 unused8;
    u8 i;
    u8 unused10;
    u16 off;
    u16 qy;
    u16 qx;
    s32 wy;
    s32 ox;
    s32 oy;
    s32 wx;
    char buf[100];

    wx = window.f0[window.unknown2eb];
    wy = window.fc[window.unknown2eb];
    unusedc = 0;
    unused8 = 0;
    unused10 = 0;
    compute_help_text(w->x, w->y, 7);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3, window.unknown2cf - window.unknown2c7);
    ox = w->x - D_00188c3c;
    oy = w->y - D_00188c40;
    set_font_from_cache(1);
    LbTextDraw(ox + D_0015f410->labels[0].x, oy + D_0015f410->labels[0].y, D_00160354[0]);
    LbTextDraw(ox + D_0015f410->labels[1].x, oy + D_0015f410->labels[1].y, D_00160354[1]);
    LbTextDraw(ox + D_0015f410->labels[2].x, oy + D_0015f410->labels[2].y, D_00160354[2]);
    if (D_0015f3f5 == SCREEN_MODE_1)
        LbTextSetClipWindow(ox + D_0015f410->labels[3].x, oy + D_0015f410->labels[3].y, 0x1c, 9);
    LbTextDraw(ox + D_0015f410->labels[3].x, oy + D_0015f410->labels[3].y, D_00160354[3]);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    sprintf(buf, D_00120a20, queue_size(&room_info[D_00155419]));
    LbTextDraw(ox + D_0015f410->labels[4].x, oy + D_0015f410->labels[4].y, buf);
    sprintf(buf, D_00120a23, room_info[D_00155419].unknown32);
    LbTextDraw(ox + D_0015f410->labels[5].x, oy + D_0015f410->labels[5].y, buf);
    sprintf(buf, D_00120a26, room_info[D_00155419].count);
    LbTextDraw(ox + D_0015f410->labels[6].x, oy + D_0015f410->labels[6].y, buf);
    sprintf(buf, D_00120a29, room_info[D_00155419].f1e);
    LbTextDraw(ox + D_0015f410->labels[7].x, oy + D_0015f410->labels[7].y, buf);
    if (!queue_person_valid(D_00155419, D_0015541b) && D_0015541f != 0) {
        D_0015541b = 0;
        D_001553fd = 0;
    } else {
        D_001553fd = get_queue_person(D_00155419, D_0015541b);
    }
    if (D_00188c7e) {
        display_queue_people(ox, oy, D_00155419, D_0015541b);
        display_queue_person(mouse_dx, mouse_dy, D_00155419, D_0015541b);
        D_0015541f = 1;
    } else if (D_00188c7f) {
        display_queue_people(ox, oy, D_00155419, D_0015541b);
        if (D_0015541b) {
            qx = queue_person_x(ox, oy, D_00155419, D_0015541b);
            qy = oy;
            display_queue_bar(qx, qy);
            D_0015541f = 2;
            compute_help_text(qx - D_0015f40c->r[4].x, qy + D_0015f40c->r[4].y, 0x12);
        } else {
            D_0015541f = 0;
        }
    }
    if (D_0015541f && D_0015541b) {
        if (!D_00188c7f) {
            if (MouseOnWindow(open_windows[0])) {
                set_mouse_mode(0);
            } else {
                RoomInfo *r;

                r = which_room(D_001553e0);
                set_mouse_mode(0x12);
                D_0015537a = get_real_person_from_room(D_00155419, D_0015541b);
                if (r && r->type == room_info[D_00155419].type)
                    compute_room_pointed(D_001553e0);
            }
        }
    } else if (D_00188c7e && !D_00155e1a) {
        RoomInfo *room;

        room = &room_info[D_00155419];
        if (room->type > 6) {
            if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[25].x, oy + D_0015f40c->r[25].y, D_0015f40c->r[25].w, D_0015f40c->r[25].h)
                || over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[26].x, oy + D_0015f40c->r[26].y, D_0015f40c->r[26].w, D_0015f40c->r[26].h))
                goto_column_row_in_window(0, room->unknown4 % 128, room->unknown4 / 128);
        }
    }
    if (D_00188c7e + D_00188c7f == 0) {
        switch (D_0015541f) {
        case 0:
            if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[1].x, oy + D_0015f40c->r[1].y, D_0015f40c->r[1].w, D_0015f40c->r[1].h)) {
                i = over_queue_person(ox, oy, D_00155419, D_0015541b);
                if (i)
                    D_0015541b = i;
            } else {
                D_0015541b = 0;
            }
            display_queue_people(ox, oy, D_00155419, D_0015541b);
            break;
        case 1:
            if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[3].x, oy + D_0015f40c->r[3].y, D_0015f40c->r[3].w, D_0015f40c->r[3].h)) {
                D_001549d0[player].type = 0x84;
                play_sound(10, 0x42, -1, -1);
                break;
            }
            if (D_0015541b) {
                if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[0].x, oy + D_0015f40c->r[0].y, D_0015f40c->r[0].w, D_0015f40c->r[0].h)) {
                    D_001549d0[player].type = 0x3c;
                    D_001549d0[player].value = D_00155419;
                    D_001549d0[player].arg = D_0015541b;
                    display_queue_people(ox, oy, D_00155419, D_0015541b);
                    play_sound(10, 0x43, -1, -1);
                    break;
                }
                if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[2].x, oy + D_0015f40c->r[2].y, D_0015f40c->r[2].w, D_0015f40c->r[2].h)) {
                    D_001549d0[player].type = 0x3d;
                    D_001549d0[player].value = D_00155419;
                    D_001549d0[player].arg = D_0015541b;
                    display_queue_people(ox, oy, D_00155419, D_0015541b);
                    play_sound(10, 0x43, -1, -1);
                    break;
                }
                if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c->r[1].x, oy + D_0015f40c->r[1].y, D_0015f40c->r[1].w, D_0015f40c->r[1].h)) {
                    display_queue_people(ox, oy, D_00155419, D_0015541b);
                    break;
                }
                {
                    RoomInfo *dest;
                    Thing *t;

                    dest = &room_info[allhosps[player].f11b9];
                    if (dest == 0) {
                        t = &thing[D_001553fb];
                        if (t > thing) {
                            if (t->renderer == 4 && t->subtype == 3)
                                dest = t->room;
                            else if (t->renderer == 6 && t->subtype == 0xb && t->unknown91)
                                dest = thing[t->unknown91].room;
                        }
                    }
                    if (dest) {
                        D_001549d0[player].type = 9;
                        D_001549d0[player].value = dest - room_info;
                        D_001549d0[player].arg = D_0015537a;
                        set_mouse_mode(0);
                        play_sound(10, 0x43, -1, -1);
                    }
                }
            } else {
                display_queue_people(ox, oy, D_00155419, 0);
            }
            break;
        case 2:
            if (D_0015541b) {
                qx = queue_person_x(ox, oy, D_00155419, D_0015541b);
                qy = oy;
                display_queue_people(ox, oy, D_00155419, D_0015541b);
                if (over_hotspot(mouse_dx, mouse_dy, qx - D_0015f40c->r[4].x + D_0015f40c->r[7].x, qy + D_0015f40c->r[4].y + D_0015f40c->r[7].y, D_0015f40c->r[7].w, D_0015f40c->r[7].h)) {
                    D_001549d0[player].type = 0x55;
                    D_001549d0[player].value = D_00155419;
                    D_001549d0[player].arg = D_0015541b;
                    play_sound(10, 0x43, -1, -1);
                    break;
                }
                if (over_hotspot(mouse_dx, mouse_dy, qx - D_0015f40c->r[4].x + D_0015f40c->r[8].x, qy + D_0015f40c->r[4].y + D_0015f40c->r[8].y, D_0015f40c->r[8].w, D_0015f40c->r[8].h)) {
                    D_001549d0[player].type = 0x51;
                    D_001549d0[player].value = D_00155419;
                    D_001549d0[player].arg = D_0015541b;
                    play_sound(10, 0xaa, -1, -1);
                    break;
                }
                {
                    off = 0;
                    for (i = 0;; i++) {
                        if (i != player) {
                            if (statvar.unknownfd > 1) {
                                if (network_num_players() <= i)
                                    break;
                            } else if (i >= 4) {
                                break;
                            }
                            if (over_hotspot(mouse_dx, mouse_dy, qx - D_0015f40c->r[4].x + D_0015f40c->r[24].x, qy + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y, D_0015f40c->r[24].w, D_0015f40c->r[24].h)) {
                                play_sound(10, 0xaa, -1, -1);
                                if (statvar.unknownfd > 1) {
                                    D_001549d0[player].type = 0x57;
                                    D_001549d0[player].value = get_queue_person(D_00155419, D_0015541b);
                                    D_001549d0[player].arg = fn_000856c8(i + 1);
                                } else {
                                    D_001549d0[player].type = 0x51;
                                    D_001549d0[player].value = D_00155419;
                                    D_001549d0[player].arg = D_0015541b;
                                }
                                break;
                            }
                            off += button_sprite[D_0015f406[0x2a]].height;
                        }
                    }
                }
            } else {
                display_queue_people(ox, oy, D_00155419, 0);
            }
            break;
        default:
            break;
        }
        D_0015541f = 0;
        set_mouse_mode(0);
    }
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(1);
    if (D_00188c7e)
        D_00155e1a = 1;
    else
        D_00155e1a = 0;
    if (D_00188c7f)
        D_00155e19 = 1;
    else
        D_00155e19 = 0;
}

// FUNCTION: 0x000969f4
u8 over_queue_person(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 n;
    RoomInfo *room;
    u16 size;
    s32 step;
    u16 px;
    u16 py;
    u16 id;
    Thing *w;
    CompFrame *fr;
    u16 x0;
    u16 x1;
    u16 y0;
    u16 y1;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = D_0015f410->queue_w / size;
        px = x + D_0015f410->queue_x;
        py = y + D_0015f410->queue_y;
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                if (n == sel)
                    px += D_00188c34 / D_0015f410->f5a0;
                fr = &frame[w->sprite];
                x0 = px - fr->width / 2;
                x1 = x0 + fr->width;
                y0 = py - fr->length;
                y1 = y0 + fr->length;
                if (x0 < mouse_dx && x1 - D_00188c34 / 32 > mouse_dx) {
                    if (y0 < mouse_dy && y1 > mouse_dy) {
                        D_00155436 = id;
                        display_panel_person_info();
                        return n;
                    }
                }
                px += step;
                if (n == sel)
                    px += D_00188c34 / D_0015f410->f5a0;
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
    return 0;
}

// NONMATCHING: register choice; stack slot order of the saved-frame locals
// FUNCTION: 0x00096c34
void display_queue_people(u16 x, u16 y, u16 room_id, u8 sel)
{
    u8 old_17;
    u16 n;
    RoomInfo *room;
    u16 size;
    s32 step;
    u16 px;
    u16 py;
    u16 id;
    Thing *w;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = D_0015f410->queue_w / size;
        px = x + D_0015f410->queue_x;
        py = y + D_0015f410->queue_y;
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                u16 old_sprite;
                u8 old_palette;
                u16 anim;
                CompFrame *fr;
                u16 x0;
                u16 x1;
                u16 y0;
                u16 y1;

                old_17 = w->unknown17;
                old_sprite = w->sprite;
                old_palette = w->palette;
                w->unknown17 = 2;
                anim = check_frame(anim_starts[w->subtype].unknown12, w) + w->unknown17;
                w->sprite = start[anim].frame;
                w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                if (n == sel)
                    px += D_00188c34 / D_0015f410->f5a0;
                if (w->style == 6)
                    fn_0002f47c(w, px, py);
                else if (w->style == 12 && D_001553d4 == 0)
                    fn_0002e804(w, px, py);
                else if ((u16)(w - thing) == D_001553fb)
                    draw_hilight_frame(w, px, py);
                else
                    draw_special_id_frame(w, px, py);
                if (n == sel)
                    px += D_00188c34 / D_0015f410->f5a0;
                w->unknown17 = old_17;
                w->sprite = old_sprite;
                w->palette = old_palette;
                fr = &frame[w->sprite];
                x0 = px - fr->width / 2;
                x1 = x0 + fr->width;
                y0 = py - fr->length;
                y1 = y0 + fr->length;
                px += step;
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x00096f50
void display_queue_person(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 n;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *w;
    u8 old_17;
    u16 old_sprite;
    u8 old_palette;
    u16 anim;
    CompFrame *fr;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                if (n == sel) {
                    old_17 = w->unknown17;
                    old_sprite = w->sprite;
                    old_palette = w->palette;
                    w->unknown17 = 2;
                    anim = check_frame(anim_starts[w->subtype].unknown12, w) + w->unknown17;
                    w->sprite = start[anim].frame;
                    w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                    InitPixBuffer(0, 0, 0x40, 0x40, 0, 0);
                    DrawIntoPixBuffer();
                    full_draw_frame(w, w->sprite, 0x20, 0x3a, 0, 0, w->palette);
                    RestoreFromPixBuffer();
                    pixBuf.x = x - 0x20;
                    pixBuf.y = y - 0x3a;
                    DrawPixBuffer(1, 0, 0, 0x40, 0x40);
                    FillPixBuffer(0xfe);
                    InitPixBuffer(0, 0, 0xb4, 0xb4, 0, 0);
                    D_00188c74 = 0;
                    fr = &frame[w->sprite];
                    set_all_refresh_zones_in_window(D_001567b0[4] + x, D_001567b0[5] + y, D_001567b0[4] + x + fr->width, y + D_001567b0[5] + fr->length, 3);
                    w->unknown17 = old_17;
                    w->sprite = old_sprite;
                    w->palette = old_palette;
                    break;
                }
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000971f8
void display_queue_bar(u16 x, u16 y)
{
    u8 i;
    u16 off;
    char buf[100];

    reqspr_draw(x - D_0015f40c->r[4].x, y + D_0015f40c->r[4].y, &button_sprite[D_0015f406[0x28]]);
    if (over_hotspot(mouse_dx, mouse_dy, x - D_0015f40c->r[4].x + D_0015f40c->r[7].x, y + D_0015f40c->r[4].y + D_0015f40c->r[7].y, D_0015f40c->r[7].w, D_0015f40c->r[7].h))
        reqspr_draw(D_0015f40c->r[7].x + (x - D_0015f40c->r[4].x), y + D_0015f40c->r[4].y + D_0015f40c->r[7].y, &button_sprite[D_0015f406[0x2c]]);
    else if (over_hotspot(mouse_dx, mouse_dy, x - D_0015f40c->r[4].x + D_0015f40c->r[8].x, y + D_0015f40c->r[4].y + D_0015f40c->r[8].y, D_0015f40c->r[8].w, D_0015f40c->r[8].h))
        reqspr_draw(D_0015f40c->r[8].x + (x - D_0015f40c->r[4].x), y + D_0015f40c->r[4].y + D_0015f40c->r[8].y, &button_sprite[D_0015f406[0x2d]]);
    set_all_refresh_zones_in_window(x - D_0015f40c->r[4].x, y + D_0015f40c->r[4].y,
                                    button_sprite[D_0015f406[0x28]].width + (x - D_0015f40c->r[4].x),
                                    button_sprite[D_0015f406[0x28]].height + (y + D_0015f40c->r[4].y), 3);
    off = 0;
    for (i = 0;; i++) {
        if (i != player) {
            if (statvar.unknownfd > 1) {
                if (network_num_players() <= i)
                    break;
            } else if (i >= 4) {
                break;
            }
            reqspr_draw(x - D_0015f40c->r[4].x, y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y, &button_sprite[D_0015f406[0x2a]]);
            set_all_refresh_zones_in_window(x - D_0015f40c->r[4].x, y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y,
                                            button_sprite[D_0015f406[0x2a]].width + (x - D_0015f40c->r[4].x),
                                            D_0015f40c->r[4].y + (y + off) + D_0015f40c->r[5].y + button_sprite[D_0015f406[0x2a]].height, 3);
            if (over_hotspot(mouse_dx, mouse_dy, x - D_0015f40c->r[4].x + D_0015f40c->r[9].x, y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y, D_0015f40c->r[9].w, D_0015f40c->r[9].h))
                reqspr_draw(D_0015f40c->r[9].x + (x - D_0015f40c->r[4].x), y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y, &button_sprite[D_0015f406[0x2e]]);
            LbTextSetWindow(D_0015f40c->r[23].x + (x - D_0015f40c->r[4].x), y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y + D_0015f40c->r[23].y, D_0015f40c->r[23].w, D_0015f40c->r[23].h);
            D_00188c74 = 0x100;
            if (statvar.unknownfd > 1)
                sprintf(buf, D_00120a2c, &allhosps[fn_000856c8(i + 1)]);
            else
                sprintf(buf, D_00120a2f, &allhosps[i]);
            LbTextDraw(0, 0, buf);
            D_00188c74 = 0;
            LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
            off += button_sprite[D_0015f406[0x2a]].height;
        }
    }
    if (D_0015f3f4 != SCREEN_MODE_1) {
        off -= D_0015f40c->r[9].y;
        reqspr_draw(x - D_0015f40c->r[4].x, y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y, &button_sprite[D_0015f406[0x2b]]);
        set_all_refresh_zones_in_window(x - D_0015f40c->r[4].x, y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y,
                                        button_sprite[D_0015f406[0x2a]].width + (x - D_0015f40c->r[4].x),
                                        button_sprite[D_0015f406[0x2b]].height + (y + off + D_0015f40c->r[4].y + D_0015f40c->r[5].y), 3);
    }
}

// NONMATCHING: register choice; stack slot order of the saved-frame locals
// FUNCTION: 0x00097904
u16 queue_person_x(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 n;
    RoomInfo *room;
    u16 size;
    s32 step;
    u16 px;
    u16 py;
    u16 id;
    Thing *w;
    CompFrame *fr;
    u16 x0;
    u16 x1;
    u16 y0;
    u16 y1;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = D_0015f410->queue_w / size;
        px = x + D_0015f410->queue_x;
        py = y + D_0015f410->queue_y;
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                u8 old_17;
                u16 old_sprite;
                u8 old_palette;
                u16 anim;

                old_17 = w->unknown17;
                old_sprite = w->sprite;
                old_palette = w->palette;
                w->unknown17 = 2;
                anim = check_frame(anim_starts[w->subtype].unknown12, w) + w->unknown17;
                w->sprite = start[anim].frame;
                w->palette = start[anim].flags > 0 ? (u8)1 : (u8)0;
                w->unknown17 = old_17;
                w->sprite = old_sprite;
                w->palette = old_palette;
                if (n == sel)
                    return px;
                fr = &frame[w->sprite];
                x0 = px - fr->width / 2;
                x1 = x0 + fr->width;
                y0 = py - fr->length;
                y1 = y0 + fr->length;
                px += step;
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00097b40
u8 queue_person_valid(u16 room_id, u8 sel)
{
    u16 n;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *w;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                if (n == sel)
                    return 1;
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00097c0c
u16 get_queue_person(u16 room_id, u8 sel)
{
    u16 n;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *w;

    n = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            w = &thing[id];
            for (;;) {
                if (n == sel)
                    return id;
                if (w->queue_next == 0)
                    break;
                id = w->queue_next;
                w = &thing[id];
                n++;
            }
        }
    }
    return 0;
}
