#include "core/world/advisor.h"
#include "core/world/arrivals.h"
#include "core/world/build.h"
#include "core/world/computer.h"
#include "core/world/game.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"
#include "core/world/world.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace th
{

namespace
{

// The rooms' staff_kind (src/tables.cpp rooms_data): the staff kind the AI
// hires for a room, or -1.
constexpr std::array<int8_t, 31> kRoomStaffKind = {-1, -1, -1, 0, -1, -1, -1, 1, 1, 0,  1,  0, 1, 1,  1, 1,
                                                   1,  1,  1,  1, 1,  0,  1,  1, 1, -1, -1, 1, 1, -1, 1};

// The action types of ComputerStrategy::type.
constexpr uint16_t kActionDecide = 0;
constexpr uint16_t kActionEmploy = 1;
constexpr uint16_t kActionBuild = 2;
constexpr uint16_t kActionPlace = 3;
constexpr uint16_t kActionLoan = 4;

// Strategy script step types handled at once by computer_decision.
constexpr uint16_t kStepSwitchStrategy = 6;
constexpr uint16_t kStepChooseRoom = 7;
constexpr uint16_t kStepRecheck = 8;
constexpr uint16_t kStepOpen = 10;
constexpr uint16_t kStepStaffLevels = 11;
constexpr uint16_t kStepObjectLevels = 12;
constexpr uint16_t kStepBuyLand = 13;
constexpr uint16_t kStepMaintainLoan = 14;
constexpr uint16_t kStepRestart = 15;
constexpr uint16_t kStepSwitchStrategy2 = 16;
constexpr uint16_t kStepFreeBuild = 17;

// A pace of at least 1 and at most 100.
int clamp_pace(int pace)
{
    return std::clamp(pace, 1, 100);
}

// The pace the personality `ai_index` acts with: its speed less a draw from
// 0 to its skill, drawn as the original function `caller`.
int ai_pace(AiContext &ctx, const RivalHospital &rival, const char *caller)
{
    const Balance::Computer &ai = ctx.settings.computer[rival.ai_index];
    return clamp_pace(ai.speed - int(old_newrand(ctx.world.seed, caller) % ai.skill));
}

// The turn an action of `type` is done after, at `pace`.
uint32_t next_action(const AiContext &ctx, uint16_t type, int pace)
{
    return ctx.turn + uint32_t(kComDecide[type].delay * pace / 10);
}

// computer_action's cases 1 and 2: a hire or a build, then the note it came
// from is removed if it worked and moved to the back if not.
void computer_action_note(RivalHospital &rival, uint8_t owner, uint16_t type, bool done)
{
    if (rival.action_from_note) {
        if (done)
            remove_a_note(rival, owner, type);
        else
            declass_a_note(rival, owner, type);
    }
    rival.action_from_note = 0;
    rival.action.type = kActionDecide;
}

// computer_action's switch on the pending action's type.
void computer_action_run(AiContext &ctx, uint8_t owner, int pace)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    switch (rival.action.type) {
    case kActionDecide:
        computer_decision(ctx, owner);
        break;
    case kActionEmploy:
        computer_action_note(
            rival, owner, kActionEmploy,
            computer_employ(ctx.world, uint8_t(rival.action.arg), uint16_t(rival.action.count), owner));
        rival.next_action_turn = next_action(ctx, 0, pace);
        break;
    case kActionBuild:
        computer_action_note(rival, owner, kActionBuild,
                             computer_build(ctx.world, ctx.settings, uint8_t(rival.action.arg), owner));
        rival.next_action_turn = next_action(ctx, 0, pace);
        break;
    case kActionPlace:
        computer_place_object(ctx.world, ctx.settings, uint8_t(rival.action.arg), false, owner);
        rival.action.count--;
        if (rival.action.count <= 0)
            rival.action.type = kActionDecide;
        rival.next_action_turn = next_action(ctx, rival.action.type, pace);
        break;
    case kActionLoan:
        computer_loan(ctx, int8_t(rival.action.arg), owner);
        rival.action.type = kActionDecide;
        rival.next_action_turn = next_action(ctx, 0, pace);
        break;
    default:
        break;
    }
}

// The pending action of the script step `step`: set from the script entry,
// done after its type's delay.
void computer_decision_action(AiContext &ctx, RivalHospital &rival, const ComputerStrategy &step, int pace)
{
    rival.action = step;
    rival.next_action_turn = next_action(ctx, step.type, pace);
}

// computer_decision's steps done at once.
void computer_decision_now(AiContext &ctx, uint8_t owner, const ComputerStrategy &step)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    switch (step.type) {
    case kStepSwitchStrategy:
    case kStepSwitchStrategy2:
        rival.strategy = 2;
        rival.strategy_step = int16_t(find_strategy_start(2));
        break;
    case kStepChooseRoom:
        computer_choose_room(ctx, 0, owner);
        break;
    case kStepRecheck:
        computer_recheck_actions(ctx, uint8_t(step.arg), owner);
        break;
    case kStepOpen:
        ctx.arrivals.hospitals[owner].open = true;
        trace_event("rival_open %u", unsigned(owner));
        announce_this(ctx.world, kAnnRivalOpened, owner);
        break;
    case kStepStaffLevels:
        computer_staff_levels(ctx, owner);
        break;
    case kStepObjectLevels:
        computer_object_levels(ctx, owner);
        break;
    case kStepBuyLand:
        computer_buy_land(ctx, owner);
        break;
    case kStepMaintainLoan:
        computer_maintain_loan(ctx, owner);
        break;
    case kStepRestart:
        rival.strategy_step = int16_t(find_strategy_start(rival.strategy));
        break;
    case kStepFreeBuild:
        computer_free_build(ctx, owner);
        break;
    default:
        break;
    }
}

