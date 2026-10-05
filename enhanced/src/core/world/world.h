#pragma once

#include "core/data/anims.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/world/object_tables.h"
#include "core/world/objects.h"

#include <array>
#include <cstdint>
#include <vector>

namespace th
{

// A coordinate on the map in 1/256 cell, as the game's MapPos: `tile` is the
// cell's column (for x) or row (for y), `frac` the position across it, 0x80
// its middle.
struct MapPos {
    uint8_t frac = 0;
    uint8_t tile = 0;
};

// Thing.type values.
constexpr uint8_t kThingFree = 0;
constexpr uint8_t kThingPatient = 2;
constexpr uint8_t kThingStaff = 4;
constexpr uint8_t kThingObject = 6;

// A map cell by column and row (the game's PathStep and wallhug_waypoint).
struct Waypoint {
    uint8_t x = 0;
    uint8_t y = 0;
};

// A person's short planned path, as the game's orthonav_path (include/hosp.h)
// in Thing.path: built by orthonav_init (core/world/nav.h), followed by
// orthonav_whatnext.
struct OrthonavPath {
    // The last waypoint reached, or where the path was planned from.
    uint8_t x0 = 0;
    uint8_t y0 = 0;
    // The destination; x1 is 0xff once the path is to be planned again.
    uint8_t x1 = 0;
    uint8_t y1 = 0;
    // The next waypoints, steps[0] first; the last need not be the
    // destination (a route keeps only its first four).
    std::array<Waypoint, 4> steps{};
    // Waypoints in steps; 0 when no route was found.
    uint8_t count = 0;
};

// Entries of the things table, including the unused entry 0 (the game's
// thing[] holds 1500).
constexpr int kThingCount = 1500;

// The random seed setup_new_level (src/start.cpp) sets before loading a level.
constexpr uint32_t kLevelStartSeed = 0x546b2;

// One entry of the things table, with the fields of the game's Thing
// (include/hosp.h) that placing and drawing objects and walking people use.
struct Thing {
    // Links in the list of things on the cell at (x.tile, y.tile)
    // (MapCell.first); 0 ends the list.
    int16_t cell_prev = 0;
    int16_t cell_next = 0;
    MapPos x;
    MapPos y;
    // Current animation frame (AnimData::frames index).
    uint16_t sprite = 0;
    // kThingFree, kThingPatient, kThingStaff or kThingObject.
    uint8_t type = kThingFree;
    // Even state number; objects start in kObjects[subtype].initial_state.
    uint8_t state = 0;
    // Object kind (kObjects index); patient or staff kind (the game's
    // anim_starts or staff_anim_starts index).
    uint8_t subtype = 0;
    // The animation advances every anim_speed turns.
    uint8_t anim_speed = 0;
    // Nonzero: the frame is drawn mirrored.
    uint8_t flip = 0;
    // Direction the object faces, 0-3. People: the direction they walk, 0 -y,
    // 1 +x, 2 +y, 3 -x; 4 while stuck.
    uint8_t facing = 0;
    // People: turns until the next decision of where to step (person_head_for).
    int16_t countdown = 0;
    // People: distance walked per turn, in 1/256 cell.
    uint8_t speed = 0;
    // People: the cell they are walking to.
    uint8_t dest_x = 0;
    uint8_t dest_y = 0;
    // People: the planned path to (dest_x, dest_y).
    OrthonavPath path;
    // Added to the draw-order bucket within the cell (see thing_bucket).
    int8_t depth = 0;
    // Player index.
    uint8_t owner = 0;
    // The element id drawn for each animation layer; objects set 10-12.
    std::array<uint8_t, 13> layers{};
    // Where a person stands while using the object.
    MapPos use_x;
    MapPos use_y;
    // Links in the list of objects of the same room (0: none, the level's
    // objects are in no room) and of the same kind and owner
    // (World::first_of_kind).
    uint16_t room_obj_prev = 0;
    uint16_t room_obj_next = 0;
    uint16_t type_obj_prev = 0;
    uint16_t type_obj_next = 0;
    // Cells a person uses the object from (0-2) and leaves to afterwards (3-5).
    std::array<uint16_t, 6> use_pos{};
    // The room the object is in, 0 for none.
    uint16_t room_id = 0;
    // The other half of a two-part object, 0 for none.
    uint16_t pair = 0;
    // How people use the object (kObjects[subtype].use_style).
    uint8_t use_style = 0;
    // Plants: 400 healthy, wilting below.
    int16_t plant_health = 0;

