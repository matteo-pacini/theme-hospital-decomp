#include "common.h"

#include "core/data/balance.h"
#include "core/data/blocks.h"
#include "core/data/fs.h"
#include "core/data/gamedata.h"
#include "core/data/level.h"
#include "core/data/rnc.h"
#include "core/data/smacker.h"
#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/render/text.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

void test_rnc_rejects_garbage()
{
    const uint8_t junk[] = {'R', 'N', 'C', 1, 0, 0, 0, 4, 0, 0, 0, 0};
    CHECK(!th::rnc_unpack(junk).has_value());
}

void test_missing_data()
{
    // A folder without the game reports every required file.
    auto dir = std::filesystem::temp_directory_path() / "th_enhanced_empty_test_dir";
    std::filesystem::create_directories(dir);
    CHECK(th::missing_data_files(dir).size() == th::required_data_files().size());
    std::filesystem::remove(dir);
}

void test_game_data(const char *dir)
{
    for (std::string_view rel : th::required_data_files()) {
        std::string error;
        auto data = th::load_data_file(dir, rel, &error);
        if (!data)
            std::fprintf(stderr, "%s\n", error.c_str());
        CHECK(data.has_value());
    }
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));

    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(gd.blocks().size() > 0);
    CHECK(gd.panel().size() > 14);
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (level) {
        CHECK(level->player_count >= 1 && level->player_count <= 4);
        auto map = th::build_start_map(*level);
        int walls = 0;
        for (const th::MapCell &c : map) {
            CHECK(th::block_sprite(c.floor_block) >= 0);
            walls += c.wall_n != 0 || c.wall_w != 0;
        }
        CHECK(walls > 0);
    }
}

namespace
{

// "15143" in `font` draws within its width at 10,5 and nowhere else.
void check_balance_text(const th::Font &font)
{
    int glyphs = 0;
    for (char c : std::string_view("15143"))
        glyphs += font.glyph(uint8_t(c)).width;
    int width = th::text_width(font, "15143");
    CHECK(width == glyphs && width > 0);

    th::Surface surf;
    surf.resize(200, 40);
    surf.fill(0);
    th::draw_text(surf, font, 0, 0, "15143", {{10, 5, 0}, {0, 0, 200, 40}});
    int drawn = 0, left = 200, right = -1;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 200; x++) {
            if (surf.row(y)[x] == 0)
                continue;
            drawn++;
            left = std::min(left, x);
            right = std::max(right, x);
            CHECK(y >= 5 && y < 5 + font.line_height());
        }
    }
    CHECK(drawn > 0);
    CHECK(left >= 10 && right < 10 + width);
}

// The panel's date: "%d %s" centred (0x100) in the date box.
void check_date_text(const th::Font &font, const th::LanguageText &text)
{
    th::Surface surf;
    surf.resize(200, 40);
    std::string date = "1 " + std::string(text.month(0));
    int date_width = th::text_width(font, date);
    surf.fill(0);
    th::draw_text(surf, font, 0, 0, date, {{0, 0, 200}, {0, 0, 200, 40}, th::kTextCentre});
    int left = 200;
    int right = -1;
    for (int y = 0; y < 40; y++) {
        for (int x = 0; x < 200; x++) {
            if (surf.row(y)[x] != 0) {
                left = std::min(left, x);
                right = std::max(right, x);
            }
        }
    }
    CHECK(left >= (200 - date_width) / 2 && right < (200 - date_width) / 2 + date_width);
}

} // namespace

void test_language_and_fonts(const char *dir)
{
    th::LanguageText text;
    std::string error;
    CHECK(th::load_language(dir, th::kEnglish, text, &error));
    CHECK(text.section_count() == 0x44);
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    CHECK(text.size(th::TextSection::Months) == 12);
    for (int i = 0; i < 12; i++)
        CHECK(text.month(i) == months[i]);
    CHECK(text.month(12).empty());
    auto language = th::config_language(dir);
    CHECK(!language || (*language >= th::kEnglish && *language <= th::kSwedish));

    th::Font balance_font, date_font;
    CHECK(th::load_font(dir, 5, balance_font, &error));
    CHECK(th::load_font(dir, 6, date_font, &error));
    CHECK(!balance_font.empty() && !date_font.empty());
    check_balance_text(balance_font);
    check_date_text(date_font, text);
}

namespace
{

std::span<const uint8_t> text(std::string_view s)
{
    return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(s.data()), s.size());
}

void check_balance_files_read(const char *dir)
{
    // Every shipped file reads to its end.
    for (const char *id : {"EASY", "FULL", "HARD"}) {
        for (int level = 0; level <= 12; level++) {
            char rel[32];
            std::snprintf(rel, sizeof rel, "levels/%s%02d.sam", id, level);
            auto file = th::load_data_file(dir, rel);
            CHECK(file);
            th::Balance b;
            CHECK(file && th::read_balance_file(*file, b));
        }
    }

    for (int level = 1; level <= 12; level++) {
        th::Balance b;
        std::string error;
        CHECK(th::load_balance(dir, level, "", b, &error));
        CHECK(error.empty());
    }
}

