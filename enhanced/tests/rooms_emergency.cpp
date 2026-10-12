// Level 6 emergencies, the answers to patients' calls (check_playercall in
// src/emerg.cpp, src/treat.cpp), a machine exploding and earthquakes
// (src/quake.cpp).
#include "common.h"
#include "rooms_internal.h"

#include "core/ui/build_room.h"
#include "core/ui/buy_objects.h"
#include "core/ui/charts.h"
#include "core/ui/town_map.h"
#include "core/world/advisor.h"
#include "core/world/build.h"
#include "core/world/disaster.h"
#include "core/world/events.h"
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

// The player's commands of tools/replay/scripts/cardiogram.txt's start
// (level 6): a reception desk and receptionist, then a GP's office and its
// doctor.
constexpr const char *kLevel6Reception = "5 98 0 0\n6 37 11 1\n7 43 0 0\n8 7 7481 11\n10 48 3 33\n11 5 7735 33\n";
constexpr const char *kLevel6GpOffice = "101 55 0 0\n102 53 7 7\n103 2 6188 0\n104 2 6704 0\n105 8 6704 0\n"
                                        "106 18 0 0\n107 26 6062 0\n108 3 0 0\n109 4 7 0\n110 7 6446 1\n"
                                        "111 7 6192 2\n112 7 6447 6\n113 1 0 0\n114 48 1 18\n115 5 6969 18\n";
// tools/replay/scripts/emergency_accept.txt's inflation room and inflator,
// and its second doctor.
constexpr const char *kLevel6InflationRoom = "130 55 0 0\n131 53 17 17\n132 2 6196 0\n133 2 6583 0\n134 8 6583 0\n"
                                             "135 18 0 0\n136 26 6069 0\n137 3 0 0\n138 4 17 0\n139 7 6454 9\n"
                                             "140 1 0 0\n";
constexpr const char *kLevel6SecondDoctor = "141 48 1 19\n142 5 6970 19\n";
// The level 6 emergency offered at turn 6150 (6 patients with bloaty heads,
// 75% to cure, $500 each), its call answered and the offer accepted or
// refused.
constexpr const char *kAcceptEmergency = "6200 10 2 0\n6201 125 31 0\n";
constexpr const char *kRefuseEmergency = "6200 10 2 0\n6201 125 30 0\n";

// An emergency scenario played on level 6 to `turn`: its events counted, the
// emergency_over and reputation events kept, and whether a helicopter and
// an emergency patient were seen.
struct EmergencyRun {
    ScriptedGame s;
    EventCounts counts;
    std::vector<std::string> results;
    bool helicopter = false;
    int most_patients = 0;

    bool run(const char *dir, const std::string &script, uint32_t turn)
    {
        if (!s.start(dir, 6, script))
            return false;
        th::Game &g = *s.game;
        return s.run_to(turn, [&] {
            counts.add(g);
            for (const th::TraceEvent &e : g.events.events)
                if (e.text.starts_with("emergency_over ") || e.text.starts_with("rep 0 event=1") ||
                    e.text.starts_with("balance 28 "))
                    results.push_back(e.text);
            int patients = 0;
            for (const th::Thing &t : g.world.things) {
                helicopter |= t.type == th::kThingHelicopter;
                patients += t.type == th::kThingPatient && (t.person_flags & th::kPfEmergency);
            }
            most_patients = std::max(most_patients, patients);
        });
    }
};

bool has(const std::vector<std::string> &lines, const char *line)
{
    return std::find(lines.begin(), lines.end(), line) != lines.end();
}

} // namespace