    // Patient (arrivals.cpp)
    // Their illness (the game's expertise index; ILL_* in include/enums.h).
    uint8_t illness = 0;
    // The illness they are diagnosed with; 1 (general practice) while unknown.
    uint8_t known_illness = 0;
    // The expertise index of the room they are sent to next.
    uint8_t treatment = 0;
    // subtype before an illness changed their appearance.
    uint8_t orig_subtype = 0;
    // Bit of their insurance company, or 0.
    uint8_t insurance = 0;
    // Above 0 contagious; 0xff vaccinated.
    uint8_t contagion = 0;
    // Status icon over their head (the game's icon_table index), 0 for none.
    int8_t icon_id = 0;
    // 0-1000, dead at 0.
    int16_t health = 0;
    // Health over time; a new patient's is their health truncated to a byte.
    std::array<int8_t, 10> health_history{};
    // 0-100.
    uint16_t happiness = 0;
    // How warm they feel, 0-100.
    uint16_t warmth = 0;
    // Falls over time; below 30 they are thirsty.
    int16_t refreshment = 0;
    // Rises over time; they need the toilet past gbv.bowel_full.
    int16_t bladder = 0;
    // Money they can spend.
    int32_t cash = 0;
    // Subtracted from diagnosis progress.
    uint16_t diagnosis_handicap = 0;

    // Action sequences (complex.cpp)
    // People: the running complex's next step (a PersonComplexes::group
    // index), 0 for none; complex_action the complex last given
    // (set_complex); complex_resume the step a step of action 24 left to
    // resume at, 0 for none.
    uint16_t complex_pc = 0;
    uint16_t complex_action = 0;
    uint16_t complex_resume = 0;
    // People: up to two complexes queued behind the running one, 0 for none,
    // with their destination cells.
    uint16_t complex_queued = 0;
    uint16_t complex_queued2 = 0;
    uint8_t queued_arg_x = 0;
    uint8_t queued_arg_y = 0;
    uint8_t queued2_arg_x = 0;
    uint8_t queued2_arg_y = 0;
    // People: runs of the current step left (its repeat).
    uint8_t complex_repeat = 0;
    // People: the state entered when the complex ends, 0 to keep the last
    // step's; the state a failed complex returns to, 0 to retry it
    // (PS_COMPLEX_RETRY).
    uint8_t next_state = 0;
    uint8_t resume_state = 0;
    // People: the state and destination PS_STUCK resumes.
    uint8_t saved_state = 0;
    uint8_t saved_dest_x = 0;
    uint8_t saved_dest_y = 0;
    // People: turns left in PS_WAITS_FOR_BUSY.
    int16_t timer = 0;
    // People: the object or door they use or walk to, 0 for none; the cell
    // they leave it to.
    uint16_t object = 0;
    uint16_t exit_tile = 0;
    // People: the room they are in (a room_info index), 0 for none.
    uint16_t room = 0;
    // Objects and doors: 1 while someone uses it, and who.
    uint8_t in_use = 0;
    uint16_t user = 0;
    // Objects: depth while not overlaid by its user.
    int8_t saved_depth = 0;

