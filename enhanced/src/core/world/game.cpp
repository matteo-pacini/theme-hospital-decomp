#include "core/world/game.h"

#include "core/world/build.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/staff.h"

#include <array>

namespace th
{

namespace
{

constexpr std::array<int, 12> kCalendar = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Object states (the game's OS_*, include/enums.h) handled so far.
constexpr uint8_t kObjectStatic = 2;

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
    start_arrivals(arrivals, settings);
    new_patients.clear();
    walkers_.assign(kThingCount, false);
    start_level(world, level, anims);
    world.entrances = level.entrances;
    world.rooms = {};
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
    reputation = 500;
    start_finance(finance, balance, settings, number);
    start_queues(world, settings_, arrivals.hospitals[0], reputation, finance);
}

void Game::turn()
{
    if (paused)
        return;
    turn_count++;
    draw_turn_randoms(world);

    new_patients.clear();
    if (turn_count % (kTurnsPerDay / 2) == 0) {
        // twice_daily_admin
        if (uint16_t id =
                create_a_peep(world, arrivals, settings_, *anims_, *level_, level_number_, open, turn_count)) {
            new_patients.push_back(id);
            start_person_going(world, *anims_, *complexes_, id, turn_count);
        }
    }
    if (turn_count % kTurnsPerDay == 0) {
        // daily_admin, then month_end and year_end (src/finance.cpp).
        date.day++;
        if (date.day > days_in_month(date.month)) {
            // month_end's money, before the month moves on.
            month_end(finance, balance, world, settings_, level_number_, days_in_month(date.month), date.month,
                      count_radiators(world, 0), reputation);
            calc_visitors_this_month(arrivals, settings_);
            date.day = 1;
            date.month++;
            if (date.month >= 12) {
                date.month = 0;
                date.year++;
                year_end(finance, balance, world, settings_, reputation);
            }
            arrivals.game_month = uint16_t(date.year * 12 + date.month);
            create_job_market(world, staff_market, settings_, date.year * 12 + date.month, turn_count);
            monthly_update_of_advisor_counts(advisor);
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
}

void Game::process_turn()
{
    Event e = event;
    event = {};
    // Each part's commands are carried out in its own file. EV_CANCEL_BUILD
    // is the build tool's, the staff side's and the objects', in that order.
    if (apply_build_event(*this, e))
        return;
    if (apply_town_event(*this, land, e) || apply_chart_event(*this, hospital, e))
        return;
    if (e.type == EV_OPEN_HIRE_STAFF)
        cancel_buy_objects(world.placing);
    if (!apply_staff_event(*this, e))
        apply_object_event(*this, e);
}

uint16_t Game::add_walker(int cell)
{
    // Illness 16, the first of the illnesses that do not change a patient's
    // looks.
    uint16_t id = make_a_peep(world, arrivals, settings_, *anims_, level_number_, uint16_t(cell), 16, 0);
    if (id)
        walkers_[id] = true;
    return id;
}

void Game::send_walker(uint16_t id, int cell)
{
    if (id >= world.things.size())
        return;
    walkers_[id] = true;
    set_destination_coords(world, id, cell);
}

} // namespace th
