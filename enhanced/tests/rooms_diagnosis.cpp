// Scripted runs of the diagnosis and treatment rooms on levels 1 and 6, played
// with the commands of a replay script as th_replay plays them.
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
#include "core/world/reaper.h"
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

// Whether patient `p` has had treatment `treatment`.
bool had_treatment(const th::Thing &p, uint8_t treatment)
{
    return std::find(p.treatments.begin(), p.treatments.end(), treatment) != p.treatments.end();
}

// A diagnosis room run_diagnosis_room checks: its name, the level and
// script that build it, the treatment it gives, its room type, its staff's
// state while working there, the turn to run to and the original's seed
// then.
struct DiagnosisRoom {
    const char *name;
    int level_number;
    const std::string &script;
    uint8_t treatment;
    uint8_t room_type;
    uint8_t staff_state;
    uint32_t turns;
    uint32_t seed;
};

// With the game data, a diagnosis room in use as tools/replay/scripts/<room>.txt
// builds it, run to the turn the replay check agrees with the original to:
// its member of staff works there, patients come in for its examination,
// pay its fee and are sent on, ending on the original's seed. `seen` is called after
// every turn. Returns the patients diagnosed there.
int run_diagnosis_room(const char *dir, const DiagnosisRoom &spec,
                       const std::function<void(const th::Game &)> &seen = {})
{
    ScriptedGame s;
    CHECK(s.start(dir, spec.level_number, spec.script));
    if (!s.game)
        return 0;
    th::Game &g = *s.game;
    th::World &w = g.world;
    std::vector<bool> examined(th::kThingCount);
    int diagnosed = 0, sent_on = 0;
    bool working = false;
    CHECK(s.run_to(spec.turns, [&] {
        const uint16_t room = w.room_of_type[spec.room_type];
        for (uint16_t id = 1; id < w.things.size(); id++) {
            const th::Thing &t = w.things[id];
            if (t.type == th::kThingStaff && room && t.room == room && t.state == spec.staff_state)
                working = true;
            if (t.type != th::kThingPatient) {
                examined[id] = false;
                continue;
            }
            if (!examined[id] && room && t.room == room && t.treatment == spec.treatment) {
                examined[id] = true;
                diagnosed++;
            }
        }
        for (const th::TraceEvent &e : g.events.events)
            sent_on += e.text.starts_with("next_treatment ");
        if (seen)
            seen(g);
    }));
    std::printf("%s: %d patients examined, income $%d, seed %#x\n", spec.name, diagnosed,
                int(g.hospital.illnesses[spec.treatment].income), unsigned(w.seed));
    CHECK(working && diagnosed > 0 && sent_on > 0 && g.hospital.illnesses[spec.treatment].income > 0);
    CHECK(w.rooms[w.room_of_type[spec.room_type]].patients_seen > 0);
    CHECK(w.seed == spec.seed);
    return diagnosed;
}

} // namespace

// tools/replay/scripts/general_diag.txt: a general diagnosis room and a
// second doctor, who examines patients there (SS_DOCTOR_IN_MACHINE_ROOM)
// and sends them back to the GP.
void test_general_diag_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8373 0\n137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n"
                               "142 48 1 13\n143 5 10044 13";
    bool back_to_gp = false;
    run_diagnosis_room(dir, {"general diagnosis", 1, script, 41, 27, 0x24, 18249, 0xae815976}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things)
            back_to_gp |= t.type == th::kThingPatient && had_treatment(t, 41) &&
                          t.treatment == th::kIllGeneralPractice && t.state == th::kPsGotoQueue;
    });
    CHECK(back_to_gp);
}

