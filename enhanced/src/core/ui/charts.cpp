#include "core/ui/charts.h"

#include "core/ui/charts_internal.h"

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/render/panel.h"
#include "core/ui/helpmsg.h"
#include "core/ui/info_windows.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <cstring>

namespace th
{

namespace
{

constexpr double kGameFrame = 0.04; // limit_framerate: 40 timer ticks

// Each chart's picture and sprites (hiresscreens and
// hiresscreen_sprites_data, src/files.cpp, at SCR_STATEMENT, SCR_DRUGS,
// SCR_POLICY, SCR_RESEARCH, SCR_GRAPHS, SCR_STAFF_LIST, SCR_BANK_MANAGER and
// SCR_OVERVIEW).
constexpr std::array<std::string_view, kChartCount> kPictures = {"stat01v",  "drugn01v", "pol01v",  "res01v",
                                                                 "graph01v", "staff01v", "bank01v", "rep01v"};
constexpr std::array<std::string_view, kChartCount> kSprites = {"stat02v",  "drugn02v", "pol02v",  "res02v",
                                                                "graph02v", "staff02v", "bank02v", "rep02v"};

// The font cache slots the charts draw with.
constexpr std::array<int, 16> kChartFonts = {1, 2, 3, 7, 8, 9, 0xb, 0xd, 0xe, 0xf, 0x1b, 0x1c, 0x24, 0x25, 0x26, 0x2e};

// A full-screen picture: 640 x 480 palette indices and its VGA palette.
bool load_picture(const std::filesystem::path &dir, std::string_view name, Charts::Picture &out, std::string *error)
{
    auto dat = load_data_file(dir, "qdata/" + std::string(name) + ".dat", error);
    if (!dat)
        return false;
    auto pal = load_data_file(dir, "qdata/" + std::string(name) + ".pal", error);
    if (!pal)
        return false;
    if (dat->size() < size_t(kUiWidth) * kUiHeight || pal->size() < out.vga.size()) {
        if (error)
            *error = std::string(name) + ": not a full-screen picture";
        return false;
    }
    out.pixels.resize(kUiWidth, kUiHeight);
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(out.pixels.row(y), dat->data() + size_t(y) * kUiWidth, kUiWidth);
    std::memcpy(out.vga.data(), pal->data(), out.vga.size());
    palette_from_vga(out.vga, out.palette);
    return true;
}

bool load_sheet(const std::filesystem::path &dir, std::string_view name, SpriteSheet &sheet, std::string *error)
{
    auto tab = load_data_file(dir, "qdata/" + std::string(name) + ".tab", error);
    auto dat = tab ? load_data_file(dir, "qdata/" + std::string(name) + ".dat", error) : std::nullopt;
    if (!dat)
        return false;
    if (!sheet.load(std::move(*tab), std::move(*dat), SpriteFormat::Req)) {
        if (error)
            *error = std::string(name) + ": not a sprite table";
        return false;
    }
    return true;
}
} // namespace

inline namespace charts_detail
{

bool has_room(const World &world, uint8_t type)
{
    return std::any_of(world.rooms.begin() + 1, world.rooms.end(),
                       [&](const RoomInfo &r) { return r.type == type && r.owner == 0; });
}

} // namespace charts_detail

Frame::Frame(Charts &c, const UiMouse &m, const ChartKeys &k, const Game &g, const HospitalSettings &s, ChartsResult &r)
    : c_(c), keys_(k), game_(g), hs_(s), r_(r), dst_(c.surface), text_(*c.text)
{
    mx_ = m.x;
    my_ = m.y;
    left_ = m.left_pressed;
    mleft_ = m.left_held;
    rleft_ = m.left_released;
    mright_ = m.right_held;
    rright_ = m.right_released;
    inkey_escape_ = k.escape || k.space;
}

void Frame::run()
{
    switch (c_.chart) {
    case kChartStatement:
        bank_statement();
        break;
    case kChartCasebook:
        display_drugs();
        break;
    case kChartPolicy:
        display_policy();
        break;
    case kChartResearch:
        display_research();
        break;
    case kChartGraphs:
        draw_graphs(kBoxes[46].x, kBoxes[46].y, kBoxes[46].h, kBoxes[46].w);
        break;
    case kChartStaff:
        display_slist();
        break;
    case kChartBank:
        display_manager();
        break;
    case kChartStatus:
        display_peep_overview();
        break;
    default:
        r_.event = {EV_CLOSE_SCREEN};
        break;
    }
    draw_help();
}

// The chart's help tooltip (src/helpmsg.cpp's per-screen compute_help_text
// calls), drawn over the chart in its palette. The bank manager's three
// groups are tested in the original's order.
void Frame::draw_help()
{
    if (!c_.help || !c_.help_context)
        return;
    HelpSystem &help = *c_.help;
    HelpContext &ctx = *c_.help_context;
    if (c_.chart == kChartBank) {
        help.compute_help_text(ctx, 0, 0, c_.value ? 0x17 : 0x16);
        help.compute_help_text(ctx, 0, 0, 0x15);
    } else {
        help.compute_help_text(ctx, 0, 0, uint8_t(chart_help_group(c_.chart)));
    }
    HelpBox &box = help.box();
    box.screen_pic_loaded = true;
    help.draw(dst_, c_.palette);
    box.screen_pic_loaded = false;
}

int Frame::chart_help_group(int chart)
{
    switch (chart) {
    case kChartStatement:
        return 0xf;
    case kChartCasebook:
        return 0xe;
    case kChartPolicy:
        return 0x19;
    case kChartResearch:
        return 0x10;
    case kChartGraphs:
        return 0x11;
    case kChartStaff:
        return 6;
    case kChartStatus:
        return 0x18;
    default:
        return 0;
    }
}

// load_ingame_pic: the chart's picture shown in its palette.
void Frame::load_pic()
{
    const Charts::Picture &p = c_.pictures[size_t(c_.chart)];
    c_.previous_palette = c_.palette;
    c_.palette = p.palette;
    copy_screen();
}

// copy_screen_to_wscreen: the picture redrawn.
void Frame::copy_screen()
{
    const Surface &src = c_.pictures[size_t(c_.chart)].pixels;
    for (int y = 0; y < kUiHeight; y++)
        std::memcpy(dst_.row(y), src.data() + size_t(y) * kUiWidth, kUiWidth);
}

// LbTextSetWindow then LbTextDraw: `s` at `at` in `window`, clipped to
// it.
void Frame::text(int slot, Rect window, Point at, std::string_view s, unsigned flags, uint8_t colour)
{
    draw_text(dst_, font(slot), at.x, at.y, s, {{window.x, window.y, window.w}, window, flags, colour});
}

void Frame::text(int slot, int x, int y, std::string_view s, unsigned flags, uint8_t colour)
{
    text(slot, {0, 0, kUiWidth, kUiHeight}, {x, y}, s, flags, colour);
}

// exit_quick_requester (src/finance.cpp): Escape or Space leaves.
bool Frame::exit_quick_requester()
{
    if (!inkey_escape_)
        return false;
    inkey_escape_ = false;
    sound(kSfxNo4);
    return true;
}

// process_keys for F1-F8 within a chart.
void Frame::chart_key(int chart)
{
    if (!game_.paused)
        r_.event = {EV_OPEN_SCREEN, uint16_t(chart)};
}

void start_hospital_settings(HospitalSettings &settings, const Balance &balance)
{
    settings = {};
    settings.rest_threshold = balance.gbv.tired;
    for (int i = 0; i < kIllnessCount; i++) {
        IllnessFigures &f = settings.illnesses[size_t(i)];
        f.discovered = balance.expertise[size_t(i)].known;
        if (kExpertise[size_t(i)].kind == kObjPharmacyCabinet) {
            f.drug_effectiveness = balance.gbv.start_rating;
            f.drug_cost = balance.gbv.start_cost;
        }
    }
}

void setup_insurances(World &world, uint32_t clock)
{
    std::vector<uint16_t> pool = {1, 2, 3, 4, 5, 6, 7, 8};
    for (size_t i = 0; i < 3; i++) {
        size_t pick = clock % pool.size();
        world.ins_control[i] = pool[pick];
        pool.erase(pool.begin() + std::ptrdiff_t(pick));
    }
}

namespace
{

// apply_chart_event: a bank statement entry and its change to the balance.
void chart_pay(Game &game, uint8_t type, int32_t amount, uint16_t arg = 0)
{
    change_balance(game.finance, game.balance, game.date.day, game.date.month, {type, amount, arg});
}

// apply_chart_event: a member of staff made 50 happier, up to 100.
void make_happier(Thing &t)
{
    t.happiness = uint16_t(std::clamp(int16_t(t.happiness + 50), int16_t(0), int16_t(100)));
}

// EV_SET_POLICY: policy setting `event.value` (1 send home, 2 guess a cure,
// 3 diagnosis, 4 rest, 5 staff leave rooms) set to `event.arg`.
void set_policy(HospitalSettings &settings, const Event &event)
{
    switch (event.value) {
    case 1:
        settings.send_home_threshold = std::min<int>(event.arg, settings.guess_cure_threshold);
        break;
    case 2:
        settings.guess_cure_threshold = std::max<int>(event.arg, settings.send_home_threshold);
        break;
    case 3:
        settings.diag_threshold = event.arg;
        break;
    case 4:
        trace_event("policy_rest %u", unsigned(event.arg));
        settings.rest_threshold = event.arg;
        break;
    case 5:
        settings.staff_leave_rooms = event.arg != 0;
        break;
    }
}

// alter_research (src/rschscrn.cpp): kept if the total would pass 100. The
// game reads the old value unsigned, so a closed category is opened too;
// the total is a byte.
void alter_research(HospitalSettings &settings, const Event &event)
{
    trace_event("alter_research %u value=%u", unsigned(uint8_t(event.value)), unsigned(uint8_t(event.arg)));
    if (event.value >= 5)
        return;
    int8_t &alloc = settings.research_alloc[event.value];
    int8_t old = alloc;
    alloc = int8_t(uint8_t(event.arg));
    uint8_t total = 0;
    for (int8_t a : settings.research_alloc)
        if (a > 0)
            total = uint8_t(total + a);
    if (total > 100)
        alloc = old;
}

// EV_PRICE_UP and EV_PRICE_DOWN: illness `event.arg`'s price scale moved
// by `event.value` percent within 50% and 200%, and its price with it.
void change_price(Game &game, const Event &event)
{
    if (event.arg >= kIllnessCount)
        return;
    HospitalMarket &hosp = game.arrivals.hospitals[0];
    int32_t &scale = hosp.price_scale[event.arg];
    bool up = event.type == EV_PRICE_UP;
    if (up ? scale < 20000 : scale > 5000) {
        scale = std::clamp(scale + (up ? 1 : -1) * event.value * 100, 5000, 20000);
        hosp.price[event.arg] = scale * max_price_for_treatment(hosp, game.settings(), event.arg) / 10000;
    }
}

// increase_staff_salaries (src/staff.cpp), by an amount: for one, up to
// 90000; for all the player's staff the game caps no one. Either way their
// discontent is cleared and they are made happier.
void increase_staff_salaries(Game &game, const Event &event)
{
    for (size_t i = 1; i < game.world.things.size(); i++) {
        Thing &t = game.world.things[i];
        bool chosen = event.arg ? i == event.arg : t.type == kThingStaff && t.owner == 0;
        if (!chosen)
            continue;
        t.salary += event.value;
        if (event.arg && t.salary > 90000)
            t.salary = 90000;
        t.discontent = 0;
        make_happier(t);
    }
}

// pay_staff_bonus (src/staff.cpp), by an amount.
void pay_staff_bonus(Game &game, const Event &event)
{
    if (event.arg) {
        if (game.balance >= event.value && game.balance > 0 && event.arg < game.world.things.size()) {
            make_happier(game.world.things[event.arg]);
            chart_pay(game, kTxPersonalBonus, -int32_t(event.value), event.arg);
        }
        return;
    }
    int32_t total = 0;
    for (const Thing &t : game.world.things)
        if (t.type == kThingStaff && t.owner == 0)
            total += event.value;
    if (game.balance >= total && game.balance > 0) {
        for (Thing &t : game.world.things)
            if (t.type == kThingStaff && t.owner == 0)
                make_happier(t);
        chart_pay(game, kTxGeneralBonus, -total);
    }
}

} // namespace

bool apply_chart_event(Game &game, HospitalSettings &settings, const Event &event)
{
    Finance &fin = game.finance;
    const Balance &bal = game.settings();
    switch (event.type) {
    case EV_OPEN_SCREEN:
        put_back_held_staff(game);
        update_cure_costs(game.arrivals, bal);
        if (event.value == kChartBank || event.value == kChartStatus)
            calculate_value(fin, game.world, bal, game.reputation());
        return true;
    case EV_TAKE_LOAN:
        chart_pay(game, kTxBankLoan, 5000);
        fin.loan += 5000;
        return true;
    case EV_REPAY_LOAN:
        if (game.balance >= 5000 && game.balance > 0) {
            chart_pay(game, kTxLoanRepayment, -5000);
            fin.loan -= 5000;
        }
        return true;
    case EV_SET_POLICY:
        set_policy(settings, event);
        return true;
    case EV_ALTER_RESEARCH:
        alter_research(settings, event);
        return true;
    case EV_CONCENTRATE_RESEARCH:
        settings.research_illness = event.value == settings.research_illness ? 0 : uint8_t(event.value);
        return true;
    case EV_PRICE_UP:
    case EV_PRICE_DOWN:
        change_price(game, event);
        return true;
    case EV_INCREASE_STAFF_SALARIES:
        increase_staff_salaries(game, event);
        return true;
    case EV_PAY_STAFF_BONUS:
        pay_staff_bonus(game, event);
        return true;
    default:
        return false;
    }
}

bool load_charts(Charts &charts, const std::filesystem::path &dir, const LanguageText &text,
                 const Palette &game_palette, std::string *error)
{
    for (size_t i = 0; i < kChartCount; i++) {
        if (!load_picture(dir, kPictures[i], charts.pictures[i], error) ||
            !load_sheet(dir, kSprites[i], charts.pictures[i].sprites, error))
            return false;
    }
    for (int slot : kChartFonts)
        if (!load_font(dir, slot, charts.fonts[size_t(slot)], error))
            return false;
    auto faces = load_data_file(dir, "qdata/face01v.dat", error);
    if (!faces)
        return false;
    charts.faces = std::move(*faces);
    charts.text = &text;
    charts.game_palette = game_palette;
    charts.palette = game_palette;
    charts.surface.resize(kUiWidth, kUiHeight);
    return true;
}

void chart_command(Charts &charts, const Event &event)
{
    switch (event.type) {
    case EV_OPEN_SCREEN:
        if (event.value >= kChartCount)
            return;
        if (!charts.open)
            charts.palette = charts.game_palette;
        charts.open = true;
        charts.chart = event.value;
        charts.state = 0;
        break;
    case EV_RETURN_FROM_SCREEN:
    case EV_CLOSE_SCREEN:
    case EV_RETURN_TO_THING_WINDOW:
    case EV_OPEN_YES_NO:
        charts.open = false;
        break;
    default:
        break;
    }
}

ChartsResult chart_frame(Charts &charts, const UiMouse &mouse, const ChartKeys &keys, double seconds, const Game &game,
                         const HospitalSettings &settings)
{
    ChartsResult result;
    if (!charts.open || !charts.text)
        return result;
    UiMouse &m = charts.pending_mouse;
    bool lp = m.left_pressed, rp = m.right_pressed, lr = m.left_released, rr = m.right_released;
    m = mouse;
    m.left_pressed |= lp;
    m.right_pressed |= rp;
    m.left_released |= lr;
    m.right_released |= rr;
    ChartKeys &k = charts.pending_keys;
    k.escape |= keys.escape;
    k.space |= keys.space;
    k.up = keys.up;
    k.down = keys.down;
    k.left = keys.left;
    k.right = keys.right;
    k.pad_held = keys.pad_held;
    k.pad_pressed |= keys.pad_pressed;
    charts.frame_time += seconds;
    // The first frame of a chart runs at once: it shows the picture.
    if (charts.frame_time < kGameFrame && charts.state != 0)
        return result;
    charts.frame_time = std::max(charts.frame_time - kGameFrame, 0.0);
    charts.frame_time = std::min(charts.frame_time, kGameFrame);
    charts.clock_ms += 40;
    Frame(charts, m, k, game, settings, result).run();
    m.left_pressed = m.right_pressed = m.left_released = m.right_released = false;
    k = {};
    return result;
}

Event open_chart_command(int chart, const Game &game, bool *refused)
{
    if (refused)
        *refused = false;
    if (chart == kChartResearch && !has_room(game.world, kRoomResearch)) {
        if (refused)
            *refused = true;
        return {};
    }
    return {EV_OPEN_SCREEN, uint16_t(chart)};
}

bool slist_view(const Charts &charts, const World &world, Rect &box, Point &camera)
{
    if (!charts.open || charts.chart != kChartStaff || !charts.value || size_t(charts.value) >= world.things.size())
        return false;
    box = kSlistView;
    Point c = info_centre(world, uint16_t(charts.value));
    camera = {c.x - box.w / 2, c.y - box.h / 2};
    return true;
}

int chart_for_panel_action(PanelAction action)
{
    switch (action) {
    case PanelAction::BankManager:
        return kChartBank;
    case PanelAction::BankStatement:
        return kChartStatement;
    case PanelAction::StaffManagement:
        return kChartStaff;
    case PanelAction::Casebook:
        return kChartCasebook;
    case PanelAction::Research:
        return kChartResearch;
    case PanelAction::Status:
        return kChartStatus;
    case PanelAction::Charts:
        return kChartGraphs;
    case PanelAction::Policy:
        return kChartPolicy;
    default:
        return -1;
    }
}

} // namespace th
