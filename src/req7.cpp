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

/*
 * The queue window's entries of the in-game position table, read through
 * resvars.coords.
 */
struct QueuePanel {
    u8 unknown0[0x48];
    // 0x48: X of the queue's patients, relative to the window.
    s32 queue_x;
    // 0x4c: Their y.
    s32 queue_y;
    // 0x50: Width the patients are spread over.
    s32 queue_w;
    u8 unknown54[0x364];
    // 0x3b8: Positions of the window's labels.
    Point labels[8];
    u8 unknown3f8[0x1a8];
    // 0x5a0: Patients after the drop position shift right by the screen width
    // over this.
    s32 drop_gap_divisor;
};

/*
 * The queue window's button numbers in the requester button table
 * (resvars.requester_buttons).
 */
struct QueueButtons {
    u8 unknown0[0xa];
    // 0xa: Raises the queue limit.
    u8 capacity_up_button;
    // 0xb: Lowers the queue limit.
    u8 capacity_down_button;
    // 0xc: Closes the window.
    u8 close_button;
};

extern "C" {
extern char **queue_text;
extern Event D_001549d0[];
extern s16 frame_bounds[8];

void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
void draw_glow_frame(Thing *w, s16 x, s16 y);
void draw_wibble_frame(Thing *w, s16 x, s16 y);
u32 nth_active_player(u32 nth);
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

// Queue window clicks (room othervar.queue_win_room): raise (event EV_QUEUE_CAPACITY_UP, up to 30) or
// lower (0x54) the queue's limit (queue_capacity), and close (event EV_CLOSE_QUEUE_WINDOW).
// FUNCTION: 0x000957f0
void req7(Window *win)
{
    u16 button_no;
    WindowButton *button;

    for (button = win->buttons, button_no = 1; button != 0; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = button_no;
            if (button_no == resvars.requester_buttons_QueueButtonsp->capacity_down_button) {
                D_001549d0[player].type = EV_QUEUE_CAPACITY_DOWN;
                D_001549d0[player].value = othervar.queue_win_room;
                if (room_info[D_001549d0[player].value].queue_capacity)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                else
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            } else if (button_no == resvars.requester_buttons_QueueButtonsp->capacity_up_button) {
                D_001549d0[player].type = EV_QUEUE_CAPACITY_UP;
                D_001549d0[player].value = othervar.queue_win_room;
                if (room_info[D_001549d0[player].value].queue_capacity < 30)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                else
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            } else if (button_no == resvars.requester_buttons_QueueButtonsp->close_button) {
                D_001549d0[player].type = EV_CLOSE_QUEUE_WINDOW;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
        }
        button_no++;
    }
}

// Draws the queue window for room othervar.queue_win_room (queue length, people on the
// way, admitted, limit) with its patients, and handles dragging them:
// dropping a patient picked up with the left button moves them to the
// front, back or another room's queue; the right button's menu sends them
// home, to reception or to another player.
// FUNCTION: 0x000959c8
void req7_overdraw(Window *win)
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

    wx = window.scroll_x[window.current_window];
    wy = window.scroll_y[window.current_window];
    unusedc = 0;
    unused8 = 0;
    unused10 = 0;
    compute_help_text(win->x, win->y, 7);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    ox = win->x - lbDisplay.graphics_window_x;
    oy = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[0].x, oy + resvars.coords_QueuePanelp->labels[0].y,
               queue_text[0]);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[1].x, oy + resvars.coords_QueuePanelp->labels[1].y,
               queue_text[1]);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[2].x, oy + resvars.coords_QueuePanelp->labels[2].y,
               queue_text[2]);
    if (resvars.game_mode == SCREEN_MODE_1)
        LbTextSetClipWindow(ox + resvars.coords_QueuePanelp->labels[3].x, oy + resvars.coords_QueuePanelp->labels[3].y,
                            0x1c, 9);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[3].x, oy + resvars.coords_QueuePanelp->labels[3].y,
               queue_text[3]);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    sprintf(buf, "%d", queue_size(&room_info[othervar.queue_win_room]));
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[4].x, oy + resvars.coords_QueuePanelp->labels[4].y, buf);
    sprintf(buf, "%d", room_info[othervar.queue_win_room].queue_incoming);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[5].x, oy + resvars.coords_QueuePanelp->labels[5].y, buf);
    sprintf(buf, "%d", room_info[othervar.queue_win_room].count);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[6].x, oy + resvars.coords_QueuePanelp->labels[6].y, buf);
    sprintf(buf, "%d", room_info[othervar.queue_win_room].queue_capacity);
    LbTextDraw(ox + resvars.coords_QueuePanelp->labels[7].x, oy + resvars.coords_QueuePanelp->labels[7].y, buf);
    if (!queue_person_valid(othervar.queue_win_room, othervar.queue_pos) && othervar.queue_drag != 0) {
        othervar.queue_pos = 0;
        othervar.queue_hilight_thing = 0;
    } else {
        othervar.queue_hilight_thing = get_queue_person(othervar.queue_win_room, othervar.queue_pos);
    }
    if (lbDisplay.mleft_button) {
        display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
        display_queue_person(lbDisplay.mmouse_x, lbDisplay.mmouse_y, othervar.queue_win_room, othervar.queue_pos);
        othervar.queue_drag = 1;
    } else if (lbDisplay.mright_button) {
        display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
        if (othervar.queue_pos) {
            qx = queue_person_x(ox, oy, othervar.queue_win_room, othervar.queue_pos);
            qy = oy;
            display_queue_bar(qx, qy);
            othervar.queue_drag = 2;
            compute_help_text(qx - resvars.boxes_QueueLayoutp->r[4].x, qy + resvars.boxes_QueueLayoutp->r[4].y, 0x12);
        } else {
            othervar.queue_drag = 0;
        }
    }
    if (othervar.queue_drag && othervar.queue_pos) {
        if (!lbDisplay.mright_button) {
            if (MouseOnWindow(open_windows[0])) {
                set_mouse_mode(MM_NORMAL);
            } else {
                RoomInfo *room_under_mouse;

                room_under_mouse = which_room(othervar.mouse_cell);
                set_mouse_mode(MM_MOVE_QUEUE_PERSON);
                gamevar.queue_selected_person = get_real_person_from_room(othervar.queue_win_room, othervar.queue_pos);
                if (room_under_mouse && room_under_mouse->type == room_info[othervar.queue_win_room].type)
                    compute_room_pointed(othervar.mouse_cell);
            }
        }
    } else if (lbDisplay.mleft_button && !othervar.left_button_was_down) {
        RoomInfo *room;

        room = &room_info[othervar.queue_win_room];
        if (room->type > 6) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[25].x,
                             oy + resvars.boxes_QueueLayoutp->r[25].y, resvars.boxes_QueueLayoutp->r[25].w,
                             resvars.boxes_QueueLayoutp->r[25].h) ||
                over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[26].x,
                             oy + resvars.boxes_QueueLayoutp->r[26].y, resvars.boxes_QueueLayoutp->r[26].w,
                             resvars.boxes_QueueLayoutp->r[26].h))
                goto_column_row_in_window(0, room->door_tile % 128, room->door_tile / 128);
        }
    }
    if (lbDisplay.mleft_button + lbDisplay.mright_button == 0) {
        switch (othervar.queue_drag) {
        case 0:
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[1].x,
                             oy + resvars.boxes_QueueLayoutp->r[1].y, resvars.boxes_QueueLayoutp->r[1].w,
                             resvars.boxes_QueueLayoutp->r[1].h)) {
                i = over_queue_person(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                if (i)
                    othervar.queue_pos = i;
            } else {
                othervar.queue_pos = 0;
            }
            display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
            break;
        case 1:
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[3].x,
                             oy + resvars.boxes_QueueLayoutp->r[3].y, resvars.boxes_QueueLayoutp->r[3].w,
                             resvars.boxes_QueueLayoutp->r[3].h)) {
                D_001549d0[player].type = EV_CLOSE_QUEUE_WINDOW;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                break;
            }
            if (othervar.queue_pos) {
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[0].x,
                                 oy + resvars.boxes_QueueLayoutp->r[0].y, resvars.boxes_QueueLayoutp->r[0].w,
                                 resvars.boxes_QueueLayoutp->r[0].h)) {
                    D_001549d0[player].type = EV_FRONT_QUEUE;
                    D_001549d0[player].value = othervar.queue_win_room;
                    D_001549d0[player].arg = othervar.queue_pos;
                    display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    break;
                }
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[2].x,
                                 oy + resvars.boxes_QueueLayoutp->r[2].y, resvars.boxes_QueueLayoutp->r[2].w,
                                 resvars.boxes_QueueLayoutp->r[2].h)) {
                    D_001549d0[player].type = EV_BACK_QUEUE;
                    D_001549d0[player].value = othervar.queue_win_room;
                    D_001549d0[player].arg = othervar.queue_pos;
                    display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    break;
                }
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes_QueueLayoutp->r[1].x,
                                 oy + resvars.boxes_QueueLayoutp->r[1].y, resvars.boxes_QueueLayoutp->r[1].w,
                                 resvars.boxes_QueueLayoutp->r[1].h)) {
                    display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                    break;
                }
                {
                    RoomInfo *dest;
                    Thing *selected;

                    dest = &room_info[allhosps[player].build.edit_room];
                    if (dest == 0) {
                        selected = &thing[othervar.pickup_thing];
                        if (selected > thing) {
                            if (selected->type == THING_STAFF && selected->subtype == STAFF_RECEPTIONIST)
                                dest = selected->room;
                            else if (selected->type == THING_OBJECT && selected->subtype == OBJ_RECEPTION_DESK &&
                                     selected->user)
                                dest = thing[selected->user].room;
                        }
                    }
                    if (dest) {
                        D_001549d0[player].type = EV_QUEUE_PERSON_TO_NEW_QUEUE;
                        D_001549d0[player].value = dest - room_info;
                        D_001549d0[player].arg = gamevar.queue_selected_person;
                        set_mouse_mode(MM_NORMAL);
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    }
                }
            } else {
                display_queue_people(ox, oy, othervar.queue_win_room, 0);
            }
            break;
        case 2:
            if (othervar.queue_pos) {
                qx = queue_person_x(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                qy = oy;
                display_queue_people(ox, oy, othervar.queue_win_room, othervar.queue_pos);
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                                 qx - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[7].x,
                                 qy + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[7].y,
                                 resvars.boxes_QueueLayoutp->r[7].w, resvars.boxes_QueueLayoutp->r[7].h)) {
                    D_001549d0[player].type = EV_QUEUE_PERSON_GO_RECEPTION;
                    D_001549d0[player].value = othervar.queue_win_room;
                    D_001549d0[player].arg = othervar.queue_pos;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    break;
                }
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                                 qx - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[8].x,
                                 qy + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[8].y,
                                 resvars.boxes_QueueLayoutp->r[8].w, resvars.boxes_QueueLayoutp->r[8].h)) {
                    D_001549d0[player].type = EV_QUEUE_PERSON_GO_HOME;
                    D_001549d0[player].value = othervar.queue_win_room;
                    D_001549d0[player].arg = othervar.queue_pos;
                    play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
                    break;
                }
                {
                    off = 0;
                    for (i = 0;; i++) {
                        if (i != player) {
                            if (statvar.num_players > 1) {
                                if (network_num_players() <= i)
                                    break;
                            } else if (i >= 4) {
                                break;
                            }
                            if (over_hotspot(
                                    lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                                    qx - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[24].x,
                                    qy + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                                    resvars.boxes_QueueLayoutp->r[24].w, resvars.boxes_QueueLayoutp->r[24].h)) {
                                play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
                                if (statvar.num_players > 1) {
                                    D_001549d0[player].type = EV_REFER_TO_PLAYER;
                                    D_001549d0[player].value =
                                        get_queue_person(othervar.queue_win_room, othervar.queue_pos);
                                    D_001549d0[player].arg = nth_active_player(i + 1);
                                } else {
                                    D_001549d0[player].type = EV_QUEUE_PERSON_GO_HOME;
                                    D_001549d0[player].value = othervar.queue_win_room;
                                    D_001549d0[player].arg = othervar.queue_pos;
                                }
                                break;
                            }
                            off += button_sprite[resvars.requester_sprites[0x2a]].height;
                        }
                    }
                }
            } else {
                display_queue_people(ox, oy, othervar.queue_win_room, 0);
            }
            break;
        default:
            break;
        }
        othervar.queue_drag = 0;
        set_mouse_mode(MM_NORMAL);
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(1);
    if (lbDisplay.mleft_button)
        othervar.left_button_was_down = 1;
    else
        othervar.left_button_was_down = 0;
    if (lbDisplay.mright_button)
        othervar.right_button_was_down = 1;
    else
        othervar.right_button_was_down = 0;
}

