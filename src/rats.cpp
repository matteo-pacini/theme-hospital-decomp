#include "hosp.h"

u16 which_room_number(u16 tile);
void remove_litter(u16 tile, u8 owner);
void remove_rathole(u8 owner, u8 idx);
void try_create_rathole(u8 owner);
void remove_rand_rathole(u8 owner);
void make_a_rat(u8 owner);
void ratnav_init(Thing *w);
s16 place_staff_in_struct(u16 a, u16 b);

extern Menu area;

extern "C" {
extern u8 D_00155da1;
extern u16 D_00155d9d;
extern u16 D_00155d9f;
extern u8 D_00155e2f;
extern u8 D_00155e2d;
extern u8 D_00155e07;
extern Event D_001549d0[];
extern u8 D_00155e2e;
void fn_000444e0(Thing *w);
void fn_00024a34(u16 tile, u16 room, u8 side);
#pragma aux fn_00024a34 "W?clear_cell_of_ratholes$n(ucus)v"
}

// FUNCTION: 0x00024a34
void fn_00024a34(u16 tile, u16 room, u8 side)
{
    s16 x;
    s16 y;
    u16 a;
    u16 b;
    u8 clear;

    x = tile % 128;
    y = tile / 128;
    switch (side) {
    case 0:
        a = which_room_number((y - 1) * 128 + x);
        b = which_room_number((y + 1) * 128 + x);
        clear = 1;
        if (a > 0 && a != room) {
            clear = 0;
        }
        if (b > 0 && b != room) {
            if (which_room_number(y * 128 + x) != 0) {
                clear = 0;
            }
        }
        if (blocks[map[x + (y << 7)].unknown3].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].unknown3 = 0;
        }
        break;
    case 1:
        a = which_room_number(x - 1 + y * 128);
        b = which_room_number((x + 1) + y * 128);
        clear = 1;
        if (a > 0 && a != room) {
            clear = 0;
        }
        if (b > 0 && b != room) {
            if (which_room_number(y * 128 + x) != 0) {
                clear = 0;
            }
        }
        if (blocks[map[x + (y << 7)].unknown4].width & 0x10) {
            clear = 0;
        }
        if (clear) {
            map[x + (y << 7)].unknown4 = 0;
        }
        break;
    }
}

// FUNCTION: 0x0007cd80
void try_create_rathole(u8 owner)
{
    Map *cell;
    u8 side;
    u16 tile;
    int i;
    int n;

    cell = &map[allhosps[owner].fcac];
    side = old_newrand() & 3;
    if (allhosps[owner].fcae == 0x28) {
        return;
    }
    switch (side) {
    case 0:
        for (; cell->unknown7 == (1 << owner); cell -= 128) {
            if (cell->unknown3 != 0 && (cell->unknown3 < 0x8e || cell->unknown3 > 0x91)) {
                goto found;
            }
        }
        break;
    case 1:
        for (; cell->unknown7 == (1 << owner); cell += 1) {
            if (cell[1].unknown4 != 0 && (cell[1].unknown4 < 0x8e || cell[1].unknown4 > 0x91)) {
                goto found;
            }
        }
        break;
    case 2:
        for (; cell->unknown7 == (1 << owner); cell += 128) {
            if (cell[128].unknown3 != 0 && (cell[128].unknown3 < 0x8e || cell[128].unknown3 > 0x91)) {
                goto found;
            }
        }
        break;
    case 3:
        for (; cell->unknown7 == (1 << owner); cell -= 1) {
            if (cell->unknown4 != 0 && (cell->unknown4 < 0x8e || cell->unknown4 > 0x91)) {
                goto found;
            }
        }
        break;
    }
    return;
found:
    tile = cell - map;
    if (which_room(tile) != 0) {
        return;
    }
    for (i = allhosps[owner].fcae - 1; i >= 0; i--) {
        if (allhosps[owner].fcaf[i].tile == tile) {
            return;
        }
    }
    n = allhosps[owner].fcae++;
    allhosps[owner].fcaf[n].tile = tile;
    allhosps[owner].fcaf[n].side = side;
    if (side == 0) {
        remove_litter(tile, owner);
        cell->floor &= 0xf0;
        cell->floor |= 7;
    } else if (side == 3) {
        remove_litter(tile, owner);
        cell->floor &= 0xf0;
        cell->floor |= 8;
    }
}

// FUNCTION: 0x0007d114
void remove_rand_rathole(u8 owner)
{
    if (allhosps[owner].fcae == 0) {
        return;
    }
    remove_rathole(owner, old_newrand() % allhosps[owner].fcae);
}

// FUNCTION: 0x0007d184
void remove_rathole(u8 owner, u8 idx)
{
    RatHole *hole;
    RatHole *last;

    hole = &allhosps[owner].fcaf[idx];
    last = &allhosps[owner].fcaf[allhosps[owner].fcae - 1];
    if (hole->side == 0 || hole->side == 3) {
        map[hole->tile].floor &= 0xf0;
        map[hole->tile].floor |= 0;
    }
    fn_000c21e6(hole, last, 3);
    memset(last, 0, 3);
    allhosps[owner].fcae--;
}

