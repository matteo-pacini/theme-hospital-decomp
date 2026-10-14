#include "core/ui/yearend.h"

#include "core/data/gamedata.h"
#include "core/render/blit.h"
#include "core/render/text.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

namespace th
{

namespace
{

// play_sound's sfx numbers (include/enums.h).
constexpr uint8_t kSfxYesX = 0x41;
constexpr uint8_t kSfxNo4 = 0x42;
constexpr uint8_t kSfxUpDown = 0x47;

// The league tables shown (chart_in_use, src/tables.cpp).
constexpr std::array<bool, 12> kChartInUse = {true,  true,  false, true, true, false,
                                              false, false, false, true, true, false};
constexpr int kChartCount = 11;

// The league table layout (ScreenCoords 0x97-0x99, src/scoords.cpp).
constexpr int kTableX[2] = {137, 327};
constexpr int kTableY[3] = {98, 204, 310};
constexpr int kTableWidth = 175;
constexpr int kTableHeight = 106;

// ScreenCoords 0x9a-0xa6: the award and trophy slots and the plaque frames.
constexpr Point kAwardSlot[6] = {{28, 87}, {28, 202}, {28, 317}, {537, 87}, {537, 202}, {537, 317}};
constexpr Point kPlaqueGood[3] = {{206, 87}, {206, 200}, {206, 304}};
constexpr Point kPlaqueBad = {301, 343};
constexpr Point kPlaqueTrophy[3] = {{206, 161}, {206, 233}, {206, 321}};

// ScreenCoords 0xbf-0xcd: the hall of fame's rows, columns and title.
constexpr int kHallRowY[10] = {156, 181, 206, 231, 256, 281, 306, 331, 356, 381};
constexpr int kHallRankX = 216;
constexpr int kHallNameX = 258;
constexpr int kHallScoreX = 362;
constexpr int kHallTitleY = 103;

// The year-end buttons (screenbuttons 0x48-0x57): hotspot, the sprite drawn
// pressed and where (src/scoords.cpp).
struct ScreenButton {
    int x, y, w, h;
    int sprite;
};
constexpr ScreenButton kYearEndButtons[] = {
    {609, 449, 26, 26, 1},  // 0x48 close (league tables)
    {272, 436, 89, 40, 3},  // 0x49 show the awards page
    {},                     // 0x4a
    {609, 449, 26, 26, 1},  // 0x4b close (awards)
    {273, 368, 89, 40, 3},  // 0x4c show the league tables
    {369, 358, 26, 26, 2},  // 0x4d close the good-award plaque
    {389, 378, 26, 26, 2},  // 0x4e close the bad-award plaque
    {16, 76, 97, 95, 0},    // 0x4f award 0
    {16, 191, 97, 95, 0},   // 0x50 award 1
    {16, 306, 97, 95, 0},   // 0x51 award 2
    {525, 76, 97, 95, 0},   // 0x52 award 3
    {525, 191, 97, 95, 0},  // 0x53 award 4
    {525, 306, 97, 95, 0},  // 0x54 award 5
    {142, 324, 61, 144, 0}, // 0x55 trophy 0
    {407, 324, 60, 144, 0}, // 0x56 trophy 1
    {466, 331, 72, 145, 0}, // 0x57 trophy 2
};

// The hall of fame buttons (screenbuttons 0xa9-0xab): show fame, show shame,
// close.
constexpr ScreenButton kHallButtons[] = {
    {137, 217, 40, 40, 2},
    {456, 217, 40, 40, 2},
    {609, 449, 26, 26, 1},
};
constexpr int kHallButtonCount = int(std::size(kHallButtons));

// The screen sprites the awards and plaque are drawn with (award03v).
constexpr int kSpriteTrophy[3] = {12, 13, 14};
constexpr int kSpriteAward[6] = {4, 5, 6, 7, 8, 9};
constexpr int kSpriteAward6th = 24;
constexpr int kSpriteGoodArrow = 10;
constexpr int kSpriteBadArrow = 11;
constexpr int kSpriteTrophyButton = 13;
constexpr int kSpritePlaqueBad = 15;
constexpr int kSpritePlaqueGood[3] = {16, 17, 18};
constexpr int kSpritePlaqueTrophy[4] = {19, 20, 21, 22};

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks
constexpr int kIdleFrames = 0x4b0;  // display_year_charts' 1200 frames
constexpr int kHallIdleFrames = 1200;

// A full-screen picture: 640 x 480 palette indices and its VGA palette.
bool load_picture(const std::filesystem::path &dir, std::string_view name, Surface &pixels,
                  std::array<uint8_t, 768> &vga, Palette &palette, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/" + std::string(name) + ".dat", error);
    if (!dat)
        return false;
    auto pal = load_data_file(dir, "qdata/" + std::string(name) + ".pal", error);
    if (!pal)
        return false;
    if (dat->size() < size_t(kUiWidth) * kUiHeight || pal->size() < vga.size()) {
        if (error)
            *error = std::string(name) + ": not a full-screen picture";
        return false;
    }
    pixels.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(pixels.row(y), dat->data() + size_t(y) * kUiWidth, kUiWidth);
    std::memcpy(vga.data(), pal->data(), vga.size());
    palette_from_vga(vga, palette);
    return true;
}

bool load_sheet(const std::filesystem::path &dir, std::string_view name, SpriteSheet &sheet, std::string *error)
{
    auto tab = load_data_file(dir, "qdata/" + std::string(name) + ".tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/" + std::string(name) + ".dat", error) : std::nullopt;
    if (!dat)
        return false;
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error)
            *error = std::string(name) + ": not a sprite table";
        return false;
    }
    return true;
}

} // namespace

