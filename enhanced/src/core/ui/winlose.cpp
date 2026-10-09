#include "core/ui/winlose.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace th
{

namespace
{

constexpr uint8_t kSfxWinLoseType = 0xee;

// ScreenCoords 0xcf and 0xd0 and ScreenBox 0xae (src/scoords.cpp).
constexpr Point kAccept = {390, 400};
constexpr Point kDecline = {340, 400};
constexpr Rect kLetterBox = {187, 158, 261, 228};
constexpr int kLetterWidth = 261;

constexpr double kGameFrame = 0.04;

// The lose text's colours: the line is drawn centred near the bottom.
constexpr int kLoseTextY = 0x83;

} // namespace

std::string WinLoseScreen::format_args(std::string text, std::string_view hospital, std::string_view offer,
                                       std::string_view town)
{
    std::array<std::string_view, 3> args = {hospital, offer, town};
    size_t arg = 0;
    std::string out;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] != '%' || i + 1 >= text.size()) {
            out += text[i];
            continue;
        }
        char spec = text[++i];
        if ((spec == 's' || spec == 'd') && arg < args.size())
            out += args[arg++];
        else
            out += spec;
    }
    return out;
}

bool WinLoseScreen::load(const std::filesystem::path &dir, const LanguageText &text, const Palette &game_palette,
                         std::string *error)
{
    text_ = &text;
    game_palette_ = game_palette;
    auto tab = load_data_file(dir, "qdata/lettr02v.tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/lettr02v.dat", error) : std::nullopt;
    if (!dat || !sprites_.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error && error->empty())
            *error = "lettr02v: not a sprite table";
        return false;
    }
    bool ok = true;
    for (int i = 0; i < 5; i++)
        ok &= load_font(dir, 0x13 + i, fonts_[size_t(i)], error);
    surface_.resize(kUiWidth, kUiHeight);
    palette_ = game_palette;
    return ok;
}

void WinLoseScreen::open(bool win, int level)
{
    open_ = true;
    win_ = win;
    level_ = level;
    letter_.clear();
    typed_ = 0;
    type_delay_ = 1;
    frame_time_ = 0;
    pending_mouse_ = {};
    pending_keys_ = {};
    surface_.fill(0);
}

std::string WinLoseScreen::composite_page(TextSection section, int page) const
{
    int n = int(text_->size(section));
    int current = 0;
    std::string out;
    for (int i = 0; i < n; i++) {
        std::string_view s = text_->at(section, i);
        if (s == ".") {
            if (current == page)
                return out;
            current++;
            out.clear();
            continue;
        }
        if (current == page)
            out += s;
    }
    return out;
}

std::string WinLoseScreen::wrap(const std::string &text, int width, const Font &font) const
{
    // letter_format_text: '/' forces a new line; words are copied with a
    // trailing space and wrapped when they would pass `width`.
    std::string out;
    int line_width = 0;
    size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && text[i] == ' ')
            i++;
        if (i >= text.size())
            break;
        if (text[i] == '/') {
            out += '\n';
            line_width = 0;
            i++;
            continue;
        }
        size_t end = i;
        while (end < text.size() && text[end] != ' ' && text[end] != '/')
            end++;
        std::string_view word = std::string_view(text).substr(i, end - i);
        int word_width = text_width(font, word);
        if (word_width + line_width > width) {
            out += '\n';
            line_width = 0;
        }
        out += word;
        out += ' ';
        line_width += word_width + font.space_width();
        i = end;
    }
    return out;
}

std::string WinLoseScreen::letter_text(const WinLoseView &view) const
{
    const Font &font = fonts_[4]; // 0x17
    std::string text = composite_page(TextSection::Win, view.pick);
    std::string town;
    if (view.level < 12 && text_->size(TextSection::TownNames) > 0) {
        int index = std::min(view.level + 1, int(text_->size(TextSection::TownNames)) - 1);
        town = std::string(text_->at(TextSection::TownNames, index));
    }
    std::string formatted = format_args(text, view.hospital_name, std::to_string(view.next_level_offer), town);
    return wrap(formatted, kLetterWidth, font);
}

std::string WinLoseScreen::make_letter(const WinLoseView &view) const
{
    return letter_text(view);
}

bool WinLoseScreen::over(int x, int y, int w, int h, const UiMouse &mouse) const
{
    return mouse.x >= x && mouse.x < x + w && mouse.y >= y && mouse.y < y + h;
}

