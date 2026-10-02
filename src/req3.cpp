#include "hosp.h"

/*
 * A tracked-window slot (irequester_slots[]); see io.cpp.
 */
struct Slot {
    // 0x0: Requester shown (REQ_).
    u8 kind;
    u8 unknown1[0x1];
    // 0x2: Thing shown.
    u16 value;
    // 0x4: Nonzero shows the health graph, 0 the diagnosis.
    s16 show_graph;
    // 0x6: Frames before the graph/diagnosis toggle works again.
    u8 toggle_delay;
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
extern Slot irequester_slots[];
extern Event player_events[];
extern char **illness_text;
void conv_string(char *dst, char *src, u8 mode);
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
}

u8 check_if_point_beyond_scroll_limit(const s32 x, const s32 y, const u8 with_margin);
void draw_health_graph(Thing *patient, u16 x, u16 y, u16 colour);
void draw_diagnosis(Thing *patient, u16 x, u16 y);
u16 find_next_thing(Thing *cand, u8 kind, s8 sub1, s8 sub2);
void draw_gauge(long value, const u16 x, const u16 y, const u16 len, TbSprite *spr, const long max, const long min,
                const long range, const u8 up);

// Patient window clicks (patient irequester_slots[1].value): close, the casebook
// for their illness once diagnosed, send home, their queue's window, and
// force a cure (EV_FORCE_CURE) when diagnosed with a treatment set, not on a
// guess, not fed up, and known_illness differs from illness.
// FUNCTION: 0x00072d60
void req3(Window *win)
{
    WindowButton *button;
    Thing *patient;
    s32 id;
    u16 i;

    button = win->buttons;
    patient = &thing[irequester_slots[1].value];
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = i;
            debug.last_action = i;
            if (i == resvars.requester_buttons[0xd]) {
                player_events[player].type = EV_CLOSE_QUERY;
                player_events[player].value = 1;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            } else if (i == resvars.requester_buttons[0xe]) {
                if (patient->diagnosis >= gbv.diagnosis_complete) {
                    display_mode = DM_CHARTS;
                    chart_type = 1;
                    drugs.state = 0;
                    set_mouse_mode(MM_SCREEN);
                    othervar.selected_illness = patient->illness;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                }
            } else if (i == resvars.requester_buttons[0xf]) {
                if (!(patient->flags & PF_CURED || patient->flags & PF_SENT_HOME)) {
                    if (patient->health > 0) {
                        player_events[player].type = EV_PERSON_GO_HOME;
                        player_events[player].value = irequester_slots[1].value;
                        play_sound(SNDSRC_GENERAL, SFX_SACK, -1, -1);
                    }
                }
            } else if (i == resvars.requester_buttons[0x10]) {
                if (patient->queue_room) {
                    player_events[player].type = EV_OPEN_QUEUE_WINDOW;
                    player_events[player].value = patient->queue_room;
                    player_events[player].arg = 0;
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                }
            } else if (i == resvars.requester_buttons[0x11]) {
                if (!(patient->treatment == 0 || patient->flags & PF_GUESSED || patient->flags & PF_FED_UP ||
                      patient->illness == patient->known_illness) &&
                    patient->diagnosis >= gbv.diagnosis_complete) {
                    if (!(patient->treatment == 0 || patient->flags & PF_GUESSED)) {
                        if (patient->health > 0) {
                            player_events[player].type = EV_FORCE_CURE;
                            player_events[player].value = irequester_slots[1].value;
                            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                        }
                    }
                }
            }
        }
        i++;
    }
}

