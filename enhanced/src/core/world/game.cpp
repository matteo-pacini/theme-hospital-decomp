#include "core/world/game.h"

#include "core/world/build.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

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
    start_entrance_rooms(world, level);
    start_town_land(land, level, number);
    std::array<uint8_t, 3> refs = staff_market.ref_counts;
    staff_market = {};
    staff_market.ref_counts = refs;
    create_job_market(world, staff_market, settings, 0, 1);
    date = {};
    turn_count = 1;
    paused = false;
    opening_countdown = open_countdown;
    this->open_countdown = open_countdown;
    open = open_countdown == 0;
    balance = size_t(number) < settings.towns.size() ? int(settings.towns[size_t(number)].start_cash) : 0;
    start_finance(finance, balance, settings, number);
    start_queues(world, settings_, arrivals.hospitals[0], finance);
    start_rooms_in_use(world, hospital, finance, balance, date, complexes, staff_complexes);
    connect_world();
    disasters = {};
    setup_next_emergency(disasters, world, settings_, date);
    setup_next_quake(disasters, world, settings_, date);
    command_log.assign(1, std::format("# commands of level {}, build {:016x}", number, snapshot_build_id()));
    inputs_logged_ = false;
}

void Game::connect_world()
{
    world.settings = &settings_;
    world.market = &arrivals.hospitals[0];
    world.finance = &finance;
    world.hospital = &hospital;
    world.ledger = &finance;
    world.balance = &balance;
    world.date = &date;
    world.person_complexes = complexes_;
    world.staff_complexes = staff_complexes_;
}

void Game::turn()
{
    log_inputs();
    if (paused)
        return;
    turn_count++;
    EventScope scope(&events, turn_count);
    draw_turn_randoms(world);

    new_patients.clear();
    if (turn_count % (kTurnsPerDay / 2) == 0) {
        // twice_daily_admin
        if (uint16_t id = create_a_peep(world, arrivals, settings_, *anims_, level_number_, open, turn_count)) {
            new_patients.push_back(id);
            start_person_going(world, *anims_, *complexes_, id, turn_count);
        }
    }
    if (turn_count % kTurnsPerDay == 0) {
        // daily_admin, then month_end and year_end (src/finance.cpp).
        date.day++;
        try_to_generate_emergency(disasters, world, settings_, date, open, turn_count);
        expire_playercalls(world);
        if (date.day > days_in_month(date.month)) {
            // month_end's money, before the month moves on.
            month_end(finance, balance, world, settings_, level_number_, days_in_month(date.month), date.month,
                      count_radiators(world, 0), reputation());
            calc_visitors_this_month(arrivals, settings_);
            date.day = 1;
            date.month++;
            if (date.month >= 12) {
                date.month = 0;
                date.year++;
                year_end(finance, balance, world, settings_, reputation());
            }
            arrivals.game_month = uint16_t(date.year * 12 + date.month);
            create_job_market(world, staff_market, settings_, date.year * 12 + date.month, turn_count);
            monthly_update_of_advisor_counts(advisor);
            update_cure_costs(arrivals, settings_);
            // The game's year_end, which puts the held member of staff back
            // (start_year_end), runs after month_end.
            if (date.month == 0)
                put_back_held_staff(*this);
        }
    }

    // The countdown is held while the tutorial runs (advisor.flags & 1).
    if (!open && !(advisor.flags & kAdvisorTutorial) && --opening_countdown <= 0) {
        opening_countdown = 0;
        open = true;
    }

    // process_things, after the turn starts, in thing order.
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &t = world.things[i];
        if (t.type == kThingFree)
            walkers_[i] = false;
        if (t.type == kThingPatient && walkers_[i]) {
            if (walk_turn(world, *anims_, uint16_t(i), turn_count) != WalkResult::Walking)
                play_frame(t, *anims_, kPatientAnims[t.subtype % kPatientAnims.size()].stand);
        } else if (t.type == kThingPatient) {
            person_turn(world, *anims_, *complexes_, uint16_t(i), turn_count);
        } else if (t.type == kThingStaff) {
            staff_turn(world, *anims_, *complexes_, *staff_complexes_, uint16_t(i), turn_count);
        } else if (t.type == kThingDoor) {
            door_turn(world, *anims_, uint16_t(i), turn_count);
        } else if (t.type == kThingNumfx) {
            numfx_ages(world, uint16_t(i));
        } else if (t.type == kThingObject) {
            if (t.state == kObjectStatic) {
                // object_static (src/objects.cpp).
                if (t.anim_speed && turn_count % t.anim_speed == 0 && t.sprite < anims_->frames.size())
                    t.sprite = anims_->frames[t.sprite].next;
            } else {
                sliding_door_turn(world, *anims_, uint16_t(i), turn_count);
            }
        }
    }
    build_turn(*this);
    remember_inputs();
}

