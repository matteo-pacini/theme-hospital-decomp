#include "common.h"

#include "core/data/balance.h"
#include "core/data/level.h"
#include "core/render/shadows.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/layer.h"
#include "core/world/advisor.h"
#include "core/world/debug_dump.h"
#include "core/world/events.h"
#include "core/world/savegame.h"
#include "core/world/snapshot.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// With the game data: a snapshot of level 1 with a manned reception desk and
// a GP's office with its doctor, saved 1500 turns in and loaded into a fresh
// game, runs the next 3000 turns as the game it was taken from: the same
// trace every turn, the same state and command log at the end.
void test_snapshot_round_trip(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    start_rooms(f);
    th::Game &g = *f.game;
    const int start = f.level->camera_start[0];
    uint16_t recep = hire_near(f, th::kStaffReceptionist, start);
    uint16_t desk = f.place_near(start + 3, 11);
    uint16_t gp = lay_out_gp_office(f, start - 10 * th::kMapSize);
    CHECK(recep && desk && gp);
    if (!recep || !desk || !gp)
        return;
    CHECK(finish_gp_office(f, gp) && hire_near(f, th::kStaffDoctor, start + 2 * th::kMapSize));
    for (int i = 0; i < 1500; i++)
        g.turn();

    std::vector<uint8_t> bytes;
    CHECK(th::save_snapshot(g, bytes));
    auto copy = std::make_unique<th::Game>();
    std::string error;
    CHECK(th::load_snapshot(*copy, bytes, f.gd, &error));
    if (!error.empty())
        std::fprintf(stderr, "%s\n", error.c_str());
    CHECK(copy->turn_count == g.turn_count && copy->level_number() == 1);
    int differs = -1, patients = 0;
    for (int i = 0; i < 3000; i++) {
        g.turn();
        copy->turn();
        if (differs < 0 && th::trace_turn(g, &g.events) != th::trace_turn(*copy, &copy->events))
            differs = i;
        g.events.events.clear();
        copy->events.events.clear();
    }
    for (const th::Thing &t : g.world.things)
        patients += t.type == th::kThingPatient;
    std::printf("snapshot: %zu bytes, the copy agrees for 3000 turns (first difference %d), %d patients\n",
                bytes.size(), differs, patients);
    CHECK(differs < 0 && patients > 0);
    CHECK(th::dump_game(g, start) == th::dump_game(*copy, start));
    CHECK(g.command_log == copy->command_log);
}

// A snapshot of another build, cut short or of something else is refused,
// saying why.
void test_snapshot_refused(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    std::vector<uint8_t> bytes;
    CHECK(th::save_snapshot(*f.game, bytes) && bytes.size() > 16);
    th::Game fresh;
    CHECK(!th::save_snapshot(fresh, bytes) || bytes.empty());
    CHECK(th::save_snapshot(*f.game, bytes));
    std::string error;

    std::vector<uint8_t> other = bytes;
    other[8] ^= 1; // the build id
    CHECK(!th::load_snapshot(fresh, other, f.gd, &error) && error.find("build") != std::string::npos);
    other = bytes;
    other[0] = 'X';
    CHECK(!th::load_snapshot(fresh, other, f.gd, &error) && error == "not a snapshot");
    other.assign(bytes.begin(), bytes.begin() + long(bytes.size() / 2));
    CHECK(!th::load_snapshot(fresh, other, f.gd, &error) && error.find("cut short") != std::string::npos);
    CHECK(th::load_snapshot(fresh, bytes, f.gd, &error));
}

namespace
{

// A save slot that does not exist is refused: no file, no level number and
// load_snapshot says so; removing a save makes it missing.
void check_missing_save(const std::filesystem::path &path, const QueueFixture &f)
{
    std::filesystem::path missing = path.parent_path() / "NOPE.h3";
    CHECK(!th::save_slot_exists(missing) && th::save_slot_level(missing) == -1);
    th::Game fresh;
    CHECK(!th::read_save_slot(missing, fresh, f.gd));
    std::error_code ec;
    std::filesystem::remove(path, ec);
    CHECK(!th::save_slot_exists(path));
}

} // namespace