// Draws the patient window: greys the unavailable buttons, the happiness,
// thirst and warmth gauges, and the health graph or diagnosis list (toggled
// by clicking it); a left click on the next button centres the view on the
// patient, a right click moves to the next patient.
// FUNCTION: 0x0007308c
void req3_overdraw(Window *win)
{
    Thing *patient;
    s32 unused;
    s32 x;
    s32 y;
    s32 n;

    patient = &thing[irequester_slots[1].value];
    n = 0x3e;
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    x = win->x - lbDisplay.graphics_window_x;
    y = win->y - lbDisplay.graphics_window_y;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    othervar.track_window_1 = window.window_count - 1;
    if (irequester_slots[1].toggle_delay)
        irequester_slots[1].toggle_delay--;
    if (patient->diagnosis < gbv.diagnosis_complete) {
        LbDrawBox(x + resvars.boxes_ReqLayoutp->box_a.x, y + resvars.boxes_ReqLayoutp->box_a.y,
                  resvars.boxes_ReqLayoutp->box_a.w, resvars.boxes_ReqLayoutp->box_a.h, 0x46);
        LbDrawBox(x + resvars.boxes_ReqLayoutp->box_c.x, y + resvars.boxes_ReqLayoutp->box_c.y,
                  resvars.boxes_ReqLayoutp->box_c.w, resvars.boxes_ReqLayoutp->box_c.h, 0x46);
    }
    if (!patient->queue_room)
        LbDrawBox(x + resvars.boxes_ReqLayoutp->box_b.x, y + resvars.boxes_ReqLayoutp->box_b.y,
                  resvars.boxes_ReqLayoutp->box_b.w, resvars.boxes_ReqLayoutp->box_b.h, 0x46);
    if (patient->treatment == 0 || patient->flags & PF_GUESSED || patient->flags & PF_FED_UP ||
        patient->illness == patient->known_illness || patient->diagnosis < gbv.diagnosis_complete) {
        LbDrawBox(x + resvars.boxes_ReqLayoutp->box_c.x, y + resvars.boxes_ReqLayoutp->box_c.y,
                  resvars.boxes_ReqLayoutp->box_c.w, resvars.boxes_ReqLayoutp->box_c.h, 0x46);
        if (patient->flags & PF_CURED || patient->flags & PF_SENT_HOME)
            LbDrawBox(x + resvars.boxes_ReqLayoutp->box_d.x, y + resvars.boxes_ReqLayoutp->box_d.y,
                      resvars.boxes_ReqLayoutp->box_d.w, resvars.boxes_ReqLayoutp->box_d.h, 0x46);
    }
    draw_gauge(patient->happiness_s, x + resvars.boxes_ReqLayoutp->gauge_x, y + resvars.boxes_ReqLayoutp->gauge_y,
               (u16)resvars.boxes_ReqLayoutp->gauge_len, &button_sprite[resvars.requester_sprites[0]], 100, 0, 100, 1);
    draw_gauge(patient->refreshment, x + resvars.coords_ReqLayoutp->gauge2_x, y + resvars.coords_ReqLayoutp->gauge2_y,
               (u16)resvars.boxes_ReqLayoutp->gauge_len, &button_sprite[resvars.requester_sprites[0x10]], 100, 0, 100,
               1);
    draw_gauge((s16)patient->warmth, x + resvars.coords_ReqLayoutp->gauge3_x, y + resvars.coords_ReqLayoutp->gauge3_y,
               (u16)resvars.boxes_ReqLayoutp->gauge_len, &button_sprite[resvars.requester_sprites[0x11]], 100, 0, 100,
               1);
    if (irequester_slots[1].show_graph)
        draw_health_graph(patient, x, y, color.red);
    else
        draw_diagnosis(patient, x, y);
    if (lbDisplay.mleft_button) {
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes_ReqLayoutp->next.x,
                         y + resvars.boxes_ReqLayoutp->next.y, resvars.boxes_ReqLayoutp->next.w,
                         resvars.boxes_ReqLayoutp->next.h)) {
            othervar.center_on_slot = 1;
            othervar.full_redraw = 1;
        } else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes_ReqLayoutp->diag.x,
                                y + resvars.boxes_ReqLayoutp->diag.y, resvars.boxes_ReqLayoutp->diag.w,
                                resvars.boxes_ReqLayoutp->diag.h)) {
            if (!irequester_slots[1].toggle_delay) {
                irequester_slots[1].show_graph = !irequester_slots[1].show_graph ? 1 : 0;
                irequester_slots[1].toggle_delay = 5;
            }
        }
    }
    if (lbDisplay.mright_button && !othervar.right_button_was_down) {
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes_ReqLayoutp->next.x,
                         y + resvars.boxes_ReqLayoutp->next.y, resvars.boxes_ReqLayoutp->next.w,
                         resvars.boxes_ReqLayoutp->next.h)) {
            u16 old;

            old = irequester_slots[1].value;
            irequester_slots[1].value = find_next_thing(patient, THING_PATIENT, -1, -1);
            if (old != irequester_slots[1].value)
                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
        }
    }
    if (lbDisplay.mright_button)
        othervar.right_button_was_down = 1;
    else
        othervar.right_button_was_down = 0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
    compute_help_text(0, 0, 9);
}

// Closes the query window in slot i (irequester_slots[i]).
// FUNCTION: 0x000736d4
void close_query(u8 i)
{
    close_irequesters();
    irequester_slots[i].kind = REQ_NONE;
}

