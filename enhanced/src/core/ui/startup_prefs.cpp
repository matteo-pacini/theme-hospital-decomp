// The preferences screen of Startup: the difficulty and tutorial choices and
// the nickname buttons (src/name.cpp's process_nickname_button).

#include "core/ui/startup_internal.h"

#include "core/data/balance.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>

namespace th
{

namespace
{

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxSelect = 0x43; // SFX_SELECTX

enum : int {
    kTutorialOn,
    kTutorialOff,
    kAccept,
    kCancel,
    kPrevBalance,
    kNextBalance,
};

// ScreenBoxes[122]: where the difficulty's title and name are centred.
constexpr Rect kBalanceBox = {179, 67, 245, 108};

} // namespace

void Startup::preferences(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    bool several = balance_files_.size() > 1;
    show(prefs_);
    if (state_ == State::PrefsZoom) {
        // draw_nickname state 6.
        if (current_display_ == desired_display_)
            state_ = State::PrefsEntry;
        draw_zoom_boxes();
    } else {
        // State 5.
        draw_new_prefs_screen(mouse, several);
        // process_keys_nickname state 5.
        for (const StartupKey &k : keys) {
            if (k.code == StartupKey::Escape) {
                sound(kSfxNo);
                state_ = State::MenuEnter;
                return;
            }
        }
    }

    // process_mouse_nickname.
    if (current_display_ & desired_display_ & 1)
        draw_sprite(surface_, kPrefsButtons[kCancel].x, kPrefsButtons[kCancel].y, prefs_sprites_.at(2));
    if (state_ != State::PrefsEntry || !(current_display_ & 1))
        return;
    State next = state_;
    for (int i = 0; i < int(kPrefsButtons.size()); i++) {
        if (!several && (i == kPrevBalance || i == kNextBalance))
            continue;
        prefs_button(mouse, i, next);
    }
    state_ = next;
}

void Startup::draw_new_prefs_screen(const UiMouse &mouse, bool several)
{
    draw_zoom_boxes();
    if (several) {
        draw_glass_text(mouse, kPrevBalance, "{", false);
        draw_glass_text(mouse, kNextBalance, "|", false);
    }
    auto nick = [&](int i) { return text_->at(TextSection::Nickname, i); };
    draw_glass_text(mouse, kAccept, nick(0x12), false);
    draw_glass_text(mouse, kTutorialOn, nick(0x10), tutorial_);
    draw_glass_text(mouse, kTutorialOff, nick(0x11), !tutorial_);
    const Font &font = fonts_[kFontGlass];
    TextWindow window{0, 0, surface_.width()};
    Rect clip{0, 0, surface_.width(), surface_.height()};
    int ch = font.glyph('A').height;
    int y = 0x12f;
    y += (0x150 - y - ch) >> 1;
    draw_text(surface_, font, 0xaf, y, nick(6), {window, clip});
    int bh = font.glyph('B').height;
    std::string_view title = text_->misc(0x1e);
    int ty = kBalanceBox.y + (kBalanceBox.h - bh * 2) / 2;
    draw_text(surface_, font, kBalanceBox.x + (kBalanceBox.w - text_width(font, title)) / 2, ty, title, {window, clip});
    draw_text(surface_, font, kBalanceBox.x + (kBalanceBox.w - text_width(font, balance_name_)) / 2, ty + bh,
              balance_name_, {window, clip});
}

void Startup::prefs_button(const UiMouse &mouse, int i, State &next)
{
    const ScreenButton &b = kPrefsButtons[size_t(i)];
    int w = b.x1 - b.x + 1, h = b.y1 - b.y + 1;
    auto press = [&] {
        if (b.sprite)
            draw_sprite(surface_, b.x, b.y, prefs_sprites_.at(1));
        else
            draw_button_edges(b.x, b.y, b.x1 - b.x, b.y1 - b.y, true);
    };
    if (mouse.left_released && over(mouse.x, mouse.y, b.x, b.y, w, h)) {
        press();
        process_nickname_button(i, next);
        return;
    }
    if (mouse.left_held && over(mouse.x, mouse.y, b.x, b.y, w, h))
        press();
    bool chosen = (i == kTutorialOn && tutorial_) || (i == kTutorialOff && !tutorial_);
    if (chosen)
        draw_button_edges(b.x, b.y, b.x1 - b.x, b.y1 - b.y, true);
}

void Startup::process_nickname_button(int i, State &next)
{
    switch (i) {
    case kPrevBalance:
    case kNextBalance:
        sound(kSfxSelect);
        balance_ = (balance_ + (i == kNextBalance ? 1 : balance_files_.size() - 1)) % balance_files_.size();
        balance_name_ = std::string(text_->at(TextSection::Nickname, 0x15));
        if (auto d = balance_description(dir_, balance_files_[balance_], language_))
            balance_name_ = *d;
        break;
    case kTutorialOn:
    case kTutorialOff:
        tutorial_ = i == kTutorialOn;
        sound(kSfxSelect);
        break;
    case kAccept:
        sound(kSfxYes);
        next = State::AreaEnter;
        break;
    case kCancel:
        sound(kSfxNo);
        next = State::MenuEnter;
        break;
    }
}

} // namespace th
