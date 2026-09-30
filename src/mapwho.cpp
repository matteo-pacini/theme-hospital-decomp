#include "hosp.h"

struct FloorDef {
    u16 anim;
    u16 animated;
};

struct Offset {
    s32 x;
    s32 y;
};

extern "C" {

extern s16 D_001567b0[8];
extern ToDraw *D_00153fa4[32];
extern FloorDef D_001379f0[];

void fn_0002f47c(Thing *w, s16 x, s16 y);
void fn_0002e804(Thing *w, s16 x, s16 y);

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
    Thing *w;

    node = draw_cells;
    memset(D_00153fa4, 0, sizeof(D_00153fa4));
    D_001567b0[0] = D_001567b0[1] = 999;
    D_001567b0[2] = D_001567b0[3] = 0;
    for (id = cell->first; id > 0; id = w->next) {
        w = &thing[id];
        screen_x = resvars.f0 == SCREEN_MODE_13 ? ((w->x.frac - w->y.frac) >> 3) + 0x20 : ((w->x.frac - w->y.frac) >> 4) + 0x10;
        screen_y = resvars.f0 == SCREEN_MODE_13 ? (w->y.frac + w->x.frac) >> 4 : (w->y.frac + w->x.frac) >> 5;
        bucket = ((w->y.frac + w->x.frac) >> 4) + w->depth;
        if (node < (ToDraw *)&pixBuf) {
            node->next = D_00153fa4[bucket];
            D_00153fa4[bucket] = node;
            node->widget = w;
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
        u8 palette;

        frame = start[D_001379f0[floor].anim].frame;
        palette = start[D_001379f0[floor].anim].flags;
        if (D_001379f0[floor].animated) {
            draw_a_frame(frame, x + (resvars.f0 == SCREEN_MODE_13 ? 32 : 16), y + (resvars.f0 == SCREEN_MODE_13 ? 16 : 8), 0, palette);
        } else {
            Offset *offset;

            offset = &resvars.f38_Offsetp[((u16)(cell - map) * 0x2e4a4321 >> 16) & 7];
            draw_a_frame(frame, x + resvars.f4 / 2 + offset->x, y + resvars.f3 / 2 + offset->y, 0, palette);
        }
    }
    for (i = 0; i < 32; i++) {
        for (node = D_00153fa4[i]; node != 0; node = node->next) {
            int draw_x;
            int draw_y;

            draw_x = node->x + x;
            draw_y = node->y + y;
            switch (node->widget->renderer) {
            case 2:
                mapwho_person(node->widget, draw_x, draw_y, left, top);
                break;
            case 4:
                mapwho_staff(node->widget, draw_x, draw_y, left, top);
                break;
            case 6:
            case 8:
                mapwho_object(node->widget, draw_x, draw_y, left, top);
                break;
            case 10:
                mapwho_reaper(node->widget, draw_x, draw_y);
                break;
            case 12:
                mapwho_rat(node->widget, draw_x, draw_y, left, top);
                break;
            case 14:
                mapwho_bang(node->widget, draw_x, draw_y);
                break;
            case 16:
                mapwho_helicopter(node->widget, draw_x, draw_y);
                break;
            case 18:
                mapwho_numfx(node->widget, draw_x, draw_y);
                break;
            case 20:
            case 22:
                mapwho_mayor(node->widget, draw_x, draw_y, left, top);
                break;
            case 24:
                mapwho_shadow(node->widget, draw_x, draw_y);
            }
        }
    }
}

// FUNCTION: 0x00014b88
void mapwho_person(Thing *w, s32 x, s32 y, s32 left, s32 top)
{
    othervar.fa72++;
    switch (w->kind) {
    case 4:
    case 6:
    case 8:
    case 12:
    case 16:
    case 18:
    case 20:
    case 22:
    case 24:
    case 26:
    case 28:
    case 32:
    case 34:
    case 36:
    case 38:
    case 42:
    case 44:
    case 50:
    case 54:
    case 56:
    case 58:
    case 60:
    case 64:
    case 68:
    case 70:
    case 240:
        if (w->style == 6) {
            fn_0002f47c(w, x, y);
        } else if (w->style == 12 && othervar.f1c == 0 && w->unknown69 == 0) {
            fn_0002e804(w, x, y);
        } else if ((u16)(w - thing) == othervar.f43) {
            draw_hilight_frame(w, x, y);
        } else if ((u16)(w - thing) == othervar.f45) {
            draw_hilight_frame(w, x, y);
        } else {
            draw_special_id_frame(w, x, y);
        }
        break;
    case 46:
        draw_split_frame(w, x, y);
        break;
    case 48:
        if (othervar.f1c == 0) {
            draw_energise_frame(w, x, y);
        } else {
            draw_special_id_frame(w, x, y);
        }
        break;
    case 0xf2:
        break;
    case 52:
    case 72:
        draw_special_id_frame(w, x, y - w->height);
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
        othervar.f41_s16 = w - thing;
    }
}

// FUNCTION: 0x00014fbc
void mapwho_staff(Thing *w, s32 x, s32 y, s32 left, s32 top)
{
    othervar.fa72++;
    switch (w->kind) {
    case 2:
    case 4:
    case 6:
    case 8:
    case 10:
    case 12:
    case 14:
    case 16:
    case 18:
    case 20:
    case 24:
    case 26:
    case 28:
    case 32:
    case 34:
    case 36:
    case 38:
    case 40:
    case 44:
    case 48:
    case 50:
    case 52:
    case 54:
    case 58:
    case 60:
    case 62:
    case 64:
    case 66:
    case 70:
    case 72:
    case 74:
    case 76:
    case 78:
    case 80:
    case 240:
        if ((u16)(w - thing) == othervar.f43) {
            draw_hilight_frame(w, x, y);
        } else {
            draw_special_id_frame(w, x, y);
        }
        break;
    case 42:
    case 46:
    case 56:
        return;
    default:
        break;
    }
    if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
        && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
        othervar.f41_s16 = w - thing;
    }
}

