#include "hosp.h"

struct SlistArea {
    u16 x0;
    u16 y0;
    u16 unknown4;
    u16 x1;
    u16 y1;
    u8 unknowna[8];
    u16 face_x;
    u16 face_y;
};

struct SlistMenuItem {
    u8 unknown0[8];
    u16 text;
    u16 row;
    u8 unknownc[4];
};

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

extern "C" {
extern Picture loaded_face;
extern Event player_events[];
extern char **slist_text;
extern Slot irequester_slots[];
}

u16 get_slist_staff(const u8 dir);
void draw_slist_staff(const u16 selected);
void draw_slist_window(const u16 id);
void draw_slist_panel(void);
void draw_slist_overall(void);
void process_slist_panel(void);
u8 staff_on_page(const u8 page);
void draw_slist_nums(void);
void draw_slist_coverup(void);
void slist_keys(void);
void draw_slist_button_overlays(Thing *staff);
void draw_slist_training_bar(u16 x, u16 y, u16 value, u16 max);
void cancel_helpmsg(void);
void draw_gauge(s32 value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const s32 max, const s32 min,
                const s32 range, const u8 up);
void give_absolute_xy_of_cell_at_column_row(const s32 x, const s32 y);
void draw_window_not_using_bscreen(const u8 win);
void draw_face(Picture *face, const u16 x, const u16 y);

// Runs one frame of the staff management screen (slist.state: 0 opening,
// 1 shown, 2 leaving to the selected staff member's window (event EV_RETURN_TO_THING_WINDOW), 3
// closing). Opened from a staff window (othervar.slist_staff), it starts on that
// staff member's kind and page.
// FUNCTION: 0x000914e0
void display_slist(void)
{
    u16 prev;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(1);
    lbDisplay.draw_flags = 0;
    switch (slist.state) {
    case 0:
        load_ingame_pic(SCR_STAFF_LIST, 1, 0x21, 0, SCREEN_MODE_13);
        loaded_face.head = 0xff;
        loaded_face.body = 0xff;
        loaded_face.legs = 0xff;
        set_mouse_mode(MM_STAFF_LIST);
        slist.state = 1;
        slist.value = 0;
        othervar.slist_filter = 1;
        close_irequesters();
        open_new_window(0, 0, 0, resvars.screen_list_pos_SlistAreap->x0, resvars.screen_list_pos_SlistAreap->y0,
                        resvars.screen_list_pos_SlistAreap->x1, resvars.screen_list_pos_SlistAreap->y1);
        window.current_window = window.window_count - 1;
        othervar.slist_page = 0;
        if (othervar.slist_staff) {
            switch ((thing + othervar.slist_staff)->subtype) {
            case STAFF_NURSE:
                othervar.slist_filter = 2;
                break;
            case STAFF_DOCTOR:
                othervar.slist_filter = 1;
                break;
            case STAFF_HANDYMAN:
                othervar.slist_filter = 4;
                break;
            case STAFF_RECEPTIONIST:
                othervar.slist_filter = 8;
                break;
            case STAFF_SURGEON:
                othervar.slist_filter = 1;
                break;
            default:
                break;
            }
            for (;;) {
                prev = slist.value;
                slist.value = get_slist_staff(0);
                if (prev == slist.value) {
                    slist.value = 0;
                    break;
                }
                if (slist.value == othervar.slist_staff)
                    break;
                if (othervar.slist_cursor / 11 > 0 && othervar.slist_cursor % 11 == 0) {
                    if (othervar.slist_page < 150) {
                        if (staff_on_page(othervar.slist_page + 1))
                            othervar.slist_page++;
                    }
                }
            }
        }
        copy_screen_to_wscreen(bscreen);
        draw_slist_staff(slist.value);
        draw_slist_window(slist.value);
        draw_slist_panel();
        draw_slist_overall();
        draw_slist_nums();
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        draw_slist_staff(slist.value);
        draw_slist_window(slist.value);
        draw_slist_panel();
        draw_slist_overall();
        draw_slist_nums();
        process_slist_panel();
        break;
    case 2:
        player_events[player].type = EV_RETURN_TO_THING_WINDOW;
        player_events[player].value = 0;
        player_events[player].arg = slist.value;
        cancel_helpmsg();
        break;
    case 3:
        player_events[player].type = EV_RETURN_FROM_SCREEN;
        player_events[player].value = 0;
        cancel_helpmsg();
        break;
    default:
        break;
    }
    slist_keys();
    lbDisplay.draw_flags = 0;
    if (!staff_on_page(othervar.slist_page)) {
        if (othervar.slist_page > 0)
            othervar.slist_page--;
    }
    lbDisplay.rleft_button = lbDisplay.left_button = 0;
    if (exit_quick_requester())
        slist.state = 3;
    compute_help_text(0, 0, 6);
}

