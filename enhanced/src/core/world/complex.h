#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace th
{

// Patients' action sequences ("complexes"), as the game's complex.cpp, and
// the patient states that run them, as process_things (src/things.cpp) and
// peeps.cpp.
//
// A complex is a run of step bytes in PersonComplexes::group, ending at 0,
// each a kPersonComplex index (core/world/complex_tables.h). Complexes are
// named by where they start; the game's start at multiples of 10. A person
// runs a step when the complex is given, on arrival at its destination
// (PS_GOTO_DESTINATION) or when an animation ends (PS_PLAY_COMPLEX_ANIM).

// Patient states (Thing.state; the game's PS_*, include/enums.h).
constexpr uint8_t kPsWaitsForBusy = 0x04;
constexpr uint8_t kPsGotoDestination = 0x06;
constexpr uint8_t kPsInBed = 0x2c;
constexpr uint8_t kPsFills = 0x2e;
constexpr uint8_t kPsEnergises = 0x30;
constexpr uint8_t kPsReturnToQueueArea = 0x08;
constexpr uint8_t kPsGoHome = 0x0c;
constexpr uint8_t kPsGone = 0x0e;
constexpr uint8_t kPsPlayComplexAnim = 0x10;
constexpr uint8_t kPsArrivesAtDoor = 0x12;
constexpr uint8_t kPsInQueue = 0x14;
constexpr uint8_t kPsGotoObject = 0x16;
constexpr uint8_t kPsEntertainsThemself = 0x18;
constexpr uint8_t kPsGotoQueue = 0x24;
constexpr uint8_t kPsGoForTreatment = 0x26;
constexpr uint8_t kPsSlaveReady = 0x20;
constexpr uint8_t kPsMerged = 0x28;
constexpr uint8_t kPsComplexRetry = 0x2a;
constexpr uint8_t kPsInBogs = 0x36;
constexpr uint8_t kPsLeaveToilets = 0x38;
constexpr uint8_t kPsStuck = 0x3c;
constexpr uint8_t kPsBpGoto = 0x40;
constexpr uint8_t kPsWaitsForDecision = 0x44;

// The complex that sends a patient to its destination cell and off the
// map (start_person_going, person_go_home).
constexpr uint16_t kComplexLeave = 20;
// The complexes a waiting patient runs: to a drinks machine for a drink,
// to a bench to sit, to a free cell to stand (find_standing_place).
constexpr uint16_t kComplexDrink = 60;
constexpr uint16_t kComplexBench = 70;
constexpr uint16_t kComplexStandingPlace = 430;
// The complex that walks a patient to a reception desk's queue
// (start_person_going).
constexpr uint16_t kComplexToReception = 470;

// The patients' complexes: DATA/P_COMP.LEX, as the game's
// person_complex_group (loaded by src/files.cpp).
struct PersonComplexes {
    std::vector<uint8_t> group;

    // The step byte at `pc`; 0 (the end of a complex) past the file's end.
    uint8_t at(uint32_t pc) const { return pc < group.size() ? group[pc] : 0; }
};

// What running patients' complexes reads and changes: the world, the
// animations, the complexes and the turn being played (the game's
// actual_game_turn).
struct PersonRun {
    World &world;
    const AnimData &anims;
    const PersonComplexes &complexes;
    uint32_t game_turn;
};

// Loads DATA/P_COMP.LEX from the game data folder `dir`. Fails, saying why
// in `error`, if it cannot be read or names a step kPersonComplex lacks.
bool load_person_complexes(const std::filesystem::path &dir, PersonComplexes &out, std::string *error = nullptr);

// set_complex (0x417ac): gives patient `id` complex `action` with
// destination cell `arg` (column + row * 128; 0 for none). With no complex
// running it starts at once, its destination set (set_destination_coords),
// or, if a step of action 24 left one to resume, that resumes with `action`
// queued behind it; returns true. Otherwise `action` is queued (in the second
// slot if the first is taken, replacing what it held) and returns false.
// The caller then runs its first step (process_person_complex) if wanted.
bool set_complex(World &world, uint16_t id, uint16_t action, uint16_t arg);

// process_person_complex (0x40524): runs the step of patient `id`'s complex
// at complex_pc and moves on, starting the queued complex or entering
// next_state at its end; chained steps run at once. Returns 1 after a step,
// 2 if no complex is running, 0 or 3 if the step failed
// (fail_person_complex). Draws from world.seed as the game does. `game_turn` is the turn being played
// (the game's actual_game_turn).
//
// complex_success (action 52) is ported for patients: a drink, a paid
// amusement, the toilet and a use of their object (2, 4, 6, 24). Doors
// (actions 14, 20 and 22, find_door in core/world/rooms.h) take the patient
// into or out of a room (move_person_in_room_list).
uint8_t process_person_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                               uint32_t game_turn);

