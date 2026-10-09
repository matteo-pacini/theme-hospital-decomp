#include "common.h"

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/render/blit.h"
#include "core/render/frame.h"
#include "core/render/iso.h"
#include "core/render/surface.h"
#include "core/render/view.h"
#include "core/ui/charts.h"
#include "core/ui/queue_window.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/object_tables.h"
#include "core/world/reaper.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace
{

// Draws `n` values from `seed`, as the game does for those it discards.
uint32_t skip_draws(uint32_t seed, int n)
{
    for (int i = 0; i < n; i++)
        th::old_newrand(seed);
    return seed;
}

} // namespace

void test_create_an_illness()
{
    th::Balance b;
    b.gbv.hold_visual_months = 0;
    b.gbv.hold_visual_peep_count = 0;
    b.towns[1].ill_rate = 1;
    b.visuals.fill(0);
    b.visuals[3] = 5;
    b.non_visuals.fill(0);
    b.non_visuals[7] = 1;
    b.non_visuals[9] = 3;
    th::balance_control(b);
    th::ArrivalState s;

    // A visual illness: the ill_rate draw, then the visual one.
    uint32_t seed = 1234;
    CHECK(th::create_an_illness(seed, s, b, 1) == 5);
    CHECK(seed == skip_draws(1234, 2));
    // Not yet available: a non-visual one after both.
    b.visuals_available[3] = 1;
    seed = 1234;
    uint8_t ill = th::create_an_illness(seed, s, b, 1);
    CHECK(ill == 23 || ill == 25);
    CHECK(seed == skip_draws(1234, 3));
    // Held back early in the level: no visual draw.
    b.visuals_available[3] = 0;
    b.gbv.hold_visual_months = 1;
    seed = 1234;
    ill = th::create_an_illness(seed, s, b, 1);
    CHECK((ill == 23 || ill == 25) && seed == skip_draws(1234, 2));
    // No visual odds: only the non-visual draw, by weight 1 : 3.
    b.towns[1].ill_rate = 0;
    seed = 1234;
    int counts[2] = {};
    for (int i = 0; i < 4000; i++) {
        ill = th::create_an_illness(seed, s, b, 1);
        CHECK(ill == 23 || ill == 25);
        counts[ill == 25]++;
    }
    CHECK(seed == skip_draws(1234, 4000));
    CHECK(counts[0] > 850 && counts[0] < 1150);
}

void test_arrivals_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    th::Balance settings;
    CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
    if (!level)
        return;
    // Open from the start, so the rivals take patients too.
    settings.gbv.open_countdown = 0;
    const th::AnimData &anims = gd.anims();
    auto game = std::make_unique<th::Game>();
    for (int run = 0; run < 2; run++) {
        game->start(*level, 1, settings, anims, gd.person_complexes(), gd.staff_complexes());
        game->skipped_calls = th::kSkipComputerAction;
        const th::ArrivalState &state = game->arrivals;
        CHECK(state.visitors_per_month == 3 && state.peep_interval == 500); // popn[0]: month 0, change 3
        int ours = 0;
        bool valid = true;
        while (game->date.month < 6) {
            game->turn();
            for (uint16_t id : game->new_patients) {
                ours++;
                const th::Thing &t = game->world.things[id];
                int cell = t.x.tile + t.y.tile * th::kMapSize;
                const auto &doors = level->entrances[0];
                valid &= t.type == th::kThingPatient && t.owner == 0 && t.subtype < 15;
                // Set going in the turn it arrived: still at its entrance.
                valid &= std::find(doors.begin(), doors.end(), cell) != doors.end();
                valid &= t.sprite < anims.frames.size();
                valid &= t.layers[0] >= 1 && t.layers[0] <= 10 && t.layers[1] < 4 && t.layers[2] < 4 &&
                         t.layers[3] < 6 && t.layers[4] < 6;
                valid &= t.illness >= 2 && t.illness <= 35 && t.known_illness == 1 && t.treatment == 1;
                valid &= t.health >= 200 && t.health <= 1000 && t.cash >= 5000 && t.cash < 100000;
                valid &= t.speed == 0x20 && t.anim_speed == 1 && t.happiness == 100;
            }
        }
        CHECK(valid);
        CHECK(cell_lists_consistent(game->world));
        CHECK(ours == int(state.hospitals[0].visitors));
        std::printf("arrivals, level 1, 6 months: %d of %d patients ours, %d a month now, seed %#x\n", ours,
                    int(state.peeps_arrived), int(state.visitors_per_month), unsigned(game->world.seed));
        // Regression values, not checked against the game.
        CHECK(ours == 14 && state.peeps_arrived == 43 && state.visitors_per_month == 11);
        CHECK(game->world.seed == 0x3755895e);
    }
}

