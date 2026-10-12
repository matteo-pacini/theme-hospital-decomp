// The info windows run, get_nearest_room_info, and the staff room and toilets
// runs, played with the commands of a replay script.
#include "common.h"
#include "rooms_internal.h"

#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/town_map.h"
#include "core/world/advisor.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/disaster.h"
#include "core/world/finance.h"
#include "core/world/objects.h"
#include "core/world/quake.h"
#include "core/world/queue.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace
{

// The trace events and per-turn states the info_windows.txt scenario checks.
struct InfoWindowRun {
    std::vector<std::string> seen;
    bool sent_home = false, guessed = false, priorities = false, home6 = false, home7 = false;
    bool queue47 = false, walking9 = false;

    void add(const th::Game &g)
    {
        for (const th::TraceEvent &e : g.events.events)
            if (e.text.starts_with("handyman_values ") || e.text.starts_with("force_cure ") ||
                e.text.starts_with("go_home "))
                seen.push_back(e.text);
        const th::World &w = g.world;
        if (g.turn_count == 7350)
            sent_home = (w.things[10].person_flags & th::kPfSentHome) && w.things[10].treatment == 0;
        if (g.turn_count == 7419)
            queue47 = w.things[47].state == th::kPsGotoQueue;
        if (g.turn_count == 7420)
            guessed = (w.things[47].person_flags & th::kPfGuessed) && w.things[47].known_illness == 2;
        if (g.turn_count == 12550)
            home6 = (w.things[6].person_flags & th::kPfSentHome) && w.things[6].treatment == 0;
        if (g.turn_count == 12559)
            walking9 = w.things[9].state == th::kPsGotoQueue;
        if (g.turn_count == 12700)
            home7 = (w.things[7].person_flags & th::kPfSentHome) && w.things[7].treatment == 0;
        if (g.turn_count == 9501) {
            const th::Thing &h = w.things[20];
            priorities = h.prio_litter == 6 && h.prio_plants == 6 && h.prio_repair == 88;
        }
    }

    bool has(const std::string &text) const { return std::find(seen.begin(), seen.end(), text) != seen.end(); }
};

} // namespace

// tools/replay/scripts/info_windows.txt: treatment.txt with a handyman whose
// priorities are changed, patients sent home (EV_PERSON_GO_HOME) and made to
// guess a cure (EV_FORCE_CURE): waiting for a decision, walking to a queue,
// with their doctor and watching the pharmacy cabinet energise; ending on the
// original's seed at turn 18000. The level's VIP visit is kept, so the run
// follows the replay script.
void test_info_windows_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8373 0\n"
        "137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n142 48 1 13\n143 5 10044 13\n"
        "160 55 0 0\n161 53 11 11\n162 2 7737 0\n163 2 8124 0\n164 8 8124 0\n165 18 0 0\n166 26 8250 0\n"
        "167 3 0 0\n168 4 11 0\n169 7 7740 39\n170 1 0 0\n171 48 0 3\n172 5 10044 3\n"
        "190 55 0 0\n191 53 17 17\n192 2 8388 0\n193 2 8775 0\n194 8 8775 0\n195 18 0 0\n196 26 8515 0\n"
        "197 3 0 0\n198 4 17 0\n199 7 8518 9\n200 1 0 0\n201 48 1 12\n202 5 10044 12\n"
        "220 113 1 0\n221 113 2 10\n"
        "230 48 2 20\n231 5 10044 20\n240 115 20 29\n241 115 20 29\n242 115 20 31\n"
        "7350 116 10 0\n7420 134 47 0\n"
        "9000 115 20 31\n9001 115 20 31\n9002 115 20 31\n9003 115 20 31\n9004 115 20 31\n9005 115 20 31\n"
        "9006 115 20 31\n9007 115 20 31\n9008 115 20 31\n9009 115 20 31\n9010 115 20 31\n9011 115 20 31\n"
        "9500 115 20 29\n9501 115 20 30\n"
        "11100 116 21 0\n12550 116 6 0\n12560 134 9 0\n12700 116 7 0\n13700 116 5 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script, true));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    InfoWindowRun run;
    CHECK(s.run_to(18000, [&] { run.add(g); }));
    CHECK(run.sent_home && run.guessed && run.priorities && run.home6 && run.home7);
    CHECK(run.queue47 && run.walking9);
    CHECK(run.has("handyman_values 20 litter=39 plants=31 repair=30"));
    CHECK(run.has("handyman_values 20 litter=0 plants=0 repair=100"));
    CHECK(run.has("force_cure 47") && run.has("force_cure 9"));
    CHECK(run.has("go_home 10 state=68") && run.has("go_home 5 state=48") && run.has("go_home 6 state=32") &&
          run.has("go_home 7 state=32"));
    CHECK(std::find(g.command_log.begin(), g.command_log.end(), "7420 134 47 0") != g.command_log.end());
    CHECK(w.seed == 0xe79eb3f5);
}

