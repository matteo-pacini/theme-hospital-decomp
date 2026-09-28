#include "hosp.h"

// FUNCTION: 0x0009d480
void create_numfx(u8 type, s16 value, u16 pos, u8 colour, u8 life)
{
    Thing *w;

    w = allocate_thing(0x32);
    if (w) {
        w->renderer = 0x12;
        w->mode = type;
        w->x.tile = pos % 128;
        w->y.tile = pos / 128;
        w->x.frac = 0x80;
        w->y.frac = 0x80;
        w->depth = 8;
        insert_node(w, w->x, w->y);
        w->value = value;
        w->unknown18 = old_normal_rand(0x16, 0x1e);
        w->anim_speed = life + (old_newrand() & 0xf);
        w->subtype = colour;
    }
}

// FUNCTION: 0x0009d560
void numfx_ages(Thing *w)
{
    if (w->mode == 3)
        w->unknown18--;
    w->unknown18--;
    if ((s8)w->unknown18 <= 0)
        remove_thing(w);
}