// computer_recheck_actions' first loop: counts the hires of the script up to
// `rival.recheck_step`, where the last recheck stopped.
void recheck_actions_resume(const RivalHospital &rival, std::array<int, 5> &counts, int &i)
{
    if (rival.recheck_step == 0)
        return;
    for (; i < rival.recheck_step; i++)
        if (kComStrategy[size_t(i)].type == kActionEmploy)
            counts[size_t(kComStrategy[size_t(i)].arg)]++;
}

// The first free note slot from the back (declass_a_note's), or -1.
int last_free_note(const RivalHospital &rival)
{
    for (int i = 7; i >= 0; i--)
        if (rival.notes[size_t(i)].type == 0)
            return i;
    return -1;
}

// A pending action of type `type` for owner's rival whose delay is
// `delay_type`'s, at a fresh pace drawn as `caller`.
void set_next_action(AiContext &ctx, RivalHospital &rival, uint16_t delay_type, const char *caller)
{
    int pace = ai_pace(ctx, rival, caller);
    rival.next_action_turn = next_action(ctx, delay_type, pace);
}

// computer_choose_room's loop over one order's rooms: the room to build, 0 for
// none; `failed` is set if a room that could be built is too dear.
uint8_t choose_room_scan(const AiContext &ctx, const RivalHospital &rival, uint8_t order, bool &failed)
{
    const std::array<uint8_t, 14> &orders = kComRoomOrders[order];
    for (int i = 1; i <= orders[0]; i++) {
        uint8_t room = orders[size_t(i)];
        if (rival.rooms[room] != 0 || !room_available(ctx.world, room))
            continue;
        if (rival.balance >= ctx.settings.rooms[room].cost && rival.balance > 0)
            return room;
        failed = true;
    }
    return 0;
}

// computer_free_build's room: noted for hiring the staff it needs. Returns 1
// for a room chosen.
uint8_t free_build_room(AiContext &ctx, uint8_t order, uint8_t owner)
{
    uint8_t room = computer_choose_room(ctx, order, owner);
    if (room == 0)
        return 0;
    int arg = kRoomStaffKind[room];
    if (arg >= 0)
        make_a_note(ctx.world.rivals[owner], owner, kActionEmploy, int16_t(arg), int16_t(kRoomDoctorSkills[room]));
    return 1;
}

} // namespace