namespace
{

// The commands of tools/replay/scripts/toilets.txt before the toilets' own:
// gp_office.txt's up to the GP's office opened (no doctor), and the warm
// corridor with a drinks machine.
std::string toilets_script(const std::string &toilets)
{
    std::string gp = kGpOfficeScript;
    gp = gp.substr(0, gp.find("114 48 1 11\n"));
    return gp + toilets +
           "160 98 0 0\n161 37 7 1\n162 37 44 3\n163 43 0 0\n164 7 9658 7\n165 7 9276 44\n166 7 9410 44\n"
           "167 7 10047 44\n170 79 10 0";
}

// The seeds the runs end on at turn 18000: toilets.txt keeps the level's VIP
// visit so it is the original's; staff_room.txt and toilets_none.txt run with
// the VIP off, so theirs are this port's.
constexpr uint32_t kStaffRoomSeed = 0xd6095cc3;
constexpr uint32_t kToiletsSeed = 0xd79852c9;
constexpr uint32_t kToiletsNoneSeed = 0xad49eab6;

// The toilets (64,52)-(67,55) of toilets.txt, up to their objects.
constexpr const char *kToiletsRoom = "130 55 0 0\n131 53 29 29\n132 2 6720 0\n133 2 7107 0\n134 8 7107 0\n"
                                     "135 18 0 0\n136 26 7233 0\n137 3 0 0\n138 4 29 0\n";

} // namespace

// get_nearest_room_info: the room of the kind whose door is nearest, of the
// person's hospital, in use (state 0 or 1), nearer than 9999 (squared).
void test_nearest_room_info()
{
    auto world = std::make_unique<th::World>();
    th::World &w = *world;
    th::Thing p;
    p.type = th::kThingStaff;
    p.x.tile = 10;
    p.y.tile = 10;
    auto room = [&](uint16_t id, int x, int y, int8_t state, uint8_t owner) {
        th::RoomInfo &r = w.rooms[id];
        r.type = 25;
        r.state = state;
        r.owner = owner;
        r.door_tile = uint16_t(x + y * 128);
    };
    CHECK(th::get_nearest_room_info(w, p, 25) == 0);
    room(5, 20, 10, 0, 0);
    room(6, 12, 12, 4, 0);
    room(7, 11, 11, 0, 1);
    CHECK(th::get_nearest_room_info(w, p, 25) == 5);
    room(8, 10, 15, 1, 0);
    CHECK(th::get_nearest_room_info(w, p, 25) == 8);
    CHECK(th::get_nearest_room_info(w, p, 29) == 0);
    // 100 cells away squares past 9999: not found.
    room(5, 110, 10, 0, 0);
    room(8, 10, 110, 0, 0);
    CHECK(th::get_nearest_room_info(w, p, 25) == 0);
}

