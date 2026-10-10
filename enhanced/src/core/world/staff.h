#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/data/text.h"
#include "core/world/complex.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

class Game;
struct Event;
struct StaffComplexes;

// Staff kinds (Thing.subtype of kThingStaff), the game's STAFF_*.
constexpr uint8_t kStaffNurse = 0;
constexpr uint8_t kStaffDoctor = 1;
constexpr uint8_t kStaffHandyman = 2;
constexpr uint8_t kStaffReceptionist = 3;
constexpr uint8_t kStaffSurgeon = 4;

// The handyman window's priority buttons, counted from 1 in its list
// (hires_requester_buttons[0x27] to [0x29], src/reqbut.cpp): litter, plants
// and repairs. EV_ADJUST_HANDYMAN_VALUES carries one in `arg`.
constexpr uint8_t kHandymanLitterButton = 29;
constexpr uint8_t kHandymanPlantsButton = 30;
constexpr uint8_t kHandymanRepairButton = 31;

// Thing.skills bits (the game's SKILL_*).
constexpr uint16_t kSkillReceptionist = 0x1;
constexpr uint16_t kSkillHandyman = 0x2;
constexpr uint16_t kSkillNurse = 0x4;
constexpr uint16_t kSkillJunior = 0x8;
constexpr uint16_t kSkillDoctor = 0x10;
constexpr uint16_t kSkillSurgeon = 0x20;
constexpr uint16_t kSkillPsychiatrist = 0x40;
constexpr uint16_t kSkillConsultant = 0x80;
constexpr uint16_t kSkillResearcher = 0x100;

// Staff states handled so far (the game's SS_*, include/enums.h).
constexpr uint8_t kStaffDoctorInOffice = 0x06;
constexpr uint8_t kStaffDoctorWorksAtDesk = 0x08;
constexpr uint8_t kStaffReceptionistWorks = 0x0a;
constexpr uint8_t kStaffReceptionistFindsReception = 0x0c;
constexpr uint8_t kStaffPlayComplexAnim = 0x0e;
constexpr uint8_t kStaffGotoObject = 0x12;
constexpr uint8_t kStaffDoctorWaitsForPatient = 0x14;
constexpr uint8_t kStaffTwoPeepsInteract = 0x1a;
constexpr uint8_t kStaffDoctorExamines = 0x1c;
constexpr uint8_t kStaffReceptWaitsForPatient = 0x20;
constexpr uint8_t kStaffReceptAdmitsPatient = 0x22;
constexpr uint8_t kStaffWaitForComplexSlave = 0x26;
constexpr uint8_t kStaffInCorridor = 0x28;
constexpr uint8_t kStaffComplexRetry = 0x2c;
constexpr uint8_t kStaffApplicant = 0x2e;
constexpr uint8_t kStaffInStaffRoom = 0x30;
constexpr uint8_t kStaffWanders = 0x34;
constexpr uint8_t kStaffEvacuate = 0x36;
constexpr uint8_t kStaffBackToApplicants = 0x38;
constexpr uint8_t kStaffHandymanWorks = 0x3c;
constexpr uint8_t kStaffHeld = 0x44;
constexpr uint8_t kStaffReceptionistSetup = 0x4a;
constexpr uint8_t kStaffBpGoto = 0x4c;
constexpr uint8_t kStaffStuck = 0x4e;
constexpr uint8_t kStaffPlayAniseq = 0x50;

// Thing.owner of applicants and of staff who left: no hospital's.
constexpr uint8_t kNoOwner = 0x63;

// Applicants: the game turn they leave the list. The game keeps this as the
// u32 at Thing+0x94 and shares those bytes with work_target, work_target2,
// crazy and surgeon_training, so a sacked handyman or doctor keeps the game's
// huge expiry; the port derives it from those fields (work_target2's bytes are
// crazy and surgeon_training's for a doctor). Setting it writes the bytes
// back (0 clears them, as hiring does).
uint32_t applicant_expiry(const Thing &t);
void set_applicant_expiry(Thing &t, uint32_t turn);

