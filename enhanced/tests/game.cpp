#include "common.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/data/sprites.h"
#include "core/data/text.h"
#include "core/render/panel.h"
#include "core/render/text.h"
#include "core/ui/hire_staff.h"
#include "core/ui/layer.h"
#include "core/world/advisor.h"
#include "core/world/events.h"
#include "core/world/fax_screen.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// The clock: a day every 50 turns, months and the year rolling over, the
// opening countdown, and three random draws a turn.
void test_game_clock(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    auto game = std::make_unique<th::Game>();
    game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    game->skipped_calls = th::kSkipComputerAction;
    CHECK(game->balance == int(settings.towns[1].start_cash));
    CHECK(game->opening_countdown == settings.gbv.open_countdown && !game->open);

    uint32_t seed = game->world.seed;
    for (int i = 0; i < 49; i++)
        game->turn();
    CHECK(game->date.day == 2 && game->date.month == 0); // turn 50
    // Three draws a turn, plus manage_rats' three at the day boundary the
    // 49th call crosses.
    for (int i = 0; i < 3 * 49 + 3; i++)
        th::old_newrand(seed);
    CHECK(seed == game->world.seed);

    for (int i = 0; i < 30 * th::kTurnsPerDay; i++)
        game->turn();
    CHECK(game->date.day == 1 && game->date.month == 1);
    for (int i = 0; i < 334 * th::kTurnsPerDay; i++)
        game->turn();
    CHECK(game->date.day == 1 && game->date.month == 0 && game->date.year == 1);
    CHECK(game->open && game->opening_countdown == 0);

    game->paused = true;
    uint32_t turns = game->turn_count;
    game->turn();
    CHECK(game->turn_count == turns);
}

// A fax with a timeout changes the colours the game finds (World::fax_palette)
// from the turn after it is shown (display_choice's 0x5a) until the turn after
// it closes itself: 400 turns past the first turn the pointer is still, then
// one more turn to take it down. An answer closes it at once.
void test_fax_palette(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::Game &g = *f.game;
    const uint32_t opened = g.turn_count;
    th::fax_open(g, 400);
    uint32_t on = 0, off = 0;
    for (int i = 0; i < 420; i++) {
        bool before = g.world.fax_palette;
        g.turn();
        if (g.world.fax_palette != before)
            (before ? off : on) = g.turn_count;
    }
    CHECK(on == opened + 2 && off == opened + 405);
    CHECK(th::colour_red(g.world) == th::kColourRed && th::colour_dark_blue(g.world) == th::kColourDarkBlue);

    // Answered with a button: the next turn has the hospital's colours back.
    th::fax_open(g, 400);
    g.turn();
    g.turn();
    CHECK(g.world.fax_palette && th::colour_red(g.world) == th::kFaxColourRed);
    g.event = {uint8_t(th::EV_CHOICE_RESULT), th::kChoiceTreatWait, 0, 0};
    g.process_turn();
    g.turn();
    CHECK(!g.world.fax_palette && !g.fax_screen.open);
}

// The panel's watch, clicked while the countdown runs: process_turn's
// EV_OPEN_HOSPITAL opens the hospital at once, its command in command_log.
void test_open_hospital(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    th::Game game;
    game.start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
    game.skipped_calls = th::kSkipComputerAction;
    CHECK(!game.open && game.opening_countdown == settings.gbv.open_countdown);

    game.event = {th::EV_OPEN_HOSPITAL};
    game.process_turn();
    CHECK(game.open && game.opening_countdown == 0);
    CHECK(!game.command_log.empty() && game.command_log.back().ends_with("139 0 0"));
}