// FUNCTION: 0x0007d364
void manage_rats(void)
{
    s8 i;
    int n;

    n = 1;
    if (statvar.unknownfd > 1) {
        n = 4;
    }
    for (i = n - 1; i >= 0; i--) {
        if (statvar.unknownfd > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        if (old_newrand() % 100 < allhosps[i].fcaa) {
            try_create_rathole(i);
        }
        if (old_newrand() % 4000 < 10) {
            remove_rand_rathole(i);
        }
        if (old_newrand() % 100 < 10) {
            make_a_rat(i);
        }
    }
}

// FUNCTION: 0x0007d448
void make_a_rat(u8 owner)
{
    u32 count;
    Thing *rat;
    u32 from;
    u32 to;

    count = allhosps[owner].fcae;
    if (count < 2) {
        return;
    }
    rat = allocate_thing(0x32);
    if (rat == 0) {
        return;
    }
    rat->renderer = 0xc;
    rat->owner = owner;
    if (D_00155da1) {
        rat->unknown29 = D_00155da1;
        D_00155da1 = 0;
    }
    from = old_newrand() % count;
    rat->x.tile = allhosps[owner].fcaf[from].tile % 128;
    rat->y.tile = allhosps[owner].fcaf[from].tile / 128;
    rat->x.frac = 0x80;
    rat->y.frac = 0x80;
    rat->unknown18 = 0x40;
    to = old_newrand() % count;
    if (to == from) {
        to = (to + 1) % count;
    }
    rat->unknown19 = allhosps[owner].fcaf[to].tile % 128;
    rat->unknown20 = allhosps[owner].fcaf[to].tile / 128;
    insert_node(rat, rat->x, rat->y);
    switch (allhosps[owner].fcaf[from].side) {
    case 0:
        rat->kind = 4;
        rat->sprite = start[0x3c4].frame;
        rat->palette = start[0x3c4].flags;
        rat->depth = -9;
        break;
    case 3:
        rat->kind = 4;
        rat->sprite = start[0x3c5].frame;
        rat->palette = start[0x3c5].flags;
        rat->depth = -9;
        break;
    default:
        rat->kind = 2;
        ratnav_init(rat);
        break;
    }
}

// FUNCTION: 0x0007d6cc
void rat_scurries(Thing *w)
{
    MapPos x;
    MapPos y;

    x.raw = (w->value >> 8) + (s16)w->x.raw;
    y.raw = (w->f3f >> 8) + (s16)w->y.raw;
    move_node(w, x, y);
    if (--w->mode == 0) {
        ratnav_init(w);
    } else {
        w->sprite = frame[w->sprite].next;
    }
}

// FUNCTION: 0x0007d760
void ratnav_init(Thing *w)
{
    s32 dx;
    s32 dy;
    int i;

    fn_000444e0(w);
    if ((w->x.tile == w->unknown19 && w->y.tile == w->unknown20) || w->f36 == 0) {
        for (i = allhosps[w->owner].fcae - 1; i >= 0; i--) {
            if (allhosps[w->owner].fcaf[i].tile == ((s16)w->y.tile << 7) + (s16)w->x.tile) {
                switch (allhosps[w->owner].fcaf[i].side) {
                case 0:
                    w->sprite = start[0x3c2].frame;
                    w->palette = start[0x3c2].flags;
                    break;
                case 3:
                    w->sprite = start[0x3c3].frame;
                    w->palette = start[0x3c3].flags;
                    break;
                default:
                    remove_thing(w);
                    return;
                }
                w->kind = 6;
                w->y.frac = 0x80;
                w->x.frac = w->y.frac;
                w->depth = -9;
                return;
            }
        }
        remove_thing(w);
        return;
    }
    if (w->x.tile == w->f2e && w->y.tile == w->f2f) {
        if (w->f36 > 1 && (w->x.tile != w->f30 || w->y.tile != w->f31)) {
            w->f2e_raw = w->f30_raw;
            w->f36 = 1;
        } else {
            remove_thing(w);
            return;
        }
    }
    dx = w->f2e - w->x.tile;
    dy = w->f2f - w->y.tile;
    w->mode = LbSqrL((dx << 8) * (dx << 8) + (dy << 8) * (dy << 8)) / w->unknown18;
    w->value = (dx << 16) / w->mode;
    w->f3f = (dy << 16) / w->mode;
    w->sprite = start[(LbArcTan(dx, dy) + 0x80) % 0x800 * 8 / 0x800 + 0x3ba].frame;
    w->palette = start[(LbArcTan(dx, dy) + 0x80) % 0x800 * 8 / 0x800 + 0x3ba].flags;
}

// FUNCTION: 0x0007dab4
void cleanse_area(u8 owner, u8 x, u8 y, u8 w, u8 h)
{
    s8 i;
    u32 tx;
    u32 ty;

    for (i = allhosps[owner].fcae - 1; i >= 0; i--) {
        tx = allhosps[owner].fcaf[i].tile % 128;
        ty = allhosps[owner].fcaf[i].tile / 128;
        if (((x <= tx && x + w > tx) && y <= ty) && y + h > ty) {
            remove_rathole(owner, i);
        }
    }
    for (tx = x; x + w > tx; tx++) {
        for (ty = y; y + h > ty; ty++) {
            if (map[(s16)tx + (s16)ty * 128].floor & 0xf) {
                remove_litter(ty * 128 + tx, owner);
            }
        }
    }
}

// FUNCTION: 0x0007dc44
void rat_arrives(Thing *w)
{
    w->sprite = frame[w->sprite].next;
    if (frame[w->sprite].flags & 1) {
        w->depth = 0;
        w->kind = 2;
        ratnav_init(w);
    }
}

// FUNCTION: 0x0007dcb4
void rat_leaves(Thing *w)
{
    w->sprite = frame[w->sprite].next;
    if (frame[w->sprite].flags & 1) {
        remove_thing(w);
    }
}

// FUNCTION: 0x0007dd18
void rat_fest_control(void)
{
    u8 i;
    u8 count;
    u16 id;

    if (gamevar.f174) {
        D_00155d9d = 0x10;
        if (myhosp->fd27u - gamevar.f13f > 10) {
            gamevar.f13f = myhosp->fd27u;
            D_00155da1 = 1;
            gamevar.f175 = old_newrand() % 3 + 1;
            switch (gamevar.f175) {
            case 1:
                D_00155d9f = 4;
                D_00155e2f = 10;
                break;
            case 2:
                D_00155d9f = 8;
                D_00155e2f = 5;
                break;
            case 3:
                D_00155d9f = 0x10;
                D_00155e2f = 3;
                break;
            default:
                gamevar.f175 = 0;
                D_00155d9f = 2;
                D_00155e2f = 0;
                break;
            }
        }
        count = old_newrand() % 5 + 1;
        for (i = 0; i < count; i++) {
            make_a_rat(player);
            gamevar.f141 += count;
        }
        if (myhosp->fcaa <= 4 || actual_game_turn >= 9000) {
            D_001549d0[player].type = 0x38;
            level_number = D_00155e2d;
            myhosp->f29 = level_number;
            area.unknown2 = 6;
            display_mode = 3;
            if (D_00155e07) {
                stop_midi();
            }
        }
        if (actual_game_turn % (200 - D_00155e2d * 2) == 0) {
            id = place_staff_in_struct(2, 0);
            if (id) {
                fn_00029c4c(0x1a32, id, 0, player);
            }
        }
    } else {
        D_00155d9d = 0x10;
        D_00155d9f = 4;
    }
}

// FUNCTION: 0x0007df1c
void rat_fest_builder(u8 level)
{
    u32 holes;
    u32 litter;
    u32 staff;
    u8 owner;
    RatFestLevel *lvl;
    Map *cell;
    u16 id;

    holes = 0;
    litter = 0;
    staff = 0;
    owner = player;
    lvl = &rat_fest_level[level];
    seed = safeseed = lvl->seed;
    for (cell = map; cell < end_map; cell++) {
        if (cell->unknown7 == (1 << owner)) {
            if (lvl->litter_max > litter && saferand() % lvl->litter_chance == 0) {
                place_litter(cell - map, owner, saferand() % 3 + 1);
                litter++;
            }
            if ((lvl->holes_max > holes && saferand() % lvl->holes_chance == 0) && allhosps[owner].fcac) {
                try_create_rathole(owner);
                holes++;
            }
            if (lvl->staff_max > staff && saferand() % lvl->staff_chance == 0) {
                id = place_staff_in_struct(2, 0);
                if (id) {
                    fn_00029c4c(cell - map, id, 0, player);
                    staff++;
                }
            }
        }
    }
    gamevar.f174 = level;
    gamevar.f175 = 0;
    D_00155d9d = 0x10;
    D_00155d9f = 4;
    vary_speed(0x1e);
}

// FUNCTION: 0x0007e140
void try_bonus_level(void)
{
    if (resvars.f1 == SCREEN_MODE_13) {
        if (debug.f219) {
            debug.f219 = 0;
            D_00155e2e = (old_newrand() & 1) + 13;
        } else if (myhosp->fd29 > 0 && myhosp->fd27u > awards_trophies.unknown0 / 2) {
            if ((myhosp->fd27u << 8) / myhosp->fd29 > (awards_trophies.unknowne << 8) / 100) {
                D_00155e2e = (old_newrand() & 1) + 13;
            }
        }
    }
}
