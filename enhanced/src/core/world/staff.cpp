#include "core/world/staff.h"

#include "core/data/blocks.h"
#include "core/world/complex_tables.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff_complex.h"
#include "core/world/trace.h"

#include <algorithm>
#include <bit>
#include <cstdlib>

namespace th
{

namespace
{

// allocate_thing's reserve for new staff (place_staff_in_struct).
constexpr size_t kStaffReserve = 0x19;

// staff[kind].layer5_count (src/tables.cpp): the layer-5 variants an
// applicant picks from, 0 for none.
constexpr std::array<uint8_t, 5> kLayer5Count = {0, 2, 1, 0, 1};

// staff[kind].anim (src/tables.cpp): the staff_anim_starts entry shown
// placed or held, 5 (standing) or 0 (walking).
constexpr std::array<uint8_t, 5> kHeldAnim = {5, 5, 5, 0, 5};

// Staff states not handled yet (the game's SS_*), which kStaffStates names.
constexpr uint8_t kStaffDoctorInMachineRoom = 0x24;
constexpr uint8_t kStaffInStaffRoom = 0x30;
constexpr uint8_t kStaffNurseInWard = 0x32;
constexpr uint8_t kStaffDoctorResearches = 0x3e;
constexpr uint8_t kStaffDoctorInTheatre = 0x42;
constexpr uint8_t kStaffDoctorIsTrained = 0x46;
// staff_states' -1: no state, the kind may not go there.
constexpr uint8_t kStaffNone = 0xff;

// The game's staff_states (src/tables.cpp): by row (kRoomStaffStateRow of
// the room type, 0 for the corridor) and staff kind, the state of staff
// entering a room.
constexpr std::array<std::array<uint8_t, 5>, 11> kStaffStates = {{
    {kStaffInCorridor, kStaffInCorridor, kStaffHandymanWorks, kStaffReceptionistFindsReception, kStaffInCorridor},
    {kStaffEvacuate, kStaffDoctorInOffice, kStaffEvacuate, kStaffNone, kStaffEvacuate},
    {kStaffEvacuate, kStaffDoctorInMachineRoom, kStaffHandymanWorks, kStaffNone, kStaffEvacuate},
    {kStaffEvacuate, kStaffDoctorResearches, kStaffHandymanWorks, kStaffNone, kStaffEvacuate},
    {kStaffNurseInWard, kStaffEvacuate, kStaffEvacuate, kStaffNone, kStaffEvacuate},
    {kStaffInStaffRoom, kStaffInStaffRoom, kStaffInStaffRoom, kStaffNone, kStaffInStaffRoom},
    {kStaffEvacuate, kStaffEvacuate, kStaffEvacuate, kStaffNone, kStaffEvacuate},
    {kStaffDoctorInMachineRoom, kStaffEvacuate, kStaffHandymanWorks, kStaffNone, kStaffEvacuate},
    {kStaffEvacuate, kStaffDoctorInTheatre, kStaffHandymanWorks, kStaffNone, kStaffDoctorInMachineRoom},
    {kStaffEvacuate, kStaffDoctorIsTrained, kStaffEvacuate, kStaffNone, kStaffEvacuate},
    {kStaffNone, kStaffNone, kStaffNone, kStaffNone, kStaffNone},
}};

// Room types (the game's ROOM_*) and RoomInfo.state (RS_EDITED).
constexpr uint8_t kRoomPsych = 8;
constexpr uint8_t kRoomTheatre = 10;
constexpr uint8_t kRoomCardio = 12;
constexpr uint8_t kRoomScanner = 13;
constexpr uint8_t kRoomUltrascan = 14;
constexpr uint8_t kRoomBloodMachine = 15;
constexpr uint8_t kRoomXray = 16;
constexpr uint8_t kRoomGeneralDiag = 27;
constexpr uint8_t kRoomTraining = 22;
constexpr uint8_t kRoomStaffRoom = 25;

// Thing.orders: notice handed in (the game's ORD_RESIGN), and the player
// call that announces it.
constexpr uint8_t kOrderResign = 0x20;
// Thing.orders: a rest ordered (ORD_REST).
constexpr uint8_t kOrderRest = 0x1;
constexpr uint16_t kCallStaffResigns = 1;
constexpr uint8_t kRoomResearch = 28;
constexpr uint8_t kRoomWard = 9;
constexpr int8_t kRoomEdited = 4;
// RoomInfo.state of a room whose machine is due a repair (RS_REPAIR_DUE),
// and closed for it (RS_CLOSED).
constexpr int8_t kRoomRepairDue = 5;
constexpr int8_t kRoomClosed = 6;
// Thing.orders: going back to research or training (ORD_RETURN_TO_*).
constexpr uint8_t kOrderReturnToResearch = 0x10;
constexpr uint8_t kOrderReturnToTraining = 0x40;

// Object kinds (the game's OBJ_*) in a GP's office.
constexpr uint8_t kObjDesk = 1;
constexpr uint8_t kObjCabinet = 2;
constexpr uint8_t kObjChair = 6;
// A psychiatrist's bookcase; a ward's bed.
constexpr uint8_t kObjBookcase = 56;
constexpr uint8_t kObjBed = 8;

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

// calculate_speed (0x2c268), for staff at a cell's middle; a training
// room's speed is not ported, as there are none.
void calculate_speed(Thing &staff, const Balance &settings)
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

// doctor_cracks_up (0x3bd34 in src/doctor.cpp): a doctor, once, goes
// crazy: dressed so and with a new ability drawn within his grade's range.
// The advisor's count of crack-ups is not kept.
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
}

// modify_staff (0x71eb8 in src/environ.cpp) for member of staff `id`: due
// once game_turn (in 16 bits) passes Thing.modify_time, which it then sets
// gbv.modify_freq turns on; not for rivals' staff, applicants or
// receptionists. Speed, warmth and surroundings, then tiredness from work
// (not while wandering nor in a staff room), a doctor past
// gbv.crack_up_tired may crack up, and happiness and discontent move by
// tiredness, warmth and happiness. The status icons are not ported.
void modify_staff(World &world, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    static const Balance statics;
    const Balance &settings = world.settings ? *world.settings : statics;
    const Balance::Gbv &gbv = settings.gbv;
    auto now = uint16_t(game_turn);
    if (!(now > s.modify_time || s.modify_time - now > 0x7fff))
        return;
    s.modify_time = uint16_t(game_turn + gbv.modify_freq);
    if (s.owner >= 4 || s.subtype == kStaffReceptionist)
        return;
    calculate_speed(s, settings);
    check_warmth(world, s, world.finance ? world.finance->heating_level : kStartHeatingLevel);
    staff_check_around(world, s);
    if (s.state != kStaffWanders && (s.room == 0 || world.rooms[s.room].type != kRoomStaffRoom))
        s.tiredness = int16_t(s.tiredness + gbv.work_light);
    if (s.tiredness > int(gbv.crack_up_tired) && old_newrand(world.seed) % 1000 == 0)
        doctor_cracks_up(world, s, gbv);
    if (s.tiredness > int(gbv.crack_up_tired))
        s.happiness--;
    else if (s.tiredness < int(gbv.not_tired))
        s.happiness++;
    if (int16_t(s.warmth) > 80 || int16_t(s.warmth) < 20)
        s.happiness--;
    if (int16_t(s.happiness) < gbv.unhappy)
        s.discontent++;
    else if (int16_t(s.happiness) > gbv.happy)
        s.discontent--;
    if (int16_t(s.happiness) < 0)
        s.happiness = 0;
    else if (int16_t(s.happiness) > 100)
        s.happiness = 100;
    s.tiredness = std::clamp<int16_t>(s.tiredness, 0, 1000);
    if (int16_t(s.discontent) < 0)
        s.discontent = 0;
}

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

constexpr uint8_t kObjReceptionDesk = 11;
constexpr uint8_t kObjSurgeonScreen = 35;

// The complexes the staff's states give (S_COMP.LEX).
constexpr uint16_t kComplexGoToDesk = 0x50;
constexpr uint16_t kComplexCallPatient = 0x1e;
constexpr uint16_t kComplexPatientCured = 0xe6;
constexpr uint16_t kComplexPatientNotCured = 0xf0;
constexpr uint16_t kComplexWalkTo = 0xbe;
constexpr uint16_t kComplexSweepUp = 0x46;
constexpr uint16_t kComplexAfterSweeping = 0x30c;
// A handyman's to a room's door and in, and to the machine to repair.
constexpr uint16_t kComplexToRoomToRepair = 0x96;
constexpr uint16_t kComplexToMachine = 0x2d0;
// A doctor's at a GP's office: sitting at the desk, the filing cabinet
// (then back to the desk's state), the cabinet before waiting for the
// patient called, and the consultation.
constexpr uint16_t kComplexSitAtDesk = 10;
constexpr uint16_t kComplexCabinet = 0x14;
constexpr uint16_t kComplexCabinetThenWait = 0x32;
constexpr uint16_t kComplexConsultation = 0xfa;
// Out of the room by its door, and to a room's door and in.
constexpr uint16_t kComplexLeaveRoom = 0x136;
constexpr uint16_t kComplexToRoom = 0x14a;
// Getting off a door or an object (interrupt_complex).
constexpr uint16_t kComplexOffDoor = 0x4ce;
constexpr uint16_t kComplexOffObject = 0x1b8;
// Giving up the complex (abandon_complex), and a receptionist's at her desk.
constexpr uint16_t kComplexAbandon = 0xf0;
constexpr uint16_t kComplexAbandonDesk = 0x4ba;
// The complex the patient called to the desk runs (P_COMP.LEX).
constexpr uint16_t kComplexToDesk = 0x6e;
// The complexes a doctor gives the patient: called to the room's door and
// in, and to the chair in a GP's office.
constexpr uint16_t kComplexCalledToRoom = 0x28;
constexpr uint16_t kComplexSitOnChair = 0x82;
// The patient's complex out of their room by its door (P_COMP.LEX).
constexpr uint16_t kComplexWalkOut = 90;
// A nurse's in a ward: to the bed for the patient called, to the desk, to a
// patient in bed; and the patient's into bed (P_COMP.LEX).
constexpr uint16_t kComplexNurseToBed = 0x15e;
constexpr uint16_t kComplexNurseToDesk = 0x168;
constexpr uint16_t kComplexNurseVisit = 0x17c;
constexpr uint16_t kComplexIntoBed = 200;

// The complexes of a receptionist's idle tasks at the desk, after their
// count (the game's recept_tasks, src/tables.cpp).
constexpr std::array<uint16_t, 6> kReceptTasks = {5, 90, 100, 110, 120, 130};

// The game's showanim_angle (src/tables.cpp): the facing a handyman turns
// to to sweep, by his facing.
constexpr std::array<uint8_t, 4> kShowanimAngle = {1, 1, 2, 2};

// The game's special_around (src/tables.cpp): a cell and its eight
// neighbours, as cell offsets.
constexpr std::array<int16_t, 9> kSpecialAround = {0, 1, 128, -1, -128, -129, 129, -127, 127};

// The animation columns of kStaffAnimStarts with a name in the game.
constexpr size_t kAnimWalk = 0;
constexpr size_t kAnimStand = 5;

// MapCell.owner_flags: an outdoor cell.
constexpr uint8_t kCellOutdoor = 0x10;

struct Ctx {
    World &world;
    const AnimData &anims;
    const PersonComplexes &person;
    const StaffComplexes &staff;
    uint32_t game_turn;
};

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

void sack(const Ctx &c, uint16_t id, uint8_t mode);

// shall_i_resign (0x2bfc8) for member of staff `id`. With notice handed in
// (kOrderResign), they leave once it runs out: their call to the player
// (type 1) removed, sack_staff mode 1, and true. Otherwise, hired more than
// six to nine months (a draw) ago and discontented past gbv.resign_max,
// they hand in notice, 1500 turns, with a call to the player, and true.
// Poaching, the status icon and the advisor's word are not ported.
bool shall_i_resign(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    if (s.orders & kOrderResign) {
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

void evacuate_staff(const Ctx &c, uint16_t id);

// staff_nav_failure (0x2b5cc): heading from the corridor for a room built
// in the last 500 turns, back to the corridor (evacuate_staff); else stuck
// (SS_STUCK). The game also announces it for the player's staff.
void staff_nav_failure(const Ctx &c, uint16_t id)
{
    Thing &staff = c.world.things[id];
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
}

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
        c.world.map[tile].owner_flags != 1 << e.owner || room_at(c.world, tile)) {
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

bool obey_pager(const Ctx &c, uint16_t id);
bool take_a_break(const Ctx &c, uint16_t id);

// The player's policy (World::hospital), or its start before
// start_rooms_in_use.
const HospitalSettings &policy_of(const World &world)
{
    static const HospitalSettings statics;
    return world.hospital ? *world.hospital : statics;
}

// The decision of staff_wanders: a page answered (obey_pager), else a
// stroll of 3 cells back the way they came (85%), else, standing, a pause of
// 20-49 turns, a stroll of 3 cells in a random direction or one to a random
// cell within 3. Vaccinating and ordered rests are not ported.
void wander_decide(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &e = world.things[id];
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
    if (--e.countdown <= 0) {
        if (e.subtype == kStaffReceptionist && has_reception(world, e.owner) &&
            find_free_object_anywhere(world, id, kObjReceptionDesk)) {
            e.state = kStaffReceptionistFindsReception;
            return;
        }
        if (off_blueprint(world, id))
            return;
        if (shall_i_resign(c, id))
            return;
        e.countdown = int16_t(0x100 / e.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
        case kNavNoPath:
            e.countdown = 1;
            wander_decide(c, id);
            return;
        case kNavBlockedByPeople:
            play_staff_anim(c.anims, e, kAnimStand);
            e.countdown = 0;
            return;
        case kNavWalledIn:
            staff_nav_failure(c, id);
            return;
        default:
            if (e.facing != dir) {
                e.facing = uint8_t(dir);
                play_staff_anim(c.anims, e, kAnimWalk);
            }
            break;
        }
    }
    step_on(world, id);
}

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
    if (pos >= kMapCells || !has_litter(world.map[pos]) || world.map[pos].owner_flags != 1 << h.owner)
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

// The game's objects[].is_machine and repair_aniseq (src/tables.cpp), by
// object kind: machines, and the ani_seq_choice group a handyman plays
// repairing one (0 none).
constexpr std::array<uint8_t, 63> kIsMachine = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
                                                0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};
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
    uint32_t count = 0;
    while (choice[count])
        count++;
    uint8_t seq = choice[old_newrand(world.seed, "choose_aniseq") % count];
    s.state = kStaffPlayAniseq;
    s.next_state = next_state;
    s.seq_index = kAniSeqStart[seq % kAniSeqStart.size()];
    setup_next_aniseq(anims, s);
}

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

// handyman_works (0x7a8a8). Substate 0 decides: in a room, he pages its
// first worn machine and takes a repair page there (substate 8), or leaves;
// in the corridor, the most urgent repair if urgent enough
// (handyman_try_fix), else the jobs least done for their priority first,
// sweeping litter he finds or repairing; else he strolls to a random cell
// within 7 he can reach. Substates 2 and 3 sweep; 8 to 10 repair: once the
// room is closed (its doctor closes it, or no staff are in it) to the
// machine, its repair (the machine's sequence, a puff of smoke cleared),
// then its wear gone (its strength maybe one less), the page dropped and
// the room open again. Plants (4-6) have no pages yet; breaks are not
// ported.
void handyman_works(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &h = world.things[id];
    switch (h.substate) {
    case 0: {
        if (shall_i_resign(c, id))
            break;
        if (h.room) {
            RoomInfo &room = world.rooms[h.room];
            for (uint16_t o = room.first_object; o; o = world.things[o].room_obj_next) {
                const Thing &machine = world.things[o];
                if (machine.type == kThingObject && kIsMachine[machine.subtype % kIsMachine.size()] &&
                    machine.times_used > 0) {
                    if (machine.owner == 0)
                        thingpage_request(world.repair_pager, o, 100);
                    break;
                }
            }
            if (thingpage_respond(world, world.repair_pager, id, h.room, 0, c.game_turn)) {
                h.substate = 8;
                if (room.state != kRoomClosed)
                    room.state = kRoomRepairDue;
                break;
            }
            h.state = kStaffEvacuate;
            break;
        }
        if (thingpage_find_highest_priority(world.repair_pager) && handyman_try_fix(c, id))
            break;
        // The jobs, least done (for their priority) first: litter, plants
        // and repairs; plants have no pages.
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
                break;
            scores[idx] = 2000000;
            if ((idx == 0 && handyman_try_litter(c, id)) || (idx == 2 && handyman_try_fix(c, id)))
                return;
        }
        // Unsigned arithmetic into a signed int, as in the game.
        auto dx = int32_t(old_newrand(world.seed) % 15 - 7);
        auto dy = int32_t(old_newrand(world.seed) % 15 - 7);
        auto pos = uint16_t(cell_of(h) + (dy << 7) + dx);
        // The game reads past the map for cells off its end; here they are
        // not his land.
        if (pos < kMapCells && world.map[pos].owner_flags == 1 << h.owner && can_i_get_to(world, h, pos) &&
            !(world.map[pos].owner_flags & kCellOutdoor)) {
            h.next_state = h.resume_state = kStaffHandymanWorks;
            set_complex(world, id, kComplexWalkTo, pos);
            process(c, id);
        }
        break;
    }
    case 2:
        if (h.x.tile != h.dest_x || h.y.tile != h.dest_y) {
            h.substate = 0;
            break;
        }
        if (h.work_target == cell_of(h))
            h.work_target = 0;
        h.facing = kShowanimAngle[h.facing & 3];
        h.next_state = kStaffHandymanWorks;
        h.substate = 3;
        set_complex(world, id, kComplexSweepUp, 0);
        process(c, id);
        break;
    case 3: {
        // remove_litter (0x7a814); the hospital's litter count is not kept.
        MapCell &cell = world.map[cell_of(h)];
        if (cell.owner_flags == 1 << h.owner && has_litter(cell))
            cell.litter_shadow &= 0xf0;
        set_complex(world, id, kComplexAfterSweeping, 0);
        h.next_state = kStaffHandymanWorks;
        process(c, id);
        h.substate = 0;
        h.litter_count++;
        break;
    }
    case 8: {
        RoomInfo &room = world.rooms[h.room];
        if (room.state == kRoomClosed) {
            h.next_state = kStaffHandymanWorks;
            h.substate = 9;
            h.resume_state = kStaffEvacuate;
            h.object = thingpage_remind_me(world.repair_pager, id);
            set_complex(world, id, kComplexToMachine, 0);
            process(c, id);
        } else if (!person_in_room(world, h.room, kThingStaff, kStaffDoctor) &&
                   !person_in_room(world, h.room, kThingStaff, kStaffNurse) &&
                   !person_in_room(world, h.room, kThingStaff, kStaffSurgeon)) {
            room.state = kRoomClosed;
        } else {
            // The row is drawn first, as the game's code does.
            auto row = int16_t(old_newrand(world.seed) % uint32_t(room.height));
            auto column = int16_t(old_newrand(world.seed) % uint32_t(room.width));
            auto pos = uint16_t(room.pos + ((row << 7) + column));
            if (pos != cell_of(h) && can_i_get_to(world, h, pos)) {
                h.next_state = h.resume_state = kStaffHandymanWorks;
                set_complex(world, id, kComplexWalkTo, pos);
                process(c, id);
            }
        }
        break;
    }
    case 9: {
        Thing &machine = world.things[h.object];
        if (uint8_t seq = kRepairAniseq[machine.subtype % kRepairAniseq.size()])
            choose_aniseq(world, c.anims, h, seq, kStaffHandymanWorks);
        if (h.object)
            machine.smoke[old_newrand(world.seed) % 3] = 0;
        h.substate = 10;
        break;
    }
    case 10: {
        if (h.object) {
            Thing &machine = world.things[h.object];
            thingpage_cancel(world.repair_pager, h.object);
            if (machine.strength > 2 &&
                old_newrand(world.seed) % uint32_t(machine.strength) < uint32_t(machine.times_used))
                machine.strength--;
            machine.times_used = 0;
            machine.smoke = {};
        }
        RoomInfo &room = world.rooms[h.room];
        room.state = 0;
        set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
        h.resume_state = kStaffEvacuate;
        h.next_state = kStaffHandymanWorks;
        h.substate = 0;
        h.repair_count++;
        process(c, id);
        break;
    }
    default:
        break;
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
    if (--e.countdown <= 0) {
        if (off_blueprint(world, id))
            return;
        e.countdown = int16_t(0x100 / e.speed);
        int dir = person_head_for(world, id);
        switch (dir) {
        case kNavArrived:
            e.dest_room = 0;
            process(c, id);
            return;
        case kNavBlockedByPeople:
            play_staff_anim(c.anims, e, kAnimStand);
            e.countdown = 0;
            return;
        case kNavWalledIn:
            staff_nav_failure(c, id);
            return;
        case kNavNoPath:
            if (e.resume_state)
                fail_staff_complex(world, c.anims, id);
            else
                staff_nav_failure(c, id);
            return;
        default: {
            size_t stand = size_t(kStaffAnimStarts[e.subtype][kAnimStand]) + e.facing;
            if (e.facing != dir || (stand < c.anims.starts.size() && c.anims.starts[stand].frame == e.sprite)) {
                e.facing = uint8_t(dir);
                play_staff_anim(c.anims, e, kAnimWalk);
            }
            break;
        }
        }
    }
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

// staff_exit_room_check (0x2bbdc) as staff leave their room: a nurse frees
// her ward. Surgeons turning back into doctors and the training room's
// projector are not ported.
void staff_exit_room_check(World &world, const Thing &s)
{
    if (!s.room)
        return;
    RoomInfo &room = world.rooms[s.room];
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

// take_a_break (0x2adfc): staff rooms (SS_IN_STAFF_ROOM) are not ported, so
// no one goes for a rest.
bool take_a_break(const Ctx &, uint16_t)
{
    return false;
}

// interrupt_complex (0x427d8) for a member of staff: their complex, the
// complexes queued and to resume and their next and resume states cleared,
// centred on their cell; one using a door or object gets off it.
void staff_interrupt_complex(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    s.complex_pc = s.complex_queued = s.complex_queued2 = s.complex_resume = 0;
    s.next_state = 0;
    s.resume_state = 0;
    s.x.frac = s.y.frac = 0x80;
    s.countdown = 0;
    if (!s.object)
        return;
    const Thing &obj = world.things[s.object];
    if ((obj.type != kThingObject && obj.type != kThingDoor) || obj.user != id)
        return;
    set_complex(world, id, obj.type == kThingDoor ? kComplexOffDoor : kComplexOffObject, 0);
    process(c, id);
}

// receptionist_reset (0x98554): the receptionist leaves her reception; if
// no other desk's receptionist works at it, its queue is sent on
// (restart_all_queue_peeps) and the reception's entry freed.
void receptionist_reset(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (!r.room || world.rooms[r.room].type != kRoomReception)
        return;
    bool shared = false;
    for (uint16_t o = r.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; o && !shared;
         o = world.things[o].type_obj_next) {
        const Thing &desk = world.things[o];
        shared = o != r.object && desk.in_use != 0 && desk.user && world.things[desk.user].room == r.room;
    }
    if (!shared) {
        restart_all_queue_peeps(world, c.anims, c.person, r.room, c.game_turn);
        world.rooms[r.room] = RoomInfo{};
    }
    r.room = 0;
}

// private_interrupt_staff (0x2c484): stops what the member of staff does,
// frees the patient they deal with in a room (interrupt_person) and puts
// them in the state for where they are, their pages dropped; a
// receptionist leaves her reception (receptionist_reset); a handyman drops
// his repair pages, reopening the room of the machine he was to repair.
// Plant pages and nurses' vaccinations are not ported.
void private_interrupt_staff(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    uint16_t patient = s.slave;
    if (s.subtype == kStaffHandyman) {
        auto reopen = [](RoomInfo &room) {
            if (room.state == kRoomRepairDue || room.state == kRoomClosed)
                room.state = 0;
        };
        if (uint16_t target = thingpage_remind_me(world.repair_pager, id); target && world.things[target].room_id)
            reopen(world.rooms[world.things[target].room_id]);
        if (s.room && s.object && kIsMachine[world.things[s.object].subtype % kIsMachine.size()])
            reopen(world.rooms[s.room]);
        thingpage_cancel_my_response(world.repair_pager, id);
    }
    staff_interrupt_complex(c, id);
    if (patient && world.things[patient].type == kThingPatient && (s.room || s.subtype == kStaffReceptionist)) {
        s.slave = 0;
        interrupt_person(world, c.anims, c.person, patient, c.game_turn);
    }
    s.state = get_state(s.room ? world.rooms[s.room].type : 0, s);
    s.substate = 0;
    staff_abandon_pager_response(world, id);
    if (s.subtype == kStaffReceptionist)
        receptionist_reset(c, id);
}

// interrupt_staff (0x2c3bc): private_interrupt_staff, and for a surgeon the
// other surgeon holding the room.
void interrupt_staff_in(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    private_interrupt_staff(c, id);
    const Thing &s = world.things[id];
    if (s.subtype != kStaffSurgeon || !s.room)
        return;
    for (uint16_t partner : world.rooms[s.room].staff) {
        if (partner && partner != id) {
            if (world.things[partner].owner != kNoOwner)
                private_interrupt_staff(c, partner);
            return;
        }
    }
}

// abandon_complex (0x41f84) for a member of staff: runs kComplexAbandon
// (kComplexAbandonDesk for a receptionist with an object, who leaves her
// reception) for up to 12 steps, each from its destination cell, puts them
// back where they were unless they used their object or changed room,
// interrupts their slave (sent on unless in the same room) and stands them
// in resume_state.
void staff_abandon_complex(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    MapPos old_x = s.x;
    MapPos old_y = s.y;
    bool user = s.object && world.things[s.object].user == id;
    if (s.object && s.subtype == kStaffReceptionist) {
        set_complex(world, id, kComplexAbandonDesk, 0);
        s.room = 0;
    } else {
        set_complex(world, id, kComplexAbandon, 0);
    }
    uint16_t room = s.room;
    for (int i = 0; i < 12; i++) {
        move_node(world, id, {0x80, s.dest_x}, {0x80, s.dest_y});
        if (process(c, id) != 1)
            break;
    }
    if (!user && room == s.room)
        move_node(world, id, old_x, old_y);
    if (uint16_t patient = s.slave) {
        interrupt_person(world, c.anims, c.person, patient, c.game_turn);
        if (s.room == 0 || s.room != world.things[patient].room)
            start_person_going(world, c.anims, c.person, patient, c.game_turn);
    }
    s.state = s.resume_state;
    play_staff_anim(c.anims, s, kAnimStand);
}

// clear_my_mutex (0x2bb58): the member of staff no longer holds their room.
void clear_my_mutex(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return;
    std::array<uint16_t, 2> &holders = world.rooms[s.room].staff;
    if (holders[0] == id)
        holders[0] = 0;
    else if (holders[1] == id)
        holders[1] = 0;
}

// staff_picked_up (0x2becc): the member of staff leaves their room and what
// they were doing (abandon_complex) and is taken off the map (state
// kStaffHeld), World::held_staff_tile noting their cell. The interface's
// part (placing them) is hire_staff_after_turn's.
void staff_picked_up(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("staff_picked_up %u", unsigned(id));
    staff_exit_room_check(world, s);
    staff_abandon_complex(c, id);
    world.held_staff_tile = cell_of(s);
    clear_my_mutex(world, id);
    // remove_staff_from_map.
    remove_person_from_room_list(world, id);
    remove_node(world, id);
    s.state = kStaffHeld;
}

// sack_staff (0x2a804): the member of staff stops holding their room and
// what they do (interrupt_staff) and, unless mode 2, walks out of their room
// and to their hospital's first entrance, to go back to the applicants
// (kStaffBackToApplicants); mode 0 pays them a month's wage, mode 1 (notice
// run out) clears it, mode 2 removes them at once. Their skills no longer
// count (remove_expertise_staff, and the counts by going to kNoOwner). Not
// for staff already no one's. The status icons are not ported.
void sack(const Ctx &c, uint16_t id, uint8_t mode)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("sack_staff %u mode=%u", unsigned(id), unsigned(mode));
    clear_my_mutex(world, id);
    if (s.owner == kNoOwner)
        return;
    interrupt_staff_in(c, id);
    if (mode == 0 && world.ledger && world.balance && world.date)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, kTxSeverancePay, -s.salary);
    else if (mode == 1)
        s.orders &= uint8_t(~kOrderResign);
    if (mode != 2) {
        staff_exit_room_check(world, s);
        if (s.room)
            set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        set_complex(world, id, kComplexWalkTo,
                    world.rooms[get_room_info(world, kRoomEntrance, 0x65, s.owner)].door_tile);
        s.next_state = kStaffBackToApplicants;
        s.resume_state = 0;
        process(c, id);
    }
    remove_expertise_staff(world, id);
    s.work_delay = uint8_t(s.work_delay + s.work_delay / 4);
    s.happiness = 100;
    s.resign_count = 0;
    s.resign_time = 0;
    s.orders = 0;
    s.discontent = 0;
    s.icon_id = 0;
    s.owner = kNoOwner;
    if (mode == 2) {
        remove_person_from_room_list(world, id);
        trace_event("remove_thing %u type=%u", unsigned(id), unsigned(s.type));
        remove_node(world, id);
        s = Thing{};
    }
}

// surgeon_screen_free (0x2d11c): false for a surgeon whose room's surgeon
// screen is in use.
bool surgeon_screen_free(World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (s.subtype != kStaffSurgeon)
        return true;
    uint16_t screen = find_object(world, cell_of(s), kObjSurgeonScreen, false, false, s.owner);
    return !(screen && world.things[screen].in_use != 0);
}

// change_lockholder (0x2c9cc): the member of staff holds `room` in place of
// its first holder, who is interrupted and sent out (but in an operating
// theatre with a free place). Nothing if no one holds it.
void change_lockholder(const Ctx &c, uint16_t id, uint16_t room_id)
{
    RoomInfo &room = c.world.rooms[room_id];
    uint16_t *slot = room.staff[0] ? &room.staff[0] : room.staff[1] ? &room.staff[1] : nullptr;
    if (!slot)
        return;
    if (room.type != kRoomTheatre || (room.staff[0] && room.staff[1])) {
        uint16_t old = *slot;
        interrupt_staff_in(c, old);
        evacuate_staff(c, old);
    }
    *slot = id;
}

// stop_waiting_for_patient (0x3b234): the call dropped, back to the room's
// working state. The surgeon waiting with them is not ported.
void stop_waiting_for_patient(World &world, Thing &s)
{
    s.state = get_state(world.rooms[s.room].type, s);
    s.slave = 0;
}

// The member of staff holds their room (check_room_mutex), or is sent out.
bool holds_room(const Ctx &c, uint16_t id)
{
    if (check_room_mutex(c.world, id))
        return true;
    evacuate_staff(c, id);
    return false;
}

// doctor_in_office (0x3a6e0): to the room's free desk to sit at it
// (SS_DOCTOR_WORKS_AT_DESK), or out without one. Status icons are not
// ported.
void doctor_in_office(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!holds_room(c, id))
        return;
    if (find_object(world, cell_of(d), kObjDesk, true, true, d.owner)) {
        d.next_state = kStaffDoctorWorksAtDesk;
        d.resume_state = kStaffDoctorInOffice;
        set_complex(world, id, kComplexSitAtDesk, 0);
        process(c, id);
    } else {
        evacuate_staff(c, id);
    }
}

