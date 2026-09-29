#include "hosp.h"

extern "C" {

void fn_00015b68(Thing *w);
void fn_000168a0(Thing *w);

}

// FUNCTION: 0x00015a20
void insert_node(Thing *w, MapPos x, MapPos y)
{
    Map *cell;
    u16 first;
    u16 id;
    Thing *other;

    cell = &map[(y.tile << 7) + x.tile];
    id = w - thing;
    first = cell->first;
    if (first) {
        w->next = first;
        if (w->next > 0) {
            other = &thing[first];
            other->prev = id;
        }
    }
    cell->first = id;
}

// FUNCTION: 0x00015ac4
u16 move_node(Thing *w, MapPos x, MapPos y)
{
    if (x.tile >= 0x80 || y.tile >= 0x80) {
        return 0;
    }
    if (x.tile != w->x.tile || y.tile != w->y.tile) {
        fn_00015b68(w);
        insert_node(w, x, y);
        w->x = x;
        w->y = y;
        return 1;
    }
    w->x = x;
    w->y = y;
    return 0;
}

// FUNCTION: 0x00015b68
void fn_00015b68(Thing *w)
{
    Map *cell;
    Thing *other;

    if (w->prev == 0) {
        cell = &map[((s16)w->y.tile << 7) + (s16)w->x.tile];
        cell->first = w->next;
        if (cell->first > 0) {
            other = &thing[w->next];
            other->prev = 0;
        }
    } else {
        if ((thing[w->prev].next = w->next) > 0) {
            thing[w->next].prev = w->prev;
        }
    }
    w->next = 0;
    w->prev = 0;
}

// FUNCTION: 0x0002bf94
void remove_node(Thing *w)
{
    if (w->room) {
        fn_000168a0(w);
    }
    fn_00015b68(w);
}