void WinLoseScreen::draw_letter()
{
    const Font &font = fonts_[4];
    // The typed-out letter, at the letter box, clipped to its width.
    std::string shown = letter_.substr(0, size_t(typed_));
    draw_text(surface_, font, 0, 0, shown,
              {{kLetterBox.x, kLetterBox.y, kLetterBox.w + 0x64},
               {kLetterBox.x, 0, kLetterBox.w, kUiHeight},
               kTextWrap,
               0,
               nullptr});
    // The accept and decline buttons, with their hover sprites.
    Sprite accept = sprites_.at(1);
    bool over_accept = over(kAccept.x, kAccept.y, accept.width, accept.height, pending_mouse_);
    bool over_decline = over(kDecline.x, kDecline.y, accept.width, accept.height, pending_mouse_);
    draw_sprite(surface_, kAccept.x, kAccept.y, sprites_.at(over_accept ? 2 : 1));
    draw_sprite(surface_, kDecline.x, kDecline.y, sprites_.at(over_decline ? 4 : 3));
}

void WinLoseScreen::win_frame(const UiMouse &mouse, WinLoseResult &result, const WinLoseView &view)
{
    if (letter_.empty())
        letter_ = make_letter(view);
    if (typed_ < int(letter_.size()) && !--type_delay_) {
        typed_++;
        type_delay_ = 1;
        result.sounds.push_back(kSfxWinLoseType);
    }
    draw_letter();
    if (!mouse.left_released)
        return;
    Sprite accept = sprites_.at(1);
    if (over(kAccept.x, kAccept.y, accept.width, accept.height, mouse)) {
        open_ = false;
        result.closed = true;
        result.next_level = level_ >= 12 ? 0 : level_ + 1;
    } else if (over(kDecline.x, kDecline.y, accept.width, accept.height, mouse)) {
        open_ = false;
        result.closed = true;
    } else if (typed_ < int(letter_.size())) {
        typed_ = int(letter_.size()); // a click skips the typing
    }
}

void WinLoseScreen::lose_frame(const UiMouse &mouse, WinLoseResult &result, const WinLoseView &view)
{
    // The losing line (win_lose[text_choice]) centred near the bottom; a line
    // wider than the screen uses the next font slot and y 4, as lose_call.
    int slot = std::clamp(view.lose_font, 0x13, 0x16);
    const Font &font = fonts_[size_t(slot - 0x13)];
    std::string line(text_->at(TextSection::WinLose, view.pick));
    int width = text_width(font, line);
    int y = kLoseTextY;
    const Font *draw_font = &font;
    if (width > kUiWidth && slot < 0x16) {
        draw_font = &fonts_[size_t(slot + 1 - 0x13)];
        width = text_width(*draw_font, line);
        y = 4;
    }
    int x = (kUiWidth - width) / 2;
    draw_text(surface_, *draw_font, x, y, line, {{0, 0, kUiWidth}, {0, 0, kUiWidth, kUiHeight}, 0, 0, nullptr});
    Sprite accept = sprites_.at(1);
    bool over_accept = over(kAccept.x, kAccept.y, accept.width, accept.height, mouse);
    draw_sprite(surface_, kAccept.x, kAccept.y, sprites_.at(over_accept ? 2 : 1));
    if (mouse.left_released && over_accept) {
        open_ = false;
        result.closed = true;
    }
}

WinLoseResult WinLoseScreen::frame(const UiMouse &mouse, const WinLoseKeys &keys, double seconds,
                                   const WinLoseView &view)
{
    WinLoseResult result;
    if (!open_)
        return result;
    UiMouse &m = pending_mouse_;
    bool lp = m.left_pressed, lr = m.left_released;
    m = mouse;
    m.left_pressed |= lp;
    m.left_released |= lr;
    pending_keys_.escape |= keys.escape;
    pending_keys_.space |= keys.space;
    frame_time_ += seconds;
    if (frame_time_ < kGameFrame && typed_ == 0 && letter_.empty())
        return result;
    frame_time_ = std::clamp(frame_time_ - kGameFrame, 0.0, kGameFrame);

    surface_.fill(0);
    if (win_)
        win_frame(m, result, view);
    else
        lose_frame(m, result, view);
    m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
    pending_keys_ = {};
    return result;
}

} // namespace th