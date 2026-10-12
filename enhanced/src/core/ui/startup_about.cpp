// The about screen of Startup: who made the game and the port, the version,
// and the changelog. It takes the place of the original's name screen
// (draw_nickname, src/name.cpp), after the title picture; the glass boxes and
// buttons are drawn as the preferences screen's are.

#include "core/ui/startup_internal.h"

#include "core/render/text.h"
#include "th_changelog.h"
#include "th_version.h"

#include <algorithm>

namespace th
{

namespace
{

// play_sound's sfx number (include/enums.h).
constexpr uint8_t kSfxSelect = 0x43; // SFX_SELECTX

// The credit lines, English only. An empty entry is a gap between groups;
// the first line, the title, is drawn in the large font and the second is
// the version.
constexpr std::array<std::string_view, 8> kCredits = {
    "Original game by Bullfrog Productions",
    "Published by Electronic Arts (1997)",
    "",
    "Decompiled and enhanced by Matteo Pacini",
    "github.com/matteo-pacini/theme-hospital-decomp",
    "",
    "Theme Hospital is a trademark of Electronic Arts.",
    "This project is not affiliated with or endorsed by EA.",
};
constexpr std::string_view kTitle = "THEME HOSPITAL ENHANCED";
constexpr int kCreditsTop = 80; // the title's top, in the panel
constexpr int kGroupGap = 12;   // between groups of credit lines
// The title picture shows bare this long (2 s) before the credits come up,
// unless a key or click brings them at once.
constexpr int kPictureFrames = 50;

// The changelog's scroll is in whole lines: a mouse wheel notch moves this
// many, a scroll button held longer than kHoldFrames moves one every
// kRepeatFrames.
constexpr int kWheelLines = 3;
constexpr int kHoldFrames = 8;
constexpr int kRepeatFrames = 3;

} // namespace

bool Startup::about_box_shown() const
{
    return state_ == State::AboutZoom || state_ == State::AboutEntry;
}

int Startup::changelog_scroll_max() const
{
    const int content = int(changelog_lines_.size()) * fonts_[kFontChangelog].line_height();
    return std::max(content - kChangelogView.h, 0);
}

void Startup::enter_about()
{
    show(title_);
    calculate_grey(title_);
    screen_ = StartupScreen::About;
    changelog_open_ = false;
    changelog_scroll_ = 0;
    timer_ = kPictureFrames;
    state_ = State::AboutDelay;
}

void Startup::about_wait(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    // The key or click that skips the wait is spent: it brings the box up and
    // presses nothing in it.
    show(title_);
    if (--timer_ <= 0 || mouse.left_pressed || mouse.right_pressed || !keys.empty()) {
        init_zoom_boxes(2);
        state_ = State::AboutZoom;
    }
}

void Startup::about_screen(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    show(title_);
    if (state_ == State::AboutZoom && current_display_ == desired_display_)
        state_ = State::AboutEntry;
    draw_zoom_boxes();
    if (state_ != State::AboutEntry)
        return;
    // The credits' text is under the changelog's panel for as long as that
    // is open or still zooming.
    const bool changelog_shown =
        std::any_of(zoom_.begin(), zoom_.end(), [](const ZoomBox &z) { return z.active && z.id == 4; });
    if (!changelog_shown)
        draw_about(mouse);
    else if (changelog_open_ && (current_display_ & 4))
        draw_changelog(mouse);
    // The boxes are still opening or closing: the buttons do not work yet.
    if (current_display_ != desired_display_)
        return;
    about_keys(keys);
    if (state_ == State::AboutEntry)
        about_mouse(mouse);
}

void Startup::about_keys(std::span<const StartupKey> keys)
{
    const int line = fonts_[kFontChangelog].line_height();
    for (const StartupKey &k : keys) {
        if (!changelog_open_) {
            // Escape does nothing here: it quit the original's name screen.
            if (k.code == StartupKey::Return || k.code == StartupKey::Space) {
                about_continue();
                return;
            }
            if (k.code == StartupKey::Letter && k.letter == 'C') {
                about_open_changelog();
                return;
            }
            continue;
        }
        switch (k.code) {
        case StartupKey::Escape:
            about_close_changelog();
            return;
        case StartupKey::Up:
            about_scroll(-line);
            break;
        case StartupKey::Down:
            about_scroll(line);
            break;
        case StartupKey::PageUp:
            about_scroll(line - kChangelogView.h);
            break;
        case StartupKey::PageDown:
            about_scroll(kChangelogView.h - line);
            break;
        default:
            break;
        }
    }
}

void Startup::about_mouse(const UiMouse &mouse)
{
    auto inside = [&](const GlassBox &g) {
        const Rect r = box_rect(g);
        return over(mouse.x, mouse.y, r.x, r.y, r.w, r.h);
    };
    const int line = fonts_[kFontChangelog].line_height();
    if (!changelog_open_) {
        if (mouse.left_released && inside(kAboutGlass[kAboutContinueButton]))
            about_continue();
        else if (mouse.left_released && inside(kAboutGlass[kAboutChangelogButton]))
            about_open_changelog();
        return;
    }
    if (mouse.wheel)
        about_scroll(-mouse.wheel * kWheelLines * line);
    if (mouse.left_released && inside(kChangelogGlass[kChangelogCloseButton])) {
        about_close_changelog();
        return;
    }
    // A press moves a line; held on, the text goes on moving.
    const bool up = inside(kChangelogGlass[kChangelogUpButton]);
    const bool down = inside(kChangelogGlass[kChangelogDownButton]);
    if ((up || down) && mouse.left_pressed) {
        sound(kSfxSelect);
        about_scroll(up ? -line : line);
    }
    scroll_hold_ = (up || down) && mouse.left_held ? scroll_hold_ + 1 : 0;
    if (scroll_hold_ > kHoldFrames && scroll_hold_ % kRepeatFrames == 0)
        about_scroll(up ? -line : line);
}

void Startup::about_continue()
{
    sound(kSfxYes);
    name_confirmed_ = true;
    state_ = State::MenuEnter;
}

void Startup::about_open_changelog()
{
    sound(kSfxSelect);
    wrap_changelog();
    changelog_open_ = true;
    changelog_scroll_ = 0;
    scroll_hold_ = 0;
    add_zoom_boxes(4);
}

void Startup::about_close_changelog()
{
    sound(kSfxNo);
    changelog_open_ = false;
    desired_display_ &= ~4;
}

void Startup::about_scroll(int pixels)
{
    changelog_scroll_ = std::clamp(changelog_scroll_ + pixels, 0, changelog_scroll_max());
}

void Startup::wrap_changelog()
{
    if (changelog_lines_.empty())
        changelog_lines_ =
            changelog_wrap(fonts_[kFontChangelog], changelog_paragraphs(kChangelogMarkdown), kChangelogTextWidth);
}

void Startup::draw_box_label(const UiMouse &mouse, const Rect &box, std::string_view label)
{
    // As draw_glass_text: centred, a pixel down and right in the highlighted
    // font while the button is held.
    const bool hover = over(mouse.x, mouse.y, box.x, box.y, box.w, box.h);
    const bool held = hover && (mouse.left_held || mouse.left_released);
    const Font &font = fonts_[hover ? kFontGlassHi : kFontGlass];
    int x = box.x + ((box.w - text_width(font, label)) >> 1);
    int y = box.y + ((box.h - font.glyph('A').height) >> 1);
    if (held) {
        x++;
        y++;
        draw_button_edges(box.x, box.y, box.w - 1, box.h - 1, true);
    }
    draw_text(surface_, font, x, y, label, {{0, 0, surface_.width()}, {0, 0, surface_.width(), surface_.height()}});
}

void Startup::draw_about(const UiMouse &mouse)
{
    const TextSettings settings{{0, 0, surface_.width()}, {0, 0, surface_.width(), surface_.height()}};
    auto centred = [&](const Font &font, int y, std::string_view text) {
        draw_text(surface_, font, (surface_.width() - text_width(font, text)) / 2, y, text, settings);
    };
    const Font &title = fonts_[kFontTitle];
    const Font &body = fonts_[kFontGlass];
    int y = kCreditsTop;
    centred(title, y, kTitle);
    y += title.line_height();
    centred(body, y, std::string("v") + kVersion);
    y += body.line_height() + kGroupGap;
    for (std::string_view line : kCredits) {
        if (line.empty()) {
            y += kGroupGap;
            continue;
        }
        centred(body, y, line);
        y += body.line_height();
    }
    draw_box_label(mouse, box_rect(kAboutGlass[kAboutChangelogButton]), "CHANGELOG");
    draw_box_label(mouse, box_rect(kAboutGlass[kAboutContinueButton]), "CONTINUE");
}

void Startup::draw_changelog(const UiMouse &mouse)
{
    const Font &font = fonts_[kFontChangelog];
    const int line = font.line_height();
    const Rect clip = kChangelogView;
    const TextSettings settings{{0, 0, surface_.width()}, clip};
    for (size_t i = 0; i < changelog_lines_.size(); i++) {
        const int y = clip.y + int(i) * line - changelog_scroll_;
        if (y + line <= clip.y || y >= clip.y + clip.h)
            continue;
        draw_text(surface_, font, clip.x + changelog_lines_[i].indent, y, changelog_lines_[i].text, settings);
    }
    // { and | are the glass font's up and down arrows.
    draw_box_label(mouse, box_rect(kChangelogGlass[kChangelogUpButton]), "{");
    draw_box_label(mouse, box_rect(kChangelogGlass[kChangelogDownButton]), "|");
    draw_box_label(mouse, box_rect(kChangelogGlass[kChangelogCloseButton]), "CLOSE");
}

} // namespace th