// Staff management keys: left/right change the kind shown (othervar.slist_filter),
// up/down select the previous/next staff member, turning the page as
// needed.
// FUNCTION: 0x0009187c
void slist_keys(void)
{
    u16 prev;
    int i;

    prev = 0;
    if (lbInkey == KC_NUMPAD6 || lbKeyOn[KC_RIGHT]) {
        if (othervar.slist_filter != 8) {
            othervar.slist_filter <<= 1;
            slist.value = 0;
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            othervar.slist_cursor = 0;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        }
        lbInkey = 0;
    } else if (lbInkey == KC_NUMPAD4 || lbKeyOn[KC_LEFT]) {
        if (othervar.slist_filter != 1) {
            othervar.slist_filter >>= 1;
            slist.value = 0;
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            othervar.slist_cursor = 0;
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        }
        lbInkey = 0;
    }
    if (lbInkey == KC_NUMPAD2 || lbKeyOn[KC_DOWN]) {
        prev = slist.value;
        slist.value = get_slist_staff(0);
        if (prev != slist.value) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            lbInkey = 0;
            if (othervar.slist_cursor / 11 > 0 && othervar.slist_cursor % 11 == 0) {
                if (othervar.slist_page < 150) {
                    if (staff_on_page(othervar.slist_page + 1)) {
                        othervar.slist_page++;
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    } else {
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    }
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                }
            }
        }
    } else if (lbInkey == KC_NUMPAD8 || lbKeyOn[KC_UP]) {
        prev = slist.value;
        slist.value = get_slist_staff(1);
        if (prev != slist.value) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (othervar.slist_cursor / 10 > 0 && othervar.slist_cursor % 10 == 0) {
                if (othervar.slist_page > 0) {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    othervar.slist_page--;
                    for (i = 0; i < 10; i++)
                        slist.value = get_slist_staff(1);
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                }
            }
            lbDisplay.mouse_x = 0;
            lbDisplay.mouse_y = 0;
            lbInkey = 0;
        }
    }
}

// Returns the next (dir 0) or previous (dir 1) staff member of this player
// of the kinds shown after the selected one (slist.value), or the
// selected one if there is none; counts the step in othervar.slist_cursor.
// FUNCTION: 0x00091bac
u16 get_slist_staff(const u8 dir)
{
    Thing *staff;

    staff = &thing[slist.value];
    if (dir == 0) {
        while (++staff < end_things) {
            if (staff->type == THING_STAFF && staff->owner == player) {
                if (staff->state != SS_APPLICANT) {
                    if (((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) &&
                         (othervar.slist_filter & 1)) ||
                        (staff->subtype == STAFF_NURSE && (othervar.slist_filter & 2)) ||
                        (staff->subtype == STAFF_RECEPTIONIST && (othervar.slist_filter & 8)) ||
                        (staff->subtype == STAFF_HANDYMAN && (othervar.slist_filter & 4))) {
                        othervar.slist_cursor++;
                        return staff - thing;
                    }
                }
            }
        }
    } else {
        while (thing + 1 <= --staff) {
            if (staff->type == THING_STAFF && staff->owner == player) {
                if (staff->state != SS_APPLICANT) {
                    if (((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) &&
                         (othervar.slist_filter & 1)) ||
                        (staff->subtype == STAFF_NURSE && (othervar.slist_filter & 2)) ||
                        (staff->subtype == STAFF_RECEPTIONIST && (othervar.slist_filter & 8)) ||
                        (staff->subtype == STAFF_HANDYMAN && (othervar.slist_filter & 4))) {
                        othervar.slist_cursor--;
                        return staff - thing;
                    }
                }
            }
        }
    }
    return slist.value;
}

