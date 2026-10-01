#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

struct ReqLayout {
    u8 unknown0[0x230];
    s32 graph_x;
    s32 graph_bottom;
    s32 graph_width;
    s32 graph_height;
    u8 unknown240[0x120];
    s32 gauge2_x;
    s32 gauge2_y;
    s32 gauge3_x;
    s32 gauge3_y;
    u8 unknown370[0x70];
    Box box_a;
    Box box_b;
    u8 unknown400[0xf0];
    Box next;
    Box diag;
    Box box_c;
    Box box_d;
    u8 unknown530[0x20];
    s32 gauge_x;
    s32 gauge_y;
    s32 gauge_len;
};

extern "C" {
extern Slot D_001581b2[];
extern Event D_001549d0[];
extern char **illness_text;
void conv_string(char *dst, char *src, u8 mode);
void fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
}

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 n);
void draw_health_graph(Thing *w, u16 x, u16 y, u16 colour);
void draw_diagnosis(Thing *w, u16 x, u16 y);
u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);

// FUNCTION: 0x00072d60
void req3(Window *win)
{
    WindowButton *button;
    Thing *patient;
    s32 id;
    u16 i;

    button = win->buttons;
    patient = &thing[D_001581b2[1].value];
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.unknown4 = i;
            debug.unknown8 = i;
            if (i == resvars.fe[0xd]) {
                D_001549d0[player].type = EV_CLOSE_QUERY;
                D_001549d0[player].value = 1;
                play_sound(10, SFX_NO4, -1, -1);
            } else if (i == resvars.fe[0xe]) {
                if (patient->diagnosis >= gbv.ffc) {
                    display_mode = 1;
                    chart_type = 1;
                    drugs.unknown2 = 0;
                    set_mouse_mode(9);
                    othervar.f8c = patient->illness;
                    play_sound(10, SFX_SELECTX, -1, -1);
                }
            } else if (i == resvars.fe[0xf]) {
                if (!(patient->flags & PF_CURED || patient->flags & PF_SENT_HOME)) {
                    if (patient->health > 0) {
                        D_001549d0[player].type = EV_PERSON_GO_HOME;
                        D_001549d0[player].value = D_001581b2[1].value;
                        play_sound(10, SFX_SACK, -1, -1);
                    }
                }
            } else if (i == resvars.fe[0x10]) {
                if (patient->queue_room) {
                    D_001549d0[player].type = 0x3b;
                    D_001549d0[player].value = patient->queue_room;
                    D_001549d0[player].arg = 0;
                    play_sound(10, SFX_SELECTX, -1, -1);
                }
            } else if (i == resvars.fe[0x11]) {
                if (!(patient->treatment == 0 || patient->flags & PF_GUESSED || patient->flags & PF_FED_UP || patient->illness == patient->known_illness)
                    && patient->diagnosis >= gbv.ffc) {
                    if (!(patient->treatment == 0 || patient->flags & PF_GUESSED)) {
                        if (patient->health > 0) {
                            D_001549d0[player].type = EV_FORCE_CURE;
                            D_001549d0[player].value = D_001581b2[1].value;
                            play_sound(10, SFX_SELECTX, -1, -1);
                        }
                    }
                }
            }
        }
        i++;
    }
}

