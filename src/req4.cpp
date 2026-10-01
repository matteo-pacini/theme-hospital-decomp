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

void remove_window_from_list(const u8 n);

u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);
void draw_face(Picture *p, const u16 x, const u16 y);
void draw_staff_handyman_bars(const Thing *const w, const u16 x, const u16 y, TbSprite *spr);

// FUNCTION: 0x00086c10
void req4(Window *win)
{
    WindowButton *button;
    Thing *staff;
    u16 i;

    button = win->buttons;
    staff = &thing[D_001581b2[2].value];
    switch (staff->subtype) {
    case 2:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.unknown4 = i;
                debug.unknown8 = button->flags;
                if (i == resvars.fe[0x26]) {
                    D_001549d0[player].type = 0x43;
                    D_001549d0[player].value = 2;
                    play_sound(10, 0x42, -1, -1);
                } else if (i == resvars.fe[0x27]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.fe[0x27];
                } else if (i == resvars.fe[0x28]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.fe[0x28];
                } else if (i == resvars.fe[0x29]) {
                    D_001549d0[player].type = 0x73;
                    D_001549d0[player].value = D_001581b2[2].value;
                    D_001549d0[player].arg = resvars.fe[0x29];
                } else if (i == resvars.fe[0x2a]) {
                    close_query(2);
                    play_sound(10, 0x43, -1, -1);
                    sprintf(othervar.f935, "%s", yesno_text[5]);
                    open_yes_no_requester(0x47, D_001581b2[2].value, 0, 0x88, D_001581b2[2].value, 0);
                } else if (i == resvars.fe[0x2b]) {
                    D_001549d0[player].type = 0x68;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(10, 0x99, -1, -1);
                }
            }
            i++;
        }
        break;
    default:
        for (i = 1; button; button = button->next) {
            if (button->flags & 6) {
                debug.unknown4 = i;
                if (i == resvars.fe[0x2c]) {
                    D_001549d0[player].type = 0x43;
                    D_001549d0[player].value = 2;
                    play_sound(10, 0x42, -1, -1);
                } else if (i == resvars.fe[0x2d]) {
                    close_query(2);
                    play_sound(10, 0x43, -1, -1);
                    sprintf(othervar.f935, "%s", yesno_text[5]);
                    open_yes_no_requester(0x47, D_001581b2[2].value, 0, 0x88, D_001581b2[2].value, 0);
                } else if (i == resvars.fe[0x2e]) {
                    D_001549d0[player].type = 0x68;
                    D_001549d0[player].value = D_001581b2[2].value;
                    play_sound(10, 0x99, -1, -1);
                }
            }
            i++;
        }
        break;
    }
}

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
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    staff = &thing[D_001581b2[2].value];
    switch (staff->subtype) {
    case 2:
        compute_help_text(0, 0, 0xc);
        set_font_from_cache(1);
        othervar.f36 = window.unknown2ec - 1;
        sprintf(buf, "%d", staff->salary);
        LbTextDraw(x + resvars.f1c[0x6b].x, y + resvars.f1c[0x6b].y, buf);
        draw_staff_handyman_bars(staff, x, y, &button_sprite[resvars.f12[3]]);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x4e].x, y + resvars.f18[0x4e].y, resvars.f18[0x4e].w,
                             resvars.f18[0x4e].h)) {
                othervar.f38 = 2;
                othervar.f10 = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x5d].x, y + resvars.f18[0x5d].y, resvars.f18[0x5d].w,
                             resvars.f18[0x5d].h)) {
                othervar.fa82 = staff - thing;
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = 5;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (lbDisplay.mright_button && !othervar.fa61) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x4e].x, y + resvars.f18[0x4e].y, resvars.f18[0x4e].w,
                             resvars.f18[0x4e].h)) {
                D_001581b2[2].value = find_next_thing(staff, 4, staff->subtype, -1);
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.fa61 = 1;
        else
            othervar.fa61 = 0;
        break;
    default:
        compute_help_text(0, 0, 10);
        set_font_from_cache(1);
        othervar.f36 = window.unknown2ec - 1;
        sprintf(buf, "%d", staff->salary);
        LbTextDraw(x + resvars.f1c[0x66].x, y + resvars.f1c[0x66].y, buf);
        slot = 0;
        slot++;
        if (staff->skills & 8)
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[10]]);
        slot++;
        if (staff->skills & 0x10 && !(staff->skills & 0x80) && !(staff->skills & 8))
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[10]]);
        slot++;
        if (staff->skills & 0x80)
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[10]]);
        slot++;
        if (staff->skills & 0x20)
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[11]]);
        slot++;
        if (staff->skills & 0x40)
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[12]]);
        slot++;
        if (staff->skills & 0x100)
            reqspr_draw(x + resvars.f18[slot + 0x19].x, y + resvars.f18[slot + 0x19].y, &button_sprite[resvars.f12[13]]);
        if (!(staff->subtype == 1 || staff->subtype == 4))
            LbDrawBox(x + resvars.f18[0x40].x, y + resvars.f18[0x40].y, resvars.f18[0x40].w, resvars.f18[0x40].h, 0x7a);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x4d].x, y + resvars.f18[0x4d].y, resvars.f18[0x4d].w,
                             resvars.f18[0x4d].h)) {
                othervar.f38 = 2;
                othervar.f10 = 1;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x5d].x, y + resvars.f18[0x5d].y, resvars.f18[0x5d].w,
                             resvars.f18[0x5d].h)) {
                othervar.fa82 = staff - thing;
                D_001549d0[player].type = 0x33;
                D_001549d0[player].value = 5;
                play_sound(10, 0x43, -1, -1);
            }
        }
        if (lbDisplay.mright_button && !othervar.fa61) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x4d].x, y + resvars.f18[0x4d].y, resvars.f18[0x4d].w,
                             resvars.f18[0x4d].h)) {
                if (staff->subtype == 1 || staff->subtype == 4)
                    D_001581b2[2].value = find_next_thing(staff, 4, 1, 4);
                else
                    D_001581b2[2].value = find_next_thing(staff, 4, staff->subtype, -1);
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.fa61 = 1;
        else
            othervar.fa61 = 0;
        break;
    }
    draw_face((Picture *)&staff->face_head, x + resvars.f1c[0x65].x, y + resvars.f1c[0x65].y);
    draw_gauge(staff->happiness_s, x + resvars.f1c[0x67].x, y + resvars.f1c[0x67].y, resvars.f1c[0x69].x,
               &button_sprite[resvars.f12[0]], 100, 0, 100, 1);
    draw_gauge(staff->tiredness, x + resvars.f1c[0x68].x, y + resvars.f1c[0x68].y, resvars.f1c[0x69].x,
               &button_sprite[resvars.f12[1]], 1000, 0, 1000, 0);
    draw_gauge(staff->ability, x + resvars.f1c[0x6a].x, y + resvars.f1c[0x6a].y, resvars.f1c[0x76].x,
               &button_sprite[resvars.f12[2]], 1000, 30, 970, 1);
    sprintf(buf, "%c. %s%s", staff->initial, staff_names_1[staff->surname_a], staff_names_2[staff->surname_b]);
    LbTextDraw(x + resvars.f1c[0x64].x, y + resvars.f1c[0x64].y, buf);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// FUNCTION: 0x00087b4c
