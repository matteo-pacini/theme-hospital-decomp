#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
s32 D_00145328 = -1;
extern char **object_names;
extern char **misc_text;
extern char **room_names;

void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 w);
}

void post_shop_readjust_build_palette(void);
u8 does_room_contain_compulsory_objects(RoomInfo *room);

// Edit room requester clicks on the enabled buttons (crequester.unknown4
// marks the disabled ones): cancel aborts building, one button sends event
// 0x5b, one switches to picking up objects (mode 8), and confirm commits the
// current step (blueprint, doors, or opens the room).
// FUNCTION: 0x0009f0c0
void req14(Window *win)
{
    WindowButton *button;
    s32 i;

    button = win->buttons;
    for (button = win->buttons, i = 1; button; button = button->next, i++) {
        if (!(button->flags & 6) || (crequester.unknown4 & (1 << i)))
            continue;
        if (resvars.fe[0x4f] == i) {
            setup_abort_all_requesters();
            play_sound(10, SFX_NO4, -1, -1);
        } else if (resvars.fe[0x50] == i) {
            D_001549d0[player].type = 0x5b;
            play_sound(10, SFX_SELECTX, -1, -1);
        } else if (resvars.fe[0x51] == i) {
            set_mouse_mode(0xb);
            set_edit_room_requester_mode(8);
            crequester.unknown9 = 0;
            play_sound(10, SFX_SELECTX, -1, -1);
        } else if (resvars.fe[0x52] == i) {
            switch (crequester.unknown0) {
            case 0:
                D_001549d0[player].type = EV_COMMIT_BLUEPRINT;
                set_mouse_mode(MM_NORMAL);
                break;
            case 4:
                D_001549d0[player].type = EV_COMMIT_DOORS;
                D_001549d0[player].value = crequester.unknown8;
                play_sound(10, SFX_YESX, -1, -1);
                break;
            case 6:
            case 8:
                D_001549d0[player].type = EV_OPEN_ROOM;
                play_sound(10, SFX_YESX, -1, -1);
                break;
            case 1:
            case 2:
            case 3:
            case 5:
            case 7:
            default:
                break;
            }
        }
    }
}