// tools/replay/scripts/emergency_accept.txt: the emergency accepted; its
// helicopter lands, its six patients go straight to the inflation room and
// are cured there, the bonus is paid and the reputation rises; its result
// call expires, ending it; ending on the original's seed at turn 9700.
void test_emergency_accept_level6(const char *dir)
{
    EmergencyRun r;
    CHECK(r.run(dir,
                std::string(kLevel6Reception) + kLevel6GpOffice + kLevel6InflationRoom + kLevel6SecondDoctor +
                    kAcceptEmergency,
                9700));
    if (!r.s.game)
        return;
    const th::Game &g = *r.s.game;
    std::printf("emergency accepted: %d patients, %d cured, seed %#x\n", r.counts["emergency_peep"],
                r.counts["emergency_cure"], unsigned(g.world.seed));
    CHECK(r.counts["emergency_offer"] == 1 && r.counts["emergency_accept"] == 1 && r.counts["emergency_start"] == 1);
    CHECK(r.counts["heli"] == 1 && r.counts["heli_landed"] == 1 && r.counts["heli_gone"] == 1 && r.helicopter);
    CHECK(r.counts["emergency_peep"] == 6 && r.most_patients == 6 && r.counts["emergency_cure"] == 6);
    CHECK(r.counts["emergency_timer"] == 100 && r.counts["emergency_dies"] == 0);
    CHECK(has(r.results, "emergency_over cured=6 patients=6 bonus=3000"));
    CHECK(has(r.results, "balance 28 amount=3000 arg=0") && has(r.results, "rep 0 event=12 amount=0"));
    CHECK(g.advisor.heli_landed && g.world.emergency.state == 0 && g.world.emergency.cured == 6);
    CHECK(g.disasters.emergencies_active == 0 && !g.disasters.offers[0].used);
    CHECK(g.world.seed == 0x678d13b0);
}

// tools/replay/scripts/emergency_refuse.txt and emergency_ignore.txt: the
// emergency refused, or its call left to expire; no helicopter comes, and
// the offer stays counted outstanding, as in the game; ending on the
// original's seeds at turn 7200.
void test_emergency_refused_level6(const char *dir)
{
    EmergencyRun refused;
    CHECK(refused.run(dir, std::string(kLevel6Reception) + kLevel6GpOffice + kRefuseEmergency, 7200));
    if (!refused.s.game)
        return;
    const th::Game &g = *refused.s.game;
    CHECK(refused.counts["emergency_offer"] == 1 && refused.counts["emergency_refuse"] == 1);
    CHECK(refused.counts["emergency_accept"] == 0 && refused.counts["heli"] == 0 && !refused.helicopter);
    CHECK(g.world.emergency.state == 0 && g.disasters.emergencies_active == 1 && g.disasters.offers[0].used);
    CHECK(g.world.seed == 0x0a7a11af);

    EmergencyRun ignored;
    CHECK(ignored.run(dir, std::string(kLevel6Reception) + kLevel6GpOffice + kLevel6InflationRoom + kLevel6SecondDoctor,
                      7200));
    if (!ignored.s.game)
        return;
    const th::Game &h = *ignored.s.game;
    CHECK(ignored.counts["emergency_offer"] == 1 && ignored.counts["expire_call"] >= 1);
    CHECK(ignored.counts["emergency_refuse"] == 0 && ignored.counts["emergency_accept"] == 0 && !ignored.helicopter);
    CHECK(th::check_playercall(h.world, th::kCallEmergency, 0, 0) == 0 && h.disasters.emergencies_active == 1);
    CHECK(h.world.seed == 0x57d27adc);
}

// tools/replay/scripts/emergency_untreatable.txt: the emergency accepted
// with no inflation room: its patients go home at once, the reputation
// falls for the six not cured, and answering its result call ends it;
// ending on the original's seed at turn 8900.
void test_emergency_untreatable_level6(const char *dir)
{
    EmergencyRun r;
    CHECK(r.run(dir, std::string(kLevel6Reception) + kLevel6GpOffice + kAcceptEmergency + "8800 10 5 0\n", 8900));
    if (!r.s.game)
        return;
    const th::Game &g = *r.s.game;
    CHECK(r.counts["emergency_peep"] == 6 && r.counts["emergency_leaves"] == 6 && r.counts["emergency_cure"] == 0);
    CHECK(has(r.results, "emergency_over cured=0 patients=6 bonus=0") && has(r.results, "rep 0 event=13 amount=6"));
    CHECK(std::none_of(r.results.begin(), r.results.end(),
                       [](const std::string &line) { return line.starts_with("balance"); }));
    CHECK(g.world.emergency.state == 0 && th::check_playercall(g.world, th::kCallEmergencyOver, 1, 0) == 0);
    CHECK(g.world.seed == 0x607b8462);
}

