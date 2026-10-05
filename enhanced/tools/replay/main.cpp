// th_replay: runs a level headless from its start, with only scripted input,
// and writes the replay check's trace (core/world/trace.h) of every turn, to
// compare (compare.py) with the instrumented original's (orig_trace.sh).
//
// Usage: th_replay [--data DIR] [--level N] [--turns N] [--script FILE] [--astar] [--out FILE]
//   --data    the game data folder (default: the development folder)
//   --level   the level to start (default 1)
//   --turns   the last turn to run, as the game counts them from 1 (default 2000)
//   --script  the player's commands: lines "turn type value arg" (decimal; the
//             game's EV_* numbers), each carried out between turn - 1 and turn;
//             other lines are ignored
//   --astar   plan routes with A* rather than the game's wall following
//   --out     where to write the trace (default: standard output)

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/ui/charts.h"
#include "core/world/game.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace
{

int usage()
{
    std::fprintf(stderr,
                 "usage: th_replay [--data DIR] [--level N] [--turns N] [--script FILE] [--astar] [--out FILE]\n");
    return 2;
}

struct ScriptedEvent {
    uint32_t turn = 0;
    th::Event event;
};

bool read_script(const char *path, std::vector<ScriptedEvent> &script)
{
    std::ifstream in(path);
    if (!in)
        return false;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream fields(line);
        uint32_t turn, type, value, arg;
        if (fields >> turn >> type >> value >> arg)
            script.push_back({turn, {uint8_t(type), uint16_t(value), 0, uint16_t(arg)}});
    }
    return true;
}

} // namespace

int main(int argc, char **argv)
{
#ifdef TH_DEV_DATA_DIR
    std::string data_dir = TH_DEV_DATA_DIR;
#else
    std::string data_dir = ".";
#endif
    int level_number = 1;
    long turns = 2000;
    bool astar = false;
    const char *out_path = nullptr;
    const char *script_path = nullptr;
    for (int i = 1; i < argc; i++) {
        bool has_value = i + 1 < argc;
        if (!std::strcmp(argv[i], "--data") && has_value)
            data_dir = argv[++i];
        else if (!std::strcmp(argv[i], "--level") && has_value)
            level_number = std::atoi(argv[++i]);
        else if (!std::strcmp(argv[i], "--turns") && has_value)
            turns = std::atol(argv[++i]);
        else if (!std::strcmp(argv[i], "--script") && has_value)
            script_path = argv[++i];
        else if (!std::strcmp(argv[i], "--out") && has_value)
            out_path = argv[++i];
        else if (!std::strcmp(argv[i], "--astar"))
            astar = true;
        else
            return usage();
    }

    std::vector<ScriptedEvent> script;
    if (script_path && !read_script(script_path, script)) {
        std::perror(script_path);
        return 1;
    }

    th::GameData data;
    std::string error;
    if (!data.load(data_dir, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    auto level = data.load_level(level_number, &error);
    th::Balance settings;
    if (!level || !th::load_balance(data.dir(), level_number, th::kDefaultBalanceId, settings, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    // As the app's begin_level and load_level: the game's start with no
    // tutorial; the insurers' clock draw does not touch the seed.
    auto game = std::make_unique<th::Game>();
    game->staff_market.ref_counts = th::reference_counts(data.text());
    game->start(*level, level_number, settings, data.anims(), data.person_complexes(), data.staff_complexes());
    th::start_advisor(game->advisor, false);
    th::start_hospital_settings(game->hospital, game->settings(), 0);
    game->world.path_finder = astar ? th::PathFinder::AStar : th::PathFinder::Original;

    std::FILE *out = out_path ? std::fopen(out_path, "w") : stdout;
    if (!out) {
        std::perror(out_path);
        return 1;
    }
    // The original's trace starts with the state as the level starts, then
    // one entry at the end of each turn; every frame is a turn, as it runs
    // with no frame limit.
    std::fputs(th::trace_turn(*game).c_str(), out);
    size_t next = 0;
    while (game->turn_count < uint32_t(turns)) {
        for (; next < script.size() && script[next].turn <= game->turn_count + 1; next++) {
            if (script[next].turn == game->turn_count + 1) {
                game->event = script[next].event;
                game->process_turn();
            }
        }
        game->turn();
        std::fputs(th::trace_turn(*game).c_str(), out);
    }
    if (out != stdout)
        std::fclose(out);
    return 0;
}
