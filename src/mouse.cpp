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
extern Event D_001549d0[];
extern Slot D_001581b2[];
}

extern u8 clicked_icon;
extern s16 mcurx;
extern s16 mcury;

void process_no_button(u16 icon);
void process_left_button(u16 icon);
void process_mleft_button(u16 icon);
void process_right_button(u16 icon);
void process_mright_button(u16 icon);
void peep_icon(Thing *const w);
u8 is_person_highlighted_for_vaccination(u8 mode);
void panel_l_button(const u16 icon);
void clicked_for_edit(void);

#define PARTNER(t)                                                                                         \
    ((t)->object && (thing[(t)->object].type == THING_OBJECT || thing[(t)->object].type == THING_DOOR)       \
             && (u16)((t) - thing) == thing[(t)->object].user                                      \
         ? (t)->object                                                                                  \
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
void draw_from_frame(const u16 anim, const s16 x, const s16 y, u8 *remap, const u8 flip);


// FUNCTION: 0x00059d00
void process_mouse(void)
{
    Thing *hovered;
    Thing *used_object;
    u16 scroll;
    u8 icon;

    if (gamevar.unknown21) {
        lbDisplay.left_button = lbDisplay.right_button = 0;
        return;
    }
    if (gamevar.f12e == MM_HELP) {
        if (lbDisplay.left_button || lbDisplay.right_button) {
            lbDisplay.left_button = 0;
            lbDisplay.right_button = 0;
            if (othervar.f8a8 <= lbDisplay.mouse_x && othervar.f8a8 + pulldv[19].width > lbDisplay.mouse_x
                && othervar.f8aa <= lbDisplay.mouse_y && othervar.f8aa + pulldv[19].height > lbDisplay.mouse_y)
                finish_composite_help();
        }
        return;
    }
    if (debug.unknownd7) {}
    scroll = 0;
    if (lbDisplay.mmouse_x <= resvars.f1c[0xa7].x)
        scroll |= 4;
    else if (lbDisplay.mmouse_x >= resvars.f1c[0xa8].x)
        scroll |= 8;
    if (lbDisplay.mmouse_y <= resvars.f1c[0xa7].y)
        scroll |= 1;
    else if (lbDisplay.mmouse_y >= resvars.f1c[0xa8].y)
        scroll |= 2;
    if (scroll > 0)
        D_001549d0[player].flags = scroll;
    othervar.f43 = 0;
    othervar.fa53 = 0;
    if (othervar.f8a && !othervar.f9c2) {
        lbDisplay.left_button = 0;
        if (!othervar.f8b) {
            set_mouse_sprite(1);
            othervar.f8b = 1;
        }
        return;
    }
    if (othervar.f8b) {
        if (othervar.f30 == 1)
            set_mouse_sprite(othervar.f31);
        othervar.f8b = 0;
    }
    if (CountWindows(open_windows)) {
        if (MouseOnWindow(open_windows[0])) {
            lbDisplay.left_button = lbDisplay.right_button = 0;
            return;
        }
    }
    if (statvar.unknown6 <= lbDisplay.mmouse_y) {
        icon = which_icon_is_mouse_over();
        if (gamevar.f12e == MM_NORMAL) {
            if (othervar.f30 != 1)
                set_mouse_sprite(1);
        }
    } else if (advisor_up_states[advisor.f11e] && lbDisplay.right_button
               && ((lbDisplay.mmouse_y >= resvars.f1c[0x93].y && advisor.f125_u16 <= lbDisplay.mmouse_x && lbDisplay.mmouse_x <= resvars.f1c[0x92].x
                    && lbDisplay.mmouse_y <= resvars.f1c[0x92].y)
                   || (lbDisplay.mmouse_x >= resvars.f1c[0x90].x && lbDisplay.mmouse_y >= resvars.f1c[0x90].y
                       && lbDisplay.mmouse_x <= resvars.f1c[0x91].x && lbDisplay.mmouse_y <= resvars.f1c[0x91].y))) {
        othervar.fa53 = 1;
        gamevar.f130 = 8;
        icon = 0x27;
    } else {
        icon = 0x27;
        switch (gamevar.f12e) {
        case MM_VACCINATE:
            if (is_person_highlighted_for_vaccination(1)) {
                gamevar.f130 = 1;
                set_mouse_sprite(0xd);
            } else {
                gamevar.f130 = 2;
                set_mouse_sprite(0xc);
            }
            break;
        case MM_NORMAL: {
            u8 hit;
            u8 set;

            hit = 0;
            set = 0;
            if (othervar.f9c2) {
                if (gamevar.f130 != 7)
                    set_mouse_sprite(1);
                gamevar.f130 = 7;
                set = 1;
            } else if (othervar.f6f) {
                if (gamevar.f130 != 6)
                    set_mouse_sprite(0xb);
                gamevar.f130 = 6;
                set = 1;
            } else if (othervar.f71) {
                if (gamevar.f130 != 5)
                    set_mouse_sprite(0xa);
                gamevar.f130 = 5;
                set = 1;
            } else if (crequester.unknown2 == 0x63 && (hovered = &thing[othervar.f41]) > thing) {
                if (gamevar.f174)
                    return;
                if ((hovered->type == THING_PATIENT || hovered->type == THING_STAFF)
                    && (used_object = &thing[hovered->object
                                               && (thing[hovered->object].type == THING_OBJECT || thing[hovered->object].type == THING_DOOR)
                                               && (u16)(hovered - thing) == thing[hovered->object].user
                                           ? hovered->object
                                           : 0])
                           > thing) {
                    if (used_object->subtype == OBJ_RECEPTION_DESK) {
                        othervar.f7d = othervar.f7c;
                        othervar.f7c = 4;
                        gamevar.f130 = 3;
                        hit = 1;
                        peep_icon(hovered);
                        if (othervar.f30 != 0x13)
                            set_mouse_sprite(0x13);
                        goto done;
                    }
                    if (objects[used_object->subtype].f20)
                        hovered = used_object;
                }
                if (hovered->type == THING_OBJECT) {
                    if (hovered->pair && objects[thing[hovered->pair].subtype].unknown17 != -1)
                        hovered = &thing[hovered->pair];
                    if (objects[hovered->subtype].f20) {
                        othervar.f7d = othervar.f7c;
                        othervar.f7c = 3;
                        gamevar.f130 = 3;
                        if (othervar.f30 != 0x11)
                            set_mouse_sprite(0x11);
                        hit = 1;
                        goto done;
                    }
                    if (objects[hovered->subtype].unknown20[2] & 2) {
                        othervar.f7d = othervar.f7c;
                        othervar.f7c = 6;
                        gamevar.f130 = 3;
                        hit = 1;
                        goto done;
                    }
                }
                hovered = &thing[othervar.f41];
                if (hovered->type == THING_OBJECT && (hovered->subtype == OBJ_SWING_DOOR || hovered->subtype == OBJ_SWING_DOOR2) && hovered->door_main)
                    hovered = &thing[hovered->door_main];
                if (hovered->type == THING_PATIENT) {
                    othervar.f7d = othervar.f7c;
                    othervar.f7c = 2;
                    gamevar.f130 = 3;
                    peep_icon(hovered);
                    hit = 1;
                    if (othervar.f30 != 0x12)
                        set_mouse_sprite(0x12);
                    if (myhosp->f20d && is_person_highlighted_for_vaccination(0)) {
                        gamevar.f130 = 1;
                        set_mouse_sprite(0xd);
                    }
                } else if (hovered->type == THING_STAFF) {
                    othervar.f7d = othervar.f7c;
                    othervar.f7c = 1;
                    gamevar.f130 = 3;
                    peep_icon(hovered);
                    hit = 1;
                    if (othervar.f30 != 0x10)
                        set_mouse_sprite(0x10);
                } else if (hovered->type == THING_DOOR) {
                    RoomInfo *room;

                    room = &room_info[hovered->room_id];
                    if (rooms[room->type].unknown27[2]) {
                        othervar.f7d = othervar.f7c;
                        othervar.f7c = 4;
                        gamevar.f130 = 4;
                        hit = 1;
                        if (othervar.f30 != 0x13)
                            set_mouse_sprite(0x13);
                    }
                } else if (hovered->type == THING_MAYOR || hovered->type == THING_INSPECTOR) {
                    othervar.f7d = othervar.f7c;
                    othervar.f7c = 5;
                    gamevar.f130 = 3;
                    hit = 1;
                }
            }
        done:
            if (hit) {
                othervar.f80 = 100;
                othervar.f7e = hovered - thing;
            } else if (!set && (othervar.f30 != 1 || gamevar.f130 != 0)) {
                set_mouse_sprite(1);
                gamevar.f130 = 0;
            }
            if (othervar.f80 != 0) {
                othervar.f80--;
                if (othervar.f80 == 0) {
                    othervar.f7d = othervar.f7c;
                    othervar.f7c = 0;
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

// FUNCTION: 0x0005a750
void process_left_button(u16 icon)
{
    Thing *clicked;

    cancel_helpmsg();
    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            switch (gamevar.f12e) {
            case MM_NORMAL:
                if (crequester.unknown2 != 0x63)
                    break;
                if (myhosp->f207) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        gamevar.f12e = MM_VACCINATE;
                        play_sound(10, SFX_SELECTX, -1, -1);
                        break;
                    }
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                     watch[13].width, watch[13].height)) {
                        if (othervar.fa90) {
                            othervar.f38 = 1;
                            D_001581b2[1].value = othervar.fa90;
                            othervar.f10 = 1;
                            play_sound(10, SFX_CAMCLICK, -1, -1);
                        }
                        break;
                    }
                }
                if (myhosp->unknown1257) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        D_001549d0[player].type = EV_ANNOUNCE;
                        play_sound(10, SFX_FANFARE, -1, -1);
                        break;
                    }
                }
                if (myhosp->f1d9) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x84].x, resvars.f1c[0x84].y,
                                     watch[13].width, watch[13].height)) {
                        if (othervar.fa88) {
                            othervar.f38 = 1;
                            D_001581b2[1].value = othervar.fa88;
                            othervar.f10 = 1;
                            play_sound(10, SFX_CAMCLICK, -1, -1);
                        }
                        break;
                    }
                }
                switch (gamevar.f130) {
                case 0:
                    break;
                case 3:
                    clicked = &thing[othervar.f7e];
                    if (clicked->owner == player) {
                        switch (clicked->type) {
                        case THING_OBJECT:
                            switch (clicked->subtype) {
                            case OBJ_RECEPTION_DESK: {
                                Thing *receptionist;

                                receptionist = &thing[clicked->user];
                                if (receptionist > thing) {
                                    if (receptionist->room) {
                                        D_001549d0[player].type = 0x3b;
                                        D_001549d0[player].value = receptionist->room - room_info;
                                    }
                                }
                                break;
                            }
                            default:
                                if (!(objects[clicked->subtype].unknown20[2] & 4)) {
                                    D_001549d0[player].type = 0x42;
                                    D_001549d0[player].value = othervar.f7e;
                                }
                                break;
                            }
                            break;
                        case THING_STAFF:
                            switch (clicked->subtype) {
                            case STAFF_RECEPTIONIST:
                                if (PARTNER(&thing[othervar.f7e]) && thing[PARTNER(&thing[othervar.f7e])].subtype == OBJ_RECEPTION_DESK) {
                                    if (thing[othervar.f7e].room) {
                                        D_001549d0[player].type = 0x3b;
                                        D_001549d0[player].value = thing[othervar.f7e].room - room_info;
                                        break;
                                    }
                                }
                            default:
                                if (crequester.unknown2 == 0x63) {
                                    close_irequesters();
                                    if ((clicked->orders & ORD_RESIGN) && clicked->playercall) {
                                        D_001549d0[player].type = 0xa;
                                        D_001549d0[player].value = 1;
                                        D_001549d0[player].arg = clicked->playercall;
                                    } else {
                                        D_001549d0[player].type = 0x42;
                                        D_001549d0[player].value = othervar.f7e;
                                    }
                                }
                                break;
                            }
                            break;
                        case THING_PATIENT:
                            if (crequester.unknown2 == 0x63) {
                                close_irequesters();
                                if ((clicked->flags1 & 4) && clicked->playercall) {
                                    D_001549d0[player].type = 0xa;
                                    D_001549d0[player].value = 8;
                                    D_001549d0[player].arg = clicked->playercall;
                                } else if ((clicked->flags0 & PF_AWAITING_DECISION) && clicked->playercall) {
                                    D_001549d0[player].type = 0xa;
                                    D_001549d0[player].value = 4;
                                    D_001549d0[player].arg = clicked->playercall;
                                } else {
                                    D_001549d0[player].type = 0x42;
                                    D_001549d0[player].value = othervar.f7e;
                                }
                            }
                            break;
                        default:
                            D_001549d0[player].type = 0x42;
                            D_001549d0[player].value = othervar.f7e;
                            break;
                        }
                    } else if (statvar.unknownfd > 1 && clicked->type == THING_STAFF && clicked->owner != 0x63) {
                        if (clicked->poach_offers) {
                            if (clicked->poach_offers != 1 << player)
                                advisor.f2 = 0xf;
                        } else if (clicked->discontent_s < gbv.f10a / 4) {
                            advisor.f2 = 0x10;
                        } else {
                            D_001549d0[player].type = 0x81;
                            D_001549d0[player].value = clicked - thing;
                            advisor.f2 = 0x11;
                        }
                    }
                    break;
                case 4:
                    if (thing[othervar.f7e].owner == player) {
                        D_001549d0[player].type = 0x3b;
                        D_001549d0[player].value = thing[othervar.f7e].room_id;
                        D_001549d0[player].arg = (thing[othervar.f7e].y.tile << 7) + thing[othervar.f7e].x.tile;
                        compute_room_pointed(othervar.f28);
                    }
                    break;
                case 5:
                case 6:
                    if (!gamevar.unknown21) {
                        D_001549d0[player].type = 0x6a;
                        D_001549d0[player].value = othervar.f6f;
                        D_001549d0[player].argb[0] = gamevar.f175;
                        if (othervar.f6f)
                            D_001549d0[player].argb[1] = 1;
                        else
                            D_001549d0[player].argb[1] = 0;
                        if (othervar.fa77) {
                            othervar.fa77--;
                            if (othervar.fa77 <= 0) {
                                gamevar.f175 = 0;
                                othervar.f9e7 = 2;
                                othervar.fa77 = 0;
                            }
                        }
                    }
                    break;
                case 7:
                    if (crequester.unknown2 == 0x63) {
                        close_irequesters();
                        if (othervar.f9c2) {
                            D_001549d0[player].type = 0xa;
                            D_001549d0[player].value = (u16)othervar.f9c2;
                            D_001549d0[player].arg = 0;
                        }
                    }
                    lbDisplay.left_button = lbDisplay.right_button = 0;
                    break;
                case 1:
                    D_001549d0[player].type = EV_TOGGLE_VACCINATION;
                    D_001549d0[player].value = othervar.f7a;
                    break;
                case 2:
                default:
                    break;
                }
                break;
            case MM_BLUEPRINT:
                D_001549d0[player].type = EV_MODIFY_BLUEPRINT;
                D_001549d0[player].value = othervar.f28;
                break;
            case 2:
                D_001549d0[player].type = EV_PLACE_BP_DOOR;
                D_001549d0[player].value = 0;
                break;
            case MM_PLACE_STAFF:
                draw_staff_cursor();
                if (gamevar.unknown72[0]) {
                    D_001549d0[player].type = 5;
                    D_001549d0[player].value = othervar.f28;
                    D_001549d0[player].arg = gamevar.unknown75;
                }
                break;
            case MM_PLACE_OBJECT:
                draw_object_cursor();
                if (gamevar.unknown2a) {
                    D_001549d0[player].type = EV_PLACE_OBJECT;
                    D_001549d0[player].value = gamevar.f23;
                    D_001549d0[player].argb[0] = gamevar.unknown25;
                    D_001549d0[player].argb[1] = gamevar.f28;
                }
                break;
            case 6:
                D_001549d0[player].type = EV_PLACE_SMALL_THING;
                D_001549d0[player].value = othervar.f28;
                D_001549d0[player].arg = (u16)gamevar.unknown1e;
                break;
            case 0xb:
                othervar.f43 = compute_object_pointed(&room_info[allhosps[player].unknown11b7]);
                if (othervar.f43) {
                    if (thing[othervar.f43].room_id == allhosps[player].unknown11b7) {
                        D_001549d0[player].type = EV_PICK_UP_OBJECT;
                        D_001549d0[player].value = othervar.f43;
                        D_001549d0[player].arg = 1;
                    }
                }
                break;
            case MM_EDIT_ROOM:
                if (map[othervar.f28].unknown7 == 1 << player) {
                    compute_room_pointed(othervar.f28);
                    clicked_for_edit();
                }
                break;
            case MM_VACCINATE:
                if (myhosp->f207) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        gamevar.f12e = MM_NORMAL;
                        break;
                    }
                }
                if (gamevar.f130 == 1) {
                    D_001549d0[player].type = EV_TOGGLE_VACCINATION;
                    D_001549d0[player].value = othervar.f7a;
                }
                break;
            case 0x12: {
                RoomInfo *room;

                room = &room_info[allhosps[player].f11b9];
                if (room == 0) {
                    Thing *selected;

                    selected = &thing[othervar.f43];
                    if (selected > thing) {
                        if (selected->type == THING_STAFF && selected->subtype == STAFF_RECEPTIONIST)
                            room = selected->room;
                        else if (selected->type == THING_OBJECT && selected->subtype == OBJ_RECEPTION_DESK && selected->user)
                            room = thing[selected->user].room;
                    }
                }
                if (room) {
                    D_001549d0[player].type = EV_QUEUE_PERSON_TO_NEW_QUEUE;
                    D_001549d0[player].value = room - room_info;
                    D_001549d0[player].arg = gamevar.f13a;
                    set_mouse_mode(MM_NORMAL);
                }
                break;
            }
            case 4:
            case 7:
            case 8:
            case 9:
            case 10:
            case MM_NAV_DEBUG:
            case 0xe:
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
// FUNCTION: 0x0005b70c
void process_mleft_button(u16 icon)
{
    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            if (D_001549d0[player].type == EV_NONE) {
                switch (gamevar.f12e) {
                case MM_BLUEPRINT:
                    D_001549d0[player].type = EV_MODIFY_BLUEPRINT;
                    D_001549d0[player].value = othervar.f28;
                    break;
                case MM_NORMAL:
                case MM_VACCINATE:
                    if (myhosp->f207) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                         watch[14].width, watch[14].height))
                            reqspr_draw(resvars.f1c[0x86].x, resvars.f1c[0x86].y, &watch[15]);
                    }
                    if (myhosp->f1d9) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x84].x, resvars.f1c[0x84].y,
                                         watch[13].width, watch[13].height)) {
                            if (othervar.fa88) {
                                othervar.f38 = 1;
                                D_001581b2[1].value = othervar.fa88;
                                othervar.f10 = 1;
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

// FUNCTION: 0x0005b8f0
void process_mright_button(u16 icon)
{
    switch (icon) {
    case 0x27:
        switch (gamevar.f12e) {
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

// FUNCTION: 0x0005b95c
void process_right_button(u16 icon)
{
    Thing *hovered;

    if (!gamevar.f174) {
        switch (icon) {
        case 0x27:
            if (display_mode == 0) {
                switch (gamevar.f12e) {
                case MM_NORMAL:
                    if (myhosp->f207) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x86].x, resvars.f1c[0x86].y,
                                         watch[13].width, watch[13].height)) {
                            othervar.fa90 = find_next_emergency_person(othervar.fa88, 0x20);
                            if (othervar.fa90) {
                                D_001549d0[player].type = 0x42;
                                D_001549d0[player].value = othervar.fa90;
                                play_sound(10, SFX_CAMCLICK, -1, -1);
                            } else {
                                D_001549d0[player].type = EV_CLOSE_QUERY;
                                D_001549d0[player].value = 1;
                                play_sound(10, SFX_WRONG2, -1, -1);
                            }
                            break;
                        }
                    }
                    if (myhosp->f1d9) {
                        if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.f1c[0x84].x, resvars.f1c[0x84].y,
                                         watch[13].width, watch[13].height)) {
                            othervar.fa88 = find_next_emergency_person(othervar.fa88, 0x40);
                            if (othervar.fa88) {
                                D_001549d0[player].type = 0x42;
                                D_001549d0[player].value = othervar.fa88;
                                play_sound(10, SFX_CAMCLICK, -1, -1);
                            } else {
                                D_001549d0[player].type = EV_CLOSE_QUERY;
                                D_001549d0[player].value = 1;
                                play_sound(10, SFX_WRONG2, -1, -1);
                            }
                            break;
                        }
                    }
                    {
                        switch (gamevar.f130) {
                        case 0:
                        case 3:
                            if (!mouse_was_on_window
                                && (!allhosps[player].unknown11b7 || crequester.unknown2 != 5)
                                && (crequester.unknown2 != 0x63 || othervar.f9b8 != 0)) {
                                process_keys(0, 1);
                            } else {
                                hovered = &thing[othervar.f41];
                                if (hovered->owner == player) {
                                    if (allhosps[player].unknown11b7) {
                                        check_object_right_click_in_room_edit();
                                    } else if (gamevar.f130 == 3 && hovered > thing && hovered->type == THING_STAFF) {
                                        D_001549d0[player].type = 0x68;
                                        D_001549d0[player].value = othervar.f41;
                                        play_sound(10, SFX_PICKUP, -1, -1);
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
                            if (othervar.fa53) {
                                othervar.fa53 = 0;
                                othervar.fa54 = 1;
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
                case 2:
                    D_001549d0[player].type = EV_PLACE_BP_DOOR;
                    D_001549d0[player].value = 1;
                    break;
                case MM_PLACE_OBJECT:
                    if (gamevar.unknown26 != 1) {
                        if (place_object_change_angle(-1))
                            play_sound(10, SFX_SWOOSH, -1, -1);
                    }
                    draw_object_cursor();
                    break;
                case MM_PLACE_STAFF:
                case MM_EDIT_ROOM:
                case MM_VACCINATE:
                    if (!mouse_was_on_window)
                        process_keys(0, 1);
                    break;
                case 0xe:
                    D_001549d0[player].type = 0x7b;
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

// FUNCTION: 0x0005be7c
void process_no_button(u16 icon)
{
    RoomInfo *room;
    Thing *hovered;

    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            switch (gamevar.f12e) {
            case MM_NORMAL:
                room = which_room(othervar.f28);
                if (room && room->owner == player) {
                    flag_all_queue_members(room);
                } else {
                    hovered = &thing[othervar.f41];
                    if (hovered > thing && hovered->owner == player) {
                        if ((hovered->type == THING_OBJECT && hovered->subtype == OBJ_RECEPTION_DESK) || (hovered->type == THING_STAFF && hovered->subtype == STAFF_RECEPTIONIST))
                            flag_all_queue_members(thing[othervar.f7e].room);
                    }
                }
                break;
            case MM_BLUEPRINT:
                if (D_001549d0[player].type == EV_NONE) {
                    D_001549d0[player].type = 8;
                    D_001549d0[player].value = othervar.f28;
                }
                break;
            case 2:
                D_001549d0[player].type = EV_TEMP_PLACE_DOOR;
                D_001549d0[player].value = othervar.f2a;
                D_001549d0[player].arg = 0;
                break;
            case MM_PLACE_STAFF:
                draw_staff_cursor();
                break;
            case MM_PLACE_OBJECT:
                draw_object_cursor();
                break;
            case 6:
                LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                draw_from_frame(start[blueprint[gamevar.unknown1e].anim].frame, resvars.f4 / 2 + mcurx,
                                mcury - resvars.f3 / 2, 0, start[blueprint[gamevar.unknown1e].anim].flags);
                break;
            case 0xb:
                othervar.f43 = compute_object_pointed(&room_info[allhosps[player].unknown11b7]);
                break;
            case 0x12:
                room = which_room(othervar.f28);
                if (room && room->type == room_info[othervar.f61].type)
                    compute_room_pointed(othervar.f28);
                break;
            case MM_EDIT_ROOM:
                compute_room_pointed(othervar.f28);
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