HospScores default_hosp_scores()
{
    HospScores s;
    s.fame = {{{"WEBLEY", 250000},
               {"SAMUEL", 210000},
               {"JAYNE", 200000},
               {"GAZZA", 190000},
               {"CHIVERS", 175000},
               {"RAJ", 165000},
               {"WAZ", 145000},
               {"BEN", 99000},
               {"LORD RIDER", 79000},
               {"BASSEY", 40000}}};
    s.shame = {{{"JAN SV.", 8000},
                {"COGGY", 10000},
                {"BRUTUS", 11000},
                {"NUTTERS", 12500},
                {"ROBBO", 16000},
                {"NATS", 16500},
                {"MR MAT", 17000},
                {"JON", 18000},
                {"ADIE", 19000},
                {"RUSS", 25000}}};
    return s;
}

bool load_highscores(const std::filesystem::path &path, HospScores &scores)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return false;
    auto read_score = [&](HighScore &h) {
        char name[0x20] = {};
        int32_t score = 0;
        in.read(name, sizeof name);
        in.read(reinterpret_cast<char *>(&score), sizeof score);
        if (in)
            h = {std::string(name), score};
    };
    for (HighScore &h : scores.fame)
        read_score(h);
    for (HighScore &h : scores.shame)
        read_score(h);
    return bool(in) || in.eof();
}

bool save_highscores(const std::filesystem::path &path, const HospScores &scores)
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
        return false;
    auto write_score = [&](const HighScore &h) {
        char name[0x20] = {};
        std::memcpy(name, h.name.data(), std::min(h.name.size(), sizeof name - 1));
        out.write(name, sizeof name);
        out.write(reinterpret_cast<const char *>(&h.score), sizeof h.score);
    };
    for (const HighScore &h : scores.fame)
        write_score(h);
    for (const HighScore &h : scores.shame)
        write_score(h);
    return bool(out);
}

namespace
{

void sort_fame(HospScores &s)
{
    std::stable_sort(s.fame.begin(), s.fame.end(),
                     [](const HighScore &a, const HighScore &b) { return a.score > b.score; });
}
void sort_shame(HospScores &s)
{
    std::stable_sort(s.shame.begin(), s.shame.end(),
                     [](const HighScore &a, const HighScore &b) { return a.score < b.score; });
}
int find_entry(const std::array<HighScore, 10> &table, std::string_view name)
{
    for (int i = 0; i < 10; i++)
        if (table[size_t(i)].name == name)
            return i;
    return 10;
}

} // namespace

bool insert_hall_details(HospScores &scores, std::string_view name, int32_t score)
{
    if (score > scores.fame[9].score) {
        int i = find_entry(scores.fame, name);
        if (i == 10) {
            scores.fame[9] = {std::string(name), score};
            sort_fame(scores);
        } else if (score > scores.fame[size_t(i)].score) {
            scores.fame[size_t(i)] = {std::string(name), score};
            sort_fame(scores);
        }
        return true;
    }
    if (score > scores.shame[0].score) {
        int i = find_entry(scores.shame, name);
        if (i == 10) {
            scores.shame[9] = {std::string(name), score};
            sort_shame(scores);
        } else if (score > scores.shame[size_t(i)].score) {
            scores.shame[size_t(i)] = {std::string(name), score};
            sort_shame(scores);
        }
    }
    return false;
}

