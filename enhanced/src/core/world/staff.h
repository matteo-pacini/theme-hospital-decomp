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
constexpr uint8_t kStaffReceptionistWorks = 0x0a;
constexpr uint8_t kStaffReceptionistFindsReception = 0x0c;
constexpr uint8_t kStaffPlayComplexAnim = 0x0e;
constexpr uint8_t kStaffGotoObject = 0x12;
constexpr uint8_t kStaffTwoPeepsInteract = 0x1a;
constexpr uint8_t kStaffReceptWaitsForPatient = 0x20;
constexpr uint8_t kStaffReceptAdmitsPatient = 0x22;
constexpr uint8_t kStaffWaitForComplexSlave = 0x26;
constexpr uint8_t kStaffInCorridor = 0x28;
constexpr uint8_t kStaffComplexRetry = 0x2c;
constexpr uint8_t kStaffApplicant = 0x2e;
constexpr uint8_t kStaffWanders = 0x34;
constexpr uint8_t kStaffHandymanWorks = 0x3c;
constexpr uint8_t kStaffHeld = 0x44;
constexpr uint8_t kStaffReceptionistSetup = 0x4a;
constexpr uint8_t kStaffStuck = 0x4e;

// Thing.owner of applicants and of staff who left: no hospital's.
constexpr uint8_t kNoOwner = 0x63;

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

// The applicants of staff kind `kind` (gamevar.applicants[kind]): those
// listed (state kStaffApplicant) and those being hired (kStaffHeld, not yet
// anyone's).
int count_applicants(const World &world, uint8_t kind);

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
// happier or less happy by whether their salary meets it. The promotions it
// also makes (and announces) are not ported: they need an ability above the
// threshold, which no applicant has.
void appraise_staff(Thing &staff, const Balance &settings);

// The animation (an AnimData::starts index, the facing to be added) a staff
// kind shows when placed or held (staff_anim_starts[kind].anims[staff[kind].anim]).
uint16_t staff_held_anim(uint8_t kind);

// Whether staff may be put down on `cell` (column + row * 128) by player 0,
// as draw_staff_cursor (0x2b690) decides it outside rooms (no
// rooms exist yet): the cell is player 0's and has a path to the
// hospital's first entrance.
bool staff_place_ok(const World &world, int cell);

// place_staff_on_map (0x29c4c) for player 0: puts staff member `id` on
// `cell` if the cell is player 0's (or `force`), standing in the middle
// facing as it was (0 if stuck), in the corridor's state for its kind,
// walking at its ability's speed. A new hire becomes player 0's, with its
// hire turn `game_turn`. Returns whether it was placed.
bool place_staff_on_map(World &world, const AnimData &anims, const Balance &settings, int cell, uint16_t id, bool force,
                        uint32_t game_turn);

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
// - a handyman with no litter, plant or machine to see to strolls to a
//   random cell near by (handyman_works), sweeping litter he spots;
// - walking to an object (staff_goto_object), the complexes' states
//   (core/world/staff_complex.h) and being stuck (peep_stuck).
// Not ported, as they need what does not exist yet: rooms (staff rooms,
// pages, vaccinations, the doctors' and nurses' work), the per-turn
// counts, status icons and modify_staff (tiredness, warmth, happiness and
// discontent stay as they are, so no one resigns, tires or cracks up).
void staff_turn(World &world, const AnimData &anims, const PersonComplexes &person_complexes,
                const StaffComplexes &staff_complexes, uint16_t id, uint32_t game_turn);

// Carries out the simulation's part of the commands of hiring and placing
// staff, as process_turn (src/io.cpp) for player 0: EV_MARK_EMPLOYING
// (the applicant is held), EV_PLACE_STAFF (placed, the hiring fee of 12/10
// of the wage paid) and EV_CANCEL_BUILD (a held applicant goes back to the
// list). The interface's part is hire_staff_event (core/ui/hire_staff.h).
// Returns true for the first two; false for others and for
// EV_CANCEL_BUILD, which the other steps also act on.
bool apply_staff_event(Game &game, const Event &event);

} // namespace th