// doctor_works_at_desk (0x3a7a8): at each end of the desk's animation,
// out if the room is being edited; else, a quarter of the time, the next
// patient of the room's queue called (call_person_to_room, the filing
// cabinet visited on the way if there is one); 7% of the rest a visit to
// the cabinet; else, free (staff_free) and with the policy letting staff
// leave rooms and no one queueing, a page answered every work_delay + 1
// turns (2 * work_delay + 1 tired); first, they may resign or leave
// (shall_i_resign). Orders, breaks (no staff rooms) and status icons are
// not ported.
void doctor_works_at_desk(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    RoomInfo &room = world.rooms[d.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (old_newrand(world.seed) % 100 < 25) {
        uint16_t queue = room.queue_head;
        if (queue && call_person_to_room(world, c.anims, c.person, id, queue, kComplexCalledToRoom, room.door_tile,
                                         true, c.game_turn)) {
            if (find_object(world, cell_of(d), kObjCabinet, true, true, d.owner)) {
                d.next_state = kStaffDoctorWaitsForPatient;
                set_complex(world, id, kComplexCabinetThenWait, 0);
                process(c, id);
            } else {
                d.state = kStaffDoctorWaitsForPatient;
            }
        }
        return;
    }
    uint32_t pct = old_newrand(world.seed) % 100;
    if (pct >= 25 && pct < 32) {
        if (find_object(world, cell_of(d), kObjCabinet, true, true, d.owner)) {
            d.resume_state = kStaffDoctorInOffice;
            set_complex(world, id, kComplexCabinet, 0);
            process(c, id);
        }
        return;
    }
    if (!room.staff_free)
        return;
    const HospitalSettings &policy = policy_of(world);
    bool tired = d.tiredness > policy.rest_threshold;
    bool paged = false;
    if (tired && take_a_break(c, id)) {
        paged = true;
    } else if (policy.staff_leave_rooms && room.queue_head == 0 &&
               c.game_turn % (tired ? d.work_delay * 2 + 1 : d.work_delay + 1) == 0 && obey_pager(c, id)) {
        paged = true;
    }
    if (paged && room.queue_head)
        page_member_of_staff(world, kSkillDoctor, d.room, false, 1, d.owner);
}

// The examinations a doctor starts once the patient called has arrived
// (doctor_waits_for_patient_to_arrive), by room type: the doctor's complex
// (S_COMP.LEX) and the SS_DOCTOR_EXAMINES substate it leads to; complex 0
// for rooms not ported (the treatment rooms').
struct Examination {
    uint16_t complex;
    uint8_t substate;
};
constexpr std::array<Examination, kRoomTypes> kExaminations = [] {
    std::array<Examination, kRoomTypes> e{};
    e[kRoomCardio] = {0xb4, 30};
    e[kRoomScanner] = {0xd2, 50};
    e[kRoomPsych] = {0x37a, 60};
    e[kRoomUltrascan] = {0x492, 80};
    e[kRoomBloodMachine] = {0x398, 90};
    e[kRoomXray] = {0x276, 100};
    e[kRoomGeneralDiag] = {0x442, 110};
    return e;
}();

// The game's illness_abilities (src/tables.cpp): 0 for the illnesses the
// scanner examines with its second complex (kComplexScanAlt).
constexpr std::array<uint8_t, 0x24> kIllnessAbilities = {1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1,
                                                         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// The other complexes of the examinations (S_COMP.LEX, P_COMP.LEX for the
// patient's): the scanner's for illnesses without kIllnessAbilities; the
// psychiatrist at the bookcase first, and the patient on the couch; the
// cardiogram's patient on and off the machine, its doctor's idle, check and
// tune; general diagnosis's three examinations and its last; the doctor
// back to the room's work after one.
constexpr uint16_t kComplexScanAlt = 0x500;
constexpr uint16_t kComplexBookcase = 0x370;
constexpr uint16_t kComplexOnCouch = 0x1e0;
constexpr uint16_t kComplexCardioOn = 0xaa;
constexpr uint16_t kComplexCardioRun = 0x78;
constexpr uint16_t kComplexCardioJolt = 0xbe;
constexpr uint16_t kComplexCardioOff = 0xb4;
constexpr uint16_t kComplexCardioIdle = 0x10e;
constexpr uint16_t kComplexCardioCheck = 0x118;
constexpr uint16_t kComplexCardioTune = 0x122;
constexpr std::array<uint16_t, 3> kComplexGeneralDiag = {0x456, 0x460, 0x46a};
constexpr uint16_t kComplexGeneralDiagEnd = 0x474;
constexpr uint16_t kComplexBackToWork = 0x154;
// The psychiatrist's on a patient of kind kPatientBigHead.
constexpr uint16_t kComplexPsychHead = 500;
constexpr uint16_t kComplexPsychHeadDoctor = 0x2b2;
constexpr uint8_t kPatientBigHead = 3;

// send_person_somewhere_else (0x17d2c): the patient walks out of their room
// and on to their next treatment.
void send_person_somewhere_else(const Ctx &c, uint16_t id)
{
    Thing &p = c.world.things[id];
    p.next_state = kPsGoForTreatment;
    if (set_complex(c.world, id, kComplexWalkOut, c.world.rooms[p.room].door_inside_tile))
        process_person_complex(c.world, c.anims, c.person, id, c.game_turn);
}

// doctor_waits_for_patient_to_arrive (0x3aba4): once the patient called
// has arrived, both start the room's examination (a GP's consultation, the
// patient to the chair; the diagnosis rooms' in kExaminations), the doctor
// to SS_DOCTOR_EXAMINES; a cracked-up doctor first sends one patient in
// four (a draw from world.seed) on elsewhere. Until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes (forget_about_my_slave). The treatment rooms'
// examinations are not ported: their patients wait.
void doctor_waits_for_patient(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!d.slave) {
        d.state = get_state(world.rooms[d.room].type, d);
        return;
    }
    uint16_t pid = d.slave;
    Thing &patient = world.things[pid];
    const RoomInfo &room = world.rooms[d.room];
    if (patient.state == kPsArrivedFromCall) {
        patient.state = kPsSlaveReady;
        if (d.subtype == kStaffDoctor && d.crazy &&
            (old_newrand(world.seed, "doctor_waits_for_patient_to_arrive") & 3) == 0) {
            send_person_somewhere_else(c, pid);
            d.state = get_state(room.type, d);
            d.slave = 0;
            return;
        }
        patient.resume_state = 0;
        if (room.type == kRoomGp) {
            if (find_object(world, cell_of(patient), kObjChair, true, true, patient.owner)) {
                patient.next_state = kPsSlaveReady;
                if (set_complex(world, pid, kComplexSitOnChair, 0))
                    process_person_complex(world, c.anims, c.person, pid, c.game_turn);
                d.substate++;
            }
            d.next_state = kStaffDoctorExamines;
            d.substate = 1;
            set_complex(world, id, kComplexConsultation, 0);
            process(c, id);
            return;
        }
        const Examination &exam = kExaminations[room.type % kRoomTypes];
        if (!exam.complex)
            return;
        if (room.type == kRoomCardio) {
            patient.next_state = kPsSlaveReady;
            set_complex(world, pid, kComplexCardioOn, 0);
            process_person_complex(world, c.anims, c.person, pid, c.game_turn);
        } else if (room.type == kRoomPsych && find_object(world, cell_of(d), kObjBookcase, true, true, d.owner)) {
            set_complex(world, id, kComplexBookcase, 0);
        }
        d.next_state = kStaffDoctorExamines;
        d.substate = exam.substate;
        uint16_t complex = exam.complex;
        if (room.type == kRoomScanner && !kIllnessAbilities[patient.illness % kIllnessAbilities.size()])
            complex = kComplexScanAlt;
        set_complex(world, id, complex, 0);
        process(c, id);
        if (room.type == kRoomPsych) {
            patient.next_state = kPsSlaveReady;
            set_complex(world, pid, kComplexOnCouch, 0);
            process_person_complex(world, c.anims, c.person, pid, c.game_turn);
        }
        return;
    }
    if (--d.timer <= 0) {
        uint8_t result =
            call_person_again(world, c.anims, c.person, id, kComplexCalledToRoom, room.door_tile, c.game_turn);
        if (result == 0) {
            stop_waiting_for_patient(world, d);
        } else if (result == 1) {
            stop_waiting_for_patient(world, d);
            interrupt_person(world, c.anims, c.person, pid, c.game_turn);
            start_person_going(world, c.anims, c.person, pid, c.game_turn);
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexCalledToRoom, c.game_turn)) {
        stop_waiting_for_patient(world, d);
    }
}

// The examination over, unless its machine broke down
// (treatment_machine_used): the patient's diagnosis and what comes next
// (done_treatment), the doctor back to the room's work (complex
// kComplexBackToWork), free again.
void examination_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    Thing &patient = world.things[d.slave];
    if (treatment_machine_used(world, patient.treatment, cell_of(patient)))
        return;
    done_treatment(world, c.anims, id, d.slave, c.game_turn);
    d.next_state = kStaffDoctorInMachineRoom;
    set_complex(world, id, kComplexBackToWork, 0);
    process(c, id);
    d.slave = 0;
    world.rooms[d.room].staff_free = 1;
}

