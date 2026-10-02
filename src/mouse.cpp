#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    u16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

extern "C" {
extern Event player_events[];
extern Slot irequester_slots[];
}

extern u8 clicked_icon;
extern s16 mcurx;
extern s16 mcury;

void process_no_button(u16 icon);
void process_left_button(u16 icon);
void process_mleft_button(u16 icon);
void process_right_button(u16 icon);
void process_mright_button(u16 icon);
void peep_icon(Thing *const peep);
u8 is_person_highlighted_for_vaccination(u8 flag);
void panel_l_button(const u16 icon);
void clicked_for_edit(void);

#define PARTNER(t)                                                                                                     \
    ((t)->object && (thing[(t)->object].type == THING_OBJECT || thing[(t)->object].type == THING_DOOR) &&              \
             (u16)((t) - thing) == thing[(t)->object].user                                                             \
         ? (t)->object                                                                                                 \
         : 0)
void panel_ml_button(const u16 icon);
void panel_r_button(const u16 icon);
u8 place_object_change_angle(s8 angle);
void draw_object_cursor(void);
void draw_staff_cursor(void);
void check_object_right_click_in_room_edit(void);
void pick_up_object_right_click(void);
u16 compute_object_pointed(RoomInfo *room);
void compute_room_pointed(u16 tile);
void draw_from_frame(const u16 frame_id, const s16 x, const s16 y, u8 *remap, const u8 flip);