    // Staff (staff.cpp)
    // Staff: the step within their state (the game's substate).
    uint8_t substate = 0;
    // Staff: kSkill* bits (core/world/staff.h).
    uint16_t skills = 0;
    // Staff: 30-1000.
    uint16_t ability = 0;
    // Staff: the monthly wage paid, and the one asked for (appraise_staff).
    int32_t salary = 0;
    int32_t wage_demand = 0;
    // Staff: 0-1000.
    int16_t tiredness = 0;
    // Applicants: they withdraw once the game turn passes this; 0 once hired.
    uint32_t hire_expiry = 0;
    // Staff: the game turn they were hired, kept in 16 bits as the game does.
    int16_t hire_time = 0;
    uint8_t work_delay = 0;
    // Staff: their name, "<initial>. <surname_a><surname_b>" (the initial
    // 'A'-'Z', the surname halves strings of language sections 8 and 9).
    uint8_t initial = 0;
    uint8_t surname_a = 0;
    uint8_t surname_b = 0;
    // Staff: their picture's head, body and legs in the faces file.
    uint8_t face_head = 0;
    uint8_t face_body = 0;
    uint8_t face_legs = 0;
    // Staff: their reference, strings of language sections 0x2d (general),
    // 0x2e (good) and 0x2f (bad).
    uint8_t ref_general = 0;
    uint8_t ref_good = 0;
    uint8_t ref_bad = 0;
    uint8_t female = 0;
    // Handymen: the priorities of litter, plants and repairs.
    uint8_t prio_litter = 0;
    uint8_t prio_plants = 0;
    uint8_t prio_repair = 0;

    // Staff at work (staff_complex.cpp)
    // Staff: the patient they deal with, or their partner in an interaction
    // (the game's slave), 0 for none.
    uint16_t slave = 0;
    // Staff: calls to their slave left before giving up (call_person_again).
    int16_t call_retries = 0;
    // People in an interaction (two_peeps_interact): the step reached in the
    // interaction table, the times it plays again, and 1 while its animation
    // plays, 2 once it has, 0 for none.
    uint8_t seq_index = 0;
    int8_t interact_repeat = 0;
    uint8_t interact_anim = 0;
    // Handymen: the nearest and second nearest litter cells spotted
    // (handyman_check_for_litter), 0 for none.
    uint16_t work_target = 0;
    uint16_t work_target2 = 0;
    // Handymen: litter swept, plants watered and machines repaired, which
    // weigh their choice of job.
    uint16_t litter_count = 0;
    uint16_t plant_count = 0;
    uint16_t repair_count = 0;

    // Litter (litter.cpp)
    // Patients: the game's Thing.flags (PF_*; kPf* in core/world/litter.h).
    uint32_t person_flags = 0;
    // Patients: the game turn, in 16 bits, from which modify_person runs
    // again.
    uint16_t modify_time = 0;
    // Patients: grows near litter and after a drink; past gbv.litter_drop
    // they drop litter.
    uint16_t litter_urge = 0;
    // Patients: rises near litter and dead plants and when too hot; from
    // gbv.vomit_limit they may vomit.
    uint16_t nausea = 0;
    // Patients: grow while they wait and are unhappy, thirsty or need the
    // toilet.
    uint16_t impatience = 0;
    uint16_t waiting_time = 0;
    // Patients: health changes held back while they are in a room.
    int16_t health_delta = 0;
    // Patients playing a reaction (PS_SHOWANIM): the frame and mirroring to
    // go back to; the state is kept in substate.
    uint16_t saved_sprite = 0;
    uint8_t saved_flip = 0;

