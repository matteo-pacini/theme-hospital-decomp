#include "hosp.h"

extern "C" {
extern Event player_events[];
s32 hilighted_object = -1;
extern char **object_names;
extern char **misc_text;
extern char **room_names;

void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 width);
}

void post_shop_readjust_build_palette(void);
u8 does_room_contain_compulsory_objects(RoomInfo *room);

// Edit room requester clicks on the enabled buttons (crequester.arg
// marks the disabled ones): cancel aborts building, one button sends
// EV_FURNISH_EDITED_ROOM, one switches to picking up objects (MM_PICK_UP_OBJECT,
// requester mode 8), and confirm commits the
// current step (blueprint, doors, or opens the room).
// FUNCTION: 0x0009f0c0
void req14(Window *win)
{
    WindowButton *button;
    s32 i;

    button = win->buttons;
    for (button = win->buttons, i = 1; button; button = button->next, i++) {
        if (!(button->flags & 6) || (crequester.arg & (1 << i)))
            continue;
        if (resvars.requester_buttons[0x4f] == i) {
            setup_abort_all_requesters();
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        } else if (resvars.requester_buttons[0x50] == i) {
            player_events[player].type = EV_FURNISH_EDITED_ROOM;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        } else if (resvars.requester_buttons[0x51] == i) {
            set_mouse_mode(MM_PICK_UP_OBJECT);
            set_edit_room_requester_mode(8);
            crequester.arg3 = 0;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        } else if (resvars.requester_buttons[0x52] == i) {
            switch (crequester.mode) {
            case 0:
                player_events[player].type = EV_COMMIT_BLUEPRINT;
                set_mouse_mode(MM_NORMAL);
                break;
            case 4:
                player_events[player].type = EV_COMMIT_DOORS;
                player_events[player].value = crequester.kind;
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                break;
            case 6:
            case 8:
                player_events[player].type = EV_OPEN_ROOM;
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
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
// name, the step's instructions (misc_text[othervar.edit_room_hint]) and, while
// placing objects, the list of objects still to place; clicking one selects
// it for placing (crequester.arg3). Sets the window's height to fit.
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
    bx = ox + resvars.coords[0x6e].x;
    hover = 0;
    if (advisor.flags & 1) {
        i = 1;
        othervar.flash_counter++;
        if (othervar.flash_counter % 10 == 0) {
            if (advisor.tutorial_highlight == resvars.requester_buttons[0x4f]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x4f])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x18]]);
            } else if (advisor.tutorial_highlight == resvars.requester_buttons[0x50]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x50])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x19]]);
            } else if (advisor.tutorial_highlight == resvars.requester_buttons[0x51]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x51])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x1a]]);
            } else if (advisor.tutorial_highlight == resvars.requester_buttons[0x52]) {
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x52])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x1b]]);
            }
            othervar.flash_counter = 0;
        }
    }
    compute_help_text(ox, oy, 0xd);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    for (button = win->buttons, j = 1; button; button = button->next, j++) {
        off_sprite = 0;
        on_sprite = 0;
        if (!((crequester.arg & (1 << j)) || (crequester.arg2 & (1 << j))))
            continue;
        if (resvars.requester_buttons[0x4f] == j) {
            on_sprite = resvars.requester_sprites[0x14];
            off_sprite = resvars.requester_sprites[0x18];
        } else if (resvars.requester_buttons[0x50] == j) {
            on_sprite = resvars.requester_sprites[0x15];
            off_sprite = resvars.requester_sprites[0x19];
        } else if (resvars.requester_buttons[0x51] == j) {
            on_sprite = resvars.requester_sprites[0x16];
            off_sprite = resvars.requester_sprites[0x1a];
        } else if (resvars.requester_buttons[0x52] == j) {
            on_sprite = resvars.requester_sprites[0x17];
            off_sprite = resvars.requester_sprites[0x1b];
        }
        if (crequester.arg & (1 << j))
            reqspr_draw(button->x + ox, button->y + oy, &button_sprite[on_sprite]);
        else
            reqspr_draw(button->x + ox, button->y + oy, &button_sprite[off_sprite]);
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetJustifyWindow(ox + resvars.boxes[0x25].x, oy + resvars.boxes[0x25].y, resvars.boxes[0x25].w);
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, 0, room_names[rooms[crequester.kind].id]);
    if (resvars.screen_mode == SCREEN_MODE_13) {
        lbDisplay.draw_flags = 0x20;
        LbTextSetJustifyWindow(ox + resvars.boxes[0x26].x, oy + resvars.boxes[0x26].y, resvars.boxes[0x26].w);
        LbTextDraw(0, 0, misc_text[othervar.edit_room_hint]);
        lbDisplay.draw_flags = 0;
    }
    top = oy + resvars.coords[0x6e].y;
    by = top;
    if (crequester.mode >= 6) {
        for (obj = 0; obj < 0x3e && rows < (resvars.game_mode == SCREEN_MODE_13 ? 8 : 6); obj++) {
            count = gamevar.buy_count[obj];
            if (count) {
                reqspr_draw(bx, by, &button_sprite[resvars.requester_sprites[0x1d]]);
                if ((ox + resvars.boxes[0x27].x <= lbDisplay.mmouse_x &&
                     ox + resvars.boxes[0x27].x + resvars.boxes[0x27].w > lbDisplay.mmouse_x) &&
                    by + resvars.boxes[0x29].y <= lbDisplay.mmouse_y &&
                    by + resvars.boxes[0x27].y + resvars.boxes[0x27].h > lbDisplay.mmouse_y) {
                    if (hilighted_object != obj)
                        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                    hilighted_object = obj;
                    hover = 1;
                    if (lbDisplay.mleft_button) {
                        if (crequester.arg3 != obj)
                            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                        crequester.arg3 = obj;
                        init_place_object(obj, gamevar.place_mode, gamevar.place_room, 0);
                        set_edit_room_requester_mode(6);
                    }
                }
                by += resvars.coords[0x6f].y;
                rows++;
            }
        }
        if (!hover)
            hilighted_object = -1;
        if (rows) {
            reqspr_draw(bx, by + resvars.coords[0x71].y, &button_sprite[resvars.requester_sprites[0x1e]]);
            reqspr_draw(bx, oy + resvars.coords[0x70].y, &button_sprite[resvars.requester_sprites[0x1c]]);
        }
        gamevar.buy_kind_count = rows;
        lbDisplay.draw_flags = 0x100;
        by = top;
        rows = 0;
        for (obj = 0; obj < 0x3e && rows < (resvars.game_mode == SCREEN_MODE_13 ? 8 : 6); obj++) {
            count = gamevar.buy_count[obj];
            if (count) {
                if (crequester.arg3 == obj) {
                    lbDisplay.draw_flags |= 0x10;
                    LbDrawBox(ox + resvars.boxes[0x27].x, by + resvars.boxes[0x27].y, resvars.boxes[0x27].w,
                              resvars.boxes[0x27].h, color.red);
                    lbDisplay.draw_flags ^= 0x10;
                }
                if (obj == hilighted_object)
                    set_font_from_cache(2);
                else
                    set_font_from_cache(1);
                LbTextSetClipWindow(ox + resvars.boxes[0x27].x, by + resvars.boxes[0x27].y,
                                    resvars.boxes[0x29].w + resvars.boxes[0x28].w + 0x14, resvars.boxes[0x27].h);
                LbTextSetJustifyWindow(ox + resvars.boxes[0x28].x, by + resvars.boxes[0x28].y, resvars.boxes[0x28].w);
                LbTextDraw(0, 0, object_names[obj]);
                LbTextSetJustifyWindow(ox + resvars.boxes[0x29].x, by + resvars.boxes[0x29].y, resvars.boxes[0x29].w);
                sprintf(buf, "%d", count);
                LbTextDraw(0, 0, buf);
                by += resvars.coords[0x6f].y;
                rows++;
            }
        }
        lbDisplay.draw_flags = 0;
    }
    win->height = by;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, win->height + oy, 3);
}

