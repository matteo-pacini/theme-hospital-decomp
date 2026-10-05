#pragma once

#include <array>
#include <cstdint>

namespace th
{

// Tile sprite (index into VBLK-0) of each floor and wall block id, from the
// game's blocks_backup (src/blocks.cpp). setup_blocks (src/main.cpp) offsets
// the entries from block 0x50 on by 0x51 sprites.
inline constexpr std::array<uint8_t, 0xec> kBlockSprites = {
    1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19, 20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41, 42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63, 64,  65,  66,
    67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  1,   2,   3,   4,   5,  6,   7,   8,
    9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27, 28,  29,  30,
    31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  51, 52,  55,  56,
    59,  60,  0,   0,   0,   0,   0,   0,   61,  62,  0,   0,   0,   0,   61,  62,  132, 133, 75, 123, 124, 125,
    126, 127, 128, 129, 130, 131, 98,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 98, 98,  99,  99,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 0,  49,  50,  53,
    54,  57,  58,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  0,   76,  77,  78,  79,  80, 81,  82,  83,
    134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
};

// Number of block ids the game defines (blocks[] before its terminator).
inline constexpr int kBlockCount = 232;

// The block drawn for cells outside the 128x128 map.
inline constexpr uint8_t kOffMapBlock = 4;

// VBLK-0 sprite index of block `id`, or -1 if `id` is not a block.
inline int block_sprite(uint8_t id)
{
    if (id >= kBlockCount)
        return -1;
    int sprite = kBlockSprites[id];
    return id >= 0x50 ? sprite + 0x51 : sprite;
}

// The flags of each block id, from the game's blocks[] (src/blocks.cpp).
// add_static_shadows tests bits 0x01, 0x20, 0x40 and 0x80 of wall blocks;
// building tests 0x04 (half-height walls) and 0x10 of walls, and floors
// whose flags are exactly 0x20 (the blueprint floors).
inline constexpr std::array<uint8_t, kBlockCount> kBlockFlags = {
    0,  0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32, 32, 32,
    32, 32, 32, 32, 32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,
    0,  0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,
    0,  0,  1,  1,  1,   1,   1,   1,   1,   1,   1,   1,   1,  1,  1,  1,  1,  1,  5,  5,  5,  5,  5, 5,  5,  5,
    5,  5,  5,  5,  5,   5,   5,   5,   17,  17,  17,  17,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 3, 3,  3,  3,
    3,  3,  3,  3,  0,   0,   0,   0,   0,   0,   17,  17,  65, 64, 65, 64, 17, 17, 0,  0,  1,  0,  0, 0,  0,  0,
    8,  8,  0,  0,  8,   8,   8,   8,   8,   8,   8,   8,   8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 8,  8,  8,
    8,  8,  8,  8,  8,   8,   8,   8,   8,   8,   8,   8,   8,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, 5,  5,  5,
    5,  5,  5,  0,  133, 133, 133, 133, 133, 133, 133, 133, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,
};

// The flags of block `id`, 0 if it is not a block.
inline uint8_t block_flags(uint8_t id)
{
    return id < kBlockCount ? kBlockFlags[id] : 0;
}

} // namespace th
