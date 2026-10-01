#include "hosp.h"

// Creates a floating number on tile pos (y * 128 + x): kind type
// (NUMFX_NUMBER, ...), showing value in text colour `colour`, drawn `height`
// pixels higher (plus 0-15 at random). Does nothing if no thing is free.
// FUNCTION: 0x0009d480
void create_numfx(u8 type, s16 value, u16 pos, u8 colour, u8 height)
{
    Thing *fx;

    fx = allocate_thing(0x32);
    if (fx) {
        fx->type = THING_NUMFX;
        fx->numfx_type = type;
        fx->x.tile = pos % 128;
        fx->y.tile = pos / 128;
        fx->x.frac = 0x80;
        fx->y.frac = 0x80;
        fx->depth = 8;
        insert_node(fx, fx->x, fx->y);
        fx->numfx_amount = value;
        fx->speed = old_normal_rand(0x16, 0x1e);
        fx->anim_speed = height + (old_newrand() & 0xf);
        fx->subtype = colour;
    }
}

// Floating number state: rises a pixel per turn (money two) and is removed
// when its lifetime (speed) runs out.
// FUNCTION: 0x0009d560
void numfx_ages(Thing *fx)
{
    if (fx->numfx_type == NUMFX_MONEY)
        fx->speed--;
    fx->speed--;
    if ((s8)fx->speed <= 0)
        remove_thing(fx);
}
