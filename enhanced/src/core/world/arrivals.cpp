#include "core/world/arrivals.h"

#include "core/world/emergency.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/trace.h"

#include <algorithm>

namespace th
{

namespace
{

// Illnesses (the game's ILL_*, include/enums.h) that make_a_peep and
// effect_illness single out.
constexpr uint8_t kIllGeneralPractice = 1;
constexpr uint8_t kIllBloatyHead = 2;
constexpr uint8_t kIllHairyitis = 3;
constexpr uint8_t kIllKingComplex = 4;
constexpr uint8_t kIllInvisibility = 5;
constexpr uint8_t kIllSlackTongue = 7;
constexpr uint8_t kIllAlienDna = 8;
constexpr uint8_t kIllFracturedBones = 9;
constexpr uint8_t kIllBaldness = 10;
constexpr uint8_t kIllTransparency = 15;
constexpr uint8_t kIllTheSquits = 29;

// The alien patient kind (PK_ALIEN).
constexpr uint8_t kPatientAlien = 14;

// allocate_thing's reserve for people.
constexpr size_t kPeopleReserve = 50;

// The towns[] entry of a campaign level: setup_new_level plays levels above
// 14 as level 1.
size_t town_index(int level_number)
{
    return level_number > 14 ? 1 : size_t(level_number);
}

} // namespace

// effect_illness (0x54444) with cured 0: gives the patient `illness` and
// changes their looks to show it.
void effect_illness(uint32_t &seed, Thing &patient, uint8_t illness)
{
    switch (illness) {
    case kIllBloatyHead:
        patient.layers[0] = uint8_t(patient.layers[0] + 5);
        patient.layers[2] = 0;
        break;
    case kIllHairyitis:
        patient.layers[0] = 1;
        transform_peep_who(patient, 1);
        patient.layers[2] = 0;
        break;
    case kIllKingComplex:
        patient.layers[0] = 1;
        transform_peep_who(patient, 5);
        patient.layers[2] = 0;
        break;
    case kIllTransparency:
        transform_peep_who(patient, 3);
        break;
    case kIllSlackTongue:
        if (patient.subtype == 1)
            patient.layers[0] = uint8_t(patient.layers[0] + 4);
        else
            transform_peep_who(patient, 7);
        break;
    case kIllInvisibility:
        patient.layers[0] = 1;
        transform_peep_who(patient, 9);
        break;
    case kIllBaldness:
        transform_peep_who(patient, 7);
        patient.layers[0] = 6;
        break;
    case kIllFracturedBones:
        if (patient.subtype == 1) {
            uint8_t r = uint8_t(old_newrand(seed) % 3 + 1);
            patient.layers[2] = r & 1;
            patient.layers[3] = (r >> 1) & 1;
        } else {
            transform_peep_who(patient, 0xd);
            patient.layers[2] = uint8_t(old_newrand(seed) & 3);
            patient.layers[3] = uint8_t(old_newrand(seed) % 6);
            patient.layers[4] = uint8_t(old_newrand(seed) % 6);
        }
        break;
    case kIllAlienDna:
        patient.layers[0] = 1;
        patient.orig_subtype = patient.subtype;
        patient.subtype = kPatientAlien;
        break;
    default:
        break;
    }
    patient.illness = illness;
}

namespace
{

// add_contagiousness (0x707f4). The game gives no contagion during its
// hospital's epidemic (allhosps.epidemic_active).
void add_contagiousness(uint32_t &seed, Thing &patient, const ArrivalState &state, const Balance &settings,
                        bool epidemic_active)
{
    if (kNotContagious[patient.illness])
        return;
    uint8_t chance = settings.expertise[patient.illness].cont_rate;
    if (!chance)
        return;
    const Balance::Gbv &gbv = settings.gbv;
    bool early = false;
    if (gbv.reduce_cont_months && state.game_month < gbv.reduce_cont_months)
        early = true;
    else if (gbv.reduce_cont_peep_count && state.peeps_arrived < gbv.reduce_cont_peep_count)
        early = true;
    if (early)
        chance = uint8_t(chance * gbv.reduce_cont_rate / 100);
    if (epidemic_active)
        chance = 0;
    if (chance && old_newrand(seed) % chance == 0)
        patient.contagion = uint8_t(old_newrand(seed) % gbv.how_contagious + 1);
}

// One bidder in who_owns_peep (the game's PeepBid). Its 27-bit score cannot
// overflow: at most 1000 * 255 * 150% per hospital.
struct PeepBid {
    int32_t score;
    uint8_t player;
};

// sort_peep_bids (0x70e50): sorts the first three bids by ascending score;
// bids[3] is never compared.
void sort_peep_bids(std::array<PeepBid, 4> &bids)
{
    bool swapped = true;
    while (swapped) {
        swapped = false;
        for (size_t i = 0; i < 2; i++) {
            if (bids[i].score > bids[i + 1].score) {
                std::swap(bids[i], bids[i + 1]);
                swapped = true;
                break;
            }
        }
    }
}

} // namespace

void transform_peep_who(Thing &person, uint8_t change)
{
    int8_t subtype = kChangePersonWho[change][person.subtype];
    person.orig_subtype = person.subtype;
    if (subtype != -1)
        person.subtype = uint8_t(subtype);
}

int32_t max_price_for_treatment(const HospitalMarket &hosp, const Balance &settings, int treatment)
{
    int32_t base = settings.expertise[size_t(treatment)].start_price;
    int32_t rep = treatment >= 0x24 || treatment == kIllGeneralPractice ? hosp.hospital_reputation
                                                                        : hosp.reputation[size_t(treatment)];
    int32_t price = base * rep / 500;
    return base > price ? base : price;
}

void calc_pricing_factor(HospitalMarket &hosp, const Balance &settings, const std::array<uint8_t, 47> &cure_available)
{
    uint32_t max_total = 0;
    int32_t price_total = 0;
    for (size_t i = 0; i < cure_available.size(); i++) {
        if (cure_available[i] == 1) {
            max_total += uint32_t(max_price_for_treatment(hosp, settings, int(i)));
            price_total += hosp.price[i];
        }
    }
    // Divided unsigned, as in the game.
    int32_t level = max_total > 0 ? int32_t(uint32_t(price_total * 100) / max_total) : 100;
    hosp.price_level = int16_t(std::clamp(level, 0, 300));
    trace_event("price_level %d", int(hosp.price_level));
}

void update_single_cure_cost(HospitalMarket &hosp, const Balance &settings, int treatment)
{
    int32_t target = max_price_for_treatment(hosp, settings, treatment) * hosp.price_scale[size_t(treatment)] / 10000;
    int32_t diff = target - hosp.price[size_t(treatment)];
    if (diff < 0)
        diff = -diff;
    if (target / 100 <= diff)
        hosp.price[size_t(treatment)] = target;
}

void update_cure_costs(ArrivalState &state, const Balance &settings)
{
    for (HospitalMarket &hosp : state.hospitals)
        for (size_t i = 0; i < hosp.price.size(); i++)
            update_single_cure_cost(hosp, settings, int(i));
}

void start_arrivals(ArrivalState &state, const Balance &settings)
{
    state = {};
    for (HospitalMarket &hosp : state.hospitals) {
        hosp.open = settings.gbv.open_countdown == 0;
        for (size_t i = 0; i < hosp.price.size(); i++) {
            hosp.price[i] = settings.expertise[i].start_price;
            hosp.price_scale[i] = 10000;
            hosp.rep_points[i] = 20;
            hosp.rep_events[i] = 20;
            hosp.reputation[i] = 20 * 500 / 20; // evaluate_reputation
        }
    }
    calc_visitors_this_month(state, settings);
}

void calc_visitors_this_month(ArrivalState &state, const Balance &settings)
{
    for (const Balance::Population &p : settings.popn) {
        if (p.month == state.game_month) {
            state.visitor_change = uint16_t(p.change);
            break;
        }
        if (p.month > state.game_month)
            break;
    }
    state.visitors_per_month = uint16_t(state.visitors_per_month + state.visitor_change);
    state.peep_interval = state.visitors_per_month ? int16_t(1500 / state.visitors_per_month) : 0;
}

uint32_t old_normal_rand(uint32_t &seed, uint32_t low, uint32_t high, std::source_location caller)
{
    int32_t range = int32_t((high - low) / 2 + 1);
    if (range <= 0)
        range = 1;
    uint32_t sum = old_newrand(seed, caller) % uint32_t(range);
    sum += old_newrand(seed, caller) % uint32_t(range);

    return sum + low;
}

uint8_t create_an_illness(uint32_t &seed, const ArrivalState &state, const Balance &settings, int level_number)
{
    const Balance::Gbv &gbv = settings.gbv;
    uint8_t ill_rate = settings.towns[town_index(level_number)].ill_rate;
    uint8_t illness = 0;
    bool ok = false;
    if (ill_rate && old_newrand(seed) % ill_rate == 0) {
        ok = true;
        if (gbv.hold_visual_months && state.game_month < gbv.hold_visual_months)
            ok = false;
        else if (gbv.hold_visual_peep_count && state.peeps_arrived < gbv.hold_visual_peep_count)
            ok = false;
    }
    if (ok && settings.visuals_total) {
        uint16_t r = uint16_t(old_newrand(seed) % settings.visuals_total);
        for (size_t i = 0; i < settings.visuals.size(); i++) {
            if (r < settings.visuals[i]) {
                if (state.game_month < settings.visuals_available[i])
                    ok = false;
                else
                    illness = uint8_t(i + 2);
                break;
            }
        }
    }
    if (!ok) {
        uint16_t r = uint16_t(old_newrand(seed) % settings.non_visuals_total);
        for (size_t i = 0; i < settings.non_visuals.size(); i++) {
            if (r < settings.non_visuals[i]) {
                illness = uint8_t(i + 16);
                break;
            }
        }
    }
    return illness;
}

namespace
{

// who_owns_peep's market allocation: each hospital bids its reputation for
// `illness`, scaled by how its price compares with the most patients will
// pay, and a roll weighted by the bids picks the winner. -1 when every bid is
// 0.
int who_owns_peep_by_bids(uint32_t &seed, ArrivalState &state, const Balance &settings, uint8_t illness,
                          std::source_location caller)
{
    const Balance::Gbv &gbv = settings.gbv;
    std::array<PeepBid, 4> bids{};
    for (uint8_t i = 0; i < 4; i++) {
        HospitalMarket &hosp = state.hospitals[i];
        bids[i].player = i;
        bids[i].score = hosp.reputation[illness] * gbv.alloc_ind_rep;
        update_single_cure_cost(hosp, settings, illness);
        int32_t pct = max_price_for_treatment(hosp, settings, illness) * 100 / hosp.price[illness];
        if (pct < 50)
            pct = 50;
        else if (pct > 150)
            pct = 150;
        bids[i].score = bids[i].score * pct / 100;
    }
    sort_peep_bids(bids);
    int32_t total = 0;
    for (PeepBid &bid : bids) {
        total += bid.score;
        bid.score = total;
    }
    if (!total)
        return -1;
    int32_t roll = int32_t(old_newrand(seed, caller) % uint32_t(total));
    for (const PeepBid &bid : bids) {
        if (bid.score > roll)
            return bid.player;
    }
    return -1;
}

// One head make_a_peep may give a patient with a visual illness.
struct VisualChoice {
    uint8_t kind, head, palette;
};

// make_a_peep's choices[] for each visual illness, in the order the game
// fills it.
struct VisualChoices {
    std::array<VisualChoice, 10> choices{};
    uint32_t count = 0;
};

constexpr std::array<VisualChoices, 16> make_visual_choices()
{
    std::array<VisualChoices, 16> table{};
    for (size_t illness = 0; illness < table.size(); illness++) {
        VisualChoices &list = table[illness];
        for (size_t i = kGeneratePerson.size(); i-- > 0;) {
            if (!kSuitableIllness[illness][i])
                continue;
            const PeepType &type = kGeneratePerson[i];
            for (size_t k = type.count; k-- > 0;)
                list.choices[list.count++] = {type.kind, type.head[k], type.palette[k]};
        }
    }
    return table;
}

constexpr std::array<VisualChoices, 16> kVisualChoices = make_visual_choices();

constexpr bool every_visual_illness_has_a_choice()
{
    for (size_t illness = kIllBloatyHead; illness < kVisualChoices.size(); illness++) {
        if (!kVisualChoices[illness].count)
            return false;
    }
    return true;
}

static_assert(every_visual_illness_has_a_choice(), "make_a_peep picks among a visual illness's choices");

// The sum of kGeneratePerson's weights.
constexpr uint32_t generate_person_weight()
{
    uint32_t total = 0;
    for (const PeepType &type : kGeneratePerson)
        total += type.weight;
    return total;
}

constexpr uint32_t kGeneratePersonWeight = generate_person_weight();
static_assert(kGeneratePersonWeight > 0);

// make_a_peep's looks: picks the patient's kind and head (by kind weight for a
// non-visual illness, among the kinds suited to a visual one) and returns the
// palette for the other layers.
const PeepPalette &make_a_peep_pick_head(uint32_t &seed, Thing &peep, uint8_t illness, std::source_location caller)
{
    if (illness >= 16) {
        uint32_t total = kGeneratePersonWeight;
        uint32_t r = old_newrand(seed, caller) % total;
        size_t i = kGeneratePerson.size() - 1;
        for (;; i--) {
            total -= kGeneratePerson[i].weight;
            if (r >= total)
                break;
        }
        const PeepType &type = kGeneratePerson[i];
        peep.subtype = type.kind;
        uint32_t k = old_newrand(seed, caller) % type.count;
        peep.layers[0] = type.head[k];
        return kVariations[type.palette[k]];
    }
    const VisualChoices &list = kVisualChoices[illness];
    const VisualChoice &pick = list.choices[old_newrand(seed, caller) % list.count];
    peep.subtype = pick.kind;
    peep.layers[0] = pick.head;
    return kVariations[pick.palette];
}

// make_a_peep's thing: the first free one outside the people reserve (0 for
// none).
uint16_t make_a_peep_free_thing(const World &world)
{
    for (size_t i = 1; i < world.things.size() - kPeopleReserve; i++) {
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    }
    return 0;
}

} // namespace

int who_owns_peep(uint32_t &seed, ArrivalState &state, const Balance &settings, uint8_t illness, bool hospital_open)
{
    const Balance::Gbv &gbv = settings.gbv;
    constexpr uint32_t players = 4;
    int owner = -1;
    if (gbv.alloc_delay && state.game_month < gbv.alloc_delay) {
        owner = int(state.peep_owner_counter % players);
        state.peep_owner_counter++;
    } else if (old_newrand(seed) % gbv.alloc_rand == 0) {
        owner = int(old_newrand(seed) % players);
    } else {
        owner = who_owns_peep_by_bids(seed, state, settings, illness, std::source_location::current());
        if (owner < 0)
            owner = int(old_newrand(seed) % players);
    }
    bool open = owner == 0 ? hospital_open : state.hospitals[size_t(owner)].open;
    return open ? owner : -1;
}

uint16_t make_a_peep(World &world, const ArrivalState &state, const PeepSetup &setup, uint16_t pos, uint8_t illness,
                     uint8_t owner)
{
    uint16_t id = make_a_peep_free_thing(world);
    if (!id)
        return 0;
    uint32_t &seed = world.seed;
    Thing &peep = world.things[id];
    peep.owner = owner;
    peep.type = kThingPatient;
    peep.x = {0x80, uint8_t(pos % kMapSize)};
    peep.y = {0x80, uint8_t(pos / kMapSize)};
    peep.dest_x = peep.x.tile;
    peep.dest_y = peep.y.tile;
    peep.speed = kPeepSpeed;
    peep.anim_speed = kPeepAnimSpeed;

    const PeepPalette *pal = &make_a_peep_pick_head(seed, peep, illness, std::source_location::current());
    peep.layers[1] = pal->layer1[old_newrand(seed) % pal->count1];
    peep.layers[2] = pal->layer2[old_newrand(seed) % pal->count2];
    peep.layers[3] = pal->layer3[old_newrand(seed) % pal->count3];
    peep.layers[4] = 0;
    effect_illness(seed, peep, illness);
    add_contagiousness(seed, peep, state, setup.settings, owner == 0 && world.epidemic.active);
    peep.icon_id = 0;
    play_frame(peep, setup.anims, kPatientAnims[peep.subtype].walk);
    peep.happiness = 100;
    peep.warmth = 50;
    peep.refreshment = 100;
    peep.bladder = illness == kIllTheSquits ? int16_t(setup.settings.gbv.bowel_full / 2) : int16_t(0);
    peep.health = int16_t(old_normal_rand(seed, 200, 1000));
    peep.health_history.fill(int8_t(peep.health));
    peep.treatment = kIllGeneralPractice;
    peep.known_illness = kIllGeneralPractice;
    peep.diagnosis_handicap = uint16_t(old_normal_rand(seed, 1, setup.settings.expertise[illness].max_diag_diff));
    if (old_newrand(seed) % kTownExtras[town_index(setup.level_number)].insurance_odds == 0)
        peep.insurance = uint8_t(1 << old_newrand(seed) % 3);
    else
        peep.insurance = 0;
    peep.cash = int32_t(old_newrand(seed) % 95000 + 5000);
    if (owner == 0) {
        nav_init(world, peep);
        insert_node(world, id, peep.x, peep.y);
    }
    trace_event("peep %u illness=%u owner=%u pos=%u", unsigned(id), unsigned(illness), unsigned(owner), unsigned(pos));
    return id;
}

uint16_t create_a_peep(World &world, ArrivalState &state, const PeepSetup &setup, bool hospital_open,
                       uint32_t game_turn)
{
    if (kTownExtras[town_index(setup.level_number)].special_kind == 1)
        return 0;
    create_a_peep_emergency(world, state, setup);
    if (!state.peep_interval)
        return 0;
    if (game_turn % uint32_t(state.peep_interval) >= 25)
        return 0;
    uint8_t illness = create_an_illness(world.seed, state, setup.settings, setup.level_number);
    int owner = who_owns_peep(world.seed, state, setup.settings, illness, hospital_open);
    if (owner < 0)
        return 0;
    uint16_t id = 0;
    if (owner == 0) {
        uint16_t door = world.rooms[world.entrances[0][old_newrand(world.seed) & 3]].door_tile;
        id = make_a_peep(world, state, setup, door, illness, 0);
    } else if (uint16_t rival = make_a_peep(world, state, setup, 0, illness, uint8_t(owner))) {
        // process_computer_peep would treat them here.
        trace_event("remove_thing %u type=%u", unsigned(rival), unsigned(world.things[rival].type));
        remove_node(world, rival);

        world.things[rival] = Thing{};
    }
    state.peeps_arrived++;
    state.hospitals[size_t(owner)].visitors++;
    state.hospitals[size_t(owner)].month_visitors++;
    return id;
}

} // namespace th