// Moves the edit room requester to step `mode` (0 blueprint, 2 placing the
// door, 4 windows once the door is placed, 6 placing objects, 8 picking objects up, 0xa corridor objects), setting
// which buttons are disabled and the instructions, and opens it if needed.
// In modes 6 and 8, confirm is only enabled once the room has its required
// objects.
// FUNCTION: 0x0009fcbc
void set_edit_room_requester_mode(const u8 mode)
{
    crequester.mode = mode;
    crequester.arg2 = 0;
    switch (mode) {
    case 0:
        crequester.arg = (1 << resvars.requester_buttons[0x50]) | (1 << resvars.requester_buttons[0x51]);
        othervar.edit_room_hint = 0xa;
        break;
    case 2:
        crequester.arg = (1 << resvars.requester_buttons[0x50]) | (1 << resvars.requester_buttons[0x51]) |
                         (1 << resvars.requester_buttons[0x52]);
        othervar.edit_room_hint = 0xb;
        break;
    case 4:
        crequester.arg = (1 << resvars.requester_buttons[0x50]) | (1 << resvars.requester_buttons[0x51]);
        othervar.edit_room_hint = 0xc;
        break;
    case 8:
        crequester.arg2 = 1 << resvars.requester_buttons[0x51];
    case 6:
        if (crequester.kind == 3 || room_info[allhosps[player].edited_room].first_object == 0)
            crequester.arg |= 1 << resvars.requester_buttons[0x51];
        if (crequester.kind != 3) {
            if (does_room_contain_compulsory_objects(&room_info[allhosps[player].edited_room])) {
                crequester.arg = 0;
                othervar.edit_room_hint = 0xe;
                advisor.flags |= 0x10;
            } else {
                crequester.arg = 1 << resvars.requester_buttons[0x52];
                othervar.edit_room_hint = 0xd;
                advisor.flags &= ~0x10;
            }
        }
        break;
    case 0xa:
        crequester.arg = (1 << resvars.requester_buttons[0x52]) | (1 << resvars.requester_buttons[0x50]) |
                         (1 << resvars.requester_buttons[0x51]);
        othervar.edit_room_hint = 0x10;
        crequester.kind = 3;
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
        othervar.edit_room_hint = 0xf;
    if (crequester.id != REQ_EDIT_ROOM)
        open_edit_room_requester();
}

// Opens the edit room requester (REQ_EDIT_ROOM) if no requester is
// open.
// FUNCTION: 0x0009ff68
void open_edit_room_requester(void)
{
    if (crequester.id == REQ_NONE) {
        close_irequesters();
        crequester.id = REQ_EDIT_ROOM;
        open_requester(crequester.id, 1);
    }
}

// Returns from the buy objects requester to placing: restores the shopping
// list saved in othervar.saved_build_palette (restore_purchases set) or re-adds the
// required objects taken off it, then places the next object to build or
// shows the place-object requester.
// FUNCTION: 0x0009ffa8
void go_back_to_place_object_requester(u8 restore_purchases)
{
    u8 obj;

    close_irequesters();
    close_crequester(REQ_BUY_OBJECTS);
    open_edit_room_requester();
    if (restore_purchases)
        gamevar.buy_list = othervar.saved_build_palette;
    else
        post_shop_readjust_build_palette();
    obj = next_object_to_build();
    if (obj) {
        init_place_object(obj, 0, allhosps[othervar.cur_player].edited_room, 0);
    } else {
        set_mouse_mode(MM_NORMAL);
        setup_place_object_requester();
    }
}
