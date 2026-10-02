#include "hosp.h"

struct Offset {
    s32 x;
    s32 y;
};

extern "C" {

extern s16 frame_bounds[8];
extern ToDraw *buckets[32];
extern FloorDef litter_starts[];

void draw_glow_frame(Thing *w, s16 x, s16 y);
void draw_wibble_frame(Thing *w, s16 x, s16 y);
}

// Draws everything on one map cell: its overlay and floor litter, a
// blueprint marker, then its things in depth order through the mapwho_*
// routine for each type. (x, y) is the cell's screen position; (left, top)
// is the view's origin, used to test which thing is under the mouse.
// FUNCTION: 0x00014510
void do_mapwho(const Map *const cell, const s32 x, const s32 y, const s32 left, const s32 top)
{
    u8 floor;
    u16 screen_y;
    s16 id;
    s16 bucket;
    u16 screen_x;
    ToDraw *node;
    int i;
    Thing *occupant;

    node = draw_cells;
    memset(buckets, 0, sizeof(buckets));
    frame_bounds[0] = frame_bounds[1] = 999;
    frame_bounds[2] = frame_bounds[3] = 0;
    for (id = cell->first; id > 0; id = occupant->cell_next) {
        occupant = &thing[id];
        screen_x = resvars.screen_mode == SCREEN_MODE_13 ? ((occupant->x.frac - occupant->y.frac) >> 3) + 0x20
                                                         : ((occupant->x.frac - occupant->y.frac) >> 4) + 0x10;
        screen_y = resvars.screen_mode == SCREEN_MODE_13 ? (occupant->y.frac + occupant->x.frac) >> 4
                                                         : (occupant->y.frac + occupant->x.frac) >> 5;
        bucket = ((occupant->y.frac + occupant->x.frac) >> 4) + occupant->depth;
        if (node < (ToDraw *)&pixBuf) {
            node->next = buckets[bucket];
            buckets[bucket] = node;
            node->widget = occupant;
            node->x = screen_x;
            node->y = screen_y;
            node++;
        }
    }
    if (cell->flags & 0xc0) {
        u32 sprite;

        sprite = start[((cell->flags & 0xc0) >> 6) + 625].frame;
        if (cell->flags & 0x10) {
            LbSpriteDraw(x, resvars.tile_height + y - blocks[cell->overlay].info->height, blocks[cell->overlay].info);
        }
        draw_a_frame(sprite, resvars.tile_width / 2 + x, resvars.tile_height / 2 + y, 0, 0);
    }
    if (id < 0) {
        draw_a_frame(start[blueprint[-id].anim].frame, (resvars.tile_width >> 1) + x, (resvars.tile_height >> 1) + y, 0,
                     start[blueprint[-id].anim].flags);
    }
    floor = cell->floor & 0xf;
    if (floor) {
        u16 frame;
        u8 flip;

        frame = start[litter_starts[floor].anim].frame;
        flip = start[litter_starts[floor].anim].flags;
        if (litter_starts[floor].animated) {
            draw_a_frame(frame, x + (resvars.screen_mode == SCREEN_MODE_13 ? 32 : 16),
                         y + (resvars.screen_mode == SCREEN_MODE_13 ? 16 : 8), 0, flip);
        } else {
            Offset *offset;

            offset = &resvars.litter_pos_Offsetp[((u16)(cell - map) * 0x2e4a4321 >> 16) & 7];
            draw_a_frame(frame, x + resvars.tile_width / 2 + offset->x, y + resvars.tile_height / 2 + offset->y, 0,
                         flip);
        }
    }
    for (i = 0; i < 32; i++) {
        for (node = buckets[i]; node != 0; node = node->next) {
            int draw_x;
            int draw_y;

            draw_x = node->x + x;
            draw_y = node->y + y;
            switch (node->widget->type) {
            case THING_PATIENT:
                mapwho_person(node->widget, draw_x, draw_y, left, top);
                break;
            case THING_STAFF:
                mapwho_staff(node->widget, draw_x, draw_y, left, top);
                break;
            case THING_OBJECT:
            case THING_DOOR:
                mapwho_object(node->widget, draw_x, draw_y, left, top);
                break;
            case THING_REAPER:
                mapwho_reaper(node->widget, draw_x, draw_y);
                break;
            case THING_RAT:
                mapwho_rat(node->widget, draw_x, draw_y, left, top);
                break;
            case THING_DEBRIS:
                mapwho_bang(node->widget, draw_x, draw_y);
                break;
            case THING_HELICOPTER:
                mapwho_helicopter(node->widget, draw_x, draw_y);
                break;
            case THING_NUMFX:
                mapwho_numfx(node->widget, draw_x, draw_y);
                break;
            case THING_MAYOR:
            case THING_INSPECTOR:
                mapwho_mayor(node->widget, draw_x, draw_y, left, top);
                break;
            case THING_SHADOW:
                mapwho_shadow(node->widget, draw_x, draw_y);
            }
        }
    }
}

