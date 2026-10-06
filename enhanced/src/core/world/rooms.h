#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

struct Date;
struct Finance;
struct HospitalSettings;
struct PersonComplexes;
struct StaffComplexes;

// Rooms in use: the treatments a hospital can give (src/skills.cpp), the
// pages calling staff to rooms, doors, the player calls, and diagnosis and
// treatment (src/treat.cpp, pay_for_treatment in src/finance.cpp). What the
// staff and patients do in a room is in core/world/staff.h and
// core/world/complex.h.

// The game's expertise[] (Expertise in include/hosp.h; src/array.cpp), by
// illness, without the fields balance files set (Balance::expertise) and
// those only research reads.
struct ExpertiseInfo {
    // The skills (Thing.skills bits) of the staff it needs.
    uint16_t staff_mask;
    // The object kind it uses (OBJ_*), 0 for none.
    uint8_t kind;
    // The possible_diagnoses entry (src/tables.cpp) listing the diagnosis
    // rooms a doctor may send for.
    uint8_t diag_set;
    // Bit 0: a ward first (ILL_DIAG_WARD).
    uint8_t needs_ward;
    // What setup_next_treatment does after it: 0 nothing more, 1 back to
    // the GP, 2 cure, 4 diagnose, 5 the next treatment; 3 nothing.
    uint8_t outcome;
    // The room type it is given in.
    uint8_t category;
};

inline constexpr std::array<ExpertiseInfo, 47> kExpertiseData = {{
    {0, 0, 0, 0, 0, 0},     {16, 6, 1, 0, 4, 7},   {16, 9, 2, 0, 2, 17},  {16, 46, 3, 0, 2, 23}, // 0
    {64, 18, 4, 0, 2, 8},   {4, 39, 5, 0, 2, 11},  {16, 54, 6, 0, 2, 30}, {16, 26, 7, 0, 2, 20}, // 4
    {16, 23, 0, 0, 2, 18},  {4, 24, 8, 0, 2, 21},  {16, 25, 9, 0, 2, 19}, {4, 39, 1, 0, 2, 11},  // 8
    {16, 47, 6, 0, 2, 24},  {4, 39, 1, 0, 2, 11},  {32, 30, 1, 3, 2, 10}, {4, 39, 10, 0, 2, 11}, // 12
    {4, 39, 1, 0, 2, 11},   {4, 39, 1, 0, 2, 11},  {32, 30, 1, 3, 2, 10}, {32, 30, 1, 3, 2, 10}, // 16
    {32, 30, 1, 3, 2, 10},  {32, 30, 1, 3, 2, 10}, {64, 18, 1, 0, 2, 8},  {64, 18, 1, 0, 2, 8},  // 20
    {4, 39, 1, 0, 2, 11},   {4, 39, 1, 0, 2, 11},  {64, 18, 1, 0, 2, 8},  {64, 18, 1, 0, 2, 8},  // 24
    {4, 39, 1, 0, 2, 11},   {4, 39, 1, 0, 2, 11},  {32, 30, 1, 3, 2, 10}, {64, 18, 1, 0, 2, 8},  // 28
    {4, 39, 1, 0, 2, 11},   {4, 39, 1, 0, 2, 11},  {32, 30, 1, 3, 2, 10}, {32, 30, 1, 3, 2, 10}, // 32
    {16, 14, 0, 0, 1, 13},  {16, 42, 0, 0, 1, 15}, {16, 13, 0, 0, 1, 12}, {16, 27, 0, 0, 1, 16}, // 36
    {16, 22, 0, 0, 1, 14},  {16, 20, 0, 0, 1, 27}, {4, 8, 0, 0, 5, 9},    {64, 18, 0, 0, 1, 8},  // 40
    {256, 55, 0, 0, 2, 28}, {0, 41, 0, 0, 0, 0},   {0, 40, 0, 0, 0, 0},                          // 44
}};

// The game's rooms[].staff_state_row and doctor_skills (src/tables.cpp), by
// room type: the kStaffStates row for staff entering the room, and the
// skills a doctor needs to work in it (0: any doctor).
inline constexpr std::array<uint8_t, 31> kRoomStaffStateRow = {0, 0, 0, 0, 0, 0, 10, 1, 2, 4, 8, 7, 2, 2, 2, 2,
                                                               2, 2, 2, 2, 2, 7, 9,  2, 2, 5, 6, 2, 3, 6, 2};
inline constexpr std::array<uint16_t, 31> kRoomDoctorSkills = {
    0, 0, 0, 0, 0, 0, 0, 16, 64, 4, 32, 4, 16, 16, 16, 16, 16, 16, 16, 16, 16, 4, 0, 16, 16, 0, 0, 16, 256, 0, 16};

// Sets up the rooms in use for a level, after start_queues: no pages (to
// staff or to repair machines), no rooms listed, no player calls; and points `world` at what treatments read
// and change (World::hospital and on), which must outlive the turns run.
void start_rooms_in_use(World &world, HospitalSettings &hospital, Finance &ledger, int &balance, const Date &date,
                        const PersonComplexes &person_complexes, const StaffComplexes &staff_complexes);