// The doctor runs `complex`, back in SS_DOCTOR_EXAMINES after it.
void examine_with(const Ctx &c, uint16_t id, uint16_t complex)
{
    Thing &d = c.world.things[id];
    d.next_state = kStaffDoctorExamines;
    set_complex(c.world, id, complex, 0);
    process(c, id);
}

// The patient runs `complex`, PS_SLAVE_READY after it.
void patient_does(const Ctx &c, uint16_t id, uint16_t complex)
{
    Thing &p = c.world.things[id];
    p.next_state = kPsSlaveReady;
    set_complex(c.world, id, complex, 0);
    process_person_complex(c.world, c.anims, c.person, id, c.game_turn);
}

// A draw from world.seed, as doctor_carries_out_examination's
// othervar.rand_pct.
uint32_t examination_pct(World &world)
{
    return old_newrand(world.seed, "doctor_carries_out_examination") % 100;
}

// doctor_carries_out_examination (0x530e0), by the doctor's substate: in a
// GP's office (1), the consultation over, back to the desk, the patient's
// diagnosis and what comes next (done_treatment), the doctor free again;
// at the cardiogram (30-33, 40), the patient on the machine, the doctor's
// checks and the jolts (the patient's animation speed drawn), then the
// patient off; general diagnosis (110, 111), one of three examinations,
// then the last; the psychiatrist (60) with a big-headed patient first;
// then (50, 80, 90, 100, 230) the end of the examination
// (examination_done). The treatment rooms' stages are not ported.
void doctor_examines(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    if (!pid)
        return;
    Thing &patient = world.things[pid];
    switch (d.substate) {
    case 1:
        d.state = kStaffDoctorWorksAtDesk;
        d.slave = 0;
        done_treatment(world, c.anims, id, pid, c.game_turn);
        world.rooms[d.room].staff_free = 1;
        break;
    case 30:
    case 31:
        if (patient.state != kPsSlaveReady) {
            uint32_t pct = examination_pct(world);
            uint32_t low = d.substate == 30 ? 30 : 33;
            if (pct >= low && pct < low + 2)
                examine_with(c, id, kComplexCardioIdle);
            break;
        }
        d.substate++;
        if (d.substate == 31)
            patient_does(c, pid, kComplexCardioRun);
        else
            examine_with(c, id, kComplexCardioCheck);
        break;
    case 32: {
        // Each test draws again, as the game's.
        uint32_t pct = examination_pct(world);
        if (pct < 5) {
            patient.anim_speed = 2;
        } else if (pct = examination_pct(world), pct >= 5 && pct < 30) {
            patient.anim_speed = 1;
        } else if (pct = examination_pct(world), pct >= 30 && pct < 35) {
            patient.anim_speed = 4;
        } else if (pct = examination_pct(world), pct >= 35 && pct < 50) {
            patient.anim_speed = kPeepAnimSpeed;
            d.substate = 33;
            patient_does(c, pid, kComplexCardioJolt);
            examine_with(c, id, kComplexCardioIdle);
            break;
        } else {
            examine_with(c, id, kComplexCardioTune);
            break;
        }
        examine_with(c, id, kComplexCardioCheck);
        break;
    }
    case 33:
        if (patient.state != kPsSlaveReady)
            break;
        d.substate = 40;
        patient_does(c, pid, kComplexCardioOff);
        break;
    case 40:
        if (patient.state != kPsSlaveReady)
            break;
        d.state = kStaffDoctorInMachineRoom;
        d.slave = 0;
        if (treatment_machine_used(world, patient.treatment, cell_of(patient)))
            break;
        done_treatment(world, c.anims, id, pid, c.game_turn);
        world.rooms[d.room].staff_free = 1;
        break;
    case 60:
        if (patient.subtype == kPatientBigHead) {
            if (set_complex(world, pid, kComplexPsychHead, 0))
                process_person_complex(world, c.anims, c.person, pid, c.game_turn);
            patient.next_state = kPsSlaveReady;
            if (find_object(world, cell_of(d), kObjBookcase, true, true, d.owner) &&
                set_complex(world, id, kComplexBookcase, 0))
                process(c, id);
            if (set_complex(world, id, kComplexPsychHeadDoctor, 0))
                process(c, id);
            d.next_state = kStaffDoctorExamines;
        }
        d.substate = 230;
        break;
    case 110:
        d.substate = 111;
        examine_with(c, id,
                     kComplexGeneralDiag[old_newrand(world.seed, "doctor_carries_out_examination") %
                                         kComplexGeneralDiag.size()]);
        break;
    case 111:
        d.substate = 230;
        examine_with(c, id, kComplexGeneralDiagEnd);
        break;
    case 50:
    case 80:
    case 90:
    case 100:
    case 230:
        examination_done(c, id);
        break;
    default:
        break;
    }
}

