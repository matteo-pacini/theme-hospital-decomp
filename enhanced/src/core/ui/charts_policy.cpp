// The policy and research charts of Frame: the policy sliders and the
// research allocation (src/policy.cpp, src/rschscrn.cpp).

#include "core/ui/charts_internal.h"

#include <algorithm>

namespace th
{

inline namespace charts_detail
{

void Frame::display_policy()
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        c_.value = 0;
        process_policy_sliders();
        left_ = mleft_ = rleft_ = false;
        break;
    case 1:
        copy_screen();
        process_policy_panel();
        process_policy_sliders();
        break;
    case 2:
        c_.state = 3;
        break;
    case 3:
        return_from_screen();
        break;
    }
    if (exit_quick_requester())
        c_.state = 2;
}

void Frame::process_policy_panel()
{
    if (!rleft_)
        return;
    for (int i = kPolicyButtons; kButtons[i].w && kButtons[i].h; i++) {
        const ScreenButton &b = kButtons[i];
        if (!over(mx_, my_, b.x, b.y, b.w, b.h))
            continue;
        int action = i - kPolicyButtons;
        if (action == 0 || action == 1)
            r_.event = {EV_SET_POLICY, 5, 0, uint16_t(action == 0 ? 1 : 0)};
        else
            c_.state = 2;
        sound(kSfxSelectX);
    }
    rleft_ = false;
}

void Frame::process_policy_sliders()
{
    draw_policy_sliders();
    if (!mleft_) {
        c_.value = 0;
        return;
    }
    drag_policy_slider();
}

// process_policy_sliders, its drawing: the names, the four sliders at
// the settings, the staff-leave-rooms switch.
void Frame::draw_policy_sliders()
{
    for (int i = 0; i < 5; i++)
        text(7, kBoxes[0x20 + i], {0, 0}, str(TextSection::PolicyNames, i), i == 0 ? kTextCentre : kTextWrap);
    struct Slider {
        int box, sprite, x;
    };
    const Slider sliders[4] = {
        {0x25, 2, std::clamp(hs_.guess_cure_threshold * 3, 0, 299)},
        {0x26, 1, std::clamp(hs_.send_home_threshold * 3, 0, 299)},
        {0x27, 3, std::clamp((hs_.diag_threshold - 1000) / 0x42 * 20, 0, 299)},
        {0x28, 3, std::clamp(hs_.rest_threshold / 0x27 * 10, 0, 0xfb)},
    };
    for (int i = 0; i < 4; i++) {
        const Slider &s = sliders[i];
        const Rect &b = kBoxes[s.box];
        Sprite spr = sprite(s.sprite);
        reqspr(b.x + s.x, b.y, s.sprite);
        Rect window = {b.x + s.x + kCoords[0x54].x, b.y + kCoords[0x54].y, spr.width, spr.height};
        text(8, window, {0, 0}, str(TextSection::PolicyNames, 5 + i), kTextCentre);
    }
    if (hs_.staff_leave_rooms)
        reqspr(kCoords[0x52].x, kCoords[0x52].y, 4);
    else
        reqspr(kCoords[0x53].x, kCoords[0x53].y, 5);
}

// process_policy_sliders while the left button is held: the slider
// grabbed, and its setting following the mouse.
void Frame::drag_policy_slider()
{
    // Which slider is dragged (pol.value): 2 guess a cure, 1 send home,
    // 3 diagnosis, 4 rest; 0x63 none.
    if (c_.value == 0) {
        c_.value = 0x63;
        const int order[4][2] = {{0x25, 2}, {0x26, 1}, {0x27, 3}, {0x28, 4}};
        for (const auto &o : order) {
            if (over(mx_, my_, 0, kBoxes[o[0]].y, kUiWidth, kBoxes[o[0]].h)) {
                c_.value = o[1];
                break;
            }
        }
    }
    struct Drag {
        int value, box, sprite, divisor, step, max, offset, top;
    };
    const Drag drags[4] = {{2, 0x25, 2, 3, 1, 100, 0, 100},
                           {1, 0x26, 1, 3, 1, 100, 0, 100},
                           {3, 0x27, 3, 20, 0x42, 1000, 1000, 2000},
                           {4, 0x28, 3, 10, 0x27, 1000, 0, 1000}};
    for (const Drag &d : drags) {
        if (c_.value != d.value)
            continue;
        const Rect &b = kBoxes[d.box];
        int half = sprite(d.sprite).width / 2;
        uint16_t which = uint16_t(d.value);
        if (over(mx_, my_, half + b.x, 0, half + b.w, kUiHeight)) {
            int x = std::clamp((mx_ - half - b.x) / d.divisor * d.step, 0, d.max);
            r_.event = {EV_SET_POLICY, which, 0, uint16_t(x + d.offset)};
        }
        if (over(mx_, my_, 0, 0, b.x, kUiHeight))
            r_.event = {EV_SET_POLICY, which, 0, uint16_t(d.offset)};
        if (over(mx_, my_, b.x + b.w + half, 0, kUiWidth - (half + b.w), kUiHeight))
            r_.event = {EV_SET_POLICY, which, 0, uint16_t(d.top)};
    }
}

