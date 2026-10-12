#include "core/ui/fade.h"

#include "core/ui/startup.h"

#include <algorithm>

namespace th
{

namespace
{
constexpr int kFadeSteps = 10;      // fade_out_menu, the frame loop's fade_palette
constexpr int kReturnFadeSteps = 5; // return_from_screen (src/screens.cpp:76)
} // namespace

void PaletteFade::start(Kind kind, int steps, double period)
{
    kind_ = kind;
    steps_ = std::max(steps, 1);
    period_ = period;
    elapsed_ = 0;
    running_ = true;
}

void PaletteFade::advance(double seconds)
{
    if (running_)
        elapsed_ += seconds;
}

int PaletteFade::step() const
{
    // Counted in whole blanks; the epsilon keeps a run of exact frame times
    // from landing a blank short.
    return std::min(steps_, int(elapsed_ / period_ + 1e-6));
}

float PaletteFade::brightness() const
{
    float n = float(step()) / float(steps_);
    return kind_ == Kind::Out ? 1.0f - n : n;
}

FadePlan fade_between(const ScreenId &from, const ScreenId &to)
{
    if (from == to || from.kind == FadeScreen::Setup || to.kind == FadeScreen::Setup)
        return {};
    // The about screen is drawn over the title picture, without a reload.
    if (from.kind == FadeScreen::Startup && to.kind == FadeScreen::Startup) {
        const int title = int(StartupScreen::Title), about = int(StartupScreen::About);
        if ((from.sub == title && to.sub == about) || (from.sub == about && to.sub == title))
            return {};
    }
    if (from.kind == FadeScreen::WinLoseMovie && to.kind == FadeScreen::WinLoseLetter)
        return {};
    if (to.kind == FadeScreen::Movie || to.kind == FadeScreen::WinLoseMovie)
        return {kFadeSteps, 0};
    if (to.kind == FadeScreen::Hospital)
        return {kReturnFadeSteps, kFadeSteps};
    return {kFadeSteps, kFadeSteps};
}

int fade_component(int from, int to, int count, int steps)
{
    return (to - from) * count / steps + from;
}

} // namespace th