namespace
{

// Month ends after `salary` was first paid from `balance`.
void check_month_end_lines(th::Finance &fin, int32_t &balance, th::World &w, const th::Balance &settings,
                           int32_t salary)
{
    // Heating, loan interest and overdraft interest by the game's formulas.
    int rate = settings.towns[1].interest_rate;
    CHECK(rate > 0);
    fin.loan = 20000;
    balance = -3000 + salary; // overdrawn by 3000 once she is paid
    th::month_end(fin, balance, w, settings, 1, {28, 1, 4, 500});
    int32_t heating = 4 * 5 * 750 / 100;
    int32_t interest = rate * 20000 / 10000 / 12;
    int32_t after = -3000 - heating - interest;
    int32_t overdraft = rate * after * 2 / 10000 / 12;
    CHECK(fin.entry_count == 5);
    CHECK(fin.entries[2].type == th::kTxHeating && fin.entries[2].amount == -heating);
    CHECK(fin.entries[3].type == th::kTxLoanInterest && fin.entries[3].amount == -interest);
    CHECK(fin.entries[4].type == th::kTxOverdraftInterest && fin.entries[4].amount == overdraft && overdraft < 0);
    CHECK(balance == after + overdraft);

    // An insurer's claims due this month: the line names its company
    // (ins_control[slot]), not its slot.
    w.ins_control = {4, 7, 2, 0, 0};
    w.hospital->insurance_claims[1][2] = 300;
    fin.loan = 0;
    balance = 100000;
    th::month_end(fin, balance, w, settings, 1, {31, 2, 0, 500});
    const th::BankEntry &paid = fin.entries[size_t(fin.entry_count - 1)];
    CHECK(paid.type == th::kTxInsurance && paid.amount == 300 && paid.extra == 7);
    CHECK(w.hospital->insurance_claims[1][2] == 0);
}

void check_statement_merging(const th::Balance &settings)
{
    // Lines of the same type and day merge; a full statement drops its
    // oldest ten into the opening balance.
    th::Finance s;
    int32_t b = 1000;
    th::start_finance(s, b, settings, 1);
    th::change_balance(s, b, 3, 2, {th::kTxBuyObject, -10});
    th::change_balance(s, b, 3, 2, {th::kTxBuyObject, -15});
    CHECK(s.entry_count == 1 && s.entries[0].amount == -25 && b == 975 && s.expenditure_this_month == 25);
    for (int i = 0; i < 49; i++)
        th::change_balance(s, b, 4 + i % 2, 2, {th::kTxCheatMoney, 1});
    CHECK(s.entry_count == 40 && s.statement_balance == 1000 - 25 + 9);
    CHECK(s.income_this_month == 0 && b == 975 + 49);
    th::change_balance(s, b, 9, 2, {th::kTxCheatMoney, 99999999});
    CHECK(b == 9999999);
}

void check_year_end(th::Finance &fin, int32_t balance, th::World &w, const th::Balance &settings)
{
    // The year's end values the hospital and opens a new statement.
    th::year_end(fin, balance, w, settings, 500);
    int area = 0;
    for (const th::MapCell &c : w.map)
        area += c.owner_flags & 1;
    CHECK(area > 0 && fin.area == area);
    CHECK(fin.hospital_value == area * settings.gbv.land_cost_per_tile + 10000 + 10000);
    CHECK(fin.max_loan == std::min(fin.hospital_value / 3 + 10000, 100000));
    CHECK(fin.entry_count == 0 && fin.statement_balance == balance && fin.year_start_balance == balance);
}

} // namespace

void test_finance_level1(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    const th::Balance &settings = f.settings;
    int32_t balance = f.game->balance;
    th::Finance fin;
    th::start_finance(fin, balance, settings, 1);
    CHECK(fin.start_cash == settings.towns[1].start_cash && uint32_t(balance) == fin.start_cash);
    CHECK(fin.heating_level == 5 && fin.heating_price == 750 && fin.statement_balance == balance);

    // A receptionist hired: her wage is paid at month end, as one line.
    uint16_t id = th::alter_applicant(w, true, 0, th::kStaffReceptionist);
    int cell = free_staff_cell(w, *f.level);
    CHECK(id != 0 && cell >= 0);
    if (!id || cell < 0)
        return;
    CHECK(th::place_staff_on_map(w, {f.gd.anims(), settings, f.game->turn_count}, cell, id, false));
    int32_t salary = w.things[id].salary;
    CHECK(salary > 0);
    th::month_end(fin, balance, w, settings, 1, {31, 0, 0, 500});
    CHECK(balance == f.game->balance - salary);
    CHECK(fin.entry_count == 1 && fin.entries[0].type == th::kTxWages && fin.entries[0].amount == -salary);
    CHECK(fin.entries[0].day == 31 && fin.entries[0].month == 0);
    CHECK(fin.history[th::kHistoryWages][11] == salary && fin.history[th::kHistoryExpenditure][11] == salary);
    CHECK(fin.history[th::kHistoryBalance][11] == balance && fin.history[th::kHistoryReputation][11] == 500);
    CHECK(fin.wages_this_month == 0 && fin.expenditure_this_month == 0 && fin.income_this_month == 0);
    check_month_end_lines(fin, balance, w, settings, salary);
    check_statement_merging(settings);
    check_year_end(fin, balance, w, settings);
}

void test_balance_roll(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto draw = [&](const th::PanelState &state, int balance) {
        th::PanelFigures figures;
        figures.balance = balance;
        th::UiLayer layer;
        layer.resize(th::kUiWidth, th::kUiHeight);
        layer.clear();
        th::draw_panel_ui(layer, state, &figures, gd);
        return layer.to_argb(gd.palette());
    };
    // The digits' box, and its first column, which shows the new figure.
    auto region = [](const std::vector<uint32_t> &argb, int x0, int x1) {
        std::vector<uint32_t> out;
        for (int y = 442; y < 460; y++)
            for (int x = x0; x < x1; x++)
                out.push_back(argb[size_t(y * th::kUiWidth + x)]);
        return out;
    };
    th::PanelState still;
    still.shown_balance = 1234;
    std::vector<uint32_t> old_image = draw(still, 1234);
    still.shown_balance = 1999;
    std::vector<uint32_t> new_image = draw(still, 1999);
    CHECK(region(old_image, 59, 135) != region(new_image, 59, 135));

    th::PanelState state;
    state.shown_balance = 1234;
    std::vector<std::vector<uint32_t>> frames;
    for (int frame = 0; frame < 4; frame++) {
        CHECK(state.balance_roll_step == frame && state.shown_balance == 1234);
        frames.push_back(draw(state, 1999));
        th::advance_balance_roll(state, 1999);
    }
    CHECK(state.balance_roll_step == 0 && state.shown_balance == 1999);
    th::advance_balance_roll(state, 1999);
    CHECK(state.balance_roll_step == 0);
    // Step 0 shows the old figures where they roll; then they move.
    CHECK(region(frames[0], 59, 135) == region(old_image, 59, 135));
    for (int i = 1; i < 4; i++) {
        CHECK(region(frames[size_t(i)], 59, 135) != region(frames[size_t(i - 1)], 59, 135));
        CHECK(region(frames[size_t(i)], 59, 135) != region(new_image, 59, 135));
    }
    CHECK(draw(state, 1999) == new_image);
}

