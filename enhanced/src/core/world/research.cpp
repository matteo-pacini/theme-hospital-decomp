#include "core/world/research.h"

#include "core/data/balance.h"
#include "core/ui/charts.h"
#include "core/world/game.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>

namespace th
{

namespace
{

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

// OBJ_PHARMACY_CABINET: the object of the illnesses drugs cure.
constexpr uint8_t kObjPharmacyCabinet = 39;
// ROOM_RESEARCH.
constexpr uint8_t kRoomResearch = 28;

// expertise[].research_type (src/array.cpp), by illness: what research
// gives for it, 1 a drug, 2 a treatment machine, 3 a diagnosis machine, 0
// nothing.
constexpr std::array<uint8_t, 47> kResearchType = {0, 0, 2, 2, 0, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 1,
                                                   1, 1, 2, 2, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 2, 0,
                                                   1, 1, 2, 2, 3, 3, 3, 3, 3, 0, 0, 0, 0, 2, 2};

// research_workstations (src/tables.cpp): what counts as a place to work in
// a research room: the research computer, the atom analyser, the autopsy
// machine, desks and filing cabinets.
constexpr std::array<uint8_t, 5> kResearchWorkstations = {40, 41, 55, 1, 2};

const Balance &settings_of(const World &world)
{
    static const Balance statics;
    return world.settings ? *world.settings : statics;
}

HospitalSettings &hospital_of(World &world)
{
    static HospitalSettings none;
    return world.hospital ? *world.hospital : none;
}

const HospitalSettings &hospital_of(const World &world)
{
    static const HospitalSettings none;
    return world.hospital ? *world.hospital : none;
}

uint8_t kind_of(uint8_t illness)
{
    return kExpertiseData[illness % 47].kind;
}

// The illness has a machine to research (expertise research_type 2 or 3).
bool machine_research(uint8_t illness)
{
    return kResearchType[illness % 47] == 2 || kResearchType[illness % 47] == 3;
}

// expertise[].cost: the points the illness's research needs (RschReqd).
uint32_t expertise_cost(const World &world, uint8_t illness)
{
    return settings_of(world).expertise[illness % 47].rsch_reqd;
}

// The object can be researched: in the level and not yet available.
bool equipment_researchable(const World &world, uint8_t kind)
{
    return world.research.objects[kind].equip_available == 0 &&
           settings_of(world).objects[kind].available_for_level == 1;
}

// The object is bought or researched and in the level.
bool equipment_owned(const World &world, uint8_t kind)
{
    return world.research.objects[kind].equip_available == 1 &&
           settings_of(world).objects[kind].available_for_level == 1;
}

bool drug_known(const World &world, uint8_t illness)
{
    return kind_of(illness) == kObjPharmacyCabinet && hospital_of(world).illnesses[illness % 47].discovered == 1;
}

// find_drug_improve_cost's and do_illness_research's cost of a drug's next
// improvement at effectiveness `rating`.
int32_t drug_improve_cost(const World &world, uint8_t illness, uint32_t rating)
{
    const Balance::Gbv &gbv = settings_of(world).gbv;
    return int32_t((gbv.rsch_improve_cost_percent + rating / 25 * gbv.rsch_improve_increment_percent) *
                   expertise_cost(world, illness) / 100);
}

// find_hw_improve_cost's and do_illness_research's cost of a machine's next
// upgrade.
int32_t hw_improve_cost(const World &world, uint8_t illness, uint8_t kind)
{
    const Balance::Gbv &gbv = settings_of(world).gbv;
    uint32_t level = world.research.objects[kind].upgrade_level / 2;
    return int32_t((gbv.rsch_improve_cost_percent + gbv.rsch_improve_increment_percent * level) *
                   expertise_cost(world, illness) / 100);
}

// find_best_drug_rating (0x6e184): hospital 0's drug effectiveness for
// `illness`, as the game's byte.
uint8_t find_best_drug_rating(const World &world, uint8_t illness)
{
    return uint8_t(hospital_of(world).illnesses[illness % 47].drug_effectiveness);
}

// item_researched (0x6d288) in a one-player game: object `item` can be
// bought, announced. The first-to-research bonus and the rivals' later date
// are a network game's.
void item_researched(World &world, uint8_t item, uint8_t illness)
{
    trace_event("researched %u illness=%u", unsigned(item), unsigned(illness));
    world.research.objects[item].equip_available = 1;
    announce_this(world, kAnnObjectResearched, item);
}

// drug_improved (0x6d428): at random, the drug for `illness` costs 5% less
// (down to 10; one time in seven it also works gbv.drug_improve_rate better,
// up to 100) or, if it works less than perfectly, works
// gbv.drug_improve_rate better (one time in seven the cost then falls by 1,
// and is cut to 10 if above). Announced if either happened.
void drug_improved(World &world, uint8_t illness)
{
    IllnessFigures &drug = hospital_of(world).illnesses[illness % 47];
    const uint8_t rate = settings_of(world).gbv.drug_improve_rate;
    bool done = false;
    auto raise = [&](uint32_t effect) { drug.drug_effectiveness = std::min<uint16_t>(uint8_t(effect), 100); };
    if (!(old_newrand(world.seed) & 1)) {
        auto cost = uint16_t(drug.drug_cost);
        if (cost > 10) {
            cost = uint16_t(std::max(cost * 95 / 100, 10));
            drug.drug_cost = cost;
            if (old_newrand(world.seed) % 7 == 0)
                raise(drug.drug_effectiveness + rate);
            done = true;
        }
    } else if (uint8_t(drug.drug_effectiveness) < 100) {
        raise(uint8_t(drug.drug_effectiveness) + rate);
        if (old_newrand(world.seed) % 7 == 0)
            drug.drug_cost = std::min<uint16_t>(uint16_t(drug.drug_cost - 1), 10);
        done = true;
    }
    trace_event("drug_improved %u effect=%u cost=%d", unsigned(illness), unsigned(drug.drug_effectiveness),
                int(drug.drug_cost));
    if (done)
        announce_this(world, kAnnDrugImproved, illness);
}

// drug_researched (0x6d6a0): the drug for `illness` at the level's starting
// effectiveness and cost, and the illness treatable, announced.
void drug_researched(World &world, uint8_t illness)
{
    trace_event("drug_researched %u", unsigned(illness));
    const Balance::Gbv &gbv = settings_of(world).gbv;
    IllnessFigures &drug = hospital_of(world).illnesses[illness % 47];
    drug.drug_effectiveness = uint8_t(gbv.start_rating);
    drug.drug_cost = gbv.start_cost;
    world.cure_available[illness % 47] = 1;
    announce_this(world, kAnnNewDrug, illness);
}

// item_upgraded (0x6d748): object `item` one level up: at levels 2, 8, 14,
// ... (by six) new ones are gbv.research_increment stronger, up to
// gbv.max_object_strength; at 4, 10, 16, ... they cost an eighth less,
// rounded down to tens, but not below a third of the level's price.
// Announced.
void item_upgraded(World &world, uint8_t item)
{
    ObjectStats &stats = world.research.objects[item];
    const Balance &settings = settings_of(world);
    switch (stats.upgrade_level % 6) {
    case 2:
        if (stats.strength < settings.gbv.max_object_strength)
            stats.strength = uint16_t(stats.strength + settings.gbv.research_increment);
        break;
    case 4: {
        auto cost = uint32_t(stats.cost) * 7 / 8;
        cost -= cost % 10;
        auto floor = uint32_t(settings.objects[item].start_cost) / 3;
        stats.cost = int32_t(cost < floor ? floor : cost);
        break;
    }
    default:
        break;
    }
    stats.upgrade_level = uint8_t(stats.upgrade_level + 2);
    trace_event("upgraded %u level=%u strength=%u cost=%d", unsigned(item), unsigned(stats.upgrade_level),
                unsigned(stats.strength), int(stats.cost));
    announce_this(world, kAnnObjectImproved, item);
}

// research_category_exhausted (0x6e694): category `category` closed
// (research_alloc -1), announced, with its sound for the interface
// (Research::exhausted).
void research_category_exhausted(World &world, uint8_t category)
{
    trace_event("category_exhausted %u", unsigned(category));
    hospital_of(world).research_alloc[category] = -1;
    announce_this(world, kAnnCategoryDone, category);
    world.research.exhausted++;
}

// do_research's work for one category with `amount` points; false if it
// had nothing to spend them on.
bool do_research_category(World &world, uint8_t category, int32_t amount)
{
    auto spend = [&](ResearchCost kind) {
        uint8_t illness = choose_next_research_illness(world, kind);
        if (illness)
            do_illness_research(world, illness, uint32_t(amount), category);
        return illness != 0;
    };
    switch (category) {
    case kRcCure:
    case kRcDiagnosis:
        if (spend(category == kRcCure ? ResearchCost::NewCure : ResearchCost::NewDiag))
            return true;
        research_category_exhausted(world, category);
        return false;
    case kRcDrugs:
        return spend(ResearchCost::NewDrug) || spend(ResearchCost::DrugImprove);
    case kRcImprovements:
        return spend(ResearchCost::HwImprove);
    default:
        if (uint8_t illness = hospital_of(world).research_illness)
            do_illness_research(world, illness, uint32_t(amount), kRcSpecialisation);
        return true;
    }
}

} // namespace

void start_research(World &world)
{
    world.research = {};
    world.announce = {};
    const Balance &settings = settings_of(world);
    for (size_t i = 0; i < 0x3e; i++) {
        const Balance::Object &o = settings.objects[i];
        ObjectStats &stats = world.research.objects[i];
        if (o.available_for_level) {
            stats.equip_available = o.start_avail;
            stats.available = o.when_avail;
        }
        stats.cost = o.start_cost;
        stats.strength = o.start_strength;
    }
}

void announce_this(World &world, uint8_t kind, uint32_t value)
{
    for (uint32_t &slot : world.announce[kind % kAnnounceKinds]) {
        if (slot == 0) {
            slot = value;
            break;
        }
        if (slot == value)
            break;
    }
}

void done_announcement(World &world, uint8_t kind)
{
    std::array<uint32_t, 4> &queue = world.announce[kind % kAnnounceKinds];
    for (size_t i = 0; i + 1 < queue.size(); i++)
        queue[i] = queue[i + 1];
    queue.back() = 0;
}

bool equipment_available(const World &world, int kind)
{
    return kind > 0 && kind < kObjectKinds && equipment_owned(world, uint8_t(kind));
}

int32_t object_cost(const World &world, int kind)
{
    return kind >= 0 && kind < kObjectKinds ? world.research.objects[size_t(kind)].cost : 0;
}

uint16_t object_strength(const World &world, int kind)
{
    return kind >= 0 && kind < kObjectKinds ? world.research.objects[size_t(kind)].strength : 0;
}

void do_research(World &world)
{
    Research &r = world.research;
    const HospitalSettings &hospital = hospital_of(world);
    for (uint8_t category = 0; category < kResearchCategories; category++) {
        int32_t alloc = hospital.research_alloc[category];
        int32_t amount = alloc * r.points / 100;
        if (alloc <= 0 || amount <= 0)
            continue;
        r.points -= amount;
        r.spent += amount;
        if (!do_research_category(world, category, amount)) {
            r.points += amount;
            r.spent -= amount;
        }
    }
}

void auto_available(World &world, const Date &date)
{
    auto month = uint32_t(date.year * 12 + date.month);
    for (uint8_t item = 0; item < 0x3e; item++) {
        ObjectStats &stats = world.research.objects[item];
        if (!equipment_researchable(world, item) || stats.available == 0 || month < stats.available)
            continue;
        trace_event("available %u", unsigned(item));
        stats.equip_available = 1;
        announce_this(world, kAnnObjectAvailable, item);
    }
}

void research_contribution(World &world, const Thing &staff, int16_t amount)
{
    if (staff.owner != kPlayer)
        return;
    const Balance::Gbv &gbv = settings_of(world).gbv;
    int32_t work = amount;
    if (staff.tiredness > gbv.very_tired)
        work += staff.ability / 4;
    else if (staff.tiredness > gbv.tired)
        work += staff.ability / 2;
    else
        work += staff.ability;
    work = int16_t(work);
    if (staff.subtype == kStaffDoctor && staff.crazy)
        work = -work / 2;
    Research &r = world.research;
    trace_event("research_points %u amount=%d points=%d", unsigned(&staff - world.things.data()), int(work),
                int(r.points));
    r.points += work;
    r.earned += work;
    if (r.points < 0)
        r.points = 0;
}

int32_t research_cost(const World &world, ResearchCost kind, uint8_t illness)
{
    illness %= 47;
    const uint8_t object = kind_of(illness);
    switch (kind) {
    case ResearchCost::NewCure:
    case ResearchCost::NewDiag:
        if (equipment_researchable(world, object) && kResearchType[illness] == (kind == ResearchCost::NewCure ? 2 : 3))
            return int32_t(expertise_cost(world, illness));
        return -1;
    case ResearchCost::NewDrug:
        if (drug_known(world, illness) && world.cure_available[illness] == 0)
            return int32_t(expertise_cost(world, illness));
        return -1;
    case ResearchCost::DrugImprove:
        if (drug_known(world, illness) && world.cure_available[illness] == 1) {
            uint8_t rating = find_best_drug_rating(world, illness);
            if (rating < 100)
                return drug_improve_cost(world, illness, rating);
        }
        return -1;
    case ResearchCost::HwImprove:
        if (equipment_owned(world, object) && machine_research(illness) &&
            world.research.objects[object].upgrade_level < 40)
            return hw_improve_cost(world, illness, object);
        return -1;
    }
    return -1;
}

uint8_t choose_next_research_illness(const World &world, ResearchCost kind)
{
    uint8_t found = 0;
    uint32_t best = 0xffffffff;
    for (int i = 0x2e; i >= 0; i--) {
        int32_t cost = research_cost(world, kind, uint8_t(i));
        if (cost > 0 && uint32_t(cost) < best) {
            best = uint32_t(cost);
            found = uint8_t(i);
        }
    }
    return found;
}

void do_illness_research(World &world, uint8_t illness, uint32_t amount, uint8_t category)
{
    trace_event("research %u illness=%u amount=%u", unsigned(category), unsigned(illness), unsigned(amount));
    Research &r = world.research;
    illness %= 47;
    const uint8_t kind = kind_of(illness);
    if (machine_research(illness)) {
        for (uint8_t i = 0; i < 0x2f; i++) {
            if (kind_of(i) == kind && expertise_cost(world, i) != 0) {
                illness = i;
                break;
            }
        }
    }
    int32_t &banked = r.illness_research[illness];
    banked += int32_t(amount);
    int32_t cost = 0;
    if (machine_research(illness) && equipment_researchable(world, kind)) {
        cost = r.cost[category] = int32_t(expertise_cost(world, illness));
        if (banked >= cost) {
            banked -= cost;
            item_researched(world, kind_of(illness), illness);
        }
    } else if (drug_known(world, illness) && world.cure_available[illness] == 0) {
        cost = r.cost[category] = int32_t(expertise_cost(world, illness));
        if (banked >= cost) {
            banked -= cost;
            drug_researched(world, illness);
        }
    } else if (drug_known(world, illness) && world.cure_available[illness] == 1) {
        cost = drug_improve_cost(world, illness, find_best_drug_rating(world, illness));
        if (banked >= cost) {
            banked -= cost;
            drug_improved(world, illness);
        }
    } else if (machine_research(illness) && equipment_owned(world, kind) && r.objects[kind].upgrade_level < 40) {
        cost = hw_improve_cost(world, illness, kind);
        if (banked >= cost) {
            banked -= cost;
            item_upgraded(world, kind);
        }
    }
}

uint16_t emptiest_research_room(const World &world, uint8_t owner)
{
    int32_t best = -10000;
    uint16_t found = 0;
    for (size_t id = 1; id < world.rooms.size(); id++) {
        const RoomInfo &room = world.rooms[id];
        if (room.type != kRoomResearch || room.owner != owner)
            continue;
        int32_t count = 0;
        for (uint16_t t = room.first_object; t > 0; t = world.things[t].room_obj_next)
            for (uint8_t kind : kResearchWorkstations)
                if (world.things[t].subtype == kind)
                    count++;
        for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next)
            if (world.things[t].type == kThingStaff)
                count--;
        if (count > best) {
            best = count;
            found = uint16_t(id);
        }
    }
    return found;
}

uint16_t find_rand_research_room(World &world, uint8_t owner, uint16_t exclude)
{
    std::array<uint16_t, 30> list{};
    size_t count = 0;
    for (size_t id = 1; id < world.rooms.size() && count < list.size(); id++) {
        const RoomInfo &room = world.rooms[id];
        if (room.type == kRoomResearch && room.owner == owner && id != exclude)
            list[count++] = uint16_t(id);
    }
    if (count == 0)
        return 0;
    return list[old_newrand(world.seed) % count];
}

} // namespace th
