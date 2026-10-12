// The palette fade's timing and rules, the loading screen's picture and the
// screen shot file.

#include "common.h"

#include "core/ui/fade.h"
#include "core/ui/loading_screen.h"
#include "core/ui/startup.h"

#include <vector>

using namespace th;

void test_palette_fade()
{
    // LbPaletteFade: palette n of `steps` is the difference scaled by
    // n / steps, truncated towards zero, on the start.
    CHECK(fade_component(63, 0, 0, 10) == 63);
    CHECK(fade_component(63, 0, 1, 10) == 57);
    CHECK(fade_component(63, 0, 10, 10) == 0);
    CHECK(fade_component(0, 63, 3, 10) == 18);
    CHECK(fade_component(0, 63, 10, 10) == 63);

    // Ten steps of a 60 Hz blank, one frame each: black after the tenth.
    PaletteFade out;
    out.start(PaletteFade::Kind::Out, 10, PaletteFade::kPeriod);
    CHECK(out.step() == 0 && out.brightness() == 1.0f);
    for (int i = 1; i <= 10; i++) {
        out.advance(PaletteFade::kPeriod);
        CHECK(out.step() == i);
        CHECK(out.done() == (i == 10));
    }
    CHECK(out.brightness() == 0.0f);

    PaletteFade in;
    in.start(PaletteFade::Kind::In, 10, PaletteFade::kPeriod);
    CHECK(in.brightness() == 0.0f);
    in.advance(5 * PaletteFade::kPeriod);
    CHECK(in.step() == 5 && in.brightness() == 0.5f);
    in.advance(1.0);
    CHECK(in.done() && in.brightness() == 1.0f);
}

void test_fade_between()
{
    auto id = [](FadeScreen kind, int sub = 0) { return ScreenId{kind, sub}; };
    const ScreenId hospital = id(FadeScreen::Hospital);
    // A chart or the town map is loaded with a fade in and out of 10 steps,
    // and left for the hospital in 5 and 10.
    FadePlan open = fade_between(hospital, id(FadeScreen::Chart, 3));
    CHECK(open.out_steps == 10 && open.in_steps == 10);
    FadePlan leave = fade_between(id(FadeScreen::Chart, 3), hospital);
    CHECK(leave.out_steps == 5 && leave.in_steps == 10);
    FadePlan other = fade_between(id(FadeScreen::Chart, 3), id(FadeScreen::Chart, 4));
    CHECK(other.out_steps == 10 && other.in_steps == 10);
    CHECK(fade_between(hospital, hospital).out_steps == 0);

    // A movie sets its own palette; the letter reuses the movie's picture; the
    // name box is drawn over the title.
    FadePlan movie = fade_between(hospital, id(FadeScreen::WinLoseMovie));
    CHECK(movie.out_steps == 10 && movie.in_steps == 0);
    FadePlan letter = fade_between(id(FadeScreen::WinLoseMovie), id(FadeScreen::WinLoseLetter));
    CHECK(letter.out_steps == 0 && letter.in_steps == 0);
    ScreenId title = id(FadeScreen::Startup, int(StartupScreen::Title));
    ScreenId name = id(FadeScreen::Startup, int(StartupScreen::Name));
    ScreenId menu = id(FadeScreen::Startup, int(StartupScreen::MainMenu));
    CHECK(fade_between(title, name).out_steps == 0);
    CHECK(fade_between(name, menu).out_steps == 10);
    CHECK(fade_between(id(FadeScreen::Setup), hospital).out_steps == 0);
}

void test_loading_screen(const char *dir)
{
    LoadingScreen screen;
    std::string error;
    CHECK(load_loading_screen(dir, screen, &error));
    CHECK(screen.pixels.width() == 640 && screen.pixels.height() == 480);
    // A picture, not a blank: it uses several colours.
    bool varied = false;
    for (int y = 0; y < 480 && !varied; y++)
        for (int x = 1; x < 640; x++)
            if (screen.pixels.row(y)[x] != screen.pixels.row(y)[0]) {
                varied = true;
                break;
            }
    CHECK(varied);
}