// Draws the current page (othervar.slist_page, ten rows) of staff of the kinds
// shown: number, name, wage and gauges, with `selected` framed; hovering
// and clicking select a row. Draws the page marker.
// FUNCTION: 0x00091dec
void draw_slist_staff(const u16 selected)
{
    u16 row;
    char buf[100];
    Thing *staff;
    u16 height;
    u16 listed;
    int pages;

    listed = 0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(1);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    for (row = 0; row < 3; row++) {
        lbDisplay.draw_flags = 0x100;
        LbTextSetWindow(resvars.screen_boxes_Boxp[row + 61].x, resvars.screen_boxes_Boxp[row + 61].y,
                        resvars.screen_boxes_Boxp[row + 61].w, resvars.screen_boxes_Boxp[row + 61].h);
        LbTextDraw(0, (resvars.screen_boxes_Boxp[row + 61].h - height) / 2, slist_text[row + 2]);
        lbDisplay.draw_flags = 0;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    }
    row = 0;
    for (staff = thing + 1; staff < end_things; staff++) {
        if (staff->type == THING_STAFF && staff->owner == player) {
            if (staff->state == SS_APPLICANT)
                continue;
            if (row >= 10)
                break;
            if (((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) && (othervar.slist_filter & 1)) ||
                (staff->subtype == STAFF_NURSE && (othervar.slist_filter & 2)) ||
                (staff->subtype == STAFF_RECEPTIONIST && (othervar.slist_filter & 8)) ||
                (staff->subtype == STAFF_HANDYMAN && (othervar.slist_filter & 4))) {
                if (listed >= othervar.slist_page * 10 && listed < (othervar.slist_page + 1) * 10) {
                    set_font_from_cache(1);
                    if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_boxes_Boxp[row + 104].x,
                                     resvars.screen_boxes_Boxp[row + 104].y, resvars.screen_boxes_Boxp[row + 104].w,
                                     resvars.screen_boxes_Boxp[row + 104].h)) {
                        if (othervar.slist_cursor != row + othervar.slist_page * 10 + 1)
                            play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                        set_font_from_cache(2);
                        othervar.slist_cursor = othervar.slist_page * 10 + row + 1;
                    }
                    if (selected == (u16)(staff - thing)) {
                        set_font_from_cache(2);
                        draw_frame(resvars.screen_boxes_Boxp[row + 104].x, resvars.screen_boxes_Boxp[row + 104].y - 1,
                                   resvars.screen_boxes_Boxp[row + 104].w, resvars.screen_boxes_Boxp[row + 104].h,
                                   (u16)color.red);
                        draw_slist_button_overlays(staff);
                    }
                    sprintf(buf, "%d", listed + 1);
                    LbTextDraw(resvars.screen_coords[row + 114].x, resvars.screen_coords[row + 114].y, buf);
                    sprintf(buf, "%c. %s%s", staff->initial, staff_names_1[staff->surname_a],
                            staff_names_2[staff->surname_b]);
                    LbTextDraw(resvars.screen_coords[row + 124].x, resvars.screen_coords[row + 124].y, buf);
                    sprintf(buf, "$%d", staff->salary);
                    LbTextDraw(resvars.screen_coords[row + 134].x, resvars.screen_coords[row + 134].y, buf);
                    draw_gauge(staff->happiness_s, resvars.screen_boxes_Boxp[row + 71].x,
                               resvars.screen_boxes_Boxp[row + 71].y, resvars.screen_boxes_Boxp[row + 71].w,
                               &screen_sprites_tab_start[resvars.screen_boxes_Boxp[row + 71].h], 100, 0, 100, 1);
                    draw_gauge(staff->tiredness, resvars.screen_boxes_Boxp[row + 81].x,
                               resvars.screen_boxes_Boxp[row + 81].y, resvars.screen_boxes_Boxp[row + 81].w,
                               &screen_sprites_tab_start[resvars.screen_boxes_Boxp[row + 81].h], 1000, 0, 1000, 0);
                    draw_gauge(staff->ability, resvars.screen_boxes_Boxp[row + 91].x,
                               resvars.screen_boxes_Boxp[row + 91].y, resvars.screen_boxes_Boxp[row + 91].w,
                               &screen_sprites_tab_start[resvars.screen_boxes_Boxp[row + 91].h], 1000, 30, 970, 1);
                    if (lbDisplay.mleft_button) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_boxes_Boxp[row + 104].x,
                                         resvars.screen_boxes_Boxp[row + 104].y, resvars.screen_boxes_Boxp[row + 104].w,
                                         resvars.screen_boxes_Boxp[row + 104].h)) {
                            if ((u16)(staff - thing) != slist.value)
                                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                            slist.value = staff - thing;
                        }
                    }
                    row++;
                }
                listed++;
            }
        }
    }
    if (row < 10)
        LbDrawBox(resvars.screen_boxes_Boxp[row + 104].x, resvars.screen_boxes_Boxp[row + 104].y,
                  resvars.screen_boxes_Boxp[row + 104].w, (10 - row) * resvars.screen_boxes_Boxp[row + 104].h, 0x77);
    othervar.slist_rows = row;
    for (pages = 0; staff_on_page(pages); pages++)
        ;
    if (pages > 1) {
        u8 pos;

        pos = resvars.screen_boxes_Boxp[114].h / (pages - 1) * othervar.slist_page;
        reqspr_draw(resvars.screen_boxes_Boxp[114].x, pos + resvars.screen_boxes_Boxp[114].y,
                    &screen_sprites_tab_start[resvars.screen_boxes_Boxp[114].w]);
    }
}