// With the game data: a game saved to a slot file at turn 1500 and loaded
// from it runs the next 2000 turns as the game it was saved from, the same
// trace, state and command log. The slot file's existence and level number
// are the original's save_game_valid/read_level_number.
void test_save_slot_round_trip(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    start_rooms(f);
    th::Game &g = *f.game;
    const int start = f.level->camera_start[0];
    uint16_t recep = hire_near(f, th::kStaffReceptionist, start);
    uint16_t desk = f.place_near(start + 3, 11);
    CHECK(recep && desk);
    if (!recep || !desk)
        return;
    for (int i = 0; i < 1500; i++)
        g.turn();

    std::filesystem::path path = std::filesystem::path("build") / "th-tests-save" / "TESTER.h3";
    std::string error;
    CHECK(th::write_save_slot(path, g, th::SaveInfo{0, "FULL", false, 12, 34}, &error));
    CHECK(error.empty() && th::save_slot_exists(path) && th::save_slot_compatible(path));
    CHECK(th::save_slot_level(path) == 1);

    auto copy = std::make_unique<th::Game>();
    th::SaveInfo info;
    CHECK(th::read_save_slot(path, *copy, f.gd, &info, &error));
    CHECK(info.level == 1 && info.balance_id == "FULL" && info.view_centre_x == 12 && info.view_centre_y == 34);
    CHECK(copy->turn_count == g.turn_count);
    int differs = -1, patients = 0;
    for (int i = 0; i < 2000; i++) {
        g.turn();
        copy->turn();
        if (differs < 0 && th::trace_turn(g, &g.events) != th::trace_turn(*copy, &copy->events))
            differs = i;
        g.events.events.clear();
        copy->events.events.clear();
    }
    for (const th::Thing &t : g.world.things)
        patients += t.type == th::kThingPatient;
    std::printf("save slot: %lld bytes, the copy agrees for 2000 turns (first difference %d), %d patients\n",
                static_cast<long long>(std::filesystem::file_size(path)), differs, patients);
    CHECK(differs < 0 && patients > 0);
    CHECK(th::dump_game(g, start) == th::dump_game(*copy, start));

    check_missing_save(path, f);
}

namespace
{

// Reads and rewrites a file's bytes, so a test can damage a save.
std::vector<uint8_t> read_bytes(const std::filesystem::path &path)
{
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void write_bytes(const std::filesystem::path &path, const std::vector<uint8_t> &bytes)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(bytes.data()), std::streamsize(bytes.size()));
}

} // namespace

