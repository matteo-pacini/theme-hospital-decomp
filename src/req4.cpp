#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct Picture;

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

extern "C" {
extern Slot D_001581b2[];
extern Event D_001549d0[];
}

void remove_window_from_list(const u8 win);

u16 find_next_thing(Thing *cand, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);
void draw_face(Picture *face, const u16 x, const u16 y);
void draw_staff_handyman_bars(const Thing *const handyman, const u16 x, const u16 y, TbSprite *spr);

// Staff window clicks (staff D_001581b2[2].value): close, sack (asks to
// confirm), pick up (event EV_PICK_UP_STAFF), and for a handyman the three priority
// buttons (EV_ADJUST_HANDYMAN_VALUES).
// FUNCTION: 0x00086c10
void req4(Window *win)
{
    WindowButton *button;
    Thing *staff;
    u16 i;

    button = win->buttons;
    staff = &thing[D_001581b2[2].value];
    switch (staff->subtype) {
    case STAFF_HANDYMAN:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.last_button = i;
                debug.last_action = button->flags;
                if (i == resvars.requester_buttons[0x26]) {
                    D_001549d0[player].type = EV_CLOSE_QUERY;
                    D_001549d0[player].value = 2;
                    play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                } else if (i == resvars.requester_buttons[0x27]) {
                    D_001549d0[player].type = EV_ADJUST_HANDYMAN_VALUES;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.requester_buttons[0x27];
                } else if (i == resvars.requester_buttons[0x28]) {
                    D_001549d0[player].type = EV_ADJUST_HANDYMAN_VALUES;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.requester_buttons[0x28];
                } else if (i == resvars.requester_buttons[0x29]) {
                    D_001549d0[player].type = EV_ADJUST_HANDYMAN_VALUES;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.requester_buttons[0x29];
                } else if (i == resvars.requester_buttons[0x2a]) {
                    close_query(2);
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    sprintf(othervar.yesno_msg, "%s", yesno_text[5]);
                    open_yes_no_requester(EV_SACK_STAFF, D_001581b2[2].value, 0, EV_CANCEL_SACK, D_001581b2[2].value, 0);
                } else if (i == resvars.requester_buttons[0x2b]) {
                    D_001549d0[player].type = EV_PICK_UP_STAFF;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(SNDSRC_GENERAL, SFX_PICKUP, -1, -1);
                }
            }
            i++;
        }
        break;
    default:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.last_button = i;
                if (i == resvars.requester_buttons[0x2c]) {
                    D_001549d0[player].type = EV_CLOSE_QUERY;
                    D_001549d0[player].value = 2;
                    play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                } else if (i == resvars.requester_buttons[0x2d]) {
                    close_query(2);
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    sprintf(othervar.yesno_msg, "%s", yesno_text[5]);
                    open_yes_no_requester(EV_SACK_STAFF, D_001581b2[2].value, 0, EV_CANCEL_SACK, D_001581b2[2].value, 0);
                } else if (i == resvars.requester_buttons[0x2e]) {
                    D_001549d0[player].type = EV_PICK_UP_STAFF;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(SNDSRC_GENERAL, SFX_PICKUP, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

// Draws the staff window: wage, qualifications or a handyman's priorities,
// face, happiness, tiredness and ability gauges and name. A left click on
// the view centres the map on them, a right click moves to the next staff
// member of their kind; the staff list button opens it on them.
// FUNCTION: 0x0008704c
void req4_overdraw(Window *win)
{
    u8 unused_a;
    u8 unused_b;
    u8 unused_c;
    u8 slot;
    s32 y;
    s32 x;
    Thing *staff;
    char buf[100];

    unused_a = 0;
    unused_b = 0;
    unused_c = 0;
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    staff = &thing[D_001581b2[2].value];
    switch (staff->subtype) {
    case STAFF_HANDYMAN:
        compute_help_text(0, 0, 0xc);
        set_font_from_cache(1);
        othervar.track_window_2 = window.window_count - 1;
        sprintf(buf, "%d", staff->salary);
        LbTextDraw(x + resvars.coords[0x6b].x, y + resvars.coords[0x6b].y, buf);
        draw_staff_handyman_bars(staff, x, y, &button_sprite[resvars.requester_sprites[3]]);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x4e].x, y + resvars.boxes[0x4e].y, resvars.boxes[0x4e].w,
                             resvars.boxes[0x4e].h)) {
                othervar.center_on_slot = 2;
                othervar.full_redraw = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x5d].x, y + resvars.boxes[0x5d].y, resvars.boxes[0x5d].w,
                             resvars.boxes[0x5d].h)) {
                othervar.slist_staff = staff - thing;
                D_001549d0[player].type = EV_OPEN_SCREEN;
                D_001549d0[player].value = 5;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            }
        }
        if (lbDisplay.mright_button && !othervar.right_button_was_down) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x4e].x, y + resvars.boxes[0x4e].y, resvars.boxes[0x4e].w,
                             resvars.boxes[0x4e].h)) {
                D_001581b2[2].value = find_next_thing(staff, THING_STAFF, staff->subtype, -1);
                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.right_button_was_down = 1;
        else
            othervar.right_button_was_down = 0;
        break;
    default:
        compute_help_text(0, 0, 10);
        set_font_from_cache(1);
        othervar.track_window_2 = window.window_count - 1;
        sprintf(buf, "%d", staff->salary);
        LbTextDraw(x + resvars.coords[0x66].x, y + resvars.coords[0x66].y, buf);
        slot = 0;
        slot++;
        if (staff->skills & SKILL_JUNIOR)
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[10]]);
        slot++;
        if (staff->skills & SKILL_DOCTOR && !(staff->skills & SKILL_CONSULTANT) && !(staff->skills & SKILL_JUNIOR))
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[10]]);
        slot++;
        if (staff->skills & SKILL_CONSULTANT)
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[10]]);
        slot++;
        if (staff->skills & SKILL_SURGEON)
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[11]]);
        slot++;
        if (staff->skills & SKILL_PSYCHIATRIST)
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[12]]);
        slot++;
        if (staff->skills & SKILL_RESEARCHER)
            reqspr_draw(x + resvars.boxes[slot + 0x19].x, y + resvars.boxes[slot + 0x19].y, &button_sprite[resvars.requester_sprites[13]]);
        if (!(staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON))
            LbDrawBox(x + resvars.boxes[0x40].x, y + resvars.boxes[0x40].y, resvars.boxes[0x40].w, resvars.boxes[0x40].h, 0x7a);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x4d].x, y + resvars.boxes[0x4d].y, resvars.boxes[0x4d].w,
                             resvars.boxes[0x4d].h)) {
                othervar.center_on_slot = 2;
                othervar.full_redraw = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x5d].x, y + resvars.boxes[0x5d].y, resvars.boxes[0x5d].w,
                             resvars.boxes[0x5d].h)) {
                othervar.slist_staff = staff - thing;
                D_001549d0[player].type = EV_OPEN_SCREEN;
                D_001549d0[player].value = 5;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            }
        }
        if (lbDisplay.mright_button && !othervar.right_button_was_down) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x4d].x, y + resvars.boxes[0x4d].y, resvars.boxes[0x4d].w,
                             resvars.boxes[0x4d].h)) {
                if (staff->subtype == STAFF_DOCTOR || staff->subtype == STAFF_SURGEON)
                    D_001581b2[2].value = find_next_thing(staff, THING_STAFF, STAFF_DOCTOR, STAFF_SURGEON);
                else
                    D_001581b2[2].value = find_next_thing(staff, THING_STAFF, staff->subtype, -1);
                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.right_button_was_down = 1;
        else
            othervar.right_button_was_down = 0;
        break;
    }
    draw_face((Picture *)&staff->face_head, x + resvars.coords[0x65].x, y + resvars.coords[0x65].y);
    draw_gauge(staff->happiness_s, x + resvars.coords[0x67].x, y + resvars.coords[0x67].y, resvars.coords[0x69].x,
               &button_sprite[resvars.requester_sprites[0]], 100, 0, 100, 1);
    draw_gauge(staff->tiredness, x + resvars.coords[0x68].x, y + resvars.coords[0x68].y, resvars.coords[0x69].x,
               &button_sprite[resvars.requester_sprites[1]], 1000, 0, 1000, 0);
    draw_gauge(staff->ability, x + resvars.coords[0x6a].x, y + resvars.coords[0x6a].y, resvars.coords[0x76].x,
               &button_sprite[resvars.requester_sprites[2]], 1000, 30, 970, 1);
    sprintf(buf, "%c. %s%s", staff->initial, staff_names_1[staff->surname_a], staff_names_2[staff->surname_b]);
    LbTextDraw(x + resvars.coords[0x64].x, y + resvars.coords[0x64].y, buf);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// Closes the patient, staff and other query windows and every window opened