    // Queues (queue.cpp)
    // Patients: their neighbours in the queue of room queue_room (a
    // World::rooms index; RoomInfo.queue_head), 0 for none.
    uint16_t queue_prev = 0;
    uint16_t queue_next = 0;
    uint16_t queue_room = 0;
    // Patients: the room they are on their way to queue for, 0 for none.
    uint16_t dest_room = 0;
    // Patients: the price quoted for their treatment.
    int32_t fee = 0;
    // Patients: the treatment after `treatment`, 0 for none.
    uint8_t next_treatment = 0;
};

// Staff at work (staff_complex.cpp)
// RoomInfo.type values (the game's ROOM_*).
constexpr uint8_t kRoomFree = 0;
constexpr uint8_t kRoomReception = 4;

// Entries of the rooms table, including the unused entry 0 (the game's
// room_info holds 100).
constexpr int kRoomCount = 100;

// One entry of the rooms table, with the fields of the game's RoomInfo
// (include/hosp.h) used so far. Only receptions exist yet: one per group of
// manned desks (allocate_reception_roominfo), a 1x1 room at cell 0x3fff.
struct RoomInfo {
    uint8_t type = kRoomFree;
    int8_t width = 0;
    int8_t height = 0;
    // 0 or 1 in use; 3 blueprint, 4 being edited, 5 repair due, 6 closed.
    int8_t state = 0;
    // The cell (column + row * 128) of its top-left corner.
    uint16_t pos = 0;
    // Patients served by its receptionist.
    uint16_t patients_seen = 0;
    uint8_t owner = 0;
    uint8_t queue_capacity = 0;

    // Queues (queue.cpp)
    // The cell outside its door, where patients queue, and the one inside;
    // 0 for a reception.
    uint16_t door_tile = 0;
    uint16_t door_inside_tile = 0;
    // The first patient queueing (Thing.queue_next), 0 for none; the
    // patients queueing, and those on their way to queue (not counted for a
    // reception).
    uint16_t queue_head = 0;
    uint8_t queue_length = 0;
    uint8_t queue_incoming = 0;
};

// The map and the things on it.
// The route planner people use: A*, the default, or the game's wall
// follower (wallhug_tricky), kept for comparing with the original.
enum class PathFinder : uint8_t { AStar, Original };

// Queues (queue.cpp)
struct Balance;
struct HospitalMarket;
struct Finance;

struct World {
    std::array<MapCell, kMapCells> map{};
    std::vector<Thing> things = std::vector<Thing>(kThingCount);
    // Per object kind, the first object of player 0's list of that kind
    // (Thing.type_obj_next), as the game's listhead[0].objects.
    std::array<uint16_t, kObjectKinds> first_of_kind{};
    // The simulation's random generator state (the game's seed).
    uint32_t seed = kLevelStartSeed;
    // How people's routes are planned (orthonav_init, core/world/nav.h).
    PathFinder path_finder = PathFinder::AStar;

    // Action sequences (complex.cpp)
    // Per hospital, the cells of its four entrances, where patients arrive
    // and leave (the door_tile of its ROOM_ENTRANCE rooms): Level::entrances
    // for the hospitals that play (load_level, src/io.cpp).
    std::array<std::array<uint16_t, 4>, 4> entrances{};

    // The player's objects bought and being placed, and the requesters for
    // them (core/world/objects.h).
    ObjectPlacing placing;

    // Staff at work (staff_complex.cpp)
    // The rooms (the game's room_info), indexed by Thing.room.
    std::array<RoomInfo, kRoomCount> rooms{};