// Draws the details of staff member id: a view of them on the map, their
// face, their qualifications with training progress, and a bar from their
// work_delay (0-10).
// FUNCTION: 0x000926ec
void draw_slist_window(const u16 id)
{
    u8 count;
    u8 i;
    u8 j;
    u16 save_y;
    Thing *staff;
    u16 save_w;
    u16 save_h;
    u16 save_x;
    int dx;
    int dy;
    int delay;

    staff = &thing[id];
    if (!id)
        return;
    if (resvars.game_mode == SCREEN_MODE_13) {
        give_absolute_xy_of_cell_at_column_row(staff->x.tile, staff->y.tile);
        if (resvars.screen_mode == SCREEN_MODE_13)
            dx = ((staff->x.frac - staff->y.frac) >> 3) + 32;
        else
            dx = ((staff->x.frac - staff->y.frac) >> 4) + 16;
        window.scroll_x[window.window_count - 1] = dx + (info.cell_x - *(s32 *)&resvars.coords_u8p[0x558]);
        if (resvars.screen_mode == SCREEN_MODE_13)
            dy = (staff->y.frac + staff->x.frac) >> 4;
        else
            dy = (staff->y.frac + staff->x.frac) >> 5;
        window.scroll_y[window.window_count - 1] = info.cell_y - *(s32 *)&resvars.coords_u8p[0x55c] + dy;
        save_x = window.clip_x0;
        save_y = window.clip_y0;
        save_w = window.clip_x1;
        save_h = window.clip_y1;
        window.clip_x0 = resvars.screen_list_pos_SlistAreap->x0;
        window.clip_y0 = resvars.screen_list_pos_SlistAreap->y0;
        window.clip_x1 = resvars.screen_list_pos_SlistAreap->x1;
        window.clip_y1 = resvars.screen_list_pos_SlistAreap->y1;
        draw_window_not_using_bscreen(window.window_count - 1);
        window.clip_x0 = save_x;
        window.clip_y0 = save_y;
        window.clip_x1 = save_w;
        window.clip_y1 = save_h;
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    draw_face((Picture *)&thing[id].face_head, resvars.screen_list_pos_SlistAreap->face_x,
              resvars.screen_list_pos_SlistAreap->face_y);
    if (staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) {
        i = 0;
        if ((staff->skills & SKILL_JUNIOR) && !(staff->skills & SKILL_CONSULTANT))
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        i++;
        if ((staff->skills & SKILL_DOCTOR) && !(staff->skills & SKILL_CONSULTANT) && !(staff->skills & SKILL_JUNIOR))
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        i++;
        if (staff->skills & SKILL_CONSULTANT)
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        i++;
        if (staff->skills & SKILL_SURGEON) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        } else if (staff->surgeon_training) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                                    staff->surgeon_training, gbv.ability_threshold_surgeon);
        }
        i++;
        if (staff->skills & SKILL_PSYCHIATRIST) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        } else if (staff->psych_training) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                                    staff->psych_training, gbv.ability_threshold_psych);
        }
        i++;
        if (staff->skills & SKILL_RESEARCHER) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h]);
        } else if (staff->research_training) {
            reqspr_draw(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                        &screen_sprites_tab_start[resvars.screen_boxes_Boxp[i + 64].h - 3]);
            draw_slist_training_bar(resvars.screen_boxes_Boxp[i + 64].x, resvars.screen_boxes_Boxp[i + 64].y,
                                    staff->research_training, gbv.ability_threshold_research);
        }
    }
    delay = staff->work_delay;
    if (delay > 10)
        delay = 10;
    if (delay < 0)
        delay = 0;
    count = delay * 4200 / 1000;
    for (j = 0; j < count; j++)
        reqspr_draw(resvars.screen_boxes_Boxp[70].x + j, resvars.screen_boxes_Boxp[70].y,
                    &screen_sprites_tab_start[resvars.screen_boxes_Boxp[70].h]);
}