// tools/replay/scripts/emergency_fail.txt: the emergency accepted with an
// inflation room but no doctor: its patients wait in the room's queue until
// the time is up, then lose their health and die; the reputation falls;
// ending on the original's seed at turn 9800.
void test_emergency_fail_level6(const char *dir)
{
    EmergencyRun r;
    CHECK(r.run(dir, std::string(kLevel6Reception) + kLevel6InflationRoom + kAcceptEmergency, 9800));
    if (!r.s.game)
        return;
    const th::Game &g = *r.s.game;
    CHECK(r.counts["emergency_peep"] == 6 && r.counts["emergency_dies"] == 6 && r.counts["emergency_cure"] == 0);
    CHECK(has(r.results, "emergency_over cured=0 patients=6 bonus=0") && has(r.results, "rep 0 event=13 amount=6"));
    CHECK(g.world.emergency.state == 0 && g.finance.deaths >= 6);
    CHECK(g.world.seed == 0xbcc69a42);
}

namespace
{

// A run of a script answering patients' calls: the answers' events, and
// the faxes the calls answered asked for (Game::choice_screen) by kind.
struct DecisionRun {
    ScriptedGame s;
    EventCounts counts;
    std::vector<std::string> choices;
    std::array<int, 11> faxes{};

    bool run(const char *dir, const std::string &script, uint32_t turn)
    {
        if (!s.start(dir, 1, script))
            return false;
        th::Game &g = *s.game;
        while (g.turn_count < turn) {
            for (; s.next < s.lines.size() && s.lines[s.next].first <= g.turn_count + 1; s.next++) {
                if (s.lines[s.next].first != g.turn_count + 1)
                    continue;
                if (!g.replay_input(s.lines[s.next].second))
                    return false;
                faxes[g.choice_screen.type % faxes.size()]++;
            }
            g.turn();
            counts.add(g);
            for (const th::TraceEvent &e : g.events.events) {
                if (e.text.starts_with("choice "))
                    choices.push_back(e.text);
            }
            g.events.events.clear();
        }
        return true;
    }
};

} // namespace

// tools/replay/scripts/decisions_diagnosis.txt: patients whose diagnosis is
// given up, their calls answered (the fax about them shown) and each
// answer taken: sent home, a cure guessed (which leads to a call about no
// treatment, answered by the research department) and asked to wait;
// ending on the level's seed at turn 18000.
void test_decisions_diagnosis_level1(const char *dir)
{
    DecisionRun r;
    CHECK(r.run(dir,
                std::string(kGpOfficeScript) + "7260 10 8 1\n7261 125 40 10\n8300 10 8 4\n8301 125 41 3\n"
                                               "8350 10 4 0\n8351 125 2 3\n8950 10 8 6\n8951 125 42 6\n",
                18000));
    if (!r.s.game)
        return;
    const std::vector<std::string> expected = {"choice 40 arg=10 awaiting=1", "choice 41 arg=3 awaiting=1",
                                               "choice 2 arg=3 awaiting=1", "choice 42 arg=6 awaiting=1"};
    CHECK(r.choices == expected);
    CHECK(r.faxes[th::kFaxNoDiagnosis] == 1 && r.faxes[th::kFaxNoTreatment] == 1);
    CHECK(r.counts["go_for_treatment"] > 0 && r.counts["choice"] == 4);
    CHECK(r.s.game->world.seed == 0xe1b5ffee);
}

// tools/replay/scripts/decisions_treatment.txt: patients with no treatment,
// their calls answered from the patient and from the panel's icon (the
// first call of the kind), one sent home and one asked to wait; ending on
// the original's seed at turn 18000.
void test_decisions_treatment_level1(const char *dir)
{
    DecisionRun r;
    const std::string general_diag = "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n"
                                     "135 18 0 0\n136 26 8373 0\n137 3 0 0\n138 4 27 0\n139 7 7860 20\n"
                                     "140 7 8119 16\n141 1 0 0\n142 48 1 13\n143 5 10044 13\n";
    CHECK(
        r.run(dir, kGpOfficeScript + general_diag + "6960 10 4 2\n6961 125 0 10\n7250 10 4 0\n7251 125 1 15\n", 18000));
    if (!r.s.game)
        return;
    const std::vector<std::string> expected = {"choice 0 arg=10 awaiting=1", "choice 1 arg=15 awaiting=1"};
    CHECK(r.choices == expected);
    CHECK(r.faxes[th::kFaxNoTreatment] == 2);
    CHECK(r.s.game->world.seed == 0xf6f6ed7e);
}