// Draws a patient (glowing, wobbling, split or highlighted as its illness and
// state require) and, if the mouse is over the frame drawn, records it in
// othervar.pointed_thing_s as the thing under the mouse.
// FUNCTION: 0x00014b88
void mapwho_person(Thing *person, s32 x, s32 y, s32 left, s32 top)
{
    othervar.visible_people++;
    switch (person->state) {
    case PS_WAITS_FOR_BUSY:
    case PS_GOTO_DESTINATION:
    case PS_RETURN_TO_QUEUE_AREA:
    case PS_GO_HOME:
    case PS_PLAY_COMPLEX_ANIM:
    case PS_ARRIVES_AT_DOOR:
    case PS_IN_QUEUE:
    case PS_GOTO_OBJECT:
    case PS_ENTERTAINS_THEMSELF:
    case PS_WAIT_FOR_COMPLEX_SLAVE:
    case 28:
    case PS_SLAVE_READY:
    case PS_TWO_PEEPS_INTERACT:
    case PS_GOTO_QUEUE:
    case PS_GO_FOR_TREATMENT:
    case PS_COMPLEX_RETRY:
    case PS_IN_BED:
    case PS_DEAD:
    case PS_IN_BOGS:
    case PS_LEAVE_TOILETS:
    case PS_SHOWANIM:
    case PS_STUCK:
    case PS_BP_GOTO:
    case PS_WAITS_FOR_DECISION:
    case PS_ARRIVED_FROM_CALL:
    case PS_NAV_DEBUG:
        if (person->illness == ILL_SERIOUS_RADIATION) {
            draw_glow_frame(person, x, y);
        } else if (person->illness == ILL_JELLYITIS && othervar.drawing_pixbuf == 0 && person->object == 0) {
            draw_wibble_frame(person, x, y);
        } else if ((u16)(person - thing) == othervar.pickup_thing) {
            draw_hilight_frame(person, x, y);
        } else if ((u16)(person - thing) == othervar.queue_hilight_thing) {
            draw_hilight_frame(person, x, y);
        } else {
            draw_special_id_frame(person, x, y);
        }
        break;
    case PS_FILLS:
        draw_split_frame(person, x, y);
        break;
    case PS_ENERGISES:
        if (othervar.drawing_pixbuf == 0) {
            draw_energise_frame(person, x, y);
        } else {
            draw_special_id_frame(person, x, y);
        }
        break;
    case 0xf2:
        break;
    case PS_FLYS:
    case 72:
        draw_special_id_frame(person, x, y - person->fly_height);
        break;
    case PS_GONE:
    case PS_MERGED:
    case PS_AT_HELL_GATE:
        return;
    default:
        break;
    }
    if (frame_bounds[4] + left < lbDisplay.mmouse_x && frame_bounds[4] + frame_bounds[6] + left > lbDisplay.mmouse_x &&
        frame_bounds[5] + top < lbDisplay.mmouse_y && frame_bounds[5] + frame_bounds[7] + top > lbDisplay.mmouse_y) {
        othervar.pointed_thing_s = person - thing;
    }
}

