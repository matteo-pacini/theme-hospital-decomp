#include "core/world/game.h"

#include "core/world/nav.h"
#include "core/world/objects.h"
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
                 const PersonComplexes &complexes)
{
    complexes_ = &complexes;
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
}

void Game::turn()
{
    if (paused)
        return;
    turn_count++;
    // The per-turn random values (rand_pct, queue_near_dist,
    // object_near_dist): unused so far, but drawn to keep the generator in
    // step with the game's.
    old_newrand(world.seed);
    old_newrand(world.seed);
    old_newrand(world.seed);

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
            calc_visitors_this_month(arrivals, settings_);
            date.day = 1;
            date.month++;
            if (date.month >= 12) {
                date.month = 0;
                date.year++;
            }
            arrivals.game_month = uint16_t(date.year * 12 + date.month);
            create_job_market(world, staff_market, settings_, date.year * 12 + date.month, turn_count);
        }
    }

    // The game holds the countdown while the advisor's opening message is
    // up (advisor.flags & 1); there is no advisor yet.
    if (!open && --opening_countdown <= 0) {
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
            staff_turn(world, *anims_, uint16_t(i), turn_count);
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
}

void Game::process_turn()
{
    Event e = event;
    event = {};
    // Each part's commands are carried out in its own file. EV_CANCEL_BUILD
    // is both the staff side's and the objects': staff's part first.
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
