// A member of staff's turn: the dispatch of staff_turn, wandering, walking to an
// object and getting stuck, entering and leaving rooms, and the mood and speed
// upkeep (src/staff.cpp, src/goto.cpp, and modify_staff of src/environ.cpp).

#include "core/world/staff.h"

#include "core/data/blocks.h"
#include "core/world/arrivals.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/rooms.h"
#include "core/world/staff_complex.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"
#include "core/world/training.h"
#include "core/world/vaccine.h"
#include "core/world/wrap32.h"

#include <algorithm>
#include <cstdlib>

namespace th
{

inline namespace staff_detail
{

// calculate_speed (0x2c268), for staff at a cell's middle; a member of staff
// in a training room walks at the game's fixed speed for it.
void calculate_speed(const World &world, Thing &staff, const Balance &settings)
{
    if (staff.x.frac != 0x80 || staff.y.frac != 0x80)
        return;
    const Balance::Gbv &gbv = settings.gbv;
    uint32_t speed = 3;
    if (staff.tiredness > int(gbv.crack_up_tired))
        speed = 1;
    else if (staff.tiredness > int(gbv.very_tired))
        speed = 2;
    if (staff.ability >= gbv.consultant_threshold)
        speed += 30;
    else if (staff.ability > gbv.doctor_threshold)
        speed += 20;
    else
        speed += 10;
    if (staff.room && world.rooms[staff.room].type == kRoomTraining)
        speed = 31;
    switch (speed) {
    case 11:
    case 12:
    case 13:
    case 21:
    case 22:
    case 31:
        staff.speed = 0x10;
        staff.anim_speed = 2;
        break;
    case 23:
    case 32:
        staff.speed = 0x20;
        staff.anim_speed = 1;
        break;
    case 33:
        staff.speed = 0x40;
        staff.anim_speed = 1;
        break;
    }
}

} // namespace staff_detail

namespace
{

// doctor_cracks_up (0x3bd34 in src/doctor.cpp): a doctor, once, goes
// crazy: dressed so and with a new ability drawn within his grade's range,
// counted for the advisor (AdCounts::crackups) if he is hospital 0's.
void doctor_cracks_up(World &world, Thing &doctor, const Balance::Gbv &gbv)
{
    if (doctor.subtype != kStaffDoctor || doctor.crazy)
        return;
    if (doctor.layers[5] == 1)
        doctor.layers[5] = 3;
    else if (doctor.layers[5] == 2)
        doctor.layers[5] = 4;
    doctor.crazy = 1;
    uint32_t low = gbv.doctor_threshold, high = gbv.consultant_threshold;
    if (doctor.skills & kSkillConsultant) {
        low = gbv.consultant_threshold;
        high = 1000;
    } else if (doctor.skills & kSkillJunior) {
        low = 30;
        high = gbv.doctor_threshold;
    }
    doctor.ability = uint16_t(old_normal_rand(world.seed, low, high));
    if (doctor.owner == 0)
        world.counts.crackups++;
}

// modify_staff's tiredness and mood step: a doctor past gbv.crack_up_tired
// may crack up (a draw), the tired icon and happiness by tiredness, the too
// hot and too cold icons by warmth, and discontent by happiness.
void modify_staff_mood(World &world, Thing &s, const AnimData &anims, const Balance::Gbv &gbv)
{
    if (s.tiredness > int(gbv.crack_up_tired) && old_newrand(world.seed, "modify_staff") % 1000 == 0)
        doctor_cracks_up(world, s, gbv);
    if (s.tiredness > int(gbv.crack_up_tired)) {
        set_icon_value(anims, s, kIconTired);
        s.happiness--;
    } else if (s.tiredness < int(gbv.not_tired)) {
        reset_icon_value(s, kIconTired);
        s.happiness++;
    }
    if (int16_t(s.warmth) > 80) {
        s.happiness--;
        set_icon_value(anims, s, kIconTooHot);
    } else if (int16_t(s.warmth) < 20) {
        s.happiness--;
        set_icon_value(anims, s, kIconTooCold);
    }
    if (int16_t(s.happiness) < gbv.unhappy)
        s.discontent++;
    else if (int16_t(s.happiness) > gbv.happy)
        s.discontent--;
}

// modify_staff (0x71eb8 in src/environ.cpp) for member of staff `id`: due
// once game_turn (in 16 bits) passes Thing.modify_time, which it then sets
// gbv.modify_freq turns on; not for rivals' staff, applicants or
// receptionists, nor in the rat game. Speed, warmth and surroundings, then
// tiredness from work (not while wandering nor in a staff room), and
// modify_staff_mood.
void modify_staff(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    static const Balance statics;
    const Balance &settings = world.settings ? *world.settings : statics;
    const Balance::Gbv &gbv = settings.gbv;
    auto now = uint16_t(game_turn);
    if (!(now > s.modify_time || s.modify_time - now > 0x7fff))
        return;
    s.modify_time = uint16_t(game_turn + gbv.modify_freq);
    if (s.owner >= 4 || world.rats.fest_level || s.subtype == kStaffReceptionist)
        return;
    calculate_speed(world, s, settings);
    check_warmth(world, s, world.finance ? world.finance->heating_level : kStartHeatingLevel);
    staff_check_around(world, s);
    if (s.state != kStaffWanders && (s.room == 0 || world.rooms[s.room].type != kRoomStaffRoom))
        s.tiredness = int16_t(s.tiredness + gbv.work_light);
    modify_staff_mood(world, s, anims, gbv);
    if (int16_t(s.happiness) < 0)
        s.happiness = 0;
    else if (int16_t(s.happiness) > 100)
        s.happiness = 100;
    s.tiredness = std::clamp<int16_t>(s.tiredness, 0, 1000);
    if (int16_t(s.discontent) < 0)
        s.discontent = 0;
}
} // namespace

inline namespace staff_detail
{

// staff_in_corridor (0x2abb0): wanders from where they stand.
void staff_in_corridor(Thing &staff, const AnimData &anims)
{
    staff.dest_x = staff.x.tile;
    staff.dest_y = staff.y.tile;
    staff.state = kStaffWanders;
    staff.substate = 6;
    staff.countdown = 1;
    play_frame(staff, anims, kStaffAnims[staff.subtype].stand);
}

// A Ctx with the action sequences start_rooms_in_use gave the world (none
// before).
Ctx ctx_of(World &world, const AnimData &anims, uint32_t game_turn)
{
    static const PersonComplexes no_person;
    static const StaffComplexes no_staff;
    return {world, anims, world.person_complexes ? *world.person_complexes : no_person,
            world.staff_complexes ? *world.staff_complexes : no_staff, game_turn};
}

uint16_t cell_of(const Thing &t)
{
    return uint16_t(t.x.tile + (t.y.tile << 7));
}

void play_staff_anim(const AnimData &anims, Thing &s, size_t column)
{
    play_frame(s, anims, kStaffAnimStarts[s.subtype][column]);
}

uint8_t process(const Ctx &c, uint16_t id)
{
    return process_staff_complex(c.world, c.anims, c.person, c.staff, id, c.game_turn);
}

// can_i_get_to (0x19918) for a person: whether a path leads from their
// cell to `pos`.
bool can_i_get_to(const World &world, const Thing &person, uint16_t pos)
{
    OrthonavPath path;
    orthonav_init(world, path, person.x.tile, person.y.tile, pos % kMapSize, pos / kMapSize);
    return path.count != 0;
}

} // namespace staff_detail

namespace
{

// Moves a walking person speed / 256 cell on in their facing; one stuck
// (facing 4) stays.
void step_on(World &world, uint16_t id)
{
    Thing &t = world.things[id];
    if (t.facing >= 4)
        return;
    uint16_t raw_x = uint16_t((t.x.tile << 8 | t.x.frac) + t.speed * kAroundXy[t.facing][0]);
    uint16_t raw_y = uint16_t((t.y.tile << 8 | t.y.frac) + kAroundXy[t.facing][1] * t.speed);
    move_node(world, id, {uint8_t(raw_x), uint8_t(raw_x >> 8)}, {uint8_t(raw_y), uint8_t(raw_y >> 8)});
}
} // namespace

inline namespace staff_detail
{

// Whether `cell` holds litter to sweep (MapCell.litter_shadow's low nibble:
// 0 none, 7 and 8 are not swept).
bool has_litter(const MapCell &cell)
{
    uint8_t kind = cell.litter_shadow & 0xf;
    return !(kind == 0 || kind == 7 || kind == 8);
}

// listhead[owner].rooms[ROOM_RECEPTION] != 0: place_existing_object sets it
// when the hospital's first reception desk is placed (to an uninitialised
// value, taken to be nonzero) and nothing clears it. Desks cannot be sold
// yet, so it is whether the hospital has one; only player 0's are listed.
bool has_reception(const World &world, uint8_t owner)
{
    return owner == 0 && world.first_of_kind[kObjReceptionDesk] != 0;
}

// shall_i_resign (0x2bfc8) for member of staff `id`. With notice handed in
// (kOrderResign), they leave once it runs out: their call to the player
// (type 1) removed, sack_staff mode 1, and true. Otherwise, hired more than
// six to nine months (a draw) ago and discontented past gbv.resign_max,
// they hand in notice, 1500 turns, with a call to the player, and true.
// While they have notice handed in, the advisor hears of it
// (kAdvisorEventResign). Poaching is not ported.
bool shall_i_resign(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    if (s.orders & kOrderResign) {
        set_icon_value(c.anims, s, kIconLeaving);
        if (s.owner == 0)
            world.advisor_events |= kAdvisorEventResign;
        if (c.game_turn < s.resign_time)
            return false;
        get_playercall(world, kCallStaffResigns, s.playercall, s.owner);
        sack(c, id, 1);
        s.playercall = 0;
        return true;
    }
    uint32_t limit = ((old_newrand(world.seed) & 3) + 6) * 1500;
    static const Balance statics;
    const Balance::Gbv &gbv = world.settings ? world.settings->gbv : statics.gbv;
    if (c.game_turn - uint16_t(s.hire_time) > limit && int16_t(s.discontent) > gbv.resign_max) {
        s.orders |= kOrderResign;
        s.resign_count++;
        s.resign_time = c.game_turn + 1500;
        s.playercall = make_a_playercall(world, kCallStaffResigns, id, 0, s.owner);
        return true;
    }
    return false;
}

} // namespace staff_detail

namespace
{

// staff_nav_failure (0x2b5cc): heading from the corridor for a room built
// in the last 500 turns, back to the corridor (evacuate_staff); else stuck
// (SS_STUCK), announced to the advisor (World::announce_stuck) for
// hospital 0's staff.
void staff_nav_failure(const Ctx &c, uint16_t id)
{
    Thing &staff = c.world.things[id];
    trace_event("staff_nav_failure %u", unsigned(id));
    if (staff.room == 0) {
        uint16_t room = room_at(c.world, uint16_t(staff.dest_x + (staff.dest_y << 7)));
        if (room && c.game_turn - 500 < c.world.rooms[room].created_turn) {
            evacuate_staff(c, id);
            return;
        }
    }
    staff.saved_state = staff.state;
    staff.saved_dest_x = staff.dest_x;
    staff.saved_dest_y = staff.dest_y;
    staff.state = kStaffStuck;
    staff.countdown = 0;
    if (staff.owner == 0)
        c.world.announce_stuck = true;
}
} // namespace

inline namespace staff_detail
{

// find_free_object_anywhere (0x3800c), outside rooms: the nearest free
// object of `kind` of the seeker's hospital (by cells across and down)
// whose use cell (0 for staff, 2 for handymen, 1 for patients) a path from
// the seeker reaches. 0 if none.
uint16_t find_free_object_anywhere(const World &world, uint16_t id, uint8_t kind)
{
    const Thing &seeker = world.things[id];
    if (seeker.owner != 0)
        return 0;
    size_t side = 1;
    if (seeker.type == kThingStaff)
        side = seeker.subtype == kStaffHandyman ? 2 : 0;
    uint16_t best = 0;
    uint32_t best_dist = 9999;
    for (uint16_t o = world.first_of_kind[kind]; o; o = world.things[o].type_obj_next) {
        const Thing &obj = world.things[o];
        if (obj.in_use != 0 || !can_i_get_to(world, seeker, obj.use_pos[side]) || obj.room_id)
            continue;
        auto dist = uint32_t(std::abs(seeker.x.tile - obj.x.tile) + std::abs(seeker.y.tile - obj.y.tile));
        if (dist < best_dist) {
            best_dist = dist;
            best = o;
        }
    }
    return best;
}

} // namespace staff_detail

namespace
{

// try_staff_wander (0x778b4): if the cell (dx, dy) from the wanderer is
// walkable corridor of their hospital, in no room, and their destination
// moved by (dx, dy) is on the map, moves it there and sets off in
// `substate`; otherwise substate 6, to decide again.
void try_staff_wander(const Ctx &c, uint16_t id, int dx, int dy, uint8_t substate)
{
    Thing &e = c.world.things[id];
    int x = e.x.tile + dx;
    int y = e.y.tile + dy;
    auto tile = uint16_t(x + (y << 7));
    if (!(tile < kMapCells && !(c.world.map[tile].flags & 3) && x >= 0 && x < kMapSize && y >= 0 && y < kMapSize) ||
        c.world.map[tile].owner_flags != shl32(1, e.owner) || room_at(c.world, tile)) {
        e.substate = 6;
        return;
    }
    if (!(e.dest_x + dx >= 0 && e.dest_x + dx < kMapSize && e.dest_y + dy >= 0 && e.dest_y + dy < kMapSize)) {
        e.substate = 6;
        return;
    }
    e.dest_x = uint8_t(e.dest_x + dx);
    e.dest_y = uint8_t(e.dest_y + dy);
    play_staff_anim(c.anims, e, kAnimWalk);
    nav_init(c.world, e);
    e.countdown = 1;
    e.substate = substate;
}
} // namespace

inline namespace staff_detail
{

// The player's policy (World::hospital), or its start before
// start_rooms_in_use.
const HospitalSettings &policy_of(const World &world)
{
    static const HospitalSettings statics;
    return world.hospital ? *world.hospital : statics;
}

} // namespace staff_detail

namespace
{

// The decision of staff_wanders: an ordered break (kOrderRest,
// take_a_break), a page answered (obey_pager), a break when more tired than
// the policy's rest_threshold, else a stroll of 3 cells back the way they
// came (85%), else, standing, a pause of 20-49 turns, a stroll of 3 cells in
// a random direction or one to a random cell within 3. The orders to go
// back to research or training are dropped.
void wander_decide(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    e.orders &= uint8_t(~(kOrderReturnToResearch | kOrderReturnToTraining));
    if (e.subtype == kStaffNurse && should_nurse_vaccinate(world, c.anims, c.person, c.staff, id, c.game_turn))
        return;
    if ((e.orders & kOrderRest) && take_a_break(c, id))
        return;
    if (obey_pager(c, id))
        return;
    if (e.tiredness > policy_of(world).rest_threshold && take_a_break(c, id))
        return;
    if (old_newrand(world.seed, "staff_wanders") % 100 < 0x55) {
        switch (e.substate) {
        case 1:
            try_staff_wander(c, id, 0, 3, 3);
            return;
        case 3:
            try_staff_wander(c, id, 0, -3, 1);
            return;
        case 2:
            try_staff_wander(c, id, 3, 0, 4);
            return;
        case 4:
            try_staff_wander(c, id, -3, 0, 2);
            return;
        }
    }
    e.substate = 6;
    play_staff_anim(c.anims, e, kAnimStand);
    uint32_t roll = old_newrand(world.seed, "staff_wanders") % 100;
    if (roll < 0xf) {
        e.substate = 0;
        e.countdown = int16_t(old_newrand(world.seed, "staff_wanders") % 30 + 20);
    } else if (roll < 0x17) {
        try_staff_wander(c, id, 0, -3, 1);
    } else if (roll < 0x1f) {
        try_staff_wander(c, id, -3, 0, 2);
    } else if (roll < 0x27) {
        try_staff_wander(c, id, 0, 3, 3);
    } else if (roll < 0x2f) {
        try_staff_wander(c, id, 3, 0, 4);
    } else {
        // The game takes old_newrand() % 7 - 3 in unsigned arithmetic into a
        // signed byte: -3 to 3.
        auto dx = int8_t(old_newrand(world.seed, "staff_wanders") % 7 - 3);
        auto dy = int8_t(old_newrand(world.seed, "staff_wanders") % 7 - 3);
        try_staff_wander(c, id, dx, dy, 5);
    }
}

// Whether the person stands on a blueprint's floor and was sent off it
// (move_peep_from_blueprint), as the walking states check first.
bool off_blueprint(World &world, uint16_t id)
{
    const Thing &t = world.things[id];
    return block_flags(world.map[cell_of(t)].floor_block) == 0x20 && move_peep_from_blueprint(world, id);
}

// staff_wanders with the countdown run out: a receptionist with a free desk
// off to it; else, unless sent off a blueprint or resigning, the next step
// towards the destination, a new decision there or with no path. Returns
// whether they step on.
bool staff_wanders_head_on(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (e.subtype == kStaffReceptionist && has_reception(world, e.owner) &&
        find_free_object_anywhere(world, id, kObjReceptionDesk)) {
        e.state = kStaffReceptionistFindsReception;
        return false;
    }
    if (off_blueprint(world, id))
        return false;
    if (shall_i_resign(c, id))
        return false;
    e.countdown = int16_t(0x100 / e.speed);
    int dir = person_head_for(world, id);
    switch (dir) {
    case kNavArrived:
    case kNavNoPath:
        e.countdown = 1;
        wander_decide(c, id);
        return false;
    case kNavBlockedByPeople:
        play_staff_anim(c.anims, e, kAnimStand);
        e.countdown = 0;
        return false;
    case kNavWalledIn:
        staff_nav_failure(c, id);
        return false;
    default:
        if (e.facing != dir) {
            e.facing = uint8_t(dir);
            play_staff_anim(c.anims, e, kAnimWalk);
        }
        return true;
    }
}

// staff_wanders (0x77ab8). The idle nurses' count is not kept.
void staff_wanders(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (e.subtype == kStaffHandyman) {
        e.state = kStaffHandymanWorks;
        e.substate = 0;
        return;
    }
    if (e.subtype == kStaffNurse && e.owner == 0)
        world.idle_nurses++;
    if (shall_i_resign(c, id))
        return;
    if (e.substate == 0) {
        if (off_blueprint(world, id))
            return;
        if (--e.countdown <= 0)
            wander_decide(c, id);
        return;
    }
    if (e.substate == 6) {
        wander_decide(c, id);
        return;
    }
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0 && !staff_wanders_head_on(c, id))
        return;
    step_on(world, id);
}

// staff_goto_object with the countdown run out: unless sent off a
// blueprint, the next step towards the destination, the complex's next step
// there. Returns whether they step on.
bool staff_goto_object_head_on(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (off_blueprint(world, id))
        return false;
    e.countdown = int16_t(0x100 / e.speed);
    int dir = person_head_for(world, id);
    switch (dir) {
    case kNavArrived:
        e.dest_room = 0;
        process(c, id);
        return false;
    case kNavBlockedByPeople:
        play_staff_anim(c.anims, e, kAnimStand);
        e.countdown = 0;
        return false;
    case kNavWalledIn:
        staff_nav_failure(c, id);
        return false;
    case kNavNoPath:
        if (e.resume_state)
            fail_staff_complex(world, c.anims, id);
        else
            staff_nav_failure(c, id);
        return false;
    default: {
        size_t stand = size_t(kStaffAnimStarts[e.subtype][kAnimStand]) + e.facing;
        if (e.facing != dir || (stand < c.anims.starts.size() && c.anims.starts[stand].frame == e.sprite)) {
            e.facing = uint8_t(dir);
            play_staff_anim(c.anims, e, kAnimWalk);
        }
        return true;
    }
    }
}

// staff_goto_object (0x77508): walks to the destination and runs the
// complex's next step there, its room reached (dest_room cleared).
void staff_goto_object(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (e.subtype == kStaffHandyman)
        handyman_check_for_litter(world, e);
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0 && !staff_goto_object_head_on(c, id))
        return;
    step_on(world, id);
}