// The job market (the game's gamevar fields of create_job_market): the
// staff random generator and this month's wanted applicants and doctor
// skill odds. Reset it when a level starts, as the game clears gamevar.
struct StaffMarket {
    // The generator applicants are made with (gamevar.staff_seed), apart
    // from World::seed.
    uint32_t seed = 0;
    // Per kind (nurse, doctor, handyman, receptionist), the applicants
    // wanted.
    std::array<uint16_t, 4> wanted{};
    // Per kind, the applicants the game believes it has (gamevar.applicants):
    // counted as they are made, hired and sent back, so a thing freed any
    // other way (a shot rat index landing on an applicant) leaves it too
    // high. applicant_count is their total (gamevar.applicant_count).
    std::array<uint16_t, 4> applicants{};
    uint16_t applicant_count = 0;
    // 1 in n new doctors is a psychiatrist, surgeon, researcher, consultant,
    // or (if not a consultant) junior; 0 for none.
    uint8_t shrink_rate = 0;
    uint8_t surgeon_rate = 0;
    uint8_t researcher_rate = 0;
    uint8_t consultant_rate = 0;
    uint8_t junior_rate = 0;
    // The language's numbers of general, good and bad references
    // (statvar.ref_*_count; see reference_counts), which applicants' draws
    // are taken modulo. Set it before making applicants.
    std::array<uint8_t, 3> ref_counts{1, 1, 1};
};

// setup_req4_references (0x87fb8): the strings of language sections 0x2d,
// 0x2e and 0x2f before the first starting with '.'.
std::array<uint8_t, 3> reference_counts(const LanguageText &text);

// Advances `seed` as the game's staffrand (src/rand.cpp) and returns its
// low 16 bits.
uint16_t staffrand(uint32_t &seed);

// normal_staffrand: low plus the sum of two staffrand draws modulo
// (high - low) / 2 + 1.
uint32_t normal_staffrand(uint32_t &seed, uint32_t low, uint32_t high);

// The applicants of staff kind `kind` as the world holds them: those
// listed (state kStaffApplicant) and those being hired (kStaffHeld, not yet
// anyone's).
int count_applicants(const World &world, uint8_t kind);

// remove_staff_in_struct (0x2a130): deletes the applicant `staff`, taking them
// off the job market's counts and freeing the thing.
void remove_staff_in_struct(World &world, Thing &staff);

// create_job_market (0x29ad0), at a level's start and each month's end
// (`game_month` the months since the level began): takes the settings'
// staff_levels entry of that month, if any, into `market` (reseeding it),
// then makes applicants of each kind up to the number wanted
// (place_staff_in_struct). `game_turn` is the game's turn count.
void create_job_market(World &world, StaffMarket &market, const Balance &settings, int game_month, uint32_t game_turn);

// place_staff_in_struct (0x2a178): makes an applicant of staff kind `kind`
// (0-3) with skills `extra_skills` added: looks, face, doctor skills by the
// market's odds, ability, wage demand (as appraise_staff), name and
// reference, expiring 2500-5000 turns after `game_turn`, all drawn from
// market.seed in the game's order. Returns its thing, 0 if none was free.
uint16_t place_staff_in_struct(World &world, StaffMarket &market, const Balance &settings, uint8_t kind,
                               uint16_t extra_skills, uint32_t game_turn);

// appraise_staff (0x72374): sets the wage `staff` asks for and makes them
// happier or less happy by whether their salary meets it. A junior or doctor
// whose ability and time served allow it is promoted to doctor or
// consultant, announced for player 0; a doctor promoted while in a training
// room leaves it (with `anims`, the world's action sequences; null when
// there are none, as at a level's start). `game_turn` is the game's turn
// count.
void appraise_staff(World &world, Thing &staff, const Balance &settings, uint32_t game_turn, const AnimData *anims);

// The animation (an AnimData::starts index, the facing to be added) a staff
// kind shows when placed or held (staff_anim_starts[kind].anims[staff[kind].anim]).
uint16_t staff_held_anim(uint8_t kind);

// Whether staff of `kind` may be put down on `cell` (column + row * 128) by
// player 0, as draw_staff_cursor (0x2b690) decides it: in a room, the kind
// has a state there (staff_states), the cell a path to the room's inside
// door cell, and the room is built (state 0 or 1; any state for a
// handyman); elsewhere, a path to the hospital's first entrance. Unlike the
// game's cursor, a cell not on player 0's land is refused too, as
// place_staff_on_map then refuses it.
bool staff_place_ok(const World &world, int cell, uint8_t kind);

// What placing staff reads besides the world: the animations, the level's
// settings and the turn being played (the game's actual_game_turn).
struct StaffSetup {
    const AnimData &anims;
    const Balance &settings;
    uint32_t game_turn;
};

