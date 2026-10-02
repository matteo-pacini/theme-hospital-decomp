#include "hosp.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

struct Req2Layout {
    u8 unknown0[0x170];
    // 0x170: Staff requester (req12): window showing the staff member.
    s32 win_x;
    s32 win_y;
    s32 win_w;
    s32 win_h;
    // 0x180: Staff requester: text window of the message.
    s32 msg_x;
    s32 msg_y;
    s32 msg_w;
    s32 msg_h;
    u8 unknown190[0x220];
    // 0x3b0: Hire requester: cover0 and cover1 hide the doctor-only parts for a
    // candidate who is not a doctor.
    s32 cover0_x;
    s32 cover0_y;
    s32 cover0_w;
    s32 cover0_h;
    s32 cover1_x;
    s32 cover1_y;
    s32 cover1_w;
    s32 cover1_h;
    // 0x3d0: Hire requester: box blanked while crequester.mode is 1.
    s32 box_x;
    s32 box_y;
    s32 box_w;
    s32 box_h;
    u8 unknown3e0[0x30];
    // 0x410: Hire requester: text window of the candidate's reference.
    s32 text_x;
    s32 text_y;
    s32 text_w;
    s32 text_h;
    u8 unknown420[0x140];
    // 0x560: Hire requester: the candidate's ability gauge (both requesters
    // use gauge_w).
    u16 gauge_x;
    u8 unknown562[2];
    u16 gauge_y;
    u8 unknown566[2];
    u16 gauge_w;
    u8 unknown56a[6];
    // 0x570: Staff requester: cover2 and cover3 hide the doctor-only parts for
    // staff who are not doctors or surgeons.
    s32 cover2_x;
    s32 cover2_y;
    s32 cover2_w;
    s32 cover2_h;
    s32 cover3_x;
    s32 cover3_y;
    s32 cover3_w;
    s32 cover3_h;
};

extern "C" {
extern Event player_events[];
extern Picture loaded_face;
extern u8 *face_buffer;
void open_face_file(void);
}

u32 safe_normal_rand(u32 low, u32 high);
void draw_face(Picture *face, const u16 x, const u16 y);
void draw_req2_coverup(const u16 x, const u16 y);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);

extern "C" {
}

