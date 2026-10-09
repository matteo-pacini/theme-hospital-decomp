#include "core/world/staff.h"

#include "core/data/blocks.h"
#include "core/world/arrivals.h"
#include "core/world/complex_tables.h"
#include "core/world/events.h"
#include "core/world/game.h"
#include "core/world/icons.h"
#include "core/world/litter.h"
#include "core/world/nav.h"
#include "core/world/object_tables.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/research.h"
#include "core/world/rooms.h"
#include "core/world/staff_complex.h"
#include "core/world/trace.h"
#include "core/world/training.h"
#include "core/world/vaccine.h"

#include <algorithm>
#include <bit>
#include <cstdlib>

namespace th
{

namespace
{

// allocate_thing's reserve for new staff (place_staff_in_struct).
constexpr size_t kStaffReserve = 0x19;

// SFX_PLAC_ST2 (include/enums.h): a member of staff put down.
constexpr uint8_t kSfxPlaceStaff = 0x07;

// staff[kind].layer5_count (src/tables.cpp): the layer-5 variants an
// applicant picks from, 0 for none.
constexpr std::array<uint8_t, 5> kLayer5Count = {0, 2, 1, 0, 1};

// staff[kind].anim (src/tables.cpp): the staff_anim_starts entry shown
// placed or held, 5 (standing) or 0 (walking).
constexpr std::array<uint8_t, 5> kHeldAnim = {5, 5, 5, 0, 5};

// Staff states the game's SS_* name that kStaffStates uses, and the two the
// operating theatre's surgeons wait in.
constexpr uint8_t kStaffSurgeonWaits = 0x10;
constexpr uint8_t kStaffAssistantReady = 0x18;
constexpr uint8_t kStaffDoctorInMachineRoom = 0x24;
constexpr uint8_t kStaffNurseInWard = 0x32;
constexpr uint8_t kStaffDoctorResearches = 0x3e;
constexpr uint8_t kStaffDoctorInTheatre = 0x42;
constexpr uint8_t kStaffDoctorIsTrained = 0x46;
constexpr uint8_t kStaffConsultantTrains = 0x48;
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
constexpr uint8_t kRoomPharmacy = 11;
constexpr uint8_t kRoomInflation = 17;
// Treatment rooms whose work is a diagnosis room's (kExaminations).
constexpr uint8_t kRoomHairRestorer = 19;
constexpr uint8_t kRoomSlackTongue = 20;
constexpr uint8_t kRoomFracture = 21;
constexpr uint8_t kRoomElectrolysis = 23;
constexpr uint8_t kRoomJellyVat = 24;
constexpr uint8_t kRoomDecontamination = 30;
constexpr uint8_t kRoomStaffRoom = 25;

// Thing.orders: notice handed in (the game's ORD_RESIGN), and the player
// call that announces it.
constexpr uint8_t kOrderResign = 0x20;
// Thing.orders: a rest ordered (ORD_REST), and leaving the room ordered
// (ORD_LEAVE_ROOM). EV_ORDER_REST, EV_ORDER_TRAIN, EV_ORDER_LEAVE_ROOM and
// command 0x60 replace the low four bits.
constexpr uint8_t kOrderRest = 0x1;
constexpr uint8_t kOrderTrain = 0x4;
constexpr uint8_t kOrderLeaveRoom = 0x8;
// The command, unnamed in the game, that orders 2.
constexpr uint8_t kEvOrder2 = 0x60;
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
// gamevar.announce's kinds a promotion is announced with (ANN_*), and the
// value of ANN_PROMOTED for a training room's skill.
constexpr uint8_t kAnnJuniorToDoctor = 0x0a;
constexpr uint8_t kAnnDoctorToConsultant = 0x0d;

// Object kinds (the game's OBJ_*) in a GP's office.
constexpr uint8_t kObjDesk = 1;
constexpr uint8_t kObjCabinet = 2;
constexpr uint8_t kObjChair = 6;
// A psychiatrist's bookcase; a ward's bed.
constexpr uint8_t kObjBookcase = 56;
constexpr uint8_t kObjBed = 8;
// A staff room's sofa, pool table and video game.
constexpr uint8_t kObjSofa = 19;
constexpr uint8_t kObjPoolTable = 10;
constexpr uint8_t kObjVideoGame = 57;

// The game's staff_abilities (src/tables.cpp), by staff kind: [0] plays the
// video game, [1] plays pool.
constexpr std::array<std::array<uint8_t, 2>, 5> kStaffAbilities = {{{1, 0}, {1, 1}, {0, 1}, {0, 0}, {0, 0}}};

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

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
// receptionists. Speed, warmth and surroundings, then tiredness from work
// (not while wandering nor in a staff room), and modify_staff_mood.
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
    if (s.owner >= 4 || s.subtype == kStaffReceptionist)
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
// The same complex taking anyone to a staff room for a break; in it, a sit
// on the sofa, a game of pool and one of the video game.
constexpr uint16_t kComplexToStaffRoom = 0x96;
constexpr uint16_t kComplexSofa = 0x2da;
constexpr uint16_t kComplexPool = 0x2ee;
constexpr uint16_t kComplexVideoGame = 0x2f8;
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
// The complex taking a doctor to a research room's door and in.
constexpr uint16_t kComplexToResearch = 0x96;

// An operating theatre: changing at the surgeon screen as the first or
// second surgeon and back again (doctor_in_theatre, change_surgeon_to_doc);
// the lead surgeon's and the assistant's surgery complexes, the two the
// assistant runs before and after it, the lead's at the table, and the
// patient's surgery (P_COMP.LEX).
constexpr uint16_t kComplexChangeFirstSurgeon = 0x4d8;
constexpr uint16_t kComplexChangeSecondSurgeon = 0x4ec;
constexpr uint16_t kComplexChangeBackFirst = 0x3ca;
constexpr uint16_t kComplexChangeBackSecond = 0x3d4;
constexpr uint16_t kComplexSurgeryLead = 0x316;
constexpr uint16_t kComplexSurgeryAssistant = 0x32a;
constexpr uint16_t kComplexAssistantFirst = 0x33e;
constexpr uint16_t kComplexAssistantDone = 0x352;
constexpr uint16_t kComplexSurgeryHold = 0x334;
constexpr uint16_t kComplexSurgeryAfterAssistant = 0x348;
constexpr uint16_t kComplexPatientSurgery = 0x276;

// A training room: the complex taking a consultant to the projector and
// starting the lecture, the lecture, the bookcase and skeleton lessons and a
// student's to a lecture chair; and the two complexes leaving the room with
// and without the projector running (staff_exit_room_check). The room type
// and objects are in core/world/training.h.
constexpr uint16_t kComplexToProjector = 0x3de;
constexpr uint16_t kComplexLecture = 0x3e8;
constexpr uint16_t kComplexStartProjector = 0x3fc;
constexpr uint16_t kComplexTrainingBookcase = 0x370;
constexpr uint16_t kComplexSkeleton = 0x410;
constexpr uint16_t kComplexLectureChair = 0x41a;
constexpr uint16_t kComplexExitTrainingHold = 0x3b6;
constexpr uint16_t kComplexExitTrainingNoHold = 0xa0;
// The complex taking a doctor to a training room's door and in.
constexpr uint16_t kComplexToTraining = 0x96;

// The game's research_tasks (src/tables.cpp): what a doctor in a research
// room may use, one time in `chance` when it is free, and the complex they
// run on it: the research computer, the atom analyser, a desk and a filing
// cabinet.
struct ResearchTask {
    uint16_t chance;
    uint8_t object;
    uint16_t complex;
};
constexpr std::array<ResearchTask, 4> kResearchTasks = {{{8, 40, 670}, {8, 41, 680}, {12, 1, 1190}, {12, 2, 1200}}};

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

    PersonRun person_run() const { return {world, anims, person, game_turn}; }
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

void evacuate_staff(const Ctx &c, uint16_t id);
void change_surgeon_to_doc(World &world, uint16_t id);

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

// The decision of staff_wanders: an ordered break (kOrderRest,
// take_a_break), a page answered (obey_pager), a break when more tired than
// the policy's rest_threshold, else a stroll of 3 cells back the way they
// came (85%), else, standing, a pause of 20-49 turns, a stroll of 3 cells in
// a random direction or one to a random cell within 3. The orders to go
// back to research or training are dropped. The status icons are not ported.
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
    if (pos < kMapCells && world.map[pos].owner_flags == 1 << h.owner && can_i_get_to(world, h, pos) &&
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

void interrupt_staff_in(const Ctx &c, uint16_t id);

// take_a_break (0x2adfc): the member of staff goes to the nearest staff room
// in use (get_nearest_room_info) for a rest, SS_IN_STAFF_ROOM there: a
// receptionist stops what she does first (interrupt_staff); out of their
// room by its door, then to the staff room's door and in. False, doing
// nothing, if there is no staff room.
bool take_a_break(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    uint16_t room = get_nearest_room_info(world, s, kRoomStaffRoom);
    if (!room)
        return false;
    trace_event("take_a_break %u room=%u tired=%d", unsigned(id), unsigned(room), int(s.tiredness));
    if (s.subtype == kStaffReceptionist)
        interrupt_staff_in(c, id);
    staff_exit_room_check(world, s);
    s.next_state = kStaffInStaffRoom;
    if (s.room) {
        set_complex(world, id, kComplexLeaveRoom, world.rooms[s.room].door_inside_tile);
        process(c, id);
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToStaffRoom, world.rooms[room].door_tile);
    } else {
        set_complex(world, id, kComplexToStaffRoom, world.rooms[room].door_tile);
        s.resume_state = kStaffEvacuate;
        process(c, id);
    }
    s.dest_room = room;
    return true;
}

// is_staff_stepping_on_toes (0x2ce9c): whether another member of staff
// comes after them in their cell's list of things.
bool is_staff_stepping_on_toes(const World &world, const Thing &s)
{
    for (int16_t i = s.cell_next; i > 0; i = world.things[size_t(i)].cell_next)
        if (world.things[size_t(i)].type == kThingStaff)
            return true;
    return false;
}

// overlapping_staff_check (0x2cf14): on every 47th turn, or standing on the
// cell inside their room's door, on another member of staff's cell, or on
// an object's cell without using an object, the member of staff walks to
// a random cell of the room (row drawn first) they can get to, back in
// `state` there. Returns whether they set off.
bool overlapping_staff_check(const Ctx &c, uint16_t id, uint8_t state)
{
    World &world = c.world;
    Thing &s = world.things[id];
    const RoomInfo &room = world.rooms[s.room];
    uint16_t tile = cell_of(s);
    const Thing &obj = world.things[s.object];
    bool uses_object = s.object && (obj.type == kThingObject || obj.type == kThingDoor) && obj.user == id;
    if (!(c.game_turn % 47 == 0 || tile == room.door_inside_tile || is_staff_stepping_on_toes(world, s) ||
          (!uses_object && (world.map[tile].flags & 7))))
        return false;
    auto row = int16_t(old_newrand(world.seed) % uint32_t(room.height));
    auto column = int16_t(old_newrand(world.seed) % uint32_t(room.width));
    auto dest = uint16_t(room.pos + ((row << 7) + column));
    if (tile == dest || !can_i_get_to(world, s, dest))
        return false;
    s.resume_state = state;
    s.next_state = state;
    set_complex(world, id, kComplexWalkTo, dest);
    process(c, id);
    return true;
}

// nurse_check_for_work (0x6c6f4): the nurse heads for a ward in use of her
// hospital drawn at random (get_room_info_rand), SS_NURSE_IN_WARD there.
// False if there is none.
bool nurse_check_for_work(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t room = get_room_info_rand(world, kRoomWard, 100, nurse.owner);
    if (!room)
        return false;
    set_complex(world, id, kComplexToRoom, world.rooms[room].door_tile);
    nurse.substate = 0;
    nurse.next_state = kStaffNurseInWard;
    nurse.resume_state = kStaffInCorridor;
    nurse.dest_room = room;
    process(c, id);
    return true;
}

// The member of staff in a staff room plays `complex` (the sofa, pool or
// the video game), back in SS_IN_STAFF_ROOM after it.
void staff_room_pastime(const Ctx &c, uint16_t id, uint16_t complex)
{
    Thing &s = c.world.things[id];
    s.resume_state = kStaffInStaffRoom;
    s.next_state = kStaffInStaffRoom;
    set_complex(c.world, id, complex, 0);
    process(c, id);
}

// staff_in_staff_room's pastimes: by a draw of ten, pool, the video game
// (the staff who cannot play it then doing nothing more) or, not on the
// sofa already, the sofa. Returns whether they took one or stay put.
bool staff_in_staff_room_pastime(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    const Thing &s = world.things[id];
    uint32_t roll = old_newrand(world.seed, "staff_in_staff_room") % 10;
    const std::array<uint8_t, 2> &can = kStaffAbilities[s.subtype % kStaffAbilities.size()];
    if (find_object(world, cell_of(s), kObjPoolTable, true, true, s.owner) && s.complex_action != kComplexPool &&
        roll == 1 && can[1]) {
        trace_event("staff_room %u pastime=pool", unsigned(id));
        staff_room_pastime(c, id, kComplexPool);
        return true;
    }
    if (find_object(world, cell_of(s), kObjVideoGame, true, true, s.owner) && s.complex_action != kComplexVideoGame &&
        roll == 2) {
        if (can[0]) {
            trace_event("staff_room %u pastime=video", unsigned(id));
            staff_room_pastime(c, id, kComplexVideoGame);
        }
        return true;
    }
    if (s.complex_action != kComplexSofa && find_object(world, cell_of(s), kObjSofa, true, true, s.owner)) {
        trace_event("staff_room %u pastime=sofa", unsigned(id));
        staff_room_pastime(c, id, kComplexSofa);
        return true;
    }
    return false;
}

// send_doctor_to_research (0x6dedc): the doctor goes to research, in
// another research room if in one (find_rand_research_room), else in the
// emptiest (emptiest_research_room): out of their room by its door, then to
// the research room's door and in, SS_DOCTOR_RESEARCHES there. False, doing
// nothing, if there is none to go to.
bool send_doctor_to_research(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t room = d.room != 0 && world.rooms[d.room].type == kRoomResearch
                        ? find_rand_research_room(world, d.owner, d.room)
                        : emptiest_research_room(world, d.owner);
    if (!room)
        return false;
    trace_event("send_to_research %u room=%u", unsigned(id), unsigned(room));
    staff_exit_room_check(world, d);
    if (d.room)
        set_complex(world, id, kComplexLeaveRoom, world.rooms[d.room].door_inside_tile);
    set_complex(world, id, kComplexToResearch, world.rooms[room].door_tile);
    d.next_state = kStaffDoctorResearches;
    d.resume_state = kStaffEvacuate;
    d.dest_room = room;
    process(c, id);
    return true;
}

// send_doctor_to_training (0x4303c): sends the doctor to a training room
// other than their current one: a consultant to one without a consultant
// (find_training_room_no_consultant), to teach; another doctor to the one
// with the most free lecture chairs (find_emptiest_training_room). Out of
// their room by its door, then to the training room's door and in, in the
// state for their grade. False, doing nothing, if there is none.
bool send_doctor_to_training(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t room;
    uint8_t state;
    if (d.skills & kSkillConsultant) {
        room = find_training_room_no_consultant(world, d.owner, d.room);
        state = kStaffConsultantTrains;
    } else {
        room = find_emptiest_training_room(world, d.owner, d.room);
        state = kStaffDoctorIsTrained;
    }
    if (!room)
        return false;
    trace_event("send_to_training %u room=%u", unsigned(id), unsigned(room));
    staff_exit_room_check(world, d);
    if (d.room)
        set_complex(world, id, kComplexLeaveRoom, world.rooms[d.room].door_inside_tile);
    d.next_state = state;
    d.resume_state = kStaffEvacuate;
    set_complex(world, id, kComplexToTraining, world.rooms[room].door_tile);
    d.dest_room = room;
    process(c, id);
    return true;
}

// check_training_command (0x42ff0): if the doctor has been ordered to train
// (ORD_TRAIN) and a training room is available (send_doctor_to_training),
// sends them there and clears the order. Returns whether they were sent.
bool check_training_command(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    if ((d.orders & kOrderTrain) && send_doctor_to_training(c, id)) {
        d.orders &= 0xf0;
        return true;
    }
    return false;
}

// doctor_is_trained (0x42a10): SS_DOCTOR_IS_TRAINED (a student in the
// training room): a consultant switches to teaching; otherwise leaves for a
// rest, new orders or an evacuation as needed, else sits on a free lecture
// chair or walks to a random spot. Marks them to return here after a rest.
void doctor_is_trained(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    d.orders &= uint8_t(~kOrderTrain);
    if (d.skills & kSkillConsultant) {
        d.state = kStaffConsultantTrains;
        return;
    }
    if (shall_i_resign(c, id))
        return;
    d.orders |= kOrderReturnToTraining;
    if (world.rooms[d.room].being_edited || world.rooms[d.room].state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (check_training_command(c, id))
        return;
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return;
    if (find_object(world, cell_of(d), kObjLectureChair, true, true, d.owner) &&
        d.complex_action != kComplexLectureChair) {
        d.next_state = kStaffDoctorIsTrained;
        d.resume_state = kStaffDoctorIsTrained;
        set_complex(world, id, kComplexLectureChair, 0);
        process(c, id);
        return;
    }
    if (d.complex_action == kComplexLectureChair)
        return;
    const RoomInfo &room = world.rooms[d.room];
    uint8_t x = uint8_t(old_newrand(world.seed) % uint32_t(room.width) + room.pos % 128);
    uint8_t y = uint8_t(old_newrand(world.seed) % uint32_t(room.height) + room.pos / 128);
    if (world.map[(y << 7) | x].first == 0 && can_i_get_to(world, d, uint16_t(x + (y << 7)))) {
        d.next_state = kStaffDoctorIsTrained;
        d.resume_state = kStaffDoctorIsTrained;
        set_complex(world, id, kComplexWalkTo, uint16_t(x + (y << 7)));
        process(c, id);
    }
}

// consultant_trains' bookcase (2) and skeleton (1) lessons: by a draw of a
// thousand under 100, teaching from a free one near the consultant.
// Returns whether they took one.
bool consultant_trains_teach(const Ctx &c, uint16_t id, uint8_t object, uint16_t complex, uint8_t kind)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!(old_newrand(world.seed, "consultant_trains") % 1000 < 100 &&
          find_object(world, cell_of(d), object, true, true, d.owner)))
        return false;
    training_done(world, d, kind);
    d.next_state = kStaffConsultantTrains;
    d.resume_state = kStaffConsultantTrains;
    set_complex(world, id, complex, 0);
    process(c, id);
    return true;
}

// consultant_trains' checks before it works: resigning, the room being
// edited, new orders, the room mutex and a break. Returns whether the
// consultant did something other than teach.
bool consultant_trains_busy(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (shall_i_resign(c, id))
        return true;
    if (world.rooms[d.room].being_edited || world.rooms[d.room].state == kRoomEdited) {
        evacuate_staff(c, id);
        return true;
    }
    if (check_training_command(c, id))
        return true;
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return true;
    }
    if (!check_room_mutex(world, id)) {
        evacuate_staff(c, id);
        return true;
    }
    d.orders |= kOrderReturnToTraining;
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return true;
    return false;
}

// consultant_trains (0x42c90): SS_CONSULTANT_TRAINS: the consultant holds
// the room, starts the projector, and gives lectures (training_done kind 0),
// or now and then teaches from the bookcase (2) or skeleton (1); leaves for
// a rest, new orders or an evacuation as needed.
void consultant_trains(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    d.orders &= uint8_t(~kOrderTrain);
    if (consultant_trains_busy(c, id))
        return;
    uint16_t projector = find_object(world, cell_of(d), kObjProjector, false, false, d.owner);
    if (!projector)
        return;
    if (!world.things[projector].lecture_started) {
        world.things[projector].lecture_started = 1;
        set_complex(world, id, kComplexStartProjector, 0);
        d.next_state = kStaffConsultantTrains;
        process(c, id);
        return;
    }
    if (consultant_trains_teach(c, id, kObjBookcase, kComplexTrainingBookcase, 2))
        return;
    if (consultant_trains_teach(c, id, kObjSkeleton, kComplexSkeleton, 1))
        return;
    if (d.complex_action != kComplexToProjector && d.complex_action != kComplexLecture) {
        d.next_state = kStaffConsultantTrains;
        d.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexToProjector, 0);
        process(c, id);
        return;
    }
    training_done(world, d, 0);
    d.complex_pc = 0;
    d.complex_queued = 0;
    d.complex_queued2 = 0;
    d.complex_resume = 0;
    d.next_state = kStaffConsultantTrains;
    set_complex(world, id, kComplexLecture, 0);
    process(c, id);
}

// staff_in_staff_room once rested: discontent halved, a crazy doctor cured,
// the others back to work out of `room` (a doctor who was researching or
// training back to it: send_doctor_to_research, send_doctor_to_training),
// their speed again by `settings`. Returns false for a handyman or
// receptionist, sent out (SS_EVACUATE) with nothing more, and for a doctor
// gone back to research or training.
bool staff_in_staff_room_rested(const Ctx &c, uint16_t id, const Balance &settings, const RoomInfo &room)
{
    World &world = c.world;
    Thing &s = world.things[id];
    trace_event("rested %u discontent=%u crazy=%u", unsigned(id), unsigned(s.discontent), unsigned(s.crazy));
    s.discontent = uint16_t(int16_t(s.discontent) / 2);
    if (s.subtype == kStaffDoctor && s.crazy) {
        if (s.layers[5] == 3)
            s.layers[5] = 1;
        else if (s.layers[5] == 4)
            s.layers[5] = 2;
        s.crazy = 0;
    } else if (s.subtype == kStaffDoctor) {
        if ((s.orders & kOrderReturnToResearch) && send_doctor_to_research(c, id))
            return false;
        if ((s.orders & kOrderReturnToTraining) && send_doctor_to_training(c, id))
            return false;
        if (!obey_pager(c, id)) {
            set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
            s.next_state = kStaffInCorridor;
            process(c, id);
        }
    } else if (s.subtype == kStaffNurse) {
        set_complex(world, id, kComplexLeaveRoom, room.door_inside_tile);
        if (!nurse_check_for_work(c, id)) {
            s.next_state = kStaffInCorridor;
            process(c, id);
        }
    } else if (s.subtype == kStaffHandyman || s.subtype == kStaffReceptionist) {
        s.state = kStaffEvacuate;
        return false;
    }
    calculate_speed(world, s, settings);
    return true;
}

// staff_in_staff_room (0x2af04): at each end of their animation, once their
// countdown runs out (every 256 / speed of them), the member of staff rests
// (rest_staff: gbv.rest_sofa on the sofa, else gbv.rest_standing), their
// ordered rest done; ordered out of the room (kOrderLeaveRoom, the low
// orders cleared) or with the room being edited, they leave
// (evacuate_staff). Else, by a draw of ten, one time in ten they play pool
// (doctors and handymen) and one in ten the video game (doctors and
// nurses; the others then do nothing more) on a free one they are not
// already playing; else, not on the sofa already, they sit on a free one;
// else they move off a crowded cell (overlapping_staff_check); then, at
// speed 0x20, once no longer tired
// (tiredness 0 or less) their discontent halves and a crazy doctor is cured
// and stays, another doctor goes back to research if they came from it
// (kOrderReturnToResearch: send_doctor_to_research), else answers a page
// (obey_pager) or goes back to the corridor, a nurse goes to a ward
// (nurse_check_for_work) or the corridor, and a handyman or receptionist
// goes out (SS_EVACUATE). The order to go back to training, which is never
// given here (the training room's work is not ported), is not looked at.
void staff_in_staff_room(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &s = world.things[id];
    if (c.game_turn % s.anim_speed != 0)
        return;
    s.sprite = c.anims.frames[s.sprite].next;
    if (!(c.anims.frames[s.sprite].flags & 1) || --s.countdown > 0)
        return;
    static const Balance statics;
    const Balance &settings = world.settings ? *world.settings : statics;
    const Balance::Gbv &gbv = settings.gbv;
    rest_staff(world, s, int16_t(s.complex_action == kComplexSofa ? gbv.rest_sofa : gbv.rest_standing));
    s.orders &= uint8_t(~kOrderRest);
    if (s.orders & kOrderLeaveRoom) {
        s.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    const RoomInfo &room = world.rooms[s.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (staff_in_staff_room_pastime(c, id))
        return;
    if (overlapping_staff_check(c, id, kStaffInStaffRoom))
        return;
    s.speed = 0x20;
    if (s.tiredness <= 0 && !staff_in_staff_room_rested(c, id, settings, room))
        return;
    s.countdown = int16_t(0x100 / s.speed);
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
// his repair and plant pages, reopening the room of the machine he was to
// repair. The status icons are not ported.
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
        if (s.room && s.object && kObjectIsMachine[world.things[s.object].subtype % kObjectIsMachine.size()])
            reopen(world.rooms[s.room]);
        thingpage_cancel_my_response(world.repair_pager, id);
        thingpage_cancel_my_response(world.plant_pager, id);
    }
    staff_interrupt_complex(c, id);
    if (s.subtype == kStaffNurse && patient && s.room == 0) {
        world.things[patient].person_flags &= ~kPfVaccineNurseSent;
        s.slave = 0;
    } else if (patient && world.things[patient].type == kThingPatient && (s.room || s.subtype == kStaffReceptionist)) {
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
    trace_event("abandon_complex %u", unsigned(id));
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
    set_icon_value(c.anims, s, kIconLeaving);
    if (s.owner == kNoOwner)
        return;
    interrupt_staff_in(c, id);
    if (mode == 0 && world.ledger && world.balance && world.date)
        change_balance(*world.ledger, *world.balance, world.date->day, world.date->month, {kTxSeverancePay, -s.salary});
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
    clear_peep_icon(s);
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

// am_i_lead_staff (0x2b528): whether member of staff `id` holds their room's
// first lock (in an operating theatre, only once both surgeons are there).
bool am_i_lead_staff(const World &world, uint16_t id)
{
    const Thing &s = world.things[id];
    if (!s.room)
        return false;
    const RoomInfo &room = world.rooms[s.room];
    if (room.type == kRoomTheatre && (!room.staff[0] || !room.staff[1]))
        return false;
    return room.staff[0] == id;
}

// find_waiting_surgeon (0x3c154): the first member of staff in room
// `room_id` whose state is `state`, or 0. Despite its name, not limited to
// surgeons: SS_SURGEON_WAITS or SS_ASSISTANT_READY are passed.
uint16_t find_waiting_surgeon(const World &world, uint16_t room_id, uint8_t state)
{
    const RoomInfo &room = world.rooms[room_id];
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next)
        if (world.things[t].type == kThingStaff && world.things[t].state == state)
            return t;
    return 0;
}

// change_surgeon_to_doc (0x3c0a8): starts the surgeon changing back from
// surgical dress (the complex for the first or second surgeon), or turns
// them back into kStaffDoctor at once if the room counts neither one nor two
// surgeons.
void change_surgeon_to_doc(World &world, uint16_t id)
{
    Thing &s = world.things[id];
    uint8_t count = count_surgeons(world, s.room);
    if (count == 1) {
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexChangeBackFirst, 0);
    } else if (count == 2) {
        s.resume_state = kStaffEvacuate;
        set_complex(world, id, kComplexChangeBackSecond, 0);
    } else {
        s.subtype = kStaffDoctor;
    }
}

// doctor_in_theatre (0x3bed0): sends a qualified surgeon to change at the
// surgeon screen (as the first or second surgeon), or to wander if there is
// no screen; others, or a third surgeon, leave.
void doctor_in_theatre(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    const RoomInfo &room = world.rooms[d.room];
    uint8_t count = count_surgeons(world, d.room);
    d.object = 0;
    if (!(d.skills & kSkillSurgeon)) {
        evacuate_staff(c, id);
        return;
    }
    if (count == 0 || count == 1) {
        if (find_object(world, cell_of(d), kObjSurgeonScreen, false, true, d.owner)) {
            set_complex(world, id, count == 0 ? kComplexChangeFirstSurgeon : kComplexChangeSecondSurgeon, 0);
            d.resume_state = kStaffDoctorInTheatre;
            d.next_state = kStaffDoctorInMachineRoom;
            process(c, id);
            return;
        }
    } else {
        evacuate_staff(c, id);
        return;
    }
    // The row is drawn first, as the game's compiled code does.
    auto row = int16_t(old_newrand(world.seed, "doctor_in_theatre") % uint32_t(room.height));
    auto column = int16_t(old_newrand(world.seed, "doctor_in_theatre") % uint32_t(room.width));
    uint16_t pos = uint16_t((row << 7) + column + room.pos);
    if (can_i_get_to(world, d, pos)) {
        d.next_state = kStaffDoctorInTheatre;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// surgeon_waits (0x3c11c): the lead surgeon counts down, then returns to
// SS_DOCTOR_IN_MACHINE_ROOM.
void surgeon_waits(const Ctx &c, uint16_t id)
{
    Thing &s = c.world.things[id];
    s.timer--;
    if (s.timer <= 0)
        s.state = kStaffDoctorInMachineRoom;
}

// stop_waiting_for_patient (0x3b234): the call dropped, back to the room's
// working state, releasing a surgeon's assistant waiting with them.
void stop_waiting_for_patient(World &world, Thing &s)
{
    trace_event("stop_waiting");
    s.state = get_state(world.rooms[s.room].type, s);
    s.slave = 0;
    if (s.subtype == kStaffSurgeon) {
        uint16_t partner = find_waiting_surgeon(world, s.room, kStaffAssistantReady);
        if (partner) {
            Thing &p = world.things[partner];
            p.state = get_state(world.rooms[p.room].type, p);
            p.slave = 0;
        }
    }
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
// (SS_DOCTOR_WORKS_AT_DESK), or out without one.
void doctor_in_office(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!holds_room(c, id))
        return;
    if (find_object(world, cell_of(d), kObjDesk, true, true, d.owner)) {
        reset_icon_value(d, kIconCalling);
        d.next_state = kStaffDoctorWorksAtDesk;
        d.resume_state = kStaffDoctorInOffice;
        set_complex(world, id, kComplexSitAtDesk, 0);
        process(c, id);
    } else {
        evacuate_staff(c, id);
    }
}

// doctor_works_at_desk's call: the next patient of `room`'s queue called
// (call_person_to_room), the doctor to wait for them, visiting the filing
// cabinet on the way if there is one.
void doctor_works_at_desk_call_patient(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t queue = room.queue_head;
    if (!queue || !call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true))
        return;
    set_icon_value(c.anims, d, kIconCalling);
    if (find_object(world, cell_of(d), kObjCabinet, true, true, d.owner)) {
        d.next_state = kStaffDoctorWaitsForPatient;
        set_complex(world, id, kComplexCabinetThenWait, 0);
        process(c, id);
    } else {
        d.state = kStaffDoctorWaitsForPatient;
    }
}

// The end of doctor_works_at_desk and doctor_in_machine_room: free in their
// room (staff_free), a member of staff takes a break (take_a_break) when
// more tired than the policy's rest_threshold or, with the policy letting
// staff leave rooms and no one queueing, answers a page every
// work_delay + 1 turns (2 * work_delay + 1 tired); gone, a member of staff
// with `skills` is paged in their place if patients queue.
void free_staff_break_or_page(const Ctx &c, uint16_t id, uint16_t skills)
{
    World &world = c.world;
    const Thing &d = world.things[id];
    const RoomInfo &room = world.rooms[d.room];
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
        page_member_of_staff(world, skills, d.room, false, 1, d.owner);
}

// doctor_works_at_desk (0x3a7a8): at each end of the desk's animation,
// out if the room is being edited; else, a quarter of the time, the next
// patient of the room's queue called (call_person_to_room, the filing
// cabinet visited on the way if there is one); 7% of the rest a visit to
// the cabinet; else, free (staff_free), a break (take_a_break) when more
// tired than the policy's rest_threshold or, with the policy letting staff
// leave rooms and no one queueing, a page answered every work_delay + 1
// turns (2 * work_delay + 1 tired), a doctor paged in their place if
// patients queue; first, they may resign or leave (shall_i_resign), and
// take an ordered break (kOrderRest). The orders to go back to research or
// training are dropped.
void doctor_works_at_desk(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    d.orders &= uint8_t(~(kOrderReturnToResearch | kOrderReturnToTraining));
    if (!holds_room(c, id) || shall_i_resign(c, id))
        return;
    RoomInfo &room = world.rooms[d.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if ((d.orders & kOrderRest) && take_a_break(c, id))
        return;
    if (old_newrand(world.seed) % 100 < 25) {
        doctor_works_at_desk_call_patient(c, id, room);
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
    free_staff_break_or_page(c, id, kSkillDoctor);
}

// The examinations a doctor starts once the patient called has arrived
// (doctor_waits_for_patient_to_arrive), by room type: the doctor's complex
// (S_COMP.LEX) and the SS_DOCTOR_EXAMINES substate it leads to; complex 0
// for rooms not ported (the operating theatre, research and the DNA
// fixer).
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
    e[kRoomInflation] = {0x1e0, 10};
    e[kRoomPharmacy] = {0x244, 180};
    e[kRoomHairRestorer] = {0x2bc, 120};
    e[kRoomSlackTongue] = {0x1cc, 130};
    e[kRoomFracture] = {0x42e, 140};
    e[kRoomElectrolysis] = {0x186, 160};
    e[kRoomJellyVat] = {0x19a, 170};
    e[kRoomDecontamination] = {0x28a, 210};
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
// The pharmacy's: the nurse giving the drug, and fetching it for the
// patient to drink in the cabinet (transparency, invisibility).
constexpr uint16_t kComplexGiveDrug = 500;
constexpr uint16_t kComplexFetchDrug = 0x21c;
constexpr uint16_t kComplexAfterCabinet = 0x230;
// The patient's and staff's animation columns of drinking in the cabinet.
constexpr size_t kAnimDrink = 43;
constexpr uint16_t kStaffAnimDrink = 15;
// Illnesses (the game's ILL_*) the pharmacy cures in its cabinet.
constexpr uint8_t kIllInvisibility = 5;
constexpr uint8_t kIllTransparency = 15;
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

// doctor_waits_for_patient's operating theatre: the lead surgeon (who called)
// starts the surgery in SS_DOCTOR_EXAMINES substate 70 and the assistant (the
// room's other holder) waits in SS_ASSISTANT_READY.
void doctor_waits_for_patient_theatre(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t assistant_id = room.staff[1];
    d.work_target = assistant_id;
    d.next_state = kStaffDoctorExamines;
    d.substate = 0x46;
    set_complex(world, id, kComplexSurgeryLead, 0);
    process(c, id);
    world.things[assistant_id].next_state = kStaffAssistantReady;
    set_complex(world, assistant_id, kComplexSurgeryAssistant, 0);
    process(c, assistant_id);
}

// doctor_waits_for_patient's examination in a diagnosis room `room`, as
// kExaminations lists: the cardiogram's patient on the machine, the
// psychiatrist first at the bookcase and the patient then on the couch, the
// scanner's second complex for some illnesses; an operating theatre's surgery
// for its lead surgeon and assistant.
void doctor_waits_for_patient_examine(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    Thing &patient = world.things[pid];
    if (room.type == kRoomTheatre) {
        doctor_waits_for_patient_theatre(c, id, room);
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
}

// doctor_waits_for_patient with the patient called arrived in `room`: a
// crazy doctor sends one in four on elsewhere; else both start the
// examination, a GP's consultation with the patient to the chair.
void doctor_waits_for_patient_arrived(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    Thing &patient = world.things[pid];
    patient.state = kPsSlaveReady;
    if (d.subtype == kStaffDoctor && d.crazy &&
        (old_newrand(world.seed, "doctor_waits_for_patient_to_arrive") & 3) == 0) {
        send_person_somewhere_else(c, pid);
        d.state = get_state(room.type, d);
        d.slave = 0;
        return;
    }
    trace_event("treat %u patient=%u room=%u", unsigned(id), unsigned(pid), unsigned(d.room));
    patient.resume_state = 0;
    if (room.type != kRoomGp) {
        doctor_waits_for_patient_examine(c, id, room);
        return;
    }
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
}

// doctor_waits_for_patient_to_arrive (0x3aba4): once the patient called
// has arrived, both start the room's examination (a GP's consultation, the
// patient to the chair; the diagnosis rooms' in kExaminations), the doctor
// to SS_DOCTOR_EXAMINES; a cracked-up doctor first sends one patient in
// four (a draw from world.seed) on elsewhere. Until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes (forget_about_my_slave). The rooms kExaminations does
// not list are not ported: their patients wait.
void doctor_waits_for_patient(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (!d.slave) {
        d.state = get_state(world.rooms[d.room].type, d);
        return;
    }
    uint16_t pid = d.slave;
    const Thing &patient = world.things[pid];
    const RoomInfo &room = world.rooms[d.room];
    if (patient.state == kPsArrivedFromCall) {
        doctor_waits_for_patient_arrived(c, id, room);
        return;
    }
    if (--d.timer <= 0) {
        uint8_t result = call_person_again(c.person_run(), id, kComplexCalledToRoom, room.door_tile);
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
    if (treatment_machine_used(world, c.anims, patient.treatment, cell_of(patient), c.game_turn))
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

// start[anim + facing] of kPatientAnimStarts row `kind` column `column`,
// into `sprite` and `flip`, as the game sets a machine's aux_sprite.
void patient_start_frame(const AnimData &anims, uint8_t kind, size_t column, uint8_t facing, uint16_t &sprite,
                         uint8_t &flip)
{
    uint16_t anim = kPatientAnimStarts[kind % kPatientAnimStarts.size()][column];
    size_t start = size_t(anim == 9999 ? 0 : anim) + facing;
    if (start >= anims.starts.size())
        return;
    sprite = uint16_t(anims.starts[start].frame);
    flip = anims.starts[start].flip ? 1 : 0;
}

// The pharmacy's cabinet stages (doctor_carries_out_examination's 201 to
// 206): until the machine's split_height reaches `until` (moving 10 a turn
// towards it), then on to the next substate with the drawing effect `flags`
// and parameter `param` and split_height `height`. Returns whether it moved
// on.
bool cabinet_stage(Thing &d, Thing &obj, uint8_t until, uint8_t flags, uint8_t param, uint8_t height)
{
    if (obj.split_height != until) {
        obj.split_height = uint8_t(until ? obj.split_height + 10 : obj.split_height - 10);
        return false;
    }
    d.substate++;
    obj.aux_draw_flags = flags;
    obj.aux_param = param;
    obj.split_height = height;
    return true;
}

// doctor_examines's substate 1: the GP's consultation over, the doctor back
// to the desk, the patient's diagnosis and what comes next
// (done_treatment), the doctor free again.
void doctor_examines_consultation_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    d.state = kStaffDoctorWorksAtDesk;
    d.slave = 0;
    done_treatment(world, c.anims, id, pid, c.game_turn);
    world.rooms[d.room].staff_free = 1;
}

// doctor_examines's substates 30 and 31: until the cardiogram's patient is
// ready the doctor now and then idles; then the patient's run (30) or the
// doctor's check (31).
void doctor_examines_cardio_ready(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    if (world.things[pid].state != kPsSlaveReady) {
        uint32_t pct = examination_pct(world);
        uint32_t low = d.substate == 30 ? 30 : 33;
        if (pct >= low && pct < low + 2)
            examine_with(c, id, kComplexCardioIdle);
        return;
    }
    d.substate++;
    if (d.substate == 31)
        patient_does(c, pid, kComplexCardioRun);
    else
        examine_with(c, id, kComplexCardioCheck);
}

// doctor_examines's substate 32: the cardiogram's jolts, the patient's
// animation speed drawn, the doctor checking again; or the patient jolted
// (33); or the doctor's tune.
void doctor_examines_cardio_jolts(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    Thing &patient = world.things[pid];
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
        return;
    } else {
        examine_with(c, id, kComplexCardioTune);
        return;
    }
    examine_with(c, id, kComplexCardioCheck);
}

// doctor_examines's substate 33: the jolted patient ready, off the
// cardiogram (40).
void doctor_examines_cardio_off(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    uint16_t pid = d.slave;
    if (c.world.things[pid].state != kPsSlaveReady)
        return;
    d.substate = 40;
    patient_does(c, pid, kComplexCardioOff);
}

// doctor_examines's substate 40: the patient off the cardiogram, the doctor
// back to the room's work and, unless the machine broke down
// (treatment_machine_used), the patient's diagnosis and what comes next,
// the doctor free.
void doctor_examines_cardio_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    const Thing &patient = world.things[pid];
    if (patient.state != kPsSlaveReady)
        return;
    d.state = kStaffDoctorInMachineRoom;
    d.slave = 0;
    if (treatment_machine_used(world, c.anims, patient.treatment, cell_of(patient), c.game_turn))
        return;
    done_treatment(world, c.anims, id, pid, c.game_turn);
    world.rooms[d.room].staff_free = 1;
}

// doctor_examines's substate 60: the psychiatrist with a big-headed
// patient first (the bookcase if there is one), then the end (230).
void doctor_examines_psych(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    Thing &patient = world.things[pid];
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
}

// doctor_examines's substate 110: one of general diagnosis's three
// examinations, then its last (111).
void doctor_examines_general_diag(const Ctx &c, uint16_t id)
{
    c.world.things[id].substate = 111;
    examine_with(
        c, id,
        kComplexGeneralDiag[old_newrand(c.world.seed, "doctor_carries_out_examination") % kComplexGeneralDiag.size()]);
}

// doctor_examines's substate 180: in a pharmacy, a patient without a drug
// goes home; else the drug fetched for transparency (190) and invisibility
// (200), or given (181).
void doctor_examines_pharmacy_drug(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    const Thing &patient = world.things[pid];
    if (!pre_administer_drugs(world, pid)) {
        person_go_home(world, c.anims, c.person, pid, c.game_turn);
        d.slave = 0;
        d.state = kStaffDoctorInMachineRoom;
        world.rooms[d.room].staff_free = 1;
    } else if (patient.illness == kIllTransparency) {
        d.substate = 190;
        examine_with(c, id, kComplexFetchDrug);
    } else if (patient.illness == kIllInvisibility) {
        d.substate = 200;
        examine_with(c, id, kComplexFetchDrug);
    } else {
        d.substate = 181;
        examine_with(c, id, kComplexGiveDrug);
    }
}

// doctor_examines's substate 181: the drug given (administer_drugs), the
// nurse back to the room's work, free.
void doctor_examines_drug_given(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    d.next_state = kStaffDoctorInMachineRoom;
    d.slave = 0;
    set_complex(world, id, kComplexBackToWork, 0);
    process(c, id);
    administer_drugs(world, c.anims, id, pid, c.game_turn);
    world.rooms[d.room].staff_free = 1;
}

// doctor_examines's substate 190: the patient drinks the transparency drug
// in the cabinet (191).
void doctor_examines_drink_transparency(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    Thing &patient = world.things[d.slave];
    d.substate++;
    play_staff_anim(c.anims, d, kStaffAnimDrink);
    patient.state = kPsFills;
    patient.object = d.object;
    Thing &obj = world.things[patient.object];
    play_frame(patient, c.anims, kPatientAnimStarts[patient.subtype][kAnimDrink]);
    patient_start_frame(c.anims, patient.subtype == 6 ? 0 : 1, kAnimDrink, patient.facing, obj.aux_sprite,
                        obj.aux_flip);
    obj.split_height = 0;
}

// doctor_examines's substate 191: the patient done drinking, their looks
// changed back, the drug given (181).
void doctor_examines_transparency_drunk(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    Thing &patient = c.world.things[d.slave];
    if (patient.state != kPsSlaveReady)
        return;
    transform_peep_who(patient, 4);
    d.substate = 181;
    examine_with(c, id, kComplexAfterCabinet);
    patient.state = kPsMerged;
}

// doctor_examines's substate 200: the patient drinks the invisibility drug
// in the cabinet (201).
void doctor_examines_drink_invisibility(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    Thing &patient = world.things[d.slave];
    d.substate++;
    play_staff_anim(c.anims, d, kStaffAnimDrink);
    patient.state = kPsEnergises;
    patient.object = d.object;
    Thing &obj = world.things[patient.object];
    play_frame(patient, c.anims, kPatientAnimStarts[patient.subtype][kAnimDrink]);
    obj.aux_draw_flags = 4;
    patient_start_frame(c.anims, 0, kAnimDrink, patient.facing, obj.aux_sprite, obj.aux_flip);
    obj.aux_param = 0;
    obj.split_height = 0;
}

// doctor_examines's substate 203: the cabinet's stage after which the
// patient takes the machine's drawing (204).
void doctor_examines_invisibility_shown(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    Thing &patient = c.world.things[d.slave];
    Thing &obj = c.world.things[patient.object];
    if (cabinet_stage(d, obj, 200, 0xff, 1, 200)) {
        patient.sprite = obj.aux_sprite;
        patient.flip = obj.aux_flip;
    }
}

// doctor_examines's substate 206: the cabinet's last stage, then the
// patient's looks changed back and the drug given (181).
void doctor_examines_invisibility_drunk(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    Thing &patient = c.world.things[d.slave];
    Thing &obj = c.world.things[patient.object];
    if (obj.split_height != 0) {
        obj.split_height = uint8_t(obj.split_height - 10);
        return;
    }
    transform_peep_who(patient, 0xa);
    d.substate = 181;
    examine_with(c, id, kComplexAfterCabinet);
    patient.state = kPsMerged;
}

// doctor_examines's pharmacy substates (180-206); the others do nothing.
void doctor_examines_pharmacy(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    switch (d.substate) {
    case 180:
        doctor_examines_pharmacy_drug(c, id);
        break;
    case 181:
        doctor_examines_drug_given(c, id);
        break;
    case 190:
        doctor_examines_drink_transparency(c, id);
        break;
    case 191:
        doctor_examines_transparency_drunk(c, id);
        break;
    case 200:
        doctor_examines_drink_invisibility(c, id);
        break;
    case 201:
        cabinet_stage(d, c.world.things[c.world.things[d.slave].object], 200, 8, 1, 0);
        break;
    case 202:
        cabinet_stage(d, c.world.things[c.world.things[d.slave].object], 200, 0xff, 1, 0);
        break;
    case 203:
        doctor_examines_invisibility_shown(c, id);
        break;
    case 204:
        cabinet_stage(d, c.world.things[c.world.things[d.slave].object], 0, 8, 5, 200);
        break;
    case 205:
        cabinet_stage(d, c.world.things[c.world.things[d.slave].object], 0, 4, 0, 200);
        break;
    case 206:
        doctor_examines_invisibility_drunk(c, id);
        break;
    default:
        break;
    }
}

// doctor_examines's theatre substates 70-72: the lead surgeon waits (70),
// brings the assistant in (71) and, the assistant ready, sends it off while
// the surgery finishes (72).
void doctor_examines_theatre(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (d.substate == 70) {
        d.next_state = kStaffDoctorExamines;
        set_complex(world, id, kComplexSurgeryHold, 0);
        process(c, id);
        d.substate = 71;
        return;
    }
    Thing &assistant = world.things[d.work_target];
    if (assistant.state != kStaffAssistantReady)
        return;
    if (d.substate == 71) {
        assistant.next_state = kStaffAssistantReady;
        set_complex(world, d.work_target, kComplexAssistantFirst, 0);
        process(c, d.work_target);
        d.substate = 72;
        return;
    }
    assistant.next_state = kStaffDoctorInMachineRoom;
    set_complex(world, d.work_target, kComplexAssistantDone, 0);
    process(c, d.work_target);
    d.next_state = kStaffDoctorExamines;
    set_complex(world, id, kComplexSurgeryAfterAssistant, 0);
    process(c, id);
    d.substate = 73;
    d.work_target = 0;
}

// doctor_examines's theatre substate 73: the surgery over, unless the table
// is still in use (treatment_machine_used) the patient's surgery runs,
// the treatment is done and the lead surgeon free.
void doctor_examines_theatre_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    const Thing &patient = world.things[pid];
    if (treatment_machine_used(world, c.anims, patient.treatment, cell_of(patient), c.game_turn))
        return;
    set_complex(world, pid, kComplexPatientSurgery, 0);
    process_person_complex(world, c.anims, c.person, pid, c.game_turn);
    done_treatment(world, c.anims, id, pid, c.game_turn);
    d.slave = 0;
    d.state = kStaffDoctorInMachineRoom;
    world.rooms[d.room].staff_free = 1;
}

// doctor_carries_out_examination (0x530e0), by the doctor's substate: in a
// GP's office (1), the consultation over, back to the desk, the patient's
// diagnosis and what comes next (done_treatment), the doctor free again;
// at the cardiogram (30-33, 40), the patient on the machine, the doctor's
// checks and the jolts (the patient's animation speed drawn), then the
// patient off; general diagnosis (110, 111), one of three examinations,
// then the last; the psychiatrist (60) with a big-headed patient first;
// then (10, 50, 80, 90, 100, 120, 130, 140, 160, 170, 210, 230) the end of
// the examination or treatment (examination_done; 10 is inflation's, 120
// the hair restorer's, 130 the slack tongue slicer's, 140 the fracture
// clinic's, 160 electrolysis', 170 the jelly vat's, 210 decontamination's).
// In an operating theatre (70-73), the lead surgeon and its assistant run
// the surgery and the patient is cured (done_treatment). In a pharmacy (180),
// a patient
// without a drug goes home; else the nurse gives the drug (181,
// administer_drugs), first, for transparency (190, 191) and invisibility
// (200-206), the patient drinking it in the cabinet, its looks changed
// back. Research's (autopsies) and the DNA fixer's stages are not ported.
void doctor_examines(const Ctx &c, uint16_t id)
{
    Thing &d = c.world.things[id];
    if (!d.slave)
        return;
    switch (d.substate) {
    case 1:
        doctor_examines_consultation_done(c, id);
        break;
    case 30:
    case 31:
        doctor_examines_cardio_ready(c, id);
        break;
    case 32:
        doctor_examines_cardio_jolts(c, id);
        break;
    case 33:
        doctor_examines_cardio_off(c, id);
        break;
    case 40:
        doctor_examines_cardio_done(c, id);
        break;
    case 60:
        doctor_examines_psych(c, id);
        break;
    case 70:
    case 71:
    case 72:
        doctor_examines_theatre(c, id);
        break;
    case 73:
        doctor_examines_theatre_done(c, id);
        break;
    case 110:
        doctor_examines_general_diag(c, id);
        break;
    case 111:
        d.substate = 230;
        examine_with(c, id, kComplexGeneralDiagEnd);
        break;
    case 10:
    case 50:
    case 80:
    case 90:
    case 100:
    case 120:
    case 130:
    case 140:
    case 160:
    case 170:
    case 210:
    case 230:
        examination_done(c, id);
        break;
    default:
        doctor_examines_pharmacy(c, id);
        break;
    }
}

// doctor_in_machine_room's stroll to a random cell of the room they stand
// in (1% of the time its middle) they can get to.
void doctor_in_machine_room_stroll(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t tile = cell_of(d);
    const RoomInfo &here = world.rooms[room_at(world, tile)];
    uint16_t pos;
    if (old_newrand(world.seed, "doctor_in_machine_room") % 100 == 25) {
        pos = uint16_t(((here.height / 2) << 7) + here.width / 2 + here.pos);
    } else {
        // The row is drawn first, as the game's code does.
        auto row = int16_t(old_newrand(world.seed, "doctor_in_machine_room") % uint32_t(here.height));
        auto column = int16_t(old_newrand(world.seed, "doctor_in_machine_room") % uint32_t(here.width));
        pos = uint16_t((row << 7) + column + here.pos);
    }
    if (tile != pos && can_i_get_to(world, d, pos)) {
        d.next_state = d.resume_state = kStaffDoctorInMachineRoom;
        set_complex(world, id, kComplexWalkTo, pos);
        process(c, id);
    }
}

// doctor_in_machine_room's call, when the room is open and a patient queues:
// in rooms other than an operating theatre the member of staff calls them
// itself and waits; in a theatre the lead surgeon (waiting in
// SS_SURGEON_WAITS for the other to arrive) is found and calls, this
// surgeon waiting as its assistant, or, being the lead itself, waits.
void doctor_in_machine_room_call(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t queue = room.queue_head;
    if (room.state == kRoomClosed || !queue)
        return;
    if (room.type != kRoomTheatre) {
        if (call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true)) {
            set_icon_value(c.anims, d, kIconCalling);
            play_staff_anim(c.anims, d, kAnimStand);
            d.state = kStaffDoctorWaitsForPatient;
        }
        return;
    }
    if (am_i_lead_staff(world, id)) {
        d.timer = 200;
        d.state = kStaffSurgeonWaits;
        return;
    }
    uint16_t lead = find_waiting_surgeon(world, d.room, kStaffSurgeonWaits);
    if (lead && call_person_to_room(c.person_run(), lead, queue, kComplexCalledToRoom, room.door_tile, true)) {
        d.state = kStaffAssistantReady;
        set_icon_value(c.anims, world.things[lead], kIconCalling);
        play_staff_anim(c.anims, world.things[lead], kAnimStand);
        world.things[lead].state = kStaffDoctorWaitsForPatient;
    }
}

// doctor_in_machine_room (0x3b2c4): at each end of their animation, a
// member of staff in a room with machines (the diagnosis rooms) goes out if
// the room is being edited; else, a quarter of the time, the next patient
// of the room's queue is called (call_person_to_room) unless the room is
// closed; 2% of the rest, or always on the cell inside the door, they walk
// to a random cell of the room (1% of those its middle) they can get to;
// else, free (staff_free), a break (take_a_break) when more tired than the
// policy's rest_threshold or, with the policy letting staff leave rooms and
// no one queueing, a page answered every work_delay + 1 turns
// (2 * work_delay + 1 tired), another doctor paged if a queue is left.
// First, they may resign or leave (shall_i_resign), take an ordered break
// (kOrderRest), and a room due a repair is closed. A doctor in an
// operating theatre goes to SS_DOCTOR_IN_THEATRE; nurses' vaccinations are
// not ported.
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
    if ((d.orders & kOrderRest) && take_a_break(c, id))
        return;
    if (d.subtype == kStaffNurse && world.idle_nurses == 0 && could_nurse_vaccinate(world, id)) {
        evacuate_staff(c, id);
        return;
    }
    if (room.state == kRoomRepairDue)
        room.state = kRoomClosed;
    if (old_newrand(world.seed) % 100 < 25) {
        doctor_in_machine_room_call(c, id, room);
        return;
    }
    uint32_t pct = old_newrand(world.seed) % 100;
    if ((pct >= 25 && pct < 27) || cell_of(d) == room.door_inside_tile) {
        doctor_in_machine_room_stroll(c, id);
        return;
    }
    free_staff_break_or_page(c, id, kRoomDoctorSkills[room.type % kRoomTypes]);
}

// doctor_researches's call: with no patient in the room and no member of
// staff dealing with one, the next of its queue (patients sent for an
// autopsy) called (call_person_to_room), the doctor to wait for them.
// Returns whether they were called.
bool doctor_researches_call(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    uint16_t queue = room.queue_head;
    if (!queue)
        return false;
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next) {
        const Thing &occupant = world.things[t];
        if ((occupant.type == kThingStaff && occupant.slave) || occupant.type == kThingPatient)
            return false;
    }
    if (!call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, true))
        return false;
    world.things[id].state = kStaffDoctorWaitsForPatient;
    return true;
}

// doctor_researches's work: of the research tasks in order, the first
// whose object the room has free (find_object, drawing one of them) with
// no other member of staff on its use cell is taken up one time in its
// chance (a draw each). Returns whether one was.
bool doctor_researches_task(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    for (const ResearchTask &task : kResearchTasks) {
        uint16_t obj = find_object(world, cell_of(d), task.object, true, true, d.owner);
        if (!obj)
            continue;
        // search_mapwho: the first member of staff on the object's use cell.
        uint16_t user = 0;
        for (int16_t t = world.map[world.things[obj].use_pos[0]].first; t > 0 && !user;
             t = world.things[size_t(t)].cell_next)
            if (world.things[size_t(t)].type == kThingStaff)
                user = uint16_t(t);
        if (user != 0 && user != id)
            continue;
        if (old_newrand(world.seed, "doctor_researches") % task.chance == 0) {
            d.resume_state = kStaffDoctorResearches;
            d.next_state = kStaffDoctorResearches;
            set_complex(world, id, task.complex, 0);
            process(c, id);
            return true;
        }
    }
    return false;
}

// doctor_researches (0x3b9a0): at each end of their animation, a doctor
// with no research skill leaves (evacuate_staff); one who may resign or
// leave (shall_i_resign) does; else they are to come back to research after
// a break (kOrderReturnToResearch), and leave if the room is being edited
// or they are ordered out (kOrderLeaveRoom, the low orders cleared), take a
// break when ordered or more tired than the policy's rest_threshold
// (take_a_break), and otherwise, a quarter of the time, call a patient for
// an autopsy (doctor_researches_call), else take up a research task
// (doctor_researches_task); failing those, they move off a crowded cell
// (overlapping_staff_check). Status icons are not ported.
void doctor_researches(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    if (c.game_turn % d.anim_speed != 0)
        return;
    d.sprite = c.anims.frames[d.sprite].next;
    if (!(c.anims.frames[d.sprite].flags & 1))
        return;
    if (!(d.skills & kSkillResearcher)) {
        evacuate_staff(c, id);
        return;
    }
    if (shall_i_resign(c, id))
        return;
    d.orders |= kOrderReturnToResearch;
    const RoomInfo &room = world.rooms[d.room];
    if (room.being_edited || room.state == kRoomEdited) {
        evacuate_staff(c, id);
        return;
    }
    if (d.orders & kOrderLeaveRoom) {
        d.orders &= 0xf0;
        evacuate_staff(c, id);
        return;
    }
    if (((d.orders & kOrderRest) || d.tiredness > policy_of(world).rest_threshold) && take_a_break(c, id))
        return;
    if (old_newrand(world.seed) % 100 < 25) {
        if (doctor_researches_call(c, id, room))
            return;
    } else if (doctor_researches_task(c, id)) {
        return;
    }
    overlapping_staff_check(c, id, kStaffDoctorResearches);
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

// nurse_in_ward's call: the next patient of ward `room`'s queue called to
// a free bed (find_unallocated_bed), the nurse to it (substate 1). Returns
// whether she called one.
bool nurse_in_ward_call_patient(const Ctx &c, uint16_t id, const RoomInfo &room)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t queue = room.queue_head;
    if (!queue)
        return false;
    uint16_t bed = find_unallocated_bed(world, nurse.room);
    if (!bed || !call_person_to_room(c.person_run(), id, queue, kComplexCalledToRoom, room.door_tile, false))
        return false;
    nurse.object = bed;
    nurse.next_state = kStaffNurseInWard;
    nurse.substate = 1;
    set_complex(world, id, kComplexNurseToBed, 0);
    process(c, id);
    return true;
}

// nurse_in_ward's stroll to a random cell of the room she stands in (5% its
// middle) she can get to; staying put, now and then she forgets the
// patient she visited last.
void nurse_in_ward_stroll(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t cur = cell_of(nurse);
    const RoomInfo &here = world.rooms[room_at(world, cur)];
    uint32_t pct = old_newrand(world.seed, "nurse_in_ward") % 100;
    uint16_t target;
    if (pct >= 15 && pct < 20) {
        target = uint16_t(here.pos + (((here.height / 2) << 7) + here.width / 2));
    } else {
        // The row is drawn first, as the game's code does.
        auto row = int16_t(old_newrand(world.seed, "nurse_in_ward") % uint32_t(here.height));
        auto column = int16_t(old_newrand(world.seed, "nurse_in_ward") % uint32_t(here.width));
        target = uint16_t(here.pos + ((row << 7) + column));
    }
    if (cur != target && can_i_get_to(world, nurse, target)) {
        nurse.next_state = nurse.resume_state = kStaffNurseInWard;
        set_complex(world, id, kComplexWalkTo, target);
        process(c, id);
        return;
    }
    pct = old_newrand(world.seed, "nurse_in_ward") % 100;
    if (pct >= 0x46 && pct < 0x50)
        nurse.work_target = 0;
}

// nurse_in_ward's substate 0 in ward `room` with desk `desk` (0 if none):
// the decision.
void nurse_in_ward_decide(const Ctx &c, uint16_t id, RoomInfo &room, uint16_t desk)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
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
    if (world.idle_nurses == 0 && could_nurse_vaccinate(world, id)) {
        evacuate_staff(c, id);
        return;
    }
    if (nurse_in_ward_call_patient(c, id, room))
        return;
    if (desk && nurse.work_target != desk && old_newrand(world.seed, "nurse_in_ward") % 100 < 10) {
        nurse.work_target = desk;
        set_complex(world, id, kComplexNurseToDesk, 0);
        nurse.next_state = kStaffNurseInWard;
        process(c, id);
        return;
    }
    uint16_t patient = find_peep(world, nurse.room, true, kThingPatient, kPsInBed, nurse.owner);
    if (patient && patient != nurse.work_target) {
        nurse.work_target = patient;
        nurse.object = world.things[patient].object;
        nurse.next_state = kStaffNurseInWard;
        set_complex(world, id, kComplexNurseVisit, 0);
        process(c, id);
        return;
    }
    nurse_in_ward_stroll(c, id);
}

// nurse_in_ward (0x6c8d8). Substate 0, at each turn: the ward in use (unless
// edited); out if she cannot hold it; she may resign, take an ordered
// break (kOrderRest, take_a_break), answer a page every work_delay + 1
// turns (2 * work_delay + 1 tired) as the policy lets staff leave rooms, or
// take a break when more tired than the policy's rest_threshold; out if the
// ward is being edited; else the queue's next patient called
// to a free bed (find_unallocated_bed); else, 10% of the time, to the
// desk; else to a patient in bed she did not visit last (find_peep, drawn
// from world.seed); else to a random cell of the ward (5% its middle).
// Substate 1: the patient called arrived, she examines them
// (affect_diagnosis) and they go to bed (PS_IN_BED); until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes. Vaccinations and status icons are not ported.
void nurse_in_ward(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &nurse = world.things[id];
    uint16_t desk = find_object(world, cell_of(nurse), kObjDesk, false, false, nurse.owner);
    RoomInfo &room = world.rooms[nurse.room];
    if (nurse.substate == 0) {
        nurse_in_ward_decide(c, id, room, desk);
        return;
    }
    if (nurse.substate != 1)
        return;
    uint16_t pid = nurse.slave;
    Thing &patient = world.things[pid];
    if (patient.state == kPsArrivedFromCall) {
        affect_diagnosis(world, c.anims, id, pid, c.game_turn);
        nurse.substate = 0;
        nurse.slave = 0;
        patient.object = nurse.object;
        nurse.object = 0;
        set_complex(world, pid, kComplexIntoBed, 0);
        patient.next_state = kPsInBed;
        process_person_complex(world, c.anims, c.person, pid, c.game_turn);
    } else if (--nurse.timer <= 0) {
        uint8_t result = call_person_again(c.person_run(), id, kComplexCalledToRoom, room.door_tile);
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

// receptionist_finds_reception (0x97ce0).
void receptionist_finds_reception(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &r = world.things[id];
    uint16_t desk = has_reception(world, r.owner) ? find_free_object_anywhere(world, id, kObjReceptionDesk) : 0;
    if (!desk) {
        reset_icon_value(r, kIconCalling);
        staff_in_corridor(r, c.anims);
        return;
    }
    reset_icon_value(r, kIconCalling);
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
// not kept.
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
            if (call_person_to_room(c.person_run(), id, p, kComplexToDesk, pos, false)) {
                set_icon_value(c.anims, r, kIconCalling);
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
        reset_icon_value(r, kIconCalling);
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
        uint8_t result = call_person_again(c.person_run(), id, kComplexToDesk, pos);
        if (result == 0) {
            reset_icon_value(r, kIconCalling);
            r.state = kStaffReceptionistWorks;
            r.slave = 0;
        } else if (result == 1) {
            reset_icon_value(r, kIconCalling);
            r.state = kStaffReceptionistWorks;
            interrupt_person(world, c.anims, c.person, r.slave, c.game_turn);
            start_person_going(world, c.anims, c.person, r.slave, c.game_turn);
            r.slave = 0;
        }
    } else if (forget_about_my_slave(world, c.anims, c.person, id, kComplexToDesk, c.game_turn)) {
        reset_icon_value(r, kIconCalling);
        r.state = kStaffReceptionistWorks;
        r.slave = 0;
    }
}

// recept_admits_patient (0x984a0).
void recept_admits_patient(World &world, const AnimData &anims, uint16_t id)
{
    Thing &r = world.things[id];
    if (!r.slave)
        return;
    reception_serves_patient(world, anims, id, r.slave);
    RoomInfo &room = world.rooms[r.room];
    room.state = 0;
    room.patients_seen++;
    reset_icon_value(r, kIconCalling);
    r.slave = 0;
    r.state = kStaffReceptionistWorks;
}
} // namespace

// count_surgeons (0x3be38): the members of staff in room `room_id` dressed
// as surgeons (kStaffSurgeon).
uint8_t count_surgeons(const World &world, uint16_t room_id)
{
    uint8_t count = 0;
    const RoomInfo &room = world.rooms[room_id];
    for (uint16_t t = room.first_person; t > 0; t = world.things[t].room_next)
        if (world.things[t].type == kThingStaff && world.things[t].subtype == kStaffSurgeon)
            count++;
    return count;
}

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

namespace
{
// appraise_staff's junior promotion: past their service time, a junior
// becomes a doctor, announced for player 0.
void appraise_promote_junior(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t served)
{
    if (served < uint32_t(gbv.promo_doc) * 1500)
        return;
    uint16_t id = uint16_t(&staff - world.things.data());
    trace_event("appraise %u skill=%u", unsigned(id), unsigned(kSkillJunior));
    staff.skills &= uint16_t(~kSkillJunior);
    if (staff.owner == 0)
        announce_this(world, kAnnJuniorToDoctor, 1);
}

// appraise_staff's consultant promotion: past their service time, a doctor
// becomes a consultant, announced for player 0, and leaves a training room
// they are in.
void appraise_promote_consultant(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t served,
                                 const AnimData *anims, uint32_t game_turn)
{
    if (served < uint32_t(gbv.promo_con) * 1500)
        return;
    uint16_t id = uint16_t(&staff - world.things.data());
    trace_event("appraise %u skill=%u", unsigned(id), unsigned(kSkillConsultant));
    staff.skills |= kSkillConsultant;
    if (staff.owner == 0)
        announce_this(world, kAnnDoctorToConsultant, 1);
    if (staff.room == 0 || world.rooms[staff.room].type != kRoomTraining)
        return;
    staff.next_state = kStaffInCorridor;
    set_complex(world, id, kComplexLeaveRoom, world.rooms[staff.room].door_inside_tile);
    if (!anims)
        return;
    static const PersonComplexes no_person;
    static const StaffComplexes no_staff;
    process_staff_complex(world, *anims, world.person_complexes ? *world.person_complexes : no_person,
                          world.staff_complexes ? *world.staff_complexes : no_staff, id, game_turn);
}

// appraise_staff's promotions: a junior or doctor past the grade's threshold
// and their service time becomes a doctor or consultant. Draws nothing
// itself; the complex a promoted doctor's training room gives may.
void appraise_promote(World &world, Thing &staff, const Balance::Gbv &gbv, uint32_t game_turn, const AnimData *anims)
{
    if (!(staff.skills & kSkillDoctor))
        return;
    uint32_t served = game_turn - uint16_t(staff.hire_time);
    if ((staff.skills & kSkillJunior) && staff.ability > gbv.doctor_threshold)
        appraise_promote_junior(world, staff, gbv, served);
    else if (!(staff.skills & kSkillConsultant) && staff.ability > gbv.consultant_threshold)
        appraise_promote_consultant(world, staff, gbv, served, anims, game_turn);
}
} // namespace

void appraise_staff(World &world, Thing &staff, const Balance &settings, uint32_t game_turn, const AnimData *anims)
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
    appraise_promote(world, staff, gbv, game_turn, anims);
    int happiness = int16_t(staff.happiness);
    happiness += staff.salary < value ? gbv.salary_too_low : gbv.salary_too_high;
    staff.happiness = uint16_t(std::clamp(int(int16_t(happiness)), 0, 100));
}

namespace
{
// place_staff_in_struct's slot: the first free thing short of the
// kStaffReserve last, 0 if none.
uint16_t place_staff_in_struct_free_thing(const World &world)
{
    for (size_t i = 1; i < world.things.size() - kStaffReserve; i++)
        if (world.things[i].type == kThingFree)
            return uint16_t(i);
    return 0;
}

// place_staff_in_struct's nurse: her skill and looks, drawn from `seed`.
void place_staff_in_struct_nurse(Thing &s, uint32_t &seed)
{
    s.skills = kSkillNurse;
    s.female = 1;
    s.face_head = 12;
    s.face_body = uint8_t(staffrand(seed) % 5 + 10);
    s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
}

// place_staff_in_struct's doctor: their skills, each by the market's rate,
// the range of their ability by gbv's thresholds (a consultant's above a
// doctor's, a junior's below) into `ability_min` and `ability_max`, and
// their looks, all drawn from the market's seed.
void place_staff_in_struct_doctor(Thing &s, StaffMarket &market, const Balance::Gbv &gbv, uint32_t &ability_min,
                                  uint32_t &ability_max)
{
    uint32_t &seed = market.seed;
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
}

// place_staff_in_struct's handyman: his skill, priorities and looks, drawn
// from `seed`.
void place_staff_in_struct_handyman(Thing &s, uint32_t &seed)
{
    s.skills = kSkillHandyman;
    s.female = 0;
    s.prio_litter = 0x21;
    s.prio_plants = 0x21;
    s.prio_repair = 0x22;
    s.face_head = uint8_t(staffrand(seed) % 3 + 15);
    s.face_body = uint8_t(staffrand(seed) % 3 + 15);
    s.face_legs = uint8_t(staffrand(seed) % 3 + 15);
}

// place_staff_in_struct's receptionist: her skill and looks, drawn from
// `seed`.
void place_staff_in_struct_receptionist(Thing &s, uint32_t &seed)
{
    s.skills = kSkillReceptionist;
    s.female = 1;
    s.face_head = uint8_t(staffrand(seed) % 4 + 10);
    s.face_body = uint8_t(staffrand(seed) % 5 + 10);
    s.face_legs = uint8_t(staffrand(seed) % 5 + 10);
    if (s.face_head == 12)
        s.face_head = 14;
}
} // namespace

uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn)
{
    uint16_t id = place_staff_in_struct_free_thing(world);
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
    set_applicant_expiry(s, game_turn + 2500 + staffrand(seed) % 2500);
    s.work_delay = uint8_t(normal_staffrand(seed, 1, 10));
    s.happiness = 100;
    s.warmth = 50;
    s.state = kStaffApplicant;
    market.applicant_count++;
    market.applicants[kind]++;

    uint32_t ability_min = 30, ability_max = 1000;
    switch (kind) {
    case kStaffNurse:
        place_staff_in_struct_nurse(s, seed);
        break;
    case kStaffDoctor:
        place_staff_in_struct_doctor(s, market, settings.gbv, ability_min, ability_max);
        break;
    case kStaffHandyman:
        place_staff_in_struct_handyman(s, seed);
        break;
    case kStaffReceptionist:
        place_staff_in_struct_receptionist(s, seed);
        break;
    }
    s.skills |= extra_skills;
    s.ability = uint16_t(normal_staffrand(seed, ability_min, ability_max));
    appraise_staff(world, s, settings, game_turn, nullptr);
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
        for (int j = market.applicants[kind]; j < market.wanted[kind]; j++)
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

namespace
{
// place_staff_on_map's new hire, hired at `game_turn`: no orders,
// discontent or resignations yet, and their skills now count
// (staff_with_skill). The staff counts are not kept.
void place_staff_on_map_hire(World &world, uint16_t id, uint32_t game_turn)
{
    Thing &s = world.things[id];
    s.resign_count = 0;
    s.resign_time = 0;
    s.orders = 0;
    s.discontent = 0;
    clear_peep_icon(s);
    set_applicant_expiry(s, 0);
    s.hire_time = int16_t(game_turn);
    set_expertise_staff(world, id);
    if (world.staff_market) {
        world.staff_market->applicant_count--;
        world.staff_market->applicants[s.subtype]--;
    }
}
} // namespace

bool place_staff_on_map(World &world, const StaffSetup &setup, int cell, uint16_t id, bool force)
{
    const AnimData &anims = setup.anims;
    const uint32_t game_turn = setup.game_turn;
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
    calculate_speed(world, s, setup.settings);
    insert_node(world, id, s.x, s.y);
    // With no one held, a new hire.
    if (world.held_staff == 0)
        place_staff_on_map_hire(world, id, game_turn);
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
} // namespace

uint32_t applicant_expiry(const Thing &t)
{
    // Thing+0x96-0x97 is work_target2 for a handyman and crazy with
    // surgeon_training for a doctor (include/hosp.h); neither kind uses the
    // other's fields, so the union's bytes are whichever applies.
    uint32_t high = t.type == kThingStaff && t.subtype == kStaffHandyman
                        ? t.work_target2
                        : uint32_t(t.crazy) | (uint32_t(t.surgeon_training) << 8);
    return uint32_t(t.work_target) | (high << 16);
}

void set_applicant_expiry(Thing &t, uint32_t turn)
{
    t.work_target = uint16_t(turn);
    if (t.type == kThingStaff && t.subtype == kStaffHandyman) {
        t.work_target2 = uint16_t(turn >> 16);
    } else {
        t.crazy = uint8_t(turn >> 16);
        t.surgeon_training = uint8_t(turn >> 24);
    }
}

namespace
{
// remove_staff_in_struct (0x2a130): deletes an applicant, taking them off the
// job market's counts.
void remove_staff_in_struct(World &world, Thing &staff)
{
    if (world.staff_market) {
        world.staff_market->applicants[staff.subtype]--;
        world.staff_market->applicant_count--;
    }
    staff = Thing{};
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

void interrupt_staff(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    interrupt_staff_in(ctx_of(world, anims, game_turn), id);
}

void staff_abandon_for_pick_up(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn)
{
    Ctx c = ctx_of(world, anims, game_turn);
    Thing &s = world.things[id];
    if (s.subtype == kStaffReceptionist) {
        set_complex(world, id, kComplexOffObject, 0);
        s.next_state = get_state(0, s);
        staff_abandon_complex(c, id);
        play_staff_anim(anims, s, kAnimWalk);
    } else {
        staff_abandon_complex(c, id);
        s.complex_action = 0;
    }
}

void sack_staff(World &world, const AnimData &anims, uint16_t id, uint8_t mode, uint32_t game_turn)
{
    sack(ctx_of(world, anims, game_turn), id, mode);
}

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
        // req2_mark_employing; the advisor's staff happiness total is not
        // ported.
        if (is_staff_event_thing(world, event.arg))
            world.things[event.arg].state = kStaffHeld;
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