// place_staff_on_map (0x29c4c) for player 0: puts staff member `id` on
// `cell` if the cell is player 0's (or `force`), standing in the middle
// facing as it was (0 if stuck), in the state for its kind of the room it
// is in (holding the room in place of whoever did, where it may work) or of
// the corridor, walking at its ability's speed. It becomes player 0's;
// unless a picked-up member of staff is held (World::held_staff, whoever
// `id` is), it is a new hire: its hire turn `setup.game_turn`, notice and
// discontent cleared, and the treatments its skills complete available
// (set_expertise_staff). It answers the page to its room, if any. Once
// placed, no one is held. Returns whether it was placed.
bool place_staff_on_map(World &world, const StaffSetup &setup, int cell, uint16_t id, bool force);

// One game turn of staff member `id`, as process_things's staff states,
// drawing from world.seed as the game does:
// - an applicant past its expiry withdraws (the thing is freed);
// - one in the corridor starts wandering (staff_in_corridor);
// - wanderers stroll and pause (staff_wanders); a receptionist among them
//   goes to find a free reception desk once there is one, and a handyman
//   goes to work;
// - a receptionist finds a free desk and walks to it, or wanders if there
//   is none (receptionist_finds_reception), takes her place at it
//   (receptionist_setup: a reception, World::rooms, shared with manned
//   desks within 10 cells) and works there (receptionist_works,
//   recept_waits_for_patient, recept_admits_patient), calling and serving
//   the patients queueing for her through core/world/reception.h;
// - a handyman (handyman_works) repairs the machines paged for repair
//   (core/world/rooms.h's thingpage_*): into the room, closed for it, the
//   repair's animation sequence (SS_PLAY_ANISEQ), the machine's wear gone;
//   with no machine urgent enough, he waters the plants paged for it
//   (PlantPager) or sweeps litter he spots, then strolls to a random cell
//   near by;
// - walking to an object (staff_goto_object), the complexes' states
//   (core/world/staff_complex.h), being stuck (peep_stuck) and walking off
//   a blueprint (peep_bp_goto);
// - answering pages (obey_pager, core/world/rooms.h) when deciding where to
//   wander, and going out of a room (SS_EVACUATE);
// - a doctor in a GP's office: to the desk (doctor_in_office), working there
//   and calling the queue's patients (doctor_works_at_desk), waiting for
//   the one called (doctor_waits_for_patient_to_arrive) and the
//   consultation (doctor_carries_out_examination);
// - a doctor in a diagnosis room (doctor_in_machine_room): calling the
//   queue's patients, strolling in the room, waiting for the one called and
//   the room's examination (general diagnosis, cardiogram, scanner,
//   ultrascan, blood machine, X-ray, psychiatry), wearing its machine
//   (treatment_machine_used);
// - a nurse in a ward (nurse_in_ward): calling the queue's patients to a
//   free bed, examining them as they get into it, visiting the desk and the
//   patients in bed, strolling in the ward;
// - handing in notice and leaving when it runs out (shall_i_resign,
//   sack_staff), and going back to the applicants off the map at the
//   entrance (SS_BACK_TO_APPLICANTS);
// - breaks (take_a_break): ordered (EV_ORDER_REST) or, more tired than the
//   policy's rest_threshold, when deciding what to do (wandering, a
//   handyman, a doctor free at the desk or in a diagnosis room, a nurse in
//   a ward), to the nearest staff room, resting there (staff_in_staff_room)
//   on its sofa, pool table and video game until no longer tired, then
//   back to work;
// - first, modify_staff: tiredness, warmth, happiness, discontent and
//   cracking up.
// Poaching is not ported.
void staff_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