// Hire staff requester clicks: the four kind buttons choose doctors, nurses,
// handymen or receptionists (event EV_HIRE_SELECT_KIND), the arrows page through that
// kind's applicants (event EV_HIRE_NEXT_APPLICANT), hire takes on the shown applicant if
// affordable (EV_MARK_EMPLOYING), and close aborts.
// FUNCTION: 0x000697e0
void req2(Window *win)
{
    u16 i;
    u16 id;
    u16 old;
    WindowButton *button;

    for (button = win->buttons, i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = i;
            if (i == resvars.requester_buttons[2]) {
                player_events[player].type = EV_HIRE_SELECT_KIND;
                player_events[player].value = STAFF_DOCTOR;
                if (crequester.arg != STAFF_DOCTOR || crequester.mode != 2)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (i == resvars.requester_buttons[3]) {
                player_events[player].type = EV_HIRE_SELECT_KIND;
                player_events[player].value = STAFF_NURSE;
                if (crequester.arg != STAFF_NURSE || crequester.mode != 2)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (i == resvars.requester_buttons[5]) {
                player_events[player].type = EV_HIRE_SELECT_KIND;
                player_events[player].value = STAFF_RECEPTIONIST;
                if (crequester.arg != STAFF_RECEPTIONIST || crequester.mode != 2)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (i == resvars.requester_buttons[4]) {
                player_events[player].type = EV_HIRE_SELECT_KIND;
                player_events[player].value = STAFF_HANDYMAN;
                if (crequester.arg != STAFF_HANDYMAN || crequester.mode != 2)
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (i == resvars.requester_buttons[8]) {
                if (crequester.arg2) {
                    if (crequester.mode == 2) {
                        if (allhosps[player].balance >= thing[crequester.arg2].salary && allhosps[player].balance > 0) {
                            player_events[player].type = EV_MARK_EMPLOYING;
                            player_events[player].value = crequester.arg;
                            player_events[player].arg = crequester.arg2;
                            play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                        } else {
                            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                        }
                    } else {
                        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                    }
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            } else if (i == resvars.requester_buttons[6]) {
                if (crequester.arg2) {
                    old = crequester.arg2;
                    id = req2_alter_staff(0, crequester.arg2, crequester.arg);
                    if (id) {
                        if (id == old) {
                            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                        } else {
                            player_events[player].type = EV_HIRE_NEXT_APPLICANT;
                            player_events[player].value = 0;
                            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
                        }
                    } else {
                        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                    }
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            } else if (i == resvars.requester_buttons[7]) {
                if (crequester.arg2) {
                    old = crequester.arg2;
                    id = req2_alter_staff(1, crequester.arg2, crequester.arg);
                    if (id) {
                        if (id == old) {
                            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                        } else {
                            player_events[player].type = EV_HIRE_NEXT_APPLICANT;
                            player_events[player].value = 1;
                            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
                        }
                    } else {
                        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                    }
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            } else if (i == resvars.requester_buttons[9]) {
                window.current_window = 0;
                setup_abort_all_requesters();
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
        }
        i++;
    }
}

// Draws the hire staff requester: each kind's applicant count and the shown
// applicant (crequester.arg2): name, wage, face, qualifications, ability
// gauge and reference, plus the advisor's flashing hints. Moves on to the
// next applicant if the shown one was taken.
// FUNCTION: 0x00069d34
void req2_overdraw(Window *win)
{
    s32 x0;
    WindowButton *button;
    Thing *candidate;
    s32 y0;
    s32 spr;
    u8 i;
    u16 counts[4];
    char buf[200];

    spr = 0;
    if (resvars.game_mode == SCREEN_MODE_13 && loaded_requester != 0xb)
        return;
    compute_help_text(win->x, win->y, 4);
    if (crequester.arg2 > end_things - thing)
        crequester.arg2 = 0;
    if (crequester.arg2) {
        candidate = &thing[crequester.arg2];
        if (candidate->state != SS_APPLICANT)
            crequester.arg2 = req2_alter_staff(1, 0, crequester.arg);
    }
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    x0 = win->x - lbDisplay.graphics_window_x;
    y0 = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (crequester.arg2 && crequester.mode == 2) {
        for (button = win->buttons, i = 1; button; button = button->next, i++) {
            if (i == resvars.requester_buttons[2] && crequester.arg == STAFF_DOCTOR) {
                spr = resvars.requester_sprites[0x35];
                break;
            }
            if (i == resvars.requester_buttons[3] && crequester.arg == STAFF_NURSE) {
                spr = 0x10b;
                break;
            }
            if (i == resvars.requester_buttons[5] && crequester.arg == STAFF_RECEPTIONIST) {
                spr = resvars.requester_sprites[0x31];
                break;
            }
            if (i == resvars.requester_buttons[4] && crequester.arg == STAFF_HANDYMAN) {
                spr = 0x10d;
                break;
            }
        }
    }
    if (spr)
        reqspr_draw(button->x + x0, button->y + y0, &button_sprite[spr]);
    for (i = 0; i < 4; i++)
        counts[i] = 0;
    for (candidate = &thing[1]; candidate < end_things; candidate++) {
        if (candidate->type == THING_STAFF) {
            if (candidate->state == SS_APPLICANT) {
                if (candidate->subtype == STAFF_DOCTOR)
                    counts[0]++;
                else if (candidate->subtype == STAFF_NURSE)
                    counts[1]++;
                else if (candidate->subtype == STAFF_HANDYMAN)
                    counts[2]++;
                else if (candidate->subtype == STAFF_RECEPTIONIST)
                    counts[3]++;
            }
        }
    }
    set_font_from_cache(0xd);
    for (i = 0; i < 4; i++) {
        sprintf(buf, "%d", counts[i]);
        LbTextDraw(x0 + resvars.coords[i + 78].x, y0 + resvars.coords[i + 78].y, buf);
    }
    if (advisor.flags & 1) {
        i = 1;
        othervar.flash_counter++;
        if (othervar.flash_counter % 10 == 0) {
            switch (advisor.tutorial_highlight) {
            case 3:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[5])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.requester_sprites[0x31]]);
                break;
            case 5:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[8])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.requester_sprites[0x32]]);
                break;
            case 6:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[6])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.requester_sprites[0x33]]);
                break;
            case 7:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[7])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.requester_sprites[0x34]]);
                break;
            case 4:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[2])
                        break;
                    i++;
                }
                reqspr_draw((s16)x0 + button->x, (s16)y0 + button->y, &button_sprite[resvars.requester_sprites[0x35]]);
                break;
            }
            othervar.flash_counter = 0;
        }
    }
    set_font_from_cache(1);
    switch (crequester.mode) {
    case 1:
        LbDrawBox(x0 + resvars.boxes_Req2Layoutp->box_x, y0 + resvars.boxes_Req2Layoutp->box_y,
                  resvars.boxes_Req2Layoutp->box_w, resvars.boxes_Req2Layoutp->box_h, 0x77);
        break;
    case 2:
        if (crequester.arg2) {
            candidate = &thing[crequester.arg2];
            sprintf(buf, "%c. %s%s", candidate->initial, staff_names_1[candidate->surname_a],
                    staff_names_2[candidate->surname_b]);
            LbTextDraw(x0 + resvars.coords[82].x, y0 + resvars.coords[82].y, buf);
            sprintf(buf, "$%d", candidate->salary);
            LbTextDraw(x0 + resvars.coords[84].x, y0 + resvars.coords[84].y, buf);
            draw_face((Picture *)&candidate->face_head, x0 + resvars.coords[83].x, y0 + resvars.coords[83].y);
            i = 0;
            if ((candidate->skills & SKILL_JUNIOR) && !(candidate->skills & SKILL_CONSULTANT))
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[6]]);
            i++;
            if ((candidate->skills & SKILL_DOCTOR) && !(candidate->skills & SKILL_CONSULTANT) &&
                !(candidate->skills & SKILL_JUNIOR))
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[6]]);
            i++;
            if (candidate->skills & SKILL_CONSULTANT)
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[6]]);
            i++;
            if (candidate->skills & SKILL_SURGEON)
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[7]]);
            i++;
            if (candidate->skills & SKILL_PSYCHIATRIST)
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[8]]);
            i++;
            if (candidate->skills & SKILL_RESEARCHER)
                reqspr_draw(x0 + resvars.coords[i + 85].x, y0 + resvars.coords[i + 85].y,
                            &button_sprite[resvars.requester_sprites[9]]);
            if (candidate->subtype != STAFF_DOCTOR)
                draw_req2_coverup(x0, y0);
            draw_gauge(candidate->ability, x0 + resvars.boxes_Req2Layoutp->gauge_x + i,
                       y0 + resvars.boxes_Req2Layoutp->gauge_y, resvars.boxes_Req2Layoutp->gauge_w,
                       &button_sprite[resvars.requester_sprites[2]], 1000, 30, 970, 1);
            lbDisplay.draw_flags = 0x20;
            LbTextSetWindow(x0 + resvars.boxes_Req2Layoutp->text_x, y0 + resvars.boxes_Req2Layoutp->text_y,
                            resvars.boxes_Req2Layoutp->text_w, resvars.boxes_Req2Layoutp->text_h);
            if (resvars.game_mode == SCREEN_MODE_13) {
                if (candidate->ability > 500)
                    sprintf(buf, "%s%s", ref_general_text[candidate->ref_general], ref_good_text[candidate->ref_good]);
                else
                    sprintf(buf, "%s%s", ref_general_text[candidate->ref_general], ref_bad_text[candidate->ref_bad]);
            } else if (resvars.game_mode == SCREEN_MODE_1) {
                if (candidate->ability > 500)
                    sprintf(buf, "%s", ref_good_text[candidate->ref_good]);
                else
                    sprintf(buf, "%s", ref_bad_text[candidate->ref_bad]);
            }
            LbTextDraw(0, 0, buf);
            lbDisplay.draw_flags = 0;
        } else {
            LbDrawBox(x0 + resvars.boxes_Req2Layoutp->box_x, y0 + resvars.boxes_Req2Layoutp->box_y,
                      resvars.boxes_Req2Layoutp->box_w, resvars.boxes_Req2Layoutp->box_h, 0x77);
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(0);
        break;
    default:
        LbDrawBox(x0 + resvars.boxes_Req2Layoutp->box_x, y0 + resvars.boxes_Req2Layoutp->box_y,
                  resvars.boxes_Req2Layoutp->box_w, resvars.boxes_Req2Layoutp->box_h, 0x77);
        break;
    }
}