// FUNCTION: 0x0007308c
void req3_overdraw(Window *win)
{
    Thing *patient;
    s32 unused;
    s32 x;
    s32 y;
    s32 n;

    patient = &thing[D_001581b2[1].value];
    n = 0x3e;
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                              window.unknown2cf - window.unknown2c7);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    othervar.f35 = window.unknown2ec - 1;
    if (D_001581b2[1].f6)
        D_001581b2[1].f6--;
    if (patient->diagnosis < gbv.ffc) {
        LbDrawBox(x + resvars.f18_ReqLayoutp->box_a.x, y + resvars.f18_ReqLayoutp->box_a.y, resvars.f18_ReqLayoutp->box_a.w, resvars.f18_ReqLayoutp->box_a.h, 0x46);
        LbDrawBox(x + resvars.f18_ReqLayoutp->box_c.x, y + resvars.f18_ReqLayoutp->box_c.y, resvars.f18_ReqLayoutp->box_c.w, resvars.f18_ReqLayoutp->box_c.h, 0x46);
    }
    if (!patient->queue_room)
        LbDrawBox(x + resvars.f18_ReqLayoutp->box_b.x, y + resvars.f18_ReqLayoutp->box_b.y, resvars.f18_ReqLayoutp->box_b.w, resvars.f18_ReqLayoutp->box_b.h, 0x46);
    if (patient->treatment == 0 || patient->flags & PF_GUESSED || patient->flags & PF_FED_UP || patient->illness == patient->known_illness
        || patient->diagnosis < gbv.ffc) {
        LbDrawBox(x + resvars.f18_ReqLayoutp->box_c.x, y + resvars.f18_ReqLayoutp->box_c.y, resvars.f18_ReqLayoutp->box_c.w, resvars.f18_ReqLayoutp->box_c.h, 0x46);
        if (patient->flags & PF_CURED || patient->flags & PF_SENT_HOME)
            LbDrawBox(x + resvars.f18_ReqLayoutp->box_d.x, y + resvars.f18_ReqLayoutp->box_d.y, resvars.f18_ReqLayoutp->box_d.w, resvars.f18_ReqLayoutp->box_d.h, 0x46);
    }
    draw_gauge(patient->happiness_s, x + resvars.f18_ReqLayoutp->gauge_x, y + resvars.f18_ReqLayoutp->gauge_y, (u16)resvars.f18_ReqLayoutp->gauge_len,
               &button_sprite[resvars.f12[0]], 100, 0, 100, 1);
    draw_gauge(patient->refreshment, x + resvars.f1c_ReqLayoutp->gauge2_x, y + resvars.f1c_ReqLayoutp->gauge2_y, (u16)resvars.f18_ReqLayoutp->gauge_len,
               &button_sprite[resvars.f12[0x10]], 100, 0, 100, 1);
    draw_gauge((s16)patient->warmth, x + resvars.f1c_ReqLayoutp->gauge3_x, y + resvars.f1c_ReqLayoutp->gauge3_y, (u16)resvars.f18_ReqLayoutp->gauge_len,
               &button_sprite[resvars.f12[0x11]], 100, 0, 100, 1);
    if (D_001581b2[1].f4)
        draw_health_graph(patient, x, y, color.unknownD);
    else
        draw_diagnosis(patient, x, y);
    if (lbDisplay.mleft_button) {
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18_ReqLayoutp->next.x, y + resvars.f18_ReqLayoutp->next.y, resvars.f18_ReqLayoutp->next.w,
                         resvars.f18_ReqLayoutp->next.h)) {
            othervar.f38 = 1;
            othervar.f10 = 1;
        } else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18_ReqLayoutp->diag.x, y + resvars.f18_ReqLayoutp->diag.y,
                                resvars.f18_ReqLayoutp->diag.w, resvars.f18_ReqLayoutp->diag.h)) {
            if (!D_001581b2[1].f6) {
                D_001581b2[1].f4 = !D_001581b2[1].f4 ? 1 : 0;
                D_001581b2[1].f6 = 5;
            }
        }
    }
    if (lbDisplay.mright_button && !othervar.fa61) {
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18_ReqLayoutp->next.x, y + resvars.f18_ReqLayoutp->next.y, resvars.f18_ReqLayoutp->next.w,
                         resvars.f18_ReqLayoutp->next.h)) {
            u16 old;

            old = D_001581b2[1].value;
            D_001581b2[1].value = find_next_thing(patient, THING_PATIENT, -1, -1);
            if (old != D_001581b2[1].value)
                play_sound(10, SFX_CAMCLICK, -1, -1);
        }
    }
    if (lbDisplay.mright_button)
        othervar.fa61 = 1;
    else
        othervar.fa61 = 0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
    compute_help_text(0, 0, 9);
}

// FUNCTION: 0x000736d4
void close_query(u8 i)
{
    close_irequesters();
    D_001581b2[i].kind = 0x63;
}

// FUNCTION: 0x00073704
void draw_health_graph(Thing *patient, u16 x, u16 y, u16 colour)
{
    u8 i;
    u8 step;
    u8 point;
    u16 y1;
    u16 x2;
    u16 y2;
    s16 h1;
    s16 h2;
    u16 x1;

    h1 = 0;
    h2 = 0;
    point = 0;
    step = resvars.f18_ReqLayoutp->graph_width / 10;
    for (i = 1; i < 9; i++) {
        h1 = patient->health_history[i] * 10;
        h2 = patient->health_history[i + 1] * 10;
        h1 = resvars.f18_ReqLayoutp->graph_height * h1 / 1000;
        h2 = h2 * resvars.f18_ReqLayoutp->graph_height / 1000;
        if (h1 < 1)
            h1 = 1;
        else if (h1 > resvars.f18_ReqLayoutp->graph_height)
            h1 = resvars.f18_ReqLayoutp->graph_height;
        if (h2 < 1)
            h2 = 1;
        else if (h2 > resvars.f18_ReqLayoutp->graph_height)
            h2 = resvars.f18_ReqLayoutp->graph_height;
        y1 = resvars.f18_ReqLayoutp->graph_bottom - h1;
        y2 = resvars.f18_ReqLayoutp->graph_bottom - h2;
        x1 = resvars.f18_ReqLayoutp->graph_x + point++ * step;
        x2 = resvars.f18_ReqLayoutp->graph_x + point * step;
        fn_000c9be0(x + x1, y + y1, x + x2, y + y2, colour);
    }
}

