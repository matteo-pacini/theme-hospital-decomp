// A handyman's work: sweeping litter, watering plants, repairing machines, the
// animation sequences he plays and plants ageing (src/handyman.cpp,
// src/aniseq.cpp).

#include "core/world/staff.h"

#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/object_tables.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"
#include "core/world/wrap32.h"

#include <algorithm>

namespace th
{

namespace
{

// A handyman's to a room's door and in, and to the machine to repair.
constexpr uint16_t kComplexToRoomToRepair = 0x96;

// The same to a plant and its watering: to the plant, the watering itself
// and the step after it.
constexpr uint16_t kComplexToPlant = 0x26c;
constexpr uint16_t kComplexWaterPlant = 0x258;
constexpr uint16_t kComplexAfterWatering = 0x262;

// A plant (OBJ_PLANT) and the object states plant_ages works on (the game's
// OS_*), with the animation starts of a wilting plant and of a handyman
// watering it.
constexpr uint8_t kObjPlant = 45;
constexpr uint8_t kObjectPlantAges = 0x0a;
constexpr uint8_t kObjectBeingWatered = 0x0e;
constexpr int16_t kPlantHealthy = 400;
constexpr uint16_t kPlantAnim = 0x3cf;
constexpr uint16_t kWaterPlantAnim = 0x3d0;

// The game's showanim_angle (src/tables.cpp): the facing a handyman turns
// to to sweep, by his facing.
constexpr std::array<uint8_t, 4> kShowanimAngle = {1, 1, 2, 2};

// The game's special_around (src/tables.cpp): a cell and its eight
// neighbours, as cell offsets.
constexpr std::array<int16_t, 9> kSpecialAround = {0, 1, 128, -1, -128, -129, 129, -127, 127};

// MapCell.owner_flags: an outdoor cell.
constexpr uint8_t kCellOutdoor = 0x10;

// The game's objects[].repair_aniseq (src/tables.cpp), by object kind: the
// ani_seq_choice group a handyman plays repairing a machine (0 none).
constexpr std::array<uint8_t, 63> kRepairAniseq = {0, 0,  0, 0, 0,  0, 0, 0, 0, 9,  0, 0, 0, 10, 11, 0, 0, 0, 0, 0, 0,
                                                   0, 12, 0, 4, 13, 8, 7, 0, 0, 14, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0,
                                                   5, 0,  0, 0, 2,  1, 0, 0, 0, 0,  0, 0, 3, 6,  0,  0, 0, 0, 0, 0, 0};

// The game's ani_seq (src/tables.cpp): animation sequences of
// AnimData::starts indices (the facing added), each ended by -1; and
// ani_seq_choice, the sequences (from 1) each group picks among, 0 ending.
constexpr std::array<int16_t, 38> kAniSeq = {-1,   1773, 1775, 1777, -1, 1767, 1769, 1771, -1, 1757, -1, 1749, -1,
                                             1783, -1,   1779, 1781, -1, 1751, -1,   1745, -1, 1742, -1, 1740, -1,
                                             445,  447,  -1,   282,  -1, 332,  -1,   284,  -1, 330,  -1, -2};
constexpr std::array<std::array<uint8_t, 10>, 15> kAniSeqChoice = {
    {{0}, {7}, {1}, {2}, {3}, {4, 9}, {5}, {6}, {8}, {10}, {11}, {12}, {13}, {14}, {15}}};

// setup_aniseq_indices (0x9b300): where each sequence of kAniSeq starts.
constexpr std::array<uint8_t, 17> kAniSeqStart = [] {
    std::array<uint8_t, 17> start{};
    size_t seq = 0;
    for (size_t i = 0; kAniSeq[i] != -2;) {
        start[++seq] = uint8_t(++i);
        while (kAniSeq[i] != -1 && kAniSeq[i] != -2)
            i++;
    }
    return start;
}();

// How many sequences each group of kAniSeqChoice picks among. Group 0 has
// none, and choose_aniseq would divide by zero on it, as the game would:
// kRepairAniseq's 0 means no group, so it is never asked for.
constexpr std::array<uint32_t, kAniSeqChoice.size()> kAniSeqChoiceCount = [] {
    std::array<uint32_t, kAniSeqChoice.size()> count{};
    for (size_t group = 0; group < kAniSeqChoice.size(); group++)
        while (kAniSeqChoice[group][count[group]])
            count[group]++;
    return count;
}();

static_assert(std::all_of(kAniSeqChoiceCount.begin() + 1, kAniSeqChoiceCount.end(), [](uint32_t n) { return n > 0; }));
} // namespace

inline namespace staff_detail
{

// handyman_check_for_litter (0x7b554): looks at a random cell near the
// handyman (nearer the more able he is) and remembers it if it has litter
// on his hospital's land and is nearer than what he remembers.
void handyman_check_for_litter(World &world, Thing &h)
{
    int range = std::max(10 - h.ability / 100, 1);
    // Unsigned arithmetic into a signed int, as in the game.
    auto dx = int32_t(old_newrand(world.seed) % uint32_t(range * 2) - uint32_t(range));
    auto dy = int32_t(old_newrand(world.seed) % uint32_t(range * 2) - uint32_t(range));
    auto pos = uint16_t(cell_of(h) + (dy << 7) + dx);
    // The game reads past the map for cells off its end; here they hold no
    // litter.
    if (pos >= kMapCells || !has_litter(world.map[pos]) || world.map[pos].owner_flags != shl32(1, h.owner))
        return;
    auto dist = [&](uint16_t cell) {
        int x = cell % 128 - h.x.tile;
        int y = cell / 128 - h.y.tile;
        return uint32_t(x * x + y * y);
    };
    uint16_t nearest = h.work_target;
    if (pos == nearest || pos == h.work_target2)
        return;
    if (dist(pos) < dist(nearest)) {
        h.work_target2 = nearest;
        h.work_target = pos;
    } else if (dist(pos) < dist(h.work_target2)) {
        h.work_target2 = pos;
    }
}

} // namespace staff_detail

namespace
{

// handyman_try_litter (0x7c6a0): sends the handyman to sweep litter on or
// beside his cell or, failing that, the litter he remembers nearest
// (forgetting it). Returns whether he set off.
bool handyman_try_litter(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    uint16_t target = 0;
    uint16_t pos = cell_of(h);
    for (int16_t offset : kSpecialAround) {
        auto cell = uint16_t(pos + offset);
        if (cell < kMapCells && has_litter(world.map[cell])) {
            target = cell;
            break;
        }
    }
    if (target == 0 && h.work_target) {
        if (has_litter(world.map[h.work_target]))
            target = h.work_target;
        h.work_target = h.work_target2;
        h.work_target2 = 0;
    }
    if (!target)
        return false;
    h.next_state = kStaffHandymanWorks;
    h.resume_state = kStaffHandymanWorks;
    set_complex(world, id, kComplexWalkTo, target);
    process(c, id);
    h.substate = 2;
    return true;
}

// check_points_connected (src/utils.cpp) for two cells as u16 (the game
// passes the wrapped values through s8 x and y).
bool points_connected(const World &world, uint16_t from, uint16_t to)
{
    OrthonavPath path;
    orthonav_init(world, path, int8_t(from % kMapSize), int8_t(from / kMapSize), int8_t(to % kMapSize),
                  int8_t(to / kMapSize));
    return path.count != 0;
}

// trapped_plant_check (0x7cbdc): whether tile `pos` can be reached, from a
// room's inside door when `room` holds `pos`, else from hospital 0's first
// entrance. Despite its name, 1 means reachable.
bool trapped_plant_check(const World &world, uint16_t room, uint16_t pos)
{
    if (room) {
        if (room_at(world, pos) == room)
            return points_connected(world, world.rooms[room].door_inside_tile, pos);
        return false;
    }
    return points_connected(world, world.rooms[world.entrances[0][0]].door_tile, pos);
}

// is_plant_trapped (0x7cc88): whether none of the four tiles beside `plant`
// can be reached (trapped_plant_check).
bool is_plant_trapped(const World &world, const Thing &plant)
{
    auto pos = int(plant.x.tile) + (int(plant.y.tile) << 7);
    return !(trapped_plant_check(world, plant.room_id, uint16_t(pos + 1)) ||
             trapped_plant_check(world, plant.room_id, uint16_t(pos + 0x80)) ||
             trapped_plant_check(world, plant.room_id, uint16_t(pos - 1)) ||
             trapped_plant_check(world, plant.room_id, uint16_t(pos - 0x80)));
}

// direction_to_face (0x1a764): the facing for `self` to look towards
// `target`, 0 +y, 1 -x, 2 -y, 3 +x, tested in that order; else self's.
uint8_t direction_to_face(const Thing &self, const Thing &target)
{
    if (target.y.tile > self.y.tile)
        return 0;
    if (target.x.tile < self.x.tile)
        return 1;
    if (target.y.tile < self.y.tile)
        return 2;
    if (target.x.tile > self.x.tile)
        return 3;
    return self.facing;
}

// is_next_to_plant (0x7c884): whether handyman stands on a tile beside
// plant (not diagonally) with the invisible wall an object puts up (blocks
// 0x8e-0x91) on the edge between them.
bool is_next_to_plant(const World &world, const Thing &handyman, const Thing &plant)
{
    auto here = uint16_t(cell_of(handyman)), there = uint16_t(cell_of(plant));
    int dx = int(plant.x.tile) - int(handyman.x.tile);
    int dy = int(plant.y.tile) - int(handyman.y.tile);
    if (dx * dx + dy * dy > 1)
        return false;
    auto blank = [](uint8_t wall) { return wall >= 0x8e && wall <= 0x91; };
    if (handyman.y.tile == plant.y.tile + 1)
        return blank(world.map[here].wall_n);
    if (handyman.x.tile == plant.x.tile + 1)
        return blank(world.map[here].wall_w);
    if (handyman.y.tile == plant.y.tile - 1)
        return blank(world.map[there].wall_n);
    if (handyman.x.tile == plant.x.tile - 1)
        return blank(world.map[there].wall_w);
    return false;
}

// setup_next_aniseq (0x9b3d8): the sequence's next animation, or at its end
// next_state.
void setup_next_aniseq(const AnimData &anims, Thing &s)
{
    if (kAniSeq[s.seq_index] == -1) {
        s.state = s.next_state;
    } else {
        play_frame(s, anims, uint16_t(kAniSeq[s.seq_index]));
        s.seq_index++;
    }
}

// choose_aniseq (0x9b510) for a member of staff: a sequence of `group`
// drawn from world.seed, played in SS_PLAY_ANISEQ (setup_aniseq), then
// `next_state`.
void choose_aniseq(World &world, const AnimData &anims, Thing &s, uint8_t group, uint8_t next_state)
{
    const std::array<uint8_t, 10> &choice = kAniSeqChoice[group % kAniSeqChoice.size()];
    uint32_t count = kAniSeqChoiceCount[group % kAniSeqChoice.size()];
    uint8_t seq = choice[old_newrand(world.seed, "choose_aniseq") % count];
    s.state = kStaffPlayAniseq;
    s.next_state = next_state;
    s.seq_index = kAniSeqStart[seq % kAniSeqStart.size()];
    setup_next_aniseq(anims, s);
}
} // namespace

inline namespace staff_detail
{

// play_aniseq (0x9b498): every anim_speed turns a frame on, the sequence's
// next animation when this one would wrap.
void play_aniseq(const Ctx &c, Thing &s)
{
    if (c.game_turn % s.anim_speed != 0)
        return;
    uint16_t next = c.anims.frames[s.sprite].next;
    if (c.anims.frames[next].flags & 1)
        setup_next_aniseq(c.anims, s);
    else
        s.sprite = next;
}

} // namespace staff_detail

namespace
{

// find_person_in_room (0x1b150) as a test: whether someone of thing type
// `type` and kind `kind` is in room `room`.
bool person_in_room(const World &world, uint16_t room, uint8_t type, uint8_t kind)
{
    return std::any_of(world.things.begin(), world.things.end(),
                       [&](const Thing &t) { return t.type == type && t.subtype == kind && t.room == room; });
}

// handyman_try_fix (0x7c488): the handyman takes the most urgent repair
// page of at least (100 - prio_repair) * 3 / 4 and heads into its machine's
// room (substate 8), the room due a repair unless closed. Returns whether
// he took one.
bool handyman_try_fix(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    uint16_t machine =
        thingpage_respond(world, world.repair_pager, id, 0, uint8_t((100 - h.prio_repair) * 3 / 4), c.game_turn);
    if (!machine || !world.things[machine].room_id)
        return false;
    uint16_t room_id = world.things[machine].room_id;
    RoomInfo &room = world.rooms[room_id];
    set_complex(world, id, kComplexToRoomToRepair, room.door_tile);
    h.next_state = kStaffHandymanWorks;
    h.substate = 8;
    h.resume_state = kStaffEvacuate;
    h.dest_room = room_id;
    process(c, id);
    if (room.state != kRoomClosed)
        room.state = kRoomRepairDue;
    return true;
}

// handyman_try_plant (0x7c5ac): the handyman takes the most urgent plant
// page and heads for it, through its room's door and in (substate 4) when it
// is in a room, else straight to it (substate 5). Returns whether he took
// one.
bool handyman_try_plant(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    uint16_t plant = thingpage_respond(world, world.plant_pager, id, 0, 0, c.game_turn);
    if (!plant)
        return false;
    if (uint16_t room_id = world.things[plant].room_id) {
        h.dest_room = room_id;
        set_complex(world, id, kComplexToRoomToRepair, world.rooms[room_id].door_tile);
        h.substate = 4;
    } else {
        set_complex(world, id, kComplexToPlant, 0);
        h.object = plant;
        h.substate = 5;
    }
    h.next_state = kStaffHandymanWorks;
    h.resume_state = kStaffEvacuate;
    process(c, id);
    return true;
}

// handyman_works's decision in a room: he pages its first worn machine,
// unless someone owns it, and takes a repair page there (substate 8), or
// leaves.
void handyman_works_in_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    RoomInfo &room = world.rooms[h.room];
    for (uint16_t o = room.first_object; o; o = world.things[o].room_obj_next) {
        const Thing &machine = world.things[o];
        if (machine.type != kThingObject || !kObjectIsMachine[machine.subtype % kObjectIsMachine.size()] ||
            machine.times_used <= 0)
            continue;
        if (machine.owner == 0)
            thingpage_request(world.repair_pager, o, 100);
        break;
    }
    if (thingpage_respond(world, world.repair_pager, id, h.room, 0, c.game_turn)) {
        h.substate = 8;
        if (room.state != kRoomClosed)
            room.state = kRoomRepairDue;
        return;
    }
    h.state = kStaffEvacuate;
}

// handyman_works's jobs, least done (for their priority) first: litter,
// plants and repairs. Returns whether he took one.
bool handyman_works_take_job(const Ctx &c, uint16_t id)
{
    const Thing &h = c.world.things[id];
    std::array<uint32_t, 3> scores = {uint32_t(h.litter_count * 100 / (h.prio_litter + 1)),
                                      uint32_t(h.plant_count * 1000 / (h.prio_plants + 1)),
                                      uint32_t(h.repair_count * 2000 / (h.prio_repair + 1))};
    for (;;) {
        uint32_t best = 2000000;
        size_t idx = 0;
        for (size_t n = 0; n < 3; n++) {
            if (scores[n] < best) {
                best = scores[n];
                idx = n;
            }
        }
        // The game tries again while a score is from 1000000 to 1999999,
        // never to end; here it gives up.
        if (best >= 1000000)
            return false;
        scores[idx] = 2000000;
        if ((idx == 0 && handyman_try_litter(c, id)) || (idx == 1 && handyman_try_plant(c, id)) ||
            (idx == 2 && handyman_try_fix(c, id)))
            return true;
    }
}

// handyman_works's stroll to a random cell within 7 of his hospital's,
// indoors, that he can reach.
void handyman_works_stroll(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    // Unsigned arithmetic into a signed int, as in the game.
    auto dx = int32_t(old_newrand(world.seed, "handyman_works") % 15 - 7);
    auto dy = int32_t(old_newrand(world.seed, "handyman_works") % 15 - 7);
    auto pos = uint16_t(cell_of(h) + (dy << 7) + dx);
    // The game reads past the map for cells off its end; here they are
    // not his land.
    if (pos < kMapCells && world.map[pos].owner_flags == shl32(1, h.owner) && can_i_get_to(world, h, pos) &&
        !(world.map[pos].owner_flags & kCellOutdoor)) {
        h.next_state = h.resume_state = kStaffHandymanWorks;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// handyman_works's substate 0: the decision.
void handyman_works_decide(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    if (shall_i_resign(c, id))
        return;
    if (((h.orders & kOrderRest) || h.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return;
    if (h.room) {
        handyman_works_in_room(c, id);
        return;
    }
    if (thingpage_find_highest_priority(world.repair_pager) && handyman_try_fix(c, id))
        return;
    if (handyman_works_take_job(c, id))
        return;
    handyman_works_stroll(c, id);
}

// handyman_works's substate 2: arrived at the litter, he sweeps it up
// (substate 3); elsewhere, he decides again.
void handyman_works_start_sweeping(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    if (h.x.tile != h.dest_x || h.y.tile != h.dest_y) {
        h.substate = 0;
        return;
    }
    if (h.work_target == cell_of(h))
        h.work_target = 0;
    h.facing = kShowanimAngle[h.facing & 3];
    h.next_state = kStaffHandymanWorks;
    h.substate = 3;
    set_complex(world, id, kComplexSweepUp, 0);
    process(c, id);
}

// handyman_works's substate 3: the litter swept, counted, and back to
// deciding.
void handyman_works_swept(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    remove_litter(world, cell_of(h), h.owner);
    set_complex(world, id, kComplexAfterSweeping, 0);
    h.next_state = kStaffHandymanWorks;
    process(c, id);
    h.substate = 0;
    h.litter_count++;
}

// handyman_works's substate 8: once the room is closed, to the machine
// paged (substate 9); else he closes it with no doctor, nurse or surgeon
// in it, or strolls to a random cell of it.
void handyman_works_close_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    RoomInfo &room = world.rooms[h.room];
    if (room.state == kRoomClosed) {
        h.next_state = kStaffHandymanWorks;
        h.substate = 9;
        h.resume_state = kStaffEvacuate;
        h.object = thingpage_remind_me(world.repair_pager, id);
        set_complex(world, id, kComplexToMachine, 0);
        process(c, id);
        return;
    }
    if (!person_in_room(world, h.room, kThingStaff, kStaffDoctor) &&
        !person_in_room(world, h.room, kThingStaff, kStaffNurse) &&
        !person_in_room(world, h.room, kThingStaff, kStaffSurgeon)) {
        room.state = kRoomClosed;
        return;
    }
    // The row is drawn first, as the game's code does.
    auto row = int16_t(old_newrand(world.seed, "handyman_works") % uint32_t(room.height));
    auto column = int16_t(old_newrand(world.seed, "handyman_works") % uint32_t(room.width));
    auto pos = uint16_t(room.pos + ((row << 7) + column));
    if (pos != cell_of(h) && can_i_get_to(world, h, pos)) {
        h.next_state = h.resume_state = kStaffHandymanWorks;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// handyman_works's substate 9: the machine's repair sequence, a puff of
// smoke cleared.
void handyman_works_repair(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    trace_event("repair %u machine=%u", unsigned(id), unsigned(h.object));
    Thing &machine = world.things[h.object];
    if (uint8_t seq = kRepairAniseq[machine.subtype % kRepairAniseq.size()])
        choose_aniseq(world, c.anims, h, seq, kStaffHandymanWorks);
    if (h.object)
        machine.smoke[old_newrand(world.seed, "handyman_works") % 3] = 0;
    h.substate = 10;
}

// handyman_works's substate 10: the machine's wear gone (its strength
// maybe one less), the page dropped, the room open again and he out of it.
void handyman_works_repaired(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    if (h.object) {
        Thing &machine = world.things[h.object];
        thingpage_cancel(world.repair_pager, h.object);
        reset_icon_value(machine, kIconNeedsRepair);
        if (machine.strength > 2 &&
            old_newrand(world.seed, "handyman_works") % uint32_t(machine.strength) < uint32_t(machine.times_used))
            machine.strength--;
        machine.times_used = 0;
        machine.overlay_frame = 0;
        machine.smoke = {};
    }
    trace_event("repaired %u machine=%u room=%u", unsigned(id), unsigned(h.object), unsigned(h.room));
    RoomInfo &room = world.rooms[h.room];
    room.state = 0;
    set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
    h.resume_state = kStaffEvacuate;
    h.next_state = kStaffHandymanWorks;
    h.substate = 0;
    h.repair_count++;
    process(c, id);
}

// handyman_works's substate 4: he has come into the plant's room, takes the
// plant he paged for and sets off to water it (substate 5).
void handyman_works_to_plant(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    h.object = thingpage_remind_me(world.plant_pager, id);
    set_complex(world, id, kComplexToPlant, 0);
    h.substate = 5;
    h.next_state = kStaffHandymanWorks;
    h.resume_state = kStaffEvacuate;
    process(c, id);
}

// handyman_works's substate 5: beside the plant, unless it is gone, being
// watered already or unreachable, he takes it (in_use) and starts watering
// it (substate 6), showing it at its wilting stage.
void handyman_works_start_watering(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    Thing &plant = world.things[h.object];
    thingpage_cancel(world.plant_pager, h.object);
    if (plant.in_use == 1 || plant.type != kThingObject || plant.subtype != kObjPlant ||
        plant.state != kObjectPlantAges || !is_next_to_plant(world, h, plant)) {
        h.state = kStaffEvacuate;
        return;
    }
    plant.in_use = 1;
    h.facing = direction_to_face(plant, h);
    h.next_state = kStaffHandymanWorks;
    h.resume_state = kStaffEvacuate;
    h.substate = 6;
    set_complex(world, id, kComplexWaterPlant, 0);
    process(c, id);
    if (plant.plant_health < 0)
        plant.plant_health = 0;
    auto stage = uint8_t((kPlantHealthy - plant.plant_health) / 80);
    uint16_t anim = kWaterPlantAnim + plant.facing;
    if (anim < c.anims.starts.size()) {
        plant.sprite = uint16_t(c.anims.starts[anim].frame);
        plant.flip = uint8_t(c.anims.starts[anim].flip);
    }
    for (int i = 0; i < 5 - stage - 1; i++)
        if (plant.sprite < c.anims.frames.size())
            plant.sprite = uint16_t(c.anims.frames[plant.sprite].next);
    plant.state = kObjectBeingWatered;
}

// handyman_works's substate 6: the watering animation plays every other turn;
// at its end the plant is healthy again, its sprite the healthy one, its
// condition counted for the advisor (the sum raised to its new value and
// clamped), the handyman free to work again and out of its room if he came
// through one.
void handyman_works_watering(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    Thing &plant = world.things[h.object];
    if (c.game_turn & 1)
        return;
    if (plant.sprite < c.anims.frames.size())
        plant.sprite = uint16_t(c.anims.frames[plant.sprite].next);
    if (plant.sprite >= c.anims.frames.size() || !(c.anims.frames[plant.sprite].flags & 1))
        return;
    plant.in_use = 0;
    uint16_t anim = kPlantAnim + plant.facing;
    if (anim < c.anims.starts.size()) {
        plant.sprite = uint16_t(c.anims.starts[anim].frame);
        plant.flip = uint8_t(c.anims.starts[anim].flip);
    }
    trace_event("watered %u plant=%u health=%d", unsigned(id), unsigned(h.object), int(plant.plant_health));
    plant.state = kObjectPlantAges;
    if (plant.owner == 0) {
        world.plant_health_sum += 0x37 - (plant.plant_health * 2 + 0x50) / 16;
        if (world.plant_health_sum < 0)
            world.plant_health_sum = 0;
        else if (world.plants * 0x37 < world.plant_health_sum)
            world.plant_health_sum = world.plants * 0x37;
    }
    plant.plant_health = kPlantHealthy;
    h.substate = 0;
    h.plant_count++;
    set_complex(world, id, kComplexAfterWatering, 0);
    if (h.room)
        set_complex(world, id, kComplexLeaveRoom, world.rooms[h.room].door_inside_tile);
    h.next_state = kStaffHandymanWorks;
    process(c, id);
    world.thirsty_plants--;
}
} // namespace

inline namespace staff_detail
{

// handyman_works (0x7a8a8). Substate 0 decides: in a room, he pages its
// first worn machine and takes a repair page there (substate 8), or leaves;
// in the corridor, the most urgent repair if urgent enough
// (handyman_try_fix), else the jobs least done for their priority first,
// sweeping litter he finds, watering a plant he pages or repairing; else he
// strolls to a random cell within 7 he can reach. Substates 2 and 3 sweep;
// 4 to 6 water a plant (into its room, to it, the watering animation, then
// healthy again); 8 to 10 repair: once the room is closed (its doctor closes
// it, or no staff are in it) to the machine, its repair (the machine's
// sequence, a puff of smoke cleared), then its wear gone (its strength
// maybe one less), the page dropped and the room open again. Before
// deciding, with a rest ordered or more tired than the policy's
// rest_threshold, he takes a break (take_a_break) if there is a staff room.
void handyman_works(const Ctx &c, uint16_t id)
{
    switch (c.world.things[id].substate) {
    case 0:
        handyman_works_decide(c, id);
        break;
    case 2:
        handyman_works_start_sweeping(c, id);
        break;
    case 3:
        handyman_works_swept(c, id);
        break;
    case 4:
        handyman_works_to_plant(c, id);
        break;
    case 5:
        handyman_works_start_watering(c, id);
        break;
    case 6:
        handyman_works_watering(c, id);
        break;
    case 8:
        handyman_works_close_room(c, id);
        break;
    case 9:
        handyman_works_repair(c, id);
        break;
    case 10:
        handyman_works_repaired(c, id);
        break;
    default:
        break;
    }
}

} // namespace staff_detail

// plant_ages (0x7b88c): one turn of plant `id` in OS_PLANT_AGES: every 16th
// turn, half the time, it loses a point of health; every 80 points it wilts a
// stage and pages a handyman (unless it cannot be reached), counting it for
// the advisor; once dead it pages urgently 40 points past zero, then starts
// again. Draws one random number each 16th turn.
void plant_ages(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    if (game_turn % 16 != 0 || (old_newrand(world.seed, "plant_ages") & 1))
        return;
    Thing &plant = world.things[id];
    plant.plant_health--;
    if (plant.plant_health > 0) {
        if (plant.plant_health % 80 == 0) {
            auto stage = uint8_t((kPlantHealthy - plant.plant_health) / 80);
            if (plant.sprite < anims.frames.size()) {
                uint16_t next = anims.frames[plant.sprite].next;
                if (next < anims.frames.size() && !(anims.frames[next].flags & 1))
                    plant.sprite = next;
            }
            if (!is_plant_trapped(world, plant))
                thingpage_request(world.plant_pager, id, stage);
            trace_event("plant_thirsty %u priority=%u", unsigned(id), unsigned(stage));
            world.thirsty_plants++;
            world.plant_health_sum -= 11;
        }
    } else if (plant.plant_health == -40) {
        if (!is_plant_trapped(world, plant))
            thingpage_request(world.plant_pager, id, 10);
        plant.plant_health = 0;
        trace_event("plant_dead %u", unsigned(id));
        world.thirsty_plants += 2;
    }
}

} // namespace th
