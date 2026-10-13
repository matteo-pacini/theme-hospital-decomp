// The player's commands on staff: placing, picking up, orders, sacking, pay
// demands and the handyman's priorities (src/staff.cpp).

#include "core/world/staff.h"

#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// SFX_PLAC_ST2 (include/enums.h): a member of staff put down.
constexpr uint8_t kSfxPlaceStaff = 0x07;
} // namespace

void adjust_handyman_values(World &world, uint16_t id, uint8_t button)
{
    Thing &h = world.things[id];
    uint8_t *raised = nullptr, *other1 = nullptr, *other2 = nullptr;
    if (button == kHandymanLitterButton) {
        raised = &h.prio_litter, other1 = &h.prio_plants, other2 = &h.prio_repair;
    } else if (button == kHandymanPlantsButton) {
        raised = &h.prio_plants, other1 = &h.prio_litter, other2 = &h.prio_repair;
    } else if (button == kHandymanRepairButton) {
        raised = &h.prio_repair, other1 = &h.prio_plants, other2 = &h.prio_litter;
    }
    if (raised) {
        uint8_t r = *raised, o1 = *other1, o2 = *other2;
        if (r < 94) {
            r += 6;
            while (r + o1 + o2 > 100) {
                if (o1 > o2)
                    o1--;
                else
                    o2--;
            }
        } else {
            r = 100;
            o1 = 0;
            o2 = 0;
        }
        *raised = r, *other1 = o1, *other2 = o2;
    }
    trace_event("handyman_values %u litter=%u plants=%u repair=%u", unsigned(id), unsigned(h.prio_litter),
                unsigned(h.prio_plants), unsigned(h.prio_repair));
    h.litter_count = 0;
    h.plant_count = 0;
    h.repair_count = 0;
}

bool put_back_held_staff(Game &game)
{
    World &world = game.world;
    if (!world.held_staff)
        return false;
    place_staff_on_map(world, {game.anims(), game.settings(), game.turn_count}, world.held_staff_tile, world.held_staff,
                       true);
    return true;
}

namespace
{

// apply_staff_event's EV_MARK_EMPLOYING: req2_mark_employing (the advisor's
// staff happiness total is not ported) and set_mouse_mode(MM_PLACE_STAFF),
// which leaves a room-edit wait.
void apply_staff_event_mark_employing(World &world, const Event &event)
{
    if (!event.arg || event.arg >= world.things.size())
        return;
    if (world.things[event.arg].state != kPsWaitsForDecision)
        world.building.mouse = BuildMouse::None;
    world.things[event.arg].state = kStaffHeld;
}

// apply_staff_event's EV_PLACE_STAFF: member of staff event.arg placed on
// cell event.value, a new hire paid for.
void apply_staff_event_place(Game &game, const Event &event)
{
    World &world = game.world;
    const Thing &s = world.things[event.arg];
    // A picked-up member of staff is placed again for nothing.
    bool hiring = world.held_staff == 0;
    if (place_staff_on_map(world, {game.anims(), game.settings(), game.turn_count}, event.value, event.arg, false)) {
        world.building.sounds.push_back(kSfxPlaceStaff);
        // set_mouse_mode(MM_NORMAL): a room waiting to be emptied for
        // editing is then cancelled by the end of the turn's check.
        world.building.mouse = BuildMouse::None;
        if (hiring)
            change_balance(game.finance, game.balance, game.date.day, game.date.month,
                           {kTxHireStaff, -(s.salary * 12) / 10});
    }
}

// apply_staff_event's EV_PICK_UP_STAFF: member of staff event.value held
// and, unless resigning or behind a surgeon's screen, picked up.
void apply_staff_event_pick_up(Game &game, const Event &event)
{
    World &world = game.world;
    // Held even when the pick-up is refused below, as in the game.
    world.held_staff = event.value;
    const Ctx c = ctx_of(world, game.anims(), game.turn_count);
    if (!(world.things[event.value].orders & kOrderResign) && surgeon_screen_free(world, event.value)) {
        interrupt_staff_in(c, event.value);
        staff_picked_up(c, event.value);
    }
}

// Whether an event's argument `index` names a thing (not the first, unused
// one).
bool is_staff_event_thing(const World &world, uint16_t index)
{
    return index && index < world.things.size();
}

// apply_staff_event's orders: member of staff event.value's low orders
// replaced by the one event.type gives.
void apply_staff_event_order(World &world, const Event &event)
{
    uint8_t order = event.type == EV_ORDER_REST         ? kOrderRest
                    : event.type == EV_ORDER_TRAIN      ? kOrderTrain
                    : event.type == EV_ORDER_LEAVE_ROOM ? kOrderLeaveRoom
                                                        : 2;
    Thing &s = world.things[event.value];
    s.orders = uint8_t((s.orders & 0xf0) | order);
}

// apply_staff_event's answers to a member of staff's demand (event.value):
// refused, they may leave (the more often they asked, the likelier);
// else, given a raise or a bonus of event.arg, they are happy and stay.
void apply_staff_event_demand(Game &game, const Event &event)
{
    World &world = game.world;
    Thing &s = world.things[event.value];
    if (event.type == EV_REFUSE_STAFF_DEMAND &&
        old_newrand(world.seed, "process_turn") % uint32_t(s.resign_count + 1)) {
        sack_staff(world, game.anims(), event.value, 1, game.turn_count);
        return;
    }
    if (event.type == EV_RAISE_STAFF_SALARY) {
        s.salary += event.arg;
        s.happiness = 100;
    } else if (event.type == EV_GIVE_STAFF_BONUS) {
        // pay_staff_bonus (0x2b998) for one member of staff; happiness,
        // clamped, is then set to 100 anyway.
        change_balance(game.finance, game.balance, game.date.day, game.date.month,
                       {kTxPersonalBonus, -int32_t(event.arg), event.value});
        s.happiness = 100;
    }
    s.orders &= uint8_t(~kOrderResign);
    game.advisor.event_flags &= ~kAdvisorEventResign;
    s.discontent = 0;
    reset_icon_value(s, kIconLeaving);
}
} // namespace