// complex_success (0x423c8): the effect of patient `id`'s finished step of
// action 52 of `kind`: 2 a drink (a can counted for the drinks trophy, for
// hospital 0's income), 4 a paid amusement, 6 the toilet, 24 a use of their
// object. The staff's kinds do nothing here.
void complex_success(World &world, uint16_t id, uint16_t kind);

// find_nearest_free_object (0x37d28): in the seeker's room, a free object
// of `kind` of the room drawn from world.seed among the first 30 (a 31st
// found is taken at once); outside rooms, the nearest free object of `kind`
// of the seeker's hospital within 7 cells on each axis whose use cell a path
// reaches, as can_i_get_to with the object as the mover (a path from its own
// use cell 1). The use cell is the seeker's: 1 for patients, 0 for staff, 2
// for handymen. 0 if none.
uint16_t find_nearest_free_object(World &world, uint16_t id, uint8_t kind);

// move_person_in_room_list (0x167c4): person `id` goes into the list of
// the room holding `tile` (insert_person_in_room_list), or out of their
// room's (remove_person_from_room_list) for a cell in none.
void move_person_in_room_list(World &world, uint16_t id, uint16_t tile);

// insert_person_in_room_list (0x16808): person `id` at the head of room
// `room`'s list of people (RoomInfo.first_person), in that room
// (Thing.room); a patient counts in its patients_seen. They must be in no
// list.
void insert_person_in_room_list(World &world, uint16_t room, uint16_t id);

// remove_person_from_room_list (0x168a0): person `id` out of their room's
// list of people, and in no room.
void remove_person_from_room_list(World &world, uint16_t id);

// adjust_big_cell_flag (0xa8f78): sets MapCell.flags 0x20 on `cell` if a
// person on it uses its object or an object on it covers more than one
// cell, else clears it.
void adjust_big_cell_flag(World &world, uint16_t cell);

// fail_person_complex (0x41d54): ends patient `id`'s failed complex: back
// to resume_state standing, the complex and its queue cleared; with no
// resume_state, PS_COMPLEX_RETRY to retry the step.
void fail_person_complex(World &world, const AnimData &anims, uint16_t id);

// find_nearest_manned_reception (0x382d8): of the reception desks of
// patient `id`'s hospital with a receptionist in a reception (in reception
// `room`, or any with 0) and, if `need_space`, room in its queue, the one
// scoring least by its queue's length (gbv.q_points), its distance
// (gbv.dist_points) less its free places. 0 if none.
uint16_t find_nearest_manned_reception(const World &world, uint16_t id, uint8_t room, bool need_space);

// start_person_going (0x173c0): sends patient `id` on its way, out of its
// room first (complex 90): to the nearest manned reception desk with room
// in its queue (complex kComplexToReception to the desk's use cell 1,
// walking as PS_GOTO_QUEUE, with the desk's reception as dest_room) or,
// there being none, to a random one of its hospital's entrances
// (World::entrances) and off the map (kComplexLeave), and runs the
// complex's first step. Draws from world.seed when it leaves.
void start_person_going(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn);

// person_go_home (0x1665c): patient `id` stops what it is doing
// (interrupt_person), leaves its queue and walks to its hospital's first
// entrance (get_room_info's first kRoomEntrance room of its owner; for a
// rival, which has none, cell 0) and off the map, counted in the statistics
// (record_person_as_patient).
// Its player calls and status icon are not ported.
void person_go_home(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                    uint32_t game_turn);

// person_ok_to_call (0x1905c): false if patient `id` is in the toilets or
// on its way there (next_state PS_IN_BOGS).
bool person_ok_to_call(const World &world, uint16_t id);

// interrupt_complex (0x427d8) for patient `id`: clears its complex, the
// complexes queued and to resume and its next and resume states, centres
// it on its cell and, if it uses an object, gets it off (complex 160) at a
// new patient's speed.
void interrupt_complex(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                       uint32_t game_turn);

// remove_person (0x16760): takes patient `id` off the map and frees its
// entry: leaves its queue, abandons its complex (abandon_complex) and its
// room (remove_person_from_room_list), then unlinks it from its cell
// (remove_thing). Vaccinations and the patient's window, which do not
// exist yet, are not handled.
void remove_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                   uint32_t game_turn);

