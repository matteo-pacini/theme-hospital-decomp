#include "core/world/staff_complex.h"

#include "core/data/gamedata.h"
#include "core/world/counts.h"
#include "core/world/nav.h"
#include "core/world/rooms.h"
#include "core/world/staff.h"
#include "core/world/staff_complex_internal.h"
#include "core/world/staff_complex_tables.h"
#include "core/world/trace.h"

#include <algorithm>

namespace th
{

const std::array<std::array<uint16_t, 0x8c>, 5> kStaffAnimStarts = {{
    {{603,  1620, 1628, 1624, 9999, 825,  1632, 1632, 1636, 9999, 9999, 795,  797,  799,  813,  833,  9999, 1826, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1939, 1943, 1947, 2390, 1108, 2382, 999}}, // 0
    {{16,   28,   36,   32,   44,   20,   335,  335,  2375, 9999, 239,  241,  248,  232,  288,  328,  355,
      359,  363,  399,  401,  403,  405,  407,  515,  683,  685,  687,  689,  655,  649,  9999, 9999, 9999,
      657,  659,  661,  681,  663,  665,  9999, 9999, 9999, 9999, 653,  691,  9999, 1049, 2439, 1089, 1093,
      1097, 1104, 1108, 1850, 1256, 1260, 1258, 1272, 1207, 1116, 1118, 1283, 1102, 1137, 1295, 1299, 1301,
      1303, 1305, 1307, 1316, 1311, 1318, 1297, 1342, 1281, 1285, 1287, 615,  617,  619,  622,  624,  999}}, // 1
    {{929,  12,   9999, 9999, 9999, 933,  1643, 1643, 1759, 9999, 936,  938,  986,  990,  177,  9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1785, 1789,
      1793, 2386, 1108, 1850, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 1962, 1964, 1966, 1968, 1970, 999}}, // 2
    {{1834, 9999, 1051, 9999, 1051, 1838, 1842, 1842, 9999, 9999, 1051, 1051, 1051, 1051, 1051, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 1852, 1856, 1860, 1951, 1108, 1850, 9999, 9999, 9999, 9999, 9999, 999}}, // 3
    {{1144, 9999, 9999, 9999, 9999, 1148, 335,  335,  1020, 9999, 1165, 1167, 1163, 1153, 1159, 1171, 1169, 1181, 1183,
      1391, 1392, 1395, 1396, 2445, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999,
      9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 999}}, // 4
}};

inline namespace staff_complex_detail
{

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

// The use cell of an object a member of staff uses: handymen use 2, others
// 0 (and leave to 5 or 3).
size_t use_side(const Thing &staff)
{
    return staff.subtype == kStaffHandyman ? 2 : 0;
}

void play_staff_anim(const AnimData &anims, Thing &s, size_t column)
{
    play_frame(s, anims, kStaffAnimStarts[s.subtype][column]);
}

// The game's PLAY_FRAME after person_head_for: faces the step's direction,
// if any, and starts walking.
void head_off(const Run &r, uint16_t id)
{
    Thing &s = r.world.things[id];
    int dir = person_head_for(r.world, id);
    if (dir >= 0 && dir < 4)
        s.facing = uint8_t(dir);
    play_staff_anim(r.anims, s, kAnimWalk);
}

// can_i_go_near_point (0x1ac9c): of the four cells beside `target`, the
// nearest to the person that a path reaches (can_i_get_to); 0 if none.
uint16_t can_i_go_near_point(const World &world, const Thing &person, uint16_t target)
{
    uint16_t best = 0xffff;
    uint16_t result = 0;
    uint16_t pos = cell_of(person);
    for (int16_t offset : kAround) {
        uint16_t cand = uint16_t(target + offset);
        auto dist = uint16_t(std::abs(pos % 128 - cand % 128) + std::abs(pos / 128 - cand / 128));
        if (dist >= best)
            continue;
        OrthonavPath path;
        orthonav_init(world, path, person.x.tile, person.y.tile, cand % 128, cand / 128);
        if (path.count != 0) {
            best = dist;
            result = cand;
        }
    }
    return result;
}

void fail(const Run &r, uint16_t id)
{
    Thing &s = r.world.things[id];
    if (s.resume_state) {
        s.state = s.resume_state;
        s.complex_pc = s.complex_queued = s.complex_queued2 = 0;
        s.complex_action = s.complex_resume = 0;
        play_staff_anim(r.anims, s, kAnimStand);
        if (s.subtype == kStaffHandyman) {
            thingpage_cancel_my_response(r.world.plant_pager, id);
            thingpage_cancel_my_response(r.world.repair_pager, id);
        }
    } else {
        s.state = kStaffComplexRetry;
    }
}

// check_room_mutex_on_entry (0x41e44): claims the room the member of staff
// enters by its door (at exit_tile) for them: whether they hold it already
// or take a free place (two in an operating theatre); true for the
// corridor.
bool check_room_mutex_on_entry(World &world, uint16_t id)
{
    uint16_t room_id = room_at(world, world.things[id].exit_tile);
    if (!room_id)
        return true;
    RoomInfo &room = world.rooms[room_id];
    if (id == room.staff[0] || id == room.staff[1])
        return true;
    if (room.type == kRoomTheatre) {
        for (uint16_t &holder : room.staff) {
            if (!holder) {
                holder = id;
                return true;
            }
        }
        return false;
    }
    if (room.staff[0])
        return false;
    room.staff[0] = id;
    return true;
}

// clear_my_mutex (0x2bb58): the member of staff no longer holds their room.
void clear_my_mutex(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return;
    RoomInfo &room = world.rooms[s.room];
    if (room.staff[0] == id)
        room.staff[0] = 0;
    else if (room.staff[1] == id)
        room.staff[1] = 0;
}

// The starting frame `start` (an AnimData::starts index), without
// check_frame, as the game sets a door's.
void set_start(Thing &t, const AnimData &anims, size_t start)
{
    if (start >= anims.starts.size())
        return;
    t.sprite = uint16_t(anims.starts[start].frame);
    t.flip = uint8_t(anims.starts[start].flip);
}

// complex_success (0x423c8) for a member of staff: research done
// (research_contribution: kinds 8, 10, 14 and 16 for 200, 400, 50 and 100
// points), a rest on a sofa, at snooker or a video game (kinds 18, 20, 22),
// an autopsy of their patient (12, person_autopsied) and a use of their
// object (24, Thing.times_used).
void staff_success(World &world, Thing &s, uint16_t kind)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    switch (kind) {
    case 8:
        research_contribution(world, s, 200);
        break;
    case 10:
        research_contribution(world, s, 400);
        break;
    case 12:
        person_autopsied(world, s.slave);
        break;
    case 14:
        research_contribution(world, s, 50);
        break;
    case 16:
        research_contribution(world, s, 100);
        break;
    case 18:
        rest_staff(world, s, int16_t(gbv.rest_sofa));
        break;
    case 20:
        rest_staff(world, s, int16_t(gbv.rest_snooker));
        break;
    case 22:
        rest_staff(world, s, int16_t(gbv.rest_game));
        break;
    case 24:
        world.things[s.object].times_used++;
        break;
    default:
        break;
    }
    s.happiness = uint16_t(std::clamp(int(int16_t(s.happiness)), 0, 100));
}

// process_complex_either (0x3e6c0).
uint8_t process_either(const Run &r, uint16_t id)
{
    switch (r.world.things[id].type) {
    case kThingStaff:
        return process(r, id);
    case kThingPatient:
        return process_person_complex(r.world, r.anims, r.person, id, r.game_turn);
    default:
        return 0;
    }
}

namespace
{

// Moves the complex on past step `entry`: at once, or after it has repeated
// entry.repeat times (a random 1 to -entry.repeat when negative).
void process_advance(World &world, Thing &e, const ComplexStep &entry)
{
    if (!entry.repeat) {
        e.complex_pc++;
        return;
    }
    if (e.complex_repeat) {
        e.complex_repeat--;
        if (e.complex_repeat == 0)
            e.complex_pc++;
    } else if (entry.repeat < 0) {
        e.complex_repeat = uint8_t(old_newrand(world.seed, "process_staff_complex") % uint32_t(-entry.repeat) + 1);
    } else {
        e.complex_repeat = uint8_t(entry.repeat);
    }
}

// At the end of the complex, starts the one queued behind it (running its
// first step) or moves on to next_state; otherwise runs a chained step.
void process_finish(const Run &r, uint16_t id, const ComplexStep &entry)
{
    World &world = r.world;
    Thing &e = world.things[id];
    if (r.staff.at(e.complex_pc) != 0) {
        if (entry.chain)
            process(r, id);
        return;
    }
    e.complex_pc = 0;
    uint16_t action = e.complex_queued;
    if (action) {
        uint16_t arg = uint16_t(e.queued_arg_x + (e.queued_arg_y << 7));
        e.complex_queued = e.complex_queued2;
        e.complex_queued2 = 0;
        e.queued_arg_x = e.queued2_arg_x;
        e.queued2_arg_x = 0;
        e.queued_arg_y = e.queued2_arg_y;
        e.queued2_arg_y = 0;
        if (set_complex(world, id, action, arg))
            process(r, id);
    } else if (e.next_state) {
        e.state = e.next_state;
        e.next_state = 0;
    }
}
} // namespace

uint8_t process(const Run &r, uint16_t id)
{
    World &world = r.world;
    Thing &e = world.things[id];
    if (e.complex_pc == 0)
        return 2;
    uint8_t op = r.staff.at(e.complex_pc);
    if (op == 0) {
        e.complex_pc = 0;
        return 2;
    }
    uint16_t tile = cell_of(e);
    const ComplexStep &entry = kStaffComplex[op];
    uint16_t obj_id = 0;
    if (entry.object) {
        obj_id = find_nearest_free_object(world, id, entry.object);
        if (obj_id == 0) {
            fail(r, id);
            return 0;
        }
    }
    if (entry.state)
        e.state = entry.state;
    if (uint8_t result = process_action(r, id, entry, obj_id, tile); result != kStepDone)
        return result;
    process_advance(world, e, entry);
    process_finish(r, id, entry);
    // Action 50: the game runs process_things for the member of staff alone.
    if (entry.action == 50) {
        process_things_alone_begin(world, id, r.game_turn);
        staff_turn(world, r.anims, r.person, r.staff, id, r.game_turn);
        final_count(world, r.game_turn);
    }
    return 1;
}

} // namespace staff_complex_detail

