#pragma once

#include <cstdint>
#include <vector>

namespace th
{

// How a sheet's pixel rows are coded.
enum class SpriteFormat {
    // LbSpriteDraw (map blocks, game sprites): a positive byte n is followed
    // by n palette indices, a negative byte skips -n transparent pixels, 0
    // ends the row.
    Rle,
    // reqspr_draw (panel, screens): 0 ends the row, 1-63 copy that many
    // pixels, 64-127 fill c - 60 pixels with the next byte, 128-191 skip
    // c & 63, 192-254 fill (c & 63) + 68, and 255 is followed by a count and
    // the colour to fill with.
    Req,
};

// A sprite's pixels `factor` times larger, made by the install-time
// upscaling and snapped back to the game's palette. Each row is a u16 run
// count (little-endian) followed by that many runs of a u16 skip of
// transparent pixels, a u16 length and that many palette indices. Pixels
// after a row's last run are transparent.
struct HiSprite {
    const uint8_t *data = nullptr;
    // In pixels of the larger size: factor times the 1x sprite's.
    int width = 0;
    int height = 0;
    int factor = 1;

    bool empty() const { return !data || width == 0 || height == 0; }
};

// One sprite of a Bullfrog .TAB/.DAT pair, with its upscaled version when the
// sheet has one.
struct Sprite {
    const uint8_t *data = nullptr;
    SpriteFormat format = SpriteFormat::Rle;
    int width = 0;
    int height = 0;
    HiSprite hi = {};

    bool empty() const { return !data || width == 0 || height == 0; }
};

// Walks `sprite`'s upscaled pixels row by row, calling put(row, col, colour)
// for each opaque pixel in the larger sprite's own coordinates.
template <typename Put> void decode_hires(const HiSprite &sprite, Put &&put)
{
    auto u16 = [](const uint8_t *p) { return int(p[0] | p[1] << 8); };
    const uint8_t *p = sprite.data;
    for (int row = 0; row < sprite.height; row++) {
        int runs = u16(p);
        p += 2;
        int col = 0;
        for (int r = 0; r < runs; r++) {
            col += u16(p);
            int length = u16(p + 2);
            p += 4;
            for (int i = 0; i < length; i++)
                put(row, col++, p[i]);
            p += length;
        }
    }
}

// The upscaled sprites of a sheet, by the sheet's sprite index.
struct HiresSheet {
    struct Entry {
        uint32_t offset = 0;
        uint16_t width = 0; // 0: this sprite has none
        uint16_t height = 0;
    };
    int factor = 1;
    std::vector<Entry> entries;
    std::vector<uint8_t> data;
};

// A sprite sheet: the .TAB index (6 bytes per sprite: little-endian u32
// offset into the .DAT, width, height) and the .DAT pixel data, both
// unpacked.
class SpriteSheet
{
  public:
    bool load(std::vector<uint8_t> tab, std::vector<uint8_t> dat, SpriteFormat format = SpriteFormat::Rle);

    size_t size() const { return sprites_.size(); }
    // The sprite at `index`, or an empty one if out of range.
    Sprite at(size_t index) const;

    // Gives the sprites their upscaled versions (Sprite::hi); `hires` holds
    // one entry per sprite of this sheet.
    void attach_hires(HiresSheet hires);
    const HiresSheet &hires() const { return hires_; }

  private:
    struct Entry {
        uint32_t offset;
        uint8_t width;
        uint8_t height;
    };
    std::vector<Entry> sprites_;
    std::vector<uint8_t> data_;
    SpriteFormat format_ = SpriteFormat::Rle;
    HiresSheet hires_;
};

} // namespace th
