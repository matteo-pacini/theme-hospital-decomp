#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern char **D_001602cc;
extern char **D_001602d4;
extern char **D_001602e0;
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
                if (i == resvars.fe[0x17]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 3;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x18]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 4;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x19]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 5;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x1a]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 6;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x25]) {
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
                if (i == resvars.fe[0x17]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 3;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x18]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 4;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x19]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 5;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x1a]) {
                    D_001549d0[player].type = 0x20;
                    D_001549d0[player].value = 6;
                    play_sound(10, 0x43, -1, -1);
                } else if (i == resvars.fe[0x1b] || i == resvars.fe[0x1c] || i == resvars.fe[0x1d]
                    || i == resvars.fe[0x1e] || i == resvars.fe[0x1f] || i == resvars.fe[0x20]
                    || i == resvars.fe[0x21] || i == resvars.fe[0x22] || i == resvars.fe[0x23]
                    || i == resvars.fe[0x24]) {
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
                } else if (i == resvars.fe[0x25]) {
                    D_001549d0[player].type = 0x26;
                    play_sound(10, 0x42, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

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
    if (resvars.f1 == SCREEN_MODE_13 && loaded_requester != 9)
        return;
    if (resvars.f1 == SCREEN_MODE_13)
        compute_help_text(win->x, win->y, 2);
    else
        compute_help_text(0, 0, 2);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
        window.unknown2cf - window.unknown2c7);
    ox = win->x - lbDisplay.graphics_window_x;
    oy = win->y - lbDisplay.graphics_window_y;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (advisor.f11f & 1) {
        i = 1;
        othervar.f9bc++;
        if (othervar.f9bc % 10 == 0) {
            switch (advisor.f120) {
            case 8:
                for (b = win->buttons; b; b = b->next) {
                    if (i == resvars.fe[0x17])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + b->x, (s16)oy + b->y, &button_sprite[resvars.f12[0x2f]]);
                break;
            case 0xb:
                for (b = win->buttons; b; b = b->next) {
                    if (i == resvars.fe[0x1b])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + b->x, (s16)oy + b->y, &button_sprite[resvars.f12[0x30]]);
                break;
            }
            othervar.f9bc = 0;
        }
    }
    switch (crequester.unknown0) {
    case 1:
        LbTextDraw(ox + resvars.f1c[0x16].x, oy + resvars.f1c[0x16].y, D_001602d4[1]);
        break;
    case 4:
        LbTextDraw(ox + resvars.f1c[0x16].x, oy + resvars.f1c[0x16].y, D_001602d4[2]);
        if (croom) {
            obj = rooms[croom].fa;
            fr = &frame[start[objects[obj].f10].frame];
            sx = 0x790000 / fr->width;
            sy = 0x790000 / fr->length;
            if (sy >= 0x10000 && sx >= 0x10000) {
                draw_a_frame(start[objects[obj].f10].frame, ox + resvars.f1c[0x18].x, oy + resvars.f1c[0x18].y, 0,
                    start[objects[obj].f10].flags);
            } else if (sy <= sx) {
                draw_a_frame_scaled(start[objects[obj].f10].frame, ox + resvars.f1c[0x18].x,
                    oy + resvars.f1c[0x18].y, 0, start[objects[obj].f10].flags, sy >> 8 << 8);
            } else {
                draw_a_frame_scaled(start[objects[obj].f10].frame, ox + resvars.f1c[0x18].x,
                    oy + resvars.f1c[0x18].y, 0, start[objects[obj].f10].flags, sx >> 8 << 8);
            }
        }
        for (i = 0; i < 10; i++) {
            room = req_indices[i];
            set_font_from_cache(1);
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.f18[i + 1].x, oy + resvars.f18[i + 1].y,
                    resvars.f18[i + 1].w, resvars.f18[i + 1].h)) {
                if (crequester.unknown8 != room)
                    play_sound(10, 0x44, -1, -1);
                crequester.unknown8 = room;
                set_font_from_cache(2);
            }
            if (room)
                LbTextDraw(ox + resvars.f1c[i + 0x2a].x, oy + resvars.f1c[i + 0x2a].y, D_001602cc[rooms[room].id]);
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
            sprintf(buf, "%s%d", D_001602d4[4], rooms[croom].cost + total);
            LbTextDraw(ox + resvars.f1c[0x17].x, oy + resvars.f1c[0x17].y, buf);
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
            LbTextDraw(ox + resvars.f1c[row + 0x26].x, oy + resvars.f1c[row + 0x26].y, D_001602e0[i]);
            row++;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// FUNCTION: 0x0009c690
void req10_help_room(void)
{
    u8 idx;

    idx = req_indices[resvars.f2c_HelpHotspotp[othervar.f8ac].unknownA];
    if (idx)
        fn_000bbff0(othervar.f96, room_help[idx]);
}
