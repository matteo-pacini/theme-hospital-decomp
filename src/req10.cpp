#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern char **room_names;
extern char **req1_help;
extern char **department_names;
}

// Build room requester clicks (crequester.mode 1 departments, 4 rooms):
// the department buttons list that department's rooms (EV_REQ1_RESET_ROOMS
// 3-6), a room button starts building the hovered room type if affordable
// (event EV_INIT_BLUEPRINT), and the close button sends event EV_CLOSE_REQUESTERS.
// FUNCTION: 0x0009b870
void req10(Window *win)
{
    u16 i;
    s32 room;
    s32 total;
    u32 k;
    WindowButton *button;

    button = win->buttons;
    switch (crequester.mode) {
    case 1:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.last_action = i;
                if (i == resvars.requester_buttons[0x17]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 3;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x18]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 4;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x19]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 5;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x1a]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 6;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x25]) {
                    D_001549d0[player].type = EV_CLOSE_REQUESTERS;
                    play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                }
            }
            i++;
        }
        break;
    case 4:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.last_action = i;
                if (i == resvars.requester_buttons[0x17]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 3;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x18]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 4;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x19]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 5;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x1a]) {
                    D_001549d0[player].type = EV_REQ1_RESET_ROOMS;
                    D_001549d0[player].value = 6;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                } else if (i == resvars.requester_buttons[0x1b] || i == resvars.requester_buttons[0x1c] || i == resvars.requester_buttons[0x1d]
                    || i == resvars.requester_buttons[0x1e] || i == resvars.requester_buttons[0x1f] || i == resvars.requester_buttons[0x20]
                    || i == resvars.requester_buttons[0x21] || i == resvars.requester_buttons[0x22] || i == resvars.requester_buttons[0x23]
                    || i == resvars.requester_buttons[0x24]) {
                    if (crequester.kind) {
                        total = 0;
                        room = crequester.kind;
                        for (k = 0; k < rooms[room].compulsory_count; k++) {
                            u8 obj;

                            obj = rooms[room].objects[k];
                            total += allhosps[player].object_stats[obj].cost;
                        }
                        if (rooms[room].cost + total < allhosps[player].balance) {
                            D_001549d0[player].type = EV_INIT_BLUEPRINT;
                            D_001549d0[player].value = crequester.kind;
                            D_001549d0[player].arg = crequester.kind;
                            play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                        } else {
                            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                        }
                    }
                    play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                } else if (i == resvars.requester_buttons[0x25]) {
                    D_001549d0[player].type = EV_CLOSE_REQUESTERS;
                    play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

// Draws the build room requester: the departments, the department's rooms
// with the hovered one selected (crequester.kind), its picture and its
// cost with its required objects, and the advisor's flashing hints.
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
    WindowButton *button;
    CompFrame *fr;
    u32 scale_x;
    u32 scale_y;

    button = win->buttons;
    croom = crequester.kind;
    obj = crequester.item;
    if (resvars.game_mode == SCREEN_MODE_13 && loaded_requester != 9)
        return;
    if (resvars.game_mode == SCREEN_MODE_13)
        compute_help_text(win->x, win->y, 2);
    else
        compute_help_text(0, 0, 2);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
        window.clip_y1 - window.clip_y0);
    ox = win->x - lbDisplay.graphics_window_x;
    oy = win->y - lbDisplay.graphics_window_y;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (advisor.flags & 1) {
        i = 1;
        othervar.flash_counter++;
        if (othervar.flash_counter % 10 == 0) {
            switch (advisor.tutorial_highlight) {
            case 8:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x17])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x2f]]);
                break;
            case 0xb:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x1b])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x30]]);
                break;
            }
            othervar.flash_counter = 0;
        }
    }
    switch (crequester.mode) {
    case 1:
        LbTextDraw(ox + resvars.coords[0x16].x, oy + resvars.coords[0x16].y, req1_help[1]);
        break;
    case 4:
        LbTextDraw(ox + resvars.coords[0x16].x, oy + resvars.coords[0x16].y, req1_help[2]);
        if (croom) {
            obj = rooms[croom].main_object;
            fr = &frame[start[objects[obj].preview_anim].frame];
            scale_x = 0x790000 / fr->width;
            scale_y = 0x790000 / fr->length;
            if (scale_y >= 0x10000 && scale_x >= 0x10000) {
                draw_a_frame(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x18].x, oy + resvars.coords[0x18].y, 0,
                    start[objects[obj].preview_anim].flags);
            } else if (scale_y <= scale_x) {
                draw_a_frame_scaled(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x18].x,
                    oy + resvars.coords[0x18].y, 0, start[objects[obj].preview_anim].flags, scale_y >> 8 << 8);
            } else {
                draw_a_frame_scaled(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x18].x,
                    oy + resvars.coords[0x18].y, 0, start[objects[obj].preview_anim].flags, scale_x >> 8 << 8);
            }
        }
        for (i = 0; i < 10; i++) {
            room = req_indices[i];
            set_font_from_cache(1);
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes[i + 1].x, oy + resvars.boxes[i + 1].y,
                    resvars.boxes[i + 1].w, resvars.boxes[i + 1].h)) {
                if (crequester.kind != room)
                    play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                crequester.kind = room;
                set_font_from_cache(2);
            }
            if (room)
                LbTextDraw(ox + resvars.coords[i + 0x2a].x, oy + resvars.coords[i + 0x2a].y, room_names[rooms[room].id]);
        }
        total = 0;
        for (k = 0; k < rooms[croom].compulsory_count; k++) {
            u8 obj_kind;

            obj_kind = rooms[croom].objects[k];
            total += allhosps[player].object_stats[obj_kind].cost;
        }
        {
            char buf[100];

            set_font_from_cache(1);
            sprintf(buf, "%s%d", req1_help[4], rooms[croom].cost + total);
            LbTextDraw(ox + resvars.coords[0x17].x, oy + resvars.coords[0x17].y, buf);
        }
        break;
    }
    row = 0;
    for (i = 0; i < 7; i++) {
        if (departments[i].listed) {
            if (crequester.arg == i)
                set_font_from_cache(2);
            else
                set_font_from_cache(1);
            LbTextDraw(ox + resvars.coords[row + 0x26].x, oy + resvars.coords[row + 0x26].y, department_names[i]);
            row++;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// Help text for a room entry of the build room requester, into
// othervar.help_msg.
// FUNCTION: 0x0009c690
void req10_help_room(void)
{
    u8 idx;

    idx = req_indices[resvars.help_hotspots_HelpHotspotp[othervar.help_hotspot].req_slot];
    if (idx)
        fn_000bbff0(othervar.help_msg, room_help[idx]);
}
