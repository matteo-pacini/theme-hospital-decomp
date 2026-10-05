#pragma once

#include "core/data/text.h"
#include "core/render/surface.h"

#include <cstdint>
#include <string_view>

namespace th
{

// Text layout flags, as the game's lbDisplay.draw_flags. Without any of the
// four alignments a line is never wrapped. The sprite flags of DrawFlags
// (blit.h) also apply to the glyphs.
enum TextFlags : unsigned {
    // Wrap words at the window's width, lines left-aligned.
    kTextWrap = 0x20,
    // Glyphs drawn in the single text colour.
    kTextOneColour = 0x40,
    // Wrap, lines right-aligned.
    kTextRight = 0x80,
    // Wrap, lines centred.
    kTextCentre = 0x100,
    // Wrap, the spaces of wrapped lines widened to fill the window.
    kTextJustify = 0x200,
    // Underlined in the text colour.
    kTextUnderline = 0x400,
};

// Tabs are this many spaces wide (lbSpacesPerTab).
inline constexpr int kSpacesPerTab = 4;

// The text justify window (LbTextSetJustifyWindow): text is placed relative
// to its top-left corner and aligned and wrapped within its width.
struct TextWindow {
    int x = 0;
    int y = 0;
    int width = 0;
};

// Draws `text` as LbTextDraw does: at (x, y) relative to `window`, in
// `font`, laid out by `flags` (TextFlags | DrawFlags), clipped to `clip`
// (LbTextSetClipWindow: a negative width or height extends left or up, and
// it is limited to the surface; the surface's own clip rectangle is ignored
// and left as it was). The alignment offsets subtract x from the window's
// width, so x = 0 centres within the window. Lines advance by the font's
// line height.
//
// Control bytes in `text`: '\n' breaks the line; 6, 7, 8 and 9 toggle
// kTextWrap, kTextRight, kTextCentre and kTextJustify; 1-5 toggle the sprite
// flags 0x04, 0x08, 0x10 (no effect here), 0x01 and 0x02; 11 toggles kTextUnderline; 12
// toggles kTextOneColour; 14 sets the text colour (initially `colour`) to
// the next byte. Their effect lasts to the end of this call only.
// Ghost flags need `ghost` (GameData::ghost()).
void draw_text(Surface &dst, const Font &font, TextWindow window, int x, int y, std::string_view text, unsigned flags,
               Rect clip, uint8_t colour = 0, const uint8_t *ghost = nullptr);

// Width in pixels of the widest line of `text` (LbTextStringWidth), tabs
// counted as kSpacesPerTab spaces. Control bytes count 0, but the byte after
// a 14 is measured as a character, as in the game.
int text_width(const Font &font, std::string_view text);

// Height in pixels of `text`'s lines (LbTextStringHeight).
int text_height(const Font &font, std::string_view text);

} // namespace th