// Does nothing.
// FUNCTION: 0x00092e88
void draw_slist_panel(void)
{
}

// Draws the average happiness, tiredness and ability of the staff of the
// kinds shown, covering the doctor-only parts for other kinds.
// FUNCTION: 0x00092ea0
void draw_slist_overall(void)
{
    Thing *staff;
    s32 happiness;
    s32 tiredness;
    s32 ability;
    u8 count;

    happiness = tiredness = ability = 0;
    count = 0;
    for (staff = thing + 1; staff < end_things; staff++) {
        if (staff->type == THING_STAFF && staff->owner == player) {
            if (staff->state != SS_APPLICANT) {
                if (((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) &&
                     (othervar.slist_filter & 1)) ||
                    (staff->subtype == STAFF_NURSE && (othervar.slist_filter & 2)) ||
                    (staff->subtype == STAFF_RECEPTIONIST && (othervar.slist_filter & 8)) ||
                    (staff->subtype == STAFF_HANDYMAN && (othervar.slist_filter & 4))) {
                    happiness += staff->happiness_s;
                    tiredness += staff->tiredness;
                    ability += staff->ability;
                    count++;
                }
            }
        }
    }
    if (!(othervar.slist_filter & 1))
        draw_slist_coverup();
    if (count) {
        ability /= count;
        happiness /= count;
        tiredness /= count;
        count = 0;
        draw_gauge(happiness, resvars.screen_boxes_Boxp[count + 101].x, resvars.screen_boxes_Boxp[count + 101].y,
                   resvars.screen_boxes_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.screen_boxes_Boxp[count + 101].h], 100, 0, 100, 1);
        count++;
        draw_gauge(tiredness, resvars.screen_boxes_Boxp[count + 101].x, resvars.screen_boxes_Boxp[count + 101].y,
                   resvars.screen_boxes_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.screen_boxes_Boxp[count + 101].h], 1000, 0, 1000, 0);
        count++;
        draw_gauge(ability, resvars.screen_boxes_Boxp[count + 101].x, resvars.screen_boxes_Boxp[count + 101].y,
                   resvars.screen_boxes_Boxp[count + 101].w,
                   &screen_sprites_tab_start[resvars.screen_boxes_Boxp[count + 101].h], 1000, 30, 970, 1);
    }
}

