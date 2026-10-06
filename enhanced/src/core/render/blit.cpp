#include "core/render/blit.h"

#include "core/render/iso.h"

#include <algorithm>
#include <vector>

namespace th
{

namespace
{

// Decodes one RLE row starting at `p`, calling put(row, col, colour) for each
// opaque pixel; returns the start of the next row.
template <typename Put> const uint8_t *decode_rle_row(const uint8_t *p, int row, Put &put)
{
    int col = 0;
    for (int8_t n; (n = static_cast<int8_t>(*p++)) != 0;) {
        if (n < 0) {
            col -= n;
            continue;
        }
        for (int i = 0; i < n; i++)
            put(row, col++, p[i]);
        p += n;
    }
    return p;
}

// As decode_rle_row, for the packed format's runs and fills.
template <typename Put> const uint8_t *decode_packed_row(const uint8_t *p, int row, Put &put)
{
    int col = 0;
    for (uint8_t c; (c = *p++) != 0;) {
        int fill;
        if (c < 0x40) {
            for (int i = 0; i < c; i++)
                put(row, col++, p[i]);
            p += c;
            continue;
        } else if (c < 0x80) {
            fill = c - 60;
        } else if (c < 0xc0) {
            col += c & 0x3f;
            continue;
        } else if (c < 0xff) {
            fill = (c & 0x3f) + 68;
        } else {
            fill = *p++;
        }
        uint8_t colour = *p++;
        for (int i = 0; i < fill; i++)
            put(row, col++, colour);
    }
    return p;
}

// Walks `sprite` row by row, calling put(row, col, colour) for each opaque
// pixel in the sprite's own coordinates.
template <typename Put> void decode(const Sprite &sprite, Put &&put)
{
    const uint8_t *p = sprite.data;
    for (int row = 0; row < sprite.height; row++) {
        if (sprite.format == SpriteFormat::Rle)
            p = decode_rle_row(p, row, put);
        else
            p = decode_packed_row(p, row, put);
    }
}

} // namespace

void draw_sprite(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags, const uint8_t *ghost)
{
    if (sprite.empty())
        return;
    const Rect &clip = dst.clip();
    if (x >= clip.x + clip.w || y >= clip.y + clip.h || x + sprite.width <= clip.x || y + sprite.height <= clip.y)
        return;
    decode(sprite, [&](int row, int col, uint8_t colour) {
        int dx = (flags & kDrawFlipX) ? x + sprite.width - 1 - col : x + col;
        int dy = (flags & kDrawFlipY) ? y + sprite.height - 1 - row : y + row;
        if (dx < clip.x || dx >= clip.x + clip.w || dy < clip.y || dy >= clip.y + clip.h)
            return;
        uint8_t &out = dst.row(dy)[dx];
        if (ghost && (flags & kDrawGhost))
            out = ghost[colour << 8 | out];
        else if (ghost && (flags & kDrawGhostUnder))
            out = ghost[out << 8 | colour];
        else
            out = colour;
    });
}

namespace
{

// A pixel of colour `colour` drawn over `out`, see-through as `flags` say.
void put_pixel(uint8_t &out, uint8_t colour, unsigned flags, const uint8_t *ghost)
{
    if (ghost && (flags & kDrawGhost))
        out = ghost[colour << 8 | out];
    else if (ghost && (flags & kDrawGhostUnder))
        out = ghost[out << 8 | colour];
    else
        out = colour;
}

} // namespace

void draw_sprite_scaled(Surface &dst, Rect box, const Sprite &sprite, unsigned flags, const uint8_t *ghost)
{
    const int x = box.x, y = box.y;
    const Point size = {box.w, box.h};
    if (sprite.empty() || size.x <= 0 || size.y <= 0)
        return;
    // The sprite's pixels, -1 where transparent.
    std::vector<int> pixels(size_t(sprite.width) * size_t(sprite.height), -1);
    decode(sprite, [&](int row, int col, uint8_t colour) {
        if (col < sprite.width)
            pixels[size_t(row) * size_t(sprite.width) + size_t(col)] = colour;
    });
    const Rect &clip = dst.clip();
    for (int dy = std::max(0, clip.y - y); dy < size.y && y + dy < clip.y + clip.h; dy++) {
        int row = dy * sprite.height / size.y;
        if (flags & kDrawFlipY)
            row = sprite.height - 1 - row;
        for (int dx = std::max(0, clip.x - x); dx < size.x && x + dx < clip.x + clip.w; dx++) {
            int col = dx * sprite.width / size.x;
            if (flags & kDrawFlipX)
                col = sprite.width - 1 - col;
            int colour = pixels[size_t(row) * size_t(sprite.width) + size_t(col)];
            if (colour >= 0)
                put_pixel(dst.row(y + dy)[x + dx], uint8_t(colour), flags, ghost);
        }
    }
}

void draw_sprite_one_colour(Surface &dst, int x, int y, const Sprite &sprite, uint8_t colour, unsigned flags)
{
    if (sprite.empty())
        return;
    const Rect &clip = dst.clip();
    decode(sprite, [&](int row, int col, uint8_t) {
        int dx = (flags & kDrawFlipX) ? x + sprite.width - 1 - col : x + col;
        int dy = (flags & kDrawFlipY) ? y + sprite.height - 1 - row : y + row;
        if (dx >= clip.x && dx < clip.x + clip.w && dy >= clip.y && dy < clip.y + clip.h)
            dst.row(dy)[dx] = colour;
    });
}

void sprite_to_argb(const Sprite &sprite, const Palette &palette, uint32_t *out, int pitch)
{
    if (sprite.empty())
        return;
    for (int row = 0; row < sprite.height; row++)
        for (int col = 0; col < sprite.width; col++)
            reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(out) + row * pitch)[col] = 0;
    decode(sprite, [&](int row, int col, uint8_t colour) {
        if (col < sprite.width)
            reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(out) + row * pitch)[col] = palette[colour];
    });
}

} // namespace th
