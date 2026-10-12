#pragma once

#include "core/data/level.h"
#include "core/data/sprites.h"

#include <array>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace th
{

// MapCell.litter_shadow bits for the shadows cast by walls and doors.
constexpr uint8_t kShadowFloorHalf = 0x10;
constexpr uint8_t kShadowWallN = 0x20;
constexpr uint8_t kShadowFloorFull = 0x40;

// The facing (0-3) of the door standing on cell `cell`, or -1 if there is
// none.
using DoorFacing = std::function<int(int cell)>;

// Recomputes the wall shadow bits of every cell, as the game's
// add_static_shadows (src/shadows.cpp) does after a level's map is built:
// a cell with a shadow-casting west wall or a door of odd facing gets a
// shaded north wall and a full floor shadow (half if the cell one row on has
// no west wall or door to continue it), and may shade the floors of the cell
// one row back and the one before that. Other litter_shadow bits are kept. Doors are
// things in the game, not map blocks; without `door_facing` there are none.
void add_static_shadows(std::array<MapCell, kMapCells> &map, const DoorFacing &door_facing = {});

// A sprite and how far above the bottom of its cell's diamond its top is
// drawn (the draw is at base_y - lift, not always base_y - sprite.height).
struct ShadedSprite {
    Sprite sprite;
    int lift = 0;
};

// The floor and wall sprites with shadows laid over them through the blend
// table, as the game's precalculate_shadow_blocks and create_overlay_sprite
// make them, and the choice of sprite per cell of
// draw_all_mk7_selective_sprites_wscreen (src/engine.cpp).
class ShadowSprites
{
  public:
    // `blocks` is the VBLK sheet and `ghost` the 256 x 256 blend table the
    // game draws the map with, qdata/ghost66.dat (66% blends of the game
    // palette), only read here; `blocks` must outlive this object.
    ShadowSprites(const SpriteSheet &blocks, std::span<const uint8_t> ghost);

    // The sprite for `cell`'s floor given its shadow bits.
    ShadedSprite floor(const MapCell &cell) const;
    // The sprite for `cell`'s north wall given its shadow bits; empty if it
    // has none. Shaded or not, it is drawn at its own height.
    ShadedSprite wall_n(const MapCell &cell) const;

  private:
    struct Shaded {
        std::vector<uint8_t> data;
        int width = 0;
        int height = 0;
        // The same sprite from the upscaled blocks, when the sheet has them
        // (HiSprite coding); empty otherwise.
        std::vector<uint8_t> hi;
        int hi_width = 0;
        int hi_height = 0;
        int factor = 1;
    };

    static Shaded make_shaded(const Sprite &s, const Sprite &shadow, std::span<const uint8_t> ghost);
    Sprite base(uint8_t id) const;
    Sprite shaded(const std::vector<Shaded> &sprites, uint8_t id) const;

    const SpriteSheet *blocks_;
    // Indexed by block id: under block 73 (full floor shadow), block 74
    // (half) and block 150 (wall).
    std::vector<Shaded> full_;
    std::vector<Shaded> half_;
    std::vector<Shaded> wall_;
};

} // namespace th
