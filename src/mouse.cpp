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
extern Point *D_0015f410;
extern u16 D_0015536e;
extern u16 D_00155370;
extern Event D_001549d0[];
extern u16 D_001553e0;
extern u8 D_001553e1[];
extern u16 D_001553f9;
extern u16 D_001553fb;
extern u16 D_00155436;
extern u16 D_00155419;
extern u16 D_00155e40;
extern u16 D_00155e48;
extern u8 D_001553f0;
extern u8 D_001553c8;
extern u8 D_001553b4;
extern u8 D_00155d70;
extern u8 D_00155e0b;
extern u8 D_00155e0c;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern volatile s32 D_00188c5c, D_00188c60;
extern Slot D_001581b2[];
extern u8 D_0015f3f7;
extern u8 D_0015f3f8;
extern s16 D_00155c5e[];
extern s16 D_00155c60[];
extern u16 D_0012cdea[];
extern u16 D_00158321;
extern u8 D_0015831a;
extern u8 D_00155442;
extern u8 D_00155443;
extern u8 D_001553e8;
extern u8 D_001553e9;
extern u8 D_00155d7a;
extern u16 D_00155427;
extern u8 D_00155429;
extern u8 D_00155434;
extern u8 D_00155435;
extern u16 D_00155438;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u16 D_00155432;
extern u8 D_00155e2f;
extern u8 D_001553b5;
extern u16 D_00155d9f;
extern u16 D_001581fe;
extern u16 D_0015537a;
extern s16 D_0012dd7c[];
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
    ((t)->unknown69 && (thing[(t)->unknown69].renderer == 6 || thing[(t)->unknown69].renderer == 8)       \
             && (u16)((t) - thing) == thing[(t)->unknown69].unknown91                                      \
         ? (t)->unknown69                                                                                  \
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


// NONMATCHING: register choice, jump layout of two && conditions
// FUNCTION: 0x00059d00
void process_mouse(void)
{
    Thing *w;
    Thing *other;
    u16 scroll;
    u8 icon;

    if (gamevar.unknown21) {
        D_00188c7b = D_00188c7c = 0;
        return;
    }
    if (D_0015536e == 0x25) {
        if (D_00188c7b || D_00188c7c) {
            D_00188c7b = 0;
            D_00188c7c = 0;
            if (D_00155c5e[1] <= D_00188c5c && pulldv[19].width + D_00155c5e[1] > D_00188c5c
                && D_00155c60[1] <= D_00188c60 && pulldv[19].height + D_00155c60[1] > D_00188c60)
                finish_composite_help();
        }
        return;
    }
    if (debug.unknownd7) {}
    scroll = 0;
    if (mouse_dx <= D_0015f410[0xa7].x)
        scroll |= 4;
    else if (mouse_dx >= D_0015f410[0xa8].x)
        scroll |= 8;
    if (mouse_dy <= D_0015f410[0xa7].y)
        scroll |= 1;
    else if (mouse_dy >= D_0015f410[0xa8].y)
        scroll |= 2;
    if (scroll > 0)
        D_001549d0[player].flags = scroll;
    D_001553fb = 0;
    D_00155e0b = 0;
    if (D_00155442 && !D_00155d7a) {
        D_00188c7b = 0;
        if (!D_00155443) {
            set_mouse_sprite(1);
            D_00155443 = 1;
        }
        return;
    }
    if (D_00155443) {
        if (D_001553e8 == 1)
            set_mouse_sprite(D_001553e9);
        D_00155443 = 0;
    }
    if (CountWindows(open_windows)) {
        if (MouseOnWindow(open_windows[0])) {
            D_00188c7b = D_00188c7c = 0;
            return;
        }
    }
    if (statvar.unknown6 <= mouse_dy) {
        icon = which_icon_is_mouse_over();
        if (D_0015536e == 0 && D_001553e8 != 1)
            set_mouse_sprite(1);
    } else if (D_0012cdea[D_0015831a] && D_00188c7c
               && ((mouse_dy >= D_0015f410[0x93].y && D_00158321 <= mouse_dx && mouse_dx <= D_0015f410[0x92].x
                    && mouse_dy <= D_0015f410[0x92].y)
                   || (mouse_dx >= D_0015f410[0x90].x && mouse_dy >= D_0015f410[0x90].y
                       && mouse_dx <= D_0015f410[0x91].x && mouse_dy <= D_0015f410[0x91].y))) {
        D_00155e0b = 1;
        D_00155370 = 8;
        icon = 0x27;
    } else {
        icon = 0x27;
        switch (D_0015536e) {
        case 0xf:
            if (is_person_highlighted_for_vaccination(1)) {
                D_00155370 = 1;
                set_mouse_sprite(0xd);
            } else {
                D_00155370 = 2;
                set_mouse_sprite(0xc);
            }
            break;
        case 0: {
            u8 hit;
            u8 set;

            hit = 0;
            set = 0;
            if (D_00155d7a) {
                if (D_00155370 != 7)
                    set_mouse_sprite(1);
                D_00155370 = 7;
                set = 1;
            } else if (D_00155427) {
                if (D_00155370 != 6)
                    set_mouse_sprite(0xb);
                D_00155370 = 6;
                set = 1;
            } else if (D_00155429) {
                if (D_00155370 != 5)
                    set_mouse_sprite(0xa);
                D_00155370 = 5;
                set = 1;
            } else if (crequester.unknown2 == 0x63 && (w = &thing[D_001553f9]) > thing) {
                if (D_001553b4)
                    return;
                if (w->renderer == 2 || w->renderer == 4) {
                    other = &thing[w->unknown69
                                           && (thing[w->unknown69].renderer == 6 || thing[w->unknown69].renderer == 8)
                                           && (u16)(w - thing) == thing[w->unknown69].unknown91
                                       ? w->unknown69
                                       : 0];
                    if (other > thing) {
                        if (other->subtype == 0xb) {
                            D_00155435 = D_00155434;
                            D_00155434 = 4;
                            D_00155370 = 3;
                            hit = 1;
                            peep_icon(w);
                            if (D_001553e8 != 0x13)
                                set_mouse_sprite(0x13);
                            goto done;
                        }
                        if (objects[other->subtype].f20)
                            w = other;
                    }
                }
                if (w->renderer == 6) {
                    if (w->group && objects[thing[w->group].subtype].unknown17 != -1)
                        w = &thing[w->group];
                    if (objects[w->subtype].f20) {
                        D_00155435 = D_00155434;
                        D_00155434 = 3;
                        D_00155370 = 3;
                        if (D_001553e8 != 0x11)
                            set_mouse_sprite(0x11);
                        hit = 1;
                        goto done;
                    }
                    if (objects[w->subtype].unknown20[2] & 2) {
                        D_00155435 = D_00155434;
                        D_00155434 = 6;
                        D_00155370 = 3;
                        hit = 1;
                        goto done;
                    }
                }
                w = &thing[D_001553f9];
                if (w->renderer == 6 && (w->subtype == 0x34 || w->subtype == 0x35) && w->unknown102)
                    w = &thing[w->unknown102];
                if (w->renderer == 2) {
                    D_00155435 = D_00155434;
                    D_00155434 = 2;
                    D_00155370 = 3;
                    peep_icon(w);
                    hit = 1;
                    if (D_001553e8 != 0x12)
                        set_mouse_sprite(0x12);
                    if (myhosp->f20d && is_person_highlighted_for_vaccination(0)) {
                        D_00155370 = 1;
                        set_mouse_sprite(0xd);
                    }
                } else if (w->renderer == 4) {
                    D_00155435 = D_00155434;
                    D_00155434 = 1;
                    D_00155370 = 3;
                    peep_icon(w);
                    hit = 1;
                    if (D_001553e8 != 0x10)
                        set_mouse_sprite(0x10);
                } else if (w->renderer == 8) {
                    RoomInfo *room;

                    room = &room_info[w->unknown89];
                    if (rooms[room->type].unknown27[2]) {
                        D_00155435 = D_00155434;
                        D_00155434 = 4;
                        D_00155370 = 4;
                        hit = 1;
                        if (D_001553e8 != 0x13)
                            set_mouse_sprite(0x13);
                    }
                } else if (w->renderer == 0x14 || w->renderer == 0x16) {
                    D_00155435 = D_00155434;
                    D_00155434 = 5;
                    D_00155370 = 3;
                    hit = 1;
                }
            }
        done:
            if (hit) {
                D_00155438 = 100;
                D_00155436 = w - thing;
            } else if (!set && (D_001553e8 != 1 || D_00155370 != 0)) {
                set_mouse_sprite(1);
                D_00155370 = 0;
            }
            if (D_00155438 != 0) {
                D_00155438--;
                if (D_00155438 == 0) {
                    D_00155435 = D_00155434;
                    D_00155434 = 0;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    if (D_00188c7b + D_00188c7e + D_00188c7c + D_00188c7f != 0) {
        if (icon == 0x27) {
            if (D_00188c7b) {
                process_left_button((u16)icon);
                D_00188c7b = 0;
            } else if (D_00188c7e) {
                process_mleft_button((u16)icon);
            }
        } else if (D_00188c7e) {
            process_mleft_button((u16)icon);
        } else if (D_00188c7b) {
            process_left_button((u16)icon);
            D_00188c7b = 0;
        }
        if (D_00188c7c) {
            process_right_button((u16)icon);
            D_00188c7c = 0;
        } else if (D_00188c7f) {
            process_mright_button((u16)icon);
        }
    } else {
        process_no_button((u16)icon);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0005a750
void process_left_button(u16 icon)
{
    Thing *w;

    cancel_helpmsg();
    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            switch (D_0015536e) {
            case 0:
                if (crequester.unknown2 != 0x63)
                    break;
                if (myhosp->f207) {
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        D_0015536e = 0xf;
                        play_sound(10, 0x43, -1, -1);
                        break;
                    }
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                     watch[13].width, watch[13].height)) {
                        if (D_00155e48) {
                            D_001553f0 = 1;
                            D_001581b2[1].value = D_00155e48;
                            D_001553c8 = 1;
                            play_sound(10, 0xbb, -1, -1);
                        }
                        break;
                    }
                }
                if (myhosp->unknown1257) {
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        D_001549d0[player].type = 0x8b;
                        play_sound(10, 0xd9, -1, -1);
                        break;
                    }
                }
                if (myhosp->f1d9) {
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x84].x, D_0015f410[0x84].y,
                                     watch[13].width, watch[13].height)) {
                        if (D_00155e40) {
                            D_001553f0 = 1;
                            D_001581b2[1].value = D_00155e40;
                            D_001553c8 = 1;
                            play_sound(10, 0xbb, -1, -1);
                        }
                        break;
                    }
                }
                switch (D_00155370) {
                case 0:
                    break;
                case 3:
                    w = &thing[D_00155436];
                    if (w->owner == player) {
                        switch (w->renderer) {
                        case 6:
                            switch (w->subtype) {
                            case 0xb: {
                                Thing *other;

                                other = &thing[w->unknown91];
                                if (other > thing) {
                                    if (other->room) {
                                        D_001549d0[player].type = 0x3b;
                                        D_001549d0[player].value = other->room - room_info;
                                    }
                                }
                                break;
                            }
                            default:
                                if (!(objects[w->subtype].unknown20[2] & 4)) {
                                    D_001549d0[player].type = 0x42;
                                    D_001549d0[player].value = D_00155436;
                                }
                                break;
                            }
                            break;
                        case 4:
                            switch (w->subtype) {
                            case 3:
                                if (PARTNER(&thing[D_00155436]) && thing[PARTNER(&thing[D_00155436])].subtype == 0xb) {
                                    if (thing[D_00155436].room) {
                                        D_001549d0[player].type = 0x3b;
                                        D_001549d0[player].value = thing[D_00155436].room - room_info;
                                        break;
                                    }
                                }
                            default:
                                if (crequester.unknown2 == 0x63) {
                                    close_irequesters();
                                    if ((w->f8e & 0x20) && w->unknown91) {
                                        D_001549d0[player].type = 0xa;
                                        D_001549d0[player].value = 1;
                                        D_001549d0[player].arg = w->unknown91;
                                    } else {
                                        D_001549d0[player].type = 0x42;
                                        D_001549d0[player].value = D_00155436;
                                    }
                                }
                                break;
                            }
                            break;
                        case 2:
                            if (crequester.unknown2 == 0x63) {
                                close_irequesters();
                                if ((w->f75 & 4) && w->unknown91) {
                                    D_001549d0[player].type = 0xa;
                                    D_001549d0[player].value = 8;
                                    D_001549d0[player].arg = w->unknown91;
                                } else if ((w->f74 & 0x10) && w->unknown91) {
                                    D_001549d0[player].type = 0xa;
                                    D_001549d0[player].value = 4;
                                    D_001549d0[player].arg = w->unknown91;
                                } else {
                                    D_001549d0[player].type = 0x42;
                                    D_001549d0[player].value = D_00155436;
                                }
                            }
                            break;
                        default:
                            D_001549d0[player].type = 0x42;
                            D_001549d0[player].value = D_00155436;
                            break;
                        }
                    } else if (statvar.unknownfd > 1 && w->renderer == 4 && w->owner != 0x63) {
                        if (w->f93) {
                            if (w->f93 != 1 << player)
                                D_001581fe = 0xf;
                        } else if ((s16)w->unknown128 < D_0012dd7c[7] / 4) {
                            D_001581fe = 0x10;
                        } else {
                            D_001549d0[player].type = 0x81;
                            D_001549d0[player].value = w - thing;
                            D_001581fe = 0x11;
                        }
                    }
                    break;
                case 4:
                    if (thing[D_00155436].owner == player) {
                        D_001549d0[player].type = 0x3b;
                        D_001549d0[player].value = thing[D_00155436].unknown89;
                        D_001549d0[player].arg = (thing[D_00155436].y.tile << 7) + thing[D_00155436].x.tile;
                        compute_room_pointed(D_001553e0);
                    }
                    break;
                case 5:
                case 6:
                    if (!gamevar.unknown21) {
                        D_001549d0[player].type = 0x6a;
                        D_001549d0[player].value = D_00155427;
                        D_001549d0[player].argb[0] = D_001553b5;
                        if (D_00155427)
                            D_001549d0[player].argb[1] = 1;
                        else
                            D_001549d0[player].argb[1] = 0;
                        if (D_00155e2f) {
                            D_00155e2f--;
                            if (D_00155e2f <= 0) {
                                D_001553b5 = 0;
                                D_00155d9f = 2;
                                D_00155e2f = 0;
                            }
                        }
                    }
                    break;
                case 7:
                    if (crequester.unknown2 == 0x63) {
                        close_irequesters();
                        if (D_00155d7a) {
                            D_001549d0[player].type = 0xa;
                            D_001549d0[player].value = (u16)D_00155d7a;
                            D_001549d0[player].arg = 0;
                        }
                    }
                    D_00188c7b = D_00188c7c = 0;
                    break;
                case 1:
                    D_001549d0[player].type = 0x82;
                    D_001549d0[player].value = D_00155432;
                    break;
                case 2:
                default:
                    break;
                }
                break;
            case 1:
                D_001549d0[player].type = 2;
                D_001549d0[player].value = D_001553e0;
                break;
            case 2:
                D_001549d0[player].type = 3;
                D_001549d0[player].value = 0;
                break;
            case 3:
                draw_staff_cursor();
                if (gamevar.unknown72[0]) {
                    D_001549d0[player].type = 5;
                    D_001549d0[player].value = D_001553e0;
                    D_001549d0[player].arg = gamevar.unknown75;
                }
                break;
            case 5:
                draw_object_cursor();
                if (gamevar.unknown2a) {
                    D_001549d0[player].type = 7;
                    D_001549d0[player].value = gamevar.f23;
                    D_001549d0[player].argb[0] = gamevar.unknown25;
                    D_001549d0[player].argb[1] = gamevar.f28;
                }
                break;
            case 6:
                D_001549d0[player].type = 0xb;
                D_001549d0[player].value = D_001553e0;
                D_001549d0[player].arg = (u16)gamevar.unknown1e;
                break;
            case 0xb:
                D_001553fb = compute_object_pointed(&room_info[allhosps[player].unknown11b7]);
                if (D_001553fb && thing[D_001553fb].unknown89 == allhosps[player].unknown11b7) {
                    D_001549d0[player].type = 0x44;
                    D_001549d0[player].value = D_001553fb;
                    D_001549d0[player].arg = 1;
                }
                break;
            case 0xc:
                if (map[D_001553e0].unknown7 == 1 << player) {
                    compute_room_pointed(D_001553e0);
                    clicked_for_edit();
                }
                break;
            case 0xf:
                if (myhosp->f207) {
                    if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                     watch[14].width, watch[14].height)) {
                        D_0015536e = 0;
                        break;
                    }
                }
                if (D_00155370 == 1) {
                    D_001549d0[player].type = 0x82;
                    D_001549d0[player].value = D_00155432;
                }
                break;
            case 0x12: {
                RoomInfo *room;

                room = &room_info[allhosps[player].f11b9];
                if (room == 0) {
                    Thing *t;

                    t = &thing[D_001553fb];
                    if (t > thing) {
                        if (t->renderer == 4 && t->subtype == 3)
                            room = t->room;
                        else if (t->renderer == 6 && t->subtype == 0xb && t->unknown91)
                            room = thing[t->unknown91].room;
                    }
                }
                if (room) {
                    D_001549d0[player].type = 9;
                    D_001549d0[player].value = room - room_info;
                    D_001549d0[player].arg = D_0015537a;
                    set_mouse_mode(0);
                }
                break;
            }
            case 4:
            case 7:
            case 8:
            case 9:
            case 10:
            case 0xd:
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
// NONMATCHING: register choice, one missing jump after the D_00155e40 block
// FUNCTION: 0x0005b70c
void process_mleft_button(u16 icon)
{
    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            if (D_001549d0[player].type == 0) {
                switch (D_0015536e) {
                case 1:
                    D_001549d0[player].type = 2;
                    D_001549d0[player].value = D_001553e0;
                    break;
                case 0:
                case 0xf:
                    if (myhosp->f207) {
                        if (over_hotspot(mouse_dx, mouse_dy, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                         watch[14].width, watch[14].height))
                            reqspr_draw(D_0015f410[0x86].x, D_0015f410[0x86].y, &watch[15]);
                    }
                    if (myhosp->f1d9) {
                        if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x84].x, D_0015f410[0x84].y,
                                         watch[13].width, watch[13].height)) {
                            if (D_00155e40) {
                                D_001553f0 = 1;
                                D_001581b2[1].value = D_00155e40;
                                D_001553c8 = 1;
                            }
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
        switch (D_0015536e) {
        case 5:
            process_no_button(icon);
            break;
        }
        break;
    default:
        panel_ml_button(icon);
        break;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0005b95c
void process_right_button(u16 icon)
{
    Thing *w;

    if (!D_001553b4) {
        switch (icon) {
        case 0x27:
            if (display_mode == 0) {
                switch (D_0015536e) {
                case 0:
                    if (myhosp->f207) {
                        if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x86].x, D_0015f410[0x86].y,
                                         watch[13].width, watch[13].height)) {
                            D_00155e48 = fn_00073e64(D_00155e40, 0x20);
                            if (D_00155e48) {
                                D_001549d0[player].type = 0x42;
                                D_001549d0[player].value = D_00155e48;
                                play_sound(10, 0xbb, -1, -1);
                            } else {
                                D_001549d0[player].type = 0x43;
                                D_001549d0[player].value = 1;
                                play_sound(10, 8, -1, -1);
                            }
                            break;
                        }
                    }
                    if (myhosp->f1d9) {
                        if (over_hotspot(D_00188c5c, D_00188c60, D_0015f410[0x84].x, D_0015f410[0x84].y,
                                         watch[13].width, watch[13].height)) {
                            D_00155e40 = fn_00073e64(D_00155e40, 0x40);
                            if (D_00155e40) {
                                D_001549d0[player].type = 0x42;
                                D_001549d0[player].value = D_00155e40;
                                play_sound(10, 0xbb, -1, -1);
                            } else {
                                D_001549d0[player].type = 0x43;
                                D_001549d0[player].value = 1;
                                play_sound(10, 8, -1, -1);
                            }
                            break;
                        }
                    }
                    {
                        switch (D_00155370) {
                        case 0:
                        case 3:
                            if (!mouse_was_on_window
                                && (!allhosps[player].unknown11b7 || crequester.unknown2 != 5)
                                && (crequester.unknown2 != 0x63 || D_00155d70 != 0)) {
                                fn_00074220(0, 1);
                            } else {
                                w = &thing[D_001553f9];
                                if (w->owner == player) {
                                    if (allhosps[player].unknown11b7) {
                                        check_object_right_click_in_room_edit();
                                    } else if (D_00155370 == 3 && w > thing && w->renderer == 4) {
                                        D_001549d0[player].type = 0x68;
                                        D_001549d0[player].value = D_001553f9;
                                        play_sound(10, 0x99, -1, -1);
                                    } else {
                                        pick_up_object_right_click();
                                    }
                                }
                            }
                            break;
                        case 7:
                            D_00188c7b = D_00188c7c = 0;
                            break;
                        case 8:
                            if (D_00155e0b) {
                                D_00155e0b = 0;
                                D_00155e0c = 1;
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
                    D_001549d0[player].type = 3;
                    D_001549d0[player].value = 1;
                    break;
                case 5:
                    if (gamevar.unknown26 != 1 && place_object_change_angle(-1))
                        play_sound(10, 0x22, -1, -1);
                    draw_object_cursor();
                    break;
                case 3:
                case 0xc:
                case 0xf:
                    if (!mouse_was_on_window)
                        fn_00074220(0, 1);
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

// NONMATCHING: register choice; mcurx/mcury are read through the preceding global (sar trick)
// FUNCTION: 0x0005be7c
void process_no_button(u16 icon)
{
    RoomInfo *room;
    Thing *w;

    switch (icon) {
    case 0x27:
        if (display_mode == 0) {
            switch (D_0015536e) {
            case 0:
                room = which_room(D_001553e0);
                if (room && room->owner == player) {
                    flag_all_queue_members(room);
                } else {
                    w = &thing[D_001553f9];
                    if (w > thing && w->owner == player) {
                        if ((w->renderer == 6 && w->subtype == 0xb) || (w->renderer == 4 && w->subtype == 3))
                            flag_all_queue_members(thing[D_00155436].room);
                    }
                }
                break;
            case 1:
                if (D_001549d0[player].type == 0) {
                    D_001549d0[player].type = 8;
                    D_001549d0[player].value = D_001553e0;
                }
                break;
            case 2:
                D_001549d0[player].type = 0x1a;
                D_001549d0[player].value = *(u16 *)&D_001553e1[1];
                D_001549d0[player].arg = 0;
                break;
            case 3:
                draw_staff_cursor();
                break;
            case 5:
                draw_object_cursor();
                break;
            case 6:
                LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
                draw_from_frame(start[blueprint[gamevar.unknown1e].anim].frame, D_0015f3f8 / 2 + mcurx,
                                mcury - D_0015f3f7 / 2, 0, start[blueprint[gamevar.unknown1e].anim].flags);
                break;
            case 0xb:
                D_001553fb = compute_object_pointed(&room_info[allhosps[player].unknown11b7]);
                break;
            case 0x12:
                room = which_room(D_001553e0);
                if (room && room->type == room_info[D_00155419].type)
                    compute_room_pointed(D_001553e0);
                break;
            case 0xc:
                compute_room_pointed(D_001553e0);
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
