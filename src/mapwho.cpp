#include "hosp.h"

struct Offset {
    s32 x;
    s32 y;
};

extern "C" {

extern s16 D_001567b0[8];
extern ToDraw *buckets[32];
extern FloorDef litter_starts[];

void draw_glow_frame(Thing *w, s16 x, s16 y);
void draw_wibble_frame(Thing *w, s16 x, s16 y);

}

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
    D_001567b0[0] = D_001567b0[1] = 999;
    D_001567b0[2] = D_001567b0[3] = 0;
    for (id = cell->first; id > 0; id = occupant->cell_next) {
        occupant = &thing[id];
        screen_x = resvars.f0 == SCREEN_MODE_13 ? ((occupant->x.frac - occupant->y.frac) >> 3) + 0x20 : ((occupant->x.frac - occupant->y.frac) >> 4) + 0x10;
        screen_y = resvars.f0 == SCREEN_MODE_13 ? (occupant->y.frac + occupant->x.frac) >> 4 : (occupant->y.frac + occupant->x.frac) >> 5;
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
            LbSpriteDraw(x, resvars.f3 + y - blocks[cell->overlay].info->height, blocks[cell->overlay].info);
        }
        draw_a_frame(sprite, resvars.f4 / 2 + x, resvars.f3 / 2 + y, 0, 0);
    }
    if (id < 0) {
        draw_a_frame(start[blueprint[-id].anim].frame, (resvars.f4 >> 1) + x, (resvars.f3 >> 1) + y, 0,
            start[blueprint[-id].anim].flags);
    }
    floor = cell->floor & 0xf;
    if (floor) {
        u16 frame;
        u8 flip;

        frame = start[litter_starts[floor].anim].frame;
        flip = start[litter_starts[floor].anim].flags;
        if (litter_starts[floor].animated) {
            draw_a_frame(frame, x + (resvars.f0 == SCREEN_MODE_13 ? 32 : 16), y + (resvars.f0 == SCREEN_MODE_13 ? 16 : 8), 0, flip);
        } else {
            Offset *offset;

            offset = &resvars.f38_Offsetp[((u16)(cell - map) * 0x2e4a4321 >> 16) & 7];
            draw_a_frame(frame, x + resvars.f4 / 2 + offset->x, y + resvars.f3 / 2 + offset->y, 0, flip);
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

// FUNCTION: 0x00014b88
void mapwho_person(Thing *person, s32 x, s32 y, s32 left, s32 top)
{
    othervar.fa72++;
    switch (person->state) {
    case PS_WAITS_FOR_BUSY:
    case PS_GOTO_DESTINATION:
    case PS_RETURN_TO_QUEUE_AREA:
    case PS_GO_HOME:
    case PS_COMPLEX_RETRY:
    case PS_ARRIVES_AT_DOOR:
    case PS_IN_QUEUE:
    case PS_GOTO_OBJECT:
    case PS_ENTERTAINS_THEMSELF:
    case 26:
    case 28:
    case 32:
    case 34:
    case PS_GOTO_QUEUE:
    case PS_GO_FOR_TREATMENT:
    case 42:
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
        } else if (person->illness == ILL_JELLYITIS && othervar.f1c == 0 && person->object == 0) {
            draw_wibble_frame(person, x, y);
        } else if ((u16)(person - thing) == othervar.f43) {
            draw_hilight_frame(person, x, y);
        } else if ((u16)(person - thing) == othervar.f45) {
            draw_hilight_frame(person, x, y);
        } else {
            draw_special_id_frame(person, x, y);
        }
        break;
    case PS_FILLS:
        draw_split_frame(person, x, y);
        break;
    case PS_ENERGISES:
        if (othervar.f1c == 0) {
            draw_energise_frame(person, x, y);
        } else {
            draw_special_id_frame(person, x, y);
        }
        break;
    case 0xf2:
        break;
    case 52:
    case 72:
        draw_special_id_frame(person, x, y - person->fly_height);
        break;
    case 14:
    case 40:
    case 66:
        return;
    default:
        break;
    }
    if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
        && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
        othervar.f41_s16 = person - thing;
    }
}

