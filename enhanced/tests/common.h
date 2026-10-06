// Shared by the engine tests: the check macro and the fixtures and helpers
// more than one test file uses.

#pragma once

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/world/game.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/staff.h"
#include "core/world/world.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>

extern int failures;

#define CHECK(cond)                                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);                              \
            failures++;                                                                                                \
        }                                                                                                              \
    } while (0)

// Every cell's list is well linked and holds the things on that cell.
bool cell_lists_consistent(const th::World &w);

// Hire staff (milestone 4 step 3): starts level 1 with its job market made
// as setup_new_level makes it.
struct HireFixture {
    th::GameData gd;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<th::Game> game = std::make_unique<th::Game>();
    th::StaffMarket market;
    // The simulation's generator after the level started.
    uint32_t seed = 0;

    bool start(const char *dir)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        level = gd.load_level(1, &error);
        if (!level || !th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error))
            return false;
        // Game::start makes the first applicants, as the game does.
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        seed = game->world.seed;
        market = game->staff_market;
        return true;
    }
};

// A cell near the level's camera start where staff may be put down.
int free_staff_cell(const th::World &w, const th::Level &level);

// Patients at reception: level 1 (FULL) opened at once, with the queues'
// part of the world set up as Game::start is to set it.
struct QueueFixture {
    th::GameData gd;
    std::optional<th::Level> level;
    th::Balance settings;
    std::unique_ptr<th::Game> game = std::make_unique<th::Game>();

    bool start(const char *dir)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        level = gd.load_level(1, &error);
        if (!level || !th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error))
            return false;
        settings.gbv.open_countdown = 0;
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        th::start_queues(game->world, game->settings(), game->arrivals.hospitals[0], game->finance);
        return game->open;
    }

    // A game turn, with the turn's distances as draw_turn_randoms draws them
    // at its start.
    void turn()
    {
        uint32_t seed = game->world.seed;
        th::draw_turn_randoms(game->world);
        game->world.seed = seed;
        game->turn();
    }

    // Places an object of `kind` facing 0 on the first cell from `from` on
    // (along the row, then the next rows) where it fits.
    uint16_t place_near(int from, int kind)
    {
        for (int d = 0; d < 6 * th::kMapSize; d++)
            if (uint16_t id = th::place_object(game->world, gd.anims(), from + d, kind, 0, 0, false))
                return id;
        return 0;
    }
};

// Starts the rooms in use for a QueueFixture's game, with the player's
// settings as the app starts them (start_hospital_settings).
void start_rooms(QueueFixture &f);

// Lays out a GP's office's 5 x 5 blueprint on the free site of the
// hospital's land nearest `near` with a clear ring round it, through the
// build commands as the player gives them, and commits it: its door in the
// middle of its north wall. Returns the room, 0 if none could be laid out.
uint16_t lay_out_gp_office(QueueFixture &f, int near);

// Commits the blueprint's doors, runs turns until the room is built (its
// floor clear), places its objects where each first may go and opens it.
// Returns whether it opened.
bool finish_gp_office(QueueFixture &f, uint16_t r);

// Hires the first applicant of staff kind `kind` onto the first free cell
// from `from` on where staff may go. Returns them, 0 if none.
uint16_t hire_near(QueueFixture &f, uint8_t kind, int from);

// Carries out the turn's command as Game::process_turn will once the build
// commands are wired into it: the build tool's first.
void run_build_command(th::Game &game);
