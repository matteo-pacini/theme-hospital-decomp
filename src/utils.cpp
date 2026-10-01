#include "hosp.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

#define ABS(a) ((a) < 0 ? -(a) : (a))

#define PUT16BE(p, v) \
    (*(u16 *)(p) = (v), tmp = *(u8 *)((u32)(p) + 1), \
     *(u8 *)((u32)(p) + 1) = *(p), *(p) = tmp, (*(u16 **)&(p))++)

struct SortEntry {
    signed int key : 27;
};

extern "C" {

extern char D_0015fde4[];
extern Event D_001549d0[];
extern u8 D_0015f982;
extern u32 D_0014bcc4, D_0014bcd0, D_0014bcd8, D_0014bce0, D_0014bce4, D_0014bce8;
extern u32 D_0013c60c, D_0013c618, D_0013c620, D_0013c628, D_0013c62c, D_0013c630;
extern u8 D_001581ec, D_001581ed, D_001581ef;
extern u8 D_0015fe74;

void fn_00015b68(Thing *w);
Thing *fn_00019aac(Map *t, u16 kind);
void fn_0001b7e0(char *dst, s8 *src, u8 mode);
int fn_0001b874(u8 c);
void fn_0001bab4(u8 i);
void fn_0001bdf8(void);
void fn_0001be78(u8 dir);
void fn_0001bee4(void);
u8 fn_000856c8(int player);
int LbScreenLock(void);
void fn_000c93bf(char *dst, char *src);
void LbPaletteFade(u8 *palette, int steps, int flag);
void fn_000c9be0(int x0, int y0, int x1, int y1, u8 color);

}

char fingchk2[20] = { 0xd6, 0xd4, 0xee, 0xe4, 0xc8, 0xd6, 0xd4, 0xcc };

// FUNCTION: 0x000196c0
u8 pick_random_player(void)
{
    u8 i;
    u8 count;

    if (statvar.unknownfd <= 1) {
        return 0;
    }
    for (i = count = 0; i < 4; i++) {
        if (D_001609c4[i].active) {
            count++;
        }
    }
    if (!count) {
        return 0xff;
    }
    count = old_newrand() % count;
    return fn_000856c8(count + 1);
}

// FUNCTION: 0x0001974c
u16 div_err(void)
{
    return 1;
}

// FUNCTION: 0x00019770
void setup_colors(void)
{
    color.unknown0 = current_palette;
    color.unknown7 = LbPaletteFindColour(current_palette, 0x3f, 0x3f, 0x3f);
    color.unknown8 = LbPaletteFindColour(current_palette, 0, 0, 0);
    color.unknown9 = LbPaletteFindColour(current_palette, 0, 0, 0x10);
    color.unknownA = LbPaletteFindColour(current_palette, 0, 0x3f, 0);
    color.unknownB = LbPaletteFindColour(current_palette, 0x3f, 0x30, 0x17);
    color.unknownC = LbPaletteFindColour(current_palette, 0x3f, 0x17, 0x17);
    color.unknownD = LbPaletteFindColour(current_palette, 0x3f, 0, 0);
    color.unknownE = LbPaletteFindColour(current_palette, 0x1b, 0x37, 0x39);
    color.unknownF = LbPaletteFindColour(current_palette, 0x35, 0x1f, 0x42);
    color.unknown10 = LbPaletteFindColour(current_palette, 0x17, 0x43, 0x10);
    color.unknown11 = LbPaletteFindColour(current_palette, 0x26, 0x29, 0x2f);
    color.unknown12 = LbPaletteFindColour(current_palette, 0, 0x26, 0x56);
    color.unknown13 = LbPaletteFindColour(current_palette, 0x24, 0x50, 0x2f);
    color.unknown14 = LbPaletteFindColour(current_palette, 0x3f, 0x1e, 0x1e);
}

