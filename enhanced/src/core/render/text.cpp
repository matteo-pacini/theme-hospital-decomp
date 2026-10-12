#include "core/render/text.h"

#include "core/render/blit.h"
#include "core/render/iso.h"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace th
{

namespace
{

// The drawing state LbTextDraw keeps in lbDisplay: the flags and colour, which
// control bytes change, and the ghost table.
struct TextState {
    unsigned flags;
    uint8_t colour;
    const uint8_t *ghost;
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
        dst.put(x, y, colour);
}

// put_down_sprites' control bytes: toggles a drawing flag, or for 14 takes
// the next byte as the colour (advancing `i` past it).
void put_down_control(uint8_t c, std::string_view line, size_t &i, TextState &state)
{
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

// put_down_sprites: draws `line` with its top-left at `at`, spaces
// `space_len` wide.
void put_down_sprites(Surface &dst, const Font &font, std::string_view line, Point at, int space_len, TextState &state)
{
    int x = at.x;
    const int y = at.y;
    int underline_y = y + font.line_height();
    for (size_t i = 0; i < line.size(); i++) {
        uint8_t c = uint8_t(line[i]);
        if (c > 32) {
            Sprite glyph = font.glyph(c);
            if (state.flags & kTextOneColour)
                draw_sprite_one_colour(dst, x, y, glyph, state.colour);
            else
                draw_sprite(dst, x, y, glyph, state.flags & kSpriteFlags, state.ghost);
            if (state.flags & kTextUnderline)
                hline(dst, x, x + glyph.width, underline_y, state.colour);
            x += glyph.width;
        } else if (c == ' ' || c == '\t') {
            int advance = c == ' ' ? space_len : space_len * kSpacesPerTab;
            if (state.flags & kTextUnderline)
                hline(dst, x, x + advance, underline_y, state.colour);
            x += advance;
        } else {
            put_down_control(c, line, i, state);
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

// LbTextDraw's layout pass: walks the text, measuring and breaking lines,
// and hands each finished line to put_down_sprites.
class TextLayout
{
  public:
    // `at` is relative to settings.window; `cw` is the clip window drawn in.
    TextLayout(Surface &dst, const Font &font, std::string_view text, const TextSettings &settings, Rect cw, Point at)
        : dst_(dst), font_(font), text_(text), window_(settings.window), cw_(cw),
          state_{settings.flags, settings.colour, settings.ghost}, space_(font.space_width()),
          line_height_(font.line_height()), dx_(window_.x - cw.x), x_(at.x + dx_), y_(at.y + (window_.y - cw.y)),
          start_x_(x_)
    {
    }

    void run()
    {
        size_t i = 0;
        for (; i < text_.size() && text_[i]; i++) {
            uint8_t c = uint8_t(text_[i]);
            if (c > 32)
                x_ += font_.glyph(c).width;
            else if (c == ' ' || c == '\t')
                space(i, c);
            else if (c == '\n')
                newline(i);
            else if (c >= 6 && c <= 9)
                toggle(i, c);
            else if (c == 14 && i + 1 < text_.size())
                i++;
        }
        put(i, aligned_x(), space_);
    }

  private:
    void put(size_t end, int at_x, int space_len)
    {
        put_down_sprites(dst_, font_, text_.substr(begin_, end - begin_), {cw_.x + at_x, cw_.y + y_}, space_len,
                         state_);
    }

    // Where a line ending at x starts, for a hard break or the end of text.
    int aligned_x() const
    {
        if (state_.flags & kTextWrap)
            return start_x_;
        if (state_.flags & kTextRight)
            return start_x_ + (window_.width - (x_ - dx_));
        if (state_.flags & kTextCentre)
            return start_x_ + (window_.width - (x_ - dx_)) / 2;
        return start_x_;
    }

    // A space or tab at `i`: the line breaks there when the next word would
    // not fit and a wrapping or aligning mode is on.
    void space(size_t i, uint8_t c)
    {
        int advance = c == ' ' ? space_ : space_ * kSpacesPerTab;
        x_ += advance;
        if ((x_ - dx_) + word_width(font_, text_, i + 1) <= window_.width) {
            count_ = int16_t(count_ + (c == ' ' ? 1 : kSpacesPerTab));
            return;
        }
        if (state_.flags & (kTextWrap | kTextRight | kTextCentre | kTextJustify))
            break_line(i, advance);
        count_ = 0;
    }

    void break_line(size_t i, int advance)
    {
        int space_width = space_;
        int line_x = start_x_;
        if (!(state_.flags & kTextWrap)) {
            if (state_.flags & kTextRight) {
                line_x = start_x_ + (window_.width - (x_ - advance - dx_));
            } else if (state_.flags & kTextCentre) {
                line_x = start_x_ + (window_.width - (x_ - advance - dx_)) / 2;
            } else if (count_ > 0) {
                // A tab's line is measured less one space, not one tab, and
                // the division is unsigned, as in the game.
                space_width += int(uint32_t(window_.width - (x_ - space_ - dx_)) / uint32_t(count_));
            }
        }
        put(i, line_x, space_width);
        begin_ = i + 1;
        x_ = start_x_;
        y_ += line_height_;
    }

    void newline(size_t i)
    {
        put(i, aligned_x(), space_);
        begin_ = i;
        x_ = start_x_;
        y_ += line_height_;
        count_ = 0;
    }

    // Control bytes 6-9 toggle wrap, right, centre and justify, first
    // breaking the line if it has overrun the window.
    void toggle(size_t i, uint8_t c)
    {
        if (x_ - dx_ > window_.width) {
            x_ = start_x_;
            put(i, x_, space_);
            begin_ = i;
            y_ += line_height_;
            count_ = 0;
        }
        static constexpr unsigned kToggles[] = {kTextWrap, kTextRight, kTextCentre, kTextJustify};
        state_.flags ^= kToggles[c - 6];
    }

    Surface &dst_;
    const Font &font_;
    std::string_view text_;
    const TextWindow &window_;
    Rect cw_;
    TextState state_;
    int space_;
    int line_height_;
    int dx_;
    int x_;
    int y_;
    int start_x_;
    size_t begin_ = 0;
    int16_t count_ = 0;
};

} // namespace

void draw_text(Surface &dst, const Font &font, int x, int y, std::string_view text, const TextSettings &settings)
{
    if (font.empty())
        return;
    const Rect saved_clip = dst.clip();
    // Coordinates below are relative to the clip window, the graphics window
    // LbTextDraw draws in, as the original's are.
    const Rect cw = text_clip(settings.clip, dst.width(), dst.height());
    dst.set_clip(cw);
    TextLayout(dst, font, text, settings, cw, {x, y}).run();
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
