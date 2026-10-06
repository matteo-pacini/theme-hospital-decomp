#include "core/world/reaper.h"

#include "core/data/balance.h"
#include "core/ui/charts.h"
#include "core/world/build.h"
#include "core/world/complex.h"
#include "core/world/complex_tables.h"
#include "core/world/finance.h"
#include "core/world/game.h"
#include "core/world/nav.h"
#include "core/world/objects.h"
#include "core/world/queue.h"
#include "core/world/reputation.h"
#include "core/world/rooms.h"
#include "core/world/trace.h"

namespace th
{

namespace
{

// The complexes of dying (P_COMP.LEX): walking to where the patient falls,
// its death (an alien's), floating away, walking into the gates of hell.
constexpr uint16_t kComplexDying = 0x6e;
constexpr uint16_t kComplexDeath = 0x140;
constexpr uint16_t kComplexAlienDeath = 0x280;
constexpr uint16_t kComplexFlyAway = 0x1cc;
constexpr uint16_t kComplexIntoHell = 0x136;

// person_dead's substates once fallen: floating away, the reaper called,
// the reaper coming.
constexpr uint8_t kDeadFlyAway = 0x2f;
constexpr uint8_t kDeadReaper = 0x30;
constexpr uint8_t kDeadReaperComing = 0x63;

// The patient kind of an alien (PK_ALIEN), and the kPatientAnimStarts
// column (the game's reaper_test_anim) whose entry 0x4c1 marks the kinds
// the reaper may take.
constexpr uint8_t kPatientAlien = 0xe;
constexpr size_t kReaperTestColumn = 0x51;
constexpr uint16_t kReaperTestAnim = 0x4c1;

// The player calls dropped when a patient dies (kinds 8 and 4).
constexpr uint16_t kCallNoDiagnosis = 8;
constexpr uint16_t kCallNoTreatment = 4;

// The reaper's animations (AnimData::starts, plus his facing): arriving,
// walking, calling and waiting, swinging, leaving; the gates rising and
// standing, and sinking.
constexpr uint16_t kAnimReaperArrives = 0x33d;
constexpr uint16_t kAnimReaperWalks = 0x1f1;
constexpr uint16_t kAnimReaperWaits = 0x1f5;
constexpr uint16_t kAnimReaperSwings = 0x343;
constexpr uint16_t kAnimReaperLeaves = 0x347;
constexpr uint16_t kAnimGatesStand = 0x4fa;
constexpr uint16_t kAnimGatesSink = 0x4fc;

// The game's dying_place, gate_arrive and reap_arrive (src/tables.cpp):
// the count, then cell offsets. dying_place's last entry (0, the cell
// itself) is past its count but read, as in the game.
constexpr std::array<int16_t, 9> kDyingPlace = {8, -384, -383, -385, -256, -257, -255, -128, 0};
constexpr std::array<int16_t, 8> kGateArrive = {7, 384, 385, 383, 256, 255, 257, 128};
constexpr std::array<int16_t, 8> kReapArrive = {7, 381, 253, 125, 254, 126, -1, 128};

const PersonComplexes &complexes_of(const World &world)
{
    static const PersonComplexes none;
    return world.person_complexes ? *world.person_complexes : none;
}

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// can_i_get_to (0x19918): a path leads from `mover`'s cell to `tile`.
bool can_i_get_to(const World &world, const Thing &mover, uint16_t tile)
{
    OrthonavPath path;
    orthonav_init(world, path, mover.x.tile, mover.y.tile, tile % kMapSize, tile / kMapSize);
    return path.count != 0;
}

// The map cell at `tile`, or an empty one off the map.
const MapCell &cell_at(const World &world, uint32_t tile)
{
    static const MapCell none;
    return tile < uint32_t(kMapCells) ? world.map[tile] : none;
}

// start[anim + facing] as the game's PLAY_FRAME sets it, into `sprite` and
// `flip`.
void start_frame(const AnimData &anims, uint16_t anim, uint8_t facing, uint16_t &sprite, uint8_t &flip)
{
    size_t start = size_t(anim) + facing;
    if (start >= anims.starts.size())
        return;
    sprite = uint16_t(anims.starts[start].frame);
    flip = anims.starts[start].flip ? 1 : 0;
}

void process(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    process_person_complex(world, anims, complexes_of(world), id, game_turn);
}

// allocate_thing (0x1b42c) with `reserve` things kept back at the end.
uint16_t allocate_thing(const World &world, size_t reserve)
{
    for (size_t i = 1; i + reserve < world.things.size(); i++)
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    return 0;
}

// place_object (src/objects.cpp) for the gates of hell of hospital
// `owner`: in the corridor they must leave the rooms reachable
// (check_room_access), and no one may be in the way (no_peeps_in_way);
// their price, if any, is charged to hospital 0.
uint16_t place_gates(World &world, const AnimData &anims, uint16_t tile, uint8_t owner)
{
    if (!room_at(world, tile)) {
        mark_object_shape(world, tile, kObjGatesOfHell, 0);
        bool blocked = !check_room_access(world, owner);
        remove_object_shape(world, tile, kObjGatesOfHell, 0);
        if (blocked)
            return 0;
    }
    if (!no_peeps_in_way(world, tile, kObjGatesOfHell, 0))
        return 0;
    if (!allocate_thing(world, 0))
        return 0;
    int32_t cost = world.settings ? world.settings->objects[kObjGatesOfHell].start_cost : 0;
    if (cost && owner == 0 && world.ledger && world.balance && world.date)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, {kTxBuyObject, -cost});
    return place_object(world, anims, {tile, kObjGatesOfHell, 0}, owner, true);
}

// reaper_leaves_normally (0x5cb34): his leaving, then REAPER_REMOVE.
void reaper_leaves(const AnimData &anims, Thing &reaper)
{
    reaper.facing = 0;
    start_frame(anims, kAnimReaperLeaves, reaper.facing, reaper.sprite, reaper.flip);
    reaper.state = kReaperPlayAnim;
    reaper.reaper_next_state = kReaperRemove;
    reaper.next_sprite = 0;
    reaper.next_flip = 0;
}

// reaper_calls_victim (0x5c7b4): at the gates he turns to wait
// (REAPER_WAITS) and his victim walks into them (complex 310 to their use
// cell 1, PS_AT_HELL_GATE once there).
void reaper_calls_victim(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &reaper = world.things[id];
    reaper.facing = 0;
    start_frame(anims, kAnimReaperWaits, reaper.facing, reaper.sprite, reaper.flip);
    reaper.state = kReaperWaits;
    if (!reaper.slave || !reaper.gate)
        return;
    Thing &victim = world.things[reaper.slave];
    if (set_complex(world, reaper.slave, kComplexIntoHell, world.things[reaper.gate].use_pos[1])) {
        victim.depth = 0;
        victim.object = reaper.gate;
        victim.next_state = kPsAtHellGate;
        process(world, anims, reaper.slave, game_turn);
    }
}

// reaper_play_anim (0x5c6f8).
void reaper_play_anim(const AnimData &anims, Thing &reaper, uint32_t game_turn)
{
    if (game_turn % reaper.anim_speed != 0)
        return;
    reaper.sprite = anims.frames[reaper.sprite].next;
    if (!(anims.frames[reaper.sprite].flags & 1))
        return;
    if (reaper.reaper_next_state) {
        reaper.state = reaper.reaper_next_state;
        if (reaper.next_sprite) {
            reaper.sprite = reaper.next_sprite;
            reaper.flip = reaper.next_flip;
        }
    } else {
        reaper_leaves(anims, reaper);
    }
}

// reaper_waits (0x5c8e0): at each end of his animation, the victim gone
// into the gates is removed and he leaves; the victim at the gates gets a
// swing; else he leaves once his timer (1500 at his arrival, one off each
// time) runs out or the victim is not coming.
void reaper_waits(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &reaper = world.things[id];
    if (game_turn % reaper.anim_speed != 0)
        return;
    reaper.sprite = anims.frames[reaper.sprite].next;
    if (!(anims.frames[reaper.sprite].flags & 1))
        return;
    if (!reaper.slave) {
        reaper_leaves(anims, reaper);
        return;
    }
    Thing &victim = world.things[reaper.slave];
    if (victim.state == kPsAtHellGate) {
        reaper_leaves(anims, reaper);
        remove_person(world, anims, complexes_of(world), reaper.slave, game_turn);
        return;
    }
    if (!reaper.gate)
        return;
    if (cell_of(victim) == world.things[reaper.gate].use_pos[1]) {
        start_frame(anims, kAnimReaperSwings, reaper.facing, reaper.sprite, reaper.flip);
        reaper.state = kReaperPlayAnim;
        reaper.reaper_next_state = kReaperWaits;
        start_frame(anims, kAnimReaperWaits, reaper.facing, reaper.next_sprite, reaper.next_flip);
    } else if (--reaper.timer <= 0 || victim.next_state != kPsAtHellGate) {
        reaper_leaves(anims, reaper);
    }
}

// remove_reaper (0x5cbe4).
void remove_reaper(World &world, const AnimData &anims, uint16_t id)
{
    Thing &reaper = world.things[id];
    if (reaper.gate) {
        Thing &gate = world.things[reaper.gate];
        gate.state = kObjectHell;
        start_frame(anims, kAnimGatesSink, 0, gate.aux_sprite, gate.aux_flip);
        gate.remove_after_anim = 1;
    }
    trace_event("remove_thing %u type=%u", unsigned(id), unsigned(reaper.type));
    remove_node(world, id);
    reaper = Thing{};
}

// reaper_goto_hell (0x7724c).
void reaper_goto_hell(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &reaper = world.things[id];
    if (game_turn % reaper.anim_speed == 0)
        reaper.sprite = anims.frames[reaper.sprite].next;
    if (--reaper.countdown <= 0) {
        reaper.countdown = int16_t(0x100 / reaper.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
            reaper_calls_victim(world, anims, id, game_turn);
            reaper.countdown = 0;
            return;
        case kNavBlockedByPeople:
            start_frame(anims, kAnimReaperWaits, reaper.facing, reaper.sprite, reaper.flip);
            reaper.countdown = 0;
            return;
        case kNavWalledIn:
        case kNavNoPath:
            person_nav_failure(world, anims, complexes_of(world), id, game_turn);
            return;
        default:
            if (reaper.facing != dir) {
                reaper.facing = uint8_t(dir);
                start_frame(anims, kAnimReaperWalks, reaper.facing, reaper.sprite, reaper.flip);
            }
            break;
        }
    }
    uint16_t raw_x = uint16_t((reaper.x.tile << 8 | reaper.x.frac) + reaper.speed * kAroundXy[reaper.facing][0]);
    uint16_t raw_y = uint16_t((reaper.y.tile << 8 | reaper.y.frac) + kAroundXy[reaper.facing][1] * reaper.speed);
    move_node(world, id, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
}

} // namespace

void person_dies(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    trace_event("dies %u", unsigned(id));
    Thing &p = world.things[id];
    leave_queue(world, id);
    uint16_t tile = cell_of(p);
    uint16_t pos = 0;
    for (size_t i = 1; i <= size_t(kDyingPlace[0]); i++) {
        pos = uint16_t(tile + kDyingPlace[i]);
        if (cell_at(world, pos).first == 0 && can_i_get_to(world, p, pos)) {
            const MapCell &right = cell_at(world, pos + 1u);
            if (right.first == 0 && right.wall_w == 0)
                break;
            const MapCell &down = cell_at(world, pos + 0x80u);
            if (down.first == 0 && down.wall_n == 0)
                break;
        }
    }
    p.next_state = kPsDead;
    p.substate = 0;
    p.complex_pc = 0;
    p.complex_queued = 0;
    p.complex_queued2 = 0;
    if (set_complex(world, id, kComplexDying, pos))
        process(world, anims, id, game_turn);
    else
        p.state = kPsDead;
    if (p.owner == 0) {
        if (world.hospital)
            world.hospital->illnesses[p.illness % kIllnessCount].deaths++;
        if (world.ledger)
            world.ledger->deaths++;
    }
    reputation_affected(world, p.illness, kRepDied);
    record_person_as_patient(world, id);
    if (p.owner == 0 && world.ledger)
        world.ledger->deaths_this_month++;
    p.person_flags |= kPfDying;
    if (p.playercall) {
        get_playercall(world, kCallNoDiagnosis, p.playercall, p.owner);
        get_playercall(world, kCallNoTreatment, p.playercall, p.owner);
        p.playercall = 0;
    }
}

namespace
{

// person_dead's first step: the patient falls facing open floor and starts
// the death animation.
void person_dead_falls(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn,
                       std::source_location caller)
{
    Thing &p = world.things[id];
    uint16_t tile = cell_of(p);
    const MapCell &right = cell_at(world, tile + 1u);
    const MapCell &down = cell_at(world, tile + 0x80u);
    if ((old_newrand(world.seed, caller) & 1) == 0)
        p.facing = right.first == 0 && right.wall_w == 0 ? 0 : 1;
    else
        p.facing = down.first == 0 && down.wall_n == 0 ? 1 : 0;
    p.next_state = kPsDead;
    p.depth = -8;
    if (kPatientAnimStarts[p.subtype % kPatientAnimStarts.size()][kReaperTestColumn] == kReaperTestAnim)
        p.substate = uint8_t((old_newrand(world.seed, caller) & 1) + kDeadFlyAway);
    else
        p.substate = kDeadFlyAway;
    if (p.subtype == kPatientAlien) {
        p.subtype = p.orig_subtype;
        if (set_complex(world, id, kComplexAlienDeath, 0))
            process(world, anims, id, game_turn);
    } else if (set_complex(world, id, kComplexDeath, 0)) {
        process(world, anims, id, game_turn);
    }
}

// person_dead's kDeadFlyAway: the patient starts flying up.
void person_dead_flies_away(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    if (p.layers[0] > 5)
        p.layers[0] = 1;
    p.next_state = kPsFlys;
    p.countdown = 0;
    p.speed = 8;
    if (set_complex(world, id, kComplexFlyAway, 0))
        process(world, anims, id, game_turn);
}

// person_dead's kDeadReaper: summons the reaper, or flies away when he
// cannot come.
void person_dead_summons_reaper(World &world, const AnimData &anims, uint16_t id)
{
    Thing &p = world.things[id];
    if (summon_the_reaper(world, anims, id)) {
        cure_illness(world, id);
        p.substate = kDeadReaperComing;
        p.next_state = kPsDead;
    } else {
        p.substate = kDeadFlyAway;
    }
}

} // namespace

void person_dead(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    trace_event("dead %u substate=%u", unsigned(id), unsigned(world.things[id].substate));
    const Thing &p = world.things[id];
    if (p.substate < 0x1e)
        person_dead_falls(world, anims, id, game_turn, std::source_location::current());
    else if (p.substate == kDeadFlyAway)
        person_dead_flies_away(world, anims, id, game_turn);
    else if (p.substate == kDeadReaper)
        person_dead_summons_reaper(world, anims, id);
}

void person_flys(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &p = world.things[id];
    if (game_turn % p.anim_speed == 0)
        p.sprite = anims.frames[p.sprite].next;
    if (p.countdown < 400)
        p.countdown = int16_t(p.countdown + p.speed);
    else
        remove_person(world, anims, complexes_of(world), id, game_turn);
}

namespace
{

// summon_the_reaper's gates: places the gates of hell on the first spot
// around the victim that both can reach. Returns the gates (0 for none) and
// their tile in `gate_pos`.
uint16_t summon_the_reaper_place_gates(World &world, const AnimData &anims, const Thing &victim, uint16_t &gate_pos)
{
    uint16_t victim_pos = cell_of(victim);
    for (size_t i = 1; i <= size_t(kGateArrive[0]); i++) {
        gate_pos = uint16_t(victim_pos + kGateArrive[i]);
        if (!can_i_get_to(world, victim, gate_pos) ||
            !can_place_object(world, gate_pos, kObjGatesOfHell, 0, victim.owner) ||
            !corridor_object_nav_check(world, gate_pos, kObjGatesOfHell, 0))
            continue;
        uint16_t gate_id = place_gates(world, anims, gate_pos, victim.owner);
        if (!gate_id)
            continue;
        Thing &gate = world.things[gate_id];
        start_frame(anims, kAnimGatesStand, 0, gate.aux_sprite, gate.aux_flip);
        if (can_i_get_to(world, victim, gate.use_pos[1]))
            return gate_id;
        remove_object(world, gate_id, gate.owner);
    }
    return 0;
}

// summon_the_reaper's arrival: moves the reaper to the first tile around the
// gates from which he can reach them. False when there is none.
bool summon_the_reaper_find_arrival(World &world, Thing &reaper, uint16_t gate_pos, const Thing &gate,
                                    uint16_t &reaper_pos)
{
    for (size_t i = 1; i <= size_t(kReapArrive[0]); i++) {
        reaper_pos = uint16_t(gate_pos + kReapArrive[i]);
        reaper.x.tile = uint8_t(reaper_pos % kMapSize);
        reaper.y.tile = uint8_t(reaper_pos / kMapSize);
        if (can_i_get_to(world, reaper, gate.use_pos[0]))
            return true;
    }
    return false;
}

} // namespace

uint8_t summon_the_reaper(World &world, const AnimData &anims, uint16_t victim_id)
{
    trace_event("summon_reaper %u", unsigned(victim_id));
    Thing &victim = world.things[victim_id];
    if (victim.slave && world.things[victim.slave].type == kThingReaper)
        return 0;
    uint16_t id = allocate_thing(world, 0x32);
    if (!id)
        return 1;
    Thing &reaper = world.things[id];
    reaper.type = kThingReaper;
    uint16_t gate_pos = 0;
    uint16_t gate_id = summon_the_reaper_place_gates(world, anims, victim, gate_pos);
    if (!gate_id) {
        reaper = Thing{};
        return 0;
    }
    const Thing &gate = world.things[gate_id];
    uint16_t reaper_pos = 0;
    if (!summon_the_reaper_find_arrival(world, reaper, gate_pos, gate, reaper_pos)) {
        remove_object(world, gate_id, gate.owner);
        reaper = Thing{};
        return 0;
    }
    reaper.gate = gate_id;
    reaper.x = {0x80, uint8_t(reaper_pos % kMapSize)};
    reaper.y = {0x80, uint8_t(reaper_pos / kMapSize)};
    reaper.speed = 0x10;
    reaper.anim_speed = 2;
    reaper.facing = 1;
    insert_node(world, id, reaper.x, reaper.y);
    reaper.slave = victim_id;
    victim.slave = id;
    start_frame(anims, kAnimReaperArrives, reaper.facing, reaper.sprite, reaper.flip);
    reaper.state = kReaperPlayAnim;
    reaper.timer = 0x5dc;
    reaper.dest_x = uint8_t(gate.use_pos[0] % kMapSize);
    reaper.dest_y = uint8_t(gate.use_pos[0] / kMapSize);
    reaper.countdown = 0;
    reaper.reaper_next_state = kReaperGotoHell;
    start_frame(anims, kAnimReaperWalks, reaper.facing, reaper.next_sprite, reaper.next_flip);
    nav_init(world, reaper);
    return 1;
}

void reaper_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    switch (world.things[id].state) {
    case kReaperPlayAnim:
        reaper_play_anim(anims, world.things[id], game_turn);
        break;
    case kReaperWaits:
        reaper_waits(world, anims, id, game_turn);
        break;
    case kReaperRemove:
        remove_reaper(world, anims, id);
        break;
    case kReaperGotoHell:
        reaper_goto_hell(world, anims, id, game_turn);
        break;
    default:
        break;
    }
}

void object_hell(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &obj = world.things[id];
    if (game_turn % obj.anim_speed != 0)
        return;
    obj.sprite = anims.frames[obj.sprite].next;
    if (!(anims.frames[obj.sprite].flags & 1))
        return;
    if (obj.aux_sprite) {
        obj.sprite = obj.aux_sprite;
        obj.flip = obj.aux_flip;
        obj.aux_sprite = 0;
        obj.aux_flip = 0;
    } else if (obj.remove_after_anim) {
        remove_object(world, id, obj.owner);
    }
}

} // namespace th
