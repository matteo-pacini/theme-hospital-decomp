#include "core/render/text.h"

#include "core/render/blit.h"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace th
{

namespace
{

// The drawing state LbTextDraw keeps in lbDisplay, which control bytes change.
struct TextState {
    unsigned flags;
    uint8_t colour;
};

constexpr unsigned kSpriteFlags = kDrawFlipX | kDrawFlipY | kDrawGhost | kDrawGhostUnder;

int char_width(const Font &font, uint8_t c)
{
    return c == ' ' ? font.space_width() : font.glyph(c).width;
}

// LbTextWordWidth: the width of the word starting at `from`.
int word_width(const Font &font, std::string_view text, size_t from)
{
    int width = 0;
    for (size_t i = from; i < text.size(); i++) {
        uint8_t c = uint8_t(text[i]);
        if (c == ' ' || c == '\t' || c == '\n' || c == 0)
            break;
        if (c > 32)
            width += font.glyph(c).width;
    }
    return width;
}

// LbDrawHVLine for a horizontal line, both ends included.
void hline(Surface &dst, int x1, int x2, int y, uint8_t colour)
{
    const Rect &clip = dst.clip();
    if (y < clip.y || y >= clip.y + clip.h)
        return;
    x1 = std::max(x1, clip.x);
    x2 = std::min(x2, clip.x + clip.w - 1);
    for (int x = x1; x <= x2; x++)
        dst.row(y)[x] = colour;
}

// put_down_sprites: draws `line` with its top-left at (x, y), spaces
// `space_len` wide.
void put_down_sprites(Surface &dst, const Font &font, std::string_view line, int x, int y, int space_len,
                      TextState &state, const uint8_t *ghost)
{
    int underline_y = y + font.line_height();
    for (size_t i = 0; i < line.size(); i++) {
        uint8_t c = uint8_t(line[i]);
        if (c > 32) {
            Sprite glyph = font.glyph(c);
            if (state.flags & kTextOneColour)
                draw_sprite_one_colour(dst, x, y, glyph, state.colour);
            else
                draw_sprite(dst, x, y, glyph, state.flags & kSpriteFlags, ghost);
            if (state.flags & kTextUnderline)
                hline(dst, x, x + glyph.width, underline_y, state.colour);
            x += glyph.width;
        } else if (c == ' ' || c == '\t') {
            int advance = c == ' ' ? space_len : space_len * kSpacesPerTab;
            if (state.flags & kTextUnderline)
                hline(dst, x, x + advance, underline_y, state.colour);
            x += advance;
        } else {
            switch (c) {
            case 1:
                state.flags ^= 0x04;
                break;
            case 2:
                state.flags ^= 0x08;
                break;
            case 3:
                state.flags ^= 0x10;
                break;
            case 4:
                state.flags ^= 0x01;
                break;
            case 5:
                state.flags ^= 0x02;
                break;
            case 11:
                state.flags ^= kTextUnderline;
                break;
            case 12:
                state.flags ^= kTextOneColour;
                break;
            case 14:
                if (i + 1 < line.size())
                    state.colour = uint8_t(line[++i]);
                break;
            }
        }
    }
}

// LbTextSetClipWindow's box: corners swapped into order, then limited to the
// surface.
Rect text_clip(Rect clip, int width, int height)
{
    int x1 = clip.x, y1 = clip.y, x2 = clip.x + clip.w, y2 = clip.y + clip.h;
    if (x2 < x1)
        std::swap(x1, x2);
    if (y2 < y1)
        std::swap(y1, y2);
    x1 = std::clamp(x1, 0, width);
    x2 = std::clamp(x2, 0, width);
    y1 = std::clamp(y1, 0, height);
    y2 = std::clamp(y2, 0, height);
    return {x1, y1, x2 - x1, y2 - y1};
}

} // namespace

void draw_text(Surface &dst, const Font &font, TextWindow window, int x, int y, std::string_view text, unsigned flags,
               Rect clip, uint8_t colour, const uint8_t *ghost)
{
    if (font.empty())
        return;
    const Rect saved_clip = dst.clip();
    // Coordinates below are relative to the clip window, the graphics window
    // LbTextDraw draws in, as the original's are.
    const Rect cw = text_clip(clip, dst.width(), dst.height());
    dst.set_clip(cw);

    TextState state{flags, colour};
    const int space = font.space_width();
    const int line_height = font.line_height();
    const int dx = window.x - cw.x;
    const int dy = window.y - cw.y;
    x += dx;
    y += dy;
    const int start_x = x;
    size_t begin = 0;
    int16_t count = 0;

    auto put = [&](size_t end, int at_x, int space_len) {
        put_down_sprites(dst, font, text.substr(begin, end - begin), cw.x + at_x, cw.y + y, space_len, state, ghost);
    };
    // Where a line ending at x starts, for a hard break or the end of text.
    auto aligned_x = [&] {
        if (state.flags & kTextWrap)
            return start_x;
        if (state.flags & kTextRight)
            return start_x + (window.width - (x - dx));
        if (state.flags & kTextCentre)
            return start_x + (window.width - (x - dx)) / 2;
        return start_x;
    };

    size_t i = 0;
    for (; i < text.size() && text[i]; i++) {
        uint8_t c = uint8_t(text[i]);
        if (c > 32) {
            x += font.glyph(c).width;
        } else if (c == ' ' || c == '\t') {
            int advance = c == ' ' ? space : space * kSpacesPerTab;
            x += advance;
            if ((x - dx) + word_width(font, text, i + 1) <= window.width) {
                count = int16_t(count + (c == ' ' ? 1 : kSpacesPerTab));
                continue;
            }
            int space_width = space;
            if (state.flags & (kTextWrap | kTextRight | kTextCentre | kTextJustify)) {
                int line_x = start_x;
                if (!(state.flags & kTextWrap)) {
                    if (state.flags & kTextRight) {
                        line_x = start_x + (window.width - (x - advance - dx));
                    } else if (state.flags & kTextCentre) {
                        line_x = start_x + (window.width - (x - advance - dx)) / 2;
                    } else if (count > 0) {
                        // A tab's line is measured less one space, not one
                        // tab, and the division is unsigned, as in the game.
                        space_width += int(uint32_t(window.width - (x - space - dx)) / uint32_t(count));
                    }
                }
                put(i, line_x, space_width);
                begin = i + 1;
                x = start_x;
                y += line_height;
            }
            count = 0;
        } else if (c == '\n') {
            put(i, aligned_x(), space);
            begin = i;
            x = start_x;
            y += line_height;
            count = 0;
        } else if (c >= 6 && c <= 9) {
            if (x - dx > window.width) {
                x = start_x;
                put(i, x, space);
                begin = i;
                y += line_height;
                count = 0;
            }
            static constexpr unsigned kToggles[] = {kTextWrap, kTextRight, kTextCentre, kTextJustify};
            state.flags ^= kToggles[c - 6];
        } else if (c == 14 && i + 1 < text.size()) {
            i++;
        }
    }
    put(i, aligned_x(), space);
    dst.set_clip(saved_clip);
}

int text_width(const Font &font, std::string_view text)
{
    if (font.empty())
        return 0;
    int max = 0;
    int width = 0;
    for (char ch : text) {
        uint8_t c = uint8_t(ch);
        if (c == 0)
            break;
        if (c > 31) {
            width += char_width(font, c);
        } else if (c == '\t') {
            width += font.space_width() * kSpacesPerTab;
        } else if (c == '\n') {
            max = std::max(max, width);
            width = 0;
        }
    }
    return std::max(max, width);
}

int text_height(const Font &font, std::string_view text)
{
    if (font.empty())
        return 0;
    int lines = 1;
    for (char c : text) {
        if (c == 0)
            break;
        if (c == '\n')
            lines++;
    }
    return font.line_height() * lines;
}

} // namespace th