// peep_stuck (0x788d8) for staff: stands for 100 turns, then until a path
// to the saved destination opens tries random directions, then resumes
// saved_state.
void staff_stuck(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
    if (c.game_turn % e.anim_speed == 0)
        e.sprite = c.anims.frames[e.sprite].next;
    if (--e.countdown <= 0) {
        if (e.facing != 4) {
            e.countdown = 100;
            play_staff_anim(c.anims, e, kAnimStand);
            e.facing = 4;
            return;
        }
        uint16_t dest = uint16_t(e.saved_dest_x + (e.saved_dest_y << 7));
        OrthonavPath path;
        orthonav_init(world, path, e.x.tile, e.y.tile, dest % kMapSize, dest / kMapSize);
        if (path.count != 0) {
            e.state = e.saved_state;
            e.dest_x = e.saved_dest_x;
            e.dest_y = e.saved_dest_y;
            return;
        }
        auto dir = uint8_t(old_newrand(world.seed, "peep_stuck") & 3);
        if (!wall_in_way(world, e.x.tile, e.y.tile, dir)) {
            e.facing = dir;
            e.countdown = int16_t(0x100 / e.speed);
            play_staff_anim(c.anims, e, kAnimWalk);
        }
    }
    step_on(world, id);
}
} // namespace