// Per-frame mouse handling in the game view: edge scrolling, the pointer
// over menus, windows and the bottom panel, and in MM_NORMAL what the pointer
// is over (gamevar.hover_action: 3 person or object, 4 room door, 5/6 near/on a rat,
// 7 a pending call, 8 the advisor, 1/2 vaccination target or not), then
// dispatches the button state to the process_*_button routines.
// FUNCTION: 0x00059d00
void process_mouse(void)
{
    Thing *hovered;
    Thing *used_object;
    u16 scroll;
    u8 icon;

    if (gamevar.paused) {
        lbDisplay.left_button = lbDisplay.right_button = 0;
        return;
    }
    if (gamevar.mouse_mode == MM_HELP) {
        if (lbDisplay.left_button || lbDisplay.right_button) {
            lbDisplay.left_button = 0;
            lbDisplay.right_button = 0;
            if (othervar.help_close_x <= lbDisplay.mouse_x &&
                othervar.help_close_x + pulldv[19].width > lbDisplay.mouse_x &&
                othervar.help_close_y <= lbDisplay.mouse_y &&
                othervar.help_close_y + pulldv[19].height > lbDisplay.mouse_y)
                finish_composite_help();
        }
        return;
    }
    if (debug.suppress_help) {
    }
    scroll = 0;
    if (lbDisplay.mmouse_x <= resvars.coords[0xa7].x)
        scroll |= 4;
    else if (lbDisplay.mmouse_x >= resvars.coords[0xa8].x)
        scroll |= 8;
    if (lbDisplay.mmouse_y <= resvars.coords[0xa7].y)
        scroll |= 1;
    else if (lbDisplay.mmouse_y >= resvars.coords[0xa8].y)
        scroll |= 2;
    if (scroll > 0)
        player_events[player].flags = scroll;
    othervar.pickup_thing = 0;
    othervar.advisor_right_click = 0;
    if (othervar.menu_bar_active && !othervar.hovered_popup_icon) {
        lbDisplay.left_button = 0;
        if (!othervar.menu_arrow_set) {
            set_mouse_sprite(PTR_DEFAULT);
            othervar.menu_arrow_set = 1;
        }
        return;
    }
    if (othervar.menu_arrow_set) {
        if (othervar.pointer_sprite == PTR_DEFAULT)
            set_mouse_sprite(othervar.prev_pointer_sprite);
        othervar.menu_arrow_set = 0;
    }
    if (CountWindows(open_windows)) {
        if (MouseOnWindow(open_windows[0])) {
            lbDisplay.left_button = lbDisplay.right_button = 0;
            return;
        }
    }
    if (statvar.panel_top <= lbDisplay.mmouse_y) {
        icon = which_icon_is_mouse_over();
        if (gamevar.mouse_mode == MM_NORMAL) {
            if (othervar.pointer_sprite != PTR_DEFAULT)
                set_mouse_sprite(PTR_DEFAULT);
        }
    } else if (advisor_up_states[advisor.anim_state] && lbDisplay.right_button &&
               ((lbDisplay.mmouse_y >= resvars.coords[0x93].y && advisor.box_left_u <= lbDisplay.mmouse_x &&
                 lbDisplay.mmouse_x <= resvars.coords[0x92].x && lbDisplay.mmouse_y <= resvars.coords[0x92].y) ||
                (lbDisplay.mmouse_x >= resvars.coords[0x90].x && lbDisplay.mmouse_y >= resvars.coords[0x90].y &&
                 lbDisplay.mmouse_x <= resvars.coords[0x91].x && lbDisplay.mmouse_y <= resvars.coords[0x91].y))) {
        othervar.advisor_right_click = 1;
        gamevar.hover_action = 8;
        icon = 0x27;
    } else {
        icon = 0x27;
        switch (gamevar.mouse_mode) {
        case MM_VACCINATE:
            if (is_person_highlighted_for_vaccination(1)) {
                gamevar.hover_action = 1;
                set_mouse_sprite(PTR_VACCINATE_TARGET);
            } else {
                gamevar.hover_action = 2;
                set_mouse_sprite(PTR_VACCINATE);
            }
            break;
        case MM_NORMAL: {
            u8 hit;
            u8 set;

            hit = 0;
            set = 0;
            if (othervar.hovered_popup_icon) {
                if (gamevar.hover_action != 7)
                    set_mouse_sprite(PTR_DEFAULT);
                gamevar.hover_action = 7;
                set = 1;
            } else if (othervar.pointed_rat) {
                if (gamevar.hover_action != 6)
                    set_mouse_sprite(PTR_RAT_AIM);
                gamevar.hover_action = 6;
                set = 1;
            } else if (othervar.near_rat) {
                if (gamevar.hover_action != 5)
                    set_mouse_sprite(PTR_RAT_NEAR);
                gamevar.hover_action = 5;
                set = 1;
            } else if (crequester.id == REQ_NONE && (hovered = &thing[othervar.pointed_thing]) > thing) {
                if (gamevar.rat_fest_level)
                    return;
                if ((hovered->type == THING_PATIENT || hovered->type == THING_STAFF) &&
                    (used_object = &thing[hovered->object &&
                                                  (thing[hovered->object].type == THING_OBJECT ||
                                                   thing[hovered->object].type == THING_DOOR) &&
                                                  (u16)(hovered - thing) == thing[hovered->object].user
                                              ? hovered->object
                                              : 0]) > thing) {
                    if (used_object->subtype == OBJ_RECEPTION_DESK) {
                        othervar.prev_panel_info = othervar.panel_info;
                        othervar.panel_info = 4;
                        gamevar.hover_action = 3;
                        hit = 1;
                        peep_icon(hovered);
                        if (othervar.pointer_sprite != PTR_QUEUE)
                            set_mouse_sprite(PTR_QUEUE);
                        goto done;
                    }
                    if (objects[used_object->subtype].debris)
                        hovered = used_object;
                }
                if (hovered->type == THING_OBJECT) {
                    if (hovered->pair && objects[thing[hovered->pair].subtype].next_part_index != -1)
                        hovered = &thing[hovered->pair];
                    if (objects[hovered->subtype].debris) {
                        othervar.prev_panel_info = othervar.panel_info;
                        othervar.panel_info = 3;
                        gamevar.hover_action = 3;
                        if (othervar.pointer_sprite != PTR_MACHINE)
                            set_mouse_sprite(PTR_MACHINE);
                        hit = 1;
                        goto done;
                    }
                    if (objects[hovered->subtype].flags & 2) {
                        othervar.prev_panel_info = othervar.panel_info;
                        othervar.panel_info = 6;
                        gamevar.hover_action = 3;
                        hit = 1;
                        goto done;
                    }
                }
                hovered = &thing[othervar.pointed_thing];
                if (hovered->type == THING_OBJECT &&
                    (hovered->subtype == OBJ_SWING_DOOR || hovered->subtype == OBJ_SWING_DOOR2) && hovered->door_main)
                    hovered = &thing[hovered->door_main];
                if (hovered->type == THING_PATIENT) {
                    othervar.prev_panel_info = othervar.panel_info;
                    othervar.panel_info = 2;
                    gamevar.hover_action = 3;
                    peep_icon(hovered);
                    hit = 1;
                    if (othervar.pointer_sprite != PTR_PATIENT)
                        set_mouse_sprite(PTR_PATIENT);
                    if (myhosp->epidemic_active && is_person_highlighted_for_vaccination(0)) {
                        gamevar.hover_action = 1;
                        set_mouse_sprite(PTR_VACCINATE_TARGET);
                    }
                } else if (hovered->type == THING_STAFF) {
                    othervar.prev_panel_info = othervar.panel_info;
                    othervar.panel_info = 1;
                    gamevar.hover_action = 3;
                    peep_icon(hovered);
                    hit = 1;
                    if (othervar.pointer_sprite != PTR_STAFF)
                        set_mouse_sprite(PTR_STAFF);
                } else if (hovered->type == THING_DOOR) {
                    RoomInfo *room;

                    room = &room_info[hovered->room_id];
                    if (rooms[room->type].has_queue) {
                        othervar.prev_panel_info = othervar.panel_info;
                        othervar.panel_info = 4;
                        gamevar.hover_action = 4;
                        hit = 1;
                        if (othervar.pointer_sprite != PTR_QUEUE)
                            set_mouse_sprite(PTR_QUEUE);
                    }
                } else if (hovered->type == THING_MAYOR || hovered->type == THING_INSPECTOR) {
                    othervar.prev_panel_info = othervar.panel_info;
                    othervar.panel_info = 5;
                    gamevar.hover_action = 3;
                    hit = 1;
                }
            }
        done:
            if (hit) {
                othervar.panel_info_timer = 100;
                othervar.panel_thing = hovered - thing;
            } else if (!set && (othervar.pointer_sprite != PTR_DEFAULT || gamevar.hover_action != 0)) {
                set_mouse_sprite(PTR_DEFAULT);
                gamevar.hover_action = 0;
            }
            if (othervar.panel_info_timer != 0) {
                othervar.panel_info_timer--;
                if (othervar.panel_info_timer == 0) {
                    othervar.prev_panel_info = othervar.panel_info;
                    othervar.panel_info = 0;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    if (lbDisplay.left_button + lbDisplay.mleft_button + lbDisplay.right_button + lbDisplay.mright_button != 0) {
        if (icon == 0x27) {
            if (lbDisplay.left_button) {
                process_left_button((u16)icon);
                lbDisplay.left_button = 0;
            } else if (lbDisplay.mleft_button) {
                process_mleft_button((u16)icon);
            }
        } else if (lbDisplay.mleft_button) {
            process_mleft_button((u16)icon);
        } else if (lbDisplay.left_button) {
            process_left_button((u16)icon);
            lbDisplay.left_button = 0;
        }
        if (lbDisplay.right_button) {
            process_right_button((u16)icon);
            lbDisplay.right_button = 0;
        } else if (lbDisplay.mright_button) {
            process_mright_button((u16)icon);
        }
    } else {
        process_no_button((u16)icon);
    }
}

// Left click. icon 0x27 is the map: by mouse mode (gamevar.mouse_mode) it
// presses the watch buttons, opens the clicked thing's window, queue or call,
// shoots a rat, offers to poach another player's staff member, sends the
// blueprint, door, staff and object placement events, picks up or edits
// rooms' objects, moves a queued person, or toggles vaccination; any other
// icon is a panel button (panel_l_button).
// FUNCTION: 0x0005a750
void process_left_button(u16 icon)
{
    Thing *clicked;

    cancel_helpmsg();
    switch (icon) {
    case 0x27:
        if (display_mode == DM_GAME) {
            switch (gamevar.mouse_mode) {
            case MM_NORMAL:
                if (crequester.id != REQ_NONE)
                    break;
                if (myhosp->epidemic_timer) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x86].x,
                                     resvars.coords[0x86].y, watch[14].width, watch[14].height)) {
                        gamevar.mouse_mode = MM_VACCINATE;
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                        break;
                    }
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x86].x,
                                     resvars.coords[0x86].y, watch[13].width, watch[13].height)) {
                        if (othervar.epidemic_patient) {
                            othervar.center_on_slot = 1;
                            irequester_slots[1].value = othervar.epidemic_patient;
                            othervar.full_redraw = 1;
                            play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
                        }
                        break;
                    }
                }
                if (myhosp->opening_countdown) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x86].x,
                                     resvars.coords[0x86].y, watch[14].width, watch[14].height)) {
                        player_events[player].type = EV_OPEN_HOSPITAL;
                        play_sound(SNDSRC_GENERAL, SFX_FANFARE, -1, -1);
                        break;
                    }
                }
                if (myhosp->emergency_timer) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x84].x,
                                     resvars.coords[0x84].y, watch[13].width, watch[13].height)) {
                        if (othervar.emergency_patient) {
                            othervar.center_on_slot = 1;
                            irequester_slots[1].value = othervar.emergency_patient;
                            othervar.full_redraw = 1;
                            play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
                        }
                        break;
                    }
                }
                switch (gamevar.hover_action) {
                case 0:
                    break;
                case 3:
                    clicked = &thing[othervar.panel_thing];
                    if (clicked->owner == player) {
                        switch (clicked->type) {
                        case THING_OBJECT:
                            switch (clicked->subtype) {
                            case OBJ_RECEPTION_DESK: {
                                Thing *receptionist;

                                receptionist = &thing[clicked->user];
                                if (receptionist > thing) {
                                    if (receptionist->room) {
                                        player_events[player].type = EV_OPEN_QUEUE_WINDOW;
                                        player_events[player].value = receptionist->room - room_info;
                                    }
                                }
                                break;
                            }
                            default:
                                if (!(objects[clicked->subtype].flags & 4)) {
                                    player_events[player].type = EV_OPEN_THING_WINDOW;
                                    player_events[player].value = othervar.panel_thing;
                                }
                                break;
                            }
                            break;
                        case THING_STAFF:
                            switch (clicked->subtype) {
                            case STAFF_RECEPTIONIST:
                                if (PARTNER(&thing[othervar.panel_thing]) &&
                                    thing[PARTNER(&thing[othervar.panel_thing])].subtype == OBJ_RECEPTION_DESK) {
                                    if (thing[othervar.panel_thing].room) {
                                        player_events[player].type = EV_OPEN_QUEUE_WINDOW;
                                        player_events[player].value = thing[othervar.panel_thing].room - room_info;
                                        break;
                                    }
                                }
                            default:
                                if (crequester.id == REQ_NONE) {
                                    close_irequesters();
                                    if ((clicked->orders & ORD_RESIGN) && clicked->playercall) {
                                        player_events[player].type = EV_ANSWER_PLAYERCALL;
                                        player_events[player].value = 1;
                                        player_events[player].arg = clicked->playercall;
                                    } else {
                                        player_events[player].type = EV_OPEN_THING_WINDOW;
                                        player_events[player].value = othervar.panel_thing;
                                    }
                                }
                                break;
                            }
                            break;
                        case THING_PATIENT:
                            if (crequester.id == REQ_NONE) {
                                close_irequesters();
                                if ((clicked->flags1 & (PF_NEEDS_DIAGNOSIS_ROOM >> 8)) && clicked->playercall) {
                                    player_events[player].type = EV_ANSWER_PLAYERCALL;
                                    player_events[player].value = 8;
                                    player_events[player].arg = clicked->playercall;
                                } else if ((clicked->flags0 & PF_AWAITING_DECISION) && clicked->playercall) {
                                    player_events[player].type = EV_ANSWER_PLAYERCALL;
                                    player_events[player].value = 4;
                                    player_events[player].arg = clicked->playercall;
                                } else {
                                    player_events[player].type = EV_OPEN_THING_WINDOW;
                                    player_events[player].value = othervar.panel_thing;
                                }
                            }
                            break;
                        default:
                            player_events[player].type = EV_OPEN_THING_WINDOW;
                            player_events[player].value = othervar.panel_thing;
                            break;
                        }
                    } else if (statvar.num_players > 1 && clicked->type == THING_STAFF && clicked->owner != 0x63) {
                        if (clicked->poach_offers) {
                            if (clicked->poach_offers != 1 << player)
                                advisor.forced_advice_id = 0xf;
                        } else if (clicked->discontent_s < gbv.resign_max / 4) {
                            advisor.forced_advice_id = 0x10;
                        } else {
                            player_events[player].type = EV_POACH_STAFF;
                            player_events[player].value = clicked - thing;
                            advisor.forced_advice_id = 0x11;
                        }
                    }
                    break;
                case 4:
                    if (thing[othervar.panel_thing].owner == player) {
                        player_events[player].type = EV_OPEN_QUEUE_WINDOW;
                        player_events[player].value = thing[othervar.panel_thing].room_id;
                        player_events[player].arg =
                            (thing[othervar.panel_thing].y.tile << 7) + thing[othervar.panel_thing].x.tile;
                        compute_room_pointed(othervar.mouse_cell);
                    }
                    break;
                case 5:
                case 6:
                    if (!gamevar.paused) {
                        player_events[player].type = EV_SHOOT_RAT;
                        player_events[player].value = othervar.pointed_rat;
                        player_events[player].argb[0] = gamevar.rat_fest_powerup;
                        if (othervar.pointed_rat)
                            player_events[player].argb[1] = 1;
                        else
                            player_events[player].argb[1] = 0;
                        if (othervar.rat_powerup_shots) {
                            othervar.rat_powerup_shots--;
                            if (othervar.rat_powerup_shots <= 0) {
                                gamevar.rat_fest_powerup = 0;
                                othervar.rat_hit_margin = 2;
                                othervar.rat_powerup_shots = 0;
                            }
                        }
                    }
                    break;
                case 7:
                    if (crequester.id == REQ_NONE) {
                        close_irequesters();
                        if (othervar.hovered_popup_icon) {
                            player_events[player].type = EV_ANSWER_PLAYERCALL;
                            player_events[player].value = (u16)othervar.hovered_popup_icon;
                            player_events[player].arg = 0;
                        }
                    }
                    lbDisplay.left_button = lbDisplay.right_button = 0;
                    break;
                case 1:
                    player_events[player].type = EV_TOGGLE_VACCINATION;
                    player_events[player].value = othervar.vaccine_target;
                    break;
                case 2:
                default:
                    break;
                }
                break;
            case MM_BLUEPRINT:
                player_events[player].type = EV_MODIFY_BLUEPRINT;
                player_events[player].value = othervar.mouse_cell;
                break;
            case MM_PLACE_DOOR:
                player_events[player].type = EV_PLACE_BP_DOOR;
                player_events[player].value = 0;
                break;
            case MM_PLACE_STAFF:
                draw_staff_cursor();
                if (gamevar.staff_place_ok) {
                    player_events[player].type = EV_PLACE_STAFF;
                    player_events[player].value = othervar.mouse_cell;
                    player_events[player].arg = gamevar.held_staff;
                }
                break;
            case MM_PLACE_OBJECT:
                draw_object_cursor();
                if (gamevar.place_ok) {
                    player_events[player].type = EV_PLACE_OBJECT;
                    player_events[player].value = gamevar.place_tile;
                    player_events[player].argb[0] = gamevar.place_kind;
                    player_events[player].argb[1] = gamevar.place_dir;
                }
                break;
            case MM_PLACE_SMALL_THING:
                player_events[player].type = EV_PLACE_SMALL_THING;
                player_events[player].value = othervar.mouse_cell;
                player_events[player].arg = (u16)gamevar.place_blueprint;
                break;
            case MM_PICK_UP_OBJECT:
                othervar.pickup_thing = compute_object_pointed(&room_info[allhosps[player].edited_room]);
                if (othervar.pickup_thing) {
                    if (thing[othervar.pickup_thing].room_id == allhosps[player].edited_room) {
                        player_events[player].type = EV_PICK_UP_OBJECT;
                        player_events[player].value = othervar.pickup_thing;
                        player_events[player].arg = 1;
                    }
                }
                break;
            case MM_EDIT_ROOM:
                if (map[othervar.mouse_cell].owner_flags == 1 << player) {
                    compute_room_pointed(othervar.mouse_cell);
                    clicked_for_edit();
                }
                break;
            case MM_VACCINATE:
                if (myhosp->epidemic_timer) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x86].x,
                                     resvars.coords[0x86].y, watch[14].width, watch[14].height)) {
                        gamevar.mouse_mode = MM_NORMAL;
                        break;
                    }
                }
                if (gamevar.hover_action == 1) {
                    player_events[player].type = EV_TOGGLE_VACCINATION;
                    player_events[player].value = othervar.vaccine_target;
                }
                break;
            case MM_MOVE_QUEUE_PERSON: {
                RoomInfo *room;

                room = &room_info[allhosps[player].build.edit_room];
                if (room == 0) {
                    Thing *selected;

                    selected = &thing[othervar.pickup_thing];
                    if (selected > thing) {
                        if (selected->type == THING_STAFF && selected->subtype == STAFF_RECEPTIONIST)
                            room = selected->room;
                        else if (selected->type == THING_OBJECT && selected->subtype == OBJ_RECEPTION_DESK &&
                                 selected->user)
                            room = thing[selected->user].room;
                    }
                }
                if (room) {
                    player_events[player].type = EV_QUEUE_PERSON_TO_NEW_QUEUE;
                    player_events[player].value = room - room_info;
                    player_events[player].arg = gamevar.queue_selected_person;
                    set_mouse_mode(MM_NORMAL);
                }
                break;
            }
            case 4:
            case 7:
            case 8:
            case MM_SCREEN:
            case MM_TOWN_MAP:
            case MM_NAV_DEBUG:
            case MM_EDIT_ROOM_WAIT:
            case 0x10:
            case 0x11:
            default:
                break;
            }
        }
        break;
    default:
        clicked_icon = icon;
        panel_l_button(icon);
        break;
    }
}

