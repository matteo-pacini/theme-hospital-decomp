#include "core/world/emergency.h"

#include "core/ui/charts.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/epidemic.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/popups.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// The player call about an epidemic.
constexpr uint16_t kCallEpidemic = 3;

// Turns in the months emergencies and earthquakes are scheduled by.
constexpr uint32_t kTurnsPerMonth = 1500;

// World::Emergency::state values.
constexpr int8_t kEmergencySummon = 5;
constexpr int8_t kEmergencyHeliSent = 3;
constexpr int8_t kEmergencyTimeUp = 2;
constexpr int8_t kEmergencyOver = 1;
constexpr int8_t kEmergencyAccepted = 10;

// The helicopter's and its shadow's animations (summon_vehicle's arguments
// in try_to_generate_emergency), and the height it comes down from.
constexpr uint16_t kAnimHelicopter = 0x4cc;
constexpr uint16_t kAnimHelicopterShadow = 0x7a7;
constexpr int16_t kHeliAltitude = 0x190;

uint32_t month_of(const Date &date)
{
    return uint32_t(date.year * 12 + date.month);
}

// check_emergency_criteria (0xaf118) for a one-player game. The epidemic
// fax being shown is the interface's; an epidemic running counts.
bool check_emergency_criteria(const Disasters &d, const World &world, const Balance &settings, bool open,
                              uint32_t game_turn)
{
    if (game_turn < d.next_emergency_turn || !d.emergency_pending)
        return false;
    if (check_playercall(world, kCallEpidemic, 0, 0) || world.epidemic.active != 0 || world.emergency.state != 0 ||
        !open)
        return false;
    return !settings.illness_kinds.empty() && d.emergencies_active < 4;
}

// setup_emergency_stats (0xaf448) for the player's offer, in a free slot
// (one is, with fewer than four offers outstanding).
void setup_emergency_stats(Disasters &d, uint16_t call)
{
    trace_event("emergency_offer %u illness=%u count=%u bonus=%u win=%u", unsigned(call), unsigned(d.emergency_illness),
                unsigned(d.emergency_patients), unsigned(d.emergency_bonus), unsigned(d.emergency_win_percent));
    for (Disasters::Offer &offer : d.offers) {
        if (!offer.used) {
            offer = {call, d.emergency_bonus, 1, d.emergency_illness, d.emergency_patients, d.emergency_win_percent};
            break;
        }
    }
    d.emergencies_active++;
}

// allocate_thing (0x1b474): the first free thing but the last `reserve`, 0
// for none.
uint16_t allocate_thing(const World &world, size_t reserve)
{
    for (size_t i = 1; i < world.things.size() - reserve; i++)
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    return 0;
}

// remove_thing (0x199e8), traced as the game's is.
void remove_thing(World &world, uint16_t id)
{
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(world.things[id].type));
    remove_node(world, id);
    world.things[id] = Thing{};
}

// create_shadow (0x762cc) of hospital 0: a shadow on `tile` cast by thing
// `caster`, showing animation `anim`. Returns it, 0 if no thing was free.
uint16_t create_shadow(World &world, const AnimData &anims, uint16_t tile, uint16_t caster, uint16_t anim)
{
    uint16_t id = allocate_thing(world, 0x32);
    if (!id)
        return 0;
    Thing &shadow = world.things[id];
    shadow.type = kThingShadow;
    shadow.x = {0x80, uint8_t(tile % kMapSize)};
    shadow.y = {0x80, uint8_t(tile / kMapSize)};
    insert_node(world, id, shadow.x, shadow.y);
    shadow.sprite = uint16_t(anims.starts[anim].frame);
    shadow.flip = uint8_t(anims.starts[anim].flip);
    shadow.caster = caster;
    shadow.depth = -8;
    return id;
}

