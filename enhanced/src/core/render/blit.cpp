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

// Copies the `length` indices of a run to `out` at [from, from + length)
// clipped to the columns [clip.x, clip.y), mirrored if `flip`.
void copy_run(uint8_t *out, int from, int length, const uint8_t *run, Point clip, bool flip)
{
    const int a = std::max(from, clip.x), b = std::min(from + length, clip.y);
    if (a >= b)
        return;
    if (!flip) {
        std::copy(run + (a - from), run + (b - from), out + a);
        return;
    }
    for (int i = a; i < b; i++)
        out[i] = run[length - 1 - (i - from)];
}

// draw_hires_pixels for a sprite drawn solid (no see-through): whole runs at
// a time.
void draw_hires_solid(Surface &dst, int x, int y, const HiSprite &hi, unsigned flags)
{
    const int n = dst.scale();
    const Rect &clip = dst.clip();
    const int x0 = clip.x * n, x1 = (clip.x + clip.w) * n;
    const int y0 = clip.y * n, y1 = (clip.y + clip.h) * n;
    const bool flip_x = flags & kDrawFlipX;
    auto u16 = [](const uint8_t *p) { return int(p[0] | p[1] << 8); };
    const uint8_t *p = hi.data;
    for (int row = 0; row < hi.height; row++) {
        const int runs = u16(p);
        p += 2;
        const int dy = (flags & kDrawFlipY) ? y * n + hi.height - 1 - row : y * n + row;
        uint8_t *out = dy >= y0 && dy < y1 ? dst.row(dy) : nullptr;
        int col = 0;
        for (int r = 0; r < runs; r++) {
            col += u16(p);
            const int length = u16(p + 2);
            p += 4;
            if (out)
                copy_run(out, flip_x ? x * n + hi.width - col - length : x * n + col, length, p, {x0, x1}, flip_x);
            p += length;
            col += length;
        }
    }
}

// Calls write(index, colour) for each opaque pixel of the upscaled sprite `hi`
// drawn with its top-left corner at game pixel (x, y), mirrored by `flags`,
// that falls inside dst's clip rectangle.
template <typename Write>
void draw_hires_pixels(Surface &dst, int x, int y, const HiSprite &hi, unsigned flags, Write &write)
{
    const int n = dst.scale();
    const Rect &clip = dst.clip();
    const int x0 = clip.x * n, x1 = (clip.x + clip.w) * n;
    const int y0 = clip.y * n, y1 = (clip.y + clip.h) * n;
    decode_hires(hi, [&](int row, int col, uint8_t colour) {
        int dx = (flags & kDrawFlipX) ? x * n + hi.width - 1 - col : x * n + col;
        int dy = (flags & kDrawFlipY) ? y * n + hi.height - 1 - row : y * n + row;
        if (dx >= x0 && dx < x1 && dy >= y0 && dy < y1)
            write(dst.row(dy)[dx], colour);
    });
}

// As draw_hires_pixels for the 1x `sprite`: each of its pixels is the n x n
// indices of a game pixel, n being dst's scale.
template <typename Write>
void draw_block_pixels(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags, Write &write)
{
    const int n = dst.scale();
    const Rect &clip = dst.clip();
    decode(sprite, [&](int row, int col, uint8_t colour) {
        int dx = (flags & kDrawFlipX) ? x + sprite.width - 1 - col : x + col;
        int dy = (flags & kDrawFlipY) ? y + sprite.height - 1 - row : y + row;
        if (dx < clip.x || dx >= clip.x + clip.w || dy < clip.y || dy >= clip.y + clip.h)
            return;
        for (int j = 0; j < n; j++) {
            uint8_t *out = dst.row(dy * n + j) + dx * n;
            for (int i = 0; i < n; i++)
                write(out[i], colour);
        }
    });
}

// Calls write(index, colour) for each opaque pixel of `sprite` drawn with its
// top-left corner at game pixel (x, y), mirrored by `flags`, that falls
// inside the clip rectangle; `index` is the surface's index to change. On a
// surface of scale N that is the sprite's upscaled pixels when it has them
// for N, else the N x N indices of each game pixel.
template <typename Write>
void draw_pixels(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags, Write &&write)
{
    const Rect &clip = dst.clip();
    if (sprite.empty() || x >= clip.x + clip.w || y >= clip.y + clip.h || x + sprite.width <= clip.x ||
        y + sprite.height <= clip.y)
        return;
    if (dst.scale() > 1 && !sprite.hi.empty() && sprite.hi.factor == dst.scale())
        draw_hires_pixels(dst, x, y, sprite.hi, flags, write);
    else
        draw_block_pixels(dst, x, y, sprite, flags, write);
}

} // namespace

void draw_sprite(Surface &dst, int x, int y, const Sprite &sprite, unsigned flags, const uint8_t *ghost)
{
    const bool solid = !ghost || !(flags & (kDrawGhost | kDrawGhostUnder));
    const Rect &clip = dst.clip();
    if (solid && dst.scale() > 1 && !sprite.hi.empty() && sprite.hi.factor == dst.scale()) {
        if (x < clip.x + clip.w && y < clip.y + clip.h && x + sprite.width > clip.x && y + sprite.height > clip.y)
            draw_hires_solid(dst, x, y, sprite.hi, flags);
        return;
    }
    draw_pixels(dst, x, y, sprite, flags, [&](uint8_t &out, uint8_t colour) { put_pixel(out, colour, flags, ghost); });
}

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
    const int n = dst.scale();
    for (int dy = std::max(0, clip.y - y); dy < size.y && y + dy < clip.y + clip.h; dy++) {
        int row = dy * sprite.height / size.y;
        if (flags & kDrawFlipY)
            row = sprite.height - 1 - row;
        for (int dx = std::max(0, clip.x - x); dx < size.x && x + dx < clip.x + clip.w; dx++) {
            int col = dx * sprite.width / size.x;
            if (flags & kDrawFlipX)
                col = sprite.width - 1 - col;
            int colour = pixels[size_t(row) * size_t(sprite.width) + size_t(col)];
            if (colour < 0)
                continue;
            for (int j = 0; j < n; j++) {
                uint8_t *out = dst.row((y + dy) * n + j) + (x + dx) * n;
                for (int i = 0; i < n; i++)
                    put_pixel(out[i], uint8_t(colour), flags, ghost);
            }
        }
    }
}

void draw_sprite_one_colour(Surface &dst, int x, int y, const Sprite &sprite, uint8_t colour, unsigned flags)
{
    draw_pixels(dst, x, y, sprite, flags, [&](uint8_t &out, uint8_t) { out = colour; });
}

void draw_sprite_glow(Surface &dst, Point at, const Sprite &sprite, unsigned flags, uint8_t glow, const uint8_t *ghost)
{
    draw_pixels(dst, at.x, at.y, sprite, flags, [&](uint8_t &out, uint8_t colour) {
        if (ghost && (flags & kDrawGhost))
            out = ghost[colour << 8 | glow];
        else if (ghost && (flags & kDrawGhostUnder))
            out = ghost[glow << 8 | colour];
        else
            out = colour;
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