// listhead[owner].staff_by_skill[skill] (insert_skill, remove_skill): the
// hired staff of hospital `owner` with skill bit 1 << `skill`.
int staff_with_skill(const World &world, uint8_t owner, int skill);

// set_expertise_room (0x4a520), for room `room` laid out: makes available
// each treatment given in rooms of its type whose staff skills its
// hospital has (research rooms only with skills), and the ward treatments
// if general practice became so.
void set_expertise_room(World &world, uint16_t room);

// set_expertise_staff (0x4a2fc), for member of staff `id` hired: makes
// available each treatment their skills complete whose room the hospital
// has listed (World::room_of_type) and, if it needs one, its object (two
// surgeons for surgery; a ward for ward treatments).
void set_expertise_staff(World &world, uint16_t id);

// remove_expertise_staff (0x4a754), for member of staff `id` leaving while
// still their hospital's: each treatment needing a skill of theirs no one
// else has (surgery also with fewer than three surgeons, them included) is
// no longer available, nor the ward treatments if general practice went.
void remove_expertise_staff(World &world, uint16_t id);

// remove_expertise_room (0x4a964), for room `room` going: if it was its
// hospital's last of its type, its treatments are no longer available.
void remove_expertise_room(World &world, uint16_t room);

// room_deleted_update_listhead (0x4a244), before room `room` goes: lists
// another room of its type and hospital, or none.
void room_deleted_update_listhead(World &world, uint16_t room);

// page_member_of_staff (0x49820): pages staff with the skills of
// `skill_mask` (one page per skill) to room `room` for hospital `owner`,
// unless `wanted` of them are there already (with `count_present`) or
// paged; posts up to `wanted` pages. Never pages to a reception. Returns
// whether no new page was needed. The page's sound is not played.
bool page_member_of_staff(World &world, uint16_t skill_mask, uint16_t room, bool count_present, int8_t wanted,
                          uint8_t owner);

// page_staff_i_need (0x17c28): pages the staff patient `person`'s treatment
// needs to `room`, two for an operating theatre.
void page_staff_i_need(World &world, uint16_t person, uint16_t room);

// check_pager (0x49acc): the room of the nearest page member of staff `id`
// answers (their skills, not claimed after `game_turn` or theirs already, to
// an open room), which they then hold for 3000 turns; 0 if none. Pages to a
// room since freed are dropped.
uint16_t check_pager(World &world, uint16_t id, uint32_t game_turn);

// pager_arrival (0x49ce0): member of staff `id`, arrived in their room,
// clears the page to it for their skills: theirs, else one answered by
// someone else, else an unanswered one.
void pager_arrival(World &world, uint16_t id);

// clear_pager_responses (0x49e30): the pages to `room` unanswered again.
void clear_pager_responses(World &world, uint16_t room);

// staff_abandon_pager_response (0x4a1d4): member of staff `id` no longer
// answers any page.
void staff_abandon_pager_response(World &world, uint16_t id);

// make_a_playercall (0x1824c): a call of `type` about `subject` for hospital
// `owner`, expiring after `timeout` days (expire_playercalls), never if 0.
// Returns its id (World::next_call_id, from 1), or 0 if the table is full.
// The fax on the panel is not ported.
uint16_t make_a_playercall(World &world, uint16_t type, uint16_t subject, int16_t timeout, uint8_t owner);

// expire_playercalls (0x18908), once a day: the calls with a timeout age
// by a day and go once it runs out. The answers the game then gives to the
// calls of a VIP's visit, an epidemic and an emergency are not ported.
void expire_playercalls(World &world);

// get_playercall (0x18710): removes `owner`'s call of `type` with `id`, or
// the first with id 0. Returns its subject, 0 if none.
uint16_t get_playercall(World &world, uint16_t type, uint16_t id, uint8_t owner);

// check_playercall (0x18838): `owner`'s calls of `type`; for types other
// than 2, 3, 4 and 8, only those about `subject`.
uint16_t check_playercall(const World &world, uint16_t type, uint16_t subject, uint8_t owner);

// find_object (0x3791c): an object of `kind` for `tile`: in the room holding
// it, else outside rooms within 7 cells on each axis and reachable from the
// object's use cell 1 (hospital 0's only). `free_only` (flag 0x96) skips
// objects in use; `random` picks among up to 30 found, drawing from
// world.seed. 0 if none.
uint16_t find_object(World &world, uint16_t tile, uint8_t kind, bool random, bool free_only, uint8_t owner);

// is_this_a_door (0x23a64): the room whose door_tile or door_inside_tile is
// `tile`, 0 for none.
uint16_t is_this_a_door(const World &world, uint16_t tile);

// find_door (0x38788): a door (kThingDoor) on person `id`'s cell or a
// neighbour's: the first found while they are in a room, else one whose
// room's door_tile they stand on; 0 if none.
uint16_t find_door(const World &world, uint16_t id);