// Draws patient's recorded health (health_history[1-9]) as a line graph at
// window (x, y) in colour.
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
    step = resvars.boxes_ReqLayoutp->graph_width / 10;
    for (i = 1; i < 9; i++) {
        h1 = patient->health_history[i] * 10;
        h2 = patient->health_history[i + 1] * 10;
        h1 = resvars.boxes_ReqLayoutp->graph_height * h1 / 1000;
        h2 = h2 * resvars.boxes_ReqLayoutp->graph_height / 1000;
        if (h1 < 1)
            h1 = 1;
        else if (h1 > resvars.boxes_ReqLayoutp->graph_height)
            h1 = resvars.boxes_ReqLayoutp->graph_height;
        if (h2 < 1)
            h2 = 1;
        else if (h2 > resvars.boxes_ReqLayoutp->graph_height)
            h2 = resvars.boxes_ReqLayoutp->graph_height;
        y1 = resvars.boxes_ReqLayoutp->graph_bottom - h1;
        y2 = resvars.boxes_ReqLayoutp->graph_bottom - h2;
        x1 = resvars.boxes_ReqLayoutp->graph_x + point++ * step;
        x2 = resvars.boxes_ReqLayoutp->graph_x + point * step;
        LbDrawLine(x + x1, y + y1, x + x2, y + y2, colour);
    }
}

// Draws patient's treatments so far (and "cured") at window (x, y).
// FUNCTION: 0x000738e8
void draw_diagnosis(Thing *patient, u16 x, u16 y)
{
    u8 i;
    u16 dy;
    char buf[100];

    LbDrawBox(x + resvars.boxes_ReqLayoutp->diag.x, y + resvars.boxes_ReqLayoutp->diag.y,
              resvars.boxes_ReqLayoutp->diag.w, resvars.boxes_ReqLayoutp->diag.h, 0xa9);
    LbTextSetClipWindow(x + resvars.boxes_ReqLayoutp->diag.x, y + resvars.boxes_ReqLayoutp->diag.y,
                        resvars.boxes_ReqLayoutp->diag.w, resvars.boxes_ReqLayoutp->diag.h);
    set_font_from_cache(3);
    dy = 0;
    for (i = 0; i < 10; i++) {
        if (!patient->treatments[i])
            break;
        if (resvars.game_mode == SCREEN_MODE_13) {
            sprintf(buf, " %s", illness_text[patient->treatments[i]]);
            conv_string(buf, buf, 0);
        } else {
            sprintf(buf, "%s", illness_text[patient->treatments[i]]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + resvars.boxes_ReqLayoutp->diag.x, dy + y + resvars.boxes_ReqLayoutp->diag.y, buf);
    }
    if (patient->flags & PF_CURED) {
        if (resvars.game_mode == SCREEN_MODE_13) {
            sprintf(buf, " %s", info_bar_text[8]);
            conv_string(buf, buf, 0);
        } else {
            sprintf(buf, "%s", info_bar_text[8]);
        }
        dy += lbFontPtr ? lbFontPtr[34].height : 0;
        LbTextDraw(x + resvars.boxes_ReqLayoutp->diag.x, dy + y + resvars.boxes_ReqLayoutp->diag.y, buf);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, statvar.view_height);
}

// Returns the next thing after cand of type kind belonging to this player for
// which check_if_point_beyond_scroll_limit(tile x, tile y, 0) is nonzero,
// wrapping round: with sub1 >= 0 only subtypes sub1 or sub2, otherwise objects
// only machines (nonzero objects[].debris). If none matches it returns the
// thing count (an index past the last).
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
                            if (objects[cand->subtype].debris) {
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
                                if (objects[cand->subtype].debris)
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

// Draws a gauge of len pixels at (x, y) by repeating spr: value (clamped to
// min-max) as a fraction of range, filled from the left if up is set, else
// the rest.
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

// Help text for a patient window button that needs a diagnosis, into
// othervar.help_msg.
// FUNCTION: 0x00074064
extern "C" void req3_help_if_diagnosed(void)
{
    Thing *patient;

    patient = &thing[irequester_slots[1].value];
    if (patient->diagnosis >= gbv.diagnosis_complete)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// Help text for the patient window's cure button, into othervar.help_msg, when
// it is available.
// FUNCTION: 0x000740e4
extern "C" void req3_help_if_curable(void)
{
    Thing *patient;

    patient = &thing[irequester_slots[1].value];
    if (patient->treatment != 0 && !(patient->flags & PF_GUESSED) && !(patient->flags & PF_FED_UP) &&
        patient->illness != patient->known_illness && patient->diagnosis >= gbv.diagnosis_complete)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// Help text for the patient window's queue button, into othervar.help_msg, when
// the patient is queueing.
// FUNCTION: 0x000741a4
extern "C" void req3_help_if_queueing(void)
{
    Thing *patient;

    patient = &thing[irequester_slots[1].value];
    if (patient->queue_room)
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
}

// Returns the next patient after thing id with any of flags in flags32
// (PF_EMERGENCY, PF_CONTAGIOUS, ...), of this player, for which
// check_if_point_beyond_scroll_limit(tile x, tile y, 0) is nonzero, wrapping
// round. If none matches it returns the thing count.
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
