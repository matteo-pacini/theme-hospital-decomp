// The year-end screens (src/annualch.cpp, src/score.cpp) and the win/lose
// letters (src/winlose.cpp): their art loads, their state machines advance,
// and their buttons send the commands the game expects.

#include "common.h"

#include "core/data/gamedata.h"
#include "core/ui/winlose.h"
#include "core/ui/yearend.h"
#include "core/world/year_end.h"

#include <cstdint>
#include <filesystem>
#include <string>

namespace
{

th::YearEndView make_view(const th::YearEnd &progress, const std::array<std::string, 4> &names)
{
    th::YearEndView view;
    view.year = 1;
    view.level = 1;
    view.awards = &progress.awards;
    view.trophies = &progress.trophies;
    view.chart = &progress.chart;
    view.hospital_names = &names;
    return view;
}

// The "."-ended page `page` of `sec` joined into one string (concat_composite).
std::string join_page(const th::LanguageText &text, th::TextSection sec, int page)
{
    int current = 0;
    std::string out;
    for (int i = 0; i < int(text.size(sec)); i++) {
        std::string_view s = text.at(sec, i);
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

// The number of %s/%d specifiers in `s` (%% and a lone % count as none).
int spec_count(const std::string &s)
{
    int n = 0;
    for (size_t i = 0; i + 1 < s.size(); i++) {
        if (s[i] != '%')
            continue;
        if (s[i + 1] == 's' || s[i + 1] == 'd') {
            n++;
            i++;
        }
    }
    return n;
}

} // namespace

void test_hall_scores_table()
{
    th::HospScores scores = th::default_hosp_scores();
    CHECK(scores.fame[0].name == "WEBLEY");
    CHECK(scores.fame[0].score == 250000);
    CHECK(scores.shame[0].score == 8000);

    // A score that beats the hall of fame's lowest goes in and sorts.
    CHECK(th::insert_hall_details(scores, "NEWDOC", 100000));
    CHECK(scores.fame[0].name == "WEBLEY");
    bool found = false;
    for (const th::HighScore &h : scores.fame)
        found = found || h.name == "NEWDOC";
    CHECK(found);

    // A score below the fame table but above the shame table goes in shame.
    th::HospScores low = th::default_hosp_scores();
    CHECK(!th::insert_hall_details(low, "BAD", 20000));
    bool in_shame = false;
    for (const th::HighScore &h : low.shame)
        in_shame = in_shame || h.name == "BAD";
    CHECK(in_shame);

    // Saving and loading round-trips.
    std::filesystem::path path = std::filesystem::temp_directory_path() / "th_test_hiscore.dat";
    CHECK(th::save_highscores(path, scores));
    th::HospScores read = th::default_hosp_scores();
    CHECK(th::load_highscores(path, read));
    CHECK(read.fame[0].name == scores.fame[0].name && read.fame[0].score == scores.fame[0].score);
    std::filesystem::remove(path);
}

void test_yearend_screens(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    th::YearEndScreen screen;
    CHECK(screen.load(dir, gd.text(), gd.palette(), &error));

    th::HospScores scores = th::default_hosp_scores();
    th::insert_hall_details(scores, "PLAYER", 100000);
    screen.open(scores, true);
    CHECK(screen.is_open());

    // The hall advances to its done state after its idle countdown.
    bool hall_done = false;
    for (int i = 0; i < 1400 && !hall_done; i++)
        hall_done = screen.hall_frame({}, {}, 0.05, "PLAYER", 100000).hall_done;
    CHECK(hall_done);

    // The charts: the league tables, then the awards page, then close.
    screen.set_phase(1);
    th::YearEnd progress;
    progress.score = 100000;
    progress.chart[0][0] = {12345, 0};
    std::array<std::string, 4> names = {"PLAYER", "RIVAL1", "RIVAL2", "RIVAL3"};
    th::YearEndView view = make_view(progress, names);

    // A frame opens the league tables.
    th::YearEndResult r = screen.charts_frame({}, {}, 0.05, view);
    CHECK(!r.charts_done);

    // The switch button (0x49) opens the awards page: it must not finish the
    // charts.
    th::UiMouse press;
    press.x = 300;
    press.y = 450;
    press.left_pressed = true;
    r = screen.charts_frame(press, {}, 0.05, view);
    CHECK(!r.charts_done);

    // The close button (0x48) finishes the charts on release.
    th::UiMouse release;
    release.x = 615;
    release.y = 455;
    release.left_released = true;
    r = screen.charts_frame(release, {}, 0.05, view);
    for (int i = 0; i < 10 && !r.charts_done; i++)
        r = screen.charts_frame({}, {}, 0.05, view);
    CHECK(r.charts_done);
}

void test_winlose_screen(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    th::WinLoseScreen screen;
    CHECK(screen.load(dir, gd.text(), gd.palette(), &error));

    screen.open(true, 1);
    CHECK(screen.is_open());
    th::WinLoseView view;
    view.win = true;
    view.level = 1;
    view.score = 100000;
    view.next_level_offer = 50000;
    view.pick = 0;
    view.hospital_name = "PLAYER";

    // The letter types out over frames; then the accept button starts level 2.
    th::WinLoseResult r;
    for (int i = 0; i < 400 && !r.next_level; i++) {
        th::UiMouse mouse;
        if (i == 399) {
            mouse.x = 400;
            mouse.y = 410;
            mouse.left_released = true;
        }
        r = screen.frame(mouse, {}, 0.05, view);
    }
    CHECK(r.next_level == 2);
    CHECK(!screen.is_open());
}

// Every win letter variant formats as the original's sprintf would, and the
// wrapped letter carries the hospital, the offer and a newline.
void test_winlose_letters(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    const th::LanguageText &t = gd.text();
    for (int page = 0; page < 12; page++) {
        std::string raw = join_page(t, th::TextSection::Win, page);
        CHECK(!raw.empty());
        std::string got = th::WinLoseScreen::format_args(raw, "HOSP", "12345", "TOWN");
        char ref[4096] = {};
        int specs = spec_count(raw);
        CHECK(specs == 2 || specs == 3);
        if (specs == 3)
            std::snprintf(ref, sizeof ref, raw.c_str(), "HOSP", 12345, "TOWN");
        else
            std::snprintf(ref, sizeof ref, raw.c_str(), "HOSP", 12345);
        CHECK(got == ref);
    }

    th::WinLoseScreen screen;
    CHECK(screen.load(dir, t, gd.palette(), &error));
    th::WinLoseView view;
    view.level = 1;
    view.next_level_offer = 50000;
    view.pick = 0;
    view.hospital_name = "HOSP";
    std::string letter = screen.letter_text(view);
    CHECK(letter.find("HOSP") != std::string::npos);
    CHECK(letter.find("50000") != std::string::npos);
    CHECK(letter.find('\n') != std::string::npos);
}