bool YearEndScreen::load(const std::filesystem::path &dir, const LanguageText &text, const Palette &game_palette,
                         std::string *error)
{
    text_ = &text;
    game_palette_ = game_palette;
    bool ok = true;
    ok &= load_picture(dir, "fame01v", fame_.pixels, fame_.vga, fame_.palette, error);
    ok &= load_sheet(dir, "fame02v", fame_.sprites, error);
    ok &= load_picture(dir, "award01v", awards1_.pixels, awards1_.vga, awards1_.palette, error);
    ok &= load_sheet(dir, "award03v", awards1_.sprites, error);
    ok &= load_picture(dir, "award02v", awards2_.pixels, awards2_.vga, awards2_.palette, error);
    ok &= load_sheet(dir, "award03v", awards2_.sprites, error);
    for (int i = 0; i < 8; i++)
        ok &= load_font(dir, 0x10 + i, fonts_[size_t(i)], error);
    surface_.resize(kUiWidth, kUiHeight);
    return ok;
}

void YearEndScreen::open(const HospScores &scores, bool showing_fame)
{
    open_ = true;
    phase_ = 0;
    scores_ = scores;
    showing_fame_ = showing_fame;
    hall_state_ = 0;
    charts_state_ = 0;
    hall_idle_ = 0;
    charts_idle_ = 0;
    current_display_ = 0;
    current_award_ = 0;
    exit_screen_ = false;
    last_mouse_sum_ = -1;
    frame_time_ = 0;
    pending_mouse_ = {};
    pending_keys_ = {};
    text_pos_.fill(0);
    desired_text_pos_.fill(0);
    title_pos_.fill(0);
    desired_title_pos_.fill(0);
    awards_won_ = {};
    trophies_won_ = {};
}

void YearEndScreen::use_picture(int index)
{
    Picture *p = index == 0 ? &fame_ : index == 1 ? &awards1_ : &awards2_;
    active_ = p;
    palette_ = p->palette;
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(surface_.row(y), p->pixels.data() + size_t(y) * kUiWidth, kUiWidth);
}

Sprite YearEndScreen::sprite(int index) const
{
    return active_ ? active_->sprites.at(size_t(index)) : Sprite{};
}

void YearEndScreen::draw_sprite_at(int x, int y, int index)
{
    draw_sprite(surface_, x, y, sprite(index));
}

void YearEndScreen::draw_text_at(const Font &font, int x, int y, std::string_view text, unsigned flags, Rect clip)
{
    if (clip.w == 0)
        clip = {0, 0, kUiWidth, kUiHeight};
    draw_text(surface_, font, x, y, text, {{0, 0, kUiWidth}, clip, flags, 0, nullptr});
}

bool YearEndScreen::over(int x, int y, int w, int h, const UiMouse &mouse) const
{
    return mouse.x >= x && mouse.x < x + w && mouse.y >= y && mouse.y < y + h;
}

// ---- Hall of fame (src/score.cpp) ----

void YearEndScreen::set_hall_pos(bool fame)
{
    const Font &font = fonts_[0];
    for (int i = 0; i < 10; i++)
        desired_text_pos_[size_t(i)] = text_pos_[size_t(i)] = (fame ? kHallRowY[i] : kHallRowY[9 - i]) << 16;
    std::string title = std::string(text_->at(TextSection::Hall, fame ? 4 : 5));
    title_ = title;
    title_len_ = int(title.size());
    int full = text_width(font, title);
    for (int i = 0; i < 20 && i < title_len_; i++) {
        int x;
        if (fame) {
            x = (kUiWidth - full) / 2 + text_width(font, std::string_view(title).substr(0, size_t(i)));
        } else {
            x = (kUiWidth + full) / 2 - text_width(font, std::string_view(title).substr(size_t(title_len_ - 1 - i)));
        }
        desired_title_pos_[size_t(i)] = title_pos_[size_t(i)] = x << 16;
    }
}