namespace
{

struct AdvisorFixture {
    HireFixture f;
    th::Finance finance;
    th::Advisor advisor;
    th::Font font;
    uint32_t safeseed = 0x546b2;
    th::AdvisorInputs in;

    bool start(const char *dir, bool tutorial)
    {
        std::string error;
        if (!f.start(dir) || !th::load_font(dir, 0x1d, font, &error))
            return false;
        th::start_finance(finance, f.game->balance, f.settings, 1);
        th::start_advisor(advisor, tutorial);
        in.level = 1;
        in.balance = f.game->balance;
        in.finance = &finance;
        in.settings = &f.settings;
        in.world = &f.game->world;
        in.text = &f.gd.text();
        in.game_turn = f.game->turn_count;
        return true;
    }
    // A frame: the advisor, then one game turn.
    void frame()
    {
        th::advisor_frame(advisor, in, f.gd.anims(), safeseed);
        in.game_turn++;
        in.last = in.pending;
        in.pending = {};
    }
    // Frames until he is up, at most `limit`.
    bool until_up(int limit = 400)
    {
        for (int i = 0; i < limit; i++) {
            frame();
            if (th::kAdvisorUpStates[advisor.anim_state])
                return true;
        }
        return false;
    }
};

// Drawn: the message wrapped into the narrowest bubble that holds it.
void check_tutorial_bubble(AdvisorFixture &a)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    layer.clear();
    a.advisor.drawn = false;
    th::draw_advisor(layer, a.advisor, a.f.gd, a.font);
    CHECK(!a.advisor.rewrap && a.advisor.line_count >= 1 && a.advisor.line_count <= 3);
    int pieces = th::kAdvisorMinSegments + a.advisor.extra_segments;
    CHECK(a.advisor.no_pieces == pieces + 1);
    std::string wrapped = a.advisor.text;
    int widest = 0;
    int lines = 0;
    for (size_t start = 0, end; (end = wrapped.find('\n', start)) != std::string::npos; start = end + 1) {
        lines++;
        widest = std::max(widest, th::text_width(a.font, std::string_view(wrapped).substr(start, end - start)));
    }
    CHECK(lines == a.advisor.line_count);
    // Every pixel drawn is the bubble's or the text's, within the bubble.
    const th::SpriteSheet &panel = a.f.gd.panel();
    int left = 515 - panel.at(37).width - pieces * panel.at(38).width;
    int right = 515 + panel.at(39).width;
    CHECK(a.advisor.box_left == 515 - panel.at(38).width * (pieces + 1) - panel.at(39).width);
    std::vector<uint32_t> argb = layer.to_argb(a.f.gd.palette());
    int x0 = th::kUiWidth, x1 = 0, y0 = th::kUiHeight, y1 = 0;
    for (int y = 0; y < th::kUiHeight; y++)
        for (int x = 0; x < th::kUiWidth; x++)
            if (argb[size_t(y * th::kUiWidth + x)]) {
                x0 = std::min(x0, x);
                x1 = std::max(x1, x);
                y0 = std::min(y0, y);
                y1 = std::max(y1, y);
            }
    CHECK(x0 == left && x1 < right && y0 >= 375 && y1 < th::kUiHeight);
    // The text starts inside the left end and its widest line ends inside
    // the right end.
    int text_x = 515 - pieces * panel.at(38).width + 8 - panel.at(37).width;
    CHECK(text_x > left && text_x + widest < right);
}

} // namespace

void test_advisor_tutorial(const char *dir)
{
    AdvisorFixture a;
    CHECK(a.start(dir, true));
    if (!a.f.level)
        return;
    CHECK(a.f.gd.anims().starts.size() > 2374);
    const th::LanguageText &text = a.f.gd.text();
    // The level starts with the tutorial's first message.
    a.frame();
    CHECK(a.advisor.text == text.advisor(29) && a.advisor.rewrap && a.advisor.tutorial_highlight == 0x13);
    CHECK(a.advisor.advice_id == 1 && a.advisor.anim_state == 1 && a.advisor.drawn && !a.advisor.bubble);
    CHECK(a.until_up());
    a.frame();
    CHECK(a.advisor.bubble && a.advisor.drawn);
    check_tutorial_bubble(a);

    // Opening the furnish corridor requester moves on: its hints are help
    // text in high resolution, and he is hidden.
    a.in.pending = {th::EV_OPEN_FURNISH_CORRIDOR};
    a.frame();
    a.frame();
    a.frame();
    CHECK(a.advisor.tutorial_step == 0x28 && a.advisor.text.empty() && !a.advisor.drawn);
    CHECK(a.advisor.tutorial_highlight == 0xa);
}