// Draws the edit room requester: its buttons (lit or greyed), the room
// name, the step's instructions (misc_text[othervar.f9c0]) and, while
// placing objects, the list of objects still to place; clicking one selects
// it for placing (crequester.unknown9). Sets the window's height to fit.
// FUNCTION: 0x0009f2e0
void req14_overdraw(Window *win)
{
    u8 i;
    u8 hover;
    s32 off_sprite;
    WindowButton *button;
    s32 ox;
    s32 oy;
    s32 by;
    s32 rows;
    s32 on_sprite;
    s32 j;
    s32 bx;
    u32 obj;
    s32 count;
    s32 top;
    char buf[8];

    ox = win->x;
    oy = win->y;
    rows = 0;
    bx = ox + resvars.f1c[0x6e].x;
    hover = 0;
    if (advisor.f11f & 1) {
        i = 1;
        othervar.f9bc++;
        if (othervar.f9bc % 10 == 0) {
            if (advisor.f120 == resvars.fe[0x4f]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x4f])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x18]]);
            } else if (advisor.f120 == resvars.fe[0x50]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x50])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x19]]);
            } else if (advisor.f120 == resvars.fe[0x51]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x51])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x1a]]);
            } else if (advisor.f120 == resvars.fe[0x52]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x52])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x1b]]);
            }
            othervar.f9bc = 0;
        }
    }
    compute_help_text(ox, oy, 0xd);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    for (button = win->buttons, j = 1; button; button = button->next, j++) {
        off_sprite = 0;
        on_sprite = 0;
        if (!((crequester.unknown4 & (1 << j)) || (crequester.unknown6 & (1 << j))))
            continue;
        if (resvars.fe[0x4f] == j) {
            on_sprite = resvars.f12[0x14];
            off_sprite = resvars.f12[0x18];
        } else if (resvars.fe[0x50] == j) {
            on_sprite = resvars.f12[0x15];
            off_sprite = resvars.f12[0x19];
        } else if (resvars.fe[0x51] == j) {
            on_sprite = resvars.f12[0x16];
            off_sprite = resvars.f12[0x1a];
        } else if (resvars.fe[0x52] == j) {
            on_sprite = resvars.f12[0x17];
            off_sprite = resvars.f12[0x1b];
        }
        if (crequester.unknown4 & (1 << j))
            reqspr_draw(button->x + ox, button->y + oy, &button_sprite[on_sprite]);
        else
            reqspr_draw(button->x + ox, button->y + oy, &button_sprite[off_sprite]);
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetJustifyWindow(ox + resvars.f18[0x25].x, oy + resvars.f18[0x25].y, resvars.f18[0x25].w);
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, 0, room_names[rooms[crequester.unknown8].id]);
    if (resvars.f0 == SCREEN_MODE_13) {
        lbDisplay.draw_flags = 0x20;
        LbTextSetJustifyWindow(ox + resvars.f18[0x26].x, oy + resvars.f18[0x26].y, resvars.f18[0x26].w);
        LbTextDraw(0, 0, misc_text[othervar.f9c0]);
        lbDisplay.draw_flags = 0;
    }
    top = oy + resvars.f1c[0x6e].y;
    by = top;
    if (crequester.unknown0 >= 6) {
        for (obj = 0; obj < 0x3e && rows < (resvars.f1 == SCREEN_MODE_13 ? 8 : 6); obj++) {
            count = gamevar.buy_count[obj];
            if (count) {
                reqspr_draw(bx, by, &button_sprite[resvars.f12[0x1d]]);
                if ((ox + resvars.f18[0x27].x <= lbDisplay.mmouse_x && ox + resvars.f18[0x27].x + resvars.f18[0x27].w > lbDisplay.mmouse_x)
                    && by + resvars.f18[0x29].y <= lbDisplay.mmouse_y
                    && by + resvars.f18[0x27].y + resvars.f18[0x27].h > lbDisplay.mmouse_y) {
                    if (D_00145328 != obj)
                        play_sound(10, SFX_HLIGHT5, -1, -1);
                    D_00145328 = obj;
                    hover = 1;
                    if (lbDisplay.mleft_button) {
                        if (crequester.unknown9 != obj)
                            play_sound(10, SFX_SELECTX, -1, -1);
                        crequester.unknown9 = obj;
                        init_place_object(obj, gamevar.unknown26, gamevar.unknown2f, 0);
                        set_edit_room_requester_mode(6);
                    }
                }
                by += resvars.f1c[0x6f].y;
                rows++;
            }
        }
        if (!hover)
            D_00145328 = -1;
        if (rows) {
            reqspr_draw(bx, by + resvars.f1c[0x71].y, &button_sprite[resvars.f12[0x1e]]);
            reqspr_draw(bx, oy + resvars.f1c[0x70].y, &button_sprite[resvars.f12[0x1c]]);
        }
        gamevar.unknown71 = rows;
        lbDisplay.draw_flags = 0x100;
        by = top;
        rows = 0;
        for (obj = 0; obj < 0x3e && rows < (resvars.f1 == SCREEN_MODE_13 ? 8 : 6); obj++) {
            count = gamevar.buy_count[obj];
            if (count) {
                if (crequester.unknown9 == obj) {
                    lbDisplay.draw_flags |= 0x10;
                    LbDrawBox(ox + resvars.f18[0x27].x, by + resvars.f18[0x27].y, resvars.f18[0x27].w, resvars.f18[0x27].h,
                        color.unknownD);
                    lbDisplay.draw_flags ^= 0x10;
                }
                if (obj == D_00145328)
                    set_font_from_cache(2);
                else
                    set_font_from_cache(1);
                LbTextSetClipWindow(ox + resvars.f18[0x27].x, by + resvars.f18[0x27].y,
                    resvars.f18[0x29].w + resvars.f18[0x28].w + 0x14, resvars.f18[0x27].h);
                LbTextSetJustifyWindow(ox + resvars.f18[0x28].x, by + resvars.f18[0x28].y, resvars.f18[0x28].w);
                LbTextDraw(0, 0, object_names[obj]);
                LbTextSetJustifyWindow(ox + resvars.f18[0x29].x, by + resvars.f18[0x29].y, resvars.f18[0x29].w);
                sprintf(buf, "%d", count);
                LbTextDraw(0, 0, buf);
                by += resvars.f1c[0x6f].y;
                rows++;
            }
        }
        lbDisplay.draw_flags = 0;
    }
    win->height = by;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, win->height + oy, 3);
}

