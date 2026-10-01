#include "hosp.h"

extern "C" {

void remove_node(Thing *w);

}

// Links node at the head of the thing list of map cell (x, y) (Map.first).
// Neither sets node's position nor clears its cell_prev; the node must not be
// in a list.
// FUNCTION: 0x00015a20
void insert_node(Thing *node, MapPos x, MapPos y)
{
    Map *cell;
    u16 first;
    u16 id;
    Thing *other;

    cell = &map[(y.tile << 7) + x.tile];
    id = node - thing;
    first = cell->first;
    if (first) {
        node->cell_next = first;
        if (node->cell_next > 0) {
            other = &thing[first];
            other->cell_prev = id;
        }
    }
    cell->first = id;
}

// Moves node to (x, y), relinking it into the new cell's list when the tile
// changes. Returns 1 if the tile changed; 0 if not, or if (x, y) is off the
// map, in which case nothing moves.
// FUNCTION: 0x00015ac4
u16 move_node(Thing *node, MapPos x, MapPos y)
{
    if (x.tile >= 0x80 || y.tile >= 0x80) {
        return 0;
    }
    if (x.tile != node->x.tile || y.tile != node->y.tile) {
        remove_node(node);
        insert_node(node, x, y);
        node->x = x;
        node->y = y;
        return 1;
    }
    node->x = x;
    node->y = y;
    return 0;
}

// Unlinks node from its map cell's thing list and clears its links.
// FUNCTION: 0x00015b68
void remove_node(Thing *node)
{
    Map *cell;
    Thing *other;

    if (node->cell_prev == 0) {
        cell = &map[((s16)node->y.tile << 7) + (s16)node->x.tile];
        cell->first = node->cell_next;
        if (cell->first > 0) {
            other = &thing[node->cell_next];
            other->cell_prev = 0;
        }
    } else {
        if ((thing[node->cell_prev].cell_next = node->cell_next) > 0) {
            thing[node->cell_next].cell_prev = node->cell_prev;
        }
    }
    node->cell_next = 0;
    node->cell_prev = 0;
}