// tools/replay/scripts/staff_room.txt: with the policy's rest_threshold at
// 100, the doctor, the nurse and the handyman take breaks (take_a_break) in
// the staff room, resting (rest_staff) on its sofa, at pool and the video
// game until no longer tired; one picked up mid-game, one ordered out, one
// put down in the staff room; ending on the original's seed at turn 18000.
void test_staff_room_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 9 9\n132 2 7602 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8371 0\n"
        "137 3 0 0\n138 4 9 0\n139 7 7731 264\n140 7 7990 1\n141 1 0 0\n142 48 0 3\n143 5 10044 3\n"
        "160 55 0 0\n161 53 25 25\n162 2 6714 0\n163 2 7230 0\n164 8 7230 0\n165 18 0 0\n166 26 7356 0\n"
        "167 3 0 0\n168 4 25 0\n169 37 10 1\n170 37 57 1\n171 43 0 0\n172 7 6973 19\n173 7 6716 10\n"
        "174 7 7098 57\n175 1 0 0\n180 48 2 19\n181 5 10044 19\n182 48 3 23\n183 5 10044 23\n220 113 4 100\n"
        "2100 104 19 0\n2101 5 10044 19\n8100 97 3 0\n9000 113 4 300\n10000 104 11 0\n10001 5 6971 11\n"
        "17000 94 23 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    // The staff room is the eighth room listed.
    constexpr uint16_t kStaffRoom = 8;
    EventCounts counts;
    std::array<bool, 3> rested{};
    bool nurse_in = false, nurse_rested = false, nurse_out = false, put_in = false;
    CHECK(s.run_to(18000, [&] {
        counts.add(g);
        for (const th::TraceEvent &e : g.events.events) {
            rested[0] |= e.text.starts_with("rested 3 ");
            rested[1] |= e.text.starts_with("rested 11 ");
            rested[2] |= e.text.starts_with("rested 19 ");
            if (g.turn_count > 8100 && g.turn_count <= 8400)
                nurse_rested |= e.text.starts_with("rested 3 ");
        }
        if (g.turn_count == 8100)
            nurse_in = w.things[3].room == kStaffRoom;
        if (g.turn_count == 8400)
            nurse_out = w.things[3].room != kStaffRoom && !(w.things[3].orders & 0xf);
        if (g.turn_count == 10001)
            put_in = w.things[11].room == kStaffRoom;
    }));
    std::printf("staff_room: %d breaks, %d rests, %d pastimes, seed %#x\n", counts["take_a_break"],
                counts["rest_staff"], counts["staff_room"], unsigned(w.seed));
    // The nurse ordered out leaves before she is rested.
    CHECK(rested[0] && rested[1] && rested[2] && nurse_in && nurse_out && !nurse_rested && put_in);
    CHECK(counts["take_a_break"] >= 8 && counts["staff_room"] > 20 && counts["policy_rest"] == 2);
    CHECK(w.seed == kStaffRoomSeed);
}

// tools/replay/scripts/toilets.txt: waiting patients with a full bladder
// leave the queue for the toilets, use a toilet (relieved: bladder emptied)
// and a sink, and walk out again; one sent home on the toilet; ending on the
// original's seed at turn 18000. The level's VIP visit is kept, so the run
// follows the replay script.
void test_toilets_level1(const char *dir)
{
    const std::string script =
        toilets_script(std::string(kToiletsRoom) + "139 37 51 1\n140 43 0 0\n142 7 6720 51\n143 7 6723 32\n"
                                                   "144 7 6721 51\n145 1 0 0\n") +
        "\n7169 116 19 0";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script, true));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    EventCounts counts;
    int sinks = 0;
    bool emptied = true, sent_home = false;
    CHECK(s.run_to(18000, [&] {
        counts.add(g);
        for (const th::TraceEvent &e : g.events.events) {
            sinks += e.text.starts_with("use_object ") && e.text.ends_with(" kind=32");
            // PS_PLAY_COMPLEX_ANIM (0x10): on the toilet when sent home.
            sent_home |= e.text == "go_home 19 state=16";
            if (e.text.starts_with("relieved ")) {
                const th::Thing &p = w.things[std::stoul(e.text.substr(9))];
                emptied &= p.bladder == 0 || p.illness == 29;
            }
        }
    }));
    std::printf("toilets: %d visits, %d relieved, %d sinks, seed %#x\n", counts["to_toilets"], counts["relieved"],
                sinks, unsigned(w.seed));
    CHECK(counts["to_toilets"] >= 10 && counts["relieved"] >= 10 && sinks > 0 && emptied);
    CHECK(counts["leave_toilets"] >= 10 && sent_home);
    CHECK(w.seed == kToiletsSeed);
}

// tools/replay/scripts/toilets_none.txt: with no toilets, a patient's
// bladder overflows in the corridor (wets, PF_WETS); ending on the
// original's seed at turn 18000.
void test_toilets_none_level1(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 1, toilets_script("")));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    EventCounts counts;
    CHECK(s.run_to(18000, [&] { counts.add(g); }));
    CHECK(counts["wets"] > 0 && counts["to_toilets"] == 0);
    CHECK(g.world.seed == kToiletsNoneSeed);
}