// Draws a staff member (highlighted if othervar.pickup_thing) and records it in
// othervar.pointed_thing_s if under the mouse. Applicants and some hidden states are
// not drawn.
// FUNCTION: 0x00014fbc
void mapwho_staff(Thing *staff, s32 x, s32 y, s32 left, s32 top)
{
    othervar.visible_people++;
    switch (staff->state) {
    case 2:
    case 4:
    case SS_DOCTOR_IN_OFFICE:
    case SS_DOCTOR_WORKS_AT_DESK:
    case SS_RECEPTIONIST_WORKS:
    case SS_RECEPTIONIST_FINDS_RECEPTION:
    case SS_PLAY_COMPLEX_ANIM:
    case SS_SURGEON_WAITS:
    case SS_GOTO_OBJECT:
    case SS_DOCTOR_WAITS_FOR_PATIENT:
    case SS_ASSISTANT_READY:
    case SS_TWO_PEEPS_INTERACT:
    case SS_DOCTOR_EXAMINES:
    case SS_RECEPT_WAITS_FOR_PATIENT:
    case SS_RECEPT_ADMITS_PATIENT:
    case SS_DOCTOR_IN_MACHINE_ROOM:
    case SS_WAIT_FOR_COMPLEX_SLAVE:
    case SS_IN_CORRIDOR:
    case SS_COMPLEX_RETRY:
    case SS_IN_STAFF_ROOM:
    case SS_NURSE_IN_WARD:
    case SS_WANDERS:
    case SS_EVACUATE:
    case 58:
    case SS_HANDYMAN_WORKS:
    case SS_DOCTOR_RESEARCHES:
    case SS_NURSE_VACCINATES:
    case SS_DOCTOR_IN_THEATRE:
    case SS_DOCTOR_IS_TRAINED:
    case SS_CONSULTANT_TRAINS:
    case SS_RECEPTIONIST_SETUP:
    case SS_BP_GOTO:
    case SS_STUCK:
    case SS_PLAY_ANISEQ:
    case SS_NAV_DEBUG:
        if ((u16)(staff - thing) == othervar.pickup_thing) {
            draw_hilight_frame(staff, x, y);
        } else {
            draw_special_id_frame(staff, x, y);
        }
        break;
    case 42:
    case SS_APPLICANT:
    case SS_BACK_TO_APPLICANTS:
        return;
    default:
        break;
    }
    if (frame_bounds[4] + left < lbDisplay.mmouse_x && frame_bounds[4] + frame_bounds[6] + left > lbDisplay.mmouse_x &&
        frame_bounds[5] + top < lbDisplay.mmouse_y && frame_bounds[5] + frame_bounds[7] + top > lbDisplay.mmouse_y) {
        othervar.pointed_thing_s = staff - thing;
    }
}

// Draws an object or door with its overlay animation (highlighted if it or its
// pair is othervar.pickup_thing) and records it in othervar.pointed_thing_s if under the mouse.
// FUNCTION: 0x00015334
void mapwho_object(Thing *obj, const s32 x, const s32 y, const s32 left, const s32 top)
{
    switch (obj->state) {
    case OS_STATIC:
    case OS_HELL:
    case OS_PLANT_AGES:
    case OS_BEING_WATERED:
    case OS_DOUBLE_DOOR_SWINGS:
    case OS_SLIDING_DOOR_CLOSED:
    case OS_SLIDING_DOOR_OPENS:
    case OS_SLIDING_DOOR_OPEN:
    case OS_SLIDING_DOOR_CLOSES:
    case OS_BOMB_TICKS:
        if ((u16)(obj - thing) == othervar.pickup_thing || (obj->pair && obj->pair == othervar.pickup_thing)) {
            draw_hilight_frame(obj, x, y);
        } else {
            draw_special_id_frame(obj, x, y);
        }
        if (frame_bounds[4] + left < lbDisplay.mmouse_x &&
            frame_bounds[4] + frame_bounds[6] + left > lbDisplay.mmouse_x &&
            frame_bounds[5] + top < lbDisplay.mmouse_y &&
            frame_bounds[5] + frame_bounds[7] + top > lbDisplay.mmouse_y) {
            othervar.pointed_thing_s = obj - thing;
        }
        if (obj->overlay_frame) {
            draw_from_special_id_frame(obj->overlay_frame, x, y, 0, (u8 *)obj->data, obj->flip);
        }
        break;
    case OS_IN_USE:
        if (obj->overlay_frame) {
            draw_from_special_id_frame(obj->overlay_frame, x, y, 0, (u8 *)obj->data, obj->flip);
        }
        break;
    }
}

// Draws the grim reaper unless it is being removed (blinking in state 0xf2).
// FUNCTION: 0x0001557c
void mapwho_reaper(Thing *reaper, s32 x, s32 y)
{
    if (reaper->state != 0xf2 || !(game_turn & 1)) {
        if (reaper->state == REAPER_REMOVE) {
            return;
        }
        draw_special_id_frame(reaper, x, y);
    }
}