// Resets the hire staff requester (REQ_HIRE_STAFF) to no kind
// chosen.
// FUNCTION: 0x0006aab0
void req2_reset(void)
{
    crequester.id = REQ_HIRE_STAFF;
    crequester.mode = 1;
    crequester.arg = 0;
}

// Returns the next (dir 1) or previous (dir 0) applicant of staff kind
// `kind` after applicant cur, or the first one if cur is 0. Returns cur if
// there is none further, 0 if cur is not found or there are none.
// FUNCTION: 0x0006aae0
u16 req2_alter_staff(u8 dir, u16 cur, u16 kind)
{
    u8 found;
    Thing *candidate;

    found = 0;
    switch (dir) {
    case 1:
        if (cur == 0) {
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if (candidate->type == THING_STAFF) {
                    if (candidate->subtype == kind) {
                        if (candidate->state == SS_APPLICANT)
                            return candidate - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if ((u16)(candidate - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (candidate++; candidate < end_things; candidate++) {
                    if (candidate->type == THING_STAFF) {
                        if (candidate->subtype == kind) {
                            if (candidate->state == SS_APPLICANT)
                                return candidate - thing;
                        }
                    }
                }
                return cur;
            }
        }
        break;
    case 0:
        if (cur == 0) {
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if (candidate->type == THING_STAFF) {
                    if (candidate->subtype == kind) {
                        if (candidate->state == SS_APPLICANT)
                            return candidate - thing;
                    }
                }
            }
        } else {
            found = 0;
            for (candidate = &thing[1]; candidate < end_things; candidate++) {
                if ((u16)(candidate - thing) == cur) {
                    found = 1;
                    break;
                }
            }
            if (found == 1) {
                found = 0;
                for (candidate--; &thing[1] <= candidate; candidate--) {
                    if (candidate->type == THING_STAFF) {
                        if (candidate->subtype == kind) {
                            if (candidate->state == SS_APPLICANT)
                                return candidate - thing;
                        }
                    }
                }
                return cur;
            }
        }
        break;
    }
    return 0;
}

// Draws a staff member's face (head, body and legs from the faces file,
// cached in face_buffer) in a frame whose bottom left is (x, y).
// FUNCTION: 0x0006ae00
void draw_face(Picture *face, const u16 x, const u16 y)
{
    u8 *src;
    u8 *dst;
    s32 width;
    s32 rows;

    open_face_file();
    if (face->head != loaded_face.head) {
        LbFileSeek(face_file, face->head * (resvars.face_w * resvars.face_head_h), 0);
        LbFileRead(face_file, face_buffer, resvars.face_w * resvars.face_head_h);
        loaded_face.head = face->head;
    }
    if (face->body != loaded_face.body) {
        LbFileSeek(face_file,
                   resvars.face_w * resvars.face_head_h * 18 + face->body * (resvars.face_w * resvars.face_body_h), 0);
        LbFileRead(face_file, face_buffer + resvars.face_w * resvars.face_head_h, resvars.face_w * resvars.face_body_h);
        loaded_face.body = face->body;
    }
    if (face->legs != loaded_face.legs) {
        LbFileSeek(face_file,
                   resvars.face_w * resvars.face_head_h * 18 + resvars.face_w * resvars.face_body_h * 18 +
                       (u8)face->legs * (resvars.face_w * resvars.face_legs_h),
                   0);
        LbFileRead(face_file, resvars.face_w * resvars.face_head_h + resvars.face_w * resvars.face_body_h + face_buffer,
                   resvars.face_w * resvars.face_legs_h);
        loaded_face.legs = face->legs;
    }
    src = face_buffer;
    dst = x + resvars.face_border_w + lbDisplay.wscreen +
          lbDisplay.graphics_screen_width *
              (y - (resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h) - resvars.face_border_h);
    width = resvars.face_w;
    rows = resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h;
    while (--rows >= 0) {
        lib_memcpy(dst, src, width);
        src += width;
        dst += lbDisplay.graphics_screen_width;
    }
    lbDisplay.draw_flags = 0;
    LbDrawBox(x, y - (resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h) - resvars.face_border_h * 2,
              resvars.face_border_w * 2 + resvars.face_w, resvars.face_border_h, 0x9c);
    LbDrawBox(x, y - (resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h) - resvars.face_border_h * 2,
              resvars.face_border_w,
              resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h + resvars.face_border_h * 2, 0x9c);
    LbDrawBox(x, y - resvars.face_border_h, resvars.face_border_w * 2 + resvars.face_w, resvars.face_border_h, 0x9c);
    LbDrawBox(x + resvars.face_w + resvars.face_border_w,
              y - (resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h) - resvars.face_border_h * 2,
              resvars.face_border_w,
              resvars.face_head_h + resvars.face_body_h + resvars.face_legs_h + resvars.face_border_h * 2, 0x9c);
}

// Marks applicant id as being hired (SS_HELD) and adds their happiness to
// adcounts.staff_happiness_sum.
// FUNCTION: 0x0006b174
void req2_mark_employing(const u16 id)
{
    Thing *candidate;

    candidate = &thing[id];
    candidate->state = SS_HELD;
    adcounts.staff_happiness_sum += candidate->happiness;
}

// Returns applicant id to the list of applicants (SS_APPLICANT).
// FUNCTION: 0x0006b1bc
void req2_exit_employing(const u16 id)
{
    Thing *candidate;

    candidate = &thing[id];
    candidate->state = SS_APPLICANT;
}

// Staff demand requester clicks: the accept button grants what the staff
// member asks (by request kind crequester.kind: EV_REFUSE_STAFF_DEMAND,
// EV_GIVE_STAFF_BONUS, EV_RAISE_STAFF_SALARY, or for a rival's offer a raise
// to twice the salary), the other lets them go
// (req12_clicked_sack).
// FUNCTION: 0x0006b1f4
void req12(Window *win)
{
    u16 kind;
    u16 i;
    u16 sub;
    WindowButton *button;

    button = win->buttons;
    kind = crequester.kind;
    sub = crequester.item;
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = i;
            if (i == resvars.requester_buttons[0x15]) {
                play_sound(SNDSRC_GENERAL, SFX_BONUS2, -1, -1);
                switch (crequester.kind) {
                case 2:
                    player_events[player].type = EV_REFUSE_STAFF_DEMAND;
                    player_events[player].value = crequester.arg;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 3:
                case 5:
                case 8:
                    player_events[player].type = EV_GIVE_STAFF_BONUS;
                    player_events[player].value = crequester.arg;
                    player_events[player].arg = crequester.arg2;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 4:
                case 6:
                case 7:
                    player_events[player].type = EV_RAISE_STAFF_SALARY;
                    player_events[player].value = crequester.arg;
                    player_events[player].arg = crequester.arg2;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                case 9:
                    player_events[player].type = EV_RAISE_STAFF_SALARY;
                    player_events[player].value = crequester.arg;
                    player_events[player].arg = thing[crequester.arg].salary * 2;
                    lbDisplay.left_button = lbDisplay.rleft_button = 0;
                    break;
                }
            } else if (i == resvars.requester_buttons[0x16]) {
                play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
                req12_clicked_sack();
                lbDisplay.left_button = lbDisplay.rleft_button = 0;
            }
            break;
        }
        i++;
    }
}