// doctor_in_machine_room (0x3b2c4): at each end of their animation, a
// member of staff in a room with machines (the diagnosis rooms) goes out if
// the room is being edited; else, a quarter of the time, the next patient
// of the room's queue is called (call_person_to_room) unless the room is
// closed; 2% of the rest, or always on the cell inside the door, they walk
// to a random cell of the room (1% of those its middle) they can get to;
// else, free (staff_free) and with the policy letting staff leave rooms and
// no one queueing, a page answered every work_delay + 1 turns
// (2 * work_delay + 1 tired), another doctor paged if a queue is left.
// First, they may resign or leave (shall_i_resign), and a room due a
// repair is closed. A doctor in an operating theatre goes to
// SS_DOCTOR_IN_THEATRE; the theatre's calls (by its lead surgeon), breaks
// (no staff rooms), nurses' vaccinations and status icons are not ported.
void doctor_in_machine_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    RoomInfo &room = world.rooms[d.room];
    if (room.type == kRoomTheatre && d.subtype == kStaffDoctor) {
        d.state = kStaffDoctorInTheatre;
        return;
    }
    d.orders &= uint8_t(~(kOrderReturnToResearch | kOrderReturnToTraining));
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (room.state == kRoomRepairDue)
        room.state = kRoomClosed;
    if (old_newrand(world.seed) % 100 < 25) {
        uint16_t queue = room.queue_head;
        if (room.state != kRoomClosed && queue && room.type != kRoomTheatre &&
            call_person_to_room(world, c.anims, c.person, id, queue, kComplexCalledToRoom, room.door_tile, true,
                                c.game_turn)) {
            play_staff_anim(c.anims, d, kAnimStand);
            d.state = kStaffDoctorWaitsForPatient;
        }
        return;
    }
    uint32_t pct = old_newrand(world.seed) % 100;
    uint16_t tile = cell_of(d);
    if ((pct >= 25 && pct < 27) || tile == room.door_inside_tile) {
        const RoomInfo &here = world.rooms[room_at(world, tile)];
        uint16_t pos;
        if (old_newrand(world.seed) % 100 == 25) {
            pos = uint16_t(((here.height / 2) << 7) + here.width / 2 + here.pos);
        } else {
            // The row is drawn first, as the game's code does.
            auto row = int16_t(old_newrand(world.seed) % uint32_t(here.height));
            auto column = int16_t(old_newrand(world.seed) % uint32_t(here.width));
            pos = uint16_t((row << 7) + column + here.pos);
        }
        if (tile != pos && can_i_get_to(world, d, pos)) {
            d.next_state = d.resume_state = kStaffDoctorInMachineRoom;
            set_complex(world, id, kComplexWalkTo, pos);
            process(c, id);
        }
        return;
    }
    if (!room.staff_free)
        return;
    const HospitalSettings &policy = policy_of(world);
    bool tired = d.tiredness > policy.rest_threshold;
    bool paged = false;
    if (tired && take_a_break(c, id)) {
        paged = true;
    } else if (policy.staff_leave_rooms && room.queue_head == 0 &&
               c.game_turn % (tired ? d.work_delay * 2 + 1 : d.work_delay + 1) == 0 && obey_pager(c, id)) {
        paged = true;
    }
    if (paged && room.queue_head)
        page_member_of_staff(world, kRoomDoctorSkills[room.type % kRoomTypes], d.room, false, 1, d.owner);
}

