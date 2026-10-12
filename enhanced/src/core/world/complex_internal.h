// What the patients' complex code shares across complex*.cpp: the states and
// object kinds it tests, the run's context and the steps each file exports
// to the others. Private to core/world/complex*.cpp.

#pragma once

#include "core/world/complex.h"

#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{
inline namespace complex_detail
{

// Object states (the game's OS_*, include/enums.h).
constexpr uint8_t kOsStatic = 0x02;
constexpr uint8_t kOsInUse = 0x04;
constexpr uint8_t kOsDoubleDoorSwings = 0x10;
constexpr uint8_t kOsRemoved = 0x12;
constexpr uint8_t kOsSlidingDoorClosed = 0x14;
constexpr uint8_t kOsSlidingDoorOpens = 0x16;
constexpr uint8_t kOsSlidingDoorOpen = 0x18;
constexpr uint8_t kOsSlidingDoorCloses = 0x1a;

constexpr uint8_t kEntranceLeftDoor = 58;
// MapCell.flags: something on the cell covers more than one cell.
constexpr uint8_t kCellBig = 0x20;

// The complex abandon_complex runs to get a person off what it uses.
constexpr uint16_t kComplexAbandon = 0xf0;

// The animation columns of kPatientAnimStarts with a name in the game.
constexpr int kAnimWalk = 0;
constexpr int kAnimStand = 9;

// Object kinds (the game's OBJ_*).
constexpr uint8_t kObjBench = 4;
constexpr uint8_t kObjDrinksMachine = 7;
constexpr uint8_t kObjReceptionDesk = 11;

// RoomInfo.type of the toilets (the game's ROOM_TOILETS).
constexpr uint8_t kRoomToilets = 29;

// The complexes that get a patient off its object (interrupt_complex) and
// back to its queue (return_to_queue_area).
constexpr uint16_t kComplexGetOff = 160;
constexpr uint16_t kComplexReturnToQueue = 30;
// The complexes of rooms: at a single door, turned away from it to queue;
// out of the room by its door; getting off a door (interrupt_complex).
constexpr uint16_t kComplexAtDoor = 50;
constexpr uint16_t kComplexLeaveRoom = 90;
constexpr uint16_t kComplexOffDoor = 0x26c;

// Object kinds and the animation columns of kPatientAnimStarts doors use:
// through a single door (OBJ_DOOR) or a double one.
constexpr uint8_t kObjDoor = 3;
constexpr uint8_t kObjSwingDoor2 = 53;
constexpr int kAnimSingleDoor = 7;
constexpr int kAnimDoubleDoor = 67;
// The starts of a double door's swing as it opens (by the facing of who
// opens it) and as it closes, and its closed frame (OBJ_SWING_DOOR2 and the
// other half).
constexpr uint16_t kAnimSwingOpen = 0x3f8;
constexpr uint16_t kAnimSwingClose = 0x400;
constexpr uint16_t kAnimSwingDoor2Closed = 0x3eb;
constexpr uint16_t kAnimSwingDoorClosed = 0x3e6;
// MapCell.flags a door in use sets on its cell.
constexpr uint8_t kCellDoorInUse = 0x10;

struct Run {
    World &world;
    const AnimData &anims;
    const PersonComplexes &complexes;
    uint32_t game_turn;
};

uint16_t cell_of(const Thing &t);

// The settings the patients' states read: World::settings, or the game's
// static ones before start_queues.
const Balance &settings_of(const World &world);

int heating_level_of(const World &world);

// can_i_get_to (0x19918): whether a path leads from (x, y) to `tile`.
bool path_exists(const World &world, int x, int y, uint16_t tile);

// listhead[owner].rooms[type]: hospital `owner`'s room of `type` once its
// doors are committed (World::room_of_type, hospital 0's), 0 if none.
uint16_t first_room(const World &world, uint8_t owner, uint8_t type);

// Whether the patient stands on a blueprint's floor and was sent off it
// (move_peep_from_blueprint), as the walking and waiting states check
// first.
bool off_blueprint(World &world, uint16_t id);

void play_patient_anim(const AnimData &anims, Thing &p, int column);

uint8_t process(const Run &r, uint16_t id);
bool turn(const Run &r, uint16_t id);
void start_going(const Run &r, uint16_t id);

// person_nav_failure (0x1752c): heading from the corridor for a room built
// in the last 500 turns, sent on again (start_person_going); else back to
// queueing if it was to queue, or PS_STUCK, announced to the advisor
// (World::announce_stuck) for a patient of hospital 0's.
void person_nav_failure(const Run &r, uint16_t id);

// The game's PLAY_FRAME of patient_stand after a failed or abandoned
// complex.
void stand(const Run &r, Thing &p);

void fail(const Run &r, uint16_t id);

void remove(const Run &r, uint16_t id);

void go_home(const Run &r, uint16_t id);

// abandon_complex (0x41f84), for a patient: runs kComplexAbandon (queued
// behind a running complex) for up to 12 steps, each from its destination
// cell, puts it back where it was unless it uses its object or changed
// room, and stands it in resume_state.
void abandon_complex(const Run &r, uint16_t id);

// Advances a person's animation every anim_speed turns, as the waiting
// states do; returns whether it reached the end of the animation.
bool animate(const Run &r, Thing &p);

// Moves a walking person speed / 256 cell on in their facing. The game
// indexes around_xy with a stuck person's facing 4 too; here such a person
// does not move.
void step_on(World &world, uint16_t id);

// person_goto_destination (0x763d0).
void goto_destination(const Run &r, uint16_t id);

// person_goto_object (0x76768): walks to the object of its complex; near it
// (within World::object_near_dist) an object in use or gone counts as
// reached, and the complex's next step (using it) fails. With no route, the
// step runs with the object taken as busy.
void goto_object(const Run &r, uint16_t id);

// person_goto_queue (0x76c44): walks to the queue of dest_room; within
// World::queue_near_dist of the destination (of a reception, or of a room
// with a queue already) it joins the queue: PS_IN_QUEUE at a reception,
// PS_ARRIVES_AT_DOOR at a room, paging the staff its treatment needs, first
// walking on to the room's door if that is further. Sent on again
// (start_person_going) if the room is not one for its treatment or has no
// door.
void goto_queue(const Run &r, uint16_t id);

// find_standing_place (0x164f4): a patient sharing its cell with someone
// ahead of it in the cell's list, or standing on an object's cells, walks
// (complex kComplexStandingPlace, then back to its state) to the first free,
// reachable indoor cell of kStandingPlace from a random one of the first 12
// on. Returns whether it was sent. Cells off the map, which the game reads
// past its map, are skipped.
bool find_standing_place(const Run &r, uint16_t id);

// person_in_queue (0x15d48) and, with `entertaining`, person_entertains_themself
// (0x160a8): at the end of each animation, once the countdown runs out, off
// a blueprint it stands on, then the patient's needs (modify_person); then,
// alive, it pages the staff its treatment needs to the room it queues for,
// goes home fed up past gbv.leave_max impatience (entertaining only),
// fetches a drink when thirsty, goes to the nearest toilets in use with a
// bladder past gbv.bowel_full (entertaining only; out of its queue, then
// PS_IN_BOGS), sits on a free bench near by (the bench shortage counted
// down 5, or up 1 with none), or stands, moving off a crowded cell. A
// patient in the queue moves on to PS_ENTERTAINS_THEMSELF.
void wait_in_queue(const Run &r, uint16_t id, bool entertaining);

// person_in_bogs (0x17854), PS_IN_BOGS: in the toilets, the patient goes
// to a free toilet (substate 0, complex 330; PS_LEAVE_TOILETS if it fails),
// then (substate 1, its impatience gone) with an illness from 16 on (the
// non-visual ones) to a free sink (complex 350); without one free, or
// after, PS_LEAVE_TOILETS.
void person_in_bogs(const Run &r, uint16_t id);

// leave_toilets (0x179a8), PS_LEAVE_TOILETS: out of the toilets by their
// door to go for treatment again.
void leave_toilets(const Run &r, uint16_t id);

// return_to_queue_area (0x179f8): back to the queue it is in (to its
// hospital's first reception desk, or the room's door), entertaining itself
// there.
void return_to_queue_area(const Run &r, uint16_t id);

// person_go_for_treatment (0x3d91c): the patient pays or not
// (shall_peep_pay); without the treatment (or paying for it) it goes home
// through the first entrance; with no room for it, it waits for a decision;
// else it walks to the queue of the room for it (get_expertise_dest),
// counted on its way there, paging the staff the treatment needs. A
// treatment the hospital cannot give goes to reputation_affected
// (kRepGoForTreatment: no points, but its price moves). A ward's diagnosis
// goes to bed (send_person_to_bed); an autopsy to the research room
// (send_person_for_research).
void go_for_treatment(const Run &r, uint16_t id);

// person_waits_for_decision (0x17e30): a patient waiting for a diagnosis
// room goes once the hospital has one for it (find_possible_diagnosis,
// which draws from world.seed), one waiting for a room for its treatment
// once the hospital can give it and has its room; 1500 turns after it began
// waiting, it goes home, turned away. Its player call is answered then. The player's answers (EV_CHOICE_RESULT) are
// apply_call_event's. Meanwhile it stands, moving off a blueprint or a crowded cell.
void waits_for_decision(const Run &r, uint16_t id);

} // namespace complex_detail

} // namespace th
