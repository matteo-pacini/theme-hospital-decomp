// The computer players' patients: process_computer_peep, computer_diag_progress
// and the diagnosis rooms a computer player's hospital gives
// (src/computer.cpp, src/treat.cpp).

#include "common.h"

#include "core/world/arrivals.h"
#include "core/world/computer.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/world.h"

#include <cstdint>
#include <memory>

namespace
{

using namespace th;

constexpr uint8_t kOwner = 2;
constexpr uint8_t kBloatyHead = 2;
constexpr uint8_t kGeneralPractice = 1;

// One computer player's hospital, a patient (thing 1) and the arrivals, with a
// GP's price of 100.
struct PeepFixture {
    std::unique_ptr<World> world = std::make_unique<World>();
    ArrivalState state;
    Balance settings;

    PeepFixture(uint8_t illness, int16_t health)
    {
        settings.expertise[kGeneralPractice].start_price = 100;
        HospitalMarket &market = state.hospitals[kOwner];
        market.price[kGeneralPractice] = 100;
        market.price_scale[kGeneralPractice] = 10000;
        market.hospital_reputation = 500;
        Thing &patient = world->things[1];
        patient.type = kThingPatient;
        patient.owner = kOwner;
        patient.illness = illness;
        patient.health = health;
        patient.treatment = kGeneralPractice;
    }

    RivalHospital &rival() { return world->rivals[kOwner]; }
    Thing &patient() { return world->things[1]; }
    ComputerBucket &bucket(size_t month) { return rival().buckets[month]; }