uint8_t make_a_note(RivalHospital &rival, uint8_t owner, uint16_t type, int16_t arg, int16_t count)
{
    for (ComputerStrategy &note : rival.notes) {
        if (note.type == 0) {
            note = {type, arg, count};
            trace_event("computer_note %u op=make type=%u arg=%d count=%d", unsigned(owner), unsigned(note.type),
                        int(note.arg), int(note.count));
            return 1;
        }
    }
    return 0;
}

uint8_t find_a_note(RivalHospital &rival, uint16_t type)
{
    for (const ComputerStrategy &note : rival.notes) {
        if (note.type == type) {
            rival.action = note;
            rival.action_from_note = 1;
            return 1;
        }
    }
    return 0;
}

void remove_a_note(RivalHospital &rival, uint8_t owner, uint16_t type)
{
    for (ComputerStrategy &note : rival.notes) {
        if (note.type == type) {
            trace_event("computer_note %u op=remove type=%u arg=%d count=%d", unsigned(owner), unsigned(note.type),
                        int(note.arg), int(note.count));
            note.type = 0;
            break;
        }
    }
}

void declass_a_note(RivalHospital &rival, uint8_t owner, uint16_t type)
{
    find_a_note(rival, type);
    remove_a_note(rival, owner, type);
    int slot = last_free_note(rival);
    if (slot < 0)
        return;
    ComputerStrategy &note = rival.notes[size_t(slot)];
    note = rival.action;
    trace_event("computer_note %u op=declass type=%u arg=%d count=%d", unsigned(owner), unsigned(note.type),
                int(note.arg), int(note.count));
}

void computer_action(Game &game)
{
    if (game.advisor.flags & kAdvisorTutorial)
        return;
    AiContext ctx{game.world, game.arrivals, game.settings(), game.turn_count, game.finance.area};
    for (uint8_t owner = 1; owner < kHospitalCount; owner++) {
        RivalHospital &rival = ctx.world.rivals[owner];
        if (rival.player_type != 2 || rival.next_action_turn > ctx.turn)
            continue;
        int pace = ai_pace(ctx, rival, "computer_action");
        computer_action_run(ctx, owner, pace);
        trace_event("computer_action %u type=%u arg=%d count=%d next=%lu", unsigned(owner), unsigned(rival.action.type),
                    int(rival.action.arg), int(rival.action.count), static_cast<unsigned long>(rival.next_action_turn));
    }
}

void computer_decision(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    const Balance::Computer &ai = ctx.settings.computer[rival.ai_index];
    int16_t index = rival.strategy_step;
    rival.strategy_step++;
    // The step is within the script: the scripts loop before their end marker.
    int pace = ai.speed - int(old_newrand(ctx.world.seed, "computer_decision") % ai.skill);
    pace = clamp_pace(pace + int(old_newrand(ctx.world.seed, "computer_decision") % 5));
    const ComputerStrategy step = kComStrategy[size_t(index)];
    trace_event("computer_decision %u step=%d type=%u", unsigned(owner), int(index), unsigned(step.type));
    if (step.type <= kActionLoan)
        computer_decision_action(ctx, rival, step, pace);
    else
        computer_decision_now(ctx, owner, step);
}

uint8_t computer_recheck_actions(AiContext &ctx, uint8_t a, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    std::array<int, 5> counts{};
    int i = int(find_strategy_start(rival.strategy));
    recheck_actions_resume(rival, counts, i);
    for (; i < rival.strategy_step; i++) {
        const ComputerStrategy &step = kComStrategy[size_t(i)];
        if (step.type != kActionEmploy)
            continue;
        counts[size_t(step.arg)]++;
        if (rival.staff_by_kind[size_t(step.arg)] < counts[size_t(step.arg)]) {
            rival.action = step;
            rival.next_action_turn = ctx.turn;
            if (a == 1)
                rival.strategy_step--;
            else
                rival.recheck_step = int16_t(i + 1);
            break;
        }
    }
    if (a == 0 && i < rival.strategy_step)
        rival.strategy_step--;
    if (a == 1 || rival.strategy_step == i)
        rival.recheck_step = 0;
    return 0;
}

