#include "core/render/blit.h"

namespace th
{

namespace
{

// Walks `sprite` row by row, calling put(row, col, colour) for each opaque
// pixel in the sprite's own coordinates.
template <typename Put> void decode(const Sprite &sprite, Put &&put)
{
    const uint8_t *p = sprite.data;
    for (int row = 0; row < sprite.height; row++) {
        int col = 0;
        if (sprite.format == SpriteFormat::Rle) {
            for (int8_t n; (n = static_cast<int8_t>(*p++)) != 0;) {
                if (n < 0) {
                    col -= n;
                    continue;
                }
                for (int i = 0; i < n; i++)
                    put(row, col++, p[i]);
                p += n;
            }
        } else {
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
        }
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

void draw_sprite_one_colour(Surface &dst, int x, int y, const Sprite &sprite, uint8_t colour)
{
    if (sprite.empty())
        return;
    const Rect &clip = dst.clip();
    decode(sprite, [&](int row, int col, uint8_t) {
        int dx = x + col, dy = y + row;
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