void YearEndScreen::draw_hall_labels()
{
    const Font &font = fonts_[0];
    // modify_background: hall_text[i] at ScreenCoords[i + 200] while y is set;
    // 201-203 are the rank, name and score headers at y 131.
    constexpr int kLabelX[3] = {kHallRankX, kHallNameX, kHallScoreX};
    for (int i = 1; i <= 3; i++)
        draw_text_at(font, kLabelX[i - 1], 131, text_->at(TextSection::Hall, i));
}

void YearEndScreen::move_text()
{
    for (int i = 0; i < 10; i++) {
        int32_t &p = text_pos_[size_t(i)];
        p += (desired_text_pos_[size_t(i)] - p + 0x1000) >> 3;
        if (p > desired_text_pos_[size_t(i)] && p - desired_text_pos_[size_t(i)] < 0x800)
            p = desired_text_pos_[size_t(i)];
        if (p < desired_text_pos_[size_t(i)] && desired_text_pos_[size_t(i)] - p < 0x800)
            p = desired_text_pos_[size_t(i)];
    }
    bool old = showing_fame_;
    showing_fame_ = text_pos_[0] < text_pos_[5];
    if (showing_fame_ != old)
        title_ = std::string(text_->at(TextSection::Hall, showing_fame_ ? 4 : 5));
    for (int i = 0; i < 20; i++) {
        int32_t &p = title_pos_[size_t(i)];
        p += (desired_title_pos_[size_t(i)] - p + 0x1000) >> 3;
        if (p > desired_title_pos_[size_t(i)] && p - desired_title_pos_[size_t(i)] < 0x800)
            p = desired_title_pos_[size_t(i)];
        if (p < desired_title_pos_[size_t(i)] && desired_title_pos_[size_t(i)] - p < 0x800)
            p = desired_title_pos_[size_t(i)];
    }
}

void YearEndScreen::draw_fame_text()
{
    const Font &font = fonts_[0];
    int len = title_len_;
    for (int i = 0; i < len && i < 20; i++) {
        int x = (title_pos_[size_t(i)] + 0x8000) >> 16;
        std::string_view letter = showing_fame_ ? std::string_view(title_).substr(size_t(i), 1)
                                                : std::string_view(title_).substr(size_t(len - 1 - i), 1);
        draw_text_at(font, x, kHallTitleY, letter);
    }
    char buf[16];
    for (int i = 0; i < 10; i++) {
        int y = (text_pos_[size_t(i)] + 0x8000) >> 16;
        std::snprintf(buf, sizeof buf, "%2d", i + 1);
        draw_text_at(font, kHallRankX, y, buf);
        const HighScore &h = showing_fame_ ? scores_.fame[size_t(i)] : scores_.shame[size_t(9 - i)];
        draw_text_at(font, kHallNameX, y, h.name);
        std::snprintf(buf, sizeof buf, "%6d", h.score);
        draw_text_at(font, kHallScoreX, y, buf);
    }
}

void YearEndScreen::hall_open()
{
    use_picture(0);
    draw_hall_labels();
    hall_state_ = 1;
    hall_idle_ = kHallIdleFrames;
    for (int i = 0; i < 20; i++)
        title_pos_[size_t(i)] = desired_title_pos_[size_t(i)] = kUiWidth / 2;
    set_hall_pos(showing_fame_);
    move_text();
    draw_fame_text();
}

void YearEndScreen::hall_draw()
{
    use_picture(0);
    draw_hall_labels();
    move_text();
    draw_fame_text();
}

void YearEndScreen::process_hall_button(int button, YearEndResult &result)
{
    if (button == 0 || button == 1) {
        result.sounds.push_back(kSfxUpDown);
        set_hall_pos(!showing_fame_);
    } else if (button == 2) {
        result.sounds.push_back(kSfxNo4);
        hall_state_ = 2;
    }
}

void YearEndScreen::hall_buttons(const UiMouse &mouse, YearEndResult &result)
{
    for (int i = 0; i < kHallButtonCount; i++) {
        const ScreenButton &b = kHallButtons[i];
        if (!over(b.x, b.y, b.w, b.h, mouse))
            continue;
        if (mouse.left_pressed)
            draw_sprite_at(b.x, b.y, b.sprite);
        if (mouse.left_released)
            process_hall_button(i, result);
        break;
    }
}