// FUNCTION: 0x00014fbc
void mapwho_staff(Thing *staff, s32 x, s32 y, s32 left, s32 top)
{
    othervar.fa72++;
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
    case 24:
    case 26:
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
    case SS_PROCESS_COMPLEX:
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
        if ((u16)(staff - thing) == othervar.f43) {
            draw_hilight_frame(staff, x, y);
        } else {
            draw_special_id_frame(staff, x, y);
        }
        break;
    case 42:
    case SS_APPLICANT:
    case 56:
        return;
    default:
        break;
    }
    if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
        && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
        othervar.f41_s16 = staff - thing;
    }
}

// FUNCTION: 0x00015334
void mapwho_object(Thing *obj, const s32 x, const s32 y, const s32 left, const s32 top)
{
    switch (obj->state) {
    case OS_STATIC:
    case 6:
    case OS_PLANT_AGES:
    case 14:
    case OS_DOUBLE_DOOR_SWINGS:
    case OS_SLIDING_DOOR_CLOSED:
    case OS_SLIDING_DOOR_OPENS:
    case OS_SLIDING_DOOR_OPEN:
    case OS_SLIDING_DOOR_CLOSES:
    case OS_BOMB_TICKS:
        if ((u16)(obj - thing) == othervar.f43 || (obj->pair && obj->pair == othervar.f43)) {
            draw_hilight_frame(obj, x, y);
        } else {
            draw_special_id_frame(obj, x, y);
        }
        if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
            && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
            othervar.f41_s16 = obj - thing;
        }
        if (obj->overlay_frame) {
            draw_from_special_id_frame(obj->overlay_frame, x, y, 0, (u8 *)obj->data, obj->flip);
        }
        break;
    case 4:
        if (obj->overlay_frame) {
            draw_from_special_id_frame(obj->overlay_frame, x, y, 0, (u8 *)obj->data, obj->flip);
        }
        break;
    }
}

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
    left = left0 + D_001567b0[4] - othervar.f9e5;
    right = left + D_001567b0[6] + othervar.f9e5 * 2;
    top = top0 + D_001567b0[5] - othervar.f9e5;
    bottom = top + D_001567b0[7] + othervar.f9e5 * 2;
    if (mouse_x > left && mouse_x < right && mouse_y > top && mouse_y < bottom) {
        if (left + othervar.f9e5 - othervar.f9e7 < mouse_x && right - othervar.f9e5 + othervar.f9e7 > mouse_x
            && top + othervar.f9e5 - othervar.f9e7 < mouse_y && bottom - othervar.f9e5 + othervar.f9e7 > mouse_y) {
            othervar.f41_s16 = rat - thing;
            othervar.f6f_s16 = rat - thing;
        } else {
            othervar.f71 = 1;
        }
    }
}

// FUNCTION: 0x00015760
void mapwho_bang(Thing *bang, s32 x, s32 y)
{
    draw_a_frame(bang->sprite, x, y - (bang->altitude >> 8), 0, bang->flip);
}

// FUNCTION: 0x000157b8
void mapwho_helicopter(Thing *helicopter, s32 x, s32 y)
{
    draw_special_id_frame(helicopter, x, y - helicopter->altitude);
}

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
            fn_000bbff0(buf, fx->numfx_text);
        } else if (fx->numfx_type == NUMFX_FORMATTED) {
            sprintf(buf, fx->numfx_text, fx->numfx_amount);
        }
        draw_text_on_map(buf, fx->subtype, x, y + fx->speed - fx->anim_speed);
        break;
    }
}

// FUNCTION: 0x000158e8
void mapwho_mayor(Thing *mayor, s32 x, s32 y, s32 left, s32 top)
{
    draw_special_id_frame(mayor, x, y);
    if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
        && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
        othervar.f41_s16 = mayor - thing;
    }
}

// FUNCTION: 0x0001599c
void mapwho_shadow(Thing *shadow, s32 x, s32 y)
{
    Thing *caster;

    caster = &thing[shadow->caster];
    draw_a_frame_scaled(shadow->sprite, x, y, 0, shadow->flip, 0x640000 / (caster->altitude + 100));
}