// summon_vehicle (0xafd0c) of hospital 0, a helicopter (kind 0): it comes
// down on `tile` from kHeliAltitude, with its shadow. Returns it, 0 if no
// thing was free. The arrival sounds are the interface's.
uint16_t summon_vehicle(World &world, const AnimData &anims, uint16_t tile)
{
    uint16_t id = allocate_thing(world, 0x19);
    if (!id)
        return 0;
    Thing &heli = world.things[id];
    heli.type = kThingHelicopter;
    heli.x = {0x80, uint8_t(tile % kMapSize)};
    heli.y = {0x80, uint8_t(tile / kMapSize)};
    heli.anim_speed = 2;
    heli.facing = 1;
    insert_node(world, id, heli.x, heli.y);
    heli.speed = 8;
    heli.altitude = kHeliAltitude;
    heli.sprite = uint16_t(anims.starts[kAnimHelicopter].frame);
    heli.flip = uint8_t(anims.starts[kAnimHelicopter].flip);
    heli.state = kHeliLands;
    uint16_t shadow = create_shadow(world, anims, tile, id, kAnimHelicopterShadow);
    world.things[id].shadow = shadow;
    return id;
}

// try_to_generate_emergency's helicopter for an emergency at state 5.
void try_to_generate_emergency_summon(World &world, const AnimData &anims)
{
    World::Emergency &e = world.emergency;
    e.heli = summon_vehicle(world, anims, uint16_t(e.heli_pad_tile + kMapSize));
    trace_event("heli %u", unsigned(e.heli));
    if (e.heli) {
        e.state = kEmergencyHeliSent;
        e.heli_landed = 0;
    } else {
        e.to_spawn = 0;
        e.state = 0;
    }
}

// helicopter_lands (0xafef4) for a helicopter.
void helicopter_lands(World &world, const AnimData &anims, uint16_t id)
{
    Thing &heli = world.things[id];
    heli.sprite = uint16_t(anims.frames[heli.sprite].next);
    if (heli.altitude > 0) {
        heli.altitude = int16_t(heli.altitude - heli.speed);
        return;
    }
    heli.altitude = 0;
    heli.state = kHeliWaits;
    trace_event("heli_landed %u", unsigned(id));
    world.emergency.heli_landed = 1;
}

// helicopter_leaves (0xb0000) for a helicopter.
void helicopter_leaves(World &world, const AnimData &anims, uint16_t id)
{
    Thing &heli = world.things[id];
    heli.sprite = uint16_t(anims.frames[heli.sprite].next);
    if (heli.altitude < kHeliAltitude) {
        heli.altitude = int16_t(heli.altitude + heli.speed);
        return;
    }
    trace_event("heli_gone %u", unsigned(id));
    world.emergency.heli_landed = 0;
    world.emergency.heli = 0;
    if (heli.shadow)
        remove_thing(world, heli.shadow);
    remove_thing(world, id);
}

// give_emergency_stats (0xaf510): offer `call` becomes hospital 0's
// emergency and its slot is freed. False, with nothing changed, if no
// used slot holds it.
bool give_emergency_stats(Disasters &d, World::Emergency &e, uint16_t call)
{
    for (Disasters::Offer &offer : d.offers) {
        if (offer.call != call || !offer.used)
            continue;
        e.illness = offer.illness;
        e.patients = offer.count;
        e.to_spawn = int8_t(e.patients);
        e.bonus = offer.bonus;
        e.win_percent = offer.win_percent;
        trace_event("emergency_accept %u illness=%u count=%u bonus=%u win=%u", unsigned(call), unsigned(e.illness),
                    unsigned(e.patients), unsigned(e.bonus), unsigned(e.win_percent));
        offer.used = 0;
        d.emergencies_active--;
        return true;
    }
    return false;
}

// process_turn's EV_START_EMERGENCY for hospital 0.
void start_emergency(World::Emergency &e)
{
    trace_event("emergency_start");
    e.state = kEmergencyAccepted;
    e.timer = 100;
    e.cured = 0;
}

