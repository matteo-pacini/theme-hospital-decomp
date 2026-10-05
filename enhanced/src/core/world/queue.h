#pragma once

#include "core/data/anims.h"
#include "core/data/balance.h"
#include "core/world/arrivals.h"
#include "core/world/complex.h"
#include "core/world/finance.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

// The rooms' queues of patients, as the game's queue.cpp: a list per room
// (RoomInfo.queue_head, Thing.queue_prev and queue_next), with its length
// and the patients on their way to it (queue_incoming).
//
// Rooms are World::rooms indices; 0 is no room.

// RoomInfo.type of a GP's office (the game's ROOM_GP).
constexpr uint8_t kRoomGp = 7;

// Illnesses (Thing.illness and treatment; the game's ILL_*).
constexpr uint8_t kIllGeneralPractice = 1;
constexpr uint8_t kIllTheSquits = 29;
constexpr uint8_t kIllDiagWard = 42;
constexpr uint8_t kIllAutopsy = 44;

// Thing.person_flags bits (the game's PF_*, include/enums.h) besides
// core/world/litter.h's.
constexpr uint32_t kPfAwaitingDecision = 0x10;
constexpr uint32_t kPfEmergency = 0x40;
constexpr uint32_t kPfWaitingForRoom = 0x80;
constexpr uint32_t kPfSentHomeByInspector = 0x100;
constexpr uint32_t kPfNeedsDiagnosisRoom = 0x400;
constexpr uint32_t kPfFedUp = 0x4000;
constexpr uint32_t kPfNoTreatment = 0x100000;
constexpr uint32_t kPfTooExpensive = 0x800000;

// expertise[].category (src/array.cpp; not a balance key): per illness, the
// room type (RoomInfo.type) it is treated in.
inline constexpr std::array<uint8_t, 47> kExpertiseCategory = {
    0,  7,  17, 23, 8,  11, 30, 20, 18, 21, 19, 11, 24, 11, 10, 11, 11, 11, 10, 10, 10, 10, 8, 8,
    11, 11, 8,  8,  11, 11, 10, 8,  11, 11, 10, 10, 13, 15, 12, 16, 14, 27, 9,  8,  28, 0,  0,
};

// Sets up the queues' part of `world` for a level, after start_level: no
// queues, the turn's distances 0, hospital 0 able to give no treatment but
// "none" (init_player sets rooms[0].cure_available), the insurance
// companies at the game's static table, the advisor's counts cleared; and
// points it at what the patients' states read (World::settings and on),
// which must outlive the turns run.
void start_queues(World &world, const Balance &settings, HospitalMarket &market, const int &reputation,
                  const Finance &finance);

// The three values start_turn (src/game.cpp) draws from world.seed at the
// start of every turn, in order: othervar.rand_pct (which every reader
// draws anew, so it is dropped), World::queue_near_dist and
// World::object_near_dist.
void draw_turn_randoms(World &world);

// join_queue (0x4af50): appends patient `id` to `room`'s queue, leaving the
// one it was in; counts it in queue_length and no longer in queue_incoming.
void join_queue(World &world, uint16_t id, uint16_t room);

// leave_queue (0x4b058): takes patient `id` out of its queue, or, in none,
// off its way to dest_room (no longer counted in that room's
// queue_incoming, but for a reception), clearing dest_room.
void leave_queue(World &world, uint16_t id);

// queue_size (0x4b21c): `room`'s queue_length.
uint16_t queue_size(const World &world, uint16_t room);

// which_queue (0x4b244): the room whose queue patient `id` is in, found from
// the queue's head, or 0.
uint16_t which_queue(const World &world, uint16_t id);

// get_real_person_from_room (0x4b71c): the patient at position `pos`
// (counted from 1) of `room`'s queue, or 0.
uint16_t get_real_person_from_room(const World &world, uint16_t room, uint16_t pos);

// front_queue (0x4ace0) and back_queue (0x4ae60), the queue window's
// commands: move the patient at position `pos` (from 1) of `room`'s queue
// to its front or back.
void front_queue(World &world, uint16_t room, uint16_t pos);
void back_queue(World &world, uint16_t room, uint16_t pos);

// spare_queue_capacity (0x4b988): the free places of `room`'s queue
// (queue_capacity less its length, and less those on their way if
// `waiting`), or with `room` 0 those of the first room of `owner`'s
// for illness `type` that has any; 0 if none.
int16_t spare_queue_capacity(const World &world, uint16_t type, bool waiting, uint16_t room, uint8_t owner);

// all_queue_capacity (0x4ba8c): the free places, as spare_queue_capacity,
// of all of hospital `owner`'s rooms for illness `type`, 0 if it cannot give
// that treatment (World::cure_available, hospital 0's only: other
// hospitals' have none).
int16_t all_queue_capacity(const World &world, uint16_t type, bool waiting, uint8_t owner);

// queue_person_to_new_queue (0x4b7e0): unless patient `id` is in or on its
// way to the toilets (person_ok_to_call), takes it out of its queue and
// what it is doing (interrupt_person) and sends it walking (PS_GOTO_QUEUE)
// to `room`'s queue: to the nearest manned desk of a reception, or the
// room's door. As in the game, the desk's thing is kept in 8 bits, so a desk
// past thing 255 is the wrong one, and with none no destination is set.
// Staff are not paged (page_staff_i_need is not ported).
void queue_person_to_new_queue(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                               uint16_t id, uint32_t game_turn);

// queue_person_go_home (0x4b654) and queue_person_go_reception (0x4b6b0),
// the queue window's commands: the patient at position `pos` of `room`'s
// queue goes home (person_go_home), or out of the queue and back to a
// reception (start_person_going).
void queue_person_go_home(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                          uint16_t pos, uint32_t game_turn);
void queue_person_go_reception(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                               uint16_t pos, uint32_t game_turn);

// restart_all_queue_peeps (0x4bb90): empties `room`'s queue, sending each
// patient in it on again (start_person_going).
void restart_all_queue_peeps(World &world, const AnimData &anims, const PersonComplexes &complexes, uint16_t room,
                             uint32_t game_turn);

} // namespace th