// set_complex starts a complex on an idle person, queues two behind a
// running one (the second slot replaced by a third), and resumes one a step
// of action 24 left, queueing the new one.
void test_set_complex()
{
    auto w = std::make_unique<th::World>();
    th::Thing &p = w->things[1];
    p.type = th::kThingPatient;
    CHECK(th::set_complex(*w, 1, 20, 0));
    CHECK(p.complex_pc == 20 && p.complex_action == 20);
    CHECK(!th::set_complex(*w, 1, 30, 5 + 6 * th::kMapSize));
    CHECK(p.complex_queued == 30 && p.queued_arg_x == 5 && p.queued_arg_y == 6);
    CHECK(!th::set_complex(*w, 1, 40, 0));
    CHECK(!th::set_complex(*w, 1, 50, 7));
    CHECK(p.complex_queued2 == 50 && p.queued2_arg_x == 7 && p.complex_queued == 30);

    th::Thing &q = w->things[2];
    q.type = th::kThingPatient;
    q.complex_resume = 80;
    CHECK(th::set_complex(*w, 2, 60, 9));
    CHECK(q.complex_pc == 80 && q.complex_resume == 0 && q.complex_queued == 60 && q.queued_arg_x == 9);
}

namespace
{

// What run_patient saw: the turns run, whether the map's cell lists stayed
// consistent, and whether an entrance door started opening.
struct PatientRun {
    int turns = 0;
    bool consistent = true;
    bool opened = false;
};

// Runs patient `id` and the level's objects for up to `limit` turns from
// turn `turn` until the patient is gone.
PatientRun run_patient(th::World &w, const th::AnimData &anims, const th::PersonComplexes &cx, uint16_t id,
                       uint32_t &turn, int limit)
{
    PatientRun run;
    bool exists = true;
    while (exists && run.turns < limit) {
        turn++;
        run.turns++;
        for (uint16_t i = 1; i < w.things.size(); i++) {
            if (i == id)
                exists = th::person_turn(w, anims, cx, i, turn);
            else if (w.things[i].type == th::kThingObject)
                th::sliding_door_turn(w, anims, i, turn);
            run.opened |= w.things[i].type == th::kThingObject && w.things[i].state == 0x16;
        }
        run.consistent &= cell_lists_consistent(w);
    }
    return run;
}

// Where a patient starts on level 1, and how its walk off the map goes.
struct LeaveStart {
    int x, y;
    int turns;
    bool opens;
};

// Puts a patient at `s` and sends it off the map through `entrance`,
// checking its walk.
void check_patient_leaves(th::World &w, const th::AnimData &anims, const th::PersonComplexes &cx, const LeaveStart &s,
                          int entrance, uint32_t &turn)
{
    uint16_t id = 3; // after the two parts of the entrance door
    CHECK(w.things[id].type == th::kThingFree);
    th::Thing &p = w.things[id];
    p.type = th::kThingPatient;
    p.speed = 0x20;
    p.anim_speed = 1;
    p.x = {0x80, uint8_t(s.x)};
    p.y = {0x80, uint8_t(s.y)};
    th::insert_node(w, id, p.x, p.y);
    th::play_frame(p, anims, th::kPatientAnims[0].walk);
    th::start_person_going(w, anims, cx, id, turn);
    CHECK(p.state == th::kPsGotoDestination && p.complex_pc == 21);
    CHECK(p.dest_x + p.dest_y * th::kMapSize == entrance);
    const PatientRun run = run_patient(w, anims, cx, id, turn, 5000);
    CHECK(run.consistent);
    CHECK(w.things[id].type == th::kThingFree);
    CHECK(w.map[size_t(entrance)].first == 0);
    CHECK(run.opened == s.opens);
    std::printf("patient from %d,%d left in %d turns, doors %s\n", s.x, s.y, run.turns,
                run.opened ? "opened" : "closed");
    CHECK(run.turns == s.turns);
}

// The doors close again 50 turns after the patient has passed.
void check_doors_close(th::World &w, const th::AnimData &anims, uint32_t &turn)
{
    for (int i = 0; i < 100; i++) {
        turn++;
        th::sliding_door_turn(w, anims, 1, turn);
        th::sliding_door_turn(w, anims, 2, turn);
    }
    for (uint16_t door : {1, 2}) {
        const th::Thing &d = w.things[door];
        CHECK(d.state == 0x14 && d.sprite == anims.starts[th::kObjects[d.subtype].anim_start + d.facing].frame);
    }
}

} // namespace