// save_popup (0x184f4) of hospital 0's emergency offers: the head of the
// kind (World::first_call) taken off the panel into Disasters::popup_call;
// as get_playercall, only its kind is cleared, and the oldest left becomes
// the head.
void save_popup(World &world, Disasters &d)
{
    uint16_t &head = world.first_call[0][kCallEmergency];
    World::PlayerCall &call = world.calls[head];
    if (call.type == 0)
        return;
    d.popup_call = call.id;
    delete_panel_popup(world, head);
    call.type = 0;
    head = 0;
    get_new_popup_head(world, kCallEmergency, 0);
}

} // namespace

void setup_next_emergency(Disasters &d, World &world, const Balance &settings, const Date &date)
{
    uint32_t month = month_of(date);
    d.emergency_pending = false;
    for (size_t i = 0; i < d.emergency_used.size(); i++) {
        if (d.emergency_used[i])
            continue;
        d.emergency_used[i] = 1;
        const Balance::Emergency &e = settings.emergency_control[i];
        if (e.end_month > month && e.end_month > e.start_month) {
            uint32_t r = old_newrand(world.seed) % ((e.end_month + 1u - e.start_month) * kTurnsPerMonth);
            d.next_emergency_turn = r + e.start_month * kTurnsPerMonth;
            d.emergency_illness = e.illness;
            r = old_newrand(world.seed) % uint32_t(e.max + 1 - e.min);
            d.emergency_patients = uint8_t(r + e.min);
            d.emergency_win_percent = e.perc_win;
            d.emergency_bonus = e.bonus;
            d.emergency_pending = true;
            break;
        }
    }
}

void setup_next_quake(Disasters &d, World &world, const Balance &settings, const Date &date)
{
    uint32_t month = month_of(date);
    d.quake_pending = false;
    for (size_t i = 0; i < d.quake_used.size(); i++) {
        if (d.quake_used[i])
            continue;
        d.quake_used[i] = 1;
        const Balance::Quake &q = settings.quake_control[i];
        if (q.end_month > month && q.end_month > q.start_month) {
            uint32_t offset = old_newrand(world.seed) % ((q.end_month + 1u - q.start_month) * kTurnsPerMonth);
            d.next_quake_turn = offset + q.start_month * kTurnsPerMonth;
            d.quake_severity = uint8_t(q.severity);
            d.quake_pending = true;
            break;
        }
    }
}

void try_to_generate_emergency(Game &game)
{
    World &world = game.world;
    Disasters &d = game.disasters;
    if (world.emergency.state == kEmergencySummon)
        try_to_generate_emergency_summon(world, game.anims());
    if (check_emergency_criteria(d, world, game.settings(), game.open, game.turn_count)) {
        if (world.hospital && world.hospital->illnesses[d.emergency_illness % kIllnessCount].discovered == 1)
            setup_emergency_stats(d, make_a_playercall(world, kCallEmergency, 0, 0x14, 0));
        setup_next_emergency(d, world, game.settings(), game.date);
    }
    if (world.emergency.state > kEmergencySummon)
        world.emergency.state--;
}

void daily_admin_emergency(Game &game)
{
    World::Emergency &e = game.world.emergency;
    if (e.state != kEmergencyTimeUp || e.present != 0)
        return;
    e.state = kEmergencyOver;
    bool won = e.patients && e.cured * 100 / e.patients >= e.win_percent;
    int32_t bonus = won ? int32_t(e.bonus * e.cured) : 0;
    trace_event("emergency_over cured=%u patients=%u bonus=%d", unsigned(e.cured), unsigned(e.patients), int(bonus));
    if (won) {
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxEmergencyBonus, bonus});
        reputation_affected(game.world, 0, kRepEmergencySuccess);
    } else {
        reputation_affected(game.world, 0, kRepEmergencyFailed, e.patients - e.cured);
    }
    make_a_playercall(game.world, kCallEmergencyOver, 1, 0x14, 0);
}