YearEndResult YearEndScreen::hall_frame(const UiMouse &mouse, const YearEndKeys &keys, double seconds,
                                        std::string_view player_name, int32_t player_score)
{
    YearEndResult result;
    (void)player_name;
    (void)player_score;
    if (!open_ || phase_ != 0)
        return result;
    UiMouse &m = pending_mouse_;
    bool lp = m.left_pressed, rp = m.right_pressed, lr = m.left_released, rr = m.right_released;
    m = mouse;
    m.left_pressed |= lp;
    m.right_pressed |= rp;
    m.left_released |= lr;
    m.right_released |= rr;
    pending_keys_.escape |= keys.escape;
    pending_keys_.space |= keys.space;
    frame_time_ += seconds;
    if (frame_time_ < kGameFrame && hall_state_ != 0)
        return result;
    frame_time_ = std::clamp(frame_time_ - kGameFrame, 0.0, kGameFrame);

    switch (hall_state_) {
    case 0:
        hall_open();
        break;
    case 1:
        hall_draw();
        break;
    case 2:
        result.hall_done = true;
        break;
    default:
        break;
    }
    hall_buttons(m, result);
    if (m.left_pressed || m.left_released)
        m.left_pressed = m.left_released = false;
    if (last_mouse_sum_ != m.x + m.y) {
        hall_idle_ = kHallIdleFrames;
        last_mouse_sum_ = m.x + m.y;
    }
    if (!hall_idle_--)
        hall_state_ = 2;
    m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
    pending_keys_ = {};
    return result;
}

// ---- Year-end league tables, awards and plaque (src/annualch.cpp) ----

void YearEndScreen::charts_open()
{
    use_picture(2);
    charts_state_ = 2;
    charts_idle_ = kIdleFrames;
    league_draw();
}

void YearEndScreen::league_draw()
{
    const Font &font = fonts_[0];
    int height = font.glyph(65).height;
    use_picture(2);
    // The title: help_text[20] and the year (mycontrol->year + 1999).
    char buf[100];
    std::snprintf(buf, sizeof buf, "%s %4d", std::string(text_->at(TextSection::Help, 20)).c_str(), view_year_ + 0x7cf);
    draw_text_at(font, 0, 0, buf, kTextCentre, {0xe0, 0x24 - height / 2, 0xc0, 0x16});

    for (int i = 0, cat = -1; i < 6; i++) {
        for (cat++; cat < kChartCount && !kChartInUse[size_t(cat)]; cat++) {
        }
        if (cat == kChartCount)
            break;
        int x = kTableX[i % 2], y = kTableY[i / 2];
        draw_text_at(font, 0, 0, text_->at(TextSection::ChartTypes, cat), 0, {x, y, kTableWidth, kTableHeight});
        int row = height;
        for (int j = 0; j < 4; j++) {
            const ChartEntry &e = (*view_chart_)[size_t(cat)][size_t(j)];
            row += height;
            draw_text_at(font, 0, row, (*view_names_)[e.hosp], kTextWrap, {x, y, kTableWidth, kTableHeight});
            char value[24];
            std::snprintf(value, sizeof value, "%d", e.value);
            draw_text_at(font, 0, row, value, kTextRight, {x, y, kTableWidth, kTableHeight});
        }
    }
}

void YearEndScreen::collect_awards_won()
{
    int trophy_count = 0;
    for (int i = 0; i < 9 && trophy_count < 3; i++) {
        const Trophy &t = (*view_trophies_)[size_t(i)];
        if (t.id)
            trophies_won_[size_t(trophy_count++)] = t;
    }
    int award_count = 0;
    for (int i = 0; i < 13 && award_count < 6; i++) {
        const Award &a = (*view_awards_)[size_t(i)];
        if (a.text_index)
            awards_won_[size_t(award_count++)] = a;
    }
}

std::string YearEndScreen::bonus_line(std::string_view bonus, int32_t value)
{
    return std::string(bonus) + (value > 0 ? "\n+" : "\n") + std::to_string(value);
}

std::string YearEndScreen::plaque_text(std::string_view text, int number)
{
    std::string out(text);
    size_t at = out.find("%d");
    if (at != std::string::npos)
        out.replace(at, 2, std::to_string(number));
    return out;
}