// Moves the edit room requester to step `mode` (0 and 2 blueprint, 4 doors,
// 6 placing objects, 8 picking objects up, 0xa corridor objects), setting
// which buttons are disabled and the instructions, and opens it if needed.
// In modes 6 and 8, confirm is only enabled once the room has its required
// objects.
// FUNCTION: 0x0009fcbc
void set_edit_room_requester_mode(const u8 mode)
{
    crequester.unknown0 = mode;
    crequester.unknown6 = 0;
    switch (mode) {
    case 0:
        crequester.unknown4 = (1 << resvars.fe[0x50]) | (1 << resvars.fe[0x51]);
        othervar.f9c0 = 0xa;
        break;
    case 2:
        crequester.unknown4 = (1 << resvars.fe[0x50]) | (1 << resvars.fe[0x51]) | (1 << resvars.fe[0x52]);
        othervar.f9c0 = 0xb;
        break;
    case 4:
        crequester.unknown4 = (1 << resvars.fe[0x50]) | (1 << resvars.fe[0x51]);
        othervar.f9c0 = 0xc;
        break;
    case 8:
        crequester.unknown6 = 1 << resvars.fe[0x51];
    case 6:
        if (crequester.unknown8 == 3 || room_info[allhosps[player].unknown11b7].first_object == 0)
            crequester.unknown4 |= 1 << resvars.fe[0x51];
        if (crequester.unknown8 != 3) {
            if (does_room_contain_compulsory_objects(&room_info[allhosps[player].unknown11b7])) {
                crequester.unknown4 = 0;
                othervar.f9c0 = 0xe;
                advisor.f11f |= 0x10;
            } else {
                crequester.unknown4 = 1 << resvars.fe[0x52];
                othervar.f9c0 = 0xd;
                advisor.f11f &= ~0x10;
            }
        }
        break;
    case 0xa:
        crequester.unknown4 = (1 << resvars.fe[0x52]) | (1 << resvars.fe[0x50]) | (1 << resvars.fe[0x51]);
        othervar.f9c0 = 0x10;
        crequester.unknown8 = 3;
        break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    default:
        break;
    }
    if (mode == 8)
        othervar.f9c0 = 0xf;
    if (crequester.unknown2 != 5)
        open_edit_room_requester();
}

// Opens the edit room requester (crequester.unknown2 5) if no requester is
// open.
// FUNCTION: 0x0009ff68
void open_edit_room_requester(void)
{
    if (crequester.unknown2 == 0x63) {
        close_irequesters();
        crequester.unknown2 = 5;
        open_requester(crequester.unknown2, 1);
    }
}

// Returns from the buy objects requester to placing: restores the shopping
// list saved in othervar.f8b7 (restore_purchases set) or re-adds the
// required objects taken off it, then places the next object to build or
// shows the place-object requester.
// FUNCTION: 0x0009ffa8
void go_back_to_place_object_requester(u8 restore_purchases)
{
    u8 obj;

    close_irequesters();
    close_crequester(10);
    open_edit_room_requester();
    if (restore_purchases)
        gamevar.unknown33 = othervar.f8b7;
    else
        post_shop_readjust_build_palette();
    obj = next_object_to_build();
    if (obj) {
        init_place_object(obj, 0, allhosps[othervar.ff].unknown11b7, 0);
    } else {
        set_mouse_mode(MM_NORMAL);
        setup_place_object_requester();
    }
}