namespace
{

// The tutorial from `step` with its messages unshown: where the step's own
// message, hint (empty message) and highlight are.
void tutorial_at(AdvisorFixture &a, uint8_t step)
{
    a.advisor.tutorial_step = step;
    a.advisor.tutorial_shown = {};
    a.advisor.tutorial_highlight = 0;
    a.advisor.text.clear();
}

// The command `e` carried out, as the next frame's `last`.
void tutorial_command(AdvisorFixture &a, th::Event e)
{
    a.in.pending = e;
    a.frame();
    a.frame();
}

// The step's message, if `text` is not null, and the highlight; the step's
// hint when the message is empty.
void check_tutorial_message(AdvisorFixture &a, const char *what, int text, uint16_t highlight)
{
    a.frame();
    const th::LanguageText &lang = a.f.gd.text();
    bool ok = text ? a.advisor.text == lang.advisor(text) : a.advisor.text.empty();
    ok = ok && a.advisor.tutorial_highlight == highlight;
    if (!ok)
        std::fprintf(stderr, "tutorial: %s\n", what);
    CHECK(ok);
}

// The flags drawing a blueprint leaves: too small alone on the hospital's
// land, too small and placeable nowhere off it, only the latter once big
// enough, and none for no blueprint.
void check_blueprint_flags(AdvisorFixture &a)
{
    th::Advisor &adv = a.advisor;
    th::BuildState &b = a.f.game->world.building.build;
    auto flags_for = [&](int x0, int y0, int x1, int y1) {
        b = {};
        b.room_type = th::kRoomGp;
        b.draw_mode = 1;
        b.x0 = uint8_t(x0), b.y0 = uint8_t(y0), b.x1 = uint8_t(x1), b.y1 = uint8_t(y1);
        adv.flags = th::kAdvisorTutorial;
        a.frame();
        return adv.flags & (th::kAdvisorBlueprintSmall | th::kAdvisorBadPlacement);
    };
    CHECK(flags_for(62, 65, 62, 65) == th::kAdvisorBlueprintSmall);
    // A block of 5 x 5 cells with nothing on them that are not the hospital's.
    int x = 0, y = 0;
    for (int c = 0; c < th::kMapCells - 5 * th::kMapSize - 5 && !x; c++) {
        bool block = true;
        for (int i = 0; i < 25; i++) {
            const th::MapCell &m = a.f.game->world.map[size_t(c + i % 5 + i / 5 * th::kMapSize)];
            block = block && !m.first && !(m.flags & 7) && ((m.owner_flags & th::kCellOutdoor) || !(m.owner_flags & 1));
        }
        if (block)
            x = c % th::kMapSize, y = c / th::kMapSize;
    }
    CHECK(x > 0 && x + 4 < th::kMapSize);
    CHECK(flags_for(x, y, x, y) == (th::kAdvisorBlueprintSmall | th::kAdvisorBadPlacement));
    CHECK(flags_for(x, y, x + 4, y + 4) == th::kAdvisorBadPlacement);
    CHECK(flags_for(62, 65, 66, 69) == 0);
    b.draw_mode = 0;
    adv.flags = th::kAdvisorTutorial | th::kAdvisorBlueprintSmall;
    a.frame();
    CHECK(adv.flags & th::kAdvisorBlueprintSmall);
    b = {};
}

// Steps 0x20 to 0x3a: choosing and building the GP's office.
void check_tutorial_gp_office(AdvisorFixture &a)
{
    th::Advisor &adv = a.advisor;
    th::World &world = a.f.game->world;
    tutorial_at(a, 0x20);
    check_tutorial_message(a, "0x20 asks for the room building", 32, 0x12);
    tutorial_command(a, {th::EV_OPEN_BUILD_ROOM});
    CHECK(adv.tutorial_step == 0x2b);
    check_tutorial_message(a, "0x2b points at the department", 0, 8);
    tutorial_command(a, {th::EV_REQ1_RESET_ROOMS, 4});
    CHECK(adv.tutorial_step == 0x2b);
    tutorial_command(a, {th::EV_REQ1_RESET_ROOMS, 3});
    CHECK(adv.tutorial_step == 0x2c);
    // The hint is marked shown as flag 14, not 15, so it comes every turn.
    check_tutorial_message(a, "0x2c points at the room", 0, 0xb);
    adv.tutorial_highlight = 0;
    check_tutorial_message(a, "0x2c's hint comes again", 0, 0xb);
    tutorial_command(a, {th::EV_INIT_BLUEPRINT, 8});
    CHECK(adv.tutorial_step == 0x20 && adv.tutorial_shown[14] == 0);
    tutorial_at(a, 0x2c);
    tutorial_command(a, {th::EV_CLOSE_REQUESTERS});
    CHECK(adv.tutorial_step == 0x20);
    tutorial_at(a, 0x2c);
    tutorial_command(a, {th::EV_INIT_BLUEPRINT, th::kRoomGp});
    CHECK(adv.tutorial_step == 0x39);
    check_tutorial_message(a, "0x39 asks for the blueprint dragged out", 47, 0);
    tutorial_command(a, {th::EV_CANCEL_BUILD});
    CHECK(adv.tutorial_step == 0x20);
    // A room built already, or the step done: on to the doctor.
    world.room_of_type[th::kRoomGp] = 1;
    a.frame();
    CHECK(adv.tutorial_step == 0x1f && adv.tutorial_shown[3] == 2);
    world.room_of_type[th::kRoomGp] = 0;
}

// Steps 0x2f and 0x38: the flags the drawn blueprint leaves, and the
// messages they give.
void check_tutorial_blueprint(AdvisorFixture &a)
{
    th::Advisor &adv = a.advisor;
    tutorial_at(a, 0x39);
    tutorial_command(a, {th::EV_MODIFY_BLUEPRINT});
    CHECK(adv.tutorial_step == 0x2f);
    // Too small and not placeable, then neither, then not placeable alone.
    adv.flags = th::kAdvisorTutorial | th::kAdvisorBlueprintSmall | th::kAdvisorBadPlacement;
    check_tutorial_message(a, "0x2f: too small and invalid", 61, 0);
    CHECK(adv.tutorial_step == 0x2f && !(adv.flags & (th::kAdvisorBlueprintSmall | th::kAdvisorBadPlacement)));
    adv.flags = th::kAdvisorTutorial;
    check_tutorial_message(a, "0x2f: nothing wrong", 52, 0);
    adv.flags = th::kAdvisorTutorial | th::kAdvisorBadPlacement;
    check_tutorial_message(a, "0x2f: invalid", 51, 0);
    CHECK(adv.tutorial_step == 0x2f);
    // Too small alone moves on, to waiting for the commit.
    adv.flags = th::kAdvisorTutorial | th::kAdvisorBlueprintSmall;
    a.frame();
    CHECK(adv.tutorial_step == 0x38);
    adv.flags = th::kAdvisorTutorial | th::kAdvisorBlueprintSmall;
    check_tutorial_message(a, "0x38: too small", 52, 0);
    adv.flags = th::kAdvisorTutorial;
    check_tutorial_message(a, "0x38 asks for the commit", 56, 0x0c);
    tutorial_command(a, {th::EV_COMMIT_BLUEPRINT});
    CHECK(adv.tutorial_step == 0x30);
}

// Steps 0x30 to 0x3a: the door, the windows, the objects and opening.
void check_tutorial_room(AdvisorFixture &a)
{
    th::Advisor &adv = a.advisor;
    th::World &world = a.f.game->world;
    tutorial_at(a, 0x30);
    check_tutorial_message(a, "0x30 asks for the door", 48, 0);
    world.building.door_invalid = true;
    tutorial_command(a, {th::EV_PLACE_BP_DOOR});
    CHECK(adv.tutorial_step == 0x30);
    check_tutorial_message(a, "0x30: the door does not fit", 53, 0);
    world.building.door_invalid = false;
    tutorial_command(a, {th::EV_PLACE_BP_DOOR});
    CHECK(adv.tutorial_step == 0x31);
    check_tutorial_message(a, "0x31 asks for windows", 49, 0x0c);
    world.building.door_invalid = true;
    tutorial_command(a, {th::EV_PLACE_BP_DOOR});
    check_tutorial_message(a, "0x31: the window does not fit", 54, 0x0c);
    tutorial_command(a, {th::EV_EDIT_BLUEPRINT});
    CHECK(adv.tutorial_step == 0x38);
    world.building.door_invalid = false;
    tutorial_at(a, 0x31);
    tutorial_command(a, {th::EV_COMMIT_DOORS});
    CHECK(adv.tutorial_step == 0x32);
    check_tutorial_message(a, "0x32 asks for the objects", 50, 0);
    adv.flags |= th::kAdvisorBadPlacement;
    check_tutorial_message(a, "0x32: the object does not fit", 55, 0);
    CHECK(!(adv.flags & th::kAdvisorBadPlacement));
    // The room's requester sets the flag that the room has what it needs.
    world.placing.furnished = true;
    a.frame();
    CHECK(adv.tutorial_step == 0x3a && (adv.flags & th::kAdvisorRoomFurnished));
    check_tutorial_message(a, "0x3a asks to open the room", 58, 0x0c);
    tutorial_command(a, {th::EV_EDIT_BLUEPRINT});
    CHECK(adv.tutorial_step == 0x38);
    tutorial_at(a, 0x3a);
    tutorial_command(a, {th::EV_OPEN_ROOM});
    CHECK(adv.tutorial_step == 0x1f);
}

// Steps 0x1f to 0x27: the doctor, which ends the tutorial.
void check_tutorial_doctor(AdvisorFixture &a)
{
    th::Advisor &adv = a.advisor;
    int help_page = -1;
    a.in.help_page = &help_page;
    tutorial_at(a, 0x1f);
    check_tutorial_message(a, "0x1f asks for the staff hiring", 31, 0x15);
    tutorial_command(a, {th::EV_OPEN_HIRE_STAFF});
    CHECK(adv.tutorial_step == 0x23);
    check_tutorial_message(a, "0x23 points at the doctors", 0, 4);
    tutorial_command(a, {th::EV_HIRE_SELECT_KIND, th::kStaffNurse});
    CHECK(adv.tutorial_step == 0x23);
    tutorial_command(a, {th::EV_MARK_EMPLOYING, th::kStaffNurse});
    CHECK(adv.tutorial_step == 0x1f && adv.tutorial_shown[2] == 0);
    tutorial_at(a, 0x23);
    tutorial_command(a, {th::EV_HIRE_SELECT_KIND, th::kStaffDoctor});
    CHECK(adv.tutorial_step == 0x2d);
    check_tutorial_message(a, "0x2d points at the hire button", 0, 5);
    tutorial_command(a, {th::EV_HIRE_SELECT_KIND, th::kStaffNurse});
    CHECK(adv.tutorial_step == 0x23);
    tutorial_at(a, 0x2d);
    tutorial_command(a, {th::EV_MARK_EMPLOYING, th::kStaffDoctor});
    CHECK(adv.tutorial_step == 0x27);
    // Held where the doctor may not be put down, then where they may.
    a.in.staff_place_ok = false;
    check_tutorial_message(a, "0x27: not there", 59, 0);
    a.in.staff_place_ok = true;
    adv.tutorial_shown[10] = 0;
    check_tutorial_message(a, "0x27 asks for the doctor placed", 39, 0);
    tutorial_command(a, {th::EV_CANCEL_BUILD});
    CHECK(adv.tutorial_step == 0x1f);
    // Once a doctor works for the hospital the tutorial is over.
    th::Thing &doctor = a.f.game->world.things[3];
    doctor = {};
    doctor.type = th::kThingStaff;
    doctor.subtype = th::kStaffDoctor;
    doctor.skills = th::kSkillDoctor;
    doctor.owner = 0;
    doctor.state = th::kStaffWanders;
    tutorial_at(a, 0x27);
    a.frame();
    CHECK(!(adv.flags & th::kAdvisorTutorial) && help_page == 0xf);
}

} // namespace