// find_peep (0x1720c): a person of hospital `owner` in room `room` of thing
// type `type` in state `state` (0 for any): the first in its list of
// people, or with `random` one drawn from world.seed; 0 if none.
uint16_t find_peep(World &world, uint16_t room, bool random, uint8_t type, uint8_t state, uint8_t owner)
{
    auto matches = [&](const Thing &t) {
        return t.owner == owner && (!type || t.type == type) && (!state || t.state == state);
    };
    uint16_t count = 1;
    if (random) {
        count = 0;
        for (uint16_t p = world.rooms[room].first_person; p; p = world.things[p].room_next)
            count = uint16_t(count + matches(world.things[p]));
        if (count == 0)
            return 0;
        count = uint16_t(old_newrand(world.seed, "find_peep") % count + 1);
    }
    for (uint16_t p = world.rooms[room].first_person; p; p = world.things[p].room_next)
        if (matches(world.things[p]) && --count == 0)
            return p;
    return 0;
}

// find_unallocated_bed (0x6c79c): a bed in ward `room` no one uses nor is
// given (a patient in the room whose object it is); 0 if none.
uint16_t find_unallocated_bed(const World &world, uint16_t room)
{
    for (uint16_t bed = world.rooms[room].first_object; bed; bed = world.things[bed].room_obj_next) {
        if (world.things[bed].subtype != kObjBed || world.things[bed].user != 0)
            continue;
        uint16_t p = world.rooms[room].first_person;
        while (p && !(world.things[p].type == kThingPatient && world.things[p].object == bed))
            p = world.things[p].room_next;
        if (!p)
            return bed;
    }
    return 0;
}