// interrupt_staff (0x2c3bc): member of staff `id` stops what it does (gets
// off its door or object), frees the patient it deals with in a room
// (interrupt_person) and goes to the state for where it is, its pages
// dropped; a surgeon's partner too. Uses the action sequences
// start_rooms_in_use gave the world. A handyman drops his repair pages.
void interrupt_staff(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// count_surgeons (0x3be38): the members of staff in room `room_id` dressed as
// surgeons (kStaffSurgeon). The surgeon screen's change complexes use it.
uint8_t count_surgeons(const World &world, uint16_t room_id);

// adjust_handyman_values (0x87c44): raises handyman `id`'s priority for the
// job of `button` (kHandymanLitterButton, kHandymanPlantsButton or
// kHandymanRepairButton) by 6, taking from the larger of the other two
// until the three sum to at most 100; from 94 it becomes 100 and the others
// 0. Another button changes no priority. Either way his counts of jobs done
// (litter_count, plant_count, repair_count) start again from 0.
void adjust_handyman_values(World &world, uint16_t id, uint8_t button);

// plant_ages (0x7b88c): one turn of object `id` in OS_PLANT_AGES. Every
// 16th turn, half the time (one draw from world.seed), it loses a point of
// health; every 80 points it wilts a stage and pages a handyman unless it
// cannot be reached, counting it for the advisor (World::thirsty_plants,
// plant_health_sum); once dead it pages urgently 40 points past zero and
// starts again. Also advances its sprite at a wilting stage.
void plant_ages(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// sack_staff (0x2a804): member of staff `id` leaves the hospital. Mode 0
// (sacked) pays them a month's wage (kTxSeverancePay), mode 1 (notice run
// out) clears their notice; both interrupt them (interrupt_staff) and walk
// them out of their room and to the hospital's first entrance, where they
// go off the map and back to the applicants (kStaffApplicant, with no
// expiry left, so withdrawn the next turn). Mode 2 removes them at once.
// Their skills no longer count (remove_expertise_staff), they become
// kNoOwner and their notice, discontent and resignations are cleared.
// Does nothing but release their room for staff already kNoOwner. Uses the
// world's action sequences and accounts (start_rooms_in_use).
void sack_staff(World &world, const AnimData &anims, uint16_t id, uint8_t mode, uint32_t game_turn);

// If a picked-up member of staff is held (World::held_staff), puts them
// back on the cell they were taken from, wherever that is
// (place_staff_on_map forced), as the game does before a chart, at the
// year's end and on EV_CANCEL_BUILD. Returns whether one was held.
bool put_back_held_staff(Game &game);

// pick_up_object's user branch (src/build.cpp) for a member of staff:
// complex 0x1b8, the state for where they are, their object abandoned and
// the walk frame for a receptionist; any other user's complex abandoned,
// its action cleared.
void staff_abandon_for_pick_up(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// Carries out the simulation's part of the commands of hiring, placing and
// picking up staff, as process_turn (src/io.cpp) for player 0:
// - EV_MARK_EMPLOYING: the applicant is held;
// - EV_PLACE_STAFF: placed, and with no picked-up member of staff held the
//   hiring fee of 12/10 of the wage paid;
// - EV_PICK_UP_STAFF: `value` becomes World::held_staff and, unless they
//   handed in notice or are a surgeon whose screen is in use, is
//   interrupted (interrupt_staff) and picked up (staff_picked_up: out of
//   their room and complex, off the map, state kStaffHeld, their cell kept
//   in World::held_staff_tile); held_staff is set even when refused, as in
//   the game;
// - EV_STAFF_PICKED_UP: staff_picked_up alone;
// - EV_CANCEL_BUILD: the picked-up member of staff put back
//   (put_back_held_staff), else a held applicant back on the list;
// - EV_SACK_STAFF: `value` sacked with mode `arg` (sack_staff);
// - EV_ADJUST_HANDYMAN_VALUES: handyman `value`'s priorities, by the button
//   in `arg`'s low byte (adjust_handyman_values);
// - EV_ORDER_REST, EV_ORDER_TRAIN, EV_ORDER_LEAVE_ROOM and command 0x60:
//   the order (kOrderRest 1, 4, 8 and 2) replaces the low four bits of
//   member of staff `value`'s Thing.orders;
// - EV_ANSWER_PLAYERCALL with `value` 1: the staff's resignation call `arg`
//   removed (the requester it opens is core/ui/staff_demand.h's);
// - that requester's answers about member of staff `value`:
//   EV_REFUSE_STAFF_DEMAND (a draw: they leave, sack_staff mode 1, unless it
//   is a multiple of their resignations plus one, when they stay),
//   EV_RAISE_STAFF_SALARY (by `arg`) and EV_GIVE_STAFF_BONUS (of `arg`,
//   kTxPersonalBonus, happiness up 50); staying takes back their notice and
//   discontent, and the advisor's resignation news.
// The interface's part is hire_staff_event and hire_staff_after_turn
// (core/ui/hire_staff.h) and the staff list's (core/ui/charts.h). Returns
// false for other commands (other player calls too) and for
// EV_CANCEL_BUILD, which the other steps also act on.
bool apply_staff_event(Game &game, const Event &event);

} // namespace th
