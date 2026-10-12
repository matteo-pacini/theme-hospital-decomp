// Shared by the rooms_*.cpp tests: the scripted game runner, its GP's office
// script, the event counter and the queue check several of them use.

#pragma once

#include "core/data/balance.h"
#include "core/data/gamedata.h"
#include "core/render/shadows.h"
#include "core/ui/charts.h"
#include "core/world/advisor.h"
#include "core/world/game.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// The player's commands of tools/replay/scripts/gp_office.txt (level 1): a
// reception desk and receptionist, a GP's office and its doctor.
constexpr const char *kGpOfficeScript = "5 98 0 0\n6 37 11 1\n7 43 0 0\n8 7 9534 11\n10 48 3 22\n11 5 9788 22\n"
                                        "101 55 0 0\n102 53 7 7\n103 2 8382 0\n104 2 8898 0\n105 8 8898 0\n"
                                        "106 18 0 0\n107 26 8256 0\n108 3 0 0\n109 4 7 0\n110 7 8640 1\n"
                                        "111 7 8386 2\n112 7 8641 6\n113 1 0 0\n114 48 1 11\n115 5 10044 11\n";

// A level started as th_replay starts it, played with a replay script's
// commands (lines "turn type value arg").
struct ScriptedGame {
    th::GameData gd;
    std::unique_ptr<th::Game> game;
    std::vector<std::pair<uint32_t, std::string>> lines;
    size_t next = 0;

    // `vip` true keeps the level's MayorLaunch, so the visit draws at day 150
    // as the replay check's scripts do; the default turns it off, which moves
    // every later seed (the replay check's vip_*.txt cover the VIPs).
    bool start(const char *dir, int level_number, const std::string &script, bool vip = false)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        auto level = gd.load_level(level_number, &error);
        th::Balance settings;
        if (!level || !th::load_balance(dir, level_number, th::kDefaultBalanceId, settings, &error))
            return false;
        if (!vip)
            settings.gbv.mayor_launch = 0xffff;
        game = std::make_unique<th::Game>();
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, level_number, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        game->skipped_calls = th::kSkipComputerAction;
        th::add_static_shadows(game->world.map);
        th::start_advisor(game->advisor, false);
        th::start_hospital_settings(game->hospital, game->settings());
        th::setup_insurances(game->world, 0);
        game->world.path_finder = th::PathFinder::Original;
        size_t at = 0;
        while (at < script.size()) {
            size_t end = script.find('\n', at);
            std::string line = script.substr(at, end - at);
            at = end == std::string::npos ? script.size() : end + 1;
            size_t space = line.find(' ');
            lines.emplace_back(uint32_t(std::stoul(line.substr(0, space))), line.substr(space + 1));
        }
        return true;
    }

    // Runs the turns up to `turn`, calling `each` after every one.
    bool run_to(uint32_t turn, const std::function<void()> &each)
    {
        while (game->turn_count < turn) {
            for (; next < lines.size() && lines[next].first <= game->turn_count + 1; next++)
                if (lines[next].first == game->turn_count + 1 && !game->replay_input(lines[next].second))
                    return false;
            game->turn();
            each();
            game->events.events.clear();
        }
        return true;
    }
};

// Counts the events of each kind (their first word) a run records.
struct EventCounts {
    std::vector<std::pair<std::string, int>> counts;

    void add(const th::Game &g)
    {
        for (const th::TraceEvent &e : g.events.events) {
            std::string kind = e.text.substr(0, e.text.find(' '));
            auto it = std::find_if(counts.begin(), counts.end(), [&](const auto &c) { return c.first == kind; });
            if (it == counts.end())
                counts.emplace_back(kind, 1);
            else
                it->second++;
        }
    }

    int operator[](const std::string &kind) const
    {
        auto it = std::find_if(counts.begin(), counts.end(), [&](const auto &c) { return c.first == kind; });
        return it == counts.end() ? 0 : it->second;
    }
};

// Whether every queue is well linked (rooms.cpp).
bool queues_consistent(const th::World &w);
