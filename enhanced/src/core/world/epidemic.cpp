#include "core/world/epidemic.h"

#include "core/world/arrivals.h"
#include "core/world/complex_tables.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/mayor.h"
#include "core/world/nav.h"
#include "core/world/queue.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"
#include "core/world/vaccine.h"

namespace th
{

namespace
{

// allocate_thing's reserve for the inspector.
constexpr size_t kInspectorReserve = 0x19;

// The animation the inspector stands in (check_frame 0x89, + its facing).
constexpr uint16_t kAnimInspector = 0x89;

// The forced advice (ForcedAdvice section) given when an epidemic is over.
constexpr uint16_t kAdviceEpidemicOver = 9;

bool outdoors(const World &world, const Thing &t)
{
    return world.map[size_t(t.x.tile + (t.y.tile << 7))].owner_flags & kCellOutdoor;
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

// change_balance for hospital 0 (src/finance.cpp), at the game's current
// date.
void pay(World &world, uint8_t type, int32_t amount)
{
    if (!world.ledger || !world.balance || !world.date)
        return;
    change_balance(*world.ledger, *world.balance, world.date->day, world.date->month,
                   BalanceChange{type, amount, 0, 0});
}

// inspector_pauses' end: pays the compensation or fine, loses reputation
// and sends the contagious (or every) patient home, by the verdict reached.
void inspector_apply_verdict(World &world, uint8_t owner)
{
    World::Epidemic &e = world.epidemic;
    switch (e.inspection[0]) {
    case 0:
        pay(world, kTxGovernmentCompensation, e.fine);
        break;
    case 1:
        pay(world, kTxEpidemicCoverupFine, -e.fine);
        send_peeps_home(world, 0, owner);
        break;
    case 2:
        reputation_affected(world, 0, kRepEpidemicCoverUpFound);
        pay(world, kTxEpidemicCoverupFine, -e.fine);
        send_peeps_home(world, 0, owner);
        break;
    case 3:
        reputation_affected(world, 0, kRepEpidemicCoverUpFound);
        send_peeps_home(world, 1, owner);
        break;
    }
}

// inspector_pauses (0xa1ed4): after the pause, leaves and judges the
// epidemic by the contagious patients still inside, then ends the epidemic
// and raises the result fax.
void inspector_pauses(World &world, const AnimData &anims, uint16_t id)
{
    Thing &inspector = world.things[id];
    if (--inspector.timer != 0)
        return;
    World::Epidemic &e = world.epidemic;
    uint16_t entry = world.entrances[inspector.owner][old_newrand(world.seed) & 3];
    set_destination_coords(world, id, world.rooms[entry].door_tile);
    inspector.state = kMayorGoto;
    inspector.goto_state = kMayorGone;
    uint32_t contagious = count_and_flag_contagious_peeps(world, anims, e.illness, 2, inspector.owner);
    e.inspection[0] = inspector_verdict(world, contagious);
    e.fine = uint16_t(world.settings->gbv.epidemic_fine * contagious * 2);
    make_a_playercall(world, kCallEpidemicOver, 3, 0x14, inspector.owner);
    e.inspected_illness = e.illness;
    e.illness = 0;
    e.inspection[1] = 0;
    e.over = 0;
    if (e.inspection[0] == 0)
        e.fine = uint16_t(
            old_normal_rand(world.seed, world.settings->gbv.epidemic_comp_lo, world.settings->gbv.epidemic_comp_hi));
    inspector_apply_verdict(world, inspector.owner);
}

} // namespace

// inspector_pauses' verdict for `contagious` patients still inside: harsher
// the more there are, 0 (always when none, rarely otherwise) paying
// compensation; draws from world.seed as the game does.
uint8_t inspector_verdict(World &world, uint32_t contagious)
{
    if (contagious == 0)
        return 0;
    if (contagious <= 2)
        return old_newrand(world.seed, "inspector_pauses") % 5 == 0 ? 0 : 1;
    if (contagious <= 5) {
        if (old_newrand(world.seed, "inspector_pauses") % 20 == 0)
            return 0;
        return old_newrand(world.seed, "inspector_pauses") % 10 == 0 ? 1 : 2;
    }
    if (contagious < 10) {
        if (old_newrand(world.seed, "inspector_pauses") % 50 == 0)
            return 0;
        return old_newrand(world.seed, "inspector_pauses") % 25 == 0 ? 1 : 2;
    }
    return old_newrand(world.seed, "inspector_pauses") % 50 == 0 ? 0 : 3;
}

void alert_epidemic(World &world, uint8_t illness, uint8_t owner)
{
    if (world.game && world.game->fax_screen.open && world.game->fax_screen.kind == kFaxEmergency)
        return;
    if (owner != 0 || world.epidemic.illness != 0 || world.emergency.state != 0)
        return;
    get_playercall(world, 2, 0, owner);
    world.advisor_events |= kAdvisorEventEpidemic;
    world.epidemic.illness = illness;
    make_a_playercall(world, kCallEpidemic, 1, 0x14, owner);
}

// count_and_flag_contagious_peeps' handling of one of `owner`'s contagious
// patients: inside, flag it and count it (mode 1, curing the fed-up ones
// instead) or clean and count the flagged (mode 2); outside, it loses its
// contagion.
void flag_contagious_peep(World &world, const AnimData &anims, size_t i, uint8_t mode, uint32_t &count)
{
    Thing &peep = world.things[i];
    if (!outdoors(world, peep)) {
        if (mode == 1) {
            if (peep.person_flags & kPfFedUp) {
                peep.person_flags &= ~kPfContagious;
                peep.contagion = 0;
                return;
            }
            count++;
            peep.person_flags |= kPfContagious;
            set_icon_value(anims, peep, kIconContagious);
        } else if (mode == 2) {
            if (peep.person_flags & kPfContagious)
                count++;
            clean_contagious_peep(world, uint16_t(i));
        }
    } else {
        peep.person_flags &= ~kPfContagious;
        peep.contagion = 0;
    }
}

uint32_t count_and_flag_contagious_peeps(World &world, const AnimData &anims, uint8_t illness, uint8_t mode,
                                         uint8_t owner)
{
    uint32_t count = 0;
    for (size_t i = 1; i < world.things.size(); i++) {
        Thing &peep = world.things[i];
        if (peep.type != kThingPatient || peep.owner != owner)
            continue;
        if (!peep.contagion || peep.illness != illness)
            continue;
        flag_contagious_peep(world, anims, i, mode, count);
    }
    return count;
}

uint32_t epidemic_status(World &world, uint8_t owner)
{
    uint32_t over = 0;
    World::Epidemic &e = world.epidemic;
    if (e.active) {
        if (e.contagious_count) {
            e.timer--;
            if (e.timer <= 0)
                over = 1;
        } else {
            if (owner == 0)
                world.epidemic_sounds.push_back(kSfxEpidemicEnd);
            over = 1;
        }
    }
    if (over) {
        if (owner == 0 && world.game)
            world.game->advisor.forced_advice_id = kAdviceEpidemicOver;
        e.timer = 0;
        e.active = 0;
        e.over = 1;
    }
    return over;
}

void epidemic_over_result(Game &game)
{
    game.world.epidemic.over = 0;
    game.choice_screen = {kFaxEpidemicOver, game.world.epidemic.inspected_illness, 400};
}

void expire_epidemic_cover_up(World &world)
{
    trace_event("epidemic_cover_up");
    static const AnimData kNoAnims;
    const AnimData &anims = world.game ? world.game->anims() : kNoAnims;
    count_and_flag_contagious_peeps(world, anims, world.epidemic.illness, 1, 0);
    world.epidemic.active = 1;
    world.epidemic.timer = 100;
    world.epidemic_sounds.push_back(kSfxEpidemicStart);
}

void send_peeps_home(World &world, uint8_t all, uint8_t owner)
{
    for (Thing &peep : world.things) {
        if (peep.type != kThingPatient || peep.owner != owner)
            continue;
        if (outdoors(world, peep))
            continue;
        if (all || (peep.person_flags & kPfContagious)) {
            peep.person_flags &= ~kPfContagious;
            peep.contagion = 0;
            peep.person_flags |= kPfSentHomeByInspector;
        }
    }
}

void clean_contagious_peep(World &world, uint16_t id)
{
    Thing &peep = world.things[id];
    peep.person_flags &= ~kPfContagious;
    peep.contagion = 0;
    reset_icon_value(peep, kIconVaccineRequested);
    reset_icon_value(peep, kIconVaccineNurseSent);
    reset_icon_value(peep, kIconVaccinated);
    reset_icon_value(peep, kIconContagious);
    remove_person_from_vaccination_queue(world, id);
    if (world.epidemic.spread_count >= 1)
        world.epidemic.spread_count--;
}

uint16_t create_inspector(World &world, const AnimData &anims, uint8_t owner)
{
    uint16_t id = allocate_thing(world, kInspectorReserve);
    if (!id)
        return 0;
    Thing &inspector = world.things[id];
    inspector.type = kThingInspector;
    inspector.owner = owner;
    uint16_t entry = world.entrances[owner][old_newrand(world.seed) & 3];
    uint16_t pos = world.rooms[entry].door_tile;
    inspector.x.tile = uint8_t(pos % kMapSize);
    inspector.y.tile = uint8_t(pos / kMapSize);
    inspector.x.frac = 0x80;
    inspector.y.frac = 0x80;
    inspector.speed = 0x20;
    inspector.anim_speed = 1;
    inspector.facing = 1;
    insert_node(world, id, inspector.x, inspector.y);
    play_frame(inspector, anims, kAnimInspector);
    uint16_t desk = owner == 0 ? world.first_of_kind[11] : 0;
    if (desk) {
        set_destination_coords(world, id, world.things[desk].use_pos[1]);
        inspector.state = kMayorGoto;
        inspector.goto_state = kMayorInspects;
        inspector.inspect_room = 0;
    } else {
        entry = world.entrances[owner][old_newrand(world.seed) & 3];
        set_destination_coords(world, id, world.rooms[entry].door_tile);
        inspector.state = kMayorGoto;
        inspector.goto_state = kMayorGone;
    }
    return id;
}

void generate_inspector(World &world, const AnimData &anims, uint8_t owner)
{
    if (world.epidemic.inspection[1] != 0)
        return;
    create_inspector(world, anims, owner);
    world.epidemic.inspection[1] = 3;
}

void inspector_turn(Game &game, uint16_t id, uint32_t game_turn)
{
    World &world = game.world;
    const AnimData &anims = game.anims();
    switch (world.things[id].state) {
    case kMayorGoto:
        mayor_goto(game, id, game_turn);
        break;
    case kMayorPauses:
    case kMayorInspects:
        inspector_pauses(world, anims, id);
        break;
    case kMayorGone:
        mayor_gone(world, id);
        break;
    default:
        break;
    }
}

bool apply_epidemic_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_ANSWER_PLAYERCALL:
        if (event.value != kCallEpidemic)
            return false;
        get_playercall(world, kCallEpidemic, 0, 0);
        if (world.epidemic.contagious_count) {
            world.epidemic.fine = uint16_t(world.epidemic.contagious_count * world.settings->gbv.epidemic_fine);
            game.choice_screen = {kFaxEpidemic, 0, 0};
        }
        return true;
    case EV_CHOICE_RESULT:
        if (event.value == kChoiceEpidemicDeclare) {
            trace_event("epidemic_declare");
            count_and_flag_contagious_peeps(world, game.anims(), world.epidemic.illness, 2, 0);
            world.epidemic.illness = 0;
            world.epidemic.active = 0;
            pay(world, kTxEpidemicFine, -world.epidemic.fine);
            reputation_affected(world, 0, kRepEpidemicDeclared);
            world.epidemic_sounds.push_back(kSfxEpidemicEnd);
            return true;
        }
        if (event.value == kChoiceEpidemicCoverUp) {
            trace_event("epidemic_cover_up");
            count_and_flag_contagious_peeps(world, game.anims(), world.epidemic.illness, 1, 0);
            world.epidemic.active = 1;
            world.epidemic.timer = 100;
            world.epidemic_sounds.push_back(kSfxEpidemicStart);
            return true;
        }
        return false;
    case EV_TOGGLE_VACCINATION: {
        if (event.value == 0 || event.value >= world.things.size())
            return true;
        Thing &p = world.things[event.value];
        if (p.type != kThingPatient)
            return true;
        if (p.person_flags & kPfVaccineRequested) {
            world.epidemic_sounds.push_back(kSfxDeVaccin);
            cancel_vaccination_request(world, event.value);
            if (p.person_flags & kPfContagious)
                set_icon_value(game.anims(), p, kIconContagious);
        } else {
            world.epidemic_sounds.push_back(kSfxVaccin);
            request_vaccination(world, game.anims(), event.value);
        }
        return true;
    }
    default:
        return false;
    }
}

} // namespace th