// Draws a rat. A mouse over the rat's frame (within othervar.rat_hit_margin of it)
// records it in othervar.pointed_thing_s and f6f_s16; a near miss (within
// othervar.rat_near_margin) sets othervar.near_rat.
// FUNCTION: 0x000155d4
void mapwho_rat(Thing *rat, const s32 x, const s32 y, const s32 left0, const s32 top0)
{
    int mouse_x;
    int mouse_y;
    int left;
    int right;
    int top;
    int bottom;

    mouse_x = lbDisplay.mmouse_x;
    mouse_y = lbDisplay.mmouse_y;
    draw_special_id_frame(rat, x, y);
    left = left0 + frame_bounds[4] - othervar.rat_near_margin;
    right = left + frame_bounds[6] + othervar.rat_near_margin * 2;
    top = top0 + frame_bounds[5] - othervar.rat_near_margin;
    bottom = top + frame_bounds[7] + othervar.rat_near_margin * 2;
    if (mouse_x > left && mouse_x < right && mouse_y > top && mouse_y < bottom) {
        if (left + othervar.rat_near_margin - othervar.rat_hit_margin < mouse_x &&
            right - othervar.rat_near_margin + othervar.rat_hit_margin > mouse_x &&
            top + othervar.rat_near_margin - othervar.rat_hit_margin < mouse_y &&
            bottom - othervar.rat_near_margin + othervar.rat_hit_margin > mouse_y) {
            othervar.pointed_thing_s = rat - thing;
            othervar.pointed_rat_s = rat - thing;
        } else {
            othervar.near_rat = 1;
        }
    }
}

// Draws a piece of explosion debris raised by its altitude (1/256 pixel).
// FUNCTION: 0x00015760
void mapwho_bang(Thing *bang, s32 x, s32 y)
{
    draw_a_frame(bang->sprite, x, y - (bang->altitude >> 8), 0, bang->flip);
}

// Draws a helicopter raised by its altitude (pixels).
// FUNCTION: 0x000157b8
void mapwho_helicopter(Thing *helicopter, s32 x, s32 y)
{
    draw_special_id_frame(helicopter, x, y - helicopter->altitude);
}

// Draws a floating number or text (Thing.numfx_type), risen by its speed.
// FUNCTION: 0x000157f0
void mapwho_numfx(Thing *fx, s32 x, s32 y)
{
    char buf[100];

    switch (fx->numfx_type) {
    case NUMFX_MONEY:
        draw_money(x, fx->speed + y - fx->anim_speed, fx->numfx_amount);
        break;
    default:
        if (fx->numfx_type == NUMFX_NUMBER) {
            sprintf(buf, "%d", fx->numfx_amount);
        } else if (fx->numfx_type == NUMFX_TEXT) {
            lib_strcpy(buf, fx->numfx_text);
        } else if (fx->numfx_type == NUMFX_FORMATTED) {
            sprintf(buf, fx->numfx_text, fx->numfx_amount);
        }
        draw_text_on_map(buf, fx->subtype, x, y + fx->speed - fx->anim_speed);
        break;
    }
}

// Draws the mayor or inspector and records them in othervar.pointed_thing_s if under
// the mouse.
// FUNCTION: 0x000158e8
void mapwho_mayor(Thing *mayor, s32 x, s32 y, s32 left, s32 top)
{
    draw_special_id_frame(mayor, x, y);
    if (frame_bounds[4] + left < lbDisplay.mmouse_x && frame_bounds[4] + frame_bounds[6] + left > lbDisplay.mmouse_x &&
        frame_bounds[5] + top < lbDisplay.mmouse_y && frame_bounds[5] + frame_bounds[7] + top > lbDisplay.mmouse_y) {
        othervar.pointed_thing_s = mayor - thing;
    }
}

// Draws a helicopter's shadow, scaled down as its caster's altitude rises.
// FUNCTION: 0x0001599c
void mapwho_shadow(Thing *shadow, s32 x, s32 y)
{
    Thing *caster;

    caster = &thing[shadow->caster];
    draw_a_frame_scaled(shadow->sprite, x, y, 0, shadow->flip, 0x640000 / (caster->altitude + 100));
}