// person_nav_failure (0x1752c): person `id`, failing to find its way:
// heading from the corridor for a room built in the last 500 turns, it is
// sent on again (start_person_going); else back to queueing if it was to
// queue, or PS_STUCK. The game also announces a stuck patient of the
// player's, which is not ported.
void person_nav_failure(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                        uint32_t game_turn);

// One game turn of patient `id`, as process_things for a THING_PATIENT: its
// state's routine. Returns whether it still exists (PS_GONE removes it).
// Draws from world.seed as the game does, and reads World::settings and
// the rest set by start_queues (core/world/queue.h); before that, the
// game's static settings (Balance{}) at the starting heating level.
// Handled:
// - walking (PS_GOTO_DESTINATION, PS_GOTO_OBJECT, PS_GOTO_QUEUE), each step
//   decided after the patient's needs and reactions (person_needs_turn,
//   core/world/litter.h); a failed walk goes to person_nav_failure, which
//   leads back to queueing or to PS_STUCK, from which a patient tries random
//   directions and, past gbv.leave_max impatience, goes home fed up;
// - queueing (PS_IN_QUEUE, then PS_ENTERTAINS_THEMSELF): a thirsty
//   patient fetches a drink from a drinks machine near by, others sit on a
//   free bench near by or stand on a free cell, and one past gbv.leave_max
//   impatience goes home fed up (PS_ARRIVES_AT_DOOR queues, as patients
//   never enter a room yet; PS_RETURN_TO_QUEUE_AREA walks back to the
//   queue);
// - being sent on (PS_GO_FOR_TREATMENT, after the receptionist): the
//   patient pays or not (shall_peep_pay, core/world/reception.h) and goes
//   home without a treatment the hospital can give; with no room for it,
//   waits for a decision (PS_WAITS_FOR_DECISION) up to 1500 turns, then
//   goes home; else walks to the queue of the room for it (get_expertise_dest),
//   for a ward's diagnosis to a ward with a free bed (send_person_to_bed);
// - rooms: at a room's door it enters if called, else queues
//   (PS_ARRIVES_AT_DOOR); waiting in a room's queue it pages the staff its
//   treatment needs (page_staff_i_need, core/world/rooms.h); called in, it
//   waits for the member of staff who called it (PS_ARRIVED_FROM_CALL), or
//   leaves if there is none;
// - the toilets: waiting (PS_ENTERTAINS_THEMSELF) with a full bladder
//   (past gbv.bowel_full), a patient leaves its queue for the nearest
//   toilets in use (get_nearest_room_info), uses a free toilet there and
//   then, with an illness from 16 on, a free sink (person_in_bogs), and
//   walks out to go for treatment again (leave_toilets); with no toilet
//   free it walks straight out;
// - lying in a ward's bed (PS_IN_BED) for its diagnosis, then on to its
//   next treatment (setup_next_treatment);
// - stepping off a blueprint (PS_BP_GOTO) before walking or waiting on it;
// - in a treatment machine, filling it (PS_FILLS) or played to its end
//   (PS_ENERGISES);
// - dead (PS_DEAD, PS_FLYS, PS_AT_HELL_GATE; core/world/reaper.h);
// - PS_WAITS_FOR_BUSY, PS_GO_HOME, PS_GONE, PS_PLAY_COMPLEX_ANIM,
//   PS_COMPLEX_RETRY, PS_SHOWANIM, PS_SLAVE_READY and PS_MERGED.
// Not ported, as what they need does not exist yet: research, statistics
// (person_count) and status icons.
bool person_turn(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t id,
                 uint32_t game_turn);

// One game turn of object `id` if it is an entrance door in one of the
// sliding states (OS_SLIDING_DOOR_*), as process_things: closed, it starts
// opening when a patient or member of staff stands on either cell in front
// of it; it plays its opening animation, stays open until 50 turns after the
// last one has gone, then closes, reopening from where it is if someone
// comes. The half of a double door swinging (OS_DOUBLE_DOOR_SWINGS) plays
// its swing (double_door_swings). Does nothing for other objects or states.
void sliding_door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// One game turn of room door `id` (kThingDoor), as process_things: swinging
// (OS_DOUBLE_DOOR_SWINGS), it plays the swing and closes, free again;
// otherwise its animation advances every anim_speed turns.
void door_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

// peep_bp_goto (0x781f8), PS_BP_GOTO's and SS_BP_GOTO's turn: person `id`
// walks off the blueprint, a row or column at a time (which first drawn
// from world.seed when both are left), to the cell move_peep_from_blueprint
// chose, then goes back to its saved state.
void peep_bp_goto_turn(World &world, const AnimData &anims, uint16_t id, uint32_t game_turn);

} // namespace th
