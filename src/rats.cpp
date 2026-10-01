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
extern Event D_001549d0[];
void nav_init(Thing *w);
}

// Tries to make a rat hole for player `owner`: walks from tile allhosps.fcac in
// a random direction over the owner's land to the first wall, and adds a hole
// there (40 at most) unless it is in a room or already has one. Holes on sides
// 0 and 3 also get a floor sprite.
// FUNCTION: 0x0007cd80
void try_create_rathole(u8 owner)
{
    Map *cell;
    u8 side;
    u16 tile;
    int i;
    int slot;

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
    slot = allhosps[owner].fcae++;
    allhosps[owner].fcaf[slot].tile = tile;
    allhosps[owner].fcaf[slot].side = side;
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

// Removes one of player `owner`'s rat holes at random.
// FUNCTION: 0x0007d114
void remove_rand_rathole(u8 owner)
{
    if (allhosps[owner].fcae == 0) {
        return;
    }
    remove_rathole(owner, old_newrand() % allhosps[owner].fcae);
}

// Removes player `owner`'s rat hole idx (moving the last one into its place)
// and its floor sprite.
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

// Removes player `owner`'s rat hole on tile, if any. The last hole in the
// list is never checked.
// FUNCTION: 0x0007d2a8
extern "C" void clear_cell_of_ratholes(u8 owner, u16 tile)
{
    RatHole *first;
    RatHole *last;
    RatHole *hole;

    first = allhosps[owner].fcaf;
    last = allhosps[owner].fcaf + (allhosps[owner].fcae - 1);
    for (hole = first; hole < last; hole++) {
        if (tile == hole->tile) {
            remove_rathole(owner, hole - first);
            break;
        }
    }
}

// Daily rats: for each hospital, may make a rat hole (more likely with
// higher allhosps.fcaa), rarely removes one, and may send out a rat.
// FUNCTION: 0x0007d364
void manage_rats(void)
{
    s8 i;
    int nplayers;

    nplayers = 1;
    if (statvar.unknownfd > 1) {
        nplayers = 4;
    }
    for (i = nplayers - 1; i >= 0; i--) {
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

// Sends a rat from one of player `owner`'s rat holes to another, if there
// are two or more. In the rat game, the next rat takes the look in
// othervar.f9e9.
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
    rat->type = THING_RAT;
    rat->owner = owner;
    if (othervar.f9e9) {
        rat->layers[0] = othervar.f9e9;
        othervar.f9e9 = 0;
    }
    from = old_newrand() % count;
    rat->x.tile = allhosps[owner].fcaf[from].tile % 128;
    rat->y.tile = allhosps[owner].fcaf[from].tile / 128;
    rat->x.frac = 0x80;
    rat->y.frac = 0x80;
    rat->speed = 0x40;
    to = old_newrand() % count;
    if (to == from) {
        to = (to + 1) % count;
    }
    rat->dest_x = allhosps[owner].fcaf[to].tile % 128;
    rat->dest_y = allhosps[owner].fcaf[to].tile / 128;
    insert_node(rat, rat->x, rat->y);
    switch (allhosps[owner].fcaf[from].side) {
    case 0:
        rat->state = RAT_ARRIVES;
        rat->sprite = start[0x3c4].frame;
        rat->flip = start[0x3c4].flags;
        rat->depth = -9;
        break;
    case 3:
        rat->state = RAT_ARRIVES;
        rat->sprite = start[0x3c5].frame;
        rat->flip = start[0x3c5].flags;
        rat->depth = -9;
        break;
    default:
        rat->state = RAT_SCURRIES;
        ratnav_init(rat);
        break;
    }
}

// Rat state RAT_SCURRIES: moves the rat a step towards its next waypoint,
// planning the next leg when it gets there.
// FUNCTION: 0x0007d6cc
void rat_scurries(Thing *rat)
{
    MapPos x;
    MapPos y;

    x.raw = (rat->rat_dx >> 8) + (s16)rat->x.raw;
    y.raw = (rat->rat_dy >> 8) + (s16)rat->y.raw;
    move_node(rat, x, y);
    if (--rat->rat_ticks == 0) {
        ratnav_init(rat);
    } else {
        rat->sprite = frame[rat->sprite].next;
    }
}

// Plans a rat's next leg: at its destination it dives into the hole there
// (RAT_LEAVES) or vanishes; otherwise sets its step and sprite towards the
// next waypoint.
// FUNCTION: 0x0007d760
void ratnav_init(Thing *rat)
{
    s32 dx;
    s32 dy;
    int i;

    nav_init(rat);
    if ((rat->x.tile == rat->dest_x && rat->y.tile == rat->dest_y) || rat->path_step_count == 0) {
        for (i = allhosps[rat->owner].fcae - 1; i >= 0; i--) {
            if (allhosps[rat->owner].fcaf[i].tile == ((s16)rat->y.tile << 7) + (s16)rat->x.tile) {
                switch (allhosps[rat->owner].fcaf[i].side) {
                case 0:
                    rat->sprite = start[0x3c2].frame;
                    rat->flip = start[0x3c2].flags;
                    break;
                case 3:
                    rat->sprite = start[0x3c3].frame;
                    rat->flip = start[0x3c3].flags;
                    break;
                default:
                    remove_thing(rat);
                    return;
                }
                rat->state = RAT_LEAVES;
                rat->y.frac = 0x80;
                rat->x.frac = rat->y.frac;
                rat->depth = -9;
                return;
            }
        }
        remove_thing(rat);
        return;
    }
    if (rat->x.tile == rat->path_step0_x && rat->y.tile == rat->path_step0_y) {
        if (rat->path_step_count > 1 && (rat->x.tile != rat->path_step1_x || rat->y.tile != rat->path_step1_y)) {
            rat->path_step0 = rat->path_step1;
            rat->path_step_count = 1;
        } else {
            remove_thing(rat);
            return;
        }
    }
    dx = rat->path_step0_x - rat->x.tile;
    dy = rat->path_step0_y - rat->y.tile;
    rat->rat_ticks = LbSqrL((dx << 8) * (dx << 8) + (dy << 8) * (dy << 8)) / rat->speed;
    rat->rat_dx = (dx << 16) / rat->rat_ticks;
    rat->rat_dy = (dy << 16) / rat->rat_ticks;
    rat->sprite = start[(LbArcTan(dx, dy) + 0x80) % 0x800 * 8 / 0x800 + 0x3ba].frame;
    rat->flip = start[(LbArcTan(dx, dy) + 0x80) % 0x800 * 8 / 0x800 + 0x3ba].flags;
}

// Removes player `owner`'s rat holes and the litter in the box (x, y, w, h)
// of tiles.
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

// Rat state RAT_ARRIVES: plays the rat coming out of its hole, then
// scurries.
// FUNCTION: 0x0007dc44
void rat_arrives(Thing *rat)
{
    rat->sprite = frame[rat->sprite].next;
    if (frame[rat->sprite].flags & 1) {
        rat->depth = 0;
        rat->state = RAT_SCURRIES;
        ratnav_init(rat);
    }
}

// Rat state RAT_LEAVES: plays the rat going into its hole, then removes it.
// FUNCTION: 0x0007dcb4
void rat_leaves(Thing *rat)
{
    rat->sprite = frame[rat->sprite].next;
    if (frame[rat->sprite].flags & 1) {
        remove_thing(rat);
    }
}

// Per-turn control of the rat game (gamevar.f174 set): a new random weapon
// after every ten kills or so, more rats, extra handymen, and the end of the
// game when allhosps.fcaa falls to 4 or by turn 9000. Outside it, resets the
// rat hit radii.
// FUNCTION: 0x0007dd18
void rat_fest_control(void)
{
    u8 i;
    u8 count;
    u16 id;

    if (gamevar.f174) {
        othervar.f9e5 = 0x10;
        if (myhosp->fd27u - gamevar.f13f > 10) {
            gamevar.f13f = myhosp->fd27u;
            othervar.f9e9 = 1;
            gamevar.f175 = old_newrand() % 3 + 1;
            switch (gamevar.f175) {
            case 1:
                othervar.f9e7 = 4;
                othervar.fa77 = 10;
                break;
            case 2:
                othervar.f9e7 = 8;
                othervar.fa77 = 5;
                break;
            case 3:
                othervar.f9e7 = 0x10;
                othervar.fa77 = 3;
                break;
            default:
                gamevar.f175 = 0;
                othervar.f9e7 = 2;
                othervar.fa77 = 0;
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
            level_number = othervar.fa75;
            myhosp->f29 = level_number;
            area.unknown2 = 6;
            display_mode = 3;
            if (othervar.fa4f) {
                stop_midi();
            }
        }
        if (actual_game_turn % (200 - othervar.fa75 * 2) == 0) {
            id = place_staff_in_struct(2, 0);
            if (id) {
                place_staff_on_map(0x1a32, id, 0, player);
            }
        }
    } else {
        othervar.f9e5 = 0x10;
        othervar.f9e7 = 4;
    }
}

// Sets up rat game level `level` from rat_fest_level: seeds the random
// generators and scatters litter, rat holes and handymen over the player's
// land, then starts the game.
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
                    place_staff_on_map(cell - map, id, 0, player);
                    staff++;
                }
            }
        }
    }
    gamevar.f174 = level;
    gamevar.f175 = 0;
    othervar.f9e5 = 0x10;
    othervar.f9e7 = 4;
    vary_speed(0x1e);
}

// After a level in 640x480, unlocks a bonus rat level (othervar.fa76 13 or 14)
// if the rats killed (myhosp->fd27u), and their ratio to fd29, beat
// awards_trophies' targets, or by the debug flag.
// FUNCTION: 0x0007e140
void try_bonus_level(void)
{
    if (resvars.f1 == SCREEN_MODE_13) {
        if (debug.f219) {
            debug.f219 = 0;
            othervar.fa76 = (old_newrand() & 1) + 13;
        } else if (myhosp->fd29 > 0 && myhosp->fd27u > awards_trophies.unknown0 / 2) {
            if ((myhosp->fd27u << 8) / myhosp->fd29 > (awards_trophies.unknowne << 8) / 100) {
                othervar.fa76 = (old_newrand() & 1) + 13;
            }
        }
    }
}