// With the game data: P_COMP.LEX's complex 20 (walk, then leave the map);
// on level 1, with no reception, start_person_going sends a patient made at
// the entrance cell (where all four of level 1's entrances are) straight
// off the map, and one inside the entrance door out through it, the
// sliding doors opening for it, in a fixed number of turns.
void test_patient_leaves_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    th::PersonComplexes cx;
    CHECK(th::load_person_complexes(dir, cx, &error));
    CHECK(cx.group.size() == 2000);
    CHECK(cx.at(20) == 12 && cx.at(21) == 15 && cx.at(22) == 0);
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    const th::AnimData &anims = gd.anims();
    auto w = std::make_unique<th::World>();
    for (int run = 0; run < 2; run++) {
        th::start_level(*w, *level, anims);
        th::start_entrance_rooms(*w, *level);
        int entrance = level->entrances[0][0];
        // The second walk crosses indoor cells, where the patient's needs and
        // reactions draw from the generator: a regression value.
        const LeaveStart starts[] = {
            {entrance % th::kMapSize, entrance / th::kMapSize, 2, false},
            {64, 76, 314, true},
        };
        uint32_t turn = 1;
        for (const LeaveStart &s : starts)
            check_patient_leaves(*w, anims, cx, s, entrance, turn);
        check_doors_close(*w, anims, turn);
        std::printf("seed %#x\n", unsigned(w->seed));
        CHECK(w->seed == 0xd8255b23);
    }
}

namespace
{

// The free cells of player 0's land indoors nearest level 1's view start,
// at most `count`.
std::vector<int> indoor_cells(const th::World &w, const th::Level &level, size_t count)
{
    std::vector<int> cells;
    int start = level.camera_start[0];
    for (int r = 0; r < 20 && cells.size() < count; r++)
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++) {
                if (std::max(std::abs(dx), std::abs(dy)) != r || cells.size() >= count)
                    continue;
                int c = start + dx + dy * th::kMapSize;
                if (c >= 0 && c < th::kMapCells && w.map[size_t(c)].first == 0 && w.map[size_t(c)].owner_flags == 1)
                    cells.push_back(c);
            }
    return cells;
}

// The view centred on a cell, and how a world draws in it.
struct CellView {
    const th::GameData &gd;
    th::Point camera;
    th::Rect view{0, 0, 640, 480};
    th::Surface clean, drawn, scratch;

    // Draws `world` as it is for the comparison.
    CellView(const th::GameData &data, const th::World &world, int cell) : gd(data)
    {
        th::Point p = th::cell_to_world(cell % th::kMapSize, cell / th::kMapSize);
        camera = {p.x - 320, p.y - 240};
        clean.resize(640, 480);
        drawn.resize(640, 480);
        scratch.resize(640, 480);
        scratch.fill(th::kMaskColour);
        th::draw_world({clean, view, camera}, world, gd, nullptr, scratch);
    }

    // Whether `w` draws otherwise than the world it was made with, leaving
    // the scratch surface clean.
    bool differs(const th::World &w)
    {
        th::draw_world({drawn, view, camera}, w, gd, nullptr, scratch);
        bool scratch_clean =
            std::all_of(scratch.data(), scratch.data() + 640 * 480, [](uint8_t c) { return c == th::kMaskColour; });
        return scratch_clean && !std::equal(drawn.data(), drawn.data() + 640 * 480, clean.data());
    }
};