void twice_daily_admin_emergency(World &world)
{
    World::Emergency &e = world.emergency;
    if (e.timer <= 0)
        return;
    e.timer--;
    trace_event("emergency_timer %d state=%d", int(e.timer), int(e.state));
    if (e.timer == 0 && e.state > 0)
        e.state--;
}

void create_a_peep_emergency(World &world, ArrivalState &state, const PeepSetup &setup)
{
    World::Emergency &e = world.emergency;
    if (e.to_spawn <= 0 || e.heli_landed != 1)
        return;
    if (uint16_t id = make_a_peep(world, state, setup, e.heli_pad_tile, e.illness, 0)) {
        trace_event("emergency_peep %u", unsigned(id));
        Thing &p = world.things[id];
        p.known_illness = p.illness;
        p.treatment = p.known_illness;
        if (shall_peep_pay(world, setup.anims, id))
            pay_for_treatment(world, id, 3);
        else
            p.treatment = 0;
        p.state = kPsGoForTreatment;
        p.speed = kPeepSpeed;
        p.anim_speed = kPeepAnimSpeed;
        p.person_flags |= kPfEmergency;
        set_icon_value(setup.anims, p, kIconEmergency);
        state.hospitals[0].visitors++;
        state.hospitals[0].month_visitors++;
    }
    if (--e.to_spawn == 0) {
        if (e.heli)
            world.things[e.heli].state = kHeliLeaves;
        e.heli_landed = 0;
    }
}

bool modify_person_emergency(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    if (!(p.person_flags & kPfEmergency))
        return false;
    if (world.emergency.state == kEmergencyTimeUp) {
        trace_event("emergency_dies %u", unsigned(id));
        change_health(world, anims, id, -1000, game_turn);
        p.person_flags &= ~kPfEmergency;
        reset_icon_value(p, kIconEmergency);
        return true;
    }
    set_icon_value(anims, p, kIconEmergency);
    if (p.health_delta)
        change_health(world, anims, id, 0, game_turn);
    return true;
}

void helicopter_turn(World &world, const AnimData &anims, uint16_t id)
{
    Thing &heli = world.things[id];
    if (heli.state == kHeliLands)
        helicopter_lands(world, anims, id);
    else if (heli.state == kHeliWaits)
        heli.sprite = uint16_t(anims.frames[heli.sprite].next);
    else if (heli.state == kHeliLeaves)
        helicopter_leaves(world, anims, id);
}

bool apply_emergency_event(Game &game, const Event &event)
{
    World &world = game.world;
    Disasters &d = game.disasters;
    switch (event.type) {
    case EV_ANSWER_PLAYERCALL:
        if (event.value == kCallEmergency) {
            if (check_playercall(world, kCallEmergency, 0, 0)) {
                save_popup(world, d);
                game.choice_screen = {kFaxEmergency, 0, 0};
            }
            return true;
        }
        if (event.value != kCallEmergencyOver)
            return false;
        {
            uint16_t subject = get_playercall(world, kCallEmergencyOver, 0, 0);
            if (subject == 1) {
                world.emergency.state = 0;
                game.choice_screen = {kFaxEmergencyOver, 0, 400};
            } else if (subject == 2) {
                world.mayor.state = 0;
                game.choice_screen = {kFaxVipResult, 0, 400};
            } else if (subject == 3) {
                epidemic_over_result(game);
            }
        }
        return true;
    case EV_CHOICE_RESULT:
        if (event.value == kChoiceEmergencyRefuse) {
            trace_event("emergency_refuse %u", unsigned(d.popup_call));
            return true;
        }
        if (event.value != kChoiceEmergencyAccept)
            return false;
        if (give_emergency_stats(d, world.emergency, d.popup_call))
            start_emergency(world.emergency);
        return true;
    case EV_START_EMERGENCY:
        start_emergency(world.emergency);
        return true;
    default:
        return false;
    }
}

} // namespace th