void rest_staff(World &world, Thing &s, int16_t amount)
{
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    trace_event("rest_staff %u amount=%d tired=%d", unsigned(&s - world.things.data()), int(amount), int(s.tiredness));
    if (s.tiredness > gbv.recovery_factor / 2) {
        amount = int16_t(amount * gbv.recovery_factor / s.tiredness);
        if (amount < int16_t(gbv.recovery_minimum))
            amount = int16_t(gbv.recovery_minimum);
    }
    s.tiredness = int16_t(s.tiredness - amount);
    s.happiness = uint16_t(std::clamp(int(int16_t(s.happiness + 1)), 0, 100));
}

bool load_staff_complexes(const std::filesystem::path &dir, StaffComplexes &out, std::string *error)
{
    auto data = load_data_file(dir, "data/s_comp.lex", error);
    if (!data)
        return false;
    for (uint8_t op : *data) {
        if (op >= kStaffComplex.size()) {
            if (error)
                *error = "data/s_comp.lex: unknown step " + std::to_string(op);
            return false;
        }
    }
    out.group = std::move(*data);
    return true;
}

uint8_t process_staff_complex(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                              const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    return process({world, anims, person_complexes, staff_complexes, game_turn}, id);
}

void fail_staff_complex(World &world, const AnimData &anims, uint16_t id)
{
    static const PersonComplexes no_person;
    static const StaffComplexes no_staff;
    fail({world, anims, no_person, no_staff, 0}, id);
}

