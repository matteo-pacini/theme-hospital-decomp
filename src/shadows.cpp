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
extern "C" Thing *search_mapwho(Map *t, u16 kind);

// Returns 1 if a door stands on tile facing a direction of parity `parity`
// (facing % 2), else 0.
// FUNCTION: 0x000b0630
u8 door(u16 tile, u8 parity)
{
    Map *cell = &map[(u16)tile];
    Thing *door = search_mapwho(cell, THING_DOOR);

    if (door == 0) {
        return 0;
    } else if (door->facing % 2 == parity) {
        return 1;
    } else if (door->facing % 2 != parity) {
        return 0;
    } else {
        return 0;
    }
}

// Recomputes the wall shadow bits of every map cell's floor (0x10, 0x20,
// 0x40) from the walls and doors that cast them.
// FUNCTION: 0x000b06d0
void add_static_shadows(void)
{
    u32 i;

    for (i = 0; i < 0x4000; i++) {
        map[i].floor &= 0x8f;
        if ((map[i].wall_w && (blocks[map[i].wall_w].width & 1) && !(blocks[map[i].wall_w].width & 0x40) &&
             !(blocks[map[i].wall_w].width & 0x80)) ||
            door((u16)i, 1)) {
            map[i].floor |= 0x40;
            if ((!map[i + 128].wall_w || (blocks[map[i + 128].wall_w].width & 0xe0)) && !door((u16)(i + 0x80), 1)) {
                map[i].floor &= 0xbf;
                map[i].floor |= 0x10;
            }
            if (((!map[i - 128].wall_w && !door((u16)(i - 0x80), 1)) || (blocks[map[i - 128].wall_w].width & 0x60)) &&
                ((!map[i].wall_n && !door((u16)(i - 0x80), 1)) || (blocks[map[i].wall_n].width & 0x60))) {
                map[i - 128].floor |= 0x40;
                map[i - 128 - 1].floor |= 0x40;
            }
            map[i].floor |= 0x20;
        }
    }
}

// Builds the shadowed versions of the floor and wall blocks (a full and a
// half shadow over the player's interior floor and wall, and over four room
// floors) used to draw shadows.
// FUNCTION: 0x000b0928
void precalculate_shadow_blocks(void)
{
    u32 i;

    i = 0;
    reset_precalculated_shadows();
    for (; i < 0x4000; i++) {
        if (map[i].wall_n && map[i].wall_w && ((1 << player) & map[i].owner_flags)) {
            InteriorTileFull.block = map[i].overlay;
            InteriorTileFull.sprite.width = blocks[map[i].overlay].info->width;
            InteriorTileFull.sprite.height = blocks[map[i].overlay].info->height;
            create_overlay_sprite(blocks[map[i].overlay].info, blocks[73].info, &InteriorTileFull.sprite);
            InteriorTileHalf.block = map[i].overlay;
            InteriorTileHalf.sprite.width = blocks[map[i].overlay].info->width;
            InteriorTileHalf.sprite.height = blocks[map[i].overlay].info->height;
            create_overlay_sprite(blocks[map[i].overlay].info, blocks[74].info, &InteriorTileHalf.sprite);
            InteriorWallHalf.block = map[i].wall_n;
            InteriorWallHalf.sprite.width = blocks[map[i].wall_n].info->width;
            InteriorWallHalf.sprite.height = blocks[map[i].wall_n].info->height;
            create_overlay_sprite(blocks[map[i].wall_n].info, blocks[150].info, &InteriorWallHalf.sprite);
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

// Writes into dst the RLE sprite src with sprite ovr's opaque pixels laid
// over it through the ghost table. Rows of ovr are at most 64 pixels; dst
// must have room for src's data.
// FUNCTION: 0x000b0d40
void create_overlay_sprite(const TbSprite *src, const TbSprite *ovr, const TbSprite *dst)
{
    u8 line[64];
    s8 *ovr_data;
    u8 *line_ptr;
    s8 *src_data;
    s8 *dst_data;
    u8 row;
    s8 run;

    row = 0;
    src_data = (s8 *)src->data;
    dst_data = (s8 *)dst->data;
    ovr_data = (s8 *)ovr->data;
    while (++row <= src->height) {
        if (row <= ovr->height) {
            line_ptr = line;
            while (*ovr_data != 0) {
                if (*ovr_data < 0) {
                    LbMemorySet(line_ptr, 0, -*ovr_data);
                    line_ptr -= *ovr_data;
                    ovr_data++;
                } else {
                    run = *ovr_data++;
                    while (run-- > 0)
                        *line_ptr++ = *ovr_data++;
                }
            }
            if (row < src->height)
                ovr_data++;
            LbMemorySet(line_ptr, 0, line + 64 - line_ptr);
            line_ptr = line;
            while (*src_data != 0) {
                if (*src_data > 0) {
                    run = *src_data;
                    *dst_data++ = *src_data++;
                    while (run-- > 0) {
                        if (*line_ptr == 0) {
                            *dst_data++ = *src_data++;
                            line_ptr++;
                        } else {
                            *dst_data++ = lbDisplay.ghost[(*line_ptr++ << 8) + *src_data++];
                        }
                    }
                } else {
                    line_ptr -= *src_data;
                    *dst_data++ = *src_data++;
                }
            }
            if (row < src->height)
                *dst_data++ = *src_data++;
            else
                *dst_data = *src_data;
        } else {
            while (*src_data != 0)
                *dst_data++ = *src_data++;
            if (row <= src->height)
                *dst_data++ = *src_data++;
            else
                *dst_data = *src_data;
        }
    }
}

// Points the precalculated shadow sprites at their buffers. Called once at
// startup.
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

// Clears the shadow buffers and the precalculated shadow sprites.
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