// Staff management clicks: runs the callback of the button released over
// (drawing it pressed while held), and a click on the view opens the
// selected staff member's window (othervar.center_on_slot 2).
// FUNCTION: 0x000931d4
void process_slist_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x30].width != 0 && resvars.screen_buttons[i + 0x30].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0x30].x,
                             resvars.screen_buttons[i + 0x30].y, resvars.screen_buttons[i + 0x30].width,
                             resvars.screen_buttons[i + 0x30].height)) {
                resvars.screen_buttons[i + 0x30].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 0x30].sprite_x, resvars.screen_buttons[i + 0x30].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x30].sprite]);
            }
        }
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_list_pos_SlistAreap->x0,
                         resvars.screen_list_pos_SlistAreap->y0,
                         resvars.screen_list_pos_SlistAreap->x1 - resvars.screen_list_pos_SlistAreap->x0,
                         resvars.screen_list_pos_SlistAreap->y1 - resvars.screen_list_pos_SlistAreap->y0)) {
            if (slist.value) {
                othervar.center_on_slot = 2;
                irequester_slots[2].value = slist.value;
                othervar.full_redraw = 1;
                slist.state = 2;
                close_irequesters();
                window.current_window = 0;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            }
        }
        if (slist.value)
            draw_slist_button_overlays(&thing[slist.value]);
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x30].width != 0 && resvars.screen_buttons[i + 0x30].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x30].x,
                             resvars.screen_buttons[i + 0x30].y, resvars.screen_buttons[i + 0x30].width,
                             resvars.screen_buttons[i + 0x30].height)) {
                reqspr_draw(resvars.screen_buttons[i + 0x30].sprite_x, resvars.screen_buttons[i + 0x30].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0x30].sprite]);
                break;
            }
        }
        if (slist.value)
            draw_slist_button_overlays(&thing[slist.value]);
    }
}

// Staff management button `button`: 0-3 show doctors, nurses,
// receptionists or handymen, 4 sacks the selected staff member (asks to
// confirm), 5 raises their wage by 10%, 6 pays them a 10% bonus, 7 closes,
// 8/9 previous/next page, 10 and 11 send EV_INCREASE_STAFF_SALARIES 500 for
// everyone or the selected one.
// FUNCTION: 0x00093574
void process_slist_button(u8 button)
{
    u16 sound;
    u16 i;

    switch (button) {
    case 0:
        if (othervar.slist_filter != 1)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        othervar.slist_filter = 1;
        slist.value = 0;
        othervar.slist_cursor = 0;
        break;
    case 1:
        if (othervar.slist_filter != 2)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        othervar.slist_filter = 2;
        slist.value = 0;
        othervar.slist_cursor = 0;
        break;
    case 2:
        if (othervar.slist_filter != 8)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        othervar.slist_filter = 8;
        slist.value = 0;
        othervar.slist_cursor = 0;
        break;
    case 3:
        if (othervar.slist_filter != 4)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        othervar.slist_filter = 4;
        slist.value = 0;
        othervar.slist_cursor = 0;
        break;
    case 4:
        if (slist.value) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            close_irequesters();
            lbDisplay.rleft_button = lbDisplay.rright_button = lbDisplay.left_button = lbDisplay.right_button = 0;
            set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
            sprintf(othervar.yesno_msg, "%s", yesno_text[5]);
            open_yes_no_requester(EV_SACK_STAFF, slist.value, 0, EV_CANCEL_SACK, slist.value, 0);
            slist.value = 0;
        }
        break;
    case 5:
        if (slist.value) {
            player_events[player].type = EV_INCREASE_STAFF_SALARIES;
            player_events[player].value = thing[slist.value].salary / 10;
            player_events[player].arg = slist.value;
            play_sound(SNDSRC_GENERAL, SFX_BONUSAL2, -1, -1);
        }
        break;
    case 6:
        if (slist.value) {
            player_events[player].type = EV_PAY_STAFF_BONUS;
            player_events[player].value = thing[slist.value].salary / 10;
            player_events[player].arg = slist.value;
            play_sound(SNDSRC_GENERAL, SFX_BONUS2, -1, -1);
        }
        break;
    case 7:
        slist.state = 3;
        close_irequesters();
        window.current_window = 0;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    case 8:
        sound = 8;
        if (othervar.slist_page > 0) {
            if (staff_on_page(othervar.slist_page - 1)) {
                othervar.slist_page--;
                for (i = 0; i < 10; i++)
                    slist.value = get_slist_staff(1);
                sound = 0x43;
            }
        }
        play_sound(SNDSRC_GENERAL, sound, -1, -1);
        break;
    case 9:
        sound = 8;
        if (othervar.slist_page < 150) {
            if (staff_on_page(othervar.slist_page + 1)) {
                othervar.slist_page++;
                for (i = 0; i < 10; i++)
                    slist.value = get_slist_staff(0);
                sound = 0x43;
            }
        }
        play_sound(SNDSRC_GENERAL, sound, -1, -1);
        break;
    case 10:
        player_events[player].type = EV_INCREASE_STAFF_SALARIES;
        player_events[player].value = 500;
        player_events[player].arg = 0;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 11:
        if (slist.value) {
            player_events[player].type = EV_INCREASE_STAFF_SALARIES;
            player_events[player].value = 500;
            player_events[player].arg = slist.value;
        }
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    default:
        break;
    }
}