bool apply_staff_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_MARK_EMPLOYING:
        apply_staff_event_mark_employing(world, event);
        return true;
    case EV_PLACE_STAFF:
        if (is_staff_event_thing(world, event.arg))
            apply_staff_event_place(game, event);
        return true;
    case EV_PICK_UP_STAFF:
        if (is_staff_event_thing(world, event.value))
            apply_staff_event_pick_up(game, event);
        return true;
    case EV_STAFF_PICKED_UP:
        if (is_staff_event_thing(world, event.value))
            staff_picked_up(ctx_of(world, game.anims(), game.turn_count), event.value);
        return true;
    case EV_CANCEL_BUILD:
        // req2_exit_employing.
        if (!put_back_held_staff(game) && is_staff_event_thing(world, event.value))
            world.things[event.value].state = kStaffApplicant;
        return false;
    case EV_ORDER_REST:
    case EV_ORDER_TRAIN:
    case EV_ORDER_LEAVE_ROOM:
    case kEvOrder2:
        if (event.value < world.things.size())
            apply_staff_event_order(world, event);
        return true;
    case EV_SACK_STAFF:
        if (is_staff_event_thing(world, event.value))
            sack_staff(world, game.anims(), event.value, uint8_t(event.arg), game.turn_count);
        world.building.mouse = BuildMouse::None;
        return true;
    case EV_ADJUST_HANDYMAN_VALUES:
        // The game's argb[0].
        if (event.value < world.things.size())
            adjust_handyman_values(world, event.value, uint8_t(event.arg));
        return true;
    case EV_ANSWER_PLAYERCALL:
        if (event.value != kCallStaffResigns)
            return false;
        get_playercall(world, kCallStaffResigns, event.arg, kPlayer);
        return true;
    case EV_REFUSE_STAFF_DEMAND:
    case EV_RAISE_STAFF_SALARY:
    case EV_GIVE_STAFF_BONUS:
        if (is_staff_event_thing(world, event.value))
            apply_staff_event_demand(game, event);
        return true;
    default:
        return false;
    }
}

} // namespace th