// A new illness's call answered: the fax about the illness, with the call
// gone; again with none left, the fax about nothing (kind 0), as in the
// game.
void test_new_illness_call(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 1, "1 0 0 0"));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::make_a_playercall(g.world, 7, 22, 0xf, 0);
    CHECK(g.world.first_call[0][7] != 0 && g.world.popups.pos[0] == g.world.first_call[0][7]);
    g.event = {th::EV_ANSWER_PLAYERCALL, 7, 0, 0};
    g.process_turn();
    CHECK(g.choice_screen.type == th::kFaxNewIllness && g.choice_screen.subject == 22);
    CHECK(g.world.first_call[0][7] == 0 && g.world.popups.pos[0] == 0);
    g.event = {th::EV_ANSWER_PLAYERCALL, 7, 0, 0};
    g.process_turn();
    CHECK(g.choice_screen.type == th::kFaxNewIllness && g.choice_screen.subject == 0);
    g.event = {th::EV_ANSWER_PLAYERCALL, 4, 0, 0};
    g.process_turn();
    CHECK(g.choice_screen.type == 0);
}

namespace
{

// tools/replay/scripts/cardiogram.txt's commands without its handyman (level
// 6): a reception, a GP's office and a cardiogram room (its machine thing
// 45), each with its doctor.
std::string cardiogram_without_handyman()
{
    return std::string(kLevel6Reception) + kLevel6GpOffice +
           "130 55 0 0\n131 53 12 12\n132 2 6196 0\n133 2 6583 0\n134 8 6583 0\n135 18 0 0\n136 26 6069 0\n"
           "137 3 0 0\n138 4 12 0\n139 7 6454 13\n140 7 6452 272\n141 1 0 0\n142 48 1 19\n143 5 6970 19\n";
}

constexpr uint16_t kCardiogram = 45;
constexpr uint16_t kCardiogramRoom = 7;

} // namespace

// tools/replay/scripts/machine_explode.txt: the cardiogram worn out with no
// handyman smokes, then blows up (turn 15037): debris flies and falls, the
// room's objects are taken off and wrecks left, the doctor in it is gone and
// the patient on it dies, the room is destroyed and its queue sent back to
// the GP; the reputation falls. Replacing the machine gone charges its owner,
// not the player: its thing has become an applicant (kNoOwner). Ending on the
// original's seed at turn 18000.
void test_machine_explodes_level6(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 6, cardiogram_without_handyman() + "10001 104 18 0\n11000 5 6970 18\n15700 138 45 0\n", true));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    EventCounts counts;
    int most_debris = 0;
    bool smoked = false;
    CHECK(s.run_to(18000, [&] {
        counts.add(g);
        smoked |= w.things[kCardiogram].overlay_frame != 0 && w.things[kCardiogram].smoke[0] == 1;
        int debris = int(std::count_if(w.things.begin(), w.things.end(),
                                       [](const th::Thing &t) { return t.type == th::kThingDebris; }));
        most_debris = std::max(most_debris, debris);
        if (g.turn_count == 13743)
            CHECK(w.rooms[kCardiogramRoom].first_person && w.rooms[kCardiogramRoom].queue_head == 0 &&
                  w.things[kCardiogram].type != th::kThingObject);
    }));
    CHECK(smoked && counts["modify_machine"] == 6 && counts["machine_warning"] == 4);
    CHECK(counts["machine_disaster"] == 1 && counts["room_destroyed"] == 1 && counts["remove_object"] == 2);
    CHECK(most_debris == 101 && std::none_of(w.things.begin(), w.things.end(),
                                             [](const th::Thing &t) { return t.type == th::kThingDebris; }));
    const th::RoomInfo &room = w.rooms[kCardiogramRoom];
    CHECK(room.type == 6 && room.state == 6 && w.room_of_type[12] == 0 && w.cure_available[0x24 + 2] != 1);
    CHECK(w.mishaps == 1 && w.machine_exploded && th::find_small_thing(w, 6454) != 0);
    CHECK(counts["replace_machine"] == 1 && w.seed == 0x4e8cf3b9 && g.balance == 47444);
}