void Frame::display_research()
{
    int frame_no = int(game_.turn_count & 0xf) / 2 + 1;
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 1;
        c_.value = 0;
        r_.research_sound = true;
        break;
    case 1:
        copy_screen();
        break;
    case 2:
        r_.research_stop = true;
        return_from_screen();
        break;
    }
    reqspr(kCoords[85].x, kCoords[85].y, frame_no + 12);
    reqspr(kCoords[86].x, kCoords[86].y, frame_no + 4);
    process_res_panel();
    draw_research_text();
    left_ = rleft_ = rright_ = false;
    if (exit_quick_requester())
        c_.state = 2;
}

// A category's progress bar, once it has worked towards something
// (Research::cost): as the game, every bar measures the points banked
// for the next new cure machine (choose_next_research_illness by
// find_new_cure_cost) against that category's cost.
void Frame::draw_research_bar(int i)
{
    const Research &r = game_.world.research;
    if (r.cost[size_t(i)] <= 0)
        return;
    uint8_t illness = choose_next_research_illness(game_.world, ResearchCost::NewCure);
    if (!illness)
        return;
    auto len = int16_t(r.illness_research[illness] * 0x122 / r.cost[size_t(i)]);
    len = std::clamp<int16_t>(len, 0, 0x122);
    for (int x = 0; x < len; x += 5)
        reqspr(kCoords[i + 0x5c].x + x, kCoords[i + 0x5c].y, 3);
}

void Frame::draw_research_text()
{
    const int f = 9;
    for (int i = 0; i < 5; i++) {
        text(f, kCoords[i + 0x57].x, kCoords[i + 0x57].y, str(TextSection::ResearchNames, i));
        if (hs_.research_alloc[size_t(i)] >= 0) {
            text(f, kCoords[i + 0x61].x, kCoords[i + 0x61].y, format("%d", hs_.research_alloc[size_t(i)]));
            draw_research_bar(i);
        } else {
            const Rect &b = kBoxes[i + 0x29];
            draw_box(dst_, b.x, b.y, b.w, b.h, 0xb);
            text(f, kCoords[i + 0x61].x, kCoords[i + 0x61].y, text_.misc(0x14));
        }
    }
    uint8_t total = 0;
    for (int8_t a : hs_.research_alloc)
        if (a > 0)
            total = uint8_t(total + a);
    text(f, kCoords[0x66].x, kCoords[0x66].y, str(TextSection::ResearchNames, 6));
    text(f, kCoords[0x67].x, kCoords[0x67].y, format("%d", total));
}

void Frame::reset_repeat()
{
    c_.repeat_step = 0x100;
    c_.repeat_time = c_.clock_ms;
}

int Frame::next_repeat_step()
{
    c_.repeat_step = c_.repeat_step < 0xa00 ? c_.repeat_step + 0x10 : 0xa00;
    return c_.repeat_step >> 8;
}

void Frame::process_res_panel()
{
    if (rleft_) {
        for (int i = kResearchButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                if (i == kResearchButtons)
                    process_res_button(0);
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
            }
        }
        reset_repeat();
        c_.value = 0;
        rleft_ = false;
    }
    if (mleft_) {
        for (int i = kResearchButtons; kButtons[i].w && kButtons[i].h; i++) {
            const ScreenButton &b = kButtons[i];
            if (over(mx_, my_, b.x, b.y, b.w, b.h)) {
                if (i != kResearchButtons)
                    process_res_button(i - kResearchButtons);
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
                break;
            }
        }
    } else {
        reset_repeat();
    }
}

void Frame::process_res_button(int button)
{
    if (button == 0) {
        c_.state = 2;
        sound(kSfxNo4);
        return;
    }
    bool down = button <= 5;
    size_t cat = size_t(down ? button - 1 : button - 6);
    int alloc = hs_.research_alloc[cat];
    if (alloc < 0)
        return;
    int others = 0;
    for (size_t i = 0; i < 5; i++)
        if (i != cat && hs_.research_alloc[i] >= 0)
            others += hs_.research_alloc[i];
    bool can = down ? alloc > 0 : alloc + others < 100;
    if (!can) {
        if (!c_.value) {
            sound(kSfxWrong2);
            c_.value = 1;
        }
        return;
    }
    if (!repeat_due())
        return;
    int step = next_repeat_step();
    int arg = down ? std::max(alloc - step, 0) : std::min(alloc + step, 100 - others);
    r_.event = {EV_ALTER_RESEARCH, uint16_t(cat), 0, uint16_t(arg)};
    sound(kSfxSelectX);
}

} // namespace charts_detail

} // namespace th
