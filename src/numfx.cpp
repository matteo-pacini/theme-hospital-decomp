#include "hosp.h"

// FUNCTION: 0x0009d480
void create_numfx(u8 type, s16 value, u16 pos, u8 colour, u8 life)
{
    Thing *fx;

    fx = allocate_thing(0x32);
    if (fx) {
        fx->type = 0x12;
        fx->numfx_type = type;
        fx->x.tile = pos % 128;
        fx->y.tile = pos / 128;
        fx->x.frac = 0x80;
        fx->y.frac = 0x80;
        fx->depth = 8;
        insert_node(fx, fx->x, fx->y);
        fx->numfx_amount = value;
        fx->speed = old_normal_rand(0x16, 0x1e);
        fx->anim_speed = life + (old_newrand() & 0xf);
        fx->subtype = colour;
    }
}

// FUNCTION: 0x0009d560
void numfx_ages(Thing *fx)
{
    if (fx->numfx_type == 3)
        fx->speed--;
    fx->speed--;
    if ((s8)fx->speed <= 0)
        remove_thing(fx);
}
