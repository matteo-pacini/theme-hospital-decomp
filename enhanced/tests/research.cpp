#include "common.h"

#include "core/data/balance.h"
#include "core/render/shadows.h"
#include "core/ui/charts.h"
#include "core/world/advisor.h"
#include "core/world/build.h"
#include "core/world/research.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace
{

constexpr uint8_t kObjSlicer = 26;
constexpr uint8_t kObjXray = 27;
constexpr uint8_t kObjCardio = 13;
constexpr uint8_t kIllSlackTongue = 7;
constexpr uint8_t kIllXray = 39;
constexpr uint8_t kIllInvisibility = 5;
constexpr uint8_t kRoomSlackTongue = 20;
constexpr uint8_t kRoomXray = 16;

// A world with the level's settings and policy as Game::start gives them,
// without a map: what research reads.
struct ResearchWorld {
    th::Balance settings;
    th::HospitalSettings hospital;
    std::unique_ptr<th::World> world = std::make_unique<th::World>();

    ResearchWorld()
    {
        for (th::Balance::Object &o : settings.objects)
            o.available_for_level = 0;
        settings.objects[kObjSlicer] = {1500, 0, 0, 10, 1};
        settings.objects[kObjXray] = {4000, 0, 0, 12, 1};
        settings.objects[kObjCardio] = {1000, 1, 0, 13, 1};
        settings.expertise[kIllSlackTongue].rsch_reqd = 10000;
        settings.expertise[kIllXray].rsch_reqd = 25000;
        settings.expertise[38].rsch_reqd = 20000;
        settings.gbv.rsch_improve_cost_percent = 10;
        settings.gbv.rsch_improve_increment_percent = 10;
        settings.gbv.max_object_strength = 20;
        settings.gbv.research_increment = 2;
        settings.gbv.drug_improve_rate = 5;
        settings.gbv.start_rating = 95;
        settings.gbv.start_cost = 100;
        th::start_hospital_settings(hospital, settings);
        world->settings = &settings;
        world->hospital = &hospital;
        th::start_research(*world);
    }
};

// A level started as th_replay starts it, played with a replay script's
// commands (lines "turn type value arg").
struct ResearchGame {
    th::GameData gd;
    std::unique_ptr<th::Game> game;
    std::vector<std::pair<uint32_t, std::string>> lines;
    size_t next = 0;

    bool start(const char *dir, int level_number, const std::string &script)
    {
        std::string error;
        if (!gd.load(dir, &error))
            return false;
        auto level = gd.load_level(level_number, &error);
        th::Balance settings;
        if (!level || !th::load_balance(dir, level_number, th::kDefaultBalanceId, settings, &error))
            return false;
        game = std::make_unique<th::Game>();
        game->staff_market.ref_counts = th::reference_counts(gd.text());
        game->start(*level, level_number, settings, gd.anims(), gd.person_complexes(), gd.staff_complexes());
        game->skipped_calls = th::kSkipComputerAction;
        th::add_static_shadows(game->world.map);
        th::start_advisor(game->advisor, false);
        th::start_hospital_settings(game->hospital, game->settings());
        th::setup_insurances(game->world, 0);
        game->world.path_finder = th::PathFinder::Original;
        for (size_t at = 0; at < script.size();) {
            size_t end = script.find('\n', at);
            std::string line = script.substr(at, end - at);
            at = end == std::string::npos ? script.size() : end + 1;
            size_t space = line.find(' ');
            lines.emplace_back(uint32_t(std::stoul(line.substr(0, space))), line.substr(space + 1));
        }
        return true;
    }

    // Runs the turns up to `turn`, calling `each` with every event.
    bool run_to(uint32_t turn, const std::function<void(const std::string &)> &each)
    {
        while (game->turn_count < turn) {
            for (; next < lines.size() && lines[next].first <= game->turn_count + 1; next++)
                if (lines[next].first == game->turn_count + 1 && !game->replay_input(lines[next].second))
                    return false;
            game->turn();
            for (const th::TraceEvent &e : game->events.events)
                each(e.text);
            game->events.events.clear();
        }
        return true;
    }
};

// tools/replay/scripts/research_default.txt (level 3): a research room
// with the autopsy machine, a desk and a filing cabinet, and two
// researchers.
constexpr const char *kResearchRoomScript =
    "101 55 0 0\n102 53 28 28\n103 2 8876 0\n104 2 9392 0\n105 8 9392 0\n106 18 0 0\n107 26 8750 0\n108 3 0 0\n"
    "109 4 28 0\n110 7 9260 55\n111 7 9264 257\n112 7 8880 258\n113 1 0 0\n114 48 1 13\n115 5 9004 13\n"
    "116 48 1 16\n117 5 9005 16\n";

} // namespace