// over the view, and cancels the help message.
// FUNCTION: 0x00087b4c
void close_irequesters(void)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    remove_window(open_windows, &resvars.window_list[2]);
    remove_window(open_windows, &resvars.window_list[0]);
    remove_window(open_windows, &resvars.window_list[1]);
    remove_window(open_windows, &resvars.window_list[3]);
    while (window.window_count > 1) {
        x = window.zone_x[window.window_count - 1];
        y = window.zone_y[window.window_count - 1];
        w = window.zone_x1[window.window_count - 1];
        h = window.zone_y1[window.window_count - 1];
        set_all_refresh_zones_in_window(x, y, w, h, 3);
        remove_window_from_list(window.window_count - 1);
    }
    window.current_window = 0;
    cancel_helpmsg();
    othervar.irequester_slot = 0;
}

// Raises handyman id's priority for the job of button `button` (a
// resvars.requester_buttons index: litter, plants or repairs) by 6, taking it from the
// larger of the other two so the three sum to at most 100; from 94 it
// becomes 100 and the others 0. Resets their job counts.
// FUNCTION: 0x00087c44
void adjust_handyman_values(const u16 id, const u8 button)
{
    Thing *handyman;
    u8 raised;
    u8 other1;
    u8 other2;

    handyman = &thing[id];
    if (button == resvars.requester_buttons[0x27]) {
        raised = handyman->prio_litter;
        other1 = handyman->prio_plants;
        other2 = handyman->prio_repair;
    } else if (button == resvars.requester_buttons[0x28]) {
        raised = handyman->prio_plants;
        other1 = handyman->prio_litter;
        other2 = handyman->prio_repair;
    } else if (button == resvars.requester_buttons[0x29]) {
        raised = handyman->prio_repair;
        other1 = handyman->prio_plants;
        other2 = handyman->prio_litter;
    }
    if (raised < 0x5e) {
        raised += 6;
        while (raised + other1 + other2 > 100) {
            if (other1 > other2)
                other1--;
            else
                other2--;
        }
        if (handyman->owner == player)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
    } else {
        raised = 100;
        other1 = 0;
        other2 = 0;
    }
    if (button == resvars.requester_buttons[0x27]) {
        handyman->prio_litter = raised;
        handyman->prio_plants = other1;
        handyman->prio_repair = other2;
    } else if (button == resvars.requester_buttons[0x28]) {
        handyman->prio_plants = raised;
        handyman->prio_litter = other1;
        handyman->prio_repair = other2;
    } else if (button == resvars.requester_buttons[0x29]) {
        handyman->prio_repair = raised;
        handyman->prio_plants = other1;
        handyman->prio_litter = other2;
    }
    handyman->litter_count = 0;
    handyman->plant_count = 0;
    handyman->repair_count = 0;
}