// tools/replay/scripts/cardiogram.txt (level 6) with a handyman hired at turn
// 14400: a cardiogram room, its doctor and the handyman, who repairs the
// machine once it has worn and is smoking (SS_PLAY_ANISEQ).
void test_cardiogram_level6(const char *dir)
{
    const std::string script =
        "5 98 0 0\n6 37 11 1\n7 43 0 0\n8 7 7481 11\n10 48 3 33\n11 5 7735 33\n101 55 0 0\n102 53 7 7\n"
        "103 2 6188 0\n104 2 6704 0\n105 8 6704 0\n106 18 0 0\n107 26 6062 0\n108 3 0 0\n109 4 7 0\n"
        "110 7 6446 1\n111 7 6192 2\n112 7 6447 6\n113 1 0 0\n114 48 1 18\n115 5 6969 18\n130 55 0 0\n"
        "131 53 12 12\n132 2 6196 0\n133 2 6583 0\n134 8 6583 0\n135 18 0 0\n136 26 6069 0\n137 3 0 0\n"
        "138 4 12 0\n139 7 6454 13\n140 7 6452 272\n141 1 0 0\n142 48 1 19\n143 5 6970 19\n14400 48 2 38\n"
        "14401 5 6841 38";
    int32_t most_used = 0;
    bool repairing = false, smoke = false, offered = false;
    run_diagnosis_room(dir, {"cardiogram", 6, script, 38, 12, 0x24, 18249, 0x4befe69e}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things) {
            if (t.type == th::kThingObject && t.subtype == 13) {
                most_used = std::max(most_used, t.times_used);
                smoke |= t.smoke[0] || t.smoke[1] || t.smoke[2];
            }
            repairing |= t.type == th::kThingStaff && t.state == th::kStaffPlayAniseq;
        }
        offered |= th::check_playercall(g.world, 2, 0, 0) != 0;
    });
    // Worn past three quarters (smoking), paged and repaired; an emergency
    // offered and expired.
    CHECK(most_used >= 10 && smoke && repairing && offered);
}

// tools/replay/scripts/ward.txt: a ward and a nurse, who puts patients to
// bed for their diagnosis (PS_IN_BED, SS_NURSE_IN_WARD).
void test_ward_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 9 9\n132 2 7602 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8371 0\n137 3 0 0\n138 4 9 0\n139 7 7731 264\n140 7 7990 1\n141 1 0 0\n"
                               "142 48 0 3\n143 5 10044 3";
    bool in_bed = false;
    run_diagnosis_room(dir, {"ward", 1, script, 42, 9, 0x32, 18249, 0xbc3054b4}, [&](const th::Game &g) {
        for (const th::Thing &t : g.world.things)
            in_bed |= t.type == th::kThingPatient && t.state == th::kPsInBed && t.object &&
                      g.world.things[t.object].subtype == 8;
    });
    CHECK(in_bed);
}

// tools/replay/scripts/psychiatry.txt: a psychiatry room and a
// psychiatrist, who examines patients there and cures multiple TV
// personalities (cure_illness).
void test_psychiatry_level1(const char *dir)
{
    const std::string script = std::string(kGpOfficeScript) +
                               "130 55 0 0\n131 53 8 8\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n"
                               "136 26 8373 0\n137 3 0 0\n138 4 8 0\n139 7 7859 530\n140 7 8119 16\n"
                               "141 7 7733 61\n142 1 0 0\n143 48 1 15\n144 5 10044 15";
    uint16_t cured = 0;
    run_diagnosis_room(dir, {"psychiatry", 1, script, 43, 8, 0x24, 18249, 0xf4854d4d},
                       [&](const th::Game &g) { cured = g.hospital.illnesses[22].cured; });
    CHECK(cured > 0);
}