// Returns the queue position (1-based) of the patient under the mouse in the
// queue window at (x, y) for room room_id, showing them on the info bar; 0
// if none. sel is the position drawn with a gap around it.
// FUNCTION: 0x000969f4
u8 over_queue_person(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 position;
    RoomInfo *room;
    u16 size;
    s32 step;
    u16 px;
    u16 py;
    u16 id;
    Thing *person;
    CompFrame *fr;
    u16 x0;
    u16 x1;
    u16 y0;
    u16 y1;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = resvars.coords_QueuePanelp->queue_w / size;
        px = x + resvars.coords_QueuePanelp->queue_x;
        py = y + resvars.coords_QueuePanelp->queue_y;
        id = room->queue_head;
        if (id) {
            person = &thing[id];
            for (;;) {
                if (position == sel)
                    px += lbDisplay.graphics_screen_width / resvars.coords_QueuePanelp->drop_gap_divisor;
                fr = &frame[person->sprite];
                x0 = px - fr->width / 2;
                x1 = x0 + fr->width;
                y0 = py - fr->length;
                y1 = y0 + fr->length;
                if (x0 < lbDisplay.mmouse_x && x1 - lbDisplay.graphics_screen_width / 32 > lbDisplay.mmouse_x) {
                    if (y0 < lbDisplay.mmouse_y && y1 > lbDisplay.mmouse_y) {
                        othervar.panel_thing = id;
                        display_panel_person_info();
                        return position;
                    }
                }
                px += step;
                if (position == sel)
                    px += lbDisplay.graphics_screen_width / resvars.coords_QueuePanelp->drop_gap_divisor;
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
    return 0;
}

// Draws room room_id's queue in the queue window at (x, y), each patient
// facing forward, leaving a gap around position sel.
// FUNCTION: 0x00096c34
void display_queue_people(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 position;
    RoomInfo *room;
    u16 size;
    s32 step;
    u16 px;
    u16 py;
    u8 old_facing;
    u16 id;
    Thing *person;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = resvars.coords_QueuePanelp->queue_w / size;
        px = x + resvars.coords_QueuePanelp->queue_x;
        py = y + resvars.coords_QueuePanelp->queue_y;
        id = room->queue_head;
        if (id) {
            person = &thing[id];
            for (;;) {
                u16 old_sprite;
                u16 anim;
                u8 old_flip;
                u16 y0;
                u16 y1;

                old_facing = person->facing;
                old_sprite = person->sprite;
                old_flip = person->flip;
                person->facing = 2;
                anim = check_frame(anim_starts[person->subtype].patient_stand, person) + person->facing;
                person->sprite = start[anim].frame;
                person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                if (position == sel)
                    px += lbDisplay.graphics_screen_width / resvars.coords_QueuePanelp->drop_gap_divisor;
                if (person->illness == ILL_SERIOUS_RADIATION)
                    draw_glow_frame(person, px, py);
                else if (person->illness == ILL_JELLYITIS && othervar.drawing_pixbuf == 0)
                    draw_wibble_frame(person, px, py);
                else if ((u16)(person - thing) == othervar.pickup_thing)
                    draw_hilight_frame(person, px, py);
                else
                    draw_special_id_frame(person, px, py);
                if (position == sel)
                    px += lbDisplay.graphics_screen_width / resvars.coords_QueuePanelp->drop_gap_divisor;
                person->facing = old_facing;
                person->sprite = old_sprite;
                person->flip = old_flip;
                {
                    CompFrame *fr;
                    u16 x0;
                    u16 x1;

                    fr = &frame[person->sprite];
                    x0 = px - fr->width / 2;
                    x1 = x0 + fr->width;
                    y0 = py - fr->length;
                    y1 = y0 + fr->length;
                }
                px += step;
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
}

// Draws the patient at queue position sel of room room_id under the mouse
// at (x, y), blended, while it is dragged.
// FUNCTION: 0x00096f50
void display_queue_person(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 position;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *person;
    u8 old_facing;
    u16 old_sprite;
    u8 old_flip;
    u16 anim;
    CompFrame *fr;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            person = &thing[id];
            for (;;) {
                if (position == sel) {
                    old_facing = person->facing;
                    old_sprite = person->sprite;
                    old_flip = person->flip;
                    person->facing = 2;
                    anim = check_frame(anim_starts[person->subtype].patient_stand, person) + person->facing;
                    person->sprite = start[anim].frame;
                    person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                    InitPixBuffer(0, 0, 0x40, 0x40, 0, 0);
                    DrawIntoPixBuffer();
                    full_draw_frame(person, person->sprite, 0x20, 0x3a, 0, 0, person->flip);
                    RestoreFromPixBuffer();
                    pixBuf.x = x - 0x20;
                    pixBuf.y = y - 0x3a;
                    DrawPixBuffer(1, 0, 0, 0x40, 0x40);
                    FillPixBuffer(0xfe);
                    InitPixBuffer(0, 0, 0xb4, 0xb4, 0, 0);
                    lbDisplay.draw_flags = 0;
                    fr = &frame[person->sprite];
                    set_all_refresh_zones_in_window(x + frame_bounds[4], y + frame_bounds[5],
                                                    x + frame_bounds[4] + fr->width, y + frame_bounds[5] + fr->length,
                                                    3);
                    person->facing = old_facing;
                    person->sprite = old_sprite;
                    person->flip = old_flip;
                    break;
                }
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
}

// Draws the right-click menu for a queued patient at (x, y): to reception,
// home, and one entry per other player to refer them to.
// FUNCTION: 0x000971f8
void display_queue_bar(u16 x, u16 y)
{
    u8 i;
    u16 off;
    char buf[100];

    reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x, y + resvars.boxes_QueueLayoutp->r[4].y,
                &button_sprite[resvars.requester_sprites[0x28]]);
    if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                     x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[7].x,
                     y + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[7].y,
                     resvars.boxes_QueueLayoutp->r[7].w, resvars.boxes_QueueLayoutp->r[7].h))
        reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[7].x,
                    y + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[7].y,
                    &button_sprite[resvars.requester_sprites[0x2c]]);
    else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                          x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[8].x,
                          y + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[8].y,
                          resvars.boxes_QueueLayoutp->r[8].w, resvars.boxes_QueueLayoutp->r[8].h))
        reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[8].x,
                    y + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[8].y,
                    &button_sprite[resvars.requester_sprites[0x2d]]);
    set_all_refresh_zones_in_window(
        x - resvars.boxes_QueueLayoutp->r[4].x, y + resvars.boxes_QueueLayoutp->r[4].y,
        button_sprite[resvars.requester_sprites[0x28]].width + (x - resvars.boxes_QueueLayoutp->r[4].x),
        button_sprite[resvars.requester_sprites[0x28]].height + (y + resvars.boxes_QueueLayoutp->r[4].y), 3);
    off = 0;
    for (i = 0;; i++) {
        if (i != player) {
            if (statvar.num_players > 1) {
                if (network_num_players() <= i)
                    break;
            } else if (i >= 4) {
                break;
            }
            reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x,
                        y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                        &button_sprite[resvars.requester_sprites[0x2a]]);
            set_all_refresh_zones_in_window(
                x - resvars.boxes_QueueLayoutp->r[4].x,
                y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                x - resvars.boxes_QueueLayoutp->r[4].x + button_sprite[resvars.requester_sprites[0x2a]].width,
                (y + off) + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y +
                    button_sprite[resvars.requester_sprites[0x2a]].height,
                3);
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y,
                             x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[9].x,
                             y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                             resvars.boxes_QueueLayoutp->r[9].w, resvars.boxes_QueueLayoutp->r[9].h))
                reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[9].x,
                            y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                            &button_sprite[resvars.requester_sprites[0x2e]]);
            LbTextSetWindow(x - resvars.boxes_QueueLayoutp->r[4].x + resvars.boxes_QueueLayoutp->r[23].x,
                            y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y +
                                resvars.boxes_QueueLayoutp->r[23].y,
                            resvars.boxes_QueueLayoutp->r[23].w, resvars.boxes_QueueLayoutp->r[23].h);
            lbDisplay.draw_flags = 0x100;
            if (statvar.num_players > 1)
                sprintf(buf, "%s", &allhosps[nth_active_player(i + 1)]);
            else
                sprintf(buf, "%s", &allhosps[i]);
            LbTextDraw(0, 0, buf);
            lbDisplay.draw_flags = 0;
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            off += button_sprite[resvars.requester_sprites[0x2a]].height;
        }
    }
    if (resvars.screen_mode != SCREEN_MODE_1) {
        off -= resvars.boxes_QueueLayoutp->r[9].y;
        reqspr_draw(x - resvars.boxes_QueueLayoutp->r[4].x,
                    y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
                    &button_sprite[resvars.requester_sprites[0x2b]]);
        set_all_refresh_zones_in_window(
            x - resvars.boxes_QueueLayoutp->r[4].x,
            y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y,
            x - resvars.boxes_QueueLayoutp->r[4].x + button_sprite[resvars.requester_sprites[0x2a]].width,
            y + off + resvars.boxes_QueueLayoutp->r[4].y + resvars.boxes_QueueLayoutp->r[5].y +
                button_sprite[resvars.requester_sprites[0x2b]].height,
            3);
    }
}

