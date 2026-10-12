// The casebook chart of Frame: an illness's figures, cure and drug prices
// (src/drugs.cpp).

#include "core/ui/charts_internal.h"

#include "core/data/balance.h"
#include "core/world/finance.h"

#include <algorithm>

namespace th
{

inline namespace charts_detail
{

bool Frame::casebook_lists(int i) const
{
    const IllnessFigures &f = hs_.illnesses[size_t(i)];
    return (i < kFirstDiagnosis && f.discovered) ||
           (i >= kFirstDiagnosis && game_.world.cure_available[size_t(i)] == 1) ||
           (i == kIllAutopsy && equipment_available(game_.world, kObjAutoAutopsy) &&
            has_room(game_.world, kRoomResearch));
}

// get_next_illness: the illness before (forward 0) or after `from`,
// 0xfa if none.
int Frame::get_next_illness(int forward, int from) const
{
    if (forward == 0) {
        for (int i = from - 1; i > 0; i--)
            if (casebook_lists(i))
                return i;
    } else {
        for (int i = from + 1; i < 0x2e; i++)
            if (casebook_lists(i))
                return i;
    }
    return 0xfa;
}

void Frame::display_drugs()
{
    switch (c_.state) {
    case 0:
        load_pic();
        c_.state = 2;
        c_.value = 0;
        c_.repeat_step = 0x100;
        c_.drug_pct_display_time = 0;
        break;
    case 1:
        return_from_screen();
        break;
    case 2:
        copy_screen();
        break;
    }
    // The figures and the cure shown are the illness selected before
    // this frame's click in the list.
    const int illness = c_.selected_illness;
    display_drugs_figures(illness);
    display_drugs_list();
    text(0xe, {kCoords[0x1e].x, kCoords[0x1e].y, 0x36, 0x1a}, {0, (0x1a - line_height(font(0xe))) / 2},
         str(TextSection::Cabinet, 7), kTextCentre);
    display_drugs_cure(illness);
    if (kExpertise[size_t(illness)].research_type != 0) {
        int f = illness == hs_.research_illness ? 0xf : 0xe;
        text(f, kCoords[0x1d].x, kCoords[0x1d].y, conv_string(str(TextSection::Cabinet, 6), 1));
    }
    process_drug_panel();
    process_drug_keys();
    if (exit_quick_requester())
        c_.state = 1;
}

// display_drugs, the cabinet's headings and the illness's figures.
void Frame::display_drugs_figures(int illness)
{
    for (int i = 0; i < 12; i++)
        text(0xe, kBoxes[0], kCoords[i + 0x11], display_drugs_figure(i, illness), kTextCentre);
}

// display_drugs, the cabinet's ith entry: a heading, or from 6 on a
// figure of `illness`.
std::string Frame::display_drugs_figure(int i, int illness)
{
    const IllnessFigures &fig = hs_.illnesses[size_t(illness)];
    const HospitalMarket &hosp = game_.arrivals.hospitals[0];
    switch (i) {
    case 6:
        return format("%d", illness >= kFirstDiagnosis || illness == kIllGeneralPractice
                                ? game_.reputation()
                                : int(hosp.reputation[size_t(illness)]));
    case 7:
        if (c_.drug_pct_display_time) {
            c_.drug_pct_display_time--;
            return format("%d%%", hosp.price_scale[size_t(illness)] / 100);
        }
        return format("$%d", hosp.price[size_t(illness)]);
    case 8:
        return format("$%d", fig.income);
    case 9:
        return format("%d", fig.cured);
    case 10:
        return format("%d", fig.deaths);
    case 11:
        return format("%d", fig.turned_away);
    default:
        return conv_string(str(TextSection::Cabinet, i), 1);
    }
}

// display_drugs, the listed illnesses: the selected one in the middle,
// up to seven after it and seven before.
std::array<int, 18> Frame::display_drugs_entries() const
{
    std::array<int, 18> list{};
    list[8] = c_.selected_illness;
    int count = 0;
    for (int i = c_.selected_illness + 1; i < 0x2e; i++) {
        if (!casebook_lists(i))
            continue;
        if (count >= 7)
            break;
        list[size_t(count + 9)] = i;
        count++;
    }
    count = 7;
    for (int i = c_.selected_illness - 1; i > 0; i--) {
        const IllnessFigures &f = hs_.illnesses[size_t(i)];
        if (((i < kFirstDiagnosis && f.discovered) ||
             (i >= kFirstDiagnosis && game_.world.cure_available[size_t(i)] == 1)) &&
            i < 0x2c) {
            if (count == 0)
                break;
            list[size_t(count)] = i;
            count--;
        }
    }
    return list;
}

// display_drugs, the list drawn, a click on an entry selecting it.
void Frame::display_drugs_list()
{
    int line_h = line_height(font(0xe));
    const std::array<int, 18> list = display_drugs_entries();
    for (int i = 0; i < 0x11; i++) {
        if (!list[size_t(i)])
            continue;
        std::string s = conv_string(str(TextSection::Illness, list[size_t(i)]), 0);
        int f = 0xe;
        if (i == 8) {
            f = 0xf;
        } else if (left_ && over(mx_, my_, kCoords[i].x, kCoords[i].y, kUiWidth - kCoords[i].y, line_h)) {
            c_.selected_illness = uint8_t(list[size_t(i)]);
            sound(kSfxPageTurn2);
            left_ = false;
        }
        text(f, kCoords[i].x, kCoords[i].y, s);
    }
}

// display_drugs, the cure: whether it is available, and the drug,
// operation or machine it takes.
void Frame::display_drugs_cure(int illness)
{
    if (illness >= kFirstDiagnosis) {
        reqspr(kCoords[0x20].x, kCoords[0x20].y, game_.world.cure_available[size_t(illness)] == 1 ? 11 : 12);
        return;
    }
    if (illness == kIllGeneralPractice)
        return;
    const ExpertiseRow &def = kExpertise[size_t(illness)];
    bool plain = def.outcome != 1 && def.outcome != 5 && def.outcome != 4;
    if (game_.world.cure_available[size_t(illness)] == 1) {
        if (def.kind == kObjPharmacyCabinet) {
            cure_sprite(7);
            drug_effect(format("%d", hs_.illnesses[size_t(illness)].drug_effectiveness));
        } else if (def.category == kRoomTheatre) {
            cure_sprite(8);
        } else if (plain) {
            cure_sprite(6);
        }
        reqspr(kCoords[0x20].x, kCoords[0x20].y, 11);
        return;
    }
    reqspr(kCoords[0x20].x, kCoords[0x20].y, 12);
    if (plain)
        display_drugs_missing_cure(def);
}

// display_drugs, the cure not yet available: what it will take, if the
// level offers it.
void Frame::display_drugs_missing_cure(const ExpertiseRow &def)
{
    if (def.kind == kObjPharmacyCabinet) {
        cure_sprite(7);
        drug_effect("?");
    } else if (equipment_available(game_.world, def.kind)) {
        cure_sprite(6);
        if (def.staff_mask == 0x40)
            cure_sprite(10);
        if (def.category == kRoomTheatre)
            cure_sprite(8);
    } else {
        cure_sprite(9);
    }
}

// The first of the buttons from `first` (to the list's end) under the
// mouse, -1 for none.
int Frame::button_under_mouse(int first) const
{
    for (int i = first; kButtons[i].w && kButtons[i].h; i++) {
        const ScreenButton &b = kButtons[i];
        if (over(mx_, my_, b.x, b.y, b.w, b.h))
            return i;
    }
    return -1;
}

void Frame::process_drug_panel()
{
    if (left_) {
        c_.value = 0;
        int i = button_under_mouse(kDrugButtons);
        if (i >= 0) {
            const ScreenButton &b = kButtons[i];
            process_drug_button(i - kDrugButtons);
            if (b.sprite)
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
        }
        reset_repeat();
    }
    if (mleft_) {
        int i = button_under_mouse(kDrugButtons);
        if (i >= 0) {
            const ScreenButton &b = kButtons[i];
            if (i - kDrugButtons <= 1)
                process_drug_button(i - kDrugButtons);
            if (b.sprite)
                reqspr(b.sprite_x, b.sprite_y, b.sprite);
        }
    }
    left_ = false;
}

// price_up and price_down: the price moved through the command, refused
// once at 200% and 50%.
void Frame::price(bool up)
{
    if (!repeat_due())
        return;
    uint16_t value = uint16_t(c_.repeat_step >> 8);
    next_repeat_step();
    r_.event = {uint8_t(up ? EV_PRICE_UP : EV_PRICE_DOWN), value, 0, c_.selected_illness};
    int scale = game_.arrivals.hospitals[0].price_scale[c_.selected_illness];
    if (up ? scale < 20000 : scale > 5000) {
        sound(kSfxSelectX);
    } else if (!c_.value) {
        sound(kSfxWrong2);
        c_.value = 1;
    }
}

void Frame::select_next(int forward)
{
    int illness = get_next_illness(forward, c_.selected_illness);
    if (illness != 0xfa) {
        c_.selected_illness = uint8_t(illness);
        sound(kSfxPageTurn2);
    } else {
        sound(kSfxWrong2);
    }
}

void Frame::process_drug_keys()
{
    const uint8_t pad = keys_.pad_held;
    if (keys_.up || (pad & ChartKeys::kPadUp))
        select_next(0);
    if (keys_.down || (pad & ChartKeys::kPadDown))
        select_next(1);
    if (keys_.right || (pad & ChartKeys::kPadRight))
        price(true);
    else if (keys_.left || (pad & ChartKeys::kPadLeft))
        price(false);
    else if (!mleft_)
        reset_repeat();
}

void Frame::process_drug_button(int action)
{
    switch (action) {
    case 0:
    case 1:
        price(action == 0);
        c_.drug_pct_display_time = 0x32;
        break;
    case 2:
    case 3:
        select_next(action - 2);
        c_.drug_pct_display_time = 0x32;
        break;
    case 4:
        c_.state = 1;
        sound(kSfxNo4);
        break;
    case 5:
        r_.event = {EV_CONCENTRATE_RESEARCH, c_.selected_illness};
        sound(kSfxSelectX);
        break;
    }
}

} // namespace charts_detail

} // namespace th