// Every kind of litter on `cell` draws, also behind a wall; place_litter and
// remove_litter keep a rat hole and replace the rest by priority.
void check_litter_drawn(CellView &v, const th::World &world, int cell)
{
    auto litter = std::make_unique<th::World>(world);
    for (int kind : {1, 2, 3, int(th::kLitterVomit), int(th::kLitterPuddle), 7}) {
        litter->map[size_t(cell)].litter_shadow = uint8_t(kind);
        CHECK(v.differs(*litter));
        litter->map[size_t(cell)].flags |= 0x10;
        CHECK(v.differs(*litter));
        litter->map[size_t(cell)].flags &= uint8_t(~0x10);
    }
    th::place_litter(*litter, uint16_t(cell), 0, 1);
    CHECK((litter->map[size_t(cell)].litter_shadow & 0xf) == 7);
    th::remove_litter(*litter, uint16_t(cell), 0);
    CHECK((litter->map[size_t(cell)].litter_shadow & 0xf) == 7);
    litter->map[size_t(cell)].litter_shadow = 0;
    CHECK(!v.differs(*litter));
    th::place_litter(*litter, uint16_t(cell), 0, 2);
    th::place_litter(*litter, uint16_t(cell), 0, 3);
    CHECK((litter->map[size_t(cell)].litter_shadow & 0xf) == 2);
    th::place_litter(*litter, uint16_t(cell), 0, th::kLitterVomit);
    CHECK((litter->map[size_t(cell)].litter_shadow & 0xf) == th::kLitterVomit);
    th::place_litter(*litter, uint16_t(cell), 1, 1); // not player 1's land
    th::remove_litter(*litter, uint16_t(cell), 0);
    CHECK((litter->map[size_t(cell)].litter_shadow & 0xf) == 0);
}

// Runs `patients`' needs for 5000 turns, moving each to the next of `cells`
// every 50 turns and giving each a drink's litter urge every 400. Returns
// the reactions.
int run_patient_needs(HireFixture &f, const std::vector<uint16_t> &patients, const std::vector<int> &cells)
{
    th::World &w = f.game->world;
    const th::AnimData &anims = f.gd.anims();
    int reactions = 0;
    for (int turn = 1; turn <= 5000; turn++) {
        f.game->turn_count++;
        for (size_t i = 0; i < patients.size(); i++) {
            uint16_t id = patients[i];
            th::Thing &p = w.things[id];
            if (turn % 400 == 0)
                th::add_litter_urge(p, 10);
            if (turn % 50 == 0 && p.state != th::kPsShowanim) {
                int cell = cells[(i * 32 + size_t(turn / 50)) % cells.size()];
                th::move_node(w, id, {0x80, uint8_t(cell % th::kMapSize)}, {0x80, uint8_t(cell / th::kMapSize)});
            }
            if (p.state == th::kPsShowanim)
                th::person_showanim_turn(w, anims, id, f.game->turn_count);
            else if (th::person_needs_turn(w, anims, f.settings, th::kStartHeatingLevel, id, f.game->turn_count))
                reactions++;
        }
    }
    return reactions;
}

} // namespace

// With the game data: litter, vomit and the cell overlays are drawn on an
// empty cell of level 1, also behind a wall, leaving the scratch surface
// clean; a rat hole's cell is not swept clean by remove_litter.
void test_litter_drawn(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    auto world = std::make_unique<th::World>();
    th::start_level(*world, *level, gd.anims());
    std::vector<int> cells = indoor_cells(*world, *level, 1);
    CHECK(cells.size() == 1);
    if (cells.empty())
        return;
    const int cell = cells[0];
    CellView v(gd, *world, cell);
    check_litter_drawn(v, *world, cell);
    auto overlay = std::make_unique<th::World>(*world);
    for (int bits : {0x40, 0x80, 0xc0}) {
        overlay->map[size_t(cell)].flags = uint8_t(bits);
        CHECK(v.differs(*overlay));
        overlay->map[size_t(cell)].flags = uint8_t(bits | 0x10);
        CHECK(v.differs(*overlay));
    }
}