// Draws a handyman's three priority gauges (litter, plants, repairs) at
// window (x, y) with sprite spr.
// FUNCTION: 0x00087e7c
void draw_staff_handyman_bars(const Thing *const handyman, const u16 x, const u16 y, TbSprite *spr)
{
    draw_gauge(handyman->prio_litter, x + resvars.coords[0x72].x, y + resvars.coords[0x72].y, resvars.coords[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(handyman->prio_plants, x + resvars.coords[0x73].x, y + resvars.coords[0x72].y, resvars.coords[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(handyman->prio_repair, x + resvars.coords[0x74].x, y + resvars.coords[0x72].y, resvars.coords[0xaa].x, spr, 100, 0, 100, 1);
}

// Counts the reference texts (general, good and bad, each list ending at
// ".") into statvar.ref_general_count-f5db.
// FUNCTION: 0x00087fb8
void setup_req4_references(void)
{
    u8 i;
    char buf[4];

    sprintf(buf, ".");
    for (i = 0; ref_general_text[i][0] != buf[0]; i++) {}
    statvar.ref_general_count = i;
    for (i = 0; ref_good_text[i][0] != buf[0]; i++) {}
    statvar.ref_good_count = i;
    for (i = 0; ref_bad_text[i][0] != buf[0]; i++) {}
    statvar.ref_bad_count = i;
}

// Help text for a doctor qualification of the staff window's staff member,
// into othervar.help_msg, if they are a doctor.
// FUNCTION: 0x00088068
void req4_help_if_doctor(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & SKILL_DOCTOR)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req4_help_if_doctor, if they are a surgeon.
// FUNCTION: 0x000880e4
void req4_help_if_surgeon(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & SKILL_SURGEON)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req4_help_if_doctor, if they are a psychiatrist.
// FUNCTION: 0x00088160
void req4_help_if_shrink(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & SKILL_PSYCHIATRIST)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// As req4_help_if_doctor, if they are a researcher.
// FUNCTION: 0x000881dc
void req4_help_if_research(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & SKILL_RESEARCHER)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}