void YearEndScreen::draw_plaque()
{
    const Font &font = fonts_[2]; // 0x12
    switch (current_display_) {
    case 1:
        for (int i = 0; i < 3; i++)
            draw_sprite_at(kPlaqueGood[i].x, kPlaqueGood[i].y, kSpritePlaqueGood[i]);
        [[fallthrough]];
    case 2: {
        if (current_display_ == 2)
            draw_sprite_at(kPlaqueBad.x, kPlaqueBad.y, kSpritePlaqueBad);
        const Award &a = awards_won_[size_t(current_award_)];
        std::string_view text =
            text_->at(current_display_ == 1 ? TextSection::GoodAwards : TextSection::BadAwards, a.text_index);
        std::string text2 = bonus_line(text_->at(TextSection::TrophyBonus, a.reward_kind), a.value);
        draw_text_at(font, 0, 0, text, 0, {0xee, 0x73, 0x9a, 0xbb});
        draw_text_at(font, 0, 0, text2, 0, {0xee, 0x12e, 0x9a, 0x26});
        break;
    }
    case 3: {
        draw_sprite_at(kPlaqueTrophy[0].x, kPlaqueTrophy[0].y, kSpritePlaqueTrophy[0]);
        draw_sprite_at(kPlaqueTrophy[1].x, kPlaqueTrophy[1].y, kSpritePlaqueTrophy[1]);
        draw_sprite_at(kPlaqueTrophy[2].x, kPlaqueTrophy[2].y, kSpritePlaqueTrophy[2]);
        const Font &tf = fonts_[1]; // 0x11
        const Trophy &t = trophies_won_[size_t(current_award_)];
        std::string text1(text_->at(TextSection::Trophy, t.text_index));
        if (t.id == 5)
            text1 = plaque_text(text1, view_rat_kills_);
        std::string text2(text_->at(TextSection::TrophyBonus, t.reward_kind));
        std::string text3 = "+" + std::to_string(t.value);
        draw_text_at(tf, 0, 0, text1, 0, {0xe1, 0x6a, 0xb4, 0xe0});
        draw_text_at(tf, 0, 0, text2, 0, {0xd7, 0x150, 0xce, 0x14});
        draw_text_at(tf, 0, 0, text3, 0, {0xd7, 0x172, 0xce, 0x14});
        break;
    }
    default:
        break;
    }
    if (trophies_won_[1].id)
        draw_sprite_at(kYearEndButtons[14].x, kYearEndButtons[14].y, kSpriteTrophyButton);
}

void YearEndScreen::awards_open()
{
    use_picture(1);
    current_display_ = 0;
    current_award_ = 0;
    charts_state_ = 3;
    charts_idle_ = kIdleFrames;
}

void YearEndScreen::awards_draw()
{
    use_picture(1);
    collect_awards_won();
    int trophy_count = 0;
    for (int i = 0; i < 3; i++) {
        if (!trophies_won_[size_t(i)].id)
            continue;
        draw_sprite_at(kYearEndButtons[13 + trophy_count].x, kYearEndButtons[13 + trophy_count].y,
                       kSpriteTrophy[trophy_count]);
        trophy_count++;
    }
    int award_count = 0;
    for (int i = 0; i < 6; i++) {
        if (!awards_won_[size_t(i)].text_index)
            continue;
        const ScreenButton &b = kYearEndButtons[7 + award_count];
        draw_sprite_at(b.x, b.y, award_count == 5 ? kSpriteAward6th : kSpriteAward[award_count]);
        draw_sprite_at(b.x, b.y, 23);
        if (awards_won_[size_t(i)].direction == 1)
            draw_sprite_at(kAwardSlot[award_count].x, kAwardSlot[award_count].y, kSpriteGoodArrow);
        else
            draw_sprite_at(kAwardSlot[award_count].x, kAwardSlot[award_count].y, kSpriteBadArrow);
        award_count++;
    }
    draw_plaque();
}

bool YearEndScreen::button_available(int button) const
{
    if (button == 0 || button == 1)
        return true;
    if (button == 2)
        return current_display_ == 1 || current_display_ == 2;
    if (button == 3)
        return current_display_ == 3;
    if (button >= 4 && button <= 9)
        return awards_won_[size_t(button - 4)].text_index != 0;
    if (button >= 10 && button <= 12)
        return trophies_won_[size_t(button - 10)].text_index != 0;
    return false;
}