// With the game data: patients indoors on level 1, moved to another cell
// every 50 turns and given a drink's litter urge every 400, their needs run
// every turn (modify_person and do_reaction, as a walking patient's when it
// decides its step): they drop litter, vomit and wet themselves, and their
// reactions leave the mess on the map. Fixed counts and generator from the
// level's start.
void test_litter_dropping_level1(const char *dir)
{
    HireFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    std::vector<int> cells = indoor_cells(w, *f.level, 256);
    CHECK(cells.size() == 256);
    std::vector<uint16_t> patients;
    for (size_t i = 0; i < 8 && i < cells.size(); i++) {
        uint16_t id = f.game->add_walker(cells[i * 32]);
        CHECK(id != 0);
        if (id)
            patients.push_back(id);
    }
    CHECK(patients.size() == 8);
    th::Thing &first = w.things[patients[0]];
    uint16_t urge = first.litter_urge;
    th::add_litter_urge(first, 10);
    CHECK(first.litter_urge == urge + 10);
    const int reactions = run_patient_needs(f, patients, cells);
    int litter = 0, vomit = 0, puddles = 0;
    for (const th::MapCell &c : w.map) {
        uint8_t kind = c.litter_shadow & 0xf;
        litter += kind >= 1 && kind <= 3;
        vomit += kind == th::kLitterVomit;
        puddles += kind == th::kLitterPuddle;
    }
    std::printf("patients' mess: %d litter, %d vomit, %d puddles, %d reactions, seed %#x\n", litter, vomit, puddles,
                reactions, unsigned(w.seed));
    CHECK(litter > 0 && vomit > 0 && puddles > 0);
    // Regression values, not checked against the game.
    CHECK(litter == 63 && vomit == 65 && puddles == 5 && reactions == 216 && w.seed == 0xe3e016bd);
}

// A patient sent home fed up counts once as a case of its illness
// (record_person_as_patient), lowering the illness's and the hospital's
// reputation from 20 points over 20 events to 20 over 21, then 22.
void test_reputation_go_home(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    th::World &w = f.game->world;
    const th::HospitalMarket &hosp = f.game->arrivals.hospitals[0];
    std::vector<int> cells = indoor_cells(w, *f.level, 2);
    CHECK(cells.size() == 2);
    if (cells.size() < 2)
        return;
    constexpr uint8_t illness = 5;
    CHECK(hosp.reputation[illness] == 500 && hosp.hospital_reputation == 500);
    uint16_t first = f.game->add_walker(cells[0]);
    uint16_t second = f.game->add_walker(cells[1]);
    CHECK(first && second);
    if (!first || !second)
        return;
    w.things[first].illness = w.things[second].illness = illness;
    th::person_go_home(w, f.gd.anims(), f.gd.person_complexes(), first, f.game->turn_count);
    CHECK(w.things[first].person_flags & th::kPfRecorded);
    CHECK(hosp.rep_points[illness] == 20 && hosp.rep_events[illness] == 21 && hosp.reputation[illness] == 476);
    CHECK(hosp.hospital_rep_events == 21 && hosp.hospital_reputation == 476);
    th::person_go_home(w, f.gd.anims(), f.gd.person_complexes(), first, f.game->turn_count);
    CHECK(hosp.rep_events[illness] == 21 && hosp.hospital_rep_events == 21);
    th::person_go_home(w, f.gd.anims(), f.gd.person_complexes(), second, f.game->turn_count);
    CHECK(hosp.rep_events[illness] == 22 && hosp.reputation[illness] == 454 && hosp.hospital_reputation == 454);
    CHECK(hosp.reputation[illness + 1] == 500);
}

namespace
{

// What happened to a dying patient: whether it fell dead, was taken (by
// floating away or through the gates of hell), and was removed with any
// reaper and gates.
struct DeathRun {
    bool dead = false;
    bool taken = false;
    bool gone = false;
};

// Runs the patients, any reaper and any gates of hell until dying patient
// `id` is removed with them, at most 5000 turns.
DeathRun run_until_removed(th::World &w, const th::AnimData &anims, const th::PersonComplexes &cx, uint16_t id)
{
    DeathRun run;
    for (uint32_t turn = 2; turn < 5000 && !run.gone; turn++) {
        for (uint16_t i = 1; i < w.things.size(); i++) {
            th::Thing &t = w.things[i];
            if (t.type == th::kThingPatient) {
                run.dead |= t.state == th::kPsDead;
                run.taken |= t.state == th::kPsFlys || t.state == th::kPsAtHellGate;
                th::person_turn(w, anims, cx, i, turn);
            } else if (t.type == th::kThingReaper) {
                th::reaper_turn(w, anims, i, turn);
            } else if (t.type == th::kThingObject && t.state == th::kObjectHell) {
                th::object_hell(w, anims, i, turn);
            }
        }
        run.gone =
            w.things[id].type == th::kThingFree &&
            std::none_of(w.things.begin(), w.things.end(), [](const th::Thing &t) {
                return t.type == th::kThingReaper || (t.type == th::kThingObject && t.subtype == th::kObjGatesOfHell);
            });
    }
    return run;
}

} // namespace