// Returns 1 if page `page` (ten per page) of staff of the kinds shown has
// anyone on it.
// FUNCTION: 0x00093a74
u8 staff_on_page(const u8 page)
{
    u16 listed;
    Thing *staff;

    listed = 0;
    for (staff = thing + 1; staff < end_things; staff++) {
        if (staff->type == THING_STAFF) {
            if (staff->state != SS_APPLICANT && staff->owner == player) {
                if (((staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON) &&
                     (othervar.slist_filter & 1)) ||
                    (staff->subtype == STAFF_NURSE && (othervar.slist_filter & 2)) ||
                    (staff->subtype == STAFF_RECEPTIONIST && (othervar.slist_filter & 8)) ||
                    (staff->subtype == STAFF_HANDYMAN && (othervar.slist_filter & 4))) {
                    if (listed >= page * 10 && listed < (page + 1) * 10)
                        return 1;
                    listed++;
                }
            }
        }
    }
    return 0;
}

// Draws the selected kind's tab pressed and the number of staff of each
// kind.
// FUNCTION: 0x00093bb8
void draw_slist_nums(void)
{
    u8 i;
    Thing *staff;
    u16 counts[4];
    char buf[100];

    if (othervar.slist_filter & 1)
        reqspr_draw(resvars.screen_buttons[0x30].sprite_x, resvars.screen_buttons[0x30].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x30].sprite]);
    else if (othervar.slist_filter & 2)
        reqspr_draw(resvars.screen_buttons[0x31].sprite_x, resvars.screen_buttons[0x31].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x31].sprite]);
    else if (othervar.slist_filter & 8)
        reqspr_draw(resvars.screen_buttons[0x32].sprite_x, resvars.screen_buttons[0x32].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x32].sprite]);
    else if (othervar.slist_filter & 4)
        reqspr_draw(resvars.screen_buttons[0x33].sprite_x, resvars.screen_buttons[0x33].sprite_y,
                    &screen_sprites_tab_start[resvars.screen_buttons[0x33].sprite]);
    for (i = 0; i < 4; i++)
        counts[i] = 0;
    for (staff = thing + 1; staff < end_things; staff++) {
        if (staff->type == THING_STAFF && staff->owner == player) {
            if (staff->state == SS_APPLICANT)
                continue;
            if (staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON)
                counts[0]++;
            else if (staff->subtype == STAFF_NURSE)
                counts[1]++;
            else if (staff->subtype == STAFF_HANDYMAN)
                counts[2]++;
            else if (staff->subtype == STAFF_RECEPTIONIST)
                counts[3]++;
        }
    }
    set_font_from_cache(0xd);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 4; i++) {
        sprintf(buf, "%d", counts[i]);
        LbTextDraw(resvars.screen_coords[i + 144].x, resvars.screen_coords[i + 144].y, buf);
    }
}

// Covers the doctor-only parts of the staff management screen.
// FUNCTION: 0x00093eac
void draw_slist_coverup(void)
{
    LbDrawBox(resvars.screen_boxes_Boxp[116].x, resvars.screen_boxes_Boxp[116].y, resvars.screen_boxes_Boxp[116].w,
              resvars.screen_boxes_Boxp[116].h, 0x76);
    LbDrawBox(resvars.screen_boxes_Boxp[117].x, resvars.screen_boxes_Boxp[117].y, resvars.screen_boxes_Boxp[117].w,
              resvars.screen_boxes_Boxp[117].h, 0x76);
}

// Help text for a staff row's wage, into othervar.help_msg, if the row is
// filled.
// FUNCTION: 0x00093f38
void slist_salary_help_if_present(void)
{
    if (resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].row < othervar.slist_rows)
        lib_strcpy(othervar.help_msg, help_text[50]);
}

