#include "hosp.h"

/* A precalculated shadow sprite and the block it shades. */
struct PrecalcShadows {
    TbSprite sprite;
    u16 block;
};

extern PrecalcShadows InteriorTileFull;
extern PrecalcShadows InteriorTileHalf;
extern PrecalcShadows InteriorWallHalf;
extern PrecalcShadows RoomTileFull[];
extern PrecalcShadows RoomTileHalf[];
extern s8 ShadowBuffer[][85];
extern s8 InteriorTileFullData[][64];
extern s8 InteriorTileHalfData[][64];
extern s8 InteriorWallHalfData[][85];
extern s8 RoomTileFullData[][32][64];
extern s8 RoomTileHalfData[][32][64];

void reset_precalculated_shadows(void);
void create_overlay_sprite(const TbSprite *src, const TbSprite *ovr, const TbSprite *dst);
extern "C" Thing *fn_00019aac(Map *t, u16 kind);

// FUNCTION: 0x000b0630
u8 fn_000b0630(u16 tile, u8 parity)
{
    Map *cell = &map[(u16)tile];
    Thing *e = fn_00019aac(cell, 8);

    if (e == 0) {
        return 0;
    } else if (e->unknown17 % 2 == parity) {
        return 1;
    } else if (e->unknown17 % 2 != parity) {
        return 0;
    } else {
        return 0;
    }
}

// FUNCTION: 0x000b06d0
void add_static_shadows(void)
{
    u32 i;

    for (i = 0; i < 0x4000; i++) {
        map[i].floor &= 0x8f;
        if ((map[i].unknown4 && (blocks[map[i].unknown4].width & 1) && !(blocks[map[i].unknown4].width & 0x40)
             && !(blocks[map[i].unknown4].width & 0x80))
            || fn_000b0630((u16)i, 1)) {
            map[i].floor |= 0x40;
            if ((!map[i + 128].unknown4 || (blocks[map[i + 128].unknown4].width & 0xe0)) && !fn_000b0630((u16)(i + 0x80), 1)) {
                map[i].floor &= 0xbf;
                map[i].floor |= 0x10;
            }
            if (((!map[i - 128].unknown4 && !fn_000b0630((u16)(i - 0x80), 1)) || (blocks[map[i - 128].unknown4].width & 0x60))
                && ((!map[i].unknown3 && !fn_000b0630((u16)(i - 0x80), 1)) || (blocks[map[i].unknown3].width & 0x60))) {
                map[i - 128].floor |= 0x40;
                map[i - 128 - 1].floor |= 0x40;
            }
            map[i].floor |= 0x20;
        }
    }
}

// FUNCTION: 0x000b0928
void precalculate_shadow_blocks(void)
{
    u32 i;

    i = 0;
    reset_precalculated_shadows();
    for (; i < 0x4000; i++) {
        if (map[i].unknown3 && map[i].unknown4 && ((1 << player) & map[i].unknown7)) {
            InteriorTileFull.block = map[i].overlay;
            InteriorTileFull.sprite.width = blocks[map[i].overlay].info->width;
            InteriorTileFull.sprite.height = blocks[map[i].overlay].info->height;
            create_overlay_sprite(blocks[map[i].overlay].info, blocks[73].info, &InteriorTileFull.sprite);
            InteriorTileHalf.block = map[i].overlay;
            InteriorTileHalf.sprite.width = blocks[map[i].overlay].info->width;
            InteriorTileHalf.sprite.height = blocks[map[i].overlay].info->height;
            create_overlay_sprite(blocks[map[i].overlay].info, blocks[74].info, &InteriorTileHalf.sprite);
            InteriorWallHalf.block = map[i].unknown3;
            InteriorWallHalf.sprite.width = blocks[map[i].unknown3].info->width;
            InteriorWallHalf.sprite.height = blocks[map[i].unknown3].info->height;
            create_overlay_sprite(blocks[map[i].unknown3].info, blocks[150].info, &InteriorWallHalf.sprite);
            break;
        }
    }
    RoomTileFull[0].block = 0x11;
    RoomTileFull[0].sprite.width = blocks[0x11].info->width;
    RoomTileFull[0].sprite.height = blocks[0x11].info->height;
    create_overlay_sprite(blocks[0x11].info, blocks[73].info, &RoomTileFull[0].sprite);
    RoomTileHalf[0].block = 0x11;
    RoomTileHalf[0].sprite.width = blocks[0x11].info->width;
    RoomTileHalf[0].sprite.height = blocks[0x11].info->height;
    create_overlay_sprite(blocks[0x11].info, blocks[74].info, &RoomTileHalf[0].sprite);
    RoomTileFull[1].block = 0x12;
    RoomTileFull[1].sprite.width = blocks[0x12].info->width;
    RoomTileFull[1].sprite.height = blocks[0x12].info->height;
    create_overlay_sprite(blocks[0x12].info, blocks[73].info, &RoomTileFull[1].sprite);
    RoomTileHalf[1].block = 0x12;
    RoomTileHalf[1].sprite.width = blocks[0x12].info->width;
    RoomTileHalf[1].sprite.height = blocks[0x12].info->height;
    create_overlay_sprite(blocks[0x12].info, blocks[74].info, &RoomTileHalf[1].sprite);
    RoomTileFull[2].block = 0x10;
    RoomTileFull[2].sprite.width = blocks[0x10].info->width;
    RoomTileFull[2].sprite.height = blocks[0x10].info->height;
    create_overlay_sprite(blocks[0x10].info, blocks[73].info, &RoomTileFull[2].sprite);
    RoomTileHalf[2].block = 0x10;
    RoomTileHalf[2].sprite.width = blocks[0x10].info->width;
    RoomTileHalf[2].sprite.height = blocks[0x10].info->height;
    create_overlay_sprite(blocks[0x10].info, blocks[74].info, &RoomTileHalf[2].sprite);
    RoomTileFull[3].block = 0x14;
    RoomTileFull[3].sprite.width = blocks[0x14].info->width;
    RoomTileFull[3].sprite.height = blocks[0x14].info->height;
    create_overlay_sprite(blocks[0x14].info, blocks[73].info, &RoomTileFull[3].sprite);
    RoomTileHalf[3].block = 0x14;
    RoomTileHalf[3].sprite.width = blocks[0x14].info->width;
    RoomTileHalf[3].sprite.height = blocks[0x14].info->height;
    create_overlay_sprite(blocks[0x14].info, blocks[74].info, &RoomTileHalf[3].sprite);
}