// With the game data, a patient in the lobby whose health runs out
// (change_health) dies (person_dies): counted as a death of its illness and
// of the hospital, it walks to a free cell and falls (PS_DEAD), then floats
// away (PS_FLYS) or the grim reaper takes it into the gates of hell, and is
// removed; any gates are removed too.
void test_patient_dies_level1(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    th::PersonComplexes cx;
    CHECK(th::load_person_complexes(dir, cx, &error));
    auto level = gd.load_level(1, &error);
    CHECK(level.has_value());
    if (!level)
        return;
    const th::AnimData &anims = gd.anims();
    auto w = std::make_unique<th::World>();
    th::start_level(*w, *level, anims);
    th::start_entrance_rooms(*w, *level);
    th::HospitalSettings hospital;
    th::Finance finance;
    w->hospital = &hospital;
    w->ledger = &finance;
    w->person_complexes = &cx;
    w->path_finder = th::PathFinder::Original;
    const uint16_t id = 3;
    th::Thing &p = w->things[id];
    p.type = th::kThingPatient;
    p.illness = 16;
    p.health = 5;
    p.speed = 0x20;
    p.anim_speed = 1;
    p.x = {0x80, 64};
    p.y = {0x80, 72};
    th::insert_node(*w, id, p.x, p.y);
    th::play_frame(p, anims, th::kPatientAnims[0].stand);
    th::change_health(*w, anims, id, -2, 1);
    CHECK(p.health == 3 && finance.deaths == 0);
    th::change_health(*w, anims, id, -10, 1);
    CHECK(p.health == 0 && (p.person_flags & th::kPfDying) && p.complex_action == 0x6e);
    CHECK(finance.deaths == 1 && finance.deaths_this_month == 1 && hospital.illnesses[16].deaths == 1);
    const DeathRun run = run_until_removed(*w, anims, cx, id);
    CHECK(run.dead && run.taken && run.gone);
    // The dead do not change.
    th::Thing corpse;
    corpse.type = th::kThingPatient;
    w->things[id] = corpse;
    th::change_health(*w, anims, id, 100, 1);
    CHECK(w->things[id].health == 0);
}

namespace
{

bool same_surface(const th::Surface &x, const th::Surface &y)
{
    return std::equal(x.data(), x.data() + 640 * 480, y.data());
}

// A level 1 game run until its first patient, who walks, with the hospital
// view drawn centred on them as it is with illness 2: the drawing tests change
// the patient's illness and draw again.
struct DrawnPatient {
    th::GameData gd;
    std::unique_ptr<th::Game> game;
    uint16_t id = 0;
    std::unique_ptr<CellView> view;
    th::Surface plain, scratch;