// FUNCTION: 0x00019918
u16 can_i_get_to(Thing *mover, u16 pos)
{
    orthonav_path path;
    u16 x = pos % 128;
    u16 y = pos / 128;

    if (mover->type == 6) {
        orthonav_init(&path, mover->use_pos[1] % 128, mover->use_pos[1] / 128, x, y);
    } else {
        orthonav_init(&path, mover->x.tile, mover->y.tile, x, y);
    }
    return path.result;
}

// FUNCTION: 0x000199e8
void remove_thing(Thing *target)
{
    fn_00015b68(target);
    memset(target, 0, sizeof(Thing));
}

// FUNCTION: 0x00019a1c
void count_hospital_area(u16 player)
{
    AllHosps *hosp = allhosps + player;
    Map *cell;

    if (hosp->f2a != 2) {
        hosp->f1f1 = 0;
        for (cell = map; cell < end_map; cell++) {
            if (cell->unknown7 & (1 << player)) {
                hosp->f1f1++;
            }
        }
    }
}

// FUNCTION: 0x00019aac
Thing *fn_00019aac(Map *cell, u16 kind)
{
    Thing *occupant;

    if (!(cell < map || cell >= end_map)) {
        for (occupant = thing + cell->first; occupant > thing; occupant = thing + occupant->cell_next) {
            if (occupant->type == kind) {
                return occupant;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00019b44
Thing *search_mapwho_for_room_door(Map *cell, u16 id)
{
    Thing *occupant;

    if (!(cell < map || cell >= end_map)) {
        for (occupant = thing + cell->first; occupant > thing; occupant = thing + occupant->cell_next) {
            if (occupant->type == 8 && occupant->room_id == id) {
                return occupant;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00019bec
void screen_shot(void)
{
    u8 palette[0x300];
    char path[0x80];
    int i;
    u8 *out;
    int shot_no;
    s32 handle;
    int j;
    char *fmt;
    u8 tmp;

    fmt = "scrshots\\hosp%04d.lbm";
    if (debug.unknown1e4 == 0) {
        debug.unknown1e4 = (u8 *)LbMemoryAlloc(400000);
        if (debug.unknown1e4 == 0) {
            return;
        }
    }
    if (debug.unknown1e0 == -1) {
        for (i = 0; i < 10000; i++) {
            sprintf(path, fmt, i);
            handle = LbFileOpen(path, 2);
            if (handle == -1) {
                debug.unknown1e0 = i - 1;
                break;
            }
            LbFileClose(handle);
        }
    }
    shot_no = ++debug.unknown1e0;
    sprintf(path, fmt, shot_no);
    if (shot_no >= 10000) {
        return;
    }
    out = debug.unknown1e4;
    *out = 'm';
    out++;
    *out = 'h';
    out++;
    *out = 'w';
    out++;
    *out = 'a';
    out++;
    *out = 'n';
    out++;
    *out = 'h';
    out++;
    PUT16BE(out, 4);
    PUT16BE(out, lbDisplay.unknown18);
    PUT16BE(out, lbDisplay.unknown1c);
    PUT16BE(out, 0x100);
    PUT16BE(out, 0x100);
    PUT16BE(out, 0x100);
    PUT16BE(out, 0x100);
    out += 0xc;
    LbPaletteGet(palette);
    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 3; j++) {
            *out = palette[i * 3 + j] << 2;
            out++;
        }
    }
    LbMemoryCopy(out, (void *)lbDisplay.unknown4, lbDisplay.unknown18 * lbDisplay.unknown1c);
    out += lbDisplay.unknown18 * lbDisplay.unknown1c;
    LbFileSaveAt(path, debug.unknown1e4, out - debug.unknown1e4);
}

// FUNCTION: 0x00019ef8
RoomInfo *get_room_info_rand(u8 kind, u8 mode, u8 owner)
{
    u8 count = 0;
    RoomInfo *room;
    RoomInfo *list[30];

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->f3 < 0 || room->owner != owner) {
            continue;
        }
        if (room->type != kind || !(mode == 0x65 || room->f3 == 0)) {
            continue;
        }
        if (count < 30) {
            list[count++] = room;
        }
    }
    if (count == 0) {
        return 0;
    }
    return list[old_newrand() % count];
}

// FUNCTION: 0x00019fe0
RoomInfo *get_room_info(u8 kind, u8 mode, u8 owner)
{
    RoomInfo *found = 0;
    RoomInfo *room;

    for (room = room_info + 1; room < end_room_info; room++) {
        if (room->f3 >= 0) {
            if (room->owner == owner) {
                if (room->type == kind) {
                    if (room->f3 == 0) {
                        return room;
                    }
                    if (mode == 0x65) {
                        return room;
                    }
                    if (mode == 0x64) {
                        found = room;
                    }
                }
            }
        }
    }
    if (mode == 0x64 && found > 0) {
        return found;
    }
    return room_info;
}

// FUNCTION: 0x0001a0a0
RoomInfo *get_nearest_room_info(Thing *person, u8 kind)
{
    u16 dist;
    RoomInfo *room;
    u16 pos;
    RoomInfo *best = 0;
    u16 bestDist = 9999;

    pos = person->x.tile + (person->y.tile << 7);
    for (room = room_info + 1; room < end_room_info; room++) {
        if (!(room->f3 == 0 || room->f3 == 1)) {
            continue;
        }
        if (room->owner == person->owner) {
            if (room->type == kind) {
                dist = (pos % 128 - room->unknown4 % 128) * (pos % 128 - room->unknown4 % 128)
                    + (pos / 128 - room->unknown4 / 128) * (pos / 128 - room->unknown4 / 128);
                if (dist < bestDist) {
                    best = room;
                    bestDist = dist;
                }
            }
        }
    }
    return best;
}

// FUNCTION: 0x0001a238
void cheat(u32 flags)
{
    u32 unused1;
    u32 unused2;
    u32 i;

    if (statvar.unknownfd <= 1) {
        if (othervar.fa8e != 0) {
            play_ambient(saferand() % 2 + 0xc4);
            if (flags & 2) {
                for (i = 1; i < 0x3e; i++) {
                    if (objects[i].unknown1c) {
                        allhosps[player].equipment[i].unknown0 = 1;
                    }
                }
            }
            if (flags & 4) {
                for (i = 1; i < 0x3e; i++) {
                    objects[i].unknown1c = 1;
                    allhosps[player].equipment[i].unknown0 = 1;
                }
            }
            if (flags & 8) {
                change_balance(0xf, 1000, 0, player);
            }
            if (flags & 0x10) {
                change_balance(0xf, -1000, 0, player);
            }
        }
    }
}

// FUNCTION: 0x0001a374
void set_mouse_mode(const u8 mode)
{
    switch (mode) {
    case 0:
        othervar.f30 = 1;
        break;
    case 1:
    case 2:
    case 7:
        othervar.f30 = 3;
        break;
    case 10:
        othervar.f30 = 0xd;
        break;
    case 9:
        othervar.f30 = 0;
        break;
    case 11:
        othervar.f30 = 0xe;
        break;
    case 12:
        othervar.f30 = 4;
        break;
    case 14:
        othervar.f30 = 9;
        break;
    case 15:
        othervar.f30 = 0xc;
        break;
    case 16:
        othervar.f30 = 9;
        break;
    case 18:
        othervar.f30 = 0xf;
        break;
    case 20:
        othervar.f30 = 0xf;
        break;
    case 21:
        othervar.f30 = 1;
        break;
    case 22:
        othervar.f30 = 0x1a;
        break;
    case 23:
        othervar.f30 = 0x10;
        break;
    case 24:
        othervar.f30 = 0xb;
        break;
    case 25:
        othervar.f30 = 3;
        break;
    case 26:
        othervar.f30 = 0x18;
        break;
    case 27:
        othervar.f30 = 5;
        break;
    case 28:
        othervar.f30 = 0x12;
        break;
    case 29:
        othervar.f30 = 1;
        break;
    case 30:
        othervar.f30 = 0x16;
        break;
    case 31:
        othervar.f30 = 9;
        break;
    case 32:
        othervar.f30 = 9;
        break;
    case 33:
        othervar.f30 = 0x20;
        break;
    case 34:
        othervar.f30 = 9;
        break;
    case 36:
        othervar.f30 = 7;
        break;
    case 38:
        othervar.f30 = 0x14;
        break;
    case 40:
        othervar.f30 = 0x1c;
        break;
    case 41:
        othervar.f30 = 0x1e;
        break;
    default:
        othervar.f30 = 1;
        break;
    }
    gamevar.f12e = mode;
    if (!mouse_was_on_window) {
        set_mouse_sprite(othervar.f30);
    } else {
        mouse_was_on_window = 0;
    }
}

// FUNCTION: 0x0001a5f4
void generate_bw_remap(u8 *palette, u8 *out)
{
    u32 grey;
    u8 *src;
    u8 i;
    u8 colour;

    for (i = 0xff, src = (u8 *)palette; i != 0; i--, src += 3) {
        grey = LbSqrL(src[0] * src[0] + src[1] * src[1]);
        grey = LbSqrL(grey * grey + src[2] * src[2]);
        grey = (grey * 0x3f + 0x36) / 0x6d;
        colour = LbPaletteFindColour(palette, (u8)grey, (u8)grey, (u8)grey);
        *out++ = colour;
    }
}

// FUNCTION: 0x0001a6cc
u8 check_points_connected(u16 from, u16 to)
{
    orthonav_path path;

    orthonav_init(&path, from % 128, from / 128, to % 128, to / 128);
    return path.result != 0;
}

// FUNCTION: 0x0001a764
u8 direction_to_face(Thing *self, Thing *target)
{
    if (target->y.tile > self->y.tile) {
        return 0;
    }
    if (target->x.tile < self->x.tile) {
        return 1;
    }
    if (target->y.tile < self->y.tile) {
        return 2;
    }
    if (target->x.tile > self->x.tile) {
        return 3;
    }
    return self->facing;
}

// FUNCTION: 0x0001a7dc
void convert_MW_to_xy(u16 pos, s8 *x, s8 *y)
{
    s8 lx = pos % 128;
    s8 ly = pos / 128;

    if (lx < -0x40) {
        lx += 0x80;
        ly += 0xff;
    }
    if (lx > 0x40) {
        lx += 0x80;
        ++ly;
    }
    *x = lx;
    *y = ly;
}

// FUNCTION: 0x0001a854
void measure_objects(void)
{
    u8 i;
    u8 j;
    u8 k;

    for (i = 0; i < 0x3e; i++) {
        for (j = 0; j < 4; j++) {
            Shapes *shape;
            s8 maxX;
            s8 minY;
            s8 minX;
            s8 maxY;

            shape = shapes + objects[i].shapes[j];
            minX = 100;
            minY = 100;
            maxX = -100;
            maxY = -100;
            for (k = 0; k < shape->count; k++) {
                s8 x;
                s8 y;

                convert_MW_to_xy(shape->points[k], &x, &y);
                minX > x ? minX = x : 0;
                minY > y ? minY = y : 0;
                maxX < x ? maxX = x : 0;
                maxY < y ? maxY = y : 0;
            }
            statvar.fff[i][j].minX = minX;
            statvar.fff[i][j].minY = minY;
            statvar.fff[i][j].maxX = maxX;
            statvar.fff[i][j].maxY = maxY;
        }
    }
}

// FUNCTION: 0x0001aa10
int descend(const void *a, const void *b)
{
    return -(((const SortEntry *)a)->key - ((const SortEntry *)b)->key);
}

// FUNCTION: 0x0001aa50
int ascend(const void *a, const void *b)
{
    return ((const SortEntry *)a)->key - ((const SortEntry *)b)->key;
}

// FUNCTION: 0x0001aa8c
void set_mouse_sprite(const u8 idx)
{
    const u8 *kinds;
    TbSprite *cursors;

    if (othervar.fa65) {
        kinds = screen_pointer_hotspots;
        cursors = screen_pointers_tab_start;
        pointer_second_frame_check = screen_pointer_second_frame;
    } else {
        kinds = ingame_pointer_hotspots;
        cursors = pointer;
        pointer_second_frame_check = ingame_pointer_second_frame;
    }
    othervar.f31 = othervar.f30;
    othervar.f30 = idx;
    switch (kinds[idx]) {
    case 1:
        LbMouseChangeSpriteAndHotspot(cursors + othervar.f30, 0, 0);
        break;
    case 2:
        LbMouseChangeSpriteAndHotspot(cursors + othervar.f30, cursors[idx].width / 2, cursors[idx].height / 2);
        break;
    }
}

// FUNCTION: 0x0001ab90
u16 can_i_go_near_point(Thing *person, u16 target)
{
    u16 best = 0xffff;
    u16 cand;
    u16 result = 0;
    u16 pos;
    u16 dist;
    u8 i;

    pos = person->x.tile + (person->y.tile << 7);
    for (i = 0; i < 4; i++) {
        cand = target + around[i];
        dist = ABS(pos % 128 - cand % 128) + ABS(pos / 128 - cand / 128);
        if (dist < best && can_i_get_to(person, cand)) {
            best = dist;
            result = cand;
        }
    }
    return result;
}

// FUNCTION: 0x0001ad58
u8 am_i_near_point(Thing *person, u16 pos)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if ((s16)person->x.tile + ((s16)person->y.tile << 7) == pos + around[i]) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0001add0
void vary_speed(s16 speed)
{
    u8 i;

    for (i = 0; i < 5; i++) {
        othervar.fa3a[i] = 0;
    }
    switch (speed) {
    case -1:
        gamevar.f138 = 0;
        myplayer.f1es--;
        gamevar.f139 = 0;
        if (myplayer.f1es < 1) {
            myplayer.f1es = 1;
        } else if (myplayer.f1es > 0x7d00) {
            myplayer.f1es = 0x7d00;
        }
        othervar.fa3b = 1;
        break;
    case 1:
        gamevar.f138 = 0;
        myplayer.f1es++;
        gamevar.f139 = 0;
        if (myplayer.f1es < 1) {
            myplayer.f1es = 1;
        } else if (myplayer.f1es > 0x7d00) {
            myplayer.f1es = 0x7d00;
        }
        othervar.fa3d = 1;
        break;
    case 10:
        gamevar.f138 = 0;
        myplayer.f1es = 1;
        gamevar.f139 = 0;
        othervar.fa3a[0] = 1;
        break;
    case 20:
        gamevar.f138 = 0;
        myplayer.f1es = 5;
        gamevar.f139 = 0;
        othervar.fa3b = 1;
        break;
    case 30:
        gamevar.f138 = 0;
        myplayer.f1es = 0x16;
        gamevar.f139 = 0;
        othervar.fa3c = 1;
        break;
    case 40:
        gamevar.f138 = 0;
        myplayer.f1es = 0x19;
        gamevar.f139 = 0;
        othervar.fa3d = 1;
        break;
    case 50:
        gamevar.f138 = 1;
        myplayer.f1es = 0x7d00;
        gamevar.f139 = 1;
        othervar.fa3e = 1;
        break;
    }
}

// FUNCTION: 0x0001afd8
void compute_whether_user_not_busy(void)
{
    u8 idle = CountWindows(open_windows) == 0 && display_mode == 0 && !debug.unknownd7;

    if (idle && !othervar.f8b1) {
        othervar.f8b2 = othervar.f8a2;
    }
    othervar.f8b1 = idle;
}

// FUNCTION: 0x0001b050
u32 calc_room_value(RoomInfo *room)
{
    Thing *obj;
    int score;
    int base;
    int count = 0;

    if (room <= room_info || room >= end_room_info) {
        return 0;
    }
    base = (room->f1 + room->f2) * 5;
    for (obj = thing + room->first_object; obj > thing; obj = thing + obj->room_obj_next) {
        count++;
    }
    count -= rooms[room->type].fc;
    count += room->f1b / 2;
    score = base + count * 5;
    if (score < 0) {
        score = 0;
    }
    return score;
}

// FUNCTION: 0x0001b150
Thing *find_person_in_room(RoomInfo *room, u16 kind, u16 sub)
{
    Thing *person;

    if (room) {
        for (person = thing + room->first; person > thing; person = thing + person->room_next) {
            if (person->type == kind) {
                if (person->subtype == sub) {
                    return person;
                }
            }
        }
    }
    return 0;
}

// FUNCTION: 0x0001b1f8
void setup_abort_all_requesters(void)
{
    if (gamevar.f12e == 0xe) {
        D_001549d0[player].type = 0x7b;
        return;
    }
    switch (crequester.unknown2) {
    case 10:
        if (crequester.unknown9) {
            D_001549d0[player].type = 0x5c;
            D_001549d0[player].value = 1;
            othervar.f9c5 = 1;
            return;
        }
        break;
    case 5:
        if (crequester.unknown0 == 0) {
            if (othervar.f9c5 == 1) {
                sprintf(othervar.f935, "%s", yesno_text[4]);
                open_yes_no_requester(0x79, 0, 0, 0x78, 1, 1);
                return;
            }
            D_001549d0[player].type = 0x79;
            return;
        }
        if (crequester.unknown8 != 3) {
            D_001549d0[player].type = 0x45;
            return;
        }
        break;
    case 12:
        req12_clicked_sack();
        return;
    }
    D_001549d0[player].type = 0x38;
    if (gamevar.unknown75) {
        D_001549d0[player].value = gamevar.unknown75;
    } else {
        D_001549d0[player].value = 0;
    }
    D_001549d0[player].arg = gamevar.f12e;
}

// FUNCTION: 0x0001b388
void copy_screen_to_wscreen(const u8 *src)
{
    LbScreenCopyBox(src, lbDisplay.unknown4, lbDisplay.unknown1c);
}

// FUNCTION: 0x0001b3b8
void copy_screen_from_wscreen(u8 *dst)
{
    LbScreenCopyBox(lbDisplay.unknown4, dst, lbDisplay.unknown1c);
}

// FUNCTION: 0x0001b3e8
void cancel_top_level_requesters(s32 keep)
{
    if (crequester.unknown2 != keep) {
        switch (crequester.unknown2) {
        case 9:
        case 11:
            close_crequester(crequester.unknown2);
            break;
        case 10:
            if (!crequester.unknown9) {
                clear_build_objects(othervar.ff);
                close_crequester(crequester.unknown2);
            }
            break;
        }
    }
}

// FUNCTION: 0x0001b474
Thing *allocate_thing(u32 reserve)
{
    Thing *slot;

    for (slot = thing + 1; slot < end_things - reserve; slot++) {
        if (slot->type == 0) {
            return slot;
        }
    }
    return 0;
}

// FUNCTION: 0x0001b4e0
s32 over_hotspot(s16 x, s16 y, s16 rx, s16 ry, s16 w, s16 h)
{
    return (x >= rx && x < rx + w) && (y >= ry && y < ry + h);
}

// FUNCTION: 0x0001b554
void draw_frame(const s16 x, const s16 y, const s16 w, const s16 h, const u16 color)
{
    fn_000c9be0(x, y, x + w, y, color);
    fn_000c9be0(x, y, x, y + h, color);
    fn_000c9be0(x + w, y, x + w, y + h, color);
    fn_000c9be0(x, y + h, x + w, y + h, color);
}

// FUNCTION: 0x0001b610
u8 hosp_play_smk(const u8 idx)
{
    u8 wasPlaying = lbDisplay.unknown4 != 0;
    u8 ok;

    ok = 1;
    if (wasPlaying) {
        LbScreenUnlock();
    }
    if (resvars.f1 == SCREEN_MODE_1 && resvars.f0 == SCREEN_MODE_1 && idx != 0x16 && idx != 0x17) {
        if (!change_res(SCREEN_MODE_13)) {
            reset_host();
            exit(1);
        }
        LbScreenClear(0);
        LbScreenSwap();
    }
    if (animation_table[idx].f28 > install_type) {
        sprintf(D_0015fde4, "%s%s", cd_install_path, animation_table[idx].name);
        check_cd_available(D_0015fde4);
    } else {
        fn_000bbff0(D_0015fde4, animation_table[idx].name);
    }
    if (LbFileExists(D_0015fde4)) {
        ok = play_smk(D_0015fde4, animation_table[idx].f32, animation_table[idx].f36);
    } else {
        ok = 0;
    }
    LbPackHeap();
    if (wasPlaying) {
        while (LbScreenLock() != 1) {
        }
    }
    lbDisplay.left_button = lbDisplay.rleft_button = 0;
    lbDisplay.right_button = lbDisplay.rright_button = 0;
    lbDisplay.mleft_button = lbDisplay.mright_button = 0;
    return ok;
}

// FUNCTION: 0x0001b7a4
void hosp_dos_printf(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(&__iob[1]);
}

// FUNCTION: 0x0001b7e0
void fn_0001b7e0(char *dst, s8 *src, u8 mode)
{
    if (mode == 0) {
        for (; *src; src++, dst++) {
            *dst = fn_0001b874(*src);
        }
        *dst = 0;
    } else if (mode == 1) {
        for (; *src; src++, dst++) {
            *dst = tolower(*src);
        }
        *dst = 0;
    }
}

// FUNCTION: 0x0001b874
int fn_0001b874(u8 ch)
{
    if (ch < 0x80) {
        return toupper(ch);
    }
    if (ch > 0xa5) {
        return ch;
    }
    switch (ch) {
    case 0x81:
        return 0x9a;
    case 0x82:
        return 0x90;
    case 0x83:
        return 0x41;
    case 0x84:
        return 0x8e;
    case 0x85:
        return 0x41;
    case 0x86:
        return 0x8f;
    case 0x87:
        return 0x80;
    case 0x88:
        return 0x45;
    case 0x89:
        return 0x45;
    case 0x8a:
        return 0x45;
    case 0x8b:
        return 0x49;
    case 0x8c:
        return 0x49;
    case 0x8d:
        return 0x49;
    case 0x93:
        return 0x4f;
    case 0x94:
        return 0x99;
    case 0x95:
        return 0x4f;
    case 0x96:
        return 0x55;
    case 0x97:
        return 0x55;
    case 0x98:
        return 0x59;
    case 0xa0:
        return 0x41;
    case 0xa1:
        return 0x49;
    case 0xa2:
        return 0x4f;
    case 0xa3:
        return 0x55;
    case 0xa4:
        return 0xa5;
    default:
        return ch;
    }
}

// FUNCTION: 0x0001bab4
void fn_0001bab4(u8 key)
{
    int y;
    int x;

    if (key < 12) {
        find_cell_at_point(window.unknown2eb, lbDisplay.graphics_screen_width / 2, lbDisplay.graphics_screen_height / 2);
        if (info.unknown5c < 0) {
            x = 0;
        } else if (info.unknown5c >= 0x80) {
            x = 0x7f;
        } else {
            x = info.unknown5c;
        }
        if (info.unknown60 >= 0x80) {
            y = 0x7f;
        } else if (info.unknown60 < 0) {
            y = 0;
        } else {
            y = info.unknown60;
        }
        key_mappos[key] = x + (y << 7);
    }
}

// FUNCTION: 0x0001bb8c
void recall_mappos(u8 i)
{
    if (i < 12) {
        if (key_mappos[i]) {
            goto_column_row_in_window(0, key_mappos[i] % 128, key_mappos[i] / 128);
        }
    }
}

// FUNCTION: 0x0001bc08
void setup_key_mappos(void)
{
    u8 i;

    for (i = 0; i < 12; i++) {
        key_mappos[i] = 0;
    }
}

// FUNCTION: 0x0001bc48
void hosp_fade_palette(u8 *palette, u8 steps, TbPaletteFadeFlag flag)
{
    if (statvar.unknownfd > 1) {
        LbPaletteFade(palette, 1, flag);
    } else {
        LbPaletteFade(palette, steps, flag);
    }
}

// FUNCTION: 0x0001bc94
void fn_0001bc94(u8 idx, u8 flag)
{
    char *name;
    char path[20];
    int size;

    name = resvars.f6_charpp[idx];

    if (resvars.f0 == SCREEN_MODE_13) {
        if (idx != loaded_requester) {
            fn_000bbff0(path, name);
            fn_000c93bf(path, ".tab");
            size = LbFileLengthRnc(path);
            if ((s32)end_button_sprites - (s32)button_sprite < size) {
                return;
            }
            if (LbFileLoadAt(path, button_sprite) != size) {
                return;
            }
            fn_000bbff0(path, name);
            fn_000c93bf(path, ".dat");
            size = LbFileLengthRnc(path);
            if ((s32)end_button_sprite_data - (s32)button_sprite_data < size) {
                return;
            }
            if (LbFileLoadAt(path, button_sprite_data) != size) {
                return;
            }
            LbSpriteSetup(button_sprite, end_button_sprites, button_sprite_data);
            loaded_requester = idx;
        }
    }
    if (flag) {
        OpenWindow(open_windows, resvars.f20, idx);
    }
}

// FUNCTION: 0x0001bdb8
void close_crequester(u8 i)
{
    remove_window(open_windows, resvars.f20 + i);
    crequester.unknown2 = 0x63;
    cancel_helpmsg();
}

// FUNCTION: 0x0001bdf8
void fn_0001bdf8(void)
{
    hosp_dos_printf("Copyright (c) 1997 Bullfrog Productions Ltd.\n");
    hosp_dos_printf("All rights reserved.\n");
    hosp_dos_printf("Theme Hospital v%d \n", 3);
    if (D_001581ec) {
        hosp_dos_printf("Recording..%d..\n", D_001581ef);
    }
    if (D_001581ed) {
        hosp_dos_printf("PlayBack..%d..\n", D_001581ef);
    }
}

// FUNCTION: 0x0001be78
void fn_0001be78(u8 dir)
{
    switch (dir) {
    case 0:
        D_0015fe74++;
        break;
    case 1:
        D_0015fe74--;
        break;
    default:
        break;
    }
    hosp_dos_printf("MOUSE_POINTER %d\n", D_0015fe74);
    set_mouse_sprite(D_0015fe74);
}

// FUNCTION: 0x0001bee4
void fn_0001bee4(void)
{
    if (face_file != -1) {
        LbFileClose(face_file);
        face_file = -1;
    }
    if (resvars.f0 == SCREEN_MODE_1 || othervar.fa8d) {
        face_file = LbFileOpen((char *)lores_faces, 2);
        if (face_file == -1) {
            D_0015f982 = 9;
            sub_error_number = 8;
            reset_host();
            exit(1);
        }
        resvars.f60_u32 = D_0014bcc4;
        resvars.f64_u32 = D_0014bcd0;
        resvars.f68_u32 = D_0014bcd8;
        resvars.f6c_u32 = D_0014bce0;
        resvars.f58_u32 = D_0014bce4;
        resvars.f5c_u32 = D_0014bce8;
    } else if (resvars.f0 == SCREEN_MODE_13) {
        face_file = LbFileOpen((char *)hires_faces, 2);
        if (face_file == -1) {
            D_0015f982 = 9;
            sub_error_number = 7;
            reset_host();
            exit(1);
        }
        resvars.f60_u32 = D_0013c60c;
        resvars.f64_u32 = D_0013c618;
        resvars.f68_u32 = D_0013c620;
        resvars.f6c_u32 = D_0013c628;
        resvars.f58_u32 = D_0013c62c;
        resvars.f5c_u32 = D_0013c630;
    }
}