// tools/replay/scripts/treatment.txt: a pharmacy and its nurse, an
// inflation room and its doctor, with a policy guessing cures early.
// Patients are cured by drugs (administer_drugs), one drinking in the
// cabinet (PS_ENERGISES) and one inflated; others die (person_dies), float
// away (PS_FLYS) or are taken into the gates of hell by the grim reaper
// (summon_the_reaper), the gates removed afterwards; ending on the
// original's seed.
void test_treatment_level1(const char *dir)
{
    const std::string script =
        std::string(kGpOfficeScript) +
        "130 55 0 0\n131 53 27 27\n132 2 7731 0\n133 2 8247 0\n134 8 8247 0\n135 18 0 0\n136 26 8373 0\n"
        "137 3 0 0\n138 4 27 0\n139 7 7860 20\n140 7 8119 16\n141 1 0 0\n142 48 1 15\n143 5 10044 15\n"
        "160 55 0 0\n161 53 11 11\n162 2 7737 0\n163 2 8124 0\n164 8 8124 0\n165 18 0 0\n166 26 8250 0\n"
        "167 3 0 0\n168 4 11 0\n169 7 7740 39\n170 1 0 0\n171 48 0 3\n172 5 10044 3\n"
        "190 55 0 0\n191 53 17 17\n192 2 8388 0\n193 2 8775 0\n194 8 8775 0\n195 18 0 0\n196 26 8515 0\n"
        "197 3 0 0\n198 4 17 0\n199 7 8518 9\n200 1 0 0\n201 48 1 12\n202 5 10044 12\n"
        "220 113 1 0\n221 113 2 10";
    ScriptedGame s;
    CHECK(s.start(dir, 1, script));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    th::World &w = g.world;
    bool energised = false, flying = false, reaper = false, into_hell = false;
    bool money_fx = false, fed_up = false;
    int drugs = 0, gates_placed = 0, gates_removed = 0;
    CHECK(s.run_to(18249, [&] {
        for (const th::Thing &t : w.things) {
            energised |= t.type == th::kThingPatient && t.state == th::kPsEnergises;
            flying |= t.type == th::kThingPatient && t.state == th::kPsFlys;
            into_hell |= t.type == th::kThingPatient && t.state == th::kPsAtHellGate;
            reaper |= t.type == th::kThingReaper;
            // A floating money number showing a value (create_numfx) and a
            // patient leaving fed up (person_go_home's ICON_FED_UP).
            money_fx |= t.type == th::kThingNumfx && t.numfx_amount > 0;
            fed_up |= t.type == th::kThingPatient && t.icon_id == 7;
        }
        for (const th::TraceEvent &e : g.events.events) {
            drugs += e.text.starts_with("drugs ");
            gates_placed += e.text.starts_with("place_object ") && e.text.find(" kind=48 ") != std::string::npos;
            gates_removed += e.text.starts_with("remove_object ");
        }
    }));
    const auto &ill = g.hospital.illnesses;
    std::printf("treatment: %d drugs given, %u cured, %u dead, seed %#x\n", drugs, unsigned(g.finance.cures),
                unsigned(g.finance.deaths), unsigned(w.seed));
    // Drugs for invisibility (5), the uncommon cold (16) and the squits
    // (29); inflation for bloaty head (2).
    CHECK(drugs > 0 && ill[5].cured > 0 && ill[16].cured > 0 && ill[29].cured > 0 && ill[2].cured > 0);
    CHECK(energised && flying && reaper && into_hell);
    CHECK(money_fx && fed_up);
    CHECK(gates_placed > 0 && gates_removed == gates_placed);
    CHECK(g.finance.deaths > 0 && ill[5].deaths > 0 && g.finance.cures > 0);
    // The cure and death sounds the simulation queued for the interface
    // (SFX_CHEER 0xaf and SFX_BOO 0xae, core/audio/sounds.h).
    int cheers = 0, boos = 0;
    for (uint8_t sfx : w.treatment_sounds) {
        cheers += sfx == 0xaf;
        boos += sfx == 0xae;
    }
    std::printf("treatment sounds: %d cheers, %d boos\n", cheers, boos);
    CHECK(cheers > 0 && boos > 0);
    CHECK(w.seed == 0x53a3bd6a);
}
