#include "hosp.h"

struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern u8 *D_0015f402;
extern u16 *D_0015f406;
extern Box *D_0015f40c;
extern Point *D_0015f410;
extern HelpHotspot *D_0015f420;
extern Event D_001549d0[];
extern TbScreenMode D_0015f3f5;
extern u8 D_0015831b;
extern u16 D_0015831c;
extern u8 D_00155d74;
extern u16 D_00155c64;
extern char D_0015544e[];
extern u16 D_00188c74;
extern s32 D_00188c38;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern char **D_001602cc;
extern char **D_001602d4;
extern char **D_001602e0;
extern char D_00121a48[];
}

// FUNCTION: 0x0009b870
void req10(Window *win)
{
    u16 i;
    s32 room;
    s32 total;
    u32 k;
    WindowButton *b;

    b = win->buttons;
    switch (crequester.unknown0) {
    case 1:
        for (i = 1; b; b = b->next) {
            if (b->flags & 6) {
                debug.unknown8 = i;
                if (i == D_0015f402[0x17]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 3;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x18]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 4;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x19]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 5;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x1a]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 6;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x25]) {
                    D_001549d0[player].type = 0x26;
                    play_sound(10, 0x42, -1, -1);
                }
            }
            i++;
        }
        break;
    case 4:
        for (i = 1; b; b = b->next) {
            if (b->flags & 6) {
                debug.unknown8 = i;
                if (i == D_0015f402[0x17]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 3;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x18]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 4;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x19]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 5;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x1a]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 6;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == D_0015f402[0x1b] || i == D_0015f402[0x1c] || i == D_0015f402[0x1d]
                    || i == D_0015f402[0x1e] || i == D_0015f402[0x1f] || i == D_0015f402[0x20]
                    || i == D_0015f402[0x21] || i == D_0015f402[0x22] || i == D_0015f402[0x23]
                    || i == D_0015f402[0x24]) {
                    if (crequester.unknown8) {
                        total = 0;
                        room = crequester.unknown8;
                        for (k = 0; k < rooms[room].fc; k++) {
                            u8 obj;

                            obj = rooms[room].objects[k];
                            total += allhosps[player].object_stats[obj].cost;
                        }
                        if (rooms[room].cost + total < allhosps[player].f35) {
                            D_001549d0[player].type = 0x35;
                            D_001549d0[player].value = crequester.unknown8;
                            D_001549d0[player].arg = crequester.unknown8;
                            play_sound(10, 0x41, -1, -1);
                        } else {
                            play_sound(10, 8, -1, -1);
                        }
                    }
                    play_sound(10, 0x41, -1, -1);
                } else if (i == D_0015f402[0x25]) {
                    D_001549d0[player].type = 0x26;
                    play_sound(10, 0x42, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0009be80
void req10_overdraw(Window *win)
{
    u8 row;
    u8 k;
    u8 i;
    u16 obj;
    u16 croom;
    u16 room;
    s32 total;
    s32 oy;
    s32 ox;
    WindowButton *b;
    CompFrame *fr;
    u32 sx;
    u32 sy;

    b = win->buttons;
    croom = crequester.unknown8;
    obj = crequester.unknown3;
    if (D_0015f3f5 == SCREEN_MODE_13 && loaded_requester != 9)
        return;
    if (D_0015f3f5 == SCREEN_MODE_13)
        compute_help_text(win->x, win->y, 2);
    else
        compute_help_text(0, 0, 2);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
        window.unknown2cf - window.unknown2c7);
    ox = win->x - D_00188c3c;
    oy = win->y - D_00188c40;
    D_00188c74 = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    if (D_0015831b & 1) {
        i = 1;
        D_00155d74++;
        if (D_00155d74 % 10 == 0) {
            switch (D_0015831c) {
            case 8:
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x17])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x2f]]);
                break;
            case 0xb:
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x1b])
                        break;
                    i++;
                }
                reqspr_draw(b->x + (s16)ox, b->y + (s16)oy, &button_sprite[D_0015f406[0x30]]);
                break;
            }
            D_00155d74 = 0;
        }
    }
    switch (crequester.unknown0) {
    case 1:
        LbTextDraw(ox + D_0015f410[0x16].x, oy + D_0015f410[0x16].y, D_001602d4[1]);
        break;
    case 4:
        LbTextDraw(ox + D_0015f410[0x16].x, oy + D_0015f410[0x16].y, D_001602d4[2]);
        if (croom) {
            obj = rooms[croom].fa;
            fr = &frame[start[objects[obj].f10].frame];
            sx = 0x790000 / fr->width;
            sy = 0x790000 / fr->length;
            if (sy >= 0x10000 && sx >= 0x10000) {
                draw_a_frame(start[objects[obj].f10].frame, ox + D_0015f410[0x18].x, oy + D_0015f410[0x18].y, 0,
                    start[objects[obj].f10].flags);
            } else if (sy <= sx) {
                draw_a_frame_scaled(start[objects[obj].f10].frame, ox + D_0015f410[0x18].x,
                    oy + D_0015f410[0x18].y, 0, start[objects[obj].f10].flags, sy >> 8 << 8);
            } else {
                draw_a_frame_scaled(start[objects[obj].f10].frame, ox + D_0015f410[0x18].x,
                    oy + D_0015f410[0x18].y, 0, start[objects[obj].f10].flags, sx >> 8 << 8);
            }
        }
        for (i = 0; i < 10; i++) {
            room = req_indices[i];
            set_font_from_cache(1);
            if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c[i + 1].x, oy + D_0015f40c[i + 1].y,
                    D_0015f40c[i + 1].w, D_0015f40c[i + 1].h)) {
                if (crequester.unknown8 != room)
                    play_sound(10, 0x44, -1, -1);
                crequester.unknown8 = room;
                set_font_from_cache(2);
            }
            if (room)
                LbTextDraw(ox + D_0015f410[i + 0x2a].x, oy + D_0015f410[i + 0x2a].y, D_001602cc[rooms[room].id]);
        }
        total = 0;
        for (k = 0; k < rooms[croom].fc; k++) {
            u8 o;

            o = rooms[croom].objects[k];
            total += allhosps[player].object_stats[o].cost;
        }
        {
            char buf[100];

            set_font_from_cache(1);
            sprintf(buf, D_00121a48, D_001602d4[4], rooms[croom].cost + total);
            LbTextDraw(ox + D_0015f410[0x17].x, oy + D_0015f410[0x17].y, buf);
        }
        break;
    }
    row = 0;
    for (i = 0; i < 7; i++) {
        if (departments[i].unknown1) {
            if (crequester.unknown4 == i)
                set_font_from_cache(2);
            else
                set_font_from_cache(1);
            LbTextDraw(ox + D_0015f410[row + 0x26].x, oy + D_0015f410[row + 0x26].y, D_001602e0[i]);
            row++;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0);
}

// FUNCTION: 0x0009c690
void req10_help_room(void)
{
    u8 idx;

    idx = req_indices[D_0015f420[D_00155c64].unknownA];
    if (idx)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, room_help[idx]);
}
