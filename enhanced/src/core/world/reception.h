#pragma once

#include "core/data/anims.h"
#include "core/world/complex.h"
#include "core/world/queue.h"
#include "core/world/world.h"

#include <cstdint>

namespace th
{

// The patients' side of being served at a reception desk: what the
// receptionist's states (staff_turn, core/world/staff.h; the game's
// recept.cpp) ask of the queue and of the patient they call, as the game's
// queue.cpp, peeps.cpp, doctor.cpp, sim.cpp and finance.cpp do it.
//
// A receptionist calls only patients whose treatment has queue space to
// spare (all_queue_capacity, core/world/queue.h): while the hospital cannot
// give general practice (no GP's office yet), she calls no one, and her
// queue waits until it is fed up, as in the game.

// Patient states (the game's PS_*) the receptionist looks for.
constexpr uint8_t kPsArrivedFromCall = 0x46;

// RoomInfo.queue_head: the first patient queueing for room `room` (a
// World::rooms index), 0 for none.
uint16_t reception_queue_head(const World &world, uint16_t room);

// Thing.queue_next: the patient queueing behind `patient`, 0 for none.
uint16_t reception_queue_next(const World &world, uint16_t patient);

// Whether queueing patient `patient` is an emergency (kPfEmergency), called
// before those queueing ahead of them.
bool reception_emergency(const World &world, uint16_t patient);

// call_person_to_room (0x16ba8): calls queued patient `id` (or, with
// `follow`, the first callable one queued from `id` on) to `caller` with
// complex `action` at cell `arg`: takes them out of the queue, sets the
// complex going with next_state PS_ARRIVED_FROM_CALL, makes them the
// caller's slave and sets the caller's timer to 200 and call_retries to 3-6
// (a draw from world.seed). Dead patients met on the way leave the queue.
// Returns the patient called, or 0.
uint16_t call_person_to_room(const PersonRun &run, uint16_t caller, uint16_t id, uint16_t action, uint16_t arg,
                             bool follow);

// call_person_again (0x16cdc): calls `caller`'s slave again, as
// call_person_to_room. Returns 0 if the call is void (the slave is gone,
// dead or busy elsewhere), 1 when the caller's call_retries have run out, 2
// to keep waiting (the caller's timer reset to 200).
uint8_t call_person_again(const PersonRun &run, uint16_t caller, uint16_t action, uint16_t arg);

// forget_about_my_slave (0x3c1e8): whether `caller` should give up on its
// slave (dead, not a patient, queueing, or doing something other than
// answering the call); one who is idle instead is called again (complex
// `action` at the caller's desk's use cell 1) and it returns false.
bool forget_about_my_slave(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t caller,
                           uint16_t action, uint32_t game_turn);

// interrupt_person (0x194f0): interrupts patient `person`'s complex
// (interrupt_complex) and puts it back to walking back to its queue
// (PS_RETURN_TO_QUEUE_AREA), on its way to a queue (PS_GOTO_QUEUE), to
// waiting for a decision if it was, or else out of any queue to go for its
// treatment (PS_GO_FOR_TREATMENT). In a room, the member of staff dealing
// with it is freed (interrupt_staff) and it walks out first.
void interrupt_person(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t person,
                      uint32_t game_turn);

// Whether the hospital of patient `patient` can give the treatment they
// need (allhosps[owner].rooms[treatment].cure_available == 1; World::
// cure_available, hospital 0's): the receptionist then deals with them by
// complex 230 (interaction 6) rather than 240 (interaction 7).
bool reception_cure_available(const World &world, uint16_t patient);

// update_single_cure_cost (0x143e0, core/world/arrivals.h) for hospital 0
// (World::market).
void update_single_cure_cost(World &world, uint16_t treatment);

// shall_peep_pay (0x58c54): whether patient `id` will pay hospital 0's price
// for its treatment, and for its next one if any: its price
// (update_single_cure_cost first moves it to its share of the most patients
// will pay, max_price_for_treatment), against that most, raised by its
// insurance company's extra (World::ins_control) or 0 if uninsured and
// short of cash, plus a draw from world.seed of up to half the price. One
// who will not is marked kPfTooExpensive, and World::too_expensive and
// unpaid_treatment are set for the advisor, and it is counted in the
// statistics (record_person_as_patient), with the too-expensive status icon.
bool shall_peep_pay(World &world, const AnimData &anims, uint16_t id);

// The patient's part of recept_admits_patient (0x984a0), once `receptionist`
// has dealt with them: patient `patient`'s impatience and waiting time
// cleared and, if it will pay (shall_peep_pay), its fee quoted at the
// treatment's price (pay_for_treatment kind 3, which takes no money: it is
// paid at the treatment), else its treatment cleared to 0; then
// PS_GO_FOR_TREATMENT.
void reception_serves_patient(World &world, const AnimData &anims, uint16_t receptionist, uint16_t patient);

} // namespace th
