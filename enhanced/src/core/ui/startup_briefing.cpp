// The level briefing of Startup: the area picker and the briefing text
// (process_keys_area_select, src/area.cpp).

#include "core/ui/startup_internal.h"

#include "core/data/balance.h"
#include "core/data/text.h"

namespace th
{

namespace
{

// The 21 level areas of the briefing when any level can be picked
// (HiresBoxes[45] to [65], src/coords.cpp): x, y, width, height.
struct AreaBox {
    int x, y, w, h;
};
constexpr int kAreaCount = 21;
constexpr AreaBox kAreaBoxes[kAreaCount] = {
    {123, 333, 142, 65}, {339, 312, 68, 96}, {439, 194, 96, 54}, {359, 48, 64, 77},  {193, 58, 72, 70},
    {134, 194, 93, 58},  {243, 252, 63, 91}, {350, 225, 81, 69}, {356, 141, 85, 59}, {290, 91, 56, 80},
    {209, 146, 80, 50},  {282, 177, 77, 71}, {434, 49, 90, 85},  {139, 21, 60, 54},  {73, 47, 79, 43},
    {73, 132, 157, 40},  {70, 13, 161, 261}, {167, 164, 50, 52}, {183, 77, 31, 38},  {41, 141, 176, 45},
    {79, 207, 146, 59},
};
} // namespace

void Startup::briefing(const UiMouse &mouse, std::span<const StartupKey> keys)
{
    // process_keys_area_select: Escape goes back to the main menu, C lets any
    // level be picked.
    for (const StartupKey &k : keys) {
        if (k.code == StartupKey::Escape) {
            state_ = State::MenuEnter;
            return;
        }
        if (k.code == StartupKey::Letter && k.letter == 'C')
            pick_any_level_ = true;
    }
    if (pick_any_level_ && pick_level(mouse))
        return;
    // process_mouse_area_select: a click, or 600 frames without the mouse
    // moving, starts the level (level 1 for a new game). While any level can
    // be picked a click is spent on the areas alone.
    bool start = mouse.left_pressed && !pick_any_level_;
    if (last_mouse_sum_ != mouse.x + mouse.y) {
        idle_ = 600;
        last_mouse_sum_ = mouse.x + mouse.y;
    }
    if (int16_t(idle_--) == 0)
        start = true;
    if (start)
        start_level();
}

bool Startup::pick_level(const UiMouse &mouse)
{
    if (!mouse.left_pressed)
        return false;
    for (int i = 0; i < kAreaCount; i++) {
        const AreaBox &b = kAreaBoxes[i];
        if (mouse.x < b.x || mouse.x >= b.x + b.w || mouse.y < b.y || mouse.y >= b.y + b.h)
            continue;
        level_ = i + 1;
        pick_any_level_ = false;
        // The original plays animation_table[level + 7], which for level 21
        // is past its 28 entries: that level starts without a movie.
        if (kMovieArea + level_ < kMovieCount)
            play_movie(kMovieArea + level_, State::AreaStart);
        else
            start_level();
        return true;
    }
    return false;
}

void Startup::start_level()
{
    if (level_ == 0)
        level_ = 1;
    result_ = {};
    result_.kind = StartupResult::Kind::StartLevel;
    result_.level = level_;
    result_.balance_id = balance_files_.empty() ? std::string(kDefaultBalanceId) : balance_files_[balance_];
    result_.tutorial = tutorial_;
    screen_ = StartupScreen::Done;
    state_ = State::Done;
}

std::vector<std::string_view> level_briefing(const LanguageText &text, int level)
{
    std::vector<std::string_view> page;
    int n = int(text.size(TextSection::Level));
    int i = 0;
    for (int p = 0; p < level; p++) {
        while (i < n && text.at(TextSection::Level, i) != ".") {
            if (text.at(TextSection::Level, i) == "..")
                return page;
            i++;
        }
        i++;
    }
    for (; i < n; i++) {
        std::string_view s = text.at(TextSection::Level, i);
        if (s == "." || s == "..")
            break;
        page.push_back(s);
    }
    return page;
}

} // namespace th