// tools/replay/scripts/machine_saved.txt with a handyman hired at turn 14400:
// the cardiogram worn past its strength and smoking, a handyman called for it
// and hired, who repairs it before it breaks down: its wear, smoke and overlay
// gone. Ending on the level's seed at turn 18000.
void test_machine_saved_level6(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 6, cardiogram_without_handyman() + "11420 118 45 0\n13800 48 2 38\n13801 5 6841 38\n", true));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    const th::World &w = g.world;
    EventCounts counts;
    bool worn_out = false;
    CHECK(s.run_to(14412, [&] {
        counts.add(g);
        const th::Thing &m = w.things[kCardiogram];
        worn_out |= m.times_used >= m.strength;
    }));
    const th::Thing &machine = w.things[kCardiogram];
    CHECK(worn_out && counts["repair_request"] == 1 && counts["repair"] == 1 && counts["repaired"] == 1);
    CHECK(machine.times_used == 0 && machine.smoke == (std::array<uint8_t, 3>{}) && machine.overlay_frame == 0);
    CHECK(s.run_to(18000, [&] { counts.add(g); }));
    CHECK(counts["machine_disaster"] == 0 && w.rooms[kCardiogramRoom].type == 12 && w.seed == 0x75ad87d3);
}

// tools/replay/scripts/machine_replace.txt: the worn cardiogram replaced
// while the handyman hired for it is on his way: paid for, new and unworn,
// its page cancelled and the handyman off elsewhere. Ending on the
// level's seed at turn 18000.
void test_machine_replace_level6(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 6, cardiogram_without_handyman() + "12900 48 2 51\n12901 5 6841 51\n12950 138 45 0\n", true));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    const th::World &w = g.world;
    CHECK(s.run_to(12949, [] {}));
    CHECK(w.repair_pager.count == 1 && w.repair_pager.pages[0].responder == 51);
    int32_t balance = g.balance;
    CHECK(s.run_to(12950, [] {}));
    const th::Thing &machine = w.things[kCardiogram];
    CHECK(g.balance == balance - 1000 && machine.times_used == 0 && machine.strength == 13);
    CHECK(machine.smoke == (std::array<uint8_t, 3>{}) && w.repair_pager.count == 0);
    CHECK(s.run_to(18000, [] {}));
    CHECK(w.seed == 0x8136e0f1);
}

// generate_quake's do_earthquake: a quake at its peak wears every machine by
// its severity (Disasters::quake_severity) and hands one past 75% of its
// strength to modify_machine.
void test_do_earthquake_level6(const char *dir)
{
    ScriptedGame s;
    CHECK(s.start(dir, 6, cardiogram_without_handyman()));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    CHECK(s.run_to(1000, [] {}));
    th::Thing &machine = w.things[kCardiogram];
    CHECK(machine.type == th::kThingObject && machine.strength > 0);
    machine.times_used = machine.strength * 75 / 100 - 1;
    g.disasters.quake_severity = 6;
    g.disasters.quake_phase = 3;
    g.disasters.quake_strength = 16;
    g.disasters.quake_amplitude = 16;
    g.disasters.quake_timer = g.turn_count + 100;
    th::EventLog log;
    th::EventScope scope(&log, g.turn_count + 1);
    th::generate_quake(g);
    CHECK(g.disasters.quake_phase == 4);
    CHECK(machine.times_used == machine.strength * 75 / 100 + 5);
    int modified = 0;
    for (const th::TraceEvent &e : log.events)
        modified += e.text.starts_with("modify_machine ") &&
                    e.text.find(" " + std::to_string(kCardiogram) + " ") != std::string::npos;
    CHECK(modified == 1);
}