inline namespace staff_detail
{

// get_state (0x2ca88): the state of member of staff `s` entering a room of
// `type` (0 for the corridor), or SS_EVACUATE for a doctor without the
// skills the room needs.
uint8_t get_state(uint8_t type, const Thing &s)
{
    uint8_t state = kStaffStates[kRoomStaffStateRow[type % kRoomTypes]][s.subtype % 5];
    uint16_t skills = kRoomDoctorSkills[type % kRoomTypes];
    if (s.subtype == kStaffDoctor && skills && !(s.skills & skills))
        state = kStaffEvacuate;
    return state;
}

// staff_exit_room_check (0x2bbdc) as staff leave their room: a surgeon
// becomes a doctor again, a consultant holding a training room stops the
// projector (leaving by the complex for whether they are the one using it),
// and a nurse frees her ward.
void staff_exit_room_check(World &world, Thing &s)
{
    if (!s.room)
        return;
    RoomInfo &room = world.rooms[s.room];
    uint16_t id = uint16_t(&s - world.things.data());
    if (s.subtype == kStaffSurgeon)
        change_surgeon_to_doc(world, id);
    if (room.type == kRoomTraining && room.staff[0] == id) {
        uint16_t projector = find_object(world, cell_of(s), kObjProjector, false, false, s.owner);
        if (projector && world.things[projector].lecture_started) {
            s.complex_pc = 0;
            s.complex_queued = 0;
            s.complex_queued2 = 0;
            s.complex_resume = 0;
            const Thing &obj = world.things[s.object];
            bool on_projector = s.object && (obj.type == kThingObject || obj.type == kThingDoor) && obj.user == id &&
                                s.object == projector;
            set_complex(world, id, on_projector ? kComplexExitTrainingHold : kComplexExitTrainingNoHold, 0);
            world.things[projector].lecture_started = 0;
        }
    }
    if (s.subtype == kStaffNurse && room.type == kRoomWard && room.state != kRoomEdited)
        room.state = 0;
}

// check_room_mutex (0x2b400): claims the member of staff's room for them
// (two holders in an operating theatre). Returns whether they hold it or
// are in no room.
bool check_room_mutex(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return true;
    RoomInfo &room = world.rooms[s.room];
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

// evacuate_staff (0x2ad8c): out of their room by its door, then the
// corridor (SS_IN_CORRIDOR).
void evacuate_staff(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    if (s.room) {
        staff_exit_room_check(world, s);
        set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        s.next_state = kStaffInCorridor;
        process(c, id);
    } else {
        s.state = kStaffInCorridor;
    }
}

// obey_pager (0x2ac70): if a page for the member of staff's skills waits in
// another room (check_pager), sends them there: out of their room first,
// then to its door and in, in the state for it. Returns whether they went.
bool obey_pager(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    uint16_t room = check_pager(world, id, c.game_turn);
    if (room == 0 || room == s.room)
        return false;
    staff_exit_room_check(world, s);
    s.next_state = get_state(world.rooms[room].type, s);
    s.substate = 0;
    if (s.room) {
        set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        process(c, id);
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToRoom, world.rooms[room].door_tile);
    } else {
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToRoom, world.rooms[room].door_tile);
        process(c, id);
    }
    s.dest_room = room;
    return true;
}

} // namespace staff_detail