// FUNCTION: 0x00015334
void mapwho_object(Thing *w, const s32 x, const s32 y, const s32 left, const s32 top)
{
    switch (w->kind) {
    case 2:
    case 6:
    case 10:
    case 14:
    case 16:
    case 20:
    case 22:
    case 24:
    case 26:
    case 28:
        if ((u16)(w - thing) == othervar.f43 || (w->group && w->group == othervar.f43)) {
            draw_hilight_frame(w, x, y);
        } else {
            draw_special_id_frame(w, x, y);
        }
        if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
            && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
            othervar.f41_s16 = w - thing;
        }
        if (w->icon) {
            draw_from_special_id_frame(w->icon, x, y, 0, (u8 *)w->label, w->palette);
        }
        break;
    case 4:
        if (w->icon) {
            draw_from_special_id_frame(w->icon, x, y, 0, (u8 *)w->label, w->palette);
        }
        break;
    }
}

// FUNCTION: 0x0001557c
void mapwho_reaper(Thing *w, s32 x, s32 y)
{
    if (w->kind != 0xf2 || !(game_turn & 1)) {
        if (w->kind == 6) {
            return;
        }
        draw_special_id_frame(w, x, y);
    }
}

// FUNCTION: 0x000155d4
void mapwho_rat(Thing *w, const s32 x, const s32 y, const s32 left0, const s32 top0)
{
    int mouse_x;
    int mouse_y;
    int left;
    int right;
    int top;
    int bottom;

    mouse_x = lbDisplay.mmouse_x;
    mouse_y = lbDisplay.mmouse_y;
    draw_special_id_frame(w, x, y);
    left = left0 + D_001567b0[4] - othervar.f9e5;
    right = left + D_001567b0[6] + othervar.f9e5 * 2;
    top = top0 + D_001567b0[5] - othervar.f9e5;
    bottom = top + D_001567b0[7] + othervar.f9e5 * 2;
    if (mouse_x > left && mouse_x < right && mouse_y > top && mouse_y < bottom) {
        if (left + othervar.f9e5 - othervar.f9e7 < mouse_x && right - othervar.f9e5 + othervar.f9e7 > mouse_x
            && top + othervar.f9e5 - othervar.f9e7 < mouse_y && bottom - othervar.f9e5 + othervar.f9e7 > mouse_y) {
            othervar.f41_s16 = w - thing;
            othervar.f6f_s16 = w - thing;
        } else {
            othervar.f71 = 1;
        }
    }
}

// FUNCTION: 0x00015760
void mapwho_bang(Thing *w, s32 x, s32 y)
{
    draw_a_frame(w->sprite, x, y - (w->offset >> 8), 0, w->palette);
}

// FUNCTION: 0x000157b8
void mapwho_helicopter(Thing *w, s32 x, s32 y)
{
    draw_special_id_frame(w, x, y - w->offset);
}

// FUNCTION: 0x000157f0
void mapwho_numfx(Thing *w, s32 x, s32 y)
{
    char buf[100];

    switch (w->mode) {
    case 3:
        draw_money(x, w->unknown18 + y - w->anim_speed, w->value);
        break;
    default:
        if (w->mode == 0) {
            sprintf(buf, "%d", w->value);
        } else if (w->mode == 1) {
            fn_000bbff0(buf, w->text);
        } else if (w->mode == 2) {
            sprintf(buf, w->text, w->value);
        }
        draw_text_on_map(buf, w->subtype, x, y + w->unknown18 - w->anim_speed);
        break;
    }
}

// FUNCTION: 0x000158e8
void mapwho_mayor(Thing *w, s32 x, s32 y, s32 left, s32 top)
{
    draw_special_id_frame(w, x, y);
    if (D_001567b0[4] + left < lbDisplay.mmouse_x && D_001567b0[4] + D_001567b0[6] + left > lbDisplay.mmouse_x
        && D_001567b0[5] + top < lbDisplay.mmouse_y && D_001567b0[5] + D_001567b0[7] + top > lbDisplay.mmouse_y) {
        othervar.f41_s16 = w - thing;
    }
}

// FUNCTION: 0x0001599c
void mapwho_shadow(Thing *w, s32 x, s32 y)
{
    Thing *other;

    other = &thing[w->link];
    draw_a_frame_scaled(w->sprite, x, y, 0, w->palette, 0x640000 / (other->offset + 100));
}