    // Whether the patient was found and the view made.
    bool start(const char *dir)
    {
        std::string error;
        bool loaded = gd.load(dir, &error);
        auto level = gd.load_level(1, &error);
        th::Balance settings;
        CHECK(loaded && level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
        if (!loaded || !level)
            return false;
        settings.gbv.open_countdown = 0;
        game = std::make_unique<th::Game>();
        game->start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        for (int i = 0; i < 5000 && !id; i++) {
            game->turn();
            if (!game->new_patients.empty())
                id = game->new_patients[0];
        }
        CHECK(id != 0);
        if (!id)
            return false;
        th::Thing &p = patient();
        p.state = th::kPsGotoDestination;
        p.illness = 2;
        view = std::make_unique<CellView>(gd, game->world, p.x.tile + p.y.tile * th::kMapSize);
        plain = view->clean;
        scratch = view->scratch;
        return true;
    }

    th::Thing &patient() { return game->world.things[id]; }

    // The view at game turn `turn`.
    th::Surface draw(uint32_t turn)
    {
        th::Surface out;
        out.resize(640, 480);
        th::draw_world({out, view->view, view->camera, turn}, game->world, gd, nullptr, scratch);
        return out;
    }

    // The view at turn 0 with illness `illness`, `object` the thing used.
    th::Surface draw_with(uint8_t illness, uint16_t object)
    {
        patient().illness = illness;
        patient().object = object;
        return draw(0);
    }

    bool scratch_clean() const
    {
        return std::all_of(scratch.data(), scratch.data() + 640 * 480, [](uint8_t c) { return c == th::kMaskColour; });
    }
};

} // namespace

// A jellyitis patient walking is drawn wobbling (draw_wibble_frame): the
// view differs from the plain drawing of the same patient, differs from one
// turn's phase to the next and leaves the scratch surface clean; a patient
// using an object is drawn plain.
void test_wibble_drawn(const char *dir)
{
    DrawnPatient d;
    if (!d.start(dir))
        return;
    th::Surface a = d.draw_with(12, 0), b = d.draw(8);
    CHECK(!same_surface(a, d.plain) && !same_surface(b, d.plain) && !same_surface(a, b));
    CHECK(d.scratch_clean());
    d.patient().object = 1;
    CHECK(same_surface(d.draw(0), d.draw(8)));
}

// A patient with radiation is drawn glowing (draw_glow_frame): its layered
// elements blend with the glow colour of the turn (glow_path), whether it
// walks or uses an object; the other illnesses are drawn as before.
void test_glow_drawn(const char *dir)
{
    DrawnPatient d;
    if (!d.start(dir))
        return;
    // glow_path (src/draw.cpp): the colour steps every second turn through
    // eight values, from the patient's first two layer choices.
    constexpr uint8_t kGlowPath[8] = {0x4d, 0x4e, 0x4f, 0x50, 0x50, 0x4f, 0x4e, 0x4d};
    const uint8_t *layers = d.patient().layers.data();
    auto expected = [&](uint32_t turn) { return kGlowPath[((turn + layers[0] + layers[1]) >> 1) & 7]; };
    for (uint32_t turn = 0; turn < 20; turn++)
        CHECK(th::glow_colour(turn, layers) == expected(turn));
    // Walking: drawn otherwise than plainly, and alike on the turns of one
    // colour only.
    d.patient().illness = 6;
    std::vector<th::Surface> glowing;
    for (uint32_t turn = 0; turn < 16; turn++)
        glowing.push_back(d.draw(turn));
    for (uint32_t i = 0; i < glowing.size(); i++) {
        CHECK(!same_surface(glowing[i], d.plain));
        for (uint32_t j = 0; j < i; j++)
            CHECK(same_surface(glowing[i], glowing[j]) == (expected(i) == expected(j)));
    }
    CHECK(d.scratch_clean());
    // Using an object glows as well: draw_glow_frame has no object test.
    th::Surface plain_on_object = d.draw_with(2, 1);
    CHECK(!same_surface(d.draw_with(6, 1), plain_on_object));
    // The other illnesses are drawn as the plain patient is.
    for (int illness : {1, 2, 3, 5, 7, 11, 13, 20, 35})
        CHECK(same_surface(d.draw_with(uint8_t(illness), 0), d.plain));
}

namespace
{

// The pixels drawn onto a layer of 640 x 480 by the queue window's `id`.
long queue_pixels(DrawnPatient &d, uint8_t illness, uint32_t turn)
{
    th::UiLayer layer;
    layer.resize(640, 480);
    layer.clear();
    d.patient().illness = illness;
    th::draw_queue_person(layer, d.game->world, d.gd, d.id, {300, 300}, turn);
    std::vector<uint32_t> argb = layer.to_argb(d.gd.palette());
    return std::count_if(argb.begin(), argb.end(), [](uint32_t c) { return (c >> 24) != 0; });
}

// What the queue window draws for `id` with `illness` at `turn`.
std::vector<uint8_t> queue_image(DrawnPatient &d, uint8_t illness, uint32_t turn)
{
    th::UiLayer layer;
    layer.resize(640, 480);
    layer.clear();
    d.patient().illness = illness;
    th::draw_queue_person(layer, d.game->world, d.gd, d.id, {300, 300}, turn);
    return {layer.pixels().data(), layer.pixels().data() + 640 * 480};
}

} // namespace

// The queue window's patients: a patient with radiation glows with the
// colour of the turn, one with jellyitis wobbles with the turn's phase (also
// with no standing frame), the others are drawn as before; the glow draws
// every pixel of the frame whatever colour it blends them to.
void test_queue_person_drawn(const char *dir)
{
    DrawnPatient d;
    if (!d.start(dir))
        return;
    auto plain = queue_image(d, 2, 0);
    CHECK(std::any_of(plain.begin(), plain.end(), [](uint8_t c) { return c != 0; }));
    CHECK(queue_image(d, 2, 5) == plain);
    const uint8_t *layers = d.patient().layers.data();
    auto glow0 = queue_image(d, 6, 0);
    auto wibble0 = queue_image(d, 12, 0);
    CHECK(glow0 != plain && wibble0 != plain);
    for (uint32_t turn = 1; turn < 20; turn++)
        CHECK((queue_image(d, 6, turn) == glow0) == (th::glow_colour(turn, layers) == th::glow_colour(0, layers)));
    CHECK(queue_image(d, 12, 8) != wibble0 && queue_image(d, 12, 17) == wibble0);
    // The same pixels are drawn in every colour of the glow, for every kind
    // of patient and head.
    int drawn_kinds = 0;
    for (uint8_t subtype = 0; subtype < 15; subtype++) {
        for (uint8_t head = 1; head <= 10; head++) {
            d.patient().subtype = subtype;
            d.patient().layers[0] = head;
            const long pixels = queue_pixels(d, 6, 0);
            drawn_kinds += pixels > 100;
            for (uint32_t turn = 2; turn < 16; turn += 2)
                CHECK(queue_pixels(d, 6, turn) == pixels);
        }
    }
    CHECK(drawn_kinds > 100);
}

namespace
{

// A sprite containing pixels of colour `pixel` (4 or 174), drawn plainly in a
// surface of the sprite's size plus a border of 1.
struct GlowSprite {
    th::Sprite sprite;
    th::Surface plain;
    int with_colour = 0;
};

// The first sprite from index `from` of `gd` that has pixels of colour `pixel`.
bool find_glow_sprite(const th::GameData &gd, uint8_t pixel, size_t &from, GlowSprite &out)
{
    for (; from < gd.sprites().size(); from++) {
        out.sprite = gd.sprites().at(from);
        out.plain.resize(out.sprite.width + 2, out.sprite.height + 2);
        out.plain.fill(th::kMaskColour);
        th::draw_sprite(out.plain, 1, 1, out.sprite);
        out.with_colour = 0;
        for (int y = 0; y < out.plain.height(); y++)
            for (int x = 0; x < out.plain.width(); x++)
                out.with_colour += out.plain.row(y)[x] == pixel;
        if (out.with_colour) {
            from++;
            return true;
        }
    }
    return false;
}

// Draws `g` blended with glow colour 0 (kDrawGhostUnder) onto colours 0 and
// 0x77: the same pixels are drawn on both, each pixel blends to ghost[pixel]
// of the row 0, and those of colour `pixel` come out as the mask colour.
void check_glow_sprite(GlowSprite &g, const uint8_t *ghost, uint8_t pixel)
{
    th::Surface on_zero, on_other;
    for (auto [surface, base] : {std::pair{&on_zero, uint8_t(0x00)}, std::pair{&on_other, uint8_t(0x77)}}) {
        surface->resize(g.plain.width(), g.plain.height());
        surface->fill(base);
        th::draw_sprite_glow(*surface, {1, 1}, g.sprite, th::kDrawGhostUnder, 0, ghost);
    }
    int masked = 0;
    for (int y = 0; y < g.plain.height(); y++) {
        for (int x = 0; x < g.plain.width(); x++) {
            const uint8_t src = g.plain.row(y)[x], a = on_zero.row(y)[x], c = on_other.row(y)[x];
            CHECK((src != th::kMaskColour) == (a != 0 || c != 0x77));
            if (src == th::kMaskColour)
                continue;
            CHECK(a == ghost[0 << 8 | src] && c == a);
            masked += src == pixel && a == th::kMaskColour;
        }
    }
    CHECK(masked == g.with_colour);
}

} // namespace

// draw_sprite_glow blends each pixel with the glow colour alone, as
// sprite_draw_glow does: kDrawGhost takes ghost[pixel << 8 | glow],
// kDrawGhostUnder ghost[glow << 8 | pixel], and what is under it never
// matters, though some of the blends are 0xfe, the mask colour (with colour 0,
// the blend of an untagged element: ghost[0 << 8 | 4] and ghost[0 << 8 | 174]).
void test_sprite_glow(const char *dir)
{
    th::GameData gd;
    std::string error;
    CHECK(gd.load(dir, &error));
    const uint8_t *ghost = gd.ghost().data();
    int tested = 0;
    for (uint8_t pixel : {uint8_t(4), uint8_t(174)}) {
        CHECK(ghost[0 << 8 | pixel] == th::kMaskColour);
        size_t from = 0;
        GlowSprite g;
        for (int n = 0; n < 2 && find_glow_sprite(gd, pixel, from, g); n++, tested++)
            check_glow_sprite(g, ghost, pixel);
    }
    CHECK(tested > 0);
}