void test_advisor_tutorial_rooms(const char *dir)
{
    AdvisorFixture a;
    CHECK(a.start(dir, true));
    if (!a.f.level)
        return;
    check_tutorial_gp_office(a);
    check_blueprint_flags(a);
    check_tutorial_blueprint(a);
    check_tutorial_room(a);
    check_tutorial_doctor(a);
}

namespace
{

// A right click on the bubble sends him away; the advice is not given
// again soon.
void check_advice_dismissed(AdvisorFixture &a)
{
    th::UiLayer layer;
    layer.resize(th::kUiWidth, th::kUiHeight);
    th::draw_advisor(layer, a.advisor, a.f.gd, a.font);
    CHECK(th::advisor_hit(a.advisor, a.advisor.box_left + 4, 400));
    CHECK(th::advisor_hit(a.advisor, 560, 400) && !th::advisor_hit(a.advisor, a.advisor.box_left - 1, 400));
    CHECK(!th::advisor_hit(a.advisor, 300, 200));
    a.advisor.dismiss = true;
    a.frame();
    CHECK(a.advisor.advice_id == 0 && !a.advisor.bubble && !a.advisor.dismiss);
    // Through the up states left (a special leads to 2, 2 to 3) to leaving.
    for (int i = 0; i < 100 && a.advisor.anim_state != 4; i++) {
        CHECK(!a.advisor.bubble);
        a.frame();
    }
    CHECK(a.advisor.anim_state == 4);
    for (int i = 0; i < 200 && a.advisor.anim_state; i++)
        a.frame();
    CHECK(a.advisor.anim_state == 0 && !th::advisor_hit(a.advisor, 560, 400));
}

} // namespace

