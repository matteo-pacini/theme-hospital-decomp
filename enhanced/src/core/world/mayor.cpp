#include "core/world/mayor.h"

#include "core/world/advisor.h"
#include "core/world/build.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// The object kind of a reception desk (OBJ_RECEPTION_DESK).
constexpr uint8_t kObjReceptionDesk = 11;

// Thing.state of a VIP (the game's MAYOR_*, include/enums.h).
constexpr uint8_t kMayorGoto = 2;
constexpr uint8_t kMayorPauses = 4;
constexpr uint8_t kMayorGone = 6;

// The animations a VIP walks and stands with (the game's 0x85 and 0x89).
constexpr uint16_t kMayorWalk = 0x85;
constexpr uint16_t kMayorStand = 0x89;

// The game's opposite_angle.
constexpr std::array<uint8_t, 4> kOppositeAngle = {2, 3, 0, 1};

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

} // namespace

// A VIP arriving at a stop: faces the reception desk (first stop) or the
// room's door and waits 50 turns (mayor_inspects, 0xa1af0). Shared with the
// health inspector, which has no rooms of interest.
void mayor_inspects(World &world, const AnimData &anims, uint16_t id)
{
    Thing &mayor = world.things[id];
    if (mayor.inspect_room == 0) {
        uint16_t desk = world.first_of_kind[kObjReceptionDesk];
        if (desk)
            mayor.facing = kOppositeAngle[world.things[desk].facing & 3];
    } else {
        uint16_t door = find_door(world, id);
        if (door)
            mayor.facing = kOppositeAngle[world.things[door].facing & 3];
    }
    play_frame(mayor, anims, kMayorStand);
    mayor.timer = 0x32;
}

namespace
{

// The staff of hospital 0 (allhosps[0].staff_count, count_staff).
uint16_t hospital_staff_count(const World &world)
{
    return uint16_t(count_staff(world).total);
}

// Litter over 20 scores the same as litter over 10: the > 20 test comes
// second (mayor_judges_hospital's litter).
int mayor_judges_hospital_litter(const World &world)
{
    if (world.litter_count > 10)
        return 1;
    if (world.litter_count > 20)
        return 2;
    return -1;
}

// mayor_judges_hospital's staff tiredness, from the statistics pass.
int mayor_judges_hospital_staff(const Balance::Gbv &gbv, const AdCounts &adcounts, uint16_t staff_count)
{
    if (staff_count <= 2)
        return 4;
    int avg = adcounts.staff_tiredness_sum / staff_count;
    if (gbv.very_tired < adcounts.max_staff_tiredness)
        return 2;
    if (gbv.tired < avg)
        return 1;
    return -1;
}

// mayor_judges_hospital's patients' warmth, from the statistics pass.
int mayor_judges_hospital_warmth(const AdCounts &adcounts)
{
    if (adcounts.patients <= 10)
        return 0;
    int avg = adcounts.warmth_sum / adcounts.patients;
    return (avg > 0x50 || avg < 0x14) ? 1 : -1;
}

// mayor_judges_hospital's doctor grades of hospital `owner`.
int mayor_judges_hospital_doctors(const World &world, uint8_t owner)
{
    int doctors = 0, juniors = 0, consultants = 0;
    for (const Thing &person : world.things) {
        if (person.type != kThingStaff || person.subtype != kStaffDoctor || person.owner != owner)
            continue;
        doctors++;
        if (person.skills & kSkillJunior)
            juniors++;
        else if (person.skills & kSkillConsultant)
            consultants++;
    }
    int score = 0;
    if (doctors / 2 < juniors)
        score += 2;
    if (doctors / 2 < consultants)
        score -= 2;
    if (doctors == 0)
        score += 4;
    return score;
}

// mayor_judges_hospital's rooms of hospital `owner`, from entry 1 (the
// game's room_info + 1; entry 0 is the unused room).
int mayor_judges_hospital_rooms(const World &world, uint8_t owner)
{
    int nrooms = 0;
    for (size_t i = 1; i < world.rooms.size(); i++) {
        const RoomInfo &room = world.rooms[i];
        if (room.type != kRoomFree && room.owner == owner && room.type != kRoomEntrance)
            nrooms++;
    }
    if (nrooms == 0)
        return 4;
    return nrooms < 3 ? 1 : 0;
}

// Scores hospital 0 for a VIP visit (mayor_judges_hospital, 0xa13a4): litter,
// staff tiredness, patients' warmth, doctor grades, rooms and a random part,
// returning a kMayorResult index 0-14.
uint8_t mayor_judges_hospital(Game &game, uint8_t owner)
{
    World &world = game.world;
    const Balance::Gbv &gbv = game.settings().gbv;
    AdCounts &adcounts = world.counts;
    uint16_t staff_count = hospital_staff_count(world);
    int score = mayor_judges_hospital_litter(world);
    score += mayor_judges_hospital_staff(gbv, adcounts, staff_count);
    score += mayor_judges_hospital_warmth(adcounts);
    score += mayor_judges_hospital_doctors(world, owner);
    score += mayor_judges_hospital_rooms(world, owner);
    score += 2 - int(old_normal_rand(world.seed, 0, 5));
    if (owner == 0) {
        game.progress.award_counts.mayor_score_total += score;
        game.progress.award_counts.mayor_visits++;
    }
    for (int i = 0; i < 15; i++)
        if (kMayorResult[i].max_score >= score)
            return uint8_t(i);
    return 14;
}

// VIP waiting state (mayor_pauses, 0xa11b8): when the timer runs out, walks
// to the next room of interest (some at random), or, after the last room,
// leaves and applies the verdict of mayor_judges_hospital.
void mayor_pauses(Game &game, uint16_t id)
{
    World &world = game.world;
    Thing &mayor = world.things[id];
    if (--mayor.timer != 0)
        return;
    for (;;) {
        mayor.inspect_room++;
        if (mayor.inspect_room >= kRoomCount)
            break;
        const RoomInfo &room = world.rooms[mayor.inspect_room];
        const RoomTypeInfo &type = kRoomTypeInfo[room.type % kRoomTypes];
        if (room.owner == mayor.owner && type.main_object) {
            if (type.door_kind == 0 || !(old_newrand(world.seed) & 1)) {
                set_destination_coords(world, id, room.door_tile);
                mayor.state = kMayorGoto;
                mayor.goto_state = kMayorPauses;
                return;
            }
        }
    }
    uint16_t tile = world.rooms[world.entrances[mayor.owner][old_newrand(world.seed) & 3]].door_tile;
    set_destination_coords(world, id, tile);
    mayor.state = kMayorGoto;
    mayor.goto_state = kMayorGone;
    world.mayor.verdict = mayor_judges_hospital(game, mayor.owner);
    int32_t value = kMayorResult[world.mayor.verdict].cash_bonus;
    if (value != 0)
        change_balance(game.finance, game.balance, game.date.day, game.date.month, {kTxVipAward, int32_t(value)});
    uint8_t rep = kMayorResult[world.mayor.verdict].reputation_event;
    if (rep != 0)
        reputation_affected(world, 0, RepEvent(rep), 0);
}

} // namespace