// calc_room_value (0x1b050): how well room `room` is furnished: 5 per cell of
// its width and height, 5 per object past the compulsory ones and per two
// windows, never below 0.
int32_t calc_room_value(const World &world, uint16_t room);

// pay_for_treatment (0x51280): with `kind` 2, patient `id` pays for its
// treatment the fee it was quoted (or the price now): its insurance
// company, if the hospital takes it, pays at the end of the month after its
// delay (HospitalSettings::insurance_claims), else the patient from its cash
// as a kTxTreat line, unless short (returns false). Otherwise the price now
// is quoted as its fee. The fee goes to the treatment's income.
bool pay_for_treatment(World &world, uint16_t id, int kind);

// done_treatment (0x3c360): member of staff `staff` has given patient
// `patient` its treatment: its diagnosis progress (affect_diagnosis, which
// draws from world.seed), its treatment recorded, and what comes next
// (setup_next_treatment), unless the room was destroyed.
void done_treatment(World &world, const AnimData &anims, uint16_t staff, uint16_t patient, uint32_t game_turn);

// affect_diagnosis (0x3c3b8): member of staff `staff` has examined patient
// `patient`: for general practice and the diagnosis rooms, its diagnosis
// progress grows to its progress less its handicap plus the staff's
// ability this time (calc_ability: a random 250 either way drawn from
// world.seed, grade, tiredness, madness and the room) plus the room's worth
// for its illness (diag_register), if that is more; a room that does it
// harm costs 5 health. The staff's ability grows by 1 (30 to 1000).
void affect_diagnosis(World &world, Thing &staff, Thing &patient);

// setup_next_treatment (0x3c4ec): patient `id` pays for the treatment
// given by member of staff `staff` (0 for none: a ward's bed) and moves on
// by its outcome (ExpertiseInfo::outcome): home, back to the GP, cured
// (cure_illness; killed if wrongly guessed), diagnosed (diagnose_patient)
// or on to its next treatment; a diagnosis left wanting is guessed at or
// given up on by the policy's guess_cure_threshold and
// send_home_threshold. Then next_treatment_now_setup, out of its room. The
// icons and sounds are not ported.
void setup_next_treatment(World &world, const AnimData &anims, uint16_t staff, uint16_t id, uint32_t game_turn);

// next_treatment_now_setup (0x3c8d8): patient `id`'s next state: waiting for
// the player's decision on a diagnosis or a missing room (a player call,
// unless more than 5 wait), else going for treatment; walked out of its
// room first (complex 90 if `in_room`, else 240).
void next_treatment_now_setup(World &world, const AnimData &anims, uint16_t id, bool in_room, uint32_t game_turn);

// The pages for handymen (the game's ThingPager, src/handyman.cpp): a page
// asks for thing `id` to be seen to at a priority; a handyman answering it
// holds it for 3000 turns.
// thingpage_request (0x7baac): pages for `id` at `priority`, raising the
// priority of its page if it has one. False if the pager is full.
bool thingpage_request(World::ThingPager &pager, uint16_t id, uint8_t priority);
// thingpage_cancel (0x7bbbc): drops the page for `id` (the last page taking
// its place). Returns whether there was one.
bool thingpage_cancel(World::ThingPager &pager, uint16_t id);
// thingpage_respond (0x7bc4c): for handyman `handyman`, the page (for a
// thing in room `room`, if not 0) of the highest priority, at least `min`,
// nearest first, that no one else holds and is not in a room being edited;
// he holds it from `game_turn`. Returns its thing, 0 if none.
uint16_t thingpage_respond(World &world, World::ThingPager &pager, uint16_t handyman, uint16_t room, uint8_t min,
                           uint32_t game_turn);
// thingpage_find_highest_priority (0x7c2f8): the highest priority paged,
// 0xff with no pages.
uint8_t thingpage_find_highest_priority(const World::ThingPager &pager);
// thingpage_remind_me (0x7c37c): the thing of a page `handyman` holds, 0 if
// none.
uint16_t thingpage_remind_me(const World::ThingPager &pager, uint16_t handyman);
// thingpage_cancel_my_response (0x7c3f8): `handyman` holds no page.
void thingpage_cancel_my_response(World::ThingPager &pager, uint16_t handyman);

// treatment_machine_used (0x38db0): a use of the machine of `illness`'s
// treatment (an object kind with an overlay, smoke, when worn) in the room
// holding `tile`: counted (up to twice its strength), the handymen paged
// to repair it once past a third of its strength (World::repair_pager,
// hospital 0's alone; the priority its wear in percent), and from 75% worn
// (modify_machine) smoking, drawing from world.seed. A breakdown
// (machine_disaster) is not ported: the machine goes on, and it returns
// false, as when it does not break down.
bool treatment_machine_used(World &world, uint8_t illness, uint16_t tile);

// find_possible_diagnosis (0x3e468): a diagnosis room the hospital has for
// patient `id`'s illness and that it has not been to, picked at random
// (world.seed), set as its treatment; 0 (also set) if none.
uint16_t find_possible_diagnosis(World &world, uint16_t id);

} // namespace th