// FUNCTION: 0x000b0d40
void create_overlay_sprite(const TbSprite *src, const TbSprite *ovr, const TbSprite *dst)
{
    u8 line[64];
    s8 *o;
    u8 *p;
    s8 *s;
    s8 *d;
    u8 row;
    s8 n;

    row = 0;
    s = (s8 *)src->data;
    d = (s8 *)dst->data;
    o = (s8 *)ovr->data;
    while (++row <= src->height) {
        if (row <= ovr->height) {
            p = line;
            while (*o != 0) {
                if (*o < 0) {
                    LbMemorySet(p, 0, -*o);
                    p -= *o;
                    o++;
                } else {
                    n = *o++;
                    while (n-- > 0)
                        *p++ = *o++;
                }
            }
            if (row < src->height)
                o++;
            LbMemorySet(p, 0, line + 64 - p);
            p = line;
            while (*s != 0) {
                if (*s > 0) {
                    n = *s;
                    *d++ = *s++;
                    while (n-- > 0) {
                        if (*p == 0) {
                            *d++ = *s++;
                            p++;
                        } else {
                            *d++ = lbDisplay.ghost[(*p++ << 8) + *s++];
                        }
                    }
                } else {
                    p -= *s;
                    *d++ = *s++;
                }
            }
            if (row < src->height)
                *d++ = *s++;
            else
                *d = *s;
        } else {
            while (*s != 0)
                *d++ = *s++;
            if (row <= src->height)
                *d++ = *s++;
            else
                *d = *s;
        }
    }
}

// FUNCTION: 0x000b0f80
void initialise_shadows(void)
{
    u8 i;

    InteriorTileFull.sprite.data = (u32)InteriorTileFullData;
    InteriorTileHalf.sprite.data = (u32)InteriorTileHalfData;
    InteriorWallHalf.sprite.data = (u32)InteriorWallHalfData;
    for (i = 0; i < 4; i++) {
        RoomTileFull[i].sprite.data = (u32)RoomTileFullData[i];
        RoomTileHalf[i].sprite.data = (u32)RoomTileHalfData[i];
    }
}

// FUNCTION: 0x000b1008
void reset_precalculated_shadows(void)
{
    u8 i;

    LbMemorySet(ShadowBuffer, 0, 0xaf5);
    LbMemorySet((void *)InteriorTileFull.sprite.data, 0, 0x800);
    LbMemorySet((void *)InteriorTileHalf.sprite.data, 0, 0x800);
    LbMemorySet((void *)InteriorWallHalf.sprite.data, 0, 0xaa0);
    for (i = 0; i < 4; i++) {
        LbMemorySet((void *)RoomTileFull[i].sprite.data, 0, 0x800);
        LbMemorySet((void *)RoomTileHalf[i].sprite.data, 0, 0x800);
    }
}