    void visit(int day, bool stale_drinks)
    {
        process_computer_peep(*world, state, settings, 1, {kOwner, day, stale_drinks});
    }
};

// A hospital with a GP and a doctor of ability `ability`, no other room.
void give_gp(PeepFixture &f, int16_t ability_sum)
{
    f.rival().cure_available[kGeneralPractice] = 1;
    f.rival().staff_by_kind[1] = 1;
    f.rival().staff_ability_sum[1] = ability_sum;
}

// computer_diag_progress: the average ability of the kind of staff the
// treatment asks for, 250 less a spread of two draws, and the room's bonus.
void check_diag_progress()
{
    PeepFixture f(kBloatyHead, 300);
    RivalHospital &rival = f.rival();
    rival.staff_by_kind = {2, 4, 1, 1};
    rival.staff_ability_sum = {600, 2000, 0, 0, 0, 0, 0, 0};
    Thing &patient = f.patient();
    patient.diagnosis = 100;
    patient.diagnosis_handicap = 50;

    // The GP asks for a doctor (skill bit 4): 2000 / 4.
    uint32_t seed = 0xabcdef01;
    uint32_t expected_seed = seed;
    int spread = int(old_normal_rand(expected_seed, 0, 500));
    patient.treatment = kGeneralPractice;
    computer_diag_progress(seed, rival, patient);
    CHECK(seed == expected_seed);
    CHECK(patient.diagnosis == std::max(100, 100 - 50 + 500 + 250 - spread));

    // A room that sets the patient back never lowers the diagnosis, and one
    // that adds to it adds its bonus (illness 2 at treatment 0x24: +400).
    patient.diagnosis = 600;
    patient.treatment = 0x24;
    seed = 0xabcdef01;
    computer_diag_progress(seed, rival, patient);
    CHECK(patient.diagnosis == std::max(600, 600 - 50 + 500 + 250 - spread + 400));

    // Treatment 5 asks for a nurse (skill bit 2): 600 / 2.
    patient.diagnosis = 0;
    patient.diagnosis_handicap = 0;
    patient.treatment = 5;
    seed = 0xabcdef01;
    computer_diag_progress(seed, rival, patient);
    CHECK(patient.diagnosis == std::max(0, 300 + 250 - spread));

    // A treatment that asks for no skill (45) takes the doctors'.
    patient.diagnosis = 0;
    patient.treatment = 45;
    seed = 0xabcdef01;
    computer_diag_progress(seed, rival, patient);
    CHECK(patient.diagnosis == std::max(0, 500 + 250 - spread));
}

// The patient's visit to a hospital with no GP ends at once, with no draw.
void check_no_gp()
{
    PeepFixture f(kBloatyHead, 300);
    uint32_t seed = f.world->seed;
    f.state.game_month = 11;
    f.visit(10, false);
    CHECK(f.world->seed == seed);
    RivalHospital &rival = f.rival();
    CHECK(rival.patient_load == 1);
    const ComputerBucket &b = f.bucket(11);
    CHECK(b.patients_done == 1 && b.cures == 0 && b.deaths == 0 && b.income == 0);
    // Only the case is counted: no treatment found and the next treatment
    // change nothing, and the patient is sent away.
    CHECK(b.rep_points == 0 && b.rep_events == 1 && b.room_rep_events[kBloatyHead] == 1);
    CHECK(f.state.hospitals[kOwner].diagnosed == 0);

    // The simulated clock starts at the day: 28 + 5 passes day 30, so the
    // visit goes into the next month, January's.
    PeepFixture g(kBloatyHead, 300);
    g.state.game_month = 11 + 12;
    g.visit(28, false);
    CHECK(g.bucket(0).patients_done == 1 && g.bucket(11).patients_done == 0);
}

// A GP who diagnoses at once: one spread draw, the GP's price, the diagnosis
// and the cure.
void check_diagnosed_by_gp()
{
    PeepFixture f(kBloatyHead, 300);
    give_gp(f, 30000);
    f.rival().cure_available[kBloatyHead] = 1;
    uint32_t seed = f.world->seed;
    uint32_t expected_seed = seed;
    old_normal_rand(expected_seed, 0, 500);
    f.visit(1, false);
    CHECK(f.world->seed == expected_seed);
    const ComputerBucket &b = f.bucket(0);
    CHECK(b.income == 100 && b.cures == 1 && b.deaths == 0 && b.patients_done == 1);
    // Diagnosed and cured: two points; the case is the only event.
    CHECK(b.rep_points == 2 && b.rep_events == 1);
    CHECK(f.state.hospitals[kOwner].diagnosed == 1);
    CHECK(f.patient().treatment == kBloatyHead);
    CHECK(f.rival().patient_load == 1);

    // A diagnosed patient the hospital cannot treat goes for treatment: the
    // diagnosis counts, the cure does not.
    PeepFixture g(kBloatyHead, 300);
    give_gp(g, 30000);
    g.visit(1, false);
    CHECK(g.bucket(0).cures == 0 && g.bucket(0).rep_points == 1 && g.bucket(0).patients_done == 1);
}

// A drug cure rolls its effectiveness (old_normal_rand(0, 99)): never
// reached with 0, always with the highest.
void check_drug_cure()
{
    // Illness 5 is cured with a drug.
    for (uint8_t effectiveness : {uint8_t(0), uint8_t(255)}) {
        PeepFixture f(5, 300);
        give_gp(f, 30000);
        f.rival().cure_available[5] = 1;
        f.rival().drug_effectiveness[5] = effectiveness;
        uint32_t expected_seed = f.world->seed;
        old_normal_rand(expected_seed, 0, 500);
        old_normal_rand(expected_seed, 0, 99);
        f.visit(1, false);
        CHECK(f.world->seed == expected_seed);
        const ComputerBucket &b = f.bucket(0);
        if (effectiveness == 0)
            CHECK(b.deaths == 1 && b.cures == 0 && b.rep_points == 0);
        else
            CHECK(b.deaths == 0 && b.cures == 1 && b.rep_points == 2);
    }
}

// The rest of the stages with no diagnosis room to go to and a doctor too
// weak to diagnose: the GP's spread draw, the drinks draw (when the visit has
// drinks), the leaving roll, the dying roll, and then the guess.
struct Oracle {
    int result = 0;
    int income = 0;
    int32_t diagnosis = 0;
    uint32_t seed = 0;
};

Oracle oracle(uint32_t seed, bool drinks, int guess_at, int health, uint32_t chance)
{
    Oracle o;
    int spread = int(old_normal_rand(seed, 0, 500));
    o.diagnosis = std::max(0, 250 - spread);
    if (drinks && old_newrand(seed) % 5 == 0)
        o.income += 20;
    if (old_newrand(seed) % chance == 0) {
        o.result = 3;
    } else if (old_newrand(seed) % uint32_t(health) == 0) {
        o.result = 2;
    } else {
        int pct = std::min(int(o.diagnosis) * 100 / 1000, 99);
        if (guess_at > pct)
            o.result = 3;
        else
            o.result = old_newrand(seed) % 100 > uint32_t(pct) ? 2 : 1;
    }
    o.seed = seed;
    return o;
}

void check_stages(bool loaded)
{
    constexpr int16_t kHealth = 3;
    int seen[4] = {};
    // 100 patients at the hospital with 3 rooms, 4 staff and 2 benches: 100 less 100 / 40,
    // 100 / 50 and 100 / 30.
    constexpr uint32_t kLoadedChance = 93;
    for (uint32_t s = 1; s <= 400; s++) {
        for (int mode = 0; mode < (loaded ? 2 : 3); mode++) {
            // 0 no drinks, 1 stale drinks, 2 enough drinks machines.
            int guess_at = (s & 1) ? 0 : 100;
            PeepFixture f(kBloatyHead, kHealth);
            f.world->seed = s * 2654435761u;
            f.settings.computer[0].guess_at = uint8_t(guess_at);
            give_gp(f, 0);
            f.rival().cure_available[kBloatyHead] = 1;
            if (mode == 2)
                f.rival().objects[7] = 1;
            if (loaded) {
                f.rival().patient_load = 99;
                f.rival().rooms_built = 3;
                f.rival().staff_count = 4;
                f.rival().objects[4] = 2;
            }
            Oracle o = oracle(f.world->seed, mode != 0, guess_at, kHealth, loaded ? kLoadedChance : 20);
            f.visit(1, mode == 1);
            const ComputerBucket &b = f.bucket(0);
            CHECK(f.world->seed == o.seed);
            CHECK(b.income == 100 + o.income);
            CHECK(b.deaths == (o.result == 2));
            CHECK(b.cures == (o.result == 1));
            CHECK(b.patients_done == 1);
            seen[o.result]++;
        }
    }
    // Every ending was reached.
    CHECK(seen[1] > 0 && seen[2] > 0 && seen[3] > 0);
}

// find_possible_diagnosis for a hospital given by its cure_available.
void check_find_possible_diagnosis()
{
    PeepFixture f(kBloatyHead, 300);
    std::array<uint8_t, 47> none{};
    uint32_t seed = 77;
    CHECK(find_possible_diagnosis(seed, f.patient(), none) == 0);
    CHECK(f.patient().treatment == 0 && seed == 77);

    // Illness 2 may be sent to treatments 36, 38, 39 and 41.
    std::array<uint8_t, 47> rooms{};
    rooms[36] = 1;
    CHECK(find_possible_diagnosis(seed, f.patient(), rooms) == 36);
    CHECK(f.patient().treatment == 36);
    // One it has been to is not sent to again, and nothing is left.
    add_treatment_to_history(f.patient());
    uint32_t before = seed;
    CHECK(find_possible_diagnosis(seed, f.patient(), rooms) == 0);
    CHECK(f.patient().treatment == 0 && seed != before);
}

} // namespace

void test_computer_peep()
{
    check_diag_progress();
    check_no_gp();
    check_diagnosed_by_gp();
    check_drug_cure();
    check_stages(false);
    check_stages(true);
    check_find_possible_diagnosis();
}
