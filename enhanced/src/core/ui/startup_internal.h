#pragma once

// What the screens of startup*.cpp share: the state names, the sounds, glass
// boxes and buttons several of them use. Private to src/core/ui/startup*.cpp.

#include "core/ui/changelog.h"
#include "core/ui/startup.h"

#include <array>
#include <cstdint>

namespace th
{

enum class Startup::State {
    Intro,
    TitleEnter,
    Title,
    AboutEnter,
    AboutDelay,
    AboutZoom,
    AboutEntry,
    MenuEnter,
    Menu,
    LoadEnter,
    LoadDelay,
    LoadZoom,
    LoadEntry,
    PrefsEnter,
    PrefsDelay,
    PrefsZoom,
    PrefsEntry,
    AreaEnter,
    Briefing,
    AreaStart,
    CreditsEnter,
    Credits,
    Quit,
    Done,
};

// Names the screens share. An inline namespace, so that none of them clashes
// with a same-named helper of another file.
inline namespace startup_detail
{

constexpr int kDelay800ms = 20; // draw_nickname's delay(800), in frames

// Frames (0.3 s) a skipped movie's screen holds input back.
constexpr int kInputGuardFrames = 8;

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxYes = 0x41; // SFX_YESX
constexpr uint8_t kSfxNo = 0x42;  // SFX_NO4

// mainmenu.state (MMS_* in include/enums.h); 0x11 to 0x16 show the menu's
// diamond turning from one entry to another.
enum : int {
    kHoverContinue = 3,
    kHoverLoad = 4,
    kHoverNewGame = 5,
    kHoverNetwork = 6,
    kHoverQuit = 7,
    kNewGame = 8,
    kContinue = 9,
    kNetwork = 0xa,
    kQuit = 0xc,
    kLoadGame = 0xf,
};

// A glass box of hiresglass (src/scoords.cpp): a rectangle with corners
// (x, y) and (x1, y1), how it is drawn (type: -1 or 0 a background panel,
// 0 see-through; 1 or 2 a foreground one; button: a button face), the
// length of its zoom in frames, its delays and where it grows from (0 its
// centre, 1 its top centre, 2 the screen's centre).
struct GlassBox {
    int x, y, x1, y1;
    int type;
    bool button;
    int steps, delay, origin;
};

// Glass screen 10 (display bit 1, the preferences): hiresglass[310..321], less
// the screen-resolution row (hiresglass[314], [317], [318]), which has no use
// here.
constexpr std::array<GlassBox, 9> kPrefsGlass = {{
    {124, 30, 515, 449, 0, false, 7, 0, 0},
    {149, 55, 490, 424, 1, false, 7, 0, 0},
    {161, 67, 479, 176, 2, false, 7, 0, 0},
    {161, 303, 334, 336, 2, false, 7, 0, 0},
    {339, 303, 406, 336, 2, true, 7, 0, 0},
    {411, 303, 478, 336, 2, true, 7, 0, 0},
    {339, 379, 406, 412, 2, true, 7, 0, 0},
    {441, 72, 474, 105, 3, true, 7, 0, 0},
    {441, 138, 474, 171, 3, true, 7, 0, 0},
}};

// The about screen (display bit 2): the credits' frame and panel, and the
// CHANGELOG and CONTINUE buttons; and the changelog's (display bit 4): its
// panel, the scroll up and down buttons and CLOSE. The original has no such
// screens; the boxes are drawn as its preferences screen's are.
constexpr std::array<GlassBox, 4> kAboutGlass = {{
    {15, 40, 624, 439, 0, false, 7, 0, 0},
    {35, 60, 604, 419, 1, false, 7, 0, 0},
    {160, 364, 309, 397, 2, true, 7, 0, 0},
    {330, 364, 479, 397, 2, true, 7, 0, 0},
}};
enum : int { kAboutChangelogButton = 2, kAboutContinueButton = 3 };
constexpr std::array<GlassBox, 4> kChangelogGlass = {{
    {40, 12, 599, 467, 1, false, 7, 0, 0},
    {553, 36, 586, 69, 2, true, 7, 0, 0},
    {553, 360, 586, 393, 2, true, 7, 0, 0},
    {235, 418, 404, 451, 2, true, 7, 0, 0},
}};
enum : int { kChangelogUpButton = 1, kChangelogDownButton = 2, kChangelogCloseButton = 3 };
// The text of the changelog is drawn in this window of its panel, 30 lines of
// its font (12 pixels) tall so that no line is cut.
constexpr Rect kChangelogView = {62, 36, kChangelogTextWidth, 360};

// The rectangle of box `g`.
constexpr Rect box_rect(const GlassBox &g)
{
    return {g.x, g.y, g.x1 - g.x + 1, g.y1 - g.y + 1};
}

// Glass screen 12 (display bit 0x2000, the load game screen): the two panels
// and the nine save-slot buttons (hiresglass, src/scoords.cpp).
constexpr std::array<GlassBox, 11> kLoadGlass = {{
    {189, 31, 450, 449, 0, false, 7, 0, 0},
    {214, 56, 425, 403, 1, false, 7, 0, 0},
    {226, 69, 413, 102, 2, true, 7, 0, 1},
    {226, 105, 413, 138, 2, true, 7, 0, 1},
    {226, 141, 413, 174, 2, true, 7, 0, 1},
    {226, 177, 413, 210, 2, true, 7, 0, 1},
    {226, 213, 413, 246, 2, true, 7, 0, 1},
    {226, 249, 413, 282, 2, true, 7, 0, 1},
    {226, 285, 413, 318, 2, true, 7, 0, 1},
    {226, 321, 413, 354, 2, true, 7, 0, 1},
    {226, 357, 413, 390, 2, true, 7, 0, 1},
}};

// The preferences' buttons, screenbuttons 0xa0 to 0xa7 (src/scoords.cpp),
// corners (x, y) and (x1, y1): tutorial on and off, accept, cancel (drawn
// with PREF02V's sprites), previous and next difficulty. The game's low and
// high resolution buttons (0xa6, 0xa7) are left out. process_nickname_button(i)
// acts on button i.
struct ScreenButton {
    int x, y, x1, y1;
    bool sprite;
};
constexpr std::array<ScreenButton, 6> kPrefsButtons = {{
    {339, 303, 406, 336, false},
    {411, 303, 478, 336, false},
    {339, 379, 406, 412, false},
    {449, 383, 474, 408, true},
    {441, 72, 474, 105, false},
    {441, 138, 474, 171, false},
}};

// Font cache slots in Startup::fonts_.
enum : int { kFontGlass, kFontGlassHi, kFontTitle, kFontChangelog, kFontSlotCount };
constexpr std::array<int, kFontSlotCount> kStartupFonts = {0x21, 0x22, 0x2a, kChangelogFontSlot};

// Whether (mx, my) is in the w x h box at (x, y).
bool over(int mx, int my, int x, int y, int w, int h);

} // namespace startup_detail

} // namespace th