// With the game data: a save whose magic, format, layout or payload is wrong
// is refused, saying why, and has no level number; the save itself loads.
void test_save_slot_refused(const char *dir)
{
    QueueFixture f;
    CHECK(f.start(dir));
    if (!f.level)
        return;
    std::filesystem::path path = std::filesystem::path("build") / "th-tests-save" / "REFUSED.h1";
    std::string error;
    CHECK(th::write_save_slot(path, *f.game, th::SaveInfo{0, "FULL", false, 0, 0}, &error));
    std::vector<uint8_t> bytes = read_bytes(path);
    CHECK(bytes.size() > 64);

    // magic 0, kSaveFormat 8, layout hash 12, the payload's start past the
    // header (magic, format, layout, level, name "FULL", tutorial, camera,
    // payload length).
    const size_t payload_at = 8 + 4 + 8 + 4 + 4 + 4 + 1 + 8 + 4;
    auto damaged = [&](size_t at, const char *want, bool header) {
        std::vector<uint8_t> b = bytes;
        b[at] ^= 1;
        write_bytes(path, b);
        th::Game g;
        CHECK(!th::read_save_slot(path, g, f.gd, nullptr, &error));
        CHECK(error.find(want) != std::string::npos);
        // A damaged header is not a save at all; a damaged payload still has
        // the header's level, though loading it fails.
        CHECK((th::save_slot_level(path) == -1 && !th::save_slot_compatible(path)) == header);
    };
    damaged(0, "not a save", true);
    damaged(8, "format", true);
    damaged(12, "layout", true);
    damaged(payload_at, "not a snapshot", false); // the payload's own magic

    write_bytes(path, bytes);
    th::Game g;
    CHECK(th::read_save_slot(path, g, f.gd, nullptr, &error));
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

namespace
{

// Level 1 as the app and th_replay start it.
struct ReplayFixture {
    th::GameData gd;
    std::optional<th::Level> level;
    th::Balance settings;

    bool load(const char *dir)
    {
        std::string error;
        CHECK(gd.load(dir, &error));
        level = gd.load_level(1, &error);
        CHECK(level && th::load_balance(dir, 1, th::kDefaultBalanceId, settings, &error));
        return level.has_value();
    }

    // Starts `g` with the insurers picked by `clock`.
    void start_game(th::Game &g, uint32_t clock) const
    {
        g.staff_market.ref_counts = th::reference_counts(gd.text());
        g.start(*level, 1, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        th::add_static_shadows(g.world.map);
        th::start_advisor(g.advisor, false);
        th::start_hospital_settings(g.hospital, g.settings());
        th::setup_insurances(g.world, clock);
    }
};

// A game played as the app plays it, keeping every turn's trace.
struct AppPlay {
    th::Game &g;
    std::vector<std::string> traces;

    // A frame: `turns` turns, then `ui` and the frame's command.
    void frame(int turns, const std::function<void()> &ui)
    {
        for (int i = 0; i < turns && !g.paused; i++) {
            g.turn();
            traces.push_back(th::trace_turn(g, &g.events));
            g.events.events.clear();
        }
        ui();
        g.process_turn();
    }
};

// Furnishes the corridor with a reception desk near `start` through the buy
// objects requester and the pointer, turning it once.
void furnish_desk(AppPlay &app, const th::GameData &gd, int start)
{
    th::Game &g = app.g;
    th::ObjectPlacing &p = g.world.placing;
    th::ObjectUi oui;
    th::init_object_ui(oui, gd.palette());
    const int desk = 11;

    app.frame(3, [&] { th::furnish_corridor(g); });
    app.frame(1, [&] { p.item = desk; }); // the pointer over its entry
    app.frame(1, [&] { g.event = {th::EV_BUY_OBJECT, uint16_t(desk), 0, 1}; });
    app.frame(1, [&] { g.event = {th::EV_PLACE_NEXT_OBJECT}; });
    CHECK(p.placing && p.kind == desk);
    th::UiMouse turn_it;
    turn_it.right_pressed = turn_it.right_held = true;
    app.frame(1, [&] { th::placement_mouse(oui, g, start, turn_it, true); });
    int cell = start;
    while (!p.ok && ++cell < start + 4 * th::kMapSize)
        app.frame(0, [&] { th::placement_mouse(oui, g, cell, {}, true); });
    CHECK(p.ok);
    th::UiMouse click;
    click.left_pressed = click.left_held = true;
    app.frame(1, [&] { th::placement_mouse(oui, g, cell, click, true); });
    CHECK(!p.placing && g.world.first_of_kind[desk] != 0);
}

// Pauses, sets the heating, switches the route planner, sends the path
// finding tool's walker and hires a receptionist near `start`, then plays on.
void pause_walk_and_hire(AppPlay &app, int start)
{
    th::Game &g = app.g;
    app.frame(1, [&] { g.paused = true; });
    app.frame(2, [&] { g.event = {th::EV_SET_HEATING, 7}; });
    g.paused = false;
    g.world.path_finder = th::PathFinder::Original; // the tools window, after the frame
    uint16_t walker = g.add_walker(start + 2);
    app.frame(20, [] {});
    g.send_walker(walker, start + 6 * th::kMapSize);
    uint16_t applicant = 0;
    for (uint16_t i = 1; i < g.world.things.size() && !applicant; i++)
        if (g.world.things[i].type == th::kThingStaff && g.world.things[i].subtype == th::kStaffReceptionist &&
            g.world.things[i].state == th::kStaffApplicant)
            applicant = i;
    CHECK(walker && applicant);
    app.frame(2, [&] { g.event = {th::EV_MARK_EMPLOYING, 0, 0, applicant}; });
    int at = start + 2 * th::kMapSize;
    while (at < start + 10 * th::kMapSize &&
           (g.world.map[size_t(at)].first || !th::staff_place_ok(g.world, at, th::kStaffReceptionist)))
        at++;
    app.frame(1, [&] { g.event = {th::EV_PLACE_STAFF, uint16_t(at), 0, applicant}; });
    for (int i = 0; i < 1500; i++)
        app.frame(2, [] {});
}

// th_replay's loop: runs `r` from the level's start on `g`'s command log up
// to `g`'s turn. Returns every turn's trace; `understood` is whether every
// line was.
std::vector<std::string> replay_log(th::Game &r, const th::Game &g, bool &understood)
{
    std::string error;
    std::vector<std::string> replayed;
    size_t next = 0;
    understood = true;
    while (r.turn_count < g.turn_count && understood) {
        for (; next < g.command_log.size(); next++) {
            const std::string &line = g.command_log[next];
            if (line.starts_with("#"))
                continue;
            size_t space = line.find(' ');
            if (uint32_t(std::stoul(line.substr(0, space))) != r.turn_count + 1)
                break;
            understood &= r.replay_input(std::string_view(line).substr(space + 1), &error);
        }
        CHECK(!r.paused);
        r.turn();
        replayed.push_back(th::trace_turn(r, &r.events));
        r.events.events.clear();
    }
    if (!understood)
        std::fprintf(stderr, "%s\n", error.c_str());
    return replayed;
}

// A replay script line: what Game::replay_input carries out before `turn`
// (th_replay's ScriptLine), read from the script file.
struct ScriptLine {
    uint32_t turn = 0;
    std::string input;
};

std::vector<ScriptLine> read_replay_script(const std::filesystem::path &path)
{
    std::vector<ScriptLine> script;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream fields(line);
        uint32_t turn = 0;
        std::string first;
        if (!(fields >> turn >> first))
            continue;
        if (first[0] == '@' || std::isdigit(static_cast<unsigned char>(first[0]))) {
            size_t turn_end = line.find_first_not_of("0123456789", line.find_first_not_of(" \t"));
            script.push_back({turn, line.substr(line.find(first, turn_end))});
        }
    }
    return script;
}

// Carries out the script's commands due before turn `turn` on `game`, leaving
// `next` after them.
bool apply_due(th::Game &game, const std::vector<ScriptLine> &script, size_t &next, uint32_t turn)
{
    std::string error;
    for (; next < script.size() && script[next].turn <= turn; next++) {
        if (script[next].turn != turn)
            continue;
        if (!game.replay_input(script[next].input, &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return false;
        }
    }
    return true;
}

// The commands due before `turn`, without advancing `next` (for two games
// that must both get them).
std::vector<std::string> due_commands(const std::vector<ScriptLine> &script, size_t &next, uint32_t turn)
{
    std::vector<std::string> due;
    for (; next < script.size() && script[next].turn <= turn; next++)
        if (script[next].turn == turn)
            due.push_back(script[next].input);
    return due;
}

} // namespace

// Runs `g` to `save_turn` with the script's commands, tracing each turn's
// random draws (events.rand). False when a command or the pause stopped it.
bool run_to_save(th::Game &g, const std::vector<ScriptLine> &script, size_t &next, uint32_t save_turn)
{
    while (g.turn_count < save_turn && !g.progress.winlose_result) {
        if (!apply_due(g, script, next, g.turn_count + 1) || g.paused)
            return false;
        g.turn();
        g.events.events.clear();
    }
    return true;
}

// Continues `g` and `copy` in lockstep, giving both the script's commands,
// to `last_turn`; the first turn their traces differ, or -1.
int compare_after_load(th::Game &g, th::Game &copy, const std::vector<ScriptLine> &script, size_t &next,
                       uint32_t last_turn)
{
    int differs = -1;
    std::string error;
    while (g.turn_count < last_turn && !g.progress.winlose_result && !copy.progress.winlose_result) {
        std::vector<std::string> due = due_commands(script, next, g.turn_count + 1);
        for (const std::string &in : due)
            if (!g.replay_input(in, &error) || !copy.replay_input(in, &error))
                return int(g.turn_count);
        // A pause is a failure, not agreement: neither game may stop early.
        if (g.paused || copy.paused)
            return int(g.turn_count);
        g.turn();
        copy.turn();
        if (differs < 0 && th::trace_turn(g, &g.events) != th::trace_turn(copy, &copy.events))
            differs = int(g.turn_count);
        g.events.events.clear();
        copy.events.events.clear();
    }
    return differs;
}

// A replay script run with every random draw traced, saved into a slot at
// save_turn and loaded from it; the loaded game continues to last_turn with
// the same trace and the same state as the one never stopped. The slot file
// is under the build folder. The first differing turn, or -1.
int save_script_round_trip(const char *dir, const std::filesystem::path &path, uint32_t save_turn, uint32_t last_turn)
{
    std::vector<ScriptLine> script = read_replay_script(path);
    ReplayFixture f;
    if (script.empty() || !f.load(dir))
        return int(save_turn);
    auto game = std::make_unique<th::Game>();
    f.start_game(*game, 0);
    game->events.limit = 0;
    game->events.rand = true;
    game->events.events.clear();
    size_t next = 0;
    std::string error;
    std::filesystem::path slot = std::filesystem::path("build") / "th-tests-save" / "roll.h1";
    if (!run_to_save(*game, script, next, save_turn) ||
        !th::write_save_slot(slot, *game, th::SaveInfo{0, "FULL", false, 0, 0}, &error))
        return int(save_turn);
    auto copy = std::make_unique<th::Game>();
    th::SaveInfo info;
    if (!th::read_save_slot(slot, *copy, f.gd, &info, &error))
        return int(save_turn);
    CHECK(copy->events.rand && copy->turn_count == game->turn_count);
    int differs = compare_after_load(*game, *copy, script, next, last_turn);
    int focus = f.level ? f.level->camera_start[0] : 0;
    CHECK(th::dump_game(*game, focus) == th::dump_game(*copy, focus));
    std::error_code ec;
    std::filesystem::remove(slot, ec);
    return differs;
}

// With the game data: three replay scripts (reception, gp_office,
// general_diag; level 1, their commands) each saved halfway and loaded, the
// two halves traced turn by turn with every random draw.
void test_save_slot_scripts(const char *dir)
{
    const char *names[] = {"reception", "gp_office", "general_diag"};
    int checked = 0;
    for (const char *name : names) {
        std::filesystem::path path =
            std::filesystem::path("enhanced/tools/replay/scripts") / (std::string(name) + ".txt");
        int differs = save_script_round_trip(dir, path, 1500, 3000);
        std::printf("save/load script %s: %s, first difference %d\n", name, differs < 0 ? "agrees" : "DIFFERS",
                    differs);
        CHECK(differs < 0);
        checked++;
    }
    CHECK(checked == 3);
}

// With the game data: level 1 played as the app plays it, frame by frame
// (turns, then the interface, then the frame's command): furnishing the
// corridor through the buy objects requester and the pointer, pausing,
// switching the route planner, the path finding tool's walker, hiring a
// receptionist and the insurers picked by another clock. Its command log
// run from the level's start as th_replay runs a script gives the same
// trace every turn, the same state and the same command log.
void test_command_log_replay(const char *dir)
{
    ReplayFixture f;
    if (!f.load(dir))
        return;
    auto app = std::make_unique<th::Game>();
    f.start_game(*app, 12345);
    th::Game &g = *app;
    // Companies 2, 6 and 5: 12345 % 8, then % 7 and % 6 of those left.
    CHECK((g.world.ins_control == std::array<uint16_t, 5>{2, 6, 5, 0, 0}));
    AppPlay play{g, {}};
    const int start = f.level->camera_start[0];
    furnish_desk(play, f.gd, start);
    pause_walk_and_hire(play, start);
    const std::vector<std::string> &traces = play.traces;

    auto replay = std::make_unique<th::Game>();
    f.start_game(*replay, 0);
    th::Game &r = *replay;
    r.world.path_finder = th::PathFinder::AStar;
    bool understood = true;
    std::vector<std::string> replayed = replay_log(r, g, understood);
    size_t differs = 0;
    while (differs < traces.size() && differs < replayed.size() && traces[differs] == replayed[differs])
        differs++;
    std::printf("command log: %zu lines, %zu turns replayed, first difference at %zu\n", g.command_log.size(),
                replayed.size(), differs);
    CHECK(understood && replayed.size() == traces.size() && differs == traces.size());
    CHECK(r.world.ins_control == g.world.ins_control && r.world.path_finder == g.world.path_finder);
    CHECK(th::dump_game(g, start) == th::dump_game(r, start));
    CHECK(g.command_log == r.command_log);
}
