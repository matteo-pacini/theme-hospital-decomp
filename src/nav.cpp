#include "hosp.h"

#define THING_PATH(w) ((orthonav_path *)&(w)->unknown30[0xc])

extern Thing *nav_person;
extern s16 opposite_angle[];

s32 orthonav_whatnext(orthonav_path *path, s8 x, s8 y, s32 a);

extern "C" {

void fn_000444e0(Thing *w);

}

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
        if (cell->unknown3) {
            return 1;
        }
        break;
    case 1:
        cell = &map[(s16)(x + 1) + ((s16)y << 7)];
        if (cell->unknown4) {
            return 1;
        }
        break;
    case 2:
        cell = &map[(s16)x + ((s16)(y + 1) << 7)];
        if (cell->unknown3) {
            return 1;
        }
        break;
    case 3:
        if (cell->unknown4) {
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

// FUNCTION: 0x00044210
s16 person_head_for(Thing *person, u8 a)
{
    if (THING_PATH(person)->result == 0) {
        fn_000444e0(person);
    }
    nav_person = person;
    if (person->unknown19 != THING_PATH(person)->x1 || person->unknown20 != THING_PATH(person)->y1) {
        fn_000444e0(person);
    }
    return orthonav_whatnext(THING_PATH(person), person->x.tile, person->y.tile, person->unknown17);
}

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
        switch (other->renderer) {
        case 2:
        case 4:
            if (other != nav_person) {
                if (other->unknown17 == opposite_angle[nav_person->unknown17])
                    return 0x8000;
            }
            break;
        }
        id = other->next;
        } else {
            break;
        }
    }
    return 0;
}

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
        switch (other->renderer) {
        case 2:
        case 4:
            if (other > nav_person) {
                if (other->unknown17 == nav_person->unknown17) {
                    return 0x8000;
                }
            }
            break;
        }
        id = other->next;
        } else {
            break;
        }
    }
    return 0;
}

extern "C" {

// FUNCTION: 0x000444e0
void fn_000444e0(Thing *person)
{
    orthonav_init(THING_PATH(person), person->x.tile, person->y.tile, person->unknown19, person->unknown20);
}

}