// levels/FULL00.SAM, then FULL01.SAM over it.
void check_full_level1(const char *dir)
{
    th::Balance b;
    CHECK(th::load_balance(dir, 1, th::kDefaultBalanceId, b));
    CHECK(b.staff[0].min_salary == 60 && b.staff[1].min_salary == 75);
    CHECK(b.gbv.salary_add[3] == -30 && b.gbv.salary_add[4] == 30);
    CHECK(b.towns[1].start_cash == 40000 && b.towns[1].ill_rate == 2 && b.towns[1].interest_rate == 100);
    CHECK(b.towns[1].inflation_rate == 450); // no file sets it: the static value
    CHECK(b.towns[3].start_cash == 50000 && b.towns[3].interest_rate == 300);
    CHECK(b.gbv.hold_visual_peep_count == 2); // FULL00 says 6
    CHECK(b.gbv.open_countdown == 5000);
    CHECK(b.expertise[2].start_price == 850 && b.expertise[2].max_diag_diff == 200);
    CHECK(b.popn[0].month == 0 && b.popn[0].change == 3);
    CHECK(b.popn[4].month == 21 && b.popn[4].change == 0);
    CHECK(b.popn[5].month == 11 && b.popn[5].change == 0); // static, carried
    CHECK(b.win_criteria[4].criteria == 6 && b.win_criteria[4].value == 55000 && b.win_criteria[4].group == 1);
    CHECK(b.lose_criteria[1].value == -20000 && b.lose_criteria[1].group == 2 && b.lose_criteria[1].bound == 0);
    CHECK(b.staff_levels[1].seed == 8654 && b.staff_levels[1].receptionists == 6);
    CHECK(b.computer[2].playing == 1);
    CHECK(b.awards_trophies.cures_penalty == -3000);
    // balance_control: visuals 0 (5) and 3 (3), four non-visuals (19).
    CHECK(b.visuals_total == 8 && b.visuals[2] == 5 && b.visuals[13] == 8);
    CHECK(b.non_visuals_total == 19);
    CHECK(b.illness_kinds.size() == 22 && b.illness_kinds[0] == 2 && b.illness_kinds[1] == 5 &&
          b.illness_kinds[2] == 16);
    CHECK(b.objects[39].available_for_level == 1 && b.expertise[1].known == 1);
}

void check_other_balances(const char *dir)
{
    th::Balance hard;
    CHECK(th::load_balance(dir, 1, "HARD", hard));
    CHECK(hard.towns[1].start_cash == 30000 && hard.towns[1].interest_rate == 500 && hard.staff[0].min_salary == 70);

    th::Balance missing;
    std::string error;
    CHECK(!th::load_balance(dir, 13, "NONE", missing, &error) && !error.empty());
    // The bonus rat levels have no balance file of their own.
    th::Balance bonus;
    CHECK(th::load_balance(dir, 13, "FULL", bonus));
    CHECK(!th::load_balance(dir, 12, "NONE", missing, &error));

    CHECK(th::balance_description(dir, "FULL", 0) == " Doctor (Medium)");
    CHECK(th::balance_description(dir, "EASY", 2) == " AIPler (Einfach)");
    CHECK(!th::balance_description(dir, "NONE", 0));
}

void check_balance_parsing()
{
    // A value out of bounds: the rest of its line is still read, the rest of
    // the file is not.
    th::Balance s;
    CHECK(!th::read_balance_file(text("#staff[0].MinSalary 70\r\n"
                                      "#towns[2].StartCash.IllRate.InterestRate 100001 7 321\r\n"
                                      "#staff[1].MinSalary 71\r\n"),
                                 s));
    CHECK(s.staff[0].min_salary == 70 && s.staff[1].min_salary == 270);
    CHECK(s.towns[2].start_cash == 50010 && s.towns[2].ill_rate == 7 && s.towns[2].interest_rate == 321);
    // Unknown keys and indexes above the bound stop it too; the bound is
    // inclusive, and an index past the table (popn has 30) is passed over.
    th::Balance u;
    CHECK(!th::read_balance_file(text("#popn[30].Month 9\r\n#gbv.Nothing 1\r\n#staff[0].MinSalary 70\r\n"), u));
    CHECK(u.staff[0].min_salary == 150);
    CHECK(!th::read_balance_file(text("#popn[31].Month 9\r\n#staff[0].MinSalary 70\r\n"), u));
    CHECK(u.staff[0].min_salary == 150);
    // Lines are taken to end in CRLF: LF-only lines lose their last
    // character, and an empty one ends the file. Text before '#' is ignored.
    th::Balance lf;
    CHECK(th::read_balance_file(text("note #staff[0].MinSalary 70\n#gbv.SalaryAdd[5] -45\r\n\n#staff[2].MinSalary 9\n"),
                                lf));
    CHECK(lf.staff[0].min_salary == 7 && lf.gbv.salary_add[5] == -45 && lf.staff[2].min_salary == 95);
}

} // namespace

void test_balance(const char *dir)
{
    check_balance_files_read(dir);
    check_full_level1(dir);
    check_other_balances(dir);
    check_balance_parsing();
}

// The intro movie decodes whole: every frame, its palette and its audio
// (22 kHz stereo 16-bit, as long as the frames).
void test_smacker_intro(const char *dir)
{
    auto path = th::resolve_ci(dir, th::movie_info(th::kMovieIntroSm4).file);
    CHECK(path.has_value());
    if (!path)
        return;
    th::SmackerVideo video;
    std::string error;
    CHECK(video.open(*path, &error));
    CHECK(video.width() == 320 && video.height() == 200 && video.frame_count() == 1559);
    CHECK(video.frame_duration_us() == 57000);
    th::SmackerAudioFormat f = video.audio_format(0);
    CHECK(f.rate == 22050 && f.channels == 2 && f.bits == 16);
    size_t samples = 0;
    int frames = 0;
    while (video.next_frame()) {
        frames++;
        samples += video.audio(0).size();
        if (frames == 1)
            CHECK(video.palette_changed());
        if (frames == 300) {
            std::vector<bool> used(256);
            for (uint8_t p : video.pixels())
                used[p] = true;
            CHECK(std::count(used.begin(), used.end(), true) > 64);
        }
    }
    CHECK(frames == 1559 && video.frame_index() == 1558);
    // 1559 frames of 57 ms.
    CHECK(samples / 2 > 22050 * 88 && samples / 2 < 22050 * 90);
}