// do_research's choices and their costs: the cheapest machine of a
// category (the highest numbered of those tied), a machine's points banked
// on its first illness, the upgrades every six levels raising the strength
// and lowering the price, and a category running out closing it.
void test_research_choices()
{
    ResearchWorld r;
    th::World &w = *r.world;
    CHECK(th::research_cost(w, th::ResearchCost::NewCure, kIllSlackTongue) == 10000);
    CHECK(th::research_cost(w, th::ResearchCost::NewDiag, kIllXray) == 25000);
    // The cardiogram is there from the start: its upgrades are open, not it.
    CHECK(th::research_cost(w, th::ResearchCost::NewDiag, 38) == -1);
    CHECK(th::research_cost(w, th::ResearchCost::HwImprove, 38) == 2000);
    CHECK(th::choose_next_research_illness(w, th::ResearchCost::NewDiag) == kIllXray);
    CHECK(th::choose_next_research_illness(w, th::ResearchCost::NewCure) == kIllSlackTongue);
    CHECK(!th::equipment_available(w, kObjXray) && th::equipment_available(w, kObjCardio));

    th::do_illness_research(w, kIllXray, 24999, th::kRcDiagnosis);
    CHECK(!th::equipment_available(w, kObjXray) && w.research.cost[th::kRcDiagnosis] == 25000);
    th::do_illness_research(w, kIllXray, 2, th::kRcDiagnosis);
    CHECK(th::equipment_available(w, kObjXray) && w.research.illness_research[kIllXray] == 1);
    CHECK(w.announce[th::kAnnObjectResearched][0] == kObjXray);
    CHECK(th::choose_next_research_illness(w, th::ResearchCost::NewDiag) == 0);

    // Upgrades: level 0 to 2 changes nothing, 2 to 4 the strength, 4 to 6
    // the price (7/8, down to tens: 3500).
    for (int i = 0; i < 3; i++)
        th::do_illness_research(w, kIllXray, 100000, th::kRcImprovements);
    const th::ObjectStats &x = w.research.objects[kObjXray];
    CHECK(x.upgrade_level == 6 && x.strength == 14 && x.cost == 3500);
    CHECK(th::object_cost(w, kObjXray) == 3500 && th::object_strength(w, kObjXray) == 14);

    // Diagnosis has nothing left: its share is kept and the category closed.
    w.research.points = 1000;
    r.hospital.research_alloc = {0, 100, 0, 0, 0};
    th::do_research(w);
    CHECK(w.research.points == 1000 && r.hospital.research_alloc[th::kRcDiagnosis] == -1);
    CHECK(w.research.exhausted == 1 && w.announce[th::kAnnCategoryDone][0] == th::kRcDiagnosis);
}

// Drugs: a known illness's drug researched without a pharmacy, then, once
// treatable, improved by a draw: its cost or its effectiveness.
void test_research_drugs()
{
    ResearchWorld r;
    th::World &w = *r.world;
    r.settings.expertise[kIllInvisibility].rsch_reqd = 1000;
    r.hospital.illnesses[kIllInvisibility].discovered = 1;
    CHECK(th::research_cost(w, th::ResearchCost::NewDrug, kIllInvisibility) == 1000);
    th::do_illness_research(w, kIllInvisibility, 1000, th::kRcDrugs);
    CHECK(w.cure_available[kIllInvisibility] == 1);
    CHECK(r.hospital.illnesses[kIllInvisibility].drug_effectiveness == 95);
    // Improving costs (10% + 10% per 25 effectiveness) of the research.
    CHECK(th::research_cost(w, th::ResearchCost::DrugImprove, kIllInvisibility) == 400);
    w.seed = 1;
    th::EventLog log;
    log.limit = 0;
    {
        th::EventScope scope(&log, 1);
        for (int i = 0; i < 20; i++)
            th::do_illness_research(w, kIllInvisibility, 400, th::kRcDrugs);
    }
    const th::IllnessFigures &drug = r.hospital.illnesses[kIllInvisibility];
    int improved = 0;
    for (const th::TraceEvent &e : log.events)
        improved += e.text.starts_with("drug_improved ");
    CHECK(improved > 0 && drug.drug_effectiveness == 100 && drug.drug_cost < 100 && drug.drug_cost >= 10);
}