// Help text for a staff row's happiness, into othervar.help_msg, if the row is
// filled.
// FUNCTION: 0x00093f88
void slist_happiness_help_if_present(void)
{
    if (resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].row < othervar.slist_rows)
        lib_strcpy(othervar.help_msg, help_text[51]);
}

// Help text for a staff row's tiredness, into othervar.help_msg, if the row is
// filled.
// FUNCTION: 0x00093fd8
void slist_tiredness_help_if_present(void)
{
    if (resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].row < othervar.slist_rows)
        lib_strcpy(othervar.help_msg, help_text[52]);
}

// Help text for a staff row's ability, into othervar.help_msg, if the row is
// filled.
// FUNCTION: 0x00094028
void slist_ability_help_if_present(void)
{
    if (resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].row < othervar.slist_rows)
        lib_strcpy(othervar.help_msg, help_text[53]);
}

// Help text for a doctor-only part, into othervar.help_msg, if doctors are
// shown.
// FUNCTION: 0x00094078
void slist_help_if_doctor(void)
{
    if (othervar.slist_filter & 1)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].text]);
}

// Help text for the surgeon qualification of the selected doctor, or their
// surgery training progress, into othervar.help_msg.
// FUNCTION: 0x000940cc
void slist_help_if_surgeon(void)
{
    Thing *staff;
    s32 pct;

    staff = &thing[slist.value];
    if ((othervar.slist_filter & 1) && staff > thing && (staff->skills & SKILL_SURGEON)) {
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].text]);
    } else {
        pct = staff->surgeon_training;
        if (pct != 0) {
            pct = pct * 100 / gbv.ability_threshold_surgeon;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.help_msg, help_text[66], pct);
        }
    }
}

// Help text for the psychiatrist qualification of the selected doctor, or
// their psychiatry training progress, into othervar.help_msg.
// FUNCTION: 0x000941bc
void slist_help_if_shrink(void)
{
    Thing *staff;
    s32 pct;

    staff = &thing[slist.value];
    if ((othervar.slist_filter & 1) && staff > thing && (staff->skills & SKILL_PSYCHIATRIST)) {
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].text]);
    } else {
        pct = staff->psych_training;
        if (pct != 0) {
            pct = pct * 100 / gbv.ability_threshold_psych;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.help_msg, help_text[67], pct);
        }
    }
}

// Help text for the researcher qualification of the selected doctor, or
// their research training progress, into othervar.help_msg.
// FUNCTION: 0x000942ac
void slist_help_if_researcher(void)
{
    Thing *staff;
    s32 pct;

    staff = &thing[slist.value];
    if ((othervar.slist_filter & 1) && staff > thing && (staff->skills & SKILL_RESEARCHER)) {
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots_SlistMenuItemp[othervar.help_hotspot].text]);
    } else {
        pct = staff->research_training;
        if (pct != 0) {
            pct = pct * 100 / gbv.ability_threshold_research;
            pct > 100 ? pct = 100 : 0;
            sprintf(othervar.help_msg, help_text[68], pct);
        }
    }
}

// Draws the selected staff member's wage after a 10% rise, and the 10%
// bonus, by their buttons.
// FUNCTION: 0x0009439c
void draw_slist_button_overlays(Thing *staff)
{
    char buf[100];

    set_font_from_cache(1);
    sprintf(buf, "$%d", staff->salary / 10 + staff->salary);
    LbTextDraw(resvars.screen_coords[149].x, resvars.screen_coords[149].y, buf);
    sprintf(buf, "$%d", staff->salary / 10);
    LbTextDraw(resvars.screen_coords[150].x, resvars.screen_coords[150].y, buf);
}

// Draws a training progress bar at (x, y) for value of max.
// FUNCTION: 0x00094458
void draw_slist_training_bar(u16 x, u16 y, u16 value, u16 max)
{
    u8 i;

    for (i = 0; i < value / (max / resvars.screen_boxes_Boxp[115].w); i++) {
        if (i < resvars.screen_boxes_Boxp[115].w)
            reqspr_draw(x + resvars.screen_boxes_Boxp[115].x + i, y + resvars.screen_boxes_Boxp[115].y,
                        &screen_sprites_tab_start[13]);
    }
}