    // Queues (queue.cpp)
    // What the patients' states read from outside the world, set by
    // start_queues (core/world/queue.h) and outliving the turns run: the
    // level's settings, hospital 0's prices and reputations
    // (ArrivalState::hospitals[0]), its reputation and its finances.
    const Balance *settings = nullptr;
    HospitalMarket *market = nullptr;
    const int *reputation = nullptr;
    const Finance *finance = nullptr;
    // The turn's random distances (othervar.queue_near_dist, 0-5, and
    // object_near_dist, 1-4; draw_turn_randoms): how near their queue's cell
    // or their object patients count as there.
    uint8_t queue_near_dist = 0;
    uint8_t object_near_dist = 0;
    // Hospital 0's treatments it can give (allhosps[0].rooms[].cure_available,
    // 1 if so), by illness; 0 (no treatment) always.
    std::array<uint8_t, 47> cure_available{};
    // The insurance companies (insurance_co indices) the patients' insurance
    // bits 1 and 2 pick (ins_control[1] and [2]); bit 4 reads ins_control[4],
    // past the game's table, as 0.
    std::array<uint16_t, 5> ins_control{1, 2, 3, 0, 0};
    // For the advisor: patients of hospital 0 who left fed up
    // (othervar.patients_left_count), the bench shortage
    // (NRAdcounts.bench_shortage), and the last treatment a patient was asked
    // to pay for (advisor.unpaid_treatment) with whether one would not pay
    // (advisor.event_flags 2).
    uint8_t patients_left_count = 0;
    int32_t bench_shortage = 0;
    uint16_t unpaid_treatment = 0;
    bool too_expensive = false;
    // Hospital 0's drinks sales this month (allhosps[0].drinks_income), which
    // month_end banks.
    int32_t drinks_income = 0;
};

// Advances `seed` as the game's old_newrand (src/rand.cpp) and returns its
// new value.
uint32_t old_newrand(uint32_t &seed);

// Resets `world` and builds a level's starting map with its objects, as the
// game's recreate_map (src/town.cpp) when a level starts: build_start_map,
// with the object a cell holds (all of player 0) placed as soon as the cell
// is copied (adjust_mapwho, place_object).
// So a cell's copy overwrites what objects on earlier cells marked on it
// (flags, invisible walls, a second part on its list), as in the game.
// Shadows are not computed. The random generator starts from `seed`.
void start_level(World &world, const Level &level, const AnimData &anims, uint32_t seed = kLevelStartSeed);

// can_place_object (src/objects.cpp): whether an object of `kind` facing
// `dir` (kObjects index, 0-3) fits for player `owner` with its corner on
// `tile`: no wall its shape forbids and nothing on its cells or its use
// cells, all on the owner's land (litter bombs on anyone's) and `tile`
// indoors. Shapes reaching off the map do not fit.
bool can_place_object(World &world, int tile, int kind, int dir, int owner);

// mark_object_shape and remove_object_shape (src/objects.cpp): mark the cells
// an object covers and is used from (MapCell.flags 1 and 2, 4) and put up
// the invisible walls round it, or take them down. Path checks mark an
// object that is not placed yet; remove clears the bits whatever else set
// them.
void mark_object_shape(World &world, int tile, int kind, int dir);
void remove_object_shape(World &world, int tile, int kind, int dir);

// place_object (src/objects.cpp) without its checks of the hospital's
// access, of people in the way and of picked-up objects, and without
// charging: a new object of `kind` for `owner` on `tile` facing `dir`, and
// its second part where that fits, linked into the map and
// World::first_of_kind (player 0's). Unless `force`, refuses with 0 when it
// does not fit (can_place_object). Returns its thing id, 0 if refused or the
// things table is full. Draws two random numbers per object made.
uint16_t place_object(World &world, const AnimData &anims, int tile, int kind, int dir, int owner, bool force);

// Map pixel position (as cell_to_world) of the point a thing's frame is
// drawn from, as the game's do_mapwho at 64 x 32 cells: the cell's top corner
// moved by its position within the cell, so the middle of a cell (fractions
// 0x80) is the middle of its diamond.
inline Point thing_to_world(const Thing &t)
{
    Point p = cell_to_world(t.x.tile, t.y.tile);
    int dx = int(t.x.frac) - int(t.y.frac);
    int sum = int(t.x.frac) + int(t.y.frac);
    return {p.x + (dx >> 3), p.y + (sum >> 4)};
}

// The game's draw-order bucket of a thing within its cell (do_mapwho): the
// cell's things are drawn by ascending bucket and, within a bucket, from the
// end of the cell's list to its head.
inline int thing_bucket(const Thing &t)
{
    return ((int(t.x.frac) + int(t.y.frac)) >> 4) + t.depth;
}

} // namespace th
