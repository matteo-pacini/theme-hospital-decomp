#include "hosp.h"

#define THING_PATH(w) ((orthonav_path *)&(w)->path[0])

extern Thing *nav_person;
extern s16 opposite_angle[];

s32 orthonav_whatnext(orthonav_path *path, s8 x, s8 y, s32 dir);

extern "C" {

void nav_init(Thing *person);

}

// Returns 1 if a step from tile (x, y) in direction dir (0 -y, 1 +x, 2 +y,
// 3 -x) is blocked: by the map's edge, a wall on the edge between the tiles,
// or either tile being impassable (Map.flags bit 0 without bit 1). 0 if free.
// FUNCTION: 0x00043ff0
s32 wall_in_way(u32 x, u32 y, u32 dir)
{
    Map *cell;

    if (dir == 0 && y == 0) {
        return 1;
    }
    if (dir == 1 && x >= 0x7f) {
        return 1;
    }
    if (dir == 2 && y >= 0x7f) {
        return 1;
    }
    if (dir == 3 && x == 0) {
        return 1;
    }
    cell = &map[(s16)x + ((s16)y << 7)];
    if ((cell->flags & 1) && !(cell->flags & 2)) {
        return 1;
    }
    switch (dir) {
    case 0:
        if (cell->wall_n) {
            return 1;
        }
        break;
    case 1:
        cell = &map[(s16)(x + 1) + ((s16)y << 7)];
        if (cell->wall_w) {
            return 1;
        }
        break;
    case 2:
        cell = &map[(s16)x + ((s16)(y + 1) << 7)];
        if (cell->wall_n) {
            return 1;
        }
        break;
    case 3:
        if (cell->wall_w) {
            return 1;
        }
        break;
    }
    switch (dir) {
    case 0:
        y--;
        break;
    case 1:
        x++;
        break;
    case 2:
        y++;
        break;
    case 3:
        x--;
        break;
    }
    cell = &map[(s16)x + ((s16)y << 7)];
    if ((cell->flags & 1) && !(cell->flags & 2)) {
        return 1;
    }
    return 0;
}

// Returns person's next step towards (dest_x, dest_y) as orthonav_whatnext
// does, replanning the path first if there is none or the destination
// changed. Sets nav_person. The second parameter is unused.
// FUNCTION: 0x00044210
s16 person_head_for(Thing *person, u8 a)
{
    if (THING_PATH(person)->count == 0) {
        nav_init(person);
    }
    nav_person = person;
    if (person->dest_x != THING_PATH(person)->x1 || person->dest_y != THING_PATH(person)->y1) {
        nav_init(person);
    }
    return orthonav_whatnext(THING_PATH(person), person->x.tile, person->y.tile, person->facing);
}

// Returns 0x8000 if a patient or staff member on the tile next to (x, y) in
// direction dir faces nav_person head-on, else 0. Looks at the tile's first
// four things only.
// FUNCTION: 0x000442a0
s32 person_in_way(u32 x, u32 y, u32 dir)
{
    Map *cell;
    Thing *other;
    s16 id;
    u8 i;

    switch (dir) {
    case 0:
        y--;
        break;
    case 1:
        x++;
        break;
    case 2:
        y++;
        break;
    case 3:
        x--;
        break;
    }
    cell = &map[(s16)x + ((s16)y << 7)];
    id = cell->first;
    for (i = 0; i < 4; i++) {
        if (id > 0) {
        other = &thing[id];
        switch (other->type) {
        case THING_PATIENT:
        case THING_STAFF:
            if (other != nav_person) {
                if (other->facing == opposite_angle[nav_person->facing])
                    return 0x8000;
            }
            break;
        }
        id = other->cell_next;
        } else {
            break;
        }
    }
    return 0;
}

// Returns 0x8000 if a patient or staff member with a higher thing index
// shares nav_person's tile facing the same way, else 0. Looks at the tile's
// first four things only.
// FUNCTION: 0x000443ec
s32 person_overlap(void)
{
    Map *cell;
    Thing *other;
    s16 id;
    u8 i;

    cell = &map[(s16)nav_person->y.tile * 128 + (s16)nav_person->x.tile];
    id = cell->first;
    for (i = 0; i < 4; i++) {
        if (id > 0) {
        other = &thing[id];
        switch (other->type) {
        case THING_PATIENT:
        case THING_STAFF:
            if (other > nav_person) {
                if (other->facing == nav_person->facing) {
                    return 0x8000;
                }
            }
            break;
        }
        id = other->cell_next;
        } else {
            break;
        }
    }
    return 0;
}

extern "C" {

// Plans a new path for person from its tile to (dest_x, dest_y).
// FUNCTION: 0x000444e0
void nav_init(Thing *person)
{
    orthonav_init(THING_PATH(person), person->x.tile, person->y.tile, person->dest_x, person->dest_y);
}

}
