#include "core/world/game.h"

#include "core/world/build.h"
#include "core/world/computer.h"
#include "core/world/counts.h"
#include "core/world/disaster.h"
#include "core/world/epidemic.h"
#include "core/world/fax_screen.h"
#include "core/world/icons.h"
#include "core/world/mayor.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/quake.h"
#include "core/world/queue.h"
#include "core/world/rats.h"
#include "core/world/reaper.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstring>
#include <format>
#include <optional>
#include <sstream>
#include <type_traits>

namespace th
{

namespace
{

constexpr std::array<int, 12> kCalendar = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Object states (the game's OS_*, include/enums.h) handled so far.
constexpr uint8_t kObjectStatic = 2;
constexpr uint8_t kObjectInUse = 4;
constexpr uint8_t kObjectPlantAges = 0x0a;

// The bytes of `now` that differ from `was` as "<offset> <hex>", from the
// first that differs to the last; all of them if `all`; empty if none differ.
template <typename T> std::string changed_bytes(const T &now, const T &was, bool all)
{
    static_assert(std::is_trivially_copyable_v<T>);
    const auto *a = reinterpret_cast<const uint8_t *>(&now);
    const auto *b = reinterpret_cast<const uint8_t *>(&was);
    size_t first = 0, last = sizeof(T);
    if (!all) {
        while (first < last && a[first] == b[first])
            first++;
        while (last > first && a[last - 1] == b[last - 1])
            last--;
        if (first == last)
            return {};
    }
    std::string s = std::format("{} ", first);
    for (size_t i = first; i < last; i++)
        s += std::format("{:02x}", a[i]);
    return s;
}

// Writes the bytes `hex` spells into `target` from byte `offset`. Returns
// false, changing nothing, if they are not hex or do not fit.
template <typename T> bool set_bytes(T &target, size_t offset, std::string_view hex)
{
    static_assert(std::is_trivially_copyable_v<T>);
    if (hex.size() % 2 || offset > sizeof(T) || hex.size() / 2 > sizeof(T) - offset)
        return false;
    std::vector<uint8_t> bytes(hex.size() / 2);
    for (size_t i = 0; i < bytes.size(); i++) {
        auto [end, ec] = std::from_chars(hex.data() + 2 * i, hex.data() + 2 * i + 2, bytes[i], 16);
        if (ec != std::errc() || end != hex.data() + 2 * i + 2)
            return false;
    }
    std::memcpy(reinterpret_cast<uint8_t *>(&target) + offset, bytes.data(), bytes.size());
    return true;
}

} // namespace

int days_in_month(int month)
{
    return kCalendar[size_t(month % 12)];
}

void Game::start(const Level &level, int number, const Balance &settings, const AnimData &anims,
                 const PersonComplexes &complexes, const StaffComplexes &staff_complexes)
{
    complexes_ = &complexes;
    staff_complexes_ = &staff_complexes;
    int open_countdown = settings.gbv.open_countdown;
    anims_ = &anims;
    settings_ = settings;
    level_ = std::make_unique<Level>(level);
    level_number_ = number;
    events.events.clear();
    start_arrivals(arrivals, settings);
    new_patients.clear();
    walkers_.assign(kThingCount, false);
    start_level(world, level, anims);
    world.scroll_min = level.scroll_min;
    world.scroll_max = level.scroll_max;
    start_entrance_rooms(world, level);
    start_town_land(land, level, number);
    std::array<uint8_t, 3> refs = staff_market.ref_counts;
    staff_market = {};
    staff_market.ref_counts = refs;
    create_job_market(world, staff_market, settings, 0, 1);
    date = {};
    fax_screen = {};
    world.fax_palette = false;
    turn_count = 1;
    paused = false;
    opening_countdown = open_countdown;
    this->open_countdown = open_countdown;
    open = open_countdown == 0;
    balance = size_t(number) < settings.towns.size() ? int(settings.towns[size_t(number)].start_cash) : 0;
    start_finance(finance, balance, settings, number);
    // init_player's.
    count_hospital_area(finance, world);
    start_queues(world, settings_, arrivals.hospitals[0], finance);
    // init_player's.
    calc_pricing_factor(arrivals.hospitals[0], settings_, world.cure_available);
    start_rivals(world, settings_, balance);
    start_rooms_in_use(*this, complexes, staff_complexes);
    connect_world();
    start_research(world);
    start_year_end(*this);
    world.emergency = {};
    world.emergency.heli_pad_tile = level.emergency_tile[0];
    disasters = {};
    // After the insurers in the game, which draw nothing from the seed. The
    // mouse position only enters the random pick of a missing player.
    setup_computer_players(world, settings_, number, finance.area, 0, 0);
    setup_next_emergency(disasters, world, settings_, date);
    setup_next_quake(disasters, world, settings_, date);
    command_log.assign(1, std::format("# commands of level {}, build {:016x}", number, snapshot_build_id()));
    inputs_logged_ = false;
}

void Game::connect_world()
{
    world.settings = &settings_;
    world.market = &arrivals.hospitals[0];
    world.staff_market = &staff_market;
    world.finance = &finance;
    world.hospital = &hospital;
    world.ledger = &finance;
    world.balance = &balance;
    world.date = &date;
    world.person_complexes = complexes_;
    world.staff_complexes = staff_complexes_;
    world.game = this;
}

namespace
{

// Game::turn's month_end (src/finance.cpp), with the year end at the end of
// the year. month_end's own order is kept: the market shares, then the month
// moves on, then the awards' monthly figures and the prices, and only then
// the year end (which start_turn calls after month_end).
void finish_month_market_share(Game &g)
{
    rival_market_share(g.world, g.arrivals);
    int32_t total = 0;
    for (const HospitalMarket &h : g.arrivals.hospitals)
        total += int32_t(h.month_visitors);
    g.progress.market_share = total != 0 ? int32_t(g.arrivals.hospitals[0].month_visitors) * 100 / total : 0;
    for (HospitalMarket &h : g.arrivals.hospitals)
        h.month_visitors = 0;
}

void turn_month_end(Game &g)
{
    Date &date = g.date;
    const Balance &settings = g.settings();
    auto_available(g.world, date);
    // month_end's money, before the month moves on.
    month_end(
        g.finance, g.balance, g.world, settings, g.level_number(),
        {days_in_month(date.month), date.month, count_radiators(g.world, 0), g.reputation(), g.turn_count, &g.anims()});
    calc_pricing_factor(g.arrivals.hospitals[0], settings, g.world.cure_available);
    rival_month_end(g.world, g.arrivals, settings, g.finance.heating_price);
    computer_month(g.world, g.arrivals);
    finish_month_market_share(g);
    calc_visitors_this_month(g.arrivals, settings);
    date.day = 1;
    date.month++;
    g.arrivals.game_month = uint16_t(date.year * 12 + date.month);
    create_job_market(g.world, g.staff_market, settings, date.year * 12 + date.month, g.turn_count);
    monthly_update_of_award_counts(g, date.month);
    monthly_update_of_advisor_counts(g.advisor);
    update_cure_costs(g.arrivals, settings);
    // The game's year_end runs after month_end, from start_turn; the
    // quarterly check when the month is a quarter's (level 12 and later have
    // no early win).
    if (date.month >= 12)
        year_end_sim(g);
    else if (date.month % 3 == 0)
        check_quarterly_game_over(g);
}

// Game::turn's daily_admin, then month_end and year_end (src/finance.cpp).
void turn_daily_admin(Game &g)
{
    g.date.day++;
    try_to_generate_emergency(g);
    generate_mayors(g);
    daily_admin_emergency(g);
    generate_disaster(g, 0);
    generate_quake(g);
    expire_playercalls(g.world);
    manage_rats(g.world, g.anims());
    computer_day(g.world, g.arrivals, g.date.day);
    do_research(g.world);
    if (g.date.day > days_in_month(g.date.month))
        turn_month_end(g);
}

// process_things' turn of object or explosion debris `id`.
void process_things_object(World &world, const AnimData &anims, uint16_t id, uint32_t turn_count)
{
    Thing &t = world.things[id];
    if (t.type == kThingDebris) {
        process_bang(world, anims, id, turn_count);
    } else if (t.state == kObjectStatic) {
        // object_static (src/objects.cpp).
        if (t.anim_speed && turn_count % t.anim_speed == 0 && t.sprite < anims.frames.size()) {
            t.sprite = anims.frames[t.sprite].next;
            if (t.overlay_frame && t.overlay_frame < anims.frames.size())
                t.overlay_frame = anims.frames[t.overlay_frame].next;
        }
    } else if (t.state == kObjectInUse) {
        // A machine in use smokes every turn.
        if (t.overlay_frame && t.overlay_frame < anims.frames.size())
            t.overlay_frame = anims.frames[t.overlay_frame].next;
    } else if (t.state == kObjectHell) {
        object_hell(world, anims, id, turn_count);
    } else if (t.state == kObjectPlantAges) {
        plant_ages(world, anims, id, turn_count);
    } else {
        sliding_door_turn(world, anims, id, turn_count);
    }
}

// process_things' turn of the thing at `i`, once its counts are taken.
void turn_thing(Game &g, std::vector<bool> &walkers, const PersonComplexes &complexes,
                const StaffComplexes &staff_complexes, size_t i, uint32_t turn_count)
{
    World &world = g.world;
    const AnimData &anims = g.anims();
    Thing &t = world.things[i];
    if (t.type == kThingPatient && walkers[i]) {
        if (walk_turn(world, anims, uint16_t(i), turn_count) != WalkResult::Walking)
            play_frame(t, anims, kPatientAnims[t.subtype % kPatientAnims.size()].stand);
    } else if (t.type == kThingPatient) {
        person_turn(world, anims, complexes, uint16_t(i), turn_count);
    } else if (t.type == kThingStaff) {
        staff_turn(world, anims, complexes, staff_complexes, uint16_t(i), turn_count);
    } else if (t.type == kThingDoor) {
        door_turn(world, anims, uint16_t(i), turn_count);
    } else if (t.type == kThingNumfx) {
        numfx_ages(world, uint16_t(i));
    } else if (t.type == kThingRat) {
        rat_turn(world, anims, uint16_t(i));
    } else if (t.type == kThingReaper) {
        reaper_turn(world, anims, uint16_t(i), turn_count);
    } else if (t.type == kThingHelicopter) {
        helicopter_turn(world, anims, uint16_t(i));
    } else if (t.type == kThingInspector) {
        inspector_turn(g, uint16_t(i), turn_count);
    } else if (t.type == kThingMayor) {
        mayor_turn(g, uint16_t(i), turn_count);
    } else if (t.type == kThingObject || t.type == kThingDebris) {
        process_things_object(world, anims, uint16_t(i), turn_count);
    }
}

// process_things, after the turn starts, in thing order. `walkers` are the
// path finding tool's.
void process_things(Game &g, std::vector<bool> &walkers, const PersonComplexes &complexes,
                    const StaffComplexes &staff_complexes)
{
    World &world = g.world;
    uint32_t turn_count = g.turn_count;
    reset_all_count(world, turn_count);
    check_win_status(g);
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &t = world.things[i];
        if (t.type == kThingFree)
            walkers[i] = false;
        if (t.type == kThingPatient)
            person_count(world, uint16_t(i), turn_count);
        else if (t.type == kThingStaff)
            staff_count(world, uint16_t(i), turn_count);
        else if (t.type == kThingObject)
            object_count(world, uint16_t(i), turn_count);
        // update_icon_counter (src/things.cpp), before the state turn, for
        // the things that show a status icon.
        if (t.type == kThingPatient || t.type == kThingStaff || t.type == kThingObject)
            update_icon_counter(g.anims(), t);
        turn_thing(g, walkers, complexes, staff_complexes, i, turn_count);
    }
    final_count(world, turn_count);
}

} // namespace

bool parse_skipped_calls(std::string_view names, uint8_t &calls)
{
    calls = 0;
    for (size_t start = 0; start < names.size();) {
        size_t end = std::min(names.find(',', start), names.size());
        if (names.substr(start, end - start) != "computer_action")
            return false;
        calls |= kSkipComputerAction;
        start = end + 1;
    }
    return true;
}

void Game::turn()
{
    if (start_turn())
        run_things();
}

bool Game::start_turn()
{
    log_inputs();
    if (paused)
        return false;
    turn_count++;
    EventScope scope(&events, turn_count);
    fax_fade(*this);
    draw_turn_randoms(world);
    world.idle_nurses = 0;

    new_patients.clear();
    if (turn_count % (kTurnsPerDay / 2) == 0) {
        // twice_daily_admin
        twice_daily_admin_emergency(world);
        if (epidemic_status(world, 0))
            generate_inspector(world, *anims_, 0);
        if (uint16_t id =
                create_a_peep(world, arrivals, {settings_, *anims_, level_number_}, open, turn_count, date.day)) {
            new_patients.push_back(id);
            start_person_going(world, *anims_, *complexes_, id, turn_count);
        }
        rat_fest_control(*this);
        if (!(skipped_calls & kSkipComputerAction))
            computer_action(*this);
    }
    if (turn_count % kTurnsPerDay == 0)
        turn_daily_admin(*this);

    // The countdown is held while the tutorial runs (advisor.flags & 1).
    if (!open && !(advisor.flags & kAdvisorTutorial) && --opening_countdown <= 0) {
        opening_countdown = 0;
        open = true;
    }
    if (disasters.quake_strength != 0)
        earthquake(*this, int8_t(disasters.quake_strength), disasters.quake_step_chance);
    // An epidemic call that ran out unanswered is covered up now, as the
    // game carries out the default answer the frame after daily_admin.
    if (world.epidemic.cover_up_expired) {
        world.epidemic.cover_up_expired = false;
        trace_event("choice 11 arg=99 awaiting=0");
        expire_epidemic_cover_up(world);
    }
    return true;
}

void Game::run_things()
{
    // The year-end turn pauses for the screens before the things run.
    if (std::exchange(progress.skip_things, false))
        return;
    EventScope scope(&events, turn_count);
    // effect_illness and person_dies tell the advisor (advisor.event_flags),
    // as helicopter_lands does (advisor.heli_landed).
    const uint16_t cures = finance.cures, deaths = finance.deaths;
    const uint8_t heli_landed = world.emergency.heli_landed;
    process_things(*this, walkers_, *complexes_, *staff_complexes_);
    if (finance.cures != cures)
        advisor.event_flags |= kAdvisorEventCure;
    if (finance.deaths != deaths)
        advisor.event_flags |= kAdvisorEventDeath;
    if (!heli_landed && world.emergency.heli_landed)
        advisor.heli_landed = true;
    advisor.event_flags |= std::exchange(world.advisor_events, 0);
    build_turn(*this);
    fax_frame(*this);
    remember_inputs();
}

void Game::process_turn()
{
    log_inputs();
    Event e = event;
    event = {};
    if (e.type != EV_NONE)
        log_line(std::format("{} {} {}", unsigned(e.type), e.value, e.arg));
    fax_event(*this, e);
    carry_out(e);
    if (choice_screen.type)
        fax_open(*this, choice_screen.timeout);
    advisor.event_flags |= std::exchange(world.advisor_events, 0);
    remember_inputs();
}

void Game::carry_out(const Event &e)
{
    EventScope scope(&events, turn_count + 1);
    choice_screen = {};
    if (e.type != EV_NONE)
        trace_event("cmd type=%u value=%u arg=%u", unsigned(e.type), unsigned(e.value), unsigned(e.arg));
    if (e.type == EV_CHOICE_RESULT) {
        const Thing &p = world.things[e.arg < world.things.size() ? e.arg : 0];
        trace_event("choice %u arg=%u awaiting=%u", unsigned(e.value), unsigned(e.arg),
                    unsigned(p.type == kThingPatient && (p.person_flags & kPfAwaitingDecision)));
    }
    // process_turn's EV_OPEN_HOSPITAL: the panel's watch, clicked while the
    // countdown runs, opens the hospital at once. Single player, so the
    // player's hospital_open is the game's own open flag
    // (allhosps[player].hospital_open, io.cpp).
    if (e.type == EV_OPEN_HOSPITAL) {
        opening_countdown = 0;
        open = true;
        return;
    }
    // process_turn's EV_CHEAT_END_YEAR (src/io.cpp): December, two days
    // before its end; EV_CHEAT_END_MONTH: the current month's last day.
    if (e.type == EV_CHEAT_END_YEAR || e.type == EV_CHEAT_END_MONTH) {
        if (e.type == EV_CHEAT_END_YEAR)
            date.month = 11;
        date.day = days_in_month(date.month) - (e.type == EV_CHEAT_END_YEAR ? 2 : 1);
        return;
    }
    // The win/lose screen's decline button (winlose.cpp): the offered win is
    // refused and the game carries on. The replay sends it as EV_DECLINE_WIN.
    if (e.type == EV_DECLINE_WIN) {
        progress.win_declined = 1;
        progress.bonus_level = 0;
        return;
    }
    // process_turn's EV_SHOOT_RAT (src/io.cpp): argb[0] is the rat game's
    // weapon, argb[1] whether the shot hit a rat.
    if (e.type == EV_SHOOT_RAT) {
        shoot_rat(*this, e.value, uint8_t(e.arg & 0xff), uint8_t(e.arg >> 8));
        return;
    }

    // Each part's commands are carried out in its own file. EV_CANCEL_BUILD
    // is the staff side's, the build tool's and the objects', in that order.
    if (apply_emergency_event(*this, e) || apply_epidemic_event(*this, e) || apply_call_event(*this, e) ||
        apply_mayor_event(*this, e) || apply_machine_event(*this, e))
        return;
    if (e.type == EV_CANCEL_BUILD)
        apply_staff_event(*this, e);
    if (apply_build_event(*this, e))
        return;
    if (apply_town_event(*this, land, e) || apply_chart_event(*this, hospital, e) || apply_patient_event(*this, e))
        return;
    if (e.type == EV_OPEN_HIRE_STAFF)
        cancel_buy_objects(world.placing);
    if (e.type == EV_CANCEL_BUILD || !apply_staff_event(*this, e))
        apply_object_event(*this, e);
}

uint16_t Game::add_walker(int cell)
{
    log_inputs();
    log_line(std::format("@walker_add {}", cell));
    // Illness 16, the first of the illnesses that do not change a patient's
    // looks.
    uint16_t id = make_a_peep(world, arrivals, {settings_, *anims_, level_number_}, uint16_t(cell), 16, 0);
    if (id)
        walkers_[id] = true;
    return id;
}

void Game::send_walker(uint16_t id, int cell)
{
    log_inputs();
    log_line(std::format("@walker_send {} {}", id, cell));
    if (id >= world.things.size())
        return;
    walkers_[id] = true;
    set_destination_coords(world, id, cell);
}

void Game::update_placement(int cell)
{
    ObjectPlacing &p = world.placing;
    if (!p.placing || (cell == p.mouse_cell && !p.redraw))
        return;
    log_inputs();
    log_line(std::format("@cursor {}", cell));
    // The check's event belongs to the turn the command runs before.
    EventScope scope(&events, turn_count + 1);
    th::update_placement(world, p, cell);
    remember_inputs();
}

void Game::log_line(std::string_view text)
{
    command_log.push_back(std::format("{} {}", turn_count + 1, text));
}

void Game::log_inputs()
{
    const LoggedInputs &was = logged_inputs_;
    bool all = !inputs_logged_;
    bool tutorial = advisor.flags & kAdvisorTutorial;
    if (all || paused != was.paused)
        log_line(std::format("@paused {}", int(paused)));
    if (all || world.path_finder != was.path_finder)
        log_line(std::format("@path_finder {}", int(world.path_finder)));
    if (all || tutorial != was.tutorial)
        log_line(std::format("@tutorial {}", int(tutorial)));
    if (all || world.building.mouse != was.build_mouse)
        log_line(std::format("@build_mouse {}", int(world.building.mouse)));
    if (std::string bytes = changed_bytes(world.placing, was.placing, all); !bytes.empty())
        log_line("@placing " + bytes);
    if (std::string bytes = changed_bytes(world.building.build, was.build, all); !bytes.empty())
        log_line("@build " + bytes);
    if (std::string bytes = changed_bytes(hospital, was.hospital, all); !bytes.empty())
        log_line("@hospital " + bytes);
    if (std::string bytes = changed_bytes(world.ins_control, was.ins_control, all); !bytes.empty())
        log_line("@ins_control " + bytes);
    remember_inputs();
}

void Game::remember_inputs()
{
    LoggedInputs &now = logged_inputs_;
    now.paused = paused;
    now.path_finder = world.path_finder;
    now.tutorial = advisor.flags & kAdvisorTutorial;
    now.build_mouse = world.building.mouse;
    std::memcpy(&now.placing, &world.placing, sizeof now.placing);
    std::memcpy(&now.build, &world.building.build, sizeof now.build);
    std::memcpy(&now.hospital, &hospital, sizeof now.hospital);
    now.ins_control = world.ins_control;
    inputs_logged_ = true;
}

namespace
{

// Game::replay_input's failure: says `why` in `error`, if wanted.
bool replay_input_fail(std::string_view line, std::string *error, const char *why)
{
    if (error)
        *error = std::format("{}: {}", why, line);
    return false;
}

// Game::replay_input's next number from `in`, if from `low` to `high`.
std::optional<long> replay_input_value(std::istream &in, long low, long high)
{
    long v = low - 1;
    in >> v;
    return v >= low && v <= high ? std::optional<long>(v) : std::nullopt;
}

// Game::replay_input's command line: "<type> <value> <arg>".
bool replay_input_command(Game &g, std::string_view line, std::string *error)
{
    std::istringstream fields{std::string(line)};
    unsigned type = 256, v = 0x10000, arg = 0x10000;
    fields >> type >> v >> arg;
    if (!fields || type > 0xff || v > 0xffff || arg > 0xffff)
        return replay_input_fail(line, error, "not a command");
    g.event = {uint8_t(type), uint16_t(v), 0, uint16_t(arg)};
    g.process_turn();
    return true;
}

// Game::replay_input's @paused, @tutorial, @path_finder and @build_mouse.
bool replay_input_setting(Game &g, const std::string &name, std::istream &in, std::string_view line, std::string *error)
{
    auto v = replay_input_value(in, 0, name == "@build_mouse" ? int(BuildMouse::MoveQueuePerson) : 1);
    if (!v)
        return replay_input_fail(line, error, "bad value");
    if (name == "@paused")
        g.paused = *v;
    else if (name == "@path_finder")
        g.world.path_finder = PathFinder(*v);
    else if (name == "@build_mouse")
        g.world.building.mouse = BuildMouse(*v);
    else if (*v)
        g.advisor.flags |= kAdvisorTutorial;
    else
        g.advisor.flags &= uint8_t(~kAdvisorTutorial);
    return true;
}

// Game::replay_input's byte changes: "<name> <offset> <hex>".
bool replay_input_bytes(Game &g, const std::string &name, std::istream &in, std::string_view line, std::string *error)
{
    size_t offset = 0;
    std::string hex;
    in >> offset >> hex;
    bool ok = !in.fail();
    if (name == "@placing")
        ok = ok && set_bytes(g.world.placing, offset, hex);
    else if (name == "@build")
        ok = ok && set_bytes(g.world.building.build, offset, hex);
    else if (name == "@hospital")
        ok = ok && set_bytes(g.hospital, offset, hex);
    else if (name == "@ins_control")
        ok = ok && set_bytes(g.world.ins_control, offset, hex);
    else
        return replay_input_fail(line, error, "unknown input");
    return ok || replay_input_fail(line, error, "bad bytes");
}

} // namespace

bool Game::replay_input(std::string_view line, std::string *error)
{
    std::istringstream in{std::string(line)};
    std::string name;
    in >> name;
    if (name.empty() || name[0] != '@')
        return replay_input_command(*this, line, error);
    if (name == "@walker_add" || name == "@cursor") {
        auto cell = replay_input_value(in, 0, kMapCells - 1);
        if (!cell)
            return replay_input_fail(line, error, "bad cell");
        if (name == "@cursor")
            update_placement(int(*cell));
        else
            add_walker(int(*cell));
        return true;
    }
    if (name == "@walker_send") {
        auto id = replay_input_value(in, 0, kThingCount - 1);
        auto cell = replay_input_value(in, 0, kMapCells - 1);
        if (!id || !cell)
            return replay_input_fail(line, error, "bad walker or cell");
        send_walker(uint16_t(*id), int(*cell));
        return true;
    }
    if (name == "@paused" || name == "@tutorial" || name == "@path_finder" || name == "@build_mouse")
        return replay_input_setting(*this, name, in, line, error);
    return replay_input_bytes(*this, name, in, line, error);
}

} // namespace th
