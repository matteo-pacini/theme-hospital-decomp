#include "app/screen_fade.h"

#include <cmath>
#include <format>
#include <utility>

namespace th
{

namespace
{
constexpr int kFadeSteps = 10; // display_loading_screen, clear_loading_screen
}

ScreenFader::~ScreenFader()
{
    release();
}

void ScreenFader::release()
{
    for (SDL_Texture *&t : targets_) {
        if (t)
            SDL_DestroyTexture(t);
        t = nullptr;
    }
    steps_.clear();
    fade_.stop();
}

void ScreenFader::queue(PaletteFade::Kind kind, int steps, Source source)
{
    steps_.push_back({kind, steps, source});
}

void ScreenFader::screen(const ScreenId &now)
{
    ScreenId before = last_;
    bool had = std::exchange(have_last_, true);
    last_ = now;
    // A change while fades are queued is the one they belong to.
    if (!enabled_ || !had || !steps_.empty() || fade_.running())
        return;
    FadePlan plan = fade_between(before, now);
    if (plan.out_steps)
        queue(PaletteFade::Kind::Out, plan.out_steps, Source::Shown);
    if (plan.in_steps)
        queue(PaletteFade::Kind::In, plan.in_steps, Source::Next);
}

void ScreenFader::queue_level_start()
{
    if (!enabled_)
        return;
    queue(PaletteFade::Kind::Out, kFadeSteps, Source::Shown);
    queue(PaletteFade::Kind::In, kFadeSteps, Source::Loading);
    queue(PaletteFade::Kind::Out, kFadeSteps, Source::Loading);
    queue(PaletteFade::Kind::In, kFadeSteps, Source::Next);
}

void ScreenFader::queue_boot(bool fade_in_first_screen)
{
    if (!enabled_)
        return;
    queue(PaletteFade::Kind::In, kFadeSteps, Source::Loading);
    queue(PaletteFade::Kind::Out, kFadeSteps, Source::Loading);
    if (fade_in_first_screen)
        queue(PaletteFade::Kind::In, kFadeSteps, Source::Next);
}

bool ScreenFader::blocking() const
{
    if (fade_.running())
        return true;
    return !steps_.empty() && steps_.front().source != Source::Next;
}

std::string ScreenFader::state() const
{
    if (!fade_.running() && steps_.empty())
        return "none";
    return std::format("{} {}, {} queued",
                       fade_.running() ? (fade_.kind() == PaletteFade::Kind::Out ? "out" : "in") : "waiting",
                       fade_.step(), steps_.size());
}

void ScreenFader::make_targets(SDL_Renderer *renderer, int width, int height)
{
    if (targets_[0] && width == width_ && height == height_)
        return;
    const bool resized = targets_[0] != nullptr;
    for (SDL_Texture *&t : targets_) {
        if (t)
            SDL_DestroyTexture(t);
        t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
        if (t)
            SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
    }
    width_ = width;
    height_ = height;
    current_ = 0;
    // The frame shown is gone with its target, and so are the fades on it.
    shown_ = -1;
    if (resized) {
        steps_.clear();
        fade_.stop();
    }
}

void ScreenFader::show(SDL_Renderer *renderer, SDL_Texture *target) const
{
    if (!target)
        return;
    uint8_t v = uint8_t(std::lround(fade_.running() ? fade_.brightness() * 255.0f : 255.0f));
    SDL_SetTextureColorMod(target, v, v, v);
    SDL_RenderTexture(renderer, target, nullptr, nullptr);
}

bool ScreenFader::present(SDL_Renderer *renderer, int width, int height, double seconds,
                          const std::function<void()> &draw_loading)
{
    make_targets(renderer, width, height);
    if (!fade_.running()) {
        if (steps_.empty() || steps_.front().source == Source::Next)
            return false;
        const Step &s = steps_.front();
        if (s.source == Source::Loading) {
            SDL_Texture *target = targets_[current_];
            SDL_SetRenderTarget(renderer, target);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            draw_loading();
            SDL_SetRenderTarget(renderer, nullptr);
            shown_ = current_;
            current_ ^= 1;
        }
        fade_.start(s.kind, s.steps, PaletteFade::kPeriod);
    } else {
        fade_.advance(seconds);
    }
    show(renderer, shown_ >= 0 ? targets_[shown_] : nullptr);
    if (fade_.done()) {
        fade_.stop();
        steps_.pop_front();
    }
    return true;
}

void ScreenFader::begin_frame(SDL_Renderer *renderer, int width, int height)
{
    make_targets(renderer, width, height);
    SDL_SetRenderTarget(renderer, targets_[current_]);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void ScreenFader::end_frame(SDL_Renderer *renderer)
{
    SDL_SetRenderTarget(renderer, nullptr);
    shown_ = current_;
    current_ ^= 1;
    // A fade waiting for the next frame starts on this one.
    if (!fade_.running() && !steps_.empty()) {
        const Step &s = steps_.front();
        fade_.start(s.kind, s.steps, PaletteFade::kPeriod);
    }
    show(renderer, targets_[shown_]);
}

} // namespace th