// Returns the screen x of the patient at queue position sel (1-based) of
// room room_id in the queue window at (x, y), or 0.
// FUNCTION: 0x00097904
u16 queue_person_x(u16 x, u16 y, u16 room_id, u8 sel)
{
    u16 px;
    u16 x1;
    CompFrame *fr;
    Thing *person;
    u16 size;
    u16 id;
    s32 step;
    u16 py;
    RoomInfo *room;
    u16 position;
    u16 x0;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        step = resvars.coords_QueuePanelp->queue_w / size;
        px = x + resvars.coords_QueuePanelp->queue_x;
        py = y + resvars.coords_QueuePanelp->queue_y;
        id = room->queue_head;
        if (id) {
            u16 y1;

            person = &thing[id];
            for (;;) {
                u16 y0;
                u8 old_facing;
                u16 old_sprite;
                u8 old_flip;
                u16 anim;

                old_facing = person->facing;
                old_sprite = person->sprite;
                old_flip = person->flip;
                person->facing = 2;
                anim = check_frame(anim_starts[person->subtype].patient_stand, person) + person->facing;
                person->sprite = start[anim].frame;
                person->flip = start[anim].flags > 0 ? (u8)1 : (u8)0;
                person->facing = old_facing;
                person->sprite = old_sprite;
                person->flip = old_flip;
                if (position == sel)
                    return px;
                fr = &frame[person->sprite];
                x0 = px - fr->width / 2;
                x1 = x0 + fr->width;
                y0 = py - fr->length;
                y1 = y0 + fr->length;
                px += step;
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
    return 0;
}

// Returns 1 if room room_id's queue has a position sel (1-based), else 0.
// FUNCTION: 0x00097b40
u8 queue_person_valid(u16 room_id, u8 sel)
{
    u16 position;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *person;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            person = &thing[id];
            for (;;) {
                if (position == sel)
                    return 1;
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
    return 0;
}

// Returns the patient at queue position sel (1-based) of room room_id, or 0.
// FUNCTION: 0x00097c0c
u16 get_queue_person(u16 room_id, u8 sel)
{
    u16 position;
    RoomInfo *room;
    u16 size;
    u16 id;
    Thing *person;

    position = 1;
    room = &room_info[room_id];
    size = queue_size(room);
    if (size) {
        id = room->queue_head;
        if (id) {
            person = &thing[id];
            for (;;) {
                if (position == sel)
                    return id;
                if (person->queue_next == 0)
                    break;
                id = person->queue_next;
                person = &thing[id];
                position++;
            }
        }
    }
    return 0;
}