uint8_t computer_staff_levels(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    const Balance::Computer &ai = ctx.settings.computer[rival.ai_index];
    if (find_a_note(rival, kActionEmploy)) {
        set_next_action(ctx, rival, kActionEmploy, "computer_staff_levels");
        return 1;
    }
    for (int i = 0; i <= 3; i++) {
        int max = rival.patient_load / kStaffMaxNbs[size_t(i)][ai.staff_levels];
        if (rival.staff_by_kind[size_t(i)] < max) {
            rival.action.type = kActionEmploy;
            rival.action.arg = int16_t(i);
            set_next_action(ctx, rival, kActionEmploy, "computer_staff_levels");
            return 1;
        }
    }
    return 0;
}

uint8_t computer_choose_room(AiContext &ctx, uint8_t order, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    int pace = ai_pace(ctx, rival, "computer_choose_room");
    bool failed = false;
    uint8_t room = choose_room_scan(ctx, rival, order, failed);
    if (room != 0) {
        rival.action.type = kActionBuild;
        rival.action.arg = room;
        rival.next_action_turn = next_action(ctx, kActionBuild, pace);
        return room;
    }
    if (failed)
        make_a_note(rival, owner, kActionLoan, 0, 0);
    return 0;
}

uint8_t computer_free_build(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    if (find_a_note(rival, kActionBuild)) {
        set_next_action(ctx, rival, kActionBuild, "computer_free_build");
        return 1;
    }
    return free_build_room(ctx, 0, owner) || free_build_room(ctx, 1, owner);
}

uint8_t computer_maintain_loan(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    if (find_a_note(rival, kActionLoan)) {
        set_next_action(ctx, rival, kActionLoan, "computer_maintain_loan");
        rival.action_from_note = 0;
        remove_a_note(rival, owner, kActionLoan);
        return 1;
    }
    if (rival.loan != 0 && rival.balance > 6000) {
        int pace = ai_pace(ctx, rival, "computer_maintain_loan");
        rival.action.type = kActionLoan;
        rival.action.arg = -1;
        rival.next_action_turn = next_action(ctx, kActionLoan, pace);
        return 1;
    }
    return 0;
}

uint8_t computer_object_levels(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    const Balance::Computer &ai = ctx.settings.computer[rival.ai_index];
    for (const std::array<uint16_t, 11> &nbs : kObjectNbs) {
        uint16_t obj = nbs[0];
        int max = rival.patient_load / nbs[size_t(ai.comfort) + 1];
        int count = rival.object_stats[obj].count;
        if (count >= max)
            continue;
        int pace = ai_pace(ctx, rival, "computer_object_levels");
        rival.action.type = kActionPlace;
        rival.action.arg = int16_t(obj);
        rival.action.count = int16_t(std::clamp(max - count, 1, 4));
        rival.next_action_turn = next_action(ctx, kActionPlace, pace);
        return 1;
    }
    return 0;
}

uint8_t computer_loan(AiContext &ctx, int8_t amount, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    // Each branch's test fits the other's body: the repaying case borrows and
    // nothing caps the loan at max_loan (docs/BUGS.md, computer_loan).
    if (amount < 0 && rival.balance >= 5000 && rival.loan > 0) {
        rival.balance += 5000;
        rival.loan += 5000;
    } else if (amount > 0 && rival.max_loan - 5000 >= rival.loan) {
        rival.balance -= 5000;
        rival.loan -= 5000;
    } else {
        return 0;
    }
    trace_event("computer_loan %u amount=%d balance=%ld loan=%ld", unsigned(owner), int(amount), long(rival.balance),
                long(rival.loan));
    return 1;
}

uint8_t computer_buy_land(AiContext &ctx, uint8_t owner)
{
    RivalHospital &rival = ctx.world.rivals[owner];
    if (ctx.player_area <= rival.area)
        return 0;
    int n = ctx.player_area - rival.area;
    int cost = ctx.settings.gbv.land_cost_per_tile * n;
    if (cost <= rival.balance && rival.balance > 0) {
        rival.balance -= cost;
        rival.area = uint16_t(rival.area + n);
        trace_event("computer_land %u area=%u balance=%ld", unsigned(owner), unsigned(rival.area), long(rival.balance));
        return 1;
    }
    make_a_note(rival, owner, kActionLoan, 0, 0);
    return 0;
}

} // namespace th