// Left button held. On the map it drags out a blueprint (MM_BLUEPRINT) or
// presses the emergency and vaccination watch buttons; otherwise
// panel_ml_button.
// FUNCTION: 0x0005b70c
void process_mleft_button(u16 icon)
{
    switch (icon) {
    case 0x27:
        if (display_mode == DM_GAME) {
            if (player_events[player].type == EV_NONE) {
                switch (gamevar.mouse_mode) {
                case MM_BLUEPRINT:
                    player_events[player].type = EV_MODIFY_BLUEPRINT;
                    player_events[player].value = othervar.mouse_cell;
                    break;
                case MM_NORMAL:
                case MM_VACCINATE:
                    if (myhosp->epidemic_timer) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.coords[0x86].x,
                                         resvars.coords[0x86].y, watch[14].width, watch[14].height))
                            reqspr_draw(resvars.coords[0x86].x, resvars.coords[0x86].y, &watch[15]);
                    }
                    if (myhosp->emergency_timer) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x84].x,
                                         resvars.coords[0x84].y, watch[13].width, watch[13].height)) {
                            if (othervar.emergency_patient) {
                                othervar.center_on_slot = 1;
                                irequester_slots[1].value = othervar.emergency_patient;
                                othervar.full_redraw = 1;
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        break;
    default:
        panel_ml_button(icon);
        break;
    }
}

