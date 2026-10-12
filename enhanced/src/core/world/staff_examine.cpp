// A doctor examining and treating a patient in the diagnosis and treatment
// rooms (src/doctor.cpp, src/examine.cpp).

#include "core/world/staff.h"

#include "core/world/arrivals.h"
#include "core/world/complex_tables.h"
#include "core/world/nav.h"
#include "core/world/patient_tables.h"
#include "core/world/queue.h"
#include "core/world/reception.h"
#include "core/world/rooms.h"
#include "core/world/staff_internal.h"
#include "core/world/trace.h"

#include <algorithm>

namespace th
{

namespace
{

// The patient's complex out of their room by its door (P_COMP.LEX).
constexpr uint16_t kComplexWalkOut = 90;

// The examinations a doctor starts once the patient called has arrived
// (doctor_waits_for_patient_to_arrive), by room type: the doctor's complex
// (S_COMP.LEX) and the SS_DOCTOR_EXAMINES substate it leads to; complex 0 for the rooms the game's switch leaves to its
// default (the operating theatre has its own path, doctor_waits_for_patient_theatre).
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
    e[kRoomDnaFixer] = {0x47e, 240};
    e[kRoomPharmacy] = {0x244, 180};
    e[kRoomHairRestorer] = {0x2bc, 120};
    e[kRoomSlackTongue] = {0x1cc, 130};
    e[kRoomFracture] = {0x42e, 140};
    e[kRoomElectrolysis] = {0x186, 160};
    e[kRoomJellyVat] = {0x19a, 170};
    e[kRoomDecontamination] = {0x28a, 210};
    e[kRoomResearch] = {0x35c, 220};
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

// The DNA fixer's: the animation column the patient steps into the machine
// with, and the doctor's complex once the patient is fixed (S_COMP.LEX).
constexpr size_t kAnimDnaStep = 0x79;
constexpr uint16_t kComplexDnaFixed = 0x488;

// send_person_somewhere_else (0x17d2c): the patient walks out of their room
// and on to their next treatment.
void send_person_somewhere_else(const Ctx &c, uint16_t id)
{
    Thing &p = c.world.things[id];
    p.next_state = kPsGoForTreatment;
    if (set_complex(c.world, id, kComplexWalkOut, c.world.rooms[p.room].door_inside_tile))
        process_person_complex(c.world, c.anims, c.person, id, c.game_turn);
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
} // namespace

inline namespace staff_detail
{

// doctor_waits_for_patient_to_arrive (0x3aba4): once the patient called
// has arrived, both start the room's examination (a GP's consultation, the
// patient to the chair; the diagnosis rooms' in kExaminations), the doctor
// to SS_DOCTOR_EXAMINES; a cracked-up doctor first sends one patient in
// four (a draw from world.seed) on elsewhere. Until then the call is
// repeated every 200 turns (call_person_again) and given up on a patient
// who no longer comes (forget_about_my_slave). The rooms the game's switch leaves to its default (ward, training, staff
// room, toilets) do nothing: their patients wait.
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

} // namespace staff_detail

namespace
{

// doctor_examines's substate 220, an autopsy done: unless the autopsy
// machine broke down (treatment_machine_used, for kIllAutopsy whatever the
// patient's treatment), the doctor back to research (complex
// kComplexBackToWork), free again, and the patient removed.
void doctor_examines_autopsy_done(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    uint16_t pid = d.slave;
    if (treatment_machine_used(world, c.anims, kIllAutopsy, cell_of(world.things[pid]), c.game_turn))
        return;
    d.next_state = kStaffDoctorResearches;
    set_complex(world, id, kComplexBackToWork, 0);
    process(c, id);
    d.slave = 0;
    world.rooms[d.room].staff_free = 1;
    remove_person(world, c.anims, c.person, pid, c.game_turn);
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

// full_draw_frame's frame_bounds.y for frame `frame` drawn for `p` with
// nothing shown: the least y offset from the frame's origin of the elements
// `p`'s layers pick, 999 if none.
int frame_top(const AnimData &anims, uint16_t frame, const Thing &p)
{
    int top = 999;
    for (size_t i = anims.frames[frame].list; i < anims.lists.size() && anims.lists[i] != kListEnd; i++) {
        const AnimElement &e = anims.elements[anims.lists[i]];
        if ((e.flags & 0xffb0) && p.layers[size_t((e.flags >> 4) & 0x1f) % p.layers.size()] != e.flags >> 9)
            continue;
        if (e.sprite != 0)
            top = std::min(top, e.y - kElementBiasY);
    }
    return top;
}

// doctor_examines's substate 240: the patient steps into the DNA fixer (241),
// PS_FILLS: its alien frame shown above the machine's split line and its
// own kind's below, the line starting its first element's height under the
// frame's top.
void doctor_examines_dna_enter(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    Thing &patient = world.things[d.slave];
    d.substate++;
    patient.state = kPsFills;
    Thing &obj = world.things[patient.object];
    play_frame(patient, c.anims, kPatientAnimStarts[patient.subtype][kAnimDnaStep]);
    patient_start_frame(c.anims, patient.orig_subtype, kAnimDnaStep, patient.facing, obj.aux_sprite, obj.aux_flip);
    const AnimElement &first = c.anims.elements[c.anims.lists[c.anims.frames[patient.sprite].list]];
    const size_t at = first.sprite / 6;
    const int height = at < c.anims.sprite_heights.size() ? c.anims.sprite_heights[at] : 0;
    obj.split_height = uint8_t(-height - frame_top(c.anims, patient.sprite, patient));
}

// doctor_examines's substate 241: the patient filled (PS_SLAVE_READY), cured
// of its alien DNA and merged with the machine's animation; the doctor's
// last complex, then the end of the treatment (230).
void doctor_examines_dna_fixed(const Ctx &c, uint16_t id)
{
    World &world = c.world;
    Thing &d = world.things[id];
    Thing &patient = world.things[d.slave];
    if (patient.state != kPsSlaveReady)
        return;
    patient.state = kPsMerged;
    cure_illness(world, d.slave);
    set_complex(world, id, kComplexDnaFixed, 0);
    d.next_state = kStaffDoctorExamines;
    process(c, id);
    d.substate = 230;
}

// doctor_examines's pharmacy substates (180-206) and the DNA fixer's (the
// patient steps in, 240, then is fixed, 241); the others do nothing.
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
    case 240:
        doctor_examines_dna_enter(c, id);
        break;
    case 241:
        doctor_examines_dna_fixed(c, id);
        break;
    default:
        break;
    }
}
} // namespace

inline namespace staff_detail
{

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
// back; research (220), an autopsy done (doctor_examines_autopsy_done). In
// the DNA fixer (240, 241) the patient steps into the machine and is cured of
// alien DNA, merged.
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
    case 220:
        doctor_examines_autopsy_done(c, id);
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

} // namespace staff_detail

} // namespace th