void test_advisor_advice(const char *dir)
{
    AdvisorFixture a;
    CHECK(a.start(dir, false));
    if (!a.f.level)
        return;
    const th::LanguageText &text = a.f.gd.text();
    // Without the tutorial nothing is said at the start.
    for (int i = 0; i < 50; i++)
        a.frame();
    CHECK(a.advisor.anim_state == 0 && !a.advisor.drawn && a.advisor.advice_id == 0);

    // A reception desk with no receptionist on level 1, and an event's
    // message: the event's comes first, being earlier in the table.
    th::World &w = a.f.game->world;
    int cell = free_staff_cell(w, *a.f.level);
    CHECK(cell >= 0 && th::place_object(w, a.f.gd.anims(), {cell, 11, 0}, 0, true) != 0);
    a.advisor.forced_advice_id = 7;
    a.frame();
    CHECK(a.advisor.text == text.at(th::TextSection::ForcedAdvice, 7) && a.advisor.forced_advice_id == 0);
    CHECK(a.advisor.advice_id == 17 && a.advisor.time_left == 0x82 / 2);
    CHECK(a.until_up());
    for (int i = 0; i < 0x82 && a.advisor.advice_id == 17; i++)
        a.frame();
    CHECK(a.advisor.advice_id == 80 && a.advisor.text == text.advisor(0x69) && a.advisor.rewrap);
    a.frame();
    CHECK(a.advisor.bubble);
    check_advice_dismissed(a);
    CHECK(a.advisor.advice[80].times_given == 1);
}