// MAYOR_GOTO (0x78640): walks the VIP towards its destination; on arrival it
// inspects and enters goto_state; a navigation failure makes it leave.
// Shared with the health inspector.
void mayor_goto(Game &game, uint16_t id, uint32_t game_turn)
{
    World &world = game.world;
    const AnimData &anims = game.anims();
    Thing &mayor = world.things[id];
    if (game_turn % mayor.anim_speed == 0)
        mayor.sprite = uint16_t(anims.frames[mayor.sprite].next);
    mayor.countdown--;
    if (mayor.countdown <= 0) {
        mayor.countdown = int16_t(0x100 / mayor.speed);
        int dir = person_head_for(world, id);
        if (dir == kNavArrived) {
            mayor_inspects(world, anims, id);
            mayor.state = mayor.goto_state;
            mayor.countdown = 0;
            return;
        }
        if (dir == kNavBlockedByPeople) {
            play_frame(mayor, anims, kMayorStand);
            mayor.countdown = 0;
            return;
        }
        if (dir == kNavWalledIn || dir == kNavNoPath) {
            mayor.state = kMayorGone;
            return;
        }
        if (mayor.facing != dir) {
            mayor.facing = uint8_t(dir);
            play_frame(mayor, anims, kMayorWalk);
        }
    }
    auto raw_x = uint16_t((mayor.x.tile << 8 | mayor.x.frac) + uint16_t(mayor.speed * kAroundXy[mayor.facing][0]));
    auto raw_y = uint16_t((mayor.y.tile << 8 | mayor.y.frac) + uint16_t(mayor.speed * kAroundXy[mayor.facing][1]));
    move_node(world, id, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
}

// A VIP or health inspector who has left the map (mayor_gone, 0xa1754):
// removes it; a VIP's visit is over and its report queued, an inspector is
// no longer on the way (World::Epidemic::inspection[1]).
void mayor_gone(World &world, uint16_t id)
{
    Thing &gone = world.things[id];
    if (gone.type == kThingMayor) {
        world.mayor.state = 4;
        make_a_playercall(world, kCallEmergencyOver, 2, 0, gone.owner);
    } else {
        world.epidemic.inspection[1] = 0;
    }
    remove_thing(world, id);
}

namespace
{

// create_mayor (0xa0f6c): the visiting VIP for hospital `owner` at one of its
// four entry points, walking to the reception desk, or straight back out with
// verdict 14 if there is none. Returns its thing id, 0 if no thing was free.
uint16_t create_mayor(Game &game, uint8_t owner)
{
    World &world = game.world;
    const AnimData &anims = game.anims();
    uint16_t id = allocate_thing(world, 0x19);
    if (!id)
        return 0;
    Thing &mayor = world.things[id];
    mayor.type = kThingMayor;
    mayor.owner = owner;
    uint16_t pos = world.rooms[world.entrances[owner][old_newrand(world.seed) & 3]].door_tile;
    mayor.x = {0x80, uint8_t(pos % kMapSize)};
    mayor.y = {0x80, uint8_t(pos / kMapSize)};
    mayor.speed = 0x10;
    mayor.anim_speed = 2;
    mayor.facing = 1;
    // The VIP's looks: the game leaves its layers as the thing slot had them;
    // the first element (an adult's) is drawn here.
    mayor.layers[0] = 1;
    insert_node(world, id, mayor.x, mayor.y);
    play_frame(mayor, anims, kMayorStand);
    uint16_t desk = world.first_of_kind[kObjReceptionDesk];
    if (desk) {
        set_destination_coords(world, id, world.things[desk].use_pos[1]);
        mayor.state = kMayorGoto;
        mayor.goto_state = kMayorPauses;
        mayor.inspect_room = 0;
    } else {
        uint16_t tile = world.rooms[world.entrances[owner][old_newrand(world.seed) & 3]].door_tile;
        set_destination_coords(world, id, tile);
        mayor.state = kMayorGoto;
        mayor.goto_state = kMayorGone;
        world.mayor.verdict = 0xe;
    }
    return id;
}

} // namespace

const std::array<MayorResult, 15> kMayorResult = {{
    {-5, kRepVipGood, 4000},
    {-4, kRepVipGood, 2000},
    {-3, kRepVipGood, 1500},
    {-2, kRepVipGood, 1200},
    {-1, kRepVipFair, 800},
    {0, kRepVipFair, 400},
    {1, kRepVipFair, 200},
    {2, kRepVipFair, 0},
    {3, kRepVipFair, 0},
    {4, kRepVipBad, 0},
    {5, kRepVipBad, 0},
    {6, kRepVipBad, 0},
    {7, kRepVipBad, 0},
    {8, kRepVipBad, 0},
    {30, kRepVipBad, 0},
}};

void generate_mayors(Game &game)
{
    World &world = game.world;
    if (world.mayor.state != 0) {
        if (world.mayor.state == 2 && game.turn_count > world.mayor.arrival_turn) {
            world.mayor.thing = create_mayor(game, 0);
            world.mayor.state = 3;
            game.advisor.forced_advice_id = 8;
            // play_ambient: the VIP's voice, for the first six names.
            if (world.mayor.name < 6)
                world.machine_announcements.push_back(uint8_t(world.mayor.name + 0xb7));
        }
        return;
    }
    if (world.first_of_kind[kObjReceptionDesk] == 0)
        return;
    world.mayor.launch_count++;
    if (world.mayor.launch_count < game.settings().gbv.mayor_launch)
        return;
    uint8_t idx = uint8_t(old_newrand(world.seed) % 11);
    world.mayor.name = idx;
    world.mayor.state = 1;
    world.mayor.launch_count = 0;
    make_a_playercall(world, kCallVip, idx, 0x14, 0);
}

void mayor_turn(Game &game, uint16_t id, uint32_t game_turn)
{
    switch (game.world.things[id].state) {
    case kMayorGoto:
        mayor_goto(game, id, game_turn);
        break;
    case kMayorPauses:
        mayor_pauses(game, id);
        break;
    case kMayorGone:
        mayor_gone(game.world, id);
        break;
    default:
        break;
    }
}

bool apply_mayor_event(Game &game, const Event &event)
{
    World &world = game.world;
    if (event.type == EV_ANSWER_PLAYERCALL && event.value == kCallVip) {
        uint16_t subject = get_playercall(world, kCallVip, 0, 0);
        game.choice_screen = {kFaxVip, subject, 0};
        return true;
    }
    if (event.type != EV_CHOICE_RESULT)
        return false;
    if (event.value == kChoiceVipAccept) {
        world.mayor.arrival_turn = game.turn_count + 1500;
        world.mayor.state = 2;
        return true;
    }
    if (event.value == kChoiceVipRefuse) {
        world.mayor.state = 0;
        return true;
    }
    return false;
}

} // namespace th