extern "C" {
extern char **misc_text;
}

void draw_window_not_using_bscreen(const u8 win);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);

// Draws the staff demand requester: the staff member's details, the request
// text for crequester.kind with its amounts, and a view of them on the
// map.
// FUNCTION: 0x0006b464
void req12_overdraw(Window *win)
{
    u8 i;
    u16 old_h;
    u16 old_w;
    u16 sub;
    u16 old_x;
    u16 old_y;
    Thing *staff;
    s32 y0;
    s32 x0;
    char buf[200];

    sub = crequester.item;
    othervar.full_redraw = 1;
    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    x0 = win->x - lbDisplay.graphics_window_x;
    y0 = win->y - lbDisplay.graphics_window_y;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (!crequester.arg3) {
        close_irequesters();
        open_new_window(0, 0, 0, x0 + resvars.boxes_Req2Layoutp->win_x, y0 + resvars.boxes_Req2Layoutp->win_y,
                        x0 + resvars.boxes_Req2Layoutp->win_x + resvars.boxes_Req2Layoutp->win_w,
                        y0 + resvars.boxes_Req2Layoutp->win_y + resvars.boxes_Req2Layoutp->win_h);
        window.current_window = 0;
        crequester.arg3 = 1;
    }
    if (crequester.arg) {
        staff = &thing[crequester.arg];
        sprintf(buf, "%c. %s%s", staff->initial, staff_names_1[staff->surname_a], staff_names_2[staff->surname_b]);
        LbTextDraw(x0 + resvars.coords[2].x, y0 + resvars.coords[2].y, buf);
        sprintf(buf, "$%d", staff->salary);
        LbTextDraw(x0 + resvars.coords[4].x, y0 + resvars.coords[4].y, buf);
        draw_face((Picture *)&staff->face_head, x0 + resvars.coords[3].x, y0 + resvars.coords[3].y);
        i = 0;
        if (staff->skills & SKILL_JUNIOR)
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[6]]);
        i++;
        if ((staff->skills & SKILL_DOCTOR) && !(staff->skills & SKILL_CONSULTANT) && !(staff->skills & SKILL_JUNIOR))
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[6]]);
        i++;
        if (staff->skills & SKILL_CONSULTANT)
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[6]]);
        i++;
        if (staff->skills & SKILL_SURGEON)
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[7]]);
        i++;
        if (staff->skills & SKILL_PSYCHIATRIST)
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[8]]);
        i++;
        if (staff->skills & SKILL_RESEARCHER)
            reqspr_draw(x0 + resvars.coords[i + 91].x, y0 + resvars.coords[i + 91].y,
                        &button_sprite[resvars.requester_sprites[9]]);
        draw_gauge(staff->ability, x0 + resvars.coords[0].x, y0 + resvars.coords[0].y,
                   resvars.boxes_Req2Layoutp->gauge_w, &button_sprite[resvars.requester_sprites[3]], 1000, 30, 970, 1);
        if (staff->subtype != STAFF_DOCTOR && staff->subtype != STAFF_SURGEON) {
            LbDrawBox(x0 + resvars.boxes_Req2Layoutp->cover2_x, y0 + resvars.boxes_Req2Layoutp->cover2_y,
                      resvars.boxes_Req2Layoutp->cover2_w, resvars.boxes_Req2Layoutp->cover2_h, 0x77);
            LbDrawBox(x0 + resvars.boxes_Req2Layoutp->cover3_x, y0 + resvars.boxes_Req2Layoutp->cover3_y,
                      resvars.boxes_Req2Layoutp->cover3_w, resvars.boxes_Req2Layoutp->cover3_h, 0x77);
        }
        switch (crequester.kind) {
        case 2:
            sprintf(buf, misc_text[crequester.kind]);
            break;
        case 3:
        case 5:
        case 7:
        case 8:
            sprintf(buf, misc_text[crequester.kind], crequester.arg2);
            break;
        case 4:
        case 6:
            sprintf(buf, misc_text[crequester.kind], crequester.arg2, crequester.arg2 + staff->salary);
            break;
        case 9:
            sprintf(buf, misc_text[crequester.kind], staff->salary * 2, &allhosps[crequester.arg2]);
            break;
        }
        lbDisplay.draw_flags = 0x20;
        LbTextSetWindow(x0 + resvars.boxes_Req2Layoutp->msg_x, y0 + resvars.boxes_Req2Layoutp->msg_y,
                        resvars.boxes_Req2Layoutp->msg_w, resvars.boxes_Req2Layoutp->msg_h);
        LbTextDraw(0, 0, buf);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        give_absolute_xy_of_cell_at_column_row(staff->x.tile, staff->y.tile);
        window.scroll_x[window.window_count - 1] =
            (resvars.screen_mode == SCREEN_MODE_13 ? ((staff->x.frac - staff->y.frac) >> 3) + 0x20
                                                   : ((staff->x.frac - staff->y.frac) >> 4) + 0x10) +
            (info.cell_x - resvars.coords[171].x);
        window.scroll_y[window.window_count - 1] =
            info.cell_y - resvars.coords[171].y +
            (resvars.screen_mode == SCREEN_MODE_13 ? (staff->y.frac + staff->x.frac) >> 4
                                                   : (staff->y.frac + staff->x.frac) >> 5);
        old_x = window.clip_x0;
        old_y = window.clip_y0;
        old_w = window.clip_x1;
        old_h = window.clip_y1;
        window.clip_x0 = resvars.boxes_Req2Layoutp->win_x;
        window.clip_y0 = resvars.boxes_Req2Layoutp->win_y;
        window.clip_x1 = resvars.boxes_Req2Layoutp->win_x + resvars.boxes_Req2Layoutp->win_w;
        window.clip_y1 = resvars.boxes_Req2Layoutp->win_y + resvars.boxes_Req2Layoutp->win_h;
        draw_window_not_using_bscreen(window.window_count - 1);
        window.clip_x0 = old_x;
        window.clip_y0 = old_y;
        window.clip_x1 = old_w;
        window.clip_y1 = old_h;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
    compute_help_text(0, 0, 0x1a);
}