// Right button held: on the map in MM_PLACE_OBJECT, as with no button;
// otherwise panel_ml_button.
// FUNCTION: 0x0005b8f0
void process_mright_button(u16 icon)
{
    switch (icon) {
    case 0x27:
        switch (gamevar.mouse_mode) {
        case MM_PLACE_OBJECT:
            process_no_button(icon);
            break;
        }
        break;
    default:
        panel_ml_button(icon);
        break;
    }
}

// Right click (ignored in the rat game). On the map: the emergency/epidemic
// watch buttons open the next such patient's window; otherwise picks up the
// staff member or object under the pointer, dismisses the advisor, rotates
// the object being placed, sends the door placement event, cancels editing a
// room that is emptying, or cancels the current mode or requester (as key 1,
// Escape). Other icons go to panel_r_button.
// FUNCTION: 0x0005b95c
void process_right_button(u16 icon)
{
    Thing *hovered;

    if (!gamevar.rat_fest_level) {
        switch (icon) {
        case 0x27:
            if (display_mode == DM_GAME) {
                switch (gamevar.mouse_mode) {
                case MM_NORMAL:
                    if (myhosp->epidemic_timer) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x86].x,
                                         resvars.coords[0x86].y, watch[13].width, watch[13].height)) {
                            othervar.epidemic_patient = find_next_emergency_person(othervar.emergency_patient, 0x20);
                            if (othervar.epidemic_patient) {
                                player_events[player].type = EV_OPEN_THING_WINDOW;
                                player_events[player].value = othervar.epidemic_patient;
                                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
                            } else {
                                player_events[player].type = EV_CLOSE_QUERY;
                                player_events[player].value = 1;
                                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                            }
                            break;
                        }
                    }
                    if (myhosp->emergency_timer) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.coords[0x84].x,
                                         resvars.coords[0x84].y, watch[13].width, watch[13].height)) {
                            othervar.emergency_patient = find_next_emergency_person(othervar.emergency_patient, 0x40);
                            if (othervar.emergency_patient) {
                                player_events[player].type = EV_OPEN_THING_WINDOW;
                                player_events[player].value = othervar.emergency_patient;
                                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
                            } else {
                                player_events[player].type = EV_CLOSE_QUERY;
                                player_events[player].value = 1;
                                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                            }
                            break;
                        }
                    }
                    {
                        switch (gamevar.hover_action) {
                        case 0:
                        case 3:
                            if (!mouse_was_on_window &&
                                (!allhosps[player].edited_room || crequester.id != REQ_EDIT_ROOM) &&
                                (crequester.id != REQ_NONE || othervar.irequester_slot != 0)) {
                                process_keys(0, 1);
                            } else {
                                hovered = &thing[othervar.pointed_thing];
                                if (hovered->owner == player) {
                                    if (allhosps[player].edited_room) {
                                        check_object_right_click_in_room_edit();
                                    } else if (gamevar.hover_action == 3 && hovered > thing &&
                                               hovered->type == THING_STAFF) {
                                        player_events[player].type = EV_PICK_UP_STAFF;
                                        player_events[player].value = othervar.pointed_thing;
                                        play_sound(SNDSRC_GENERAL, SFX_PICKUP, -1, -1);
                                    } else {
                                        pick_up_object_right_click();
                                    }
                                }
                            }
                            break;
                        case 7:
                            lbDisplay.left_button = lbDisplay.right_button = 0;
                            break;
                        case 8:
                            if (othervar.advisor_right_click) {
                                othervar.advisor_right_click = 0;
                                othervar.dismiss_advisor = 1;
                            }
                            break;
                        case 1:
                        case 2:
                        case 4:
                        case 5:
                        case 6:
                        default:
                            break;
                        }
                    }
                    break;
                case MM_PLACE_DOOR:
                    player_events[player].type = EV_PLACE_BP_DOOR;
                    player_events[player].value = 1;
                    break;
                case MM_PLACE_OBJECT:
                    if (gamevar.place_mode != 1) {
                        if (place_object_change_angle(-1))
                            play_sound(SNDSRC_GENERAL, SFX_SWOOSH, -1, -1);
                    }
                    draw_object_cursor();
                    break;
                case MM_PLACE_STAFF:
                case MM_EDIT_ROOM:
                case MM_VACCINATE:
                    if (!mouse_was_on_window)
                        process_keys(0, 1);
                    break;
                case MM_EDIT_ROOM_WAIT:
                    player_events[player].type = EV_CANCEL_EDIT_ROOM;
                    break;
                }
            }
            break;
        default:
            clicked_icon = icon;
            panel_r_button(icon);
            break;
        }
    }
}

