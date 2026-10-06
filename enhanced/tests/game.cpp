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
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
    CHECK(game->balance == int(settings.towns[1].start_cash));
    CHECK(game->opening_countdown == settings.gbv.open_countdown && !game->open);

    uint32_t seed = game->world.seed;
    for (int i = 0; i < 49; i++)
        game->turn();
    CHECK(game->date.day == 2 && game->date.month == 0); // turn 50
    for (int i = 0; i < 3 * 49; i++)
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
    CHECK(th::place_staff_on_map(w, f.gd.anims(), settings, cell, id, false, f.game->turn_count));
    int32_t salary = w.things[id].salary;
    CHECK(salary > 0);
    th::month_end(fin, balance, w, settings, 1, 31, 0, 0, 500);
    CHECK(balance == f.game->balance - salary);
    CHECK(fin.entry_count == 1 && fin.entries[0].type == th::kTxWages && fin.entries[0].amount == -salary);
    CHECK(fin.entries[0].day == 31 && fin.entries[0].month == 0);
    CHECK(fin.history[th::kHistoryWages][11] == salary && fin.history[th::kHistoryExpenditure][11] == salary);
    CHECK(fin.history[th::kHistoryBalance][11] == balance && fin.history[th::kHistoryReputation][11] == 500);
    CHECK(fin.wages_this_month == 0 && fin.expenditure_this_month == 0 && fin.income_this_month == 0);

    // Heating, loan interest and overdraft interest by the game's formulas.
    int rate = settings.towns[1].interest_rate;
    CHECK(rate > 0);
    fin.loan = 20000;
    balance = -3000 + salary; // overdrawn by 3000 once she is paid
    th::month_end(fin, balance, w, settings, 1, 28, 1, 4, 500);
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
    th::month_end(fin, balance, w, settings, 1, 31, 2, 0, 500);
    const th::BankEntry &paid = fin.entries[size_t(fin.entry_count - 1)];
    CHECK(paid.type == th::kTxInsurance && paid.amount == 300 && paid.extra == 7);
    CHECK(w.hospital->insurance_claims[1][2] == 0);

    // Lines of the same type and day merge; a full statement drops its
    // oldest ten into the opening balance.
    th::Finance s;
    int32_t b = 1000;
    th::start_finance(s, b, settings, 1);
    th::change_balance(s, b, 3, 2, th::kTxBuyObject, -10);
    th::change_balance(s, b, 3, 2, th::kTxBuyObject, -15);
    CHECK(s.entry_count == 1 && s.entries[0].amount == -25 && b == 975 && s.expenditure_this_month == 25);
    for (int i = 0; i < 49; i++)
        th::change_balance(s, b, 4 + i % 2, 2, th::kTxCheatMoney, 1);
    CHECK(s.entry_count == 40 && s.statement_balance == 1000 - 25 + 9);
    CHECK(s.income_this_month == 0 && b == 975 + 49);
    th::change_balance(s, b, 9, 2, th::kTxCheatMoney, 99999999);
    CHECK(b == 9999999);

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

    // Drawn: the message wrapped into the narrowest bubble that holds it.
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

    // Opening the furnish corridor requester moves on: its hints are help
    // text in high resolution, and he is hidden.
    a.in.pending = {th::EV_OPEN_FURNISH_CORRIDOR};
    a.frame();
    a.frame();
    a.frame();
    CHECK(a.advisor.tutorial_step == 0x28 && a.advisor.text.empty() && !a.advisor.drawn);
    CHECK(a.advisor.tutorial_highlight == 0xa);
}

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
    CHECK(cell >= 0 && th::place_object(w, a.f.gd.anims(), cell, 11, 0, 0, true) != 0);
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

    // A right click on the bubble sends him away; the advice is not given
    // again soon.
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
    CHECK(a.advisor.advice[80].times_given == 1);
}
