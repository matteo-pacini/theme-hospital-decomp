#pragma once

#include "core/world/world.h"

#include <cstdint>

namespace th
{

class AnimData;
struct PersonComplexes;
struct StaffComplexes;

// Vaccination by nurses (src/vaccine.cpp): a player marks a contagious
// patient of a cover-up for vaccination (PF_VACCINE_REQUESTED, a place in
// the hospital's vaccination queue); a nurse with none idle goes to the
// first queued patient she can reach (SS_NURSE_VACCINATES) and vaccinates
// them at gbv.vac_cost, ending their contagion.

// A staff state (SS_NURSE_VACCINATES, include/enums.h).
constexpr uint8_t kStaffNurseVaccinates = 0x40;

// is_person_highlighted_for_vaccination (0x986a0): whether patient
// `pointed` is the player's, contagious with the epidemic's illness and not
// yet queued for a vaccination, recording them in `target`. False
// otherwise, with `target` 0. The game's flag argument is overwritten with
// 0, so an already queued patient is never highlighted.
bool is_person_highlighted_for_vaccination(const World &world, uint16_t pointed, uint16_t &target);

// request_vaccination (0x98770): queues patient `id` for a vaccination
// (PF_VACCINE_REQUESTED) and counts the request.
void request_vaccination(World &world, const AnimData &anims, uint16_t id);

// cancel_vaccination_request (0x987c8): drops patient `id`'s vaccination
// request or nurse visit, clearing the flags and queue entry.
void cancel_vaccination_request(World &world, uint16_t id);

// insert_person_in_vaccination_queue (0x98844): appends patient `id` to
// their hospital's vaccination queue.
void insert_person_in_vaccination_queue(World &world, uint16_t id);

// remove_person_from_vaccination_queue (0x98950): takes patient `id` out of
// their hospital's vaccination queue. Returns 1 if they were in it.
uint8_t remove_person_from_vaccination_queue(World &world, uint16_t id);

// is_person_in_vaccine_queue (0x98a98): whether patient `id` is in their
// hospital's vaccination queue.
uint8_t is_person_in_vaccine_queue(const World &world, uint16_t id);

// should_nurse_vaccinate (0x98b2c): sends nurse `id` to the first queued
// patient outside rooms she can reach that no nurse is on the way to
// (PF_VACCINE_NURSE_SENT). Returns 1 if she went.
bool should_nurse_vaccinate(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                            const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

// nurse_vaccinates (0x98d0c), staff state SS_NURSE_VACCINATES: next to the
// patient, vaccinates them (they stop being contagious, at gbv.vac_cost's
// cost); otherwise follows them, or gives up and puts the request back.
void nurse_vaccinates(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                      const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

// could_nurse_vaccinate (0x98eb4): whether a queued patient outside rooms,
// with no nurse on the way, can be reached from nurse `id`'s room (or
// tile).
bool could_nurse_vaccinate(const World &world, uint16_t id);

} // namespace th