// No button pressed: on the map, highlights the queue of the room (or
// reception) under the pointer in MM_NORMAL, or updates the blueprint, door,
// staff, object or room-edit cursor for the current mouse mode.
// FUNCTION: 0x0005be7c
void process_no_button(u16 icon)
{
    RoomInfo *room;
    Thing *hovered;

    switch (icon) {
    case 0x27:
        if (display_mode == DM_GAME) {
            switch (gamevar.mouse_mode) {
            case MM_NORMAL:
                room = which_room(othervar.mouse_cell);
                if (room && room->owner == player) {
                    flag_all_queue_members(room);
                } else {
                    hovered = &thing[othervar.pointed_thing];
                    if (hovered > thing && hovered->owner == player) {
                        if ((hovered->type == THING_OBJECT && hovered->subtype == OBJ_RECEPTION_DESK) ||
                            (hovered->type == THING_STAFF && hovered->subtype == STAFF_RECEPTIONIST))
                            flag_all_queue_members(thing[othervar.panel_thing].room);
                    }
                }
                break;
            case MM_BLUEPRINT:
                if (player_events[player].type == EV_NONE) {
                    player_events[player].type = EV_HOVER_BLUEPRINT;
                    player_events[player].value = othervar.mouse_cell;
                }
                break;
            case MM_PLACE_DOOR:
                player_events[player].type = EV_TEMP_PLACE_DOOR;
                player_events[player].value = othervar.prev_mouse_cell;
                player_events[player].arg = 0;
                break;
            case MM_PLACE_STAFF:
                draw_staff_cursor();
                break;
            case MM_PLACE_OBJECT:
                draw_object_cursor();
                break;
            case MM_PLACE_SMALL_THING:
                LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
                LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
                draw_from_frame(start[blueprint[gamevar.place_blueprint].anim].frame, resvars.tile_width / 2 + mcurx,
                                mcury - resvars.tile_height / 2, 0,
                                start[blueprint[gamevar.place_blueprint].anim].flags);
                break;
            case MM_PICK_UP_OBJECT:
                othervar.pickup_thing = compute_object_pointed(&room_info[allhosps[player].edited_room]);
                break;
            case MM_MOVE_QUEUE_PERSON:
                room = which_room(othervar.mouse_cell);
                if (room && room->type == room_info[othervar.queue_win_room].type)
                    compute_room_pointed(othervar.mouse_cell);
                break;
            case MM_EDIT_ROOM:
                compute_room_pointed(othervar.mouse_cell);
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}
