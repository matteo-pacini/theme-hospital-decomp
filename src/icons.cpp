#include "hosp.h"

void reset_icon_value(Thing *const w, const s8 v);

// FUNCTION: 0x000b0300
void set_icon_value(Thing *const w, const s8 v)
{
    if (w->renderer != 2 || w->unknown130 > 0) {
        if (icon_table[v].unknown0 <= icon_table[fn_000cb2db(w->unknown24)].unknown0) {
            w->unknown27 = start[icon_table[v].unknown1].frame;
            if (icon_table[v].f5 < 0) {
                w->unknown24 = -v;
                w->unknown25w = icon_table[v].f3;
            } else if (icon_table[v].f5 == 0) {
                w->unknown24 = v;
                w->unknown25w = icon_table[v].f3;
            } else if (icon_table[v].f5 > 0) {
                w->unknown24 = v;
                w->unknown25w = 0xffff;
            }
        }
    }
}

// FUNCTION: 0x000b0428
void reset_icon_value(Thing *const w, const s8 v)
{
    if (w->unknown24 == fn_000cb2db(v)) {
        w->unknown24 = 0;
        w->unknown25w = 0;
        w->unknown27 = 0;
    }
}

// FUNCTION: 0x000b0474
void peep_icon(Thing *const w)
{
    if (w->renderer != 2 || w->unknown130 > 0) {
        if (w->unknown24 < 0)
            play_sound(10, 1, -1, -1);
        w->unknown24 = fn_000cb2db(w->unknown24);
        if (icon_table[w->unknown24].unknown1 > 0 && icon_table[w->unknown24].f5 == -1)
            w->unknown25w = icon_table[w->unknown24].f3;
    }
}

// FUNCTION: 0x000b0540
void update_icon_counter(Thing *const w)
{
    if (w->unknown24 > 0) {
        if ((s16)w->unknown25w > 0) {
            w->unknown25w--;
            w->unknown27 = frame[w->unknown27].next;
        }
        if ((s16)w->unknown25w == 0)
            reset_icon_value(w, w->unknown24);
        else if ((s16)w->unknown25w < 0)
            w->unknown27 = frame[w->unknown27].next;
    }
}

extern "C" {

// FUNCTION: 0x000b05f0
void fn_000b05f0(Thing *w)
{
    w->unknown24 = 0;
    w->unknown25w = 0;
    w->unknown27 = 0;
}

}
