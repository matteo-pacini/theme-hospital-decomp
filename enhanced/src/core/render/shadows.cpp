#include "core/render/shadows.h"

#include "core/data/blocks.h"

namespace th
{

namespace
{

constexpr uint8_t kFullShadowBlock = 73;
constexpr uint8_t kHalfShadowBlock = 74;
constexpr uint8_t kWallShadowBlock = 150;

// The game's create_overlay_sprite: `src` with the opaque pixels of `ovr`
// (aligned on the top-left corner) mixed into it through `ghost`, entry
// [ovr << 8 | src]. The result has the same runs as `src`.
std::vector<uint8_t> overlay(const Sprite &src, const Sprite &ovr, std::span<const uint8_t> ghost)
{
    std::vector<uint8_t> out;
    if (src.empty())
        return out;
    const uint8_t *s = src.data;
    const uint8_t *o = ovr.data;
    for (int row = 0; row < src.height; row++) {
        // The game's line is 64 bytes and overruns it on wider sprites; it
        // is made wide enough for any sprite instead.
        std::array<uint8_t, 256> line{};
        if (o && row < ovr.height) {
            size_t col = 0;
            for (int8_t n; (n = static_cast<int8_t>(*o++)) != 0;) {
                if (n < 0) {
                    col += size_t(-n);
                    continue;
                }
                for (int i = 0; i < n; i++, col++)
                    if (col < line.size())
                        line[col] = o[i];
                o += n;
            }
        }
        size_t col = 0;
        for (int8_t n; (n = static_cast<int8_t>(*s)) != 0;) {
            out.push_back(*s++);
            if (n < 0) {
                col += size_t(-n);
                continue;
            }
            for (int i = 0; i < n; i++, col++, s++) {
                uint8_t shade = col < line.size() ? line[col] : 0;
                out.push_back(shade ? ghost[size_t(shade) << 8 | *s] : *s);
            }
        }
        out.push_back(*s++);
    }
    return out;
}

} // namespace

void add_static_shadows(std::array<MapCell, kMapCells> &map, const DoorFacing &door_facing)
{
    // The game reads and writes past the map's ends here; outside it there
    // is no wall, no door and nothing to shade.
    auto in_map = [](long i) { return i >= 0 && i < kMapCells; };
    auto wall_w = [&](long i) -> uint8_t { return in_map(i) ? map[size_t(i)].wall_w : 0; };
    // door() at 0xb0630: a door on the cell with an odd facing.
    auto door = [&](long i) {
        if (!door_facing || !in_map(i))
            return false;
        int facing = door_facing(int(i));
        return facing >= 0 && facing % 2 == 1;
    };
    auto shade = [&](long i, uint8_t bit) {
        if (in_map(i))
            map[size_t(i)].litter_shadow |= bit;
    };

    for (long i = 0; i < kMapCells; i++) {
        MapCell &cell = map[size_t(i)];
        cell.litter_shadow &= 0x8f;
        uint8_t w = block_flags(cell.wall_w);
        if (!((cell.wall_w && (w & 0x01) && !(w & 0x40) && !(w & 0x80)) || door(i)))
            continue;
        cell.litter_shadow |= kShadowFloorFull;
        if ((!wall_w(i + kMapSize) || (block_flags(wall_w(i + kMapSize)) & 0xe0)) && !door(i + kMapSize)) {
            cell.litter_shadow &= ~kShadowFloorFull;
            cell.litter_shadow |= kShadowFloorHalf;
        }
        // The game tests the door north of the cell for both the west wall
        // north of it and the cell's own north wall.
        if (((!wall_w(i - kMapSize) && !door(i - kMapSize)) || (block_flags(wall_w(i - kMapSize)) & 0x60)) &&
            ((!cell.wall_n && !door(i - kMapSize)) || (block_flags(cell.wall_n) & 0x60))) {
            shade(i - kMapSize, kShadowFloorFull);
            shade(i - kMapSize - 1, kShadowFloorFull);
        }
        cell.litter_shadow |= kShadowWallN;
    }
}

ShadowSprites::ShadowSprites(const SpriteSheet &blocks, std::span<const uint8_t> ghost, const MapCell *map, int player)
    : blocks_(&blocks), full_(kBlockCount), half_(kBlockCount), wall_(kBlockCount)
{
    // The game precalculates the player's interior floor and wall and four
    // room floors and builds any other shaded block as it draws it; both give
    // the same pixels, so every block is built here.
    const Sprite full = base(kFullShadowBlock);
    const Sprite half = base(kHalfShadowBlock);
    const Sprite wall = base(kWallShadowBlock);
    for (int id = 0; id < kBlockCount; id++) {
        Sprite s = base(uint8_t(id));
        full_[size_t(id)] = {overlay(s, full, ghost), s.width, s.height};
        half_[size_t(id)] = {overlay(s, half, ghost), s.width, s.height};
        wall_[size_t(id)] = {overlay(s, wall, ghost), s.width, s.height};
    }
    for (int i = 0; i < kMapCells; i++) {
        const MapCell &cell = map[i];
        if (cell.wall_n && cell.wall_w && (cell.owner_flags & (1 << player))) {
            interior_wall_ = cell.wall_n;
            break;
        }
    }
}

Sprite ShadowSprites::base(uint8_t id) const
{
    int index = block_sprite(id);
    return index < 0 ? Sprite{} : blocks_->at(size_t(index));
}

Sprite ShadowSprites::shaded(const std::vector<Shaded> &sprites, uint8_t id) const
{
    if (id >= sprites.size() || sprites[id].data.empty())
        return {};
    const Shaded &s = sprites[id];
    return {s.data.data(), SpriteFormat::Rle, s.width, s.height};
}

ShadedSprite ShadowSprites::floor(const MapCell &cell) const
{
    Sprite s;
    if (cell.litter_shadow & kShadowFloorFull)
        s = shaded(full_, cell.floor_block);
    else if (cell.litter_shadow & kShadowFloorHalf)
        s = shaded(half_, cell.floor_block);
    else
        s = base(cell.floor_block);
    return {s, s.height};
}

ShadedSprite ShadowSprites::wall_n(const MapCell &cell) const
{
    if (!cell.wall_n)
        return {};
    if (!(cell.litter_shadow & kShadowWallN)) {
        Sprite s = base(cell.wall_n);
        return {s, s.height};
    }
    Sprite s = shaded(wall_, cell.wall_n);
    if (cell.wall_n == interior_wall_)
        return {s, s.height};
    // The game lifts other shaded walls by the shadow's height, not theirs.
    return {s, base(kWallShadowBlock).height};
}

} // namespace th