void start_new_interaction(const PersonRun &run, const StaffComplexes &staff_complexes, uint16_t id, uint8_t seq)
{
    start_interaction({run.world, run.anims, run.complexes, staff_complexes, run.game_turn}, id, seq);
}

bool staff_complex_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                        const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    const Run r{world, anims, person_complexes, staff_complexes, game_turn};
    Thing &s = world.things[id];
    switch (s.state) {
    case kStaffPlayComplexAnim:
        // play_complex_staff_anim.
        if (game_turn % s.anim_speed == 0) {
            uint16_t next = anims.frames[s.sprite].next;
            if (anims.frames[next].flags & 1)
                process(r, id);
            else
                s.sprite = next;
        }
        return true;
    case kStaffTwoPeepsInteract: {
        // two_peeps_interact.
        interaction_frame(r, s);
        Thing &partner = world.things[s.slave];
        if (s.slave)
            interaction_frame(r, partner);
        if (s.interact_anim != 1 && partner.interact_anim != 1)
            setup_next_interaction(r, id);
        return true;
    }
    case kStaffWaitForComplexSlave:
        // staff_wait_for_complex_slave.
        if (s.slave && world.things[s.slave].state == kPsSlaveReady)
            process(r, id);
        return true;
    case kStaffComplexRetry:
        // staff_complex_retry.
        if (game_turn % s.anim_speed == 0) {
            s.sprite = anims.frames[s.sprite].next;
            if (anims.frames[s.sprite].flags & 1)
                process(r, id);
        }
        return true;
    default:
        return false;
    }
}

} // namespace th