namespace
{

// staff_turn for the states of doctors and nurses at work in their rooms.
// Returns whether the member of staff is in one.
bool staff_turn_at_work(const Ctx &c, uint16_t id)
{
    switch (c.world.things[id].state) {
    case kStaffDoctorInOffice:
        doctor_in_office(c, id);
        return true;
    case kStaffDoctorWorksAtDesk:
        doctor_works_at_desk(c, id);
        return true;
    case kStaffDoctorWaitsForPatient:
        doctor_waits_for_patient(c, id);
        return true;
    case kStaffDoctorExamines:
        doctor_examines(c, id);
        return true;
    case kStaffDoctorInMachineRoom:
        doctor_in_machine_room(c, id);
        return true;
    case kStaffDoctorInTheatre:
        doctor_in_theatre(c, id);
        return true;
    case kStaffDoctorResearches:
        doctor_researches(c, id);
        return true;
    case kStaffNurseInWard:
        nurse_in_ward(c, id);
        return true;
    case kStaffSurgeonWaits:
        surgeon_waits(c, id);
        return true;
    case kStaffAssistantReady:
        // The assistant waits for the lead surgeon to drive it.
        return true;
    default:
        return false;
    }
}

// staff_turn for the receptionists' states; the others do nothing.
void staff_turn_receptionist(const Ctx &c, uint16_t id)
{
    switch (c.world.things[id].state) {
    case kStaffReceptionistFindsReception:
        receptionist_finds_reception(c, id);
        break;
    case kStaffReceptionistSetup:
        receptionist_setup(c.world, id);
        break;
    case kStaffReceptionistWorks:
        receptionist_works(c, id);
        break;
    case kStaffReceptWaitsForPatient:
        recept_waits_for_patient(c, id);
        break;
    case kStaffReceptAdmitsPatient:
        recept_admits_patient(c.world, c.anims, id);
        break;
    default:
        break;
    }
}

// process_things' SS_BACK_TO_APPLICANTS: the member of staff is an applicant
// again, counted in the job market.
void back_to_applicants(World &world, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    trace_event("back_to_applicants %u", unsigned(id));
    remove_node(world, id);
    s.state = kStaffApplicant;
    s.hire_time = int16_t(game_turn);
    if (world.staff_market) {
        world.staff_market->applicants[s.subtype]++;
        world.staff_market->applicant_count++;
    }
}
} // namespace