// FUNCTION: 0x000738e8
void draw_diagnosis(Thing *patient, u16 x, u16 y)
{
    u8 i;
    u16 dy;
    char buf[100];

    LbDrawBox(x + resvars.f18_ReqLayoutp->diag.x, y + resvars.f18_ReqLayoutp->diag.y, resvars.f18_ReqLayoutp->diag.w, resvars.f18_ReqLayoutp->diag.h, 0xa9);
    LbTextSetClipWindow(x + resvars.f18_ReqLayoutp->diag.x, y + resvars.f18_ReqLayoutp->diag.y, resvars.f18_ReqLayoutp->diag.w, resvars.f18_ReqLayoutp->diag.h);
    set_font_from_cache(3);
    dy = 0;
    for (i = 0; i < 10; i++) {
        if (!patient->treatments[i])
            break;
        if (resvars.f1 == SCREEN_MODE_13) {
            sprintf(buf, " %s", illness_text[patient->treatments[i]]);
            conv_string(buf, buf, 0);
        } else {
            sprintf(buf, "%s", illness_text[patient->treatments[i]]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + resvars.f18_ReqLayoutp->diag.x, dy + y + resvars.f18_ReqLayoutp->diag.y, buf);
    }
    if (patient->flags & PF_CURED) {
        if (resvars.f1 == SCREEN_MODE_13) {
            sprintf(buf, " %s", info_bar_text[8]);
            conv_string(buf, buf, 0);
        } else {
            sprintf(buf, "%s", info_bar_text[8]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + resvars.f18_ReqLayoutp->diag.x, dy + y + resvars.f18_ReqLayoutp->diag.y, buf);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, statvar.unknown4);
}

// FUNCTION: 0x00073bb0
u16 find_next_thing(Thing *cand, u8 kind, s8 sub1, s8 sub2)
{
    u8 found;

    found = 0;
    cand++;
    if (cand >= end_things)
        cand = thing;
    if (sub1 >= 0) {
        for (; cand < end_things; cand++) {
            if (cand->type == kind) {
                if (cand->subtype == sub1 || cand->subtype == sub2) {
                    if (cand->owner == player) {
                        if (check_if_point_beyond_scroll_limit((u8)cand->x.tile, cand->y.tile, 0)) {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (cand = thing; cand < end_things; cand++) {
                if (cand->type == kind) {
                    if (cand->subtype == sub1 || cand->subtype == sub2) {
                        if (cand->owner == player) {
                            if (check_if_point_beyond_scroll_limit((u8)cand->x.tile, cand->y.tile, 0))
                                break;
                        }
                    }
                }
            }
        }
    } else {
        for (; cand < end_things; cand++) {
            if (cand->type == kind) {
                if (cand->owner == player) {
                    if (check_if_point_beyond_scroll_limit((u8)cand->x.tile, cand->y.tile, 0)) {
                        if (cand->type == THING_OBJECT) {
                            if (objects[cand->subtype].f20) {
                                found = 1;
                                break;
                            }
                        } else {
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (!found) {
            for (cand = thing; cand < end_things; cand++) {
                if (cand->type == kind) {
                    if (cand->owner == player) {
                        if (check_if_point_beyond_scroll_limit((u8)cand->x.tile, cand->y.tile, 0)) {
                            if (cand->type == THING_OBJECT) {
                                if (objects[cand->subtype].f20)
                                    break;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return cand - thing;
}

// FUNCTION: 0x00073fbc
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up)
{
    u32 filled;
    u32 i;

    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    if (up)
        filled = len * value / range;
    else
        filled = len - len * value / range;
    for (i = 0; i < filled; i++)
        reqspr_draw(x + i, y, spr);
}

// FUNCTION: 0x00074064
extern "C" void fn_00074064(void)
{
    Thing *patient;

    patient = &thing[D_001581b2[1].value];
    if (patient->diagnosis >= gbv.ffc)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x000740e4
extern "C" void fn_000740e4(void)
{
    Thing *patient;

    patient = &thing[D_001581b2[1].value];
    if (patient->treatment != 0 && !(patient->flags & PF_GUESSED) && !(patient->flags & PF_FED_UP) && patient->illness != patient->known_illness
        && patient->diagnosis >= gbv.ffc)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x000741a4
extern "C" void fn_000741a4(void)
{
    Thing *patient;

    patient = &thing[D_001581b2[1].value];
    if (patient->queue_room)
        fn_000bbff0(othervar.f96, help_text[resvars.f2c[othervar.f8ac].text]);
}

// FUNCTION: 0x00073e64
u16 find_next_emergency_person(u16 id, u32 flags)
{
    Thing *person;
    u8 found;

    person = &thing[id];
    found = 0;
    person++;
    if (person >= end_things)
        person = thing;
    for (; person < end_things; person++) {
        if (person->type == THING_PATIENT) {
            if (person->owner == player) {
                if (check_if_point_beyond_scroll_limit((u8)person->x.tile, person->y.tile, 0)) {
                    if (person->flags32 & flags) {
                        found = 1;
                        break;
                    }
                }
            }
        }
    }
    if (!found) {
        for (person = thing; person < end_things; person++) {
            if (person->type == THING_PATIENT) {
                if (person->owner == player) {
                    if (check_if_point_beyond_scroll_limit((u8)person->x.tile, person->y.tile, 0)) {
                        if (person->flags32 & flags)
                            break;
                    }
                }
            }
        }
    }
    return person - thing;
}