void Game::process_turn()
{
    log_inputs();
    Event e = event;
    event = {};
    if (e.type != EV_NONE)
        log_line(std::format("{} {} {}", unsigned(e.type), e.value, e.arg));
    carry_out(e);
    remember_inputs();
}

void Game::carry_out(const Event &e)
{
    EventScope scope(&events, turn_count + 1);
    if (e.type != EV_NONE)
        trace_event("cmd type=%u value=%u arg=%u", unsigned(e.type), unsigned(e.value), unsigned(e.arg));

    // Each part's commands are carried out in its own file. EV_CANCEL_BUILD
    // is the staff side's, the build tool's and the objects', in that order.
    if (e.type == EV_CANCEL_BUILD)
        apply_staff_event(*this, e);
    if (apply_build_event(*this, e))
        return;
    if (apply_town_event(*this, land, e) || apply_chart_event(*this, hospital, e))
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
    uint16_t id = make_a_peep(world, arrivals, settings_, *anims_, level_number_, uint16_t(cell), 16, 0);
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

bool Game::replay_input(std::string_view line, std::string *error)
{
    std::istringstream in{std::string(line)};
    std::string name;
    in >> name;
    auto fail = [&](const char *why) {
        if (error)
            *error = std::format("{}: {}", why, line);
        return false;
    };
    auto value = [&](long low, long high) {
        long v = low - 1;
        in >> v;
        return v >= low && v <= high ? std::optional<long>(v) : std::nullopt;
    };
    if (name.empty() || name[0] != '@') {
        // A command: "<type> <value> <arg>".
        std::istringstream fields{std::string(line)};
        unsigned type = 256, v = 0x10000, arg = 0x10000;
        fields >> type >> v >> arg;
        if (!fields || type > 0xff || v > 0xffff || arg > 0xffff)
            return fail("not a command");
        event = {uint8_t(type), uint16_t(v), 0, uint16_t(arg)};
        process_turn();
        return true;
    }
    if (name == "@walker_add" || name == "@cursor") {
        auto cell = value(0, kMapCells - 1);
        if (!cell)
            return fail("bad cell");
        if (name == "@cursor")
            update_placement(int(*cell));
        else
            add_walker(int(*cell));
        return true;
    }
    if (name == "@walker_send") {
        auto id = value(0, kThingCount - 1);
        auto cell = value(0, kMapCells - 1);
        if (!id || !cell)
            return fail("bad walker or cell");
        send_walker(uint16_t(*id), int(*cell));
        return true;
    }
    if (name == "@paused" || name == "@tutorial" || name == "@path_finder" || name == "@build_mouse") {
        auto v = value(0, name == "@build_mouse" ? int(BuildMouse::WaitClear) : 1);
        if (!v)
            return fail("bad value");
        if (name == "@paused")
            paused = *v;
        else if (name == "@path_finder")
            world.path_finder = PathFinder(*v);
        else if (name == "@build_mouse")
            world.building.mouse = BuildMouse(*v);
        else if (*v)
            advisor.flags |= kAdvisorTutorial;
        else
            advisor.flags &= uint8_t(~kAdvisorTutorial);
        return true;
    }
    size_t offset = 0;
    std::string hex;
    in >> offset >> hex;
    bool ok = !in.fail();
    if (name == "@placing")
        ok = ok && set_bytes(world.placing, offset, hex);
    else if (name == "@build")
        ok = ok && set_bytes(world.building.build, offset, hex);
    else if (name == "@hospital")
        ok = ok && set_bytes(hospital, offset, hex);
    else if (name == "@ins_control")
        ok = ok && set_bytes(world.ins_control, offset, hex);
    else
        return fail("unknown input");
    return ok || fail("bad bytes");
}

} // namespace th
