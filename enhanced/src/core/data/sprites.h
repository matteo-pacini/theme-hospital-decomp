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

// One sprite of a Bullfrog .TAB/.DAT pair.
struct Sprite {
    const uint8_t *data = nullptr;
    SpriteFormat format = SpriteFormat::Rle;
    int width = 0;
    int height = 0;

    bool empty() const { return !data || width == 0 || height == 0; }
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

  private:
    struct Entry {
        uint32_t offset;
        uint8_t width;
        uint8_t height;
    };
    std::vector<Entry> sprites_;
    std::vector<uint8_t> data_;
    SpriteFormat format_ = SpriteFormat::Rle;
};

} // namespace th