// reinit_nurse (0x6c640): the nurse in the list of the room she stands in
// (not an entrance) and in its state, or in the corridor's.
void reinit_nurse(World &world, uint16_t id)
{
    Thing &nurse = world.things[id];
    remove_person_from_room_list(world, id);
    if (uint16_t room = room_at(world, cell_of(nurse))) {
        if (world.rooms[room].type != kRoomEntrance)
            insert_person_in_room_list(world, room, id);
        nurse.state = get_state(world.rooms[room].type, nurse);
    } else {
        nurse.state = get_state(0, nurse);
    }
    nurse.substate = 0;
}

// nurse_in_ward (0x6c8d8). Substate 0, at each turn: the ward in use (unless
// edited); out if she cannot hold it or it is being edited; she may resign,
// answer a page every work_delay + 1 turns (2 * work_delay + 1 tired) as
// the policy lets staff leave rooms; else the queue's next patient called
// to a free bed (find_unallocated_bed); else, 10% of the time, to the
// desk; else to a patient in bed she did not visit last (find_peep, drawn
// from world.seed); else to a random cell of the ward (5% its middle).
// Substate 1: the patient called arrived, she examines them
// (affect_diagnosis) and they go to bed (PS_IN_BED); until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes. Breaks, vaccinations and status icons are not
// ported.
void nurse_in_ward(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t desk = find_object(world, cell_of(nurse), kObjDesk, false, false, nurse.owner);
    RoomInfo &room = world.rooms[nurse.room];
    if (nurse.substate == 0) {
        if (room.state != kRoomEdited)
            room.state = 1;
        if (!holds_room(c, id) || shall_i_resign(c, id))
            return;
        if ((nurse.orders & kOrderRest) && take_a_break(c, id))
            return;
        const HospitalSettings &policy = policy_of(world);
        bool tired = nurse.tiredness > policy.rest_threshold;
        if (policy.staff_leave_rooms && c.game_turn % (tired ? nurse.work_delay * 2 + 1 : nurse.work_delay + 1) == 0 &&
            obey_pager(c, id))
            return;
        if (tired && take_a_break(c, id))
            return;
        if (room.being_edited || room.state == kRoomEdited) {
            evacuate_staff(c, id);
            return;
        }
        if (uint16_t queue = room.queue_head) {
            if (uint16_t bed = find_unallocated_bed(world, nurse.room)) {
                if (call_person_to_room(world, c.anims, c.person, id, queue, kComplexCalledToRoom, room.door_tile,
                                        false, c.game_turn)) {
                    nurse.object = bed;
                    nurse.next_state = kStaffNurseInWard;
                    nurse.substate = 1;
                    set_complex(world, id, kComplexNurseToBed, 0);
                    process(c, id);
                    return;
                }
            }
        }
        if (desk && nurse.work_target != desk && old_newrand(world.seed) % 100 < 10) {
            nurse.work_target = desk;
            set_complex(world, id, kComplexNurseToDesk, 0);
            nurse.next_state = kStaffNurseInWard;
            process(c, id);
            return;
        }
        if (uint16_t patient = find_peep(world, nurse.room, true, kThingPatient, kPsInBed, nurse.owner)) {
            if (patient != nurse.work_target) {
                nurse.work_target = patient;
                nurse.object = world.things[patient].object;
                nurse.next_state = kStaffNurseInWard;
                set_complex(world, id, kComplexNurseVisit, 0);
                process(c, id);
                return;
            }
        }
        uint16_t cur = cell_of(nurse);
        const RoomInfo &here = world.rooms[room_at(world, cur)];
        uint32_t pct = old_newrand(world.seed) % 100;
        uint16_t target;
        if (pct >= 15 && pct < 20) {
            target = uint16_t(here.pos + (((here.height / 2) << 7) + here.width / 2));
        } else {
            // The row is drawn first, as the game's code does.
            auto row = int16_t(old_newrand(world.seed) % uint32_t(here.height));
            auto column = int16_t(old_newrand(world.seed) % uint32_t(here.width));
            target = uint16_t(here.pos + ((row << 7) + column));
        }
        if (cur != target && can_i_get_to(world, nurse, target)) {
            nurse.next_state = nurse.resume_state = kStaffNurseInWard;
            set_complex(world, id, kComplexWalkTo, target);
            process(c, id);
            return;
        }
        pct = old_newrand(world.seed) % 100;
        if (pct >= 0x46 && pct < 0x50)
            nurse.work_target = 0;
        return;
    }
    if (nurse.substate != 1)
        return;
    uint16_t pid = nurse.slave;
    Thing &patient = world.things[pid];
    if (patient.state == kPsArrivedFromCall) {
        affect_diagnosis(world, nurse, patient);
        nurse.substate = 0;
        nurse.slave = 0;
        patient.object = nurse.object;
        nurse.object = 0;
        set_complex(world, pid, kComplexIntoBed, 0);
        patient.next_state = kPsInBed;
        process_person_complex(world, c.anims, c.person, pid, c.game_turn);
    } else if (--nurse.timer <= 0) {
        uint8_t result =
            call_person_again(world, c.anims, c.person, id, kComplexCalledToRoom, room.door_tile, c.game_turn);
        if (result == 0) {
            nurse.substate = 0;
            nurse.slave = 0;
        } else if (result == 1) {
            nurse.substate = 0;
            nurse.slave = 0;
            interrupt_person(world, c.anims, c.person, pid, c.game_turn);
            start_person_going(world, c.anims, c.person, pid, c.game_turn);
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexCalledToRoom, c.game_turn)) {
        nurse.substate = 0;
        nurse.slave = 0;
    }
}

// allocate_reception_roominfo (0x486a8): takes a free entry of
// World::rooms as a reception of `owner`. Returns it, 0 if there is none.
uint16_t allocate_reception_roominfo(World &world, uint8_t owner)
{
    for (uint16_t i = 1; i < world.rooms.size(); i++) {
        RoomInfo &room = world.rooms[i];
        if (room.type == kRoomFree) {
            room.type = kRoomReception;
            room.pos = 0x3fff;
            room.width = room.height = 1;
            room.state = 0;
            room.owner = owner;
            room.queue_capacity = 0x14;
            trace_event("reception %u", unsigned(i));
            return i;
        }
    }
    return 0;
}

// receptionist_finds_reception (0x97ce0). Her status icons are not ported.
void receptionist_finds_reception(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    uint16_t desk = has_reception(world, r.owner) ? find_free_object_anywhere(world, id, kObjReceptionDesk) : 0;
    if (!desk) {
        staff_in_corridor(r, c.anims);
        return;
    }
    r.next_state = kStaffReceptionistSetup;
    r.object = desk;
    set_complex(world, id, kComplexGoToDesk, world.things[desk].use_pos[0]);
    r.resume_state = kStaffReceptionistFindsReception;
    process(c, id);
    r.slave = 0;
}

// receptionist_setup (0x97de0).
void receptionist_setup(World &world, uint16_t id)
{
    Thing &r = world.things[id];
    uint16_t room = 0;
    for (uint16_t o = r.owner == 0 ? world.first_of_kind[kObjReceptionDesk] : 0; o; o = world.things[o].type_obj_next) {
        const Thing &desk = world.things[o];
        if (o == r.object || desk.in_use == 0 || std::abs(desk.x.tile - r.x.tile) > 10 ||
            std::abs(desk.y.tile - r.y.tile) > 10)
            continue;
        if (desk.user) {
            room = world.things[desk.user].room;
            break;
        }
    }
    r.room = room ? room : allocate_reception_roominfo(world, r.owner);
    r.state = kStaffReceptionistWorks;
}