void staff_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    const Ctx c{world, anims, person_complexes, staff_complexes, game_turn};
    Thing &s = world.things[id];
    modify_staff(world, anims, id, game_turn);
    if (staff_complex_turn(world, anims, person_complexes, staff_complexes, id, game_turn))
        return;
    switch (s.state) {
    case kStaffApplicant:
        if (game_turn > applicant_expiry(s))
            remove_staff_in_struct(world, s);
        break;
    case kStaffInCorridor:
        staff_in_corridor(s, anims);
        break;
    case kStaffWanders:
        staff_wanders(c, id);
        break;
    case kStaffGotoObject:
        staff_goto_object(c, id);
        break;
    case kStaffStuck:
        staff_stuck(c, id);
        break;
    case kStaffHandymanWorks:
        handyman_works(c, id);
        break;
    case kStaffEvacuate:
        evacuate_staff(c, id);
        break;
    case kStaffPlayAniseq:
        play_aniseq(c, s);
        break;
    case kStaffInStaffRoom:
        staff_in_staff_room(c, id);
        break;
    case kStaffDoctorIsTrained:
        doctor_is_trained(c, id);
        break;
    case kStaffConsultantTrains:
        consultant_trains(c, id);
        break;
    case kStaffBpGoto:
        peep_bp_goto_turn(world, anims, id, game_turn);
        break;
    case kStaffBackToApplicants:
        back_to_applicants(world, id, game_turn);
        break;
    case kStaffNurseVaccinates:
        nurse_vaccinates(world, anims, person_complexes, staff_complexes, id, game_turn);
        break;
    default:
        if (!staff_turn_at_work(c, id))
            staff_turn_receptionist(c, id);
        break;
    }
}

} // namespace th