namespace
{

// The advice the advisor gives first, from the start, once `set` has set
// the hospital up: its situation (Advice::id), 0 for none; its message in
// `text` and the announcement it plays in `ambient`.
uint16_t first_advice(AdvisorFixture &a, const std::function<void(th::World &, th::AdCounts &)> &set,
                      std::string *text = nullptr, uint8_t *ambient = nullptr)
{
    th::World &w = a.f.game->world;
    th::start_advisor(a.advisor, false);
    w.counts = {};
    w.counts.has_patients = true;
    w.patients_left_count = 0;
    w.litter_count = 0;
    w.bench_shortage = 0;
    a.finance.area = 1000;
    uint8_t played = 0;
    a.in.ambient = &played;
    set(w, w.counts);
    a.frame();
    a.in.ambient = nullptr;
    if (text)
        *text = a.advisor.text;
    if (ambient)
        *ambient = played;
    return a.advisor.advice_id ? a.advisor.advice[size_t(a.advisor.advice_id)].id : 0;
}

// Patients in their right minds: `n` counted, happy, refreshed, warm and
// with empty bladders.
void calm_patients(th::AdCounts &c, int n)
{
    c.patients = uint16_t(n + 1);
    c.happiness_sum = int16_t(80 * c.patients);
    c.refreshment_sum = int16_t(80 * c.patients);
    c.warmth_sum = int16_t(50 * c.patients);
}

// The advice about the patients' figures, each alone.
void check_advice_patients(AdvisorFixture &a)
{
    const th::LanguageText &text = a.f.gd.text();
    const int bowel_full = a.f.settings.gbv.bowel_full;
    std::string said;
    uint8_t ambient = 0;
    // Nothing is said of calm patients.
    CHECK(first_advice(a, [](th::World &, th::AdCounts &c) { calm_patients(c, 12); }) == 0);
    // Thirsty patients: the first time, its situation's own words.
    auto thirsty = [](th::World &, th::AdCounts &c) {
        calm_patients(c, 12);
        c.refreshment_sum = int16_t(20 * c.patients);
    };
    CHECK(first_advice(a, thirsty, &said) == 0xf);
    CHECK(said == text.at(th::TextSection::AdvisorSituations, 15) && a.advisor.given[18] == 1);
    // Full bladders and no toilet.
    auto bursting = [&](th::World &, th::AdCounts &c) {
        calm_patients(c, 12);
        c.bladder_sum = int16_t((bowel_full + 10) * c.patients);
    };
    CHECK(first_advice(a, bursting, &said) == 0x13 && said == text.advisor(0x13));
    // Cold patients and no radiator: the announcer says it is too cold.
    auto cold = [](th::World &, th::AdCounts &c) {
        calm_patients(c, 12);
        c.warmth_sum = int16_t(15 * c.patients);
    };
    CHECK(first_advice(a, cold, &said, &ambient) == 0x11);
    CHECK(said == text.at(th::TextSection::AdvisorSituations, 12) && ambient == 0xd8);
    // Patients gone home fed up, then forgotten.
    auto fed_up = [](th::World &w, th::AdCounts &c) {
        calm_patients(c, 12);
        w.patients_left_count = 3;
    };
    CHECK(first_advice(a, fed_up, &said) == 0xe);
    CHECK(said == text.at(th::TextSection::ExtraAdvice, 7) && a.f.game->world.patients_left_count == 0);
    // Without patients counted, none of these is said.
    auto uncounted = [](th::World &, th::AdCounts &c) {
        calm_patients(c, 12);
        c.refreshment_sum = 0;
        c.has_patients = false;
    };
    CHECK(first_advice(a, uncounted) == 0);
}

// The advice about the litter, benches, machines and staff, each alone.
void check_advice_hospital(AdvisorFixture &a)
{
    const th::LanguageText &text = a.f.gd.text();
    std::string said;
    uint8_t ambient = 0;
    // Litter and no handyman: first the once-only advice, then the litter's.
    auto littered = [](th::World &w, th::AdCounts &c) {
        calm_patients(c, 2);
        w.litter_count = 200;
    };
    CHECK(first_advice(a, littered, &said) == 0x45);
    a.advisor.advice[51].times_given = 1;
    a.advisor.advice_id = 0;
    a.in.ambient = &ambient;
    a.frame();
    a.in.ambient = nullptr;
    CHECK(a.advisor.advice[size_t(a.advisor.advice_id)].id == 0x46 && ambient == 0xd7);
    CHECK(a.advisor.text == text.at(th::TextSection::ExtraAdvice, 0x11));
    // Patients standing for want of benches.
    auto standing = [](th::World &w, th::AdCounts &c) {
        calm_patients(c, 8);
        w.bench_shortage = 100;
    };
    CHECK(first_advice(a, standing, &said) == 0x4b && said == text.at(th::TextSection::ExtraAdvice, 0x1b));
    // Worn machines and no handyman.
    auto worn = [](th::World &, th::AdCounts &c) {
        calm_patients(c, 2);
        c.machines = 2;
        c.machine_wear_sum = 600;
    };
    CHECK(first_advice(a, worn, &said) == 3 && said == text.at(th::TextSection::ExtraAdvice, 0x21));
    // A person stuck, said and forgotten; the first call about a patient,
    // once per game.
    CHECK(first_advice(a, [](th::World &w, th::AdCounts &) { w.announce_stuck = true; }, &said) == 0xc);
    CHECK(said == text.advisor(12) && !a.f.game->world.announce_stuck);
    a.advisor.event_flags = th::kAdvisorEventCall;
    a.advisor.advice_id = 0;
    a.frame();
    CHECK(a.advisor.advice[size_t(a.advisor.advice_id)].id == 0x87 && a.advisor.given[8] == 1);
    // Prices far above what patients will pay.
    CHECK(first_advice(a, [](th::World &w, th::AdCounts &) { w.market->price_level = 200; }, &said) == 0x76);
    CHECK(said == text.advisor(0x76));
    a.f.game->world.market->price_level = 100;
    // A doctor cracked up, said once and forgotten.
    CHECK(first_advice(a, [](th::World &, th::AdCounts &c) { c.crackups = 1; }, &said) == 0x5e);
    CHECK(said == text.advisor(0x5e) && a.f.game->world.counts.crackups == 0);
}

// The advice about the research announcements: each names what was found,
// and the announcement is dropped once said.
void check_advice_announcements(AdvisorFixture &a)
{
    th::World &w = a.f.game->world;
    const th::LanguageText &text = a.f.gd.text();
    std::string said;
    auto calm = [](th::World &, th::AdCounts &c) { calm_patients(c, 12); };
    auto names = [&](th::TextSection section, int value) { return text.at(section, value); };

    w.announce = {};
    w.announce[th::kAnnObjectResearched][0] = 20;
    CHECK(first_advice(a, calm, &said) == 0x17);
    CHECK(said.find(names(th::TextSection::ObjectNames, 20)) != std::string::npos);
    CHECK(w.announce[th::kAnnObjectResearched][0] == 0);

    w.announce[th::kAnnNewDrug][0] = 5;
    CHECK(first_advice(a, calm, &said) == 0x18);
    CHECK(said.find(names(th::TextSection::Illness, 5)) != std::string::npos);
    CHECK(w.announce[th::kAnnNewDrug][0] == 0);

    w.announce[th::kAnnDrugImproved][0] = 6;
    CHECK(first_advice(a, calm, &said) == 0x19);
    CHECK(said.find(names(th::TextSection::Illness, 6)) != std::string::npos);

    w.announce[th::kAnnObjectImproved][0] = 30;
    CHECK(first_advice(a, calm, &said) == 0x1a);
    CHECK(said.find(names(th::TextSection::ObjectNames, 30)) != std::string::npos);

    w.announce[th::kAnnObjectAvailable][0] = 31;
    CHECK(first_advice(a, calm, &said) == 0x1b);
    CHECK(said.find(names(th::TextSection::ObjectNames, 31)) != std::string::npos);

    w.announce[th::kAnnCategoryDone][0] = 2;
    CHECK(first_advice(a, calm, &said) == 0x79);
    CHECK(said.find(names(th::TextSection::ResearchNames, 2)) != std::string::npos);
    CHECK(w.announce[th::kAnnCategoryDone][0] == 0);

    w.announce[th::kAnnAutopsyDiscovered][0] = 1;
    CHECK(first_advice(a, calm, &said) == 0x7a);
    CHECK(said == text.advisor(0x7a) && w.announce[th::kAnnAutopsyDiscovered][0] == 0);
}

// The advice about the plants' condition: a middling one with one handyman
// speaks the plant message; a very poor one with two speaks the two-handymen
// warning.
void check_advice_plants(AdvisorFixture &a)
{
    th::World &w = a.f.game->world;
    const th::LanguageText &text = a.f.gd.text();
    std::string said;
    th::Thing &h = w.things[3];
    h = {};
    h.type = th::kThingStaff;
    h.subtype = th::kStaffHandyman;
    h.owner = 0;
    h.state = th::kStaffWanders;
    auto plants = [](th::World &w, th::AdCounts &c) {
        calm_patients(c, 12);
        w.plants = 3;
        w.plant_health_sum = 90; // 0x190000 * 90 / 3 is in the 0x5a band
    };
    CHECK(first_advice(a, plants, &said) == 0x5a);
    CHECK(said == text.advisor(0x5a));

    w.things[4] = h;
    auto dying = [](th::World &w, th::AdCounts &c) {
        calm_patients(c, 12);
        w.plants = 3;
        w.plant_health_sum = 0;
    };
    CHECK(first_advice(a, dying, &said) == 0x5a);
    CHECK(said == text.at(th::TextSection::ExtraAdvice, 0x28));
}

} // namespace

// The advice of the statistics pass (World::counts) and of what the
// hospital holds, each as the game gives it in its situation, alone.
void test_advisor_counts(const char *dir)
{
    AdvisorFixture a;
    CHECK(a.start(dir, false));
    if (!a.f.level)
        return;
    check_advice_patients(a);
    check_advice_hospital(a);
    check_advice_announcements(a);
    check_advice_plants(a);
}