// Opens the staff demand requester for a staff member who wants to leave,
// choosing the request (crequester.kind, amount in arg2): a rival's
// offer (9), quitting after many requests (2), their wage demand plus 10%
// (7), or 10-30% of their salary (3/4 when tired, 5/6 by happiness, else 8).
// FUNCTION: 0x0006bda8
void open_staff_resign_requester(Thing *staff)
{
    u32 i;
    s32 pay_gap;
    u32 nplayers;

    nplayers = 1;
    if (statvar.num_players > 1)
        nplayers = 4;
    play_sound(SNDSRC_GENERAL, SFX_STAFF_DEMAND, -1, -1);
    display_mode = DM_MODAL;
    close_irequesters();
    crequester.id = REQ_STAFF_DEMAND;
    open_requester(crequester.id, 1);
    crequester.arg = staff - thing;
    crequester.arg3 = 0;
    if (staff->poach_offers) {
        crequester.arg2 = 0;
        for (i = 0; i < nplayers; i++) {
            if (statvar.num_players > 1 && player_info[i].active == 0)
                continue;
            if (staff->poach_offers == 1 << i) {
                crequester.arg2 = i;
                break;
            }
        }
        crequester.kind = 9;
    } else if (staff->resign_count > 5) {
        crequester.arg2 = 0;
        crequester.kind = 2;
    } else if (staff->wage_demand > staff->salary) {
        pay_gap = staff->wage_demand - staff->salary;
        crequester.arg2 = staff->wage_demand * 10 / 100 + pay_gap;
        crequester.kind = 7;
    } else if (staff->tiredness > gbv.very_tired && saferand() % 2) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.kind = 3;
        else
            crequester.kind = 4;
        crequester.arg2 = staff->salary * i / 100;
    } else if (staff->happiness_s > gbv.very_tired) {
        i = safe_normal_rand(10, 30);
        if (saferand() % 2 == 0)
            crequester.kind = 5;
        else
            crequester.kind = 6;
        crequester.arg2 = staff->salary * i / 100;
    } else {
        i = safe_normal_rand(10, 30);
        crequester.kind = 8;
        crequester.arg2 = staff->salary * i / 100;
    }
    crequester.mode = 0;
}

