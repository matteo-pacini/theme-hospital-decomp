// The title screen's changelog (core/ui/changelog.h) and the about screen of
// Startup that shows it (core/ui/startup.h).

#include "common.h"

#include "core/data/text.h"
#include "core/render/surface.h"
#include "core/render/text.h"
#include "core/ui/changelog.h"
#include "core/ui/layer.h"
#include "core/ui/startup.h"
#include "th_changelog.h"
#include "th_version.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{

// Runs the start-up screens a frame at a time.
struct AboutRun {
    th::Startup &s;

    void run(int frames, std::vector<th::StartupKey> keys = {}, bool click = false)
    {
        th::UiMouse m;
        m.x = m.y = 5;
        m.left_pressed = m.left_released = click;
        for (int i = 0; i < frames; i++) {
            s.frame(m, keys, 0.04);
            keys.clear();
            m.left_pressed = m.left_released = false;
        }
    }
    // From the intro, skipped, to the about screen's bare picture.
    void reach_about()
    {
        run(1);
        run(1, {{th::StartupKey::Space, 0}});
        run(10); // the input guard after a skip
        run(1, {}, true);
        run(3);
    }
};

} // namespace

void test_changelog_text(const char *)
{
    const std::string md = "# Changelog\n\n## v1.0.0 and `code`\n\nFirst **line**\nsecond [link](http://x).\n\n\n"
                           "- one\n  more\n- two\n\nEnd\n";
    const std::vector<std::string> want = {"CHANGELOG", "",           "v1.0.0 AND CODE", "", "First line second link.",
                                           "",          "- one more", "- two",           "", "End"};
    CHECK(th::changelog_paragraphs(md) == want);
    CHECK(th::changelog_paragraphs("").empty());
    CHECK(th::changelog_paragraphs("\n\n").empty());
    // A heading may carry a date, shown as written.
    const std::vector<std::string> dated = {"v1.0.0 - 2026-10-20", "", "Text"};
    CHECK(th::changelog_paragraphs("## v1.0.0 - 2026-10-20\n\nText\n") == dated);
    // The project's own file reduces to a heading and some bullets; the
    // version's heading is undated or dated.
    std::vector<std::string> own = th::changelog_paragraphs(th::kChangelogMarkdown);
    CHECK(own.size() > 5 && own[0] == "CHANGELOG");
    const std::string head = std::string("v") + th::kVersion;
    bool version = false;
    for (const std::string &p : own)
        version |= p == head || p.starts_with(head + " - ");
    CHECK(version);

    CHECK(th::changelog_clamp_scroll(-5, 100, 40) == 0);
    CHECK(th::changelog_clamp_scroll(30, 100, 40) == 30);
    CHECK(th::changelog_clamp_scroll(90, 100, 40) == 60);
    CHECK(th::changelog_clamp_scroll(10, 30, 40) == 0);
}

void test_changelog_wrap(const char *dir)
{
    th::Font font;
    CHECK(th::load_font(dir, th::kChangelogFontSlot, font));
    // Every character the changelog draws puts pixels on the screen: a
    // missing glyph is a blank of the glyph's width.
    for (const std::string &p : th::changelog_paragraphs(th::kChangelogMarkdown)) {
        for (char c : p) {
            const auto u = uint8_t(c);
            if (u <= 32)
                continue;
            th::Surface cell;
            cell.resize(40, 40);
            cell.fill(0);
            const std::string one(1, c);
            th::draw_text(cell, font, 0, 0, one, {{0, 0, 40}, {0, 0, 40, 40}});
            bool inked = false;
            for (int y = 0; y < 40 && !inked; y++)
                for (int x = 0; x < 40; x++)
                    inked |= cell.row(y)[x] != 0;
            if (!inked)
                std::fprintf(stderr, "changelog: no glyph for '%c' in font slot 0x%02x\n", c, th::kChangelogFontSlot);
            CHECK(inked);
        }
    }
    const std::vector<std::string> paragraphs = th::changelog_paragraphs(th::kChangelogMarkdown);
    const int width = th::kChangelogTextWidth;
    std::vector<th::ChangelogLine> lines = th::changelog_wrap(font, paragraphs, width);
    CHECK(lines.size() > paragraphs.size());
    bool fits = true;
    for (const th::ChangelogLine &l : lines)
        fits &= l.indent + th::text_width(font, l.text) <= width;
    CHECK(fits);
    // A bullet's lines after the first hang under its text.
    bool hung = false;
    for (size_t i = 1; i < lines.size(); i++)
        hung |= lines[i].indent > 0 && lines[i - 1].text.starts_with("- ");
    CHECK(hung);
    // An empty paragraph is a blank line.
    std::vector<th::ChangelogLine> blank = th::changelog_wrap(font, {"a", "", "b"}, width);
    CHECK(blank.size() == 3 && blank[1].text.empty());
}

void test_startup_about_screen(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    AboutRun a{s};
    a.reach_about();
    CHECK(s.screen() == th::StartupScreen::About);
    CHECK(!s.about_box_shown());
    // The bare picture for two seconds, then the box.
    a.run(40);
    CHECK(!s.about_box_shown());
    a.run(20);
    CHECK(s.about_box_shown());
    a.run(60);
    // Escape does nothing here.
    a.run(2, {{th::StartupKey::Escape, 0}});
    CHECK(s.screen() == th::StartupScreen::About);
    // C opens the changelog; the arrow keys and Page Down scroll it by whole
    // lines to its end; Escape closes it only.
    a.run(1, {{th::StartupKey::Letter, 'C'}});
    a.run(60);
    CHECK(s.changelog_open() && s.changelog_scroll() == 0);
    const int max = s.changelog_scroll_max();
    CHECK(max > 0 && max % 12 == 0);
    a.run(1, {{th::StartupKey::Down, 0}});
    CHECK(s.changelog_scroll() == 12);
    a.run(1, {{th::StartupKey::Up, 0}, {th::StartupKey::Up, 0}});
    CHECK(s.changelog_scroll() == 0);
    for (int i = 0; i < 100; i++)
        a.run(1, {{th::StartupKey::PageDown, 0}});
    CHECK(s.changelog_scroll() == max);
    a.run(1, {{th::StartupKey::Escape, 0}});
    a.run(60);
    CHECK(!s.changelog_open() && s.screen() == th::StartupScreen::About);
    // Enter continues to the main menu.
    a.run(1, {{th::StartupKey::Return, 0}});
    a.run(2);
    CHECK(s.screen() == th::StartupScreen::Movie);
    CHECK(s.player_name() == text.misc(1));
    CHECK(s.take_name_confirmed());
}

void test_startup_about_skip(const char *dir)
{
    th::LanguageText text;
    CHECK(th::load_language(dir, 0, text));
    th::Startup s;
    if (!s.load(dir, text, 0))
        return;
    AboutRun a{s};
    a.reach_about();
    CHECK(!s.about_box_shown());
    // A key brings the box up at once and is not also CONTINUE.
    a.run(1, {{th::StartupKey::Return, 0}});
    CHECK(s.about_box_shown());
    a.run(60);
    CHECK(s.screen() == th::StartupScreen::About);
}