void YearEndScreen::process_yearend_button(int button, YearEndResult &result)
{
    switch (button) {
    case 0:
        result.sounds.push_back(kSfxNo4);
        exit_screen_ = true;
        charts_state_ = 4;
        break;
    case 1:
        result.sounds.push_back(kSfxYesX);
        if (charts_state_ == 3)
            charts_state_ = 0;
        else if (charts_state_ == 2)
            charts_state_ = 1;
        break;
    case 2:
    case 3:
        result.sounds.push_back(kSfxYesX);
        current_display_ = 0;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        result.sounds.push_back(kSfxYesX);
        current_display_ = awards_won_[size_t(button - 4)].direction == 1    ? 1
                           : awards_won_[size_t(button - 4)].direction == -1 ? 2
                                                                             : 0;
        current_award_ = button - 4;
        break;
    case 10:
    case 11:
    case 12:
        result.sounds.push_back(kSfxYesX);
        current_display_ = 3;
        current_award_ = button - 10;
        break;
    default:
        break;
    }
}

void YearEndScreen::league_buttons(const UiMouse &mouse, YearEndResult &result)
{
    if (exit_screen_)
        draw_sprite_at(kYearEndButtons[0].x, kYearEndButtons[0].y, kYearEndButtons[0].sprite);
    for (int i = 0; i < 2; i++) {
        const ScreenButton &b = kYearEndButtons[i];
        if (!over(b.x, b.y, b.w, b.h, mouse))
            continue;
        if (mouse.left_pressed)
            draw_sprite_at(b.x, b.y, b.sprite);
        if (mouse.left_released)
            process_yearend_button(i, result);
        break;
    }
}

void YearEndScreen::awards_buttons(const UiMouse &mouse, YearEndResult &result)
{
    if (exit_screen_)
        draw_sprite_at(kYearEndButtons[3].x, kYearEndButtons[3].y, kYearEndButtons[3].sprite);
    for (int i = 0; i < 13; i++) {
        if (!button_available(i))
            continue;
        const ScreenButton &b = kYearEndButtons[3 + i];
        if (!over(b.x, b.y, b.w, b.h, mouse))
            continue;
        if (mouse.left_pressed)
            draw_sprite_at(b.x, b.y, b.sprite);
        if (mouse.left_released)
            process_yearend_button(i, result);
        break;
    }
}

void YearEndScreen::set_view(const YearEndView &view)
{
    view_year_ = view.year;
    view_chart_ = view.chart;
    view_names_ = view.hospital_names;
    view_awards_ = view.awards;
    view_trophies_ = view.trophies;
    view_rat_kills_ = view.plaque_rat_kills;
}

void YearEndScreen::charts_input(UiMouse &mouse, YearEndResult &result)
{
    if ((charts_state_ == 2 || charts_state_ == 3) && (pending_keys_.escape || pending_keys_.space)) {
        result.sounds.push_back(kSfxNo4);
        exit_screen_ = true;
        charts_state_ = 4;
    }
    if (last_mouse_sum_ != mouse.x + mouse.y) {
        charts_idle_ = kIdleFrames;
        last_mouse_sum_ = mouse.x + mouse.y;
    }
    if (!charts_idle_-- && charts_state_ != 4)
        process_yearend_button(0, result);
}

YearEndResult YearEndScreen::charts_frame(const UiMouse &mouse, const YearEndKeys &keys, double seconds,
                                          const YearEndView &view)
{
    YearEndResult result;
    if (!open_ || phase_ != 1)
        return result;
    if (!view.chart || !view.awards || !view.trophies || !view.hospital_names)
        return result;
    set_view(view);

    UiMouse &m = pending_mouse_;
    bool lp = m.left_pressed, rp = m.right_pressed, lr = m.left_released, rr = m.right_released;
    m = mouse;
    m.left_pressed |= lp;
    m.right_pressed |= rp;
    m.left_released |= lr;
    m.right_released |= rr;
    pending_keys_.escape |= keys.escape;
    pending_keys_.space |= keys.space;
    frame_time_ += seconds;
    if (frame_time_ < kGameFrame && charts_state_ != 0)
        return result;
    frame_time_ = std::clamp(frame_time_ - kGameFrame, 0.0, kGameFrame);

    switch (charts_state_) {
    case 0:
        charts_open();
        break;
    case 1:
        awards_open();
        break;
    case 2:
        league_draw();
        league_buttons(m, result);
        break;
    case 3:
        awards_draw();
        awards_buttons(m, result);
        break;
    case 4:
        result.charts_done = true;
        break;
    default:
        break;
    }
    charts_input(m, result);
    m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
    pending_keys_ = {};
    return result;
}

} // namespace th