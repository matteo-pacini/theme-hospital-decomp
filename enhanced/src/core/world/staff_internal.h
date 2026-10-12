#pragma once

// What the staff_*.cpp files of core/world share: the staff states and complexes'
// constants, Ctx, and the functions more than one of them calls.

#include "core/world/staff.h"

#include <array>
#include <cstdint>

namespace th
{
inline namespace staff_detail
{

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
constexpr uint8_t kRoomDnaFixer = 18;

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

// Object kinds (the game's OBJ_*) in a GP's office.
constexpr uint8_t kObjDesk = 1;
constexpr uint8_t kObjCabinet = 2;
constexpr uint8_t kObjChair = 6;

// A psychiatrist's bookcase; a ward's bed.
constexpr uint8_t kObjBookcase = 56;
constexpr uint8_t kObjBed = 8;

// The player whose hospital the engine plays.
constexpr uint8_t kPlayer = 0;

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

// The complexes a doctor gives the patient: called to the room's door and
// in, and to the chair in a GP's office.
constexpr uint16_t kComplexCalledToRoom = 0x28;
constexpr uint16_t kComplexSitOnChair = 0x82;

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

// The animation columns of kStaffAnimStarts with a name in the game.
constexpr size_t kAnimWalk = 0;
constexpr size_t kAnimStand = 5;

struct Ctx {
    World &world;
    const AnimData &anims;
    const PersonComplexes &person;
    const StaffComplexes &staff;
    uint32_t game_turn;

    PersonRun person_run() const { return {world, anims, person, game_turn}; }
};

// staff.cpp
void calculate_speed(const World &world, Thing &staff, const Balance &settings);
void staff_in_corridor(Thing &staff, const AnimData &anims);
Ctx ctx_of(World &world, const AnimData &anims, uint32_t game_turn);
uint16_t cell_of(const Thing &t);
void play_staff_anim(const AnimData &anims, Thing &s, size_t column);
uint8_t process(const Ctx &c, uint16_t id);
bool can_i_get_to(const World &world, const Thing &person, uint16_t pos);
bool has_litter(const MapCell &cell);
bool has_reception(const World &world, uint8_t owner);
bool shall_i_resign(const Ctx &c, uint16_t id);
uint16_t find_free_object_anywhere(const World &world, uint16_t id, uint8_t kind);
const HospitalSettings &policy_of(const World &world);
uint8_t get_state(uint8_t type, const Thing &s);
void staff_exit_room_check(World &world, Thing &s);
bool check_room_mutex(World &world, uint16_t id);
void evacuate_staff(const Ctx &c, uint16_t id);
bool obey_pager(const Ctx &c, uint16_t id);

// staff_handyman.cpp
void handyman_check_for_litter(World &world, Thing &h);
void play_aniseq(const Ctx &c, Thing &s);
void handyman_works(const Ctx &c, uint16_t id);

// staff_room.cpp
bool take_a_break(const Ctx &c, uint16_t id);
bool overlapping_staff_check(const Ctx &c, uint16_t id, uint8_t state);
void doctor_is_trained(const Ctx &c, uint16_t id);
void consultant_trains(const Ctx &c, uint16_t id);
void staff_in_staff_room(const Ctx &c, uint16_t id);

// staff_interrupt.cpp
void interrupt_staff_in(const Ctx &c, uint16_t id);
void staff_picked_up(const Ctx &c, uint16_t id);
void sack(const Ctx &c, uint16_t id, uint8_t mode);

// staff_theatre.cpp
bool surgeon_screen_free(World &world, uint16_t id);
void change_lockholder(const Ctx &c, uint16_t id, uint16_t room_id);
bool am_i_lead_staff(const World &world, uint16_t id);
uint16_t find_waiting_surgeon(const World &world, uint16_t room_id, uint8_t state);
void change_surgeon_to_doc(World &world, uint16_t id);
void doctor_in_theatre(const Ctx &c, uint16_t id);
void surgeon_waits(const Ctx &c, uint16_t id);
void stop_waiting_for_patient(World &world, Thing &s);
void doctor_waits_for_patient_theatre(const Ctx &c, uint16_t id, const RoomInfo &room);
void doctor_examines_theatre(const Ctx &c, uint16_t id);
void doctor_examines_theatre_done(const Ctx &c, uint16_t id);

// staff_doctor.cpp
bool holds_room(const Ctx &c, uint16_t id);
void doctor_in_office(const Ctx &c, uint16_t id);
void doctor_works_at_desk(const Ctx &c, uint16_t id);
void doctor_in_machine_room(const Ctx &c, uint16_t id);
void doctor_researches(const Ctx &c, uint16_t id);

// staff_examine.cpp
void doctor_waits_for_patient(const Ctx &c, uint16_t id);
void doctor_examines(const Ctx &c, uint16_t id);

// staff_nurse.cpp
void reinit_nurse(World &world, uint16_t id);
void nurse_in_ward(const Ctx &c, uint16_t id);

// staff_reception.cpp
void receptionist_finds_reception(const Ctx &c, uint16_t id);
void receptionist_setup(World &world, uint16_t id);
void receptionist_works(const Ctx &c, uint16_t id);
void recept_waits_for_patient(const Ctx &c, uint16_t id);
void recept_admits_patient(World &world, const AnimData &anims, uint16_t id);

} // namespace staff_detail

} // namespace th
