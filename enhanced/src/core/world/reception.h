#pragma once

#include "core/data/anims.h"
#include "core/world/complex.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

// The patients' side of being served at a reception desk: what the
// receptionist's states (staff_turn, core/world/staff.h; the game's
// recept.cpp) ask of the queue and of the patient they call, as the game's
// queue.cpp, peeps.cpp, doctor.cpp, sim.cpp and finance.cpp do it.
//
// Queues are not ported yet: these are stubs that act as if every queue
// were empty and no patient could be called. Nothing here draws from
// world.seed until it is ported; call_person_to_room does in the game.

// Patient states (the game's PS_*) the receptionist looks for.
constexpr uint8_t kPsArrivedFromCall = 0x46;

// RoomInfo.queue_head: the first patient queueing for room `room` (a
// World::rooms index), 0 for none.
uint16_t reception_queue_head(const World &world, uint16_t room);

// Thing.queue_next: the patient queueing behind `patient`, 0 for none.
uint16_t reception_queue_next(const World &world, uint16_t patient);

// Whether queueing patient `patient` is an emergency (Thing.flags 0x40),
// called before those queueing ahead of them.
bool reception_emergency(const World &world, uint16_t patient);

// all_queue_capacity (0x4ba8c): the free places in the queues of all of
// hospital `owner`'s rooms for illness `type` (counting those waiting if
// `waiting`), 0 if the hospital cannot treat it.
int16_t all_queue_capacity(const World &world, uint16_t type, bool waiting, uint8_t owner);

// call_person_to_room (0x16ba8): calls queued patient `id` (or, with
// `follow`, the first callable one queued from `id` on) to `caller` with
// complex `action` at cell `arg`: takes them out of the queue, sets the
// complex going with next_state PS_ARRIVED_FROM_CALL, makes them the
// caller's slave and sets the caller's timer to 200 and call_retries to 3-6
// (a draw from world.seed). Dead patients met on the way leave the queue.
// Returns the patient called, or 0.
uint16_t call_person_to_room(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t caller,
                             uint16_t id, uint16_t action, uint16_t arg, bool follow, uint32_t game_turn);

// call_person_again (0x16cdc): calls `caller`'s slave again, as
// call_person_to_room. Returns 0 if the call is void (the slave is gone,
// dead or busy elsewhere), 1 when the caller's call_retries have run out, 2
// to keep waiting (the caller's timer reset to 200).
uint8_t call_person_again(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t caller,
                          uint16_t action, uint16_t arg, uint32_t game_turn);

// forget_about_my_slave (0x3c1e8): whether `caller` should give up on its
// slave (dead, not a patient, queueing, or doing something other than
// answering the call); one who is idle instead is called again (complex
// `action` at the caller's desk's use cell 1) and it returns false.
bool forget_about_my_slave(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t caller,
                           uint16_t action, uint32_t game_turn);

// interrupt_person (0x194f0): interrupts patient `person`'s complex and
// frees the member of staff dealing with them, then walks them out of
// their room to their queue or destination, or back to waiting for a
// decision.
void interrupt_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t person,
                      uint32_t game_turn);

// Whether the hospital of patient `patient` has a cure for the treatment
// they need (allhosps[owner].rooms[treatment].cure_available == 1): the
// receptionist then deals with them by complex 230 (interaction 6) rather
// than 240 (interaction 7).
bool reception_cure_available(const World &world, uint16_t patient);

// The patient's part of recept_admits_patient (0x984a0), once `receptionist`
// has dealt with them: patient `patient`'s impatience and waiting time
// cleared, their treatment paid for (shall_peep_pay, pay_for_treatment kind
// 3) or, if they will not pay, their treatment cleared to 0, and their state
// PS_GO_FOR_TREATMENT.
void reception_serves_patient(World &world, uint16_t receptionist, uint16_t patient);

} // namespace th