// Covers the doctor-only parts of the hire staff requester at window (x,
// y).
// FUNCTION: 0x0006c05c
void draw_req2_coverup(const u16 x, const u16 y)
{
    LbDrawBox(x + resvars.boxes_Req2Layoutp->cover0_x, y + resvars.boxes_Req2Layoutp->cover0_y,
              resvars.boxes_Req2Layoutp->cover0_w, resvars.boxes_Req2Layoutp->cover0_h, 0x77);
    LbDrawBox(x + resvars.boxes_Req2Layoutp->cover1_x, y + resvars.boxes_Req2Layoutp->cover1_y,
              resvars.boxes_Req2Layoutp->cover1_w, resvars.boxes_Req2Layoutp->cover1_h, 0x77);
}

// Help text for a doctor qualification of the shown applicant, into
// othervar.help_msg, if they are a doctor.
// FUNCTION: 0x0006c108
void req2_help_if_doctor(void)
{
    Thing *candidate;

    candidate = &thing[crequester.arg2];
    if (candidate > thing && (candidate->skills & SKILL_DOCTOR))
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_doctor, if they are a surgeon.
// FUNCTION: 0x0006c184
void req2_help_if_surgeon(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.arg2];
    mask = 0x30;
    if (candidate > thing && (candidate->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_doctor, if they are a psychiatrist.
// FUNCTION: 0x0006c214
void req2_help_if_psycho(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.arg2];
    mask = 0x50;
    if (candidate > thing && (candidate->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_doctor, if they are a researcher.
// FUNCTION: 0x0006c2a4
void req2_help_if_researcher(void)
{
    Thing *candidate;
    u32 mask;

    candidate = &thing[crequester.arg2];
    mask = 0x110;
    if (candidate > thing && (candidate->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// Help text for the hovered entry, into othervar.help_msg, if an applicant is
// shown.
// FUNCTION: 0x0006c334
void req2_help_if_selected(void)
{
    if (crequester.arg2)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_doctor for the staff member of the demand requester.
// FUNCTION: 0x0006c38c
void req12_help_if_doctor(void)
{
    Thing *staff;

    staff = &thing[crequester.arg];
    if (staff > thing && (staff->skills & SKILL_DOCTOR))
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_surgeon for the staff member of the demand requester.
// FUNCTION: 0x0006c408
void req12_help_if_surgeon(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.arg];
    mask = 0x30;
    if (staff > thing && (staff->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_psycho for the staff member of the demand requester.
// FUNCTION: 0x0006c498
void req12_help_if_shrink(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.arg];
    mask = 0x50;
    if (staff > thing && (staff->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req2_help_if_researcher for the staff member of the demand requester.
// FUNCTION: 0x0006c528
void req12_help_if_research(void)
{
    Thing *staff;
    u32 mask;

    staff = &thing[crequester.arg];
    mask = 0x110;
    if (staff > thing && (staff->skills & mask) == mask)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// Refuses the staff member's demand: they join the rival who made the offer
// (EV_STAFF_JOIN_NEW_EMPLOYER) or leave (event EV_REFUSE_STAFF_DEMAND).
// FUNCTION: 0x0006c5b8
void req12_clicked_sack(void)
{
    if (crequester.kind == 9) {
        player_events[player].type = EV_STAFF_JOIN_NEW_EMPLOYER;
        player_events[player].value = crequester.arg;
        player_events[player].arg = crequester.arg2;
    } else {
        player_events[player].type = EV_REFUSE_STAFF_DEMAND;
        player_events[player].value = crequester.arg;
    }
}