// research_contribution: the researcher's ability, halved when tired and
// quartered when very tired, plus the work's points; a crazy doctor takes
// away half. Never below 0.
void test_research_contribution()
{
    ResearchWorld r;
    th::World &w = *r.world;
    r.settings.gbv.tired = 600;
    r.settings.gbv.very_tired = 700;
    th::Thing &d = w.things[5];
    d.type = th::kThingStaff;
    d.subtype = th::kStaffDoctor;
    d.ability = 500;
    th::research_contribution(w, d, 200);
    CHECK(w.research.points == 700);
    d.tiredness = 650;
    th::research_contribution(w, d, 100);
    CHECK(w.research.points == 1050);
    d.tiredness = 800;
    th::research_contribution(w, d, 0);
    CHECK(w.research.points == 1175 && w.research.earned == 1175);
    d.crazy = 1;
    d.tiredness = 0;
    th::research_contribution(w, d, 1000);
    CHECK(w.research.points == 425);
    th::research_contribution(w, d, 1000);
    CHECK(w.research.points == 0);
}

// person_autopsied: the patient's illness gains the autopsy percent of its
// research cost; the reputation draw (one in three, two in three when the
// staff's average happiness is under 25) announces the loss to the advisor.
void test_person_autopsied()
{
    ResearchWorld r;
    th::World &w = *r.world;
    r.settings.gbv.autopsy_rsch_percent = 50;
    r.settings.expertise[kIllSlackTongue].rsch_reqd = 10000;
    th::Thing &patient = w.things[5];
    patient.type = th::kThingPatient;
    patient.illness = kIllSlackTongue;
    th::Thing &doctor = w.things[6];
    doctor.type = th::kThingStaff;
    doctor.subtype = th::kStaffDoctor;
    w.staff_count = 1;
    th::person_autopsied(w, 5);
    CHECK(w.research.illness_research[kIllSlackTongue] == 5000);
    for (uint16_t happiness : {uint16_t(100), uint16_t(0)}) {
        doctor.happiness = happiness;
        for (uint32_t seed = 1; seed <= 12; seed++) {
            w.announce = {};
            w.seed = seed;
            uint32_t drawn = seed;
            uint32_t draw = th::old_newrand(drawn, "person_autopsied") % 3;
            w.seed = seed;
            th::person_autopsied(w, 5);
            bool bad = happiness == 0 ? draw != 0 : draw == 0;
            CHECK((w.announce[th::kAnnAutopsyDiscovered][0] == 1) == bad);
        }
    }
}

// research_default.txt and research_concentrate.txt (level 3), run to the
// turn the replay check agrees with the original to: researchers at work
// (SS_DOCTOR_RESEARCHES) earn points; by default the slack tongue slicer,
// the cardiogram and the operating table are researched, a drug and
// upgrades; concentrated on the X-ray it is researched by turn 2850, its
// room then offered, and upgraded.
void test_research_level3(const char *dir)
{
    ResearchGame s;
    CHECK(s.start(dir, 3, kResearchRoomScript));
    if (!s.game)
        return;
    th::Game &g = *s.game;
    int points = 0, researched = 0, upgraded = 0, drugs = 0;
    CHECK(!th::room_available(g.world, kRoomSlackTongue));
    CHECK(s.run_to(18000, [&](const std::string &e) {
        points += e.starts_with("research_points ");
        researched += e.starts_with("researched ");
        upgraded += e.starts_with("upgraded ");
        drugs += e.starts_with("drug_researched ");
    }));
    std::printf("research: %d contributions, %d researched, %d upgrades, %d drugs, seed %#x\n", points, researched,
                upgraded, drugs, unsigned(g.world.seed));
    CHECK(points == 157 && researched == 2 && upgraded == 4 && drugs == 1);
    CHECK(th::room_available(g.world, kRoomSlackTongue) && th::equipment_available(g.world, kObjCardio));
    CHECK(g.world.seed == 0x0d86179a && g.balance == 42115);

    ResearchGame c;
    CHECK(c.start(dir, 3,
                  std::string(kResearchRoomScript) + "120 114 39 0\n121 80 0 0\n122 80 1 0\n123 80 2 0\n124 80 3 0\n"
                                                     "125 80 4 100\n"));
    if (!c.game)
        return;
    CHECK(c.run_to(2849, [](const std::string &) {}));
    CHECK(!th::room_available(c.game->world, kRoomXray));
    CHECK(c.run_to(2850, [](const std::string &) {}));
    CHECK(th::room_available(c.game->world, kRoomXray));
    CHECK(c.game->hospital.research_alloc[th::kRcSpecialisation] == 100 &&
          c.game->hospital.research_illness == kIllXray);
}