void close_irequesters(void)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    remove_window(open_windows, &resvars.f20[2]);
    remove_window(open_windows, &resvars.f20[0]);
    remove_window(open_windows, &resvars.f20[1]);
    remove_window(open_windows, &resvars.f20[3]);
    while (window.unknown2ec > 1) {
        x = window.zone_x[window.unknown2ec - 1];
        y = window.zone_y[window.unknown2ec - 1];
        w = window.zone_w[window.unknown2ec - 1];
        h = window.zone_h[window.unknown2ec - 1];
        set_all_refresh_zones_in_window(x, y, w, h, 3);
        remove_window_from_list(window.unknown2ec - 1);
    }
    window.unknown2eb = 0;
    cancel_helpmsg();
    othervar.f9b8 = 0;
}

// FUNCTION: 0x00087c44
void adjust_handyman_values(const u16 id, const u8 button)
{
    Thing *handyman;
    u8 raised;
    u8 other1;
    u8 other2;

    handyman = &thing[id];
    if (button == resvars.fe[0x27]) {
        raised = handyman->prio_litter;
        other1 = handyman->prio_plants;
        other2 = handyman->prio_repair;
    } else if (button == resvars.fe[0x28]) {
        raised = handyman->prio_plants;
        other1 = handyman->prio_litter;
        other2 = handyman->prio_repair;
    } else if (button == resvars.fe[0x29]) {
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
            play_sound(10, 0x43, -1, -1);
    } else {
        raised = 100;
        other1 = 0;
        other2 = 0;
    }
    if (button == resvars.fe[0x27]) {
        handyman->prio_litter = raised;
        handyman->prio_plants = other1;
        handyman->prio_repair = other2;
    } else if (button == resvars.fe[0x28]) {
        handyman->prio_plants = raised;
        handyman->prio_litter = other1;
        handyman->prio_repair = other2;
    } else if (button == resvars.fe[0x29]) {
        handyman->prio_repair = raised;
        handyman->prio_plants = other1;
        handyman->prio_litter = other2;
    }
    handyman->litter_count = 0;
    handyman->plant_count = 0;
    handyman->repair_count = 0;
}

// FUNCTION: 0x00087e7c
void draw_staff_handyman_bars(const Thing *const handyman, const u16 x, const u16 y, TbSprite *spr)
{
    draw_gauge(handyman->prio_litter, x + resvars.f1c[0x72].x, y + resvars.f1c[0x72].y, resvars.f1c[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(handyman->prio_plants, x + resvars.f1c[0x73].x, y + resvars.f1c[0x72].y, resvars.f1c[0xaa].x, spr, 100, 0, 100, 1);
    draw_gauge(handyman->prio_repair, x + resvars.f1c[0x74].x, y + resvars.f1c[0x72].y, resvars.f1c[0xaa].x, spr, 100, 0, 100, 1);
}

// FUNCTION: 0x00087fb8
void setup_req4_references(void)
{
    u8 i;
    char buf[4];

    sprintf(buf, ".");
    for (i = 0; ref_general_text[i][0] != buf[0]; i++) {}
    statvar.f5d9 = i;
    for (i = 0; ref_good_text[i][0] != buf[0]; i++) {}
    statvar.f5da = i;
    for (i = 0; ref_bad_text[i][0] != buf[0]; i++) {}
    statvar.f5db = i;
}

// FUNCTION: 0x00088068
void req4_help_if_doctor(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & 0x10)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x000880e4
void req4_help_if_surgeon(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & 0x20)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x00088160
void req4_help_if_shrink(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & 0x40)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x000881dc
void req4_help_if_research(void)
{
    Thing *staff;

    staff = &thing[D_001581b2[2].value];
    if (staff > thing && staff->skills & 0x100)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}