// receptionist_works (0x97fac). othervar.rand_pct, which it overwrites, is
// not kept. Her status icons are not ported.
void receptionist_works(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (r.room == 0 || c.game_turn % r.anim_speed != 0)
        return;
    r.sprite = c.anims.frames[r.sprite].next;
    if (!(c.anims.frames[r.sprite].flags & 1))
        return;
    uint16_t head = reception_queue_head(world, r.room);
    if (head) {
        uint16_t p = head;
        while (p && !reception_emergency(world, p))
            p = reception_queue_next(world, p);
        if (!p)
            p = head;
        for (; p; p = reception_queue_next(world, p)) {
            // The game keeps the capacity in a u16.
            if (uint16_t(all_queue_capacity(world, world.things[p].treatment, true, r.owner)) == 0 || !r.object)
                continue;
            uint16_t pos = world.things[r.object].use_pos[1];
            if (call_person_to_room(world, c.anims, c.person, id, p, kComplexToDesk, pos, false, c.game_turn)) {
                r.next_state = kStaffReceptWaitsForPatient;
                set_complex(world, id, kComplexCallPatient, 0);
                process(c, id);
                break;
            }
        }
        return;
    }
    if (old_newrand(world.seed) % 100 < 30) {
        r.next_state = kStaffReceptionistWorks;
        uint16_t action = kReceptTasks[old_newrand(world.seed) % kReceptTasks[0] + 1];
        set_complex(world, id, action, 0);
        process(c, id);
    } else {
        play_staff_anim(c.anims, r, 2);
        r.state = kStaffReceptionistWorks;
    }
}

// recept_waits_for_patient (0x982a8). othervar.recept_call_timer, which it
// clears, is not kept.
void recept_waits_for_patient(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    if (!r.slave) {
        r.state = kStaffReceptionistWorks;
        return;
    }
    Thing &patient = world.things[r.slave];
    if (patient.state == kPsArrivedFromCall) {
        set_complex(world, id,
                    reception_cure_available(world, r.slave) ? kComplexPatientCured : kComplexPatientNotCured, 0);
        patient.facing = kOppositeAngle[r.facing & 3];
        r.next_state = kStaffReceptAdmitsPatient;
        process(c, id);
    } else if (--r.timer <= 0) {
        if (!r.object)
            return;
        uint16_t pos = world.things[r.object].use_pos[1];
        uint8_t result = call_person_again(world, c.anims, c.person, id, kComplexToDesk, pos, c.game_turn);
        if (result == 0) {
            r.state = kStaffReceptionistWorks;
            r.slave = 0;
        } else if (result == 1) {
            r.state = kStaffReceptionistWorks;
            interrupt_person(world, c.anims, c.person, r.slave, c.game_turn);
            start_person_going(world, c.anims, c.person, r.slave, c.game_turn);
            r.slave = 0;
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexToDesk, c.game_turn)) {
        r.state = kStaffReceptionistWorks;
        r.slave = 0;
    }
}

// recept_admits_patient (0x984a0).
void recept_admits_patient(World &world, uint16_t id)
{
    Thing &r = world.things[id];
    if (!r.slave)
        return;
    reception_serves_patient(world, id, r.slave);
    RoomInfo &room = world.rooms[r.room];
    room.state = 0;
    room.patients_seen++;
    r.slave = 0;
    r.state = kStaffReceptionistWorks;
}
} // namespace

uint16_t staffrand(uint32_t &seed)
{
    seed = std::rotr(seed * 0x24a1u + 0x24dfu, 13);
    return uint16_t(seed);
}

uint32_t normal_staffrand(uint32_t &seed, uint32_t low, uint32_t high)
{
    int32_t range = int32_t((high - low) / 2 + 1);
    if (range <= 0)
        range = 1;
    uint32_t sum = uint32_t(staffrand(seed) % range);
    sum += uint32_t(staffrand(seed) % range);
    return sum + low;
}

std::array<uint8_t, 3> reference_counts(const LanguageText &text)
{
    std::array<uint8_t, 3> counts{};
    const TextSection sections[3] = {TextSection::RefGeneral, TextSection::RefGood, TextSection::RefBad};
    for (size_t s = 0; s < 3; s++) {
        // The game's counter is a byte, and the list ends at a string starting
        // with '.'.
        uint8_t i = 0;
        while (i < text.size(sections[s]) && !text.at(sections[s], i).starts_with('.'))
            i++;
        counts[s] = i;
    }
    return counts;
}

int count_applicants(const World &world, uint8_t kind)
{
    int n = 0;
    for (const Thing &t : world.things)
        n += t.type == kThingStaff && t.subtype == kind && t.owner == kNoOwner &&
             (t.state == kStaffApplicant || t.state == kStaffHeld);
    return n;
}

void appraise_staff(Thing &staff, const Balance &settings)
{
    const Balance::Gbv &gbv = settings.gbv;
    int32_t value = staff.ability / gbv.salary_ability_divisor + settings.staff[staff.subtype].min_salary;
    // The game reads SalaryAdd[3..8] (gbv.salary_add, include/hosp.h).
    const uint16_t bits[6] = {kSkillJunior,       kSkillDoctor,     kSkillSurgeon,
                              kSkillPsychiatrist, kSkillConsultant, kSkillResearcher};
    for (size_t i = 0; i < 6; i++)
        if (staff.skills & bits[i])
            value += gbv.salary_add[i + 3];
    staff.wage_demand = value;
    int happiness = int16_t(staff.happiness);
    happiness += staff.salary < value ? gbv.salary_too_low : gbv.salary_too_high;
    staff.happiness = uint16_t(std::clamp(int(int16_t(happiness)), 0, 100));
}

uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn)
{
    uint16_t id = 0;
    for (size_t i = 1; i < world.things.size() - kStaffReserve; i++) {
        if (world.things[i].type == kThingFree) {
            id = uint16_t(i);
            break;
        }
    }
    if (!id)
        return 0;
    uint32_t &seed = market.seed;
    Thing &s = world.things[id];
    s.subtype = kind;
    s.type = kThingStaff;
    s.facing = 0;
    s.owner = kNoOwner;
    s.layers[5] = kLayer5Count[kind] ? uint8_t(staffrand(seed) % kLayer5Count[kind] + 1) : 0;
    for (size_t layer = 6; layer <= 9; layer++)
        s.layers[layer] = uint8_t(staffrand(seed) & 3);
    if (kind == kStaffNurse || kind == kStaffReceptionist)
        s.layers[5] = 1;
    s.hire_expiry = game_turn + 2500 + staffrand(seed) % 2500;
    s.work_delay = uint8_t(normal_staffrand(seed, 1, 10));
    s.happiness = 100;
    s.warmth = 50;
    s.state = kStaffApplicant;

    const Balance::Gbv &gbv = settings.gbv;
    uint32_t ability_min = 30, ability_max = 1000;
    switch (kind) {
    case kStaffNurse:
        s.skills = kSkillNurse;
        s.female = 1;
        s.face_head = 12;
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        break;
    case kStaffDoctor:
        ability_min = gbv.doctor_threshold;
        ability_max = gbv.consultant_threshold;
        s.skills |= kSkillDoctor;
        if (market.shrink_rate > 0 && staffrand(seed) % market.shrink_rate == 0)
            s.skills |= kSkillPsychiatrist;
        if (market.surgeon_rate > 0 && staffrand(seed) % market.surgeon_rate == 0)
            s.skills |= kSkillSurgeon;
        if (market.researcher_rate > 0 && staffrand(seed) % market.researcher_rate == 0)
            s.skills |= kSkillResearcher;
        if (market.consultant_rate > 0 && staffrand(seed) % market.consultant_rate == 0) {
            s.skills |= kSkillConsultant;
            ability_min = gbv.consultant_threshold;
            ability_max = 1000;
        } else if (market.junior_rate > 0 && staffrand(seed) % market.junior_rate == 0) {
            s.skills |= kSkillJunior;
            ability_min = 30;
            ability_max = gbv.doctor_threshold;
        }
        s.female = staffrand(seed) % 100 < 53;
        s.face_head = uint8_t(staffrand(seed) % 5);
        s.face_body = uint8_t(staffrand(seed) % 5);
        s.face_legs = uint8_t(staffrand(seed) % 5);
        if (s.layers[5] == 2) {
            s.face_legs += 5;
            s.face_body += 5;
            s.face_head += 5;
        }
        break;
    case kStaffHandyman:
        s.skills = kSkillHandyman;
        s.female = 0;
        s.prio_litter = 0x21;
        s.prio_plants = 0x21;
        s.prio_repair = 0x22;
        s.face_head = uint8_t(staffrand(seed) % 3 + 15);
        s.face_body = uint8_t(staffrand(seed) % 3 + 15);
        s.face_legs = uint8_t(staffrand(seed) % 3 + 15);
        break;
    case kStaffReceptionist:
        s.skills = kSkillReceptionist;
        s.female = 1;
        s.face_head = uint8_t(staffrand(seed) % 4 + 10);
        s.face_body = uint8_t(staffrand(seed) % 5 + 10);
        s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
        if (s.face_head == 12)
            s.face_head = 14;
        break;
    }
    s.skills |= extra_skills;
    s.ability = uint16_t(normal_staffrand(seed, ability_min, ability_max));
    appraise_staff(s, settings);
    s.salary = s.wage_demand;
    s.initial = uint8_t(staffrand(seed) % 26 + 'A');
    s.surname_a = uint8_t(staffrand(seed) % 33);
    s.surname_b = uint8_t(staffrand(seed) % 26);
    // The game divides by the counts unchecked; a language without
    // references would crash it.
    s.ref_general = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[0], 1));
    s.ref_good = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[1], 1));
    s.ref_bad = uint8_t(staffrand(seed) % std::max<uint8_t>(market.ref_counts[2], 1));
    return id;
}

void create_job_market(World &world, StaffMarket &market, const Balance &settings, int game_month, uint32_t game_turn)
{
    for (const Balance::StaffLevel &level : settings.staff_levels) {
        if (level.month == game_month) {
            market.seed = level.seed;
            market.wanted = {level.nurses, level.doctors, level.handymen, level.receptionists};
            market.shrink_rate = level.shrk_rate;
            market.surgeon_rate = level.surg_rate;
            market.researcher_rate = level.rsch_rate;
            market.consultant_rate = level.cons_rate;
            market.junior_rate = level.jr_rate;
            break;
        }
        if (level.month > game_month)
            break;
    }
    for (uint8_t kind = 0; kind <= 3; kind++) {
        int have = count_applicants(world, kind);
        for (int j = have; j < market.wanted[kind]; j++)
            place_staff_in_struct(world, market, settings, kind, 0, game_turn);
    }
}

uint16_t staff_held_anim(uint8_t kind)
{
    const PersonAnims &a = kStaffAnims[kind];
    return kHeldAnim[kind] == 0 ? a.walk : a.stand;
}

bool staff_place_ok(const World &world, int cell, uint8_t kind)
{
    if (cell < 0 || cell >= kMapCells || world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    // check_points_connected to the room's inside door cell, or to the door
    // of the first entrance (allhosps[player].entrances[0]).
    int door = world.rooms[world.entrances[kPlayer][0]].door_tile;
    uint8_t row = 0;
    if (uint16_t r = room_at(world, uint16_t(cell))) {
        const RoomInfo &room = world.rooms[r];
        if (room.state != 0 && room.state != 1 && kind != kStaffHandyman)
            return false;
        row = kRoomStaffStateRow[room.type % kRoomTypes];
        door = room.door_inside_tile;
    }
    if (kStaffStates[row][kind % 5] == kStaffNone)
        return false;
    OrthonavPath path;
    orthonav_init(world, path, cell % kMapSize, cell / kMapSize, door % kMapSize, door / kMapSize);
    return path.count != 0;
}

bool place_staff_on_map(World &world, const AnimData &anims, const Balance &settings, int cell, uint16_t id, bool force,
                        uint32_t game_turn)
{
    trace_event("place_staff %u tile=%d", unsigned(id), cell);
    if (id == 0 || id >= world.things.size() || cell < 0 || cell >= kMapCells)
        return false;

    if (!force && world.map[size_t(cell)].owner_flags != 1 << kPlayer)
        return false;
    Thing &s = world.things[id];
    s.owner = kPlayer;
    if (s.facing > 3)
        s.facing = 0;
    play_frame(s, anims, staff_held_anim(s.subtype));
    s.x = {0x80, uint8_t(cell % kMapSize)};
    s.y = {0x80, uint8_t(cell / kMapSize)};
    s.path.x1 = 0xff;
    if (uint16_t room = room_at(world, uint16_t(cell))) {
        uint8_t type = world.rooms[room].type;
        insert_person_in_room_list(world, room, id);
        s.state = get_state(type, s);
        // The advisor's word on staff placed where they cannot work is not
        // ported.
        if (s.state != kStaffEvacuate && s.subtype != kStaffHandyman &&
            (type != kRoomTraining || s.subtype != kStaffDoctor || (s.skills & kSkillConsultant)) &&
            type != kRoomResearch)
            change_lockholder(ctx_of(world, anims, game_turn), id, room);
    } else {
        s.state = get_state(0, s);
    }
    s.substate = 0;
    s.complex_action = 0;
    calculate_speed(s, settings);
    insert_node(world, id, s.x, s.y);
    // With no one held, a new hire, whose skills now count
    // (staff_with_skill). The staff counts are not kept.
    if (world.held_staff == 0) {
        s.resign_count = 0;
        s.resign_time = 0;
        s.orders = 0;
        s.discontent = 0;
        s.icon_id = 0;
        s.hire_expiry = 0;
        s.hire_time = int16_t(game_turn);
        set_expertise_staff(world, id);
    }
    if (s.subtype == kStaffNurse)
        reinit_nurse(world, id);
    else if (s.subtype == kStaffHandyman)
        s.work_target = s.work_target2 = 0;
    pager_arrival(world, id);
    if (s.room)
        world.rooms[s.room].staff_free = 1;
    world.held_staff = 0;
    world.held_staff_tile = 0;
    return true;
}

void staff_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn)
{
    const Ctx c{world, anims, person_complexes, staff_complexes, game_turn};
    Thing &s = world.things[id];
    modify_staff(world, id, game_turn);
    if (staff_complex_turn(world, anims, person_complexes, staff_complexes, id, game_turn))
        return;
    switch (s.state) {
    case kStaffApplicant:
        // remove_staff_in_struct.
        if (game_turn > s.hire_expiry)
            s = Thing{};
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
    case kStaffReceptionistFindsReception:
        receptionist_finds_reception(c, id);
        break;
    case kStaffReceptionistSetup:
        receptionist_setup(world, id);
        break;
    case kStaffReceptionistWorks:
        receptionist_works(c, id);
        break;
    case kStaffReceptWaitsForPatient:
        recept_waits_for_patient(c, id);
        break;
    case kStaffReceptAdmitsPatient:
        recept_admits_patient(world, id);
        break;
    case kStaffDoctorInOffice:
        doctor_in_office(c, id);
        break;
    case kStaffDoctorWorksAtDesk:
        doctor_works_at_desk(c, id);
        break;
    case kStaffDoctorWaitsForPatient:
        doctor_waits_for_patient(c, id);
        break;
    case kStaffDoctorExamines:
        doctor_examines(c, id);
        break;
    case kStaffDoctorInMachineRoom:
        doctor_in_machine_room(c, id);
        break;
    case kStaffEvacuate:
        evacuate_staff(c, id);
        break;
    case kStaffPlayAniseq:
        play_aniseq(c, s);
        break;
    case kStaffNurseInWard:
        nurse_in_ward(c, id);
        break;
    case kStaffBpGoto:
        peep_bp_goto_turn(world, anims, id, game_turn);
        break;
    case kStaffBackToApplicants:
        trace_event("back_to_applicants %u", unsigned(id));
        remove_node(world, id);
        s.state = kStaffApplicant;
        s.hire_time = int16_t(game_turn);
        break;
    default:
        break;
    }
}

void interrupt_staff(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    interrupt_staff_in(ctx_of(world, anims, game_turn), id);
}

void sack_staff(World &world, const AnimData &anims, uint16_t id, uint8_t mode, uint32_t game_turn)
{
    sack(ctx_of(world, anims, game_turn), id, mode);
}

bool put_back_held_staff(Game &game)
{
    World &world = game.world;
    if (!world.held_staff)
        return false;
    place_staff_on_map(world, game.anims(), game.settings(), world.held_staff_tile, world.held_staff, true,
                       game.turn_count);
    return true;
}

bool apply_staff_event(Game &game, const Event &event)
{
    World &world = game.world;
    switch (event.type) {
    case EV_MARK_EMPLOYING:
        // req2_mark_employing; the advisor's staff happiness total is not
        // ported.
        if (event.arg && event.arg < world.things.size())
            world.things[event.arg].state = kStaffHeld;
        return true;
    case EV_PLACE_STAFF:
        if (event.arg && event.arg < world.things.size()) {
            Thing &s = world.things[event.arg];
            // A picked-up member of staff is placed again for nothing.
            bool hiring = world.held_staff == 0;
            if (place_staff_on_map(world, game.anims(), game.settings(), event.value, event.arg, false,
                                   game.turn_count) &&
                hiring)
                change_balance(game.finance, game.balance, game.date.day, game.date.month, kTxHireStaff,
                               -(s.salary * 12) / 10);
        }
        return true;
    case EV_PICK_UP_STAFF:
        if (event.value && event.value < world.things.size()) {
            // Held even when the pick-up is refused below, as in the game.
            world.held_staff = event.value;
            const Ctx c = ctx_of(world, game.anims(), game.turn_count);
            if (!(world.things[event.value].orders & kOrderResign) && surgeon_screen_free(world, event.value)) {
                interrupt_staff_in(c, event.value);
                staff_picked_up(c, event.value);
            }
        }
        return true;
    case EV_STAFF_PICKED_UP:
        if (event.value && event.value < world.things.size())
            staff_picked_up(ctx_of(world, game.anims(), game.turn_count), event.value);
        return true;
    case EV_CANCEL_BUILD:
        // req2_exit_employing.
        if (!put_back_held_staff(game) && event.value && event.value < world.things.size())
            world.things[event.value].state = kStaffApplicant;
        return false;
    case EV_SACK_STAFF:
        if (event.value && event.value < world.things.size())
            sack_staff(world, game.anims(), event.value, uint8_t(event.arg), game.turn_count);
        return true;
    case EV_ANSWER_PLAYERCALL:
        if (event.value != kCallStaffResigns)
            return false;
        get_playercall(world, kCallStaffResigns, event.arg, kPlayer);
        return true;
    case EV_REFUSE_STAFF_DEMAND:
    case EV_RAISE_STAFF_SALARY:
    case EV_GIVE_STAFF_BONUS: {
        if (event.value == 0 || event.value >= world.things.size())
            return true;
        Thing &s = world.things[event.value];
        if (event.type == EV_REFUSE_STAFF_DEMAND &&
            old_newrand(world.seed, "process_turn") % uint32_t(s.resign_count + 1)) {
            sack_staff(world, game.anims(), event.value, 1, game.turn_count);
            return true;
        }
        if (event.type == EV_RAISE_STAFF_SALARY) {
            s.salary += event.arg;
            s.happiness = 100;
        } else if (event.type == EV_GIVE_STAFF_BONUS) {
            // pay_staff_bonus (0x2b998) for one member of staff; happiness,
            // clamped, is then set to 100 anyway.
            change_balance(game.finance, game.balance, game.date.day, game.date.month, kTxPersonalBonus,
                           -int32_t(event.arg), event.value);
            s.happiness = 100;
        }
        s.orders &= uint8_t(~kOrderResign);
        game.advisor.event_flags &= ~kAdvisorEventResign;
        s.discontent = 0;
        return true;
    }
    default:
        return false;
    }
}

} // namespace th
