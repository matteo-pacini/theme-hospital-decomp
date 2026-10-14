#pragma once

#include "core/data/anims.h"
#include "core/data/level.h"
#include "core/render/iso.h"
#include "core/world/build.h"
#include "core/world/computer.h"
#include "core/world/object_tables.h"
#include "core/world/objects.h"
#include "core/world/research.h"

#include <array>
#include <cstdint>
#include <source_location>

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
constexpr uint8_t kThingReaper = 0xa;
constexpr uint8_t kThingRat = 0xc;
constexpr uint8_t kThingDebris = 0xe;
constexpr uint8_t kThingHelicopter = 0x10;
constexpr uint8_t kThingNumfx = 0x12;
constexpr uint8_t kThingMayor = 0x14;
constexpr uint8_t kThingInspector = 0x16;
constexpr uint8_t kThingShadow = 0x18;

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
    // A dead patient floating away (PS_FLYS): its height above the floor
    // (the game's fly_height, which shares the field).
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
    // The status icon's animation frame (icon_id's start[] frame, advanced by
    // update_icon_counter), and how many turns it has left (0xffff, -1 as a
    // signed value, for a permanent icon).
    uint16_t icon_frame = 0;
    int16_t icon_timer = 0;
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
    // A VIP or inspector: the state it enters on reaching its destination
    // (mayor_goto).
    uint8_t goto_state = 0;
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
    // People: the room they are in (a room_info index), 0 for none, and
    // their neighbours in its list of people (RoomInfo.first_person), 0 for
    // none.
    uint16_t room = 0;
    uint16_t room_next = 0;
    uint16_t room_prev = 0;
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
    // Staff: grows while they are unhappy (modify_staff).
    uint16_t discontent = 0;
    // Doctors: 1 once they have cracked up (doctor_cracks_up).
    uint8_t crazy = 0;
    // Doctors: training points towards each skill (surgeon, psychiatrist,
    // researcher), gained in a training room (training_done).
    uint8_t surgeon_training = 0;
    uint8_t psych_training = 0;
    uint8_t research_training = 0;
    // Staff: kOrder* bits (the game's ORD_*), and the game turn their
    // notice runs out; the notices handed in since hired.
    uint8_t orders = 0;
    uint32_t resign_time = 0;
    uint8_t resign_count = 0;
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
    // Patients: their neighbour in their hospital's vaccination queue
    // (World::Epidemic::vaccine_queue), 0 for none (vaccine.cpp).
    uint16_t vaccine_next = 0;
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

    // Building (build.cpp)
    // Double doors (kThingDoor): on the main half, the other half (an
    // object); on that half, the main one; 0 for none.
    uint16_t door_partner = 0;
    uint16_t door_main = 0;

    // Floating numbers (kThingNumfx): how it is drawn (the game's NUMFX_*),
    // the value it shows, its colour in subtype, its lifetime in speed and
    // its height in anim_speed.
    uint8_t numfx_type = 0;
    int16_t numfx_amount = 0;

    // Rooms in use (gp)
    // Patients: diagnosis progress, complete at the game's
    // gbv.diagnosis_complete (1000); the treatments given (Thing.treatments),
    // oldest first, 0 past the last; their player call (PlayerCall::id), 0
    // for none.
    int16_t diagnosis = 0;
    std::array<uint8_t, 10> treatments{};
    uint16_t playercall = 0;
    // Doors in use: 1 if their cell had MapCell.flags 0x10 before (the
    // game's aux_param).
    uint8_t door_cell_flag = 0;

    // Rooms in use (diagnosis)
    // Machines: uses since the last repair, and the uses they stand (the
    // level's StartStrength for their kind when placed); the three smoke
    // puffs shown as they wear out, 1 each once shown.
    int32_t times_used = 0;
    int32_t strength = 0;
    std::array<uint8_t, 3> smoke{};

    // Treatment rooms
    // Machines: a second frame drawn with their user (draw_split_frame) and
    // its mirroring, the height at which the user's frame is split, and the
    // drawing effect's flags (0xff: the user not drawn) and parameter (the
    // game's aux_param). The gates of hell: the frame to go on to once
    // their animation ends (object_hell), and remove_after_anim, removed
    // then instead.
    uint16_t aux_sprite = 0;
    uint8_t aux_flip = 0;
    uint8_t split_height = 0;
    uint8_t aux_draw_flags = 0;
    uint8_t aux_param = 0;
    uint8_t remove_after_anim = 0;
    // Projectors: 1 once the consultant has started the lecture, cleared
    // when they leave the training room (consultant_trains,
    // staff_exit_room_check).
    uint8_t lecture_started = 0;

    // The grim reaper (kThingReaper; reaper.cpp)
    // The gates of hell it leads its victim (slave) to; the frame, mirroring
    // and state to go on to once its animation ends (reaper_play_anim).
    uint16_t gate = 0;
    uint16_t next_sprite = 0;
    uint8_t next_flip = 0;
    uint8_t reaper_next_state = 0;

    // Rats (rats.cpp)
    // A rat's step: how far it moves each turn (1/65536 cell) across and down,
    // and the turns left on its current leg.
    int32_t rat_dx = 0;
    int32_t rat_dy = 0;
    int32_t rat_ticks = 0;

    // Emergencies (emergency.cpp)
    // Helicopters (kThingHelicopter): their height above their cell, falling
    // by `speed` a turn as they land and rising as they leave, and their
    // shadow (a kThingShadow), 0 for none. Shadows: the thing casting them.
    int16_t altitude = 0;
    uint16_t shadow = 0;
    uint16_t caster = 0;

    // Epidemics (epidemic.cpp)
    // The health inspector (kThingInspector): the room it is at, counted
    // through the hospital's rooms, 0 for the reception desk (src/mayor.cpp).
    uint8_t inspect_room = 0;

    // Machine disasters (disaster.cpp)
    // Machines: the frame of their smoke drawn over them once worn
    // (modify_machine), 0 for none. Explosion debris (kThingDebris): its
    // speed across (1/256 cell) and up (1/256 pixel, its altitude the
    // same) a turn, and the room it was thrown in, 0 for none.
    uint16_t overlay_frame = 0;
    int16_t vel_x = 0;
    int16_t vel_y = 0;
    int16_t vel_z = 0;
    uint16_t bang_room = 0;
};

// Staff at work (staff_complex.cpp)
// RoomInfo.type values (the game's ROOM_*).
constexpr uint8_t kRoomFree = 0;
constexpr uint8_t kRoomEntrance = 1;
constexpr uint8_t kRoomReception = 4;

// Entries of the rooms table, including the unused entry 0 (the game's
// room_info holds 100).
constexpr int kRoomCount = 100;

// One entry of the rooms table, with the fields of the game's RoomInfo
// (include/hosp.h) used so far: the hospitals' entrances (kRoomEntrance,
// made when the level starts: size 0, with pos, door_tile and
// door_inside_tile all the entrance's cell; start_entrance_rooms), the rooms
// the player builds (core/world/build.h) and the receptions, one per group of
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

    // Building (build.cpp)
    // The first object in the room (Thing.room_obj_next), 0 for none.
    uint16_t first_object = 0;
    // The game turn its blueprint was laid down.
    uint32_t created_turn = 0;
    // Radiators in the room, and windows in its walls.
    uint8_t radiators = 0;
    uint8_t windows = 0;
    // 1 once its price was paid (when it first opened).
    uint8_t paid_for = 0;
    // 1 while it waits for the people in it to leave before it is edited.
    uint8_t being_edited = 0;

    // Rooms in use (gp)
    // The staff holding the room (check_room_mutex): two in an operating
    // theatre, staff[0] alone elsewhere; 0 for none. staff_free is 1 while
    // its member of staff is free for a patient.
    std::array<uint16_t, 2> staff{};
    uint8_t staff_free = 0;

    // Rooms in use (diagnosis)
    // The first of the people in it (Thing.room_next), the latest come; 0
    // for none. Receptionists are not listed in their reception.
    uint16_t first_person = 0;
};

// The map and the things on it.
// The route planner people use: A*, the default, or the game's wall
// follower (wallhug_tricky), kept for comparing with the original.
enum class PathFinder : uint8_t { AStar, Original };

// Queues (queue.cpp)
struct Balance;
struct HospitalMarket;
struct StaffMarket;
struct Finance;
class Game;

// The panel's fax icons (src/panel.cpp), as the game keeps them with its
// calls: per slot 0-8, left to right, the World::calls index shown there
// (panel_popup_pos), 0 for none; the message bar's shutter
// (othervar.popup_shutter_state, width and timer), which a new icon closes
// before it slides in; and the calls given to the player since the
// interface last played their sound (add_panel_popup's SFX_NEWFAX). The
// panel's drawing advances them; nothing in the simulation reads them.
struct PanelPopups {
    std::array<uint16_t, 9> pos{};
    uint8_t shutter_state = 0;
    uint8_t shutter_width = 0;
    uint8_t shutter_timer = 0;
    uint8_t new_faxes = 0;
};

// The statistics pass of every 32nd turn (the game's adcounts, src/environ.cpp:
// reset_all_count, person_count, staff_count, object_count, final_count),
// hospital 0's figures for the advisor; sums kept in the game's widths.
// patients starts at 1, so that it divides. idle_staff is per staff kind,
// [4] the surgeons' (the game's byte past its idle_staff[4]).
struct AdCounts {
    uint16_t patients = 1;
    uint16_t impatience_sum = 0;
    int32_t waiting_sum = 0;
    std::array<int32_t, 5> wait_sums{};
    std::array<int32_t, 5> wait_counts{};
    int16_t happiness_sum = 0;
    int16_t max_happiness = 0;
    int16_t min_happiness = 999;
    int16_t refreshment_sum = 0;
    int16_t min_refreshment = 999;
    int16_t warmth_sum = 0;
    int16_t max_warmth = 0;
    int16_t min_warmth = 999;
    int16_t bladder_sum = 0;
    int16_t max_bladder = 0;
    std::array<uint8_t, 0x26> illness_counts{};
    uint16_t doctors_and_nurses = 0;
    uint8_t surgeons = 0;
    uint8_t consultants = 0;
    uint8_t researchers = 0;
    uint8_t psychiatrists = 0;
    std::array<int16_t, 4> staff_tiredness_by_kind{};
    std::array<int16_t, 4> staff_happiness_by_kind{};
    uint16_t staff_happiness_sum = 0;
    int16_t staff_tiredness_sum = 0;
    int16_t max_staff_tiredness = 0;
    int16_t staff_warmth_sum = 0;
    std::array<uint8_t, 5> idle_staff{};
    // Doctors who cracked up since the pass began (doctor_cracks_up).
    uint8_t crackups = 0;
    uint8_t machines = 0;
    int32_t machine_wear_sum = 0;
    int32_t max_machine_wear = 0;
    uint8_t worst_machine = 0;
    // The statistics pass found a patient, of any hospital, since the level
    // began (advisor.has_patients).
    bool has_patients = false;
    // The strength of the last earthquake (NRAdcounts.last_quake_size), for
    // the advisor's damage report.
    uint8_t last_quake_size = 0;
    // The rooms opened and not deleted (NRAdcounts.open_rooms): open_room
    // counts one up, delete_room one down, whoever's room and whether or not
    // it was ever opened.
    uint8_t open_rooms = 0;
};

// Rat state values (the game's RAT_*, include/enums.h).
constexpr uint8_t kRatScurries = 2;
constexpr uint8_t kRatArrives = 4;
constexpr uint8_t kRatLeaves = 6;

// A player's rat hole (the game's RatHole): the cell and the side of the
// building its wall is on (0 north, 1 east, 2 south, 3 west), which decides
// how a rat appears and leaves.
struct RatHole {
    uint16_t tile = 0;
    uint8_t side = 0;
};

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
    // Per hospital, its four entrances, where patients arrive and leave
    // (allhosps[].entrances): their kRoomEntrance rooms, World::rooms
    // indices. Every hospital has hospital 0's (start_entrance_rooms).
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
    // (ArrivalState::hospitals[0]) and its finances.
    const Balance *settings = nullptr;
    HospitalMarket *market = nullptr;
    // The job market the applicants are counted in (Game::staff_market).
    StaffMarket *staff_market = nullptr;
    // The fax picture's palette is the one the game has loaded, so its colours
    // (colour_red, colour_dark_blue) are the current ones; see FaxScreen.
    bool fax_palette = false;
    const Finance *finance = nullptr;
    // The turn's random distances (othervar.queue_near_dist, 0-5, and
    // object_near_dist, 1-4; draw_turn_randoms): how near their queue's cell
    // or their object patients count as there. rand_pct is the turn's first
    // draw modulo 100 (othervar.rand_pct).
    uint8_t rand_pct = 0;
    uint8_t queue_near_dist = 0;
    uint8_t object_near_dist = 0;
    // Hospital 0's treatments it can give (allhosps[0].rooms[].cure_available,
    // 1 if so), by illness; 0 (no treatment) always.
    std::array<uint8_t, 47> cure_available{};
    // The game's ins_control: the level's three insurance companies
    // (insurance_co indices 1-8, setup_insurances; the claims' payment
    // delays and the bank manager go by them), then 0. shall_peep_pay indexes
    // it by a patient's insurance bit instead: bits 1 and 2 read [1] and [2];
    // bit 4 reads [4], past the game's table, as 0.
    std::array<uint16_t, 5> ins_control{1, 2, 3, 0, 0};
    // For the advisor: patients of hospital 0 who left fed up
    // (othervar.patients_left_count), the bench shortage
    // (NRAdcounts.bench_shortage), and the last treatment a patient was asked
    // to pay for (advisor.unpaid_treatment) with whether one would not pay
    // (advisor.event_flags 2).
    uint8_t patients_left_count = 0;
    int32_t bench_shortage = 0;
    // Hospital 0's plant figures as the game keeps them (NRAdcounts.plants,
    // thirsty_plants and plant_health_sum), which placing, plant_ages,
    // watering and removal update; the advisor's plant advice and the year's
    // plant trophy read them.
    uint16_t plants = 0;
    uint16_t thirsty_plants = 0;
    int32_t plant_health_sum = 0;
    // What the simulation tells the advisor (advisor.event_flags'
    // kAdvisorEventResign and kAdvisorEventCall of core/world/advisor.h),
    // which Game moves into Advisor::event_flags after each turn and
    // command.
    uint32_t advisor_events = 0;
    // A person of hospital 0 got stuck (gamevar.announce[ANN_STUCK]), until
    // the advisor says so.
    bool announce_stuck = false;
    uint16_t unpaid_treatment = 0;
    bool too_expensive = false;
    // A member of staff of hospital 0 was put down in a room they cannot
    // work in (advisor.new_staff_pending, new_staff_kind, new_staff_room:
    // the staff kind and the room's type), until the advisor says so.
    struct NewStaff {
        bool pending = false;
        uint8_t kind = 0;
        uint8_t room = 0;
    };
    NewStaff new_staff;
    // Hospital 0's drinks sales and paid amusements this month
    // (allhosps[0].drinks_income and jukebox_income, sales_income[0] and
    // [1]), which month_end banks.
    int32_t drinks_income = 0;
    int32_t jukebox_income = 0;
    // Hospital 0's member of staff picked up off the map to be placed again
    // (allhosps[0].held_staff), 0 for none, and the cell they were taken
    // from (held_staff_tile), where cancelling puts them back.
    uint16_t held_staff = 0;
    uint16_t held_staff_tile = 0;
    // Hospital 0's nurses that wandered this turn (allhosps[0].idle_nurses,
    // reset by start_turn), which a nurse in a room leaving to vaccinate and
    // the advisors read.
    uint16_t idle_nurses = 0;
    // Hospital 0's staff count (allhosps[0].staff_count): a counter, up by
    // each hire placed (inc_staff_stats) and down by each sacking, so it
    // follows the original's even where a hire names a thing that is not an
    // applicant.
    uint16_t staff_count = 0;

    // Building (build.cpp)
    // The room the player builds and the build tool's state.
    Building building;
    // 1 while a finished room is opened for editing (othervar.editing_room):
    // open_room then leaves last_built_room alone.
    uint8_t editing_room = 0;

    // Rooms in use (gp)
    // A request for staff with given skills to come to a room (the game's
    // SkillPager); a free one has mask 0.
    struct Pager {
        uint16_t mask = 0;
        uint16_t room = 0;
        // The game turn until which the responder holds it.
        uint32_t claimed_until = 0;
        uint16_t responder = 0;
        uint8_t owner = 0;
    };
    std::array<Pager, 30> pagers{};
    // Per room type, hospital 0's room of that type (listhead[0].rooms),
    // set once a room's doors are committed; 0 for none.
    std::array<uint16_t, 31> room_of_type{};
    // The player calls waiting for an answer (the game's playercall[], entry
    // 0 unused): their kind (0 for a free entry), subject, id and hospital,
    // and the days left before they expire (0 never). A call of hospital 0
    // also has its fax icon's panel slot (0-8, 9 when all were taken) and
    // the step of its slide onto the panel (kPopupSlide index, 0 once in
    // place), which the panel's drawing advances (core/world/popups.h).
    struct PlayerCall {
        uint16_t type = 0;
        uint16_t subject = 0;
        uint16_t id = 0;
        uint8_t owner = 0;
        int16_t timer = 0;
        uint8_t slot = 0;
        uint8_t slide_step = 0;
    };
    std::array<PlayerCall, 50> calls{};
    // Hospital 0's requests for a handyman to see to a thing (the game's
    // ThingPager allhosps[0].repair_pager): the machines to repair, by
    // priority; the first `count` of `pages` are used.
    struct ThingPage {
        uint16_t id = 0;
        uint8_t priority = 0;
        uint16_t responder = 0;
        uint32_t claimed_until = 0;
    };
    struct ThingPager {
        uint8_t count = 0;
        std::array<ThingPage, 0x40> pages{};
    };
    ThingPager repair_pager;
    // Hospital 0's requests for a handyman to water a plant (the game's
    // ThingPager allhosps[0].plant_pager), by priority.
    ThingPager plant_pager;
    // The id the next call gets (popup_id[0], from 1).
    uint16_t next_call_id = 1;
    // Per hospital and call kind (1-8), the call its panel icon stands for
    // (the game's listhead[].first_call): a calls index, 0 for none.
    std::array<std::array<uint16_t, 9>, 4> first_call{};
    // The panel's fax icons (core/world/popups.h).
    PanelPopups popups;
    // The statistics pass (core/world/counts.h).
    AdCounts counts;
    // Hospital 0's litter on the map (allhosps[0].litter_count).
    uint16_t litter_count = 0;

    // Rats (rats.cpp): hospital 0's rat holes (allhosps[0].ratholes and
    // rathole_count), the cell of its last litter (last_litter_tile), and
    // the rat game's state (othervar.next_rat_layer, rat_near_margin,
    // rat_hit_margin, rat_powerup_shots and gamevar.rat_fest_*).
    struct Rats {
        static constexpr int kMaxHoles = 0x28;
        std::array<RatHole, kMaxHoles> holes{};
        uint8_t hole_count = 0;
        uint16_t last_litter_tile = 0;
        uint8_t next_rat_layer = 0;
        uint16_t near_margin = 0x10;
        uint16_t hit_margin = 4;
        uint8_t powerup_shots = 0;
        uint8_t fest_level = 0;
        uint16_t fest_kills_mark = 0;
        uint8_t fest_powerup = 0;
        uint32_t fest_rats = 0;
    };
    Rats rats;

    // Emergencies (emergency.cpp)
    // Hospital 0's emergency taken on (allhosps[0]'s emergency fields).
    struct Emergency {
        // 0 none; 10 once accepted, falling by a day to 5, when the
        // helicopter is sent (3 once it is); 2 once the timer runs out; 1
        // once over, until its result call is answered or expires.
        int8_t state = 0;
        // Half days left to treat the patients, from 100 (the panel's watch).
        int16_t timer = 0;
        // The patients' illness, how many come, how many are still to leave
        // the helicopter, how many were cured, the bonus per patient cured
        // and the percentage to cure for it.
        uint8_t illness = 0;
        uint8_t patients = 0;
        int8_t to_spawn = 0;
        uint8_t cured = 0;
        uint16_t bonus = 0;
        uint8_t win_percent = 0;
        // Emergency patients indoors at the last statistics pass
        // (person_count, every 32 turns).
        uint8_t present = 0;
        // The helicopter (a thing), 0 for none, and 1 while it waits landed.
        uint16_t heli = 0;
        uint8_t heli_landed = 0;
        // Where the patients come out of the helicopter (Level::emergency_tile[0]);
        // it lands a row below.
        uint16_t heli_pad_tile = 0;
    };
    Emergency emergency;

    // VIP visits (mayor.cpp)
    // Hospital 0's VIP visit (allhosps[0]'s mayor fields): the thing (0 for
    // none), the VIP's name index (mayor_names), 0 none / 1 offered / 2
    // scheduled / 3 on site / 4 gone, the turn an accepted visit starts, the
    // verdict (a mayor_result index), how many offers in a row were refused,
    // and the days counted towards the next offer.
    struct Mayor {
        uint32_t arrival_turn = 0;
        uint16_t thing = 0;
        uint8_t name = 0;
        uint8_t state = 0;
        uint16_t verdict = 0;
        uint16_t postponed = 0;
        uint16_t launch_count = 0;
    };
    Mayor mayor;
    // The level's scroll area (Level::scroll_min and scroll_max), which the
    // earthquake's screen shake stays within.
    uint16_t scroll_min = 0;
    uint16_t scroll_max = kMapCells - 1;
    // The earthquake's view shake so far, in map pixels (shake_screen), which
    // the interface adds to its camera.
    int16_t shake_x = 0;
    int16_t shake_y = 0;

    // Machine disasters (disaster.cpp)
    // Hospital 0's mishaps (allhosps[0].mishaps): machines blown up and
    // hopeless diagnoses (diagnose_patient); and whether a machine has
    // (award_counts.machine_exploded, for the year's awards). The machines'
    // sound effects (SFX_*) and announcements (play_ambient's Ambients
    // index) made since the interface last played and cleared them, up to 8
    // of each.
    uint16_t mishaps = 0;
    bool machine_exploded = false;
    std::vector<uint8_t> machine_sounds;
    std::vector<uint8_t> machine_announcements;

    // Random disasters (disaster.cpp)
    // Hospital 0's disaster (allhosps[0]'s disaster fields): the kind running
    // (0 none, 1 heating cold, 2 heating hot, 3 sickness), the days counted
    // since the last one (drawn from world.seed once past
    // gbv.disaster_launch), the days left of the one running, the
    // heating level it saved before breaking it, and whether the heating is
    // broken (so the player's set heating is ignored, src/io.cpp's
    // EV_SET_HEATING).
    uint16_t disaster_counter = 0;
    uint8_t disaster_kind = 0;
    int8_t disaster_timer = 0;
    uint8_t saved_heating = 0;
    bool heating_broken = false;

    // Epidemics (epidemic.cpp)
    // Hospital 0's epidemic (allhosps[0]'s epidemic fields): the illness it
    // is of and the fine declaring it would cost, how many patients the last
    // statistics pass found contagious (contagious_count), the cover-up's
    // timer in half days (epidemic_timer, 0 for none) and whether one is
    // covered up (epidemic_active) or over (epidemic_over), the illness the
    // inspector judged (inspected_illness) and its verdict (inspection[0]),
    // whether an inspector is on the way (inspection[1]), and the patients
    // queued for vaccination (vaccine_queue, the head) with how many asked
    // (vaccine_requests).
    struct Epidemic {
        uint8_t illness = 0;
        uint16_t fine = 0;
        uint16_t contagious_count = 0;
        // The patients the epidemic spread to, less those cleaned
        // (othervar.contagious_count), which the advisor's contagion advice
        // reads.
        uint16_t spread_count = 0;
        int16_t timer = 0;
        uint8_t active = 0;
        uint8_t over = 0;
        uint8_t inspected_illness = 0;
        std::array<uint8_t, 2> inspection{};
        uint16_t vaccine_queue = 0;
        uint16_t vaccine_requests = 0;
        // An epidemic call ran out unanswered (expire_playercalls), to be
        // covered up as the game's default answer once the turn's admin is
        // done (Game::start_turn).
        bool cover_up_expired = false;
    };
    Epidemic epidemic;
    // The epidemic's sound effects (SFX_*) made since the interface last
    // played and cleared them.
    std::vector<uint8_t> epidemic_sounds;

    // The treatments' sound effects (SFX_*) made since the interface last
    // played and cleared them: the cheer of a cure and the boo of a death
    // (administer_drugs and setup_next_treatment).
    std::vector<uint8_t> treatment_sounds;

    // The player's shots at rats' sound effects (SFX_*) made since the
    // interface last played and cleared them (shoot_rat).
    std::vector<uint8_t> rat_sounds;

    // Research (research.cpp)
    // Hospital 0's research and object figures (core/world/research.h).
    Research research;
    // The advisor's announcements waiting, by kind (the game's
    // gamevar.announce, ANN_* kinds): up to four values each, oldest first,
    // 0 past the last (announce_this).
    std::array<std::array<uint32_t, 4>, kAnnounceKinds> announce{};
    // The computer players' hospitals (core/world/computer.h), by owner;
    // [0] is the player's and unused.
    std::array<RivalHospital, kHospitalCount> rivals{};
    // What rooms in use read and change outside the world, set by
    // start_rooms_in_use (core/world/rooms.h): the player's policy, prices'
    // statistics and insurance claims, the accounts, the balance, the date
    // and the action sequences.
    struct HospitalSettings *hospital = nullptr;
    Finance *ledger = nullptr;
    int *balance = nullptr;
    // The game, for the nested process_things that re-checks the win status
    // (complex.cpp's action 50), set by Game::connect_world.
    Game *game = nullptr;
    const struct Date *date = nullptr;
    const struct PersonComplexes *person_complexes = nullptr;
    const struct StaffComplexes *staff_complexes = nullptr;
};

// Advances `seed` as the game's old_newrand (src/rand.cpp) and returns its
// new value. `caller` names the drawing function in the event trace
// (core/world/trace.h); a helper that draws for its caller passes its own.
uint32_t old_newrand(uint32_t &seed, std::source_location caller = std::source_location::current());
// As old_newrand, traced as drawn by the game's function `caller`, for a
// port named otherwise.
uint32_t old_newrand(uint32_t &seed, const char *caller);

// Resets `world` and builds a level's starting map with its objects, as the
// game's recreate_map (src/town.cpp) when a level starts: build_start_map,
// with the object a cell holds (all of player 0) placed as soon as the cell
// is copied (adjust_mapwho, place_object).
// So a cell's copy overwrites what objects on earlier cells marked on it
// (flags, invisible walls, a second part on its list), as in the game.
// Shadows are not computed. The random generator starts from `seed`.
void start_level(World &world, const Level &level, const AnimData &anims, uint32_t seed = kLevelStartSeed);

// Empties the rooms table (setup_new_level's clearing of room_info) and
// makes hospital 0's entrances as load_level (src/io.cpp) does when a level
// starts: a kRoomEntrance room of owner 0, state 0, for each of
// Level::entrances[0] in order, in the first free entries (so rooms 1-4),
// with pos, door_tile and door_inside_tile the entrance's cell; then sets
// every hospital's World::entrances to those rooms. The other hospitals,
// which no one plays, get no rooms of their own, as in a one-player game.
void start_entrance_rooms(World &world, const Level &level);

// get_room_info (0x19fe0 in src/utils.cpp): hospital `owner`'s first room of
// type `kind` in state 0; failing that, with `mode` 0x65 the first of the
// kind in any state, with 0x64 the last. Returns 0 (the unused room 0, whose
// fields are all 0) if there is none.
uint16_t get_room_info(const World &world, uint8_t kind, uint8_t mode, uint8_t owner);

// get_room_info_rand (0x19ef8 in src/utils.cpp): one of hospital `owner`'s
// rooms of type `kind` (in state 0 unless `mode` is 0x65; among the first 30
// found) drawn from world.seed; 0 if there is none, without a draw.
uint16_t get_room_info_rand(World &world, uint8_t kind, uint8_t mode, uint8_t owner);

// get_nearest_room_info (0x1a0a0 in src/utils.cpp): the room of type `kind`
// of `person`'s hospital in use (state 0 or 1) whose door cell is nearest
// (by the squares of the cells across and down, in 16 bits) to the
// person's cell, if nearer than 9999; 0 if none.
uint16_t get_nearest_room_info(const World &world, const Thing &person, uint8_t kind);

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
// them. With `cleanse` (as place_existing_object), the litter on the cells
// it covers goes (cleanse_area: remove_litter, for the hospital whose land
// each is, and the rat holes under it).
void mark_object_shape(World &world, int tile, int kind, int dir, bool cleanse = false);
void remove_object_shape(World &world, int tile, int kind, int dir);

// place_object (src/objects.cpp) without its checks of the hospital's
// access, of people in the way and of picked-up objects, and without
// charging: a new `object` for `owner`, and its second part where that
// fits (a picked-up one of its kind put back, else one bought at
// object_stats' cost), linked into the map and World::first_of_kind
// (player 0's). Unless
// `force`, refuses with 0 when it does not fit (can_place_object). Returns
// its thing id, 0 if refused or the things table is full. Draws two random
// numbers per object made.
uint16_t place_object(World &world, const AnimData &anims, ObjectAt object, int owner, bool force);

// compute_plant_frame (src/handyman.cpp): the frame of `plant` at its
// plant_health, the healthy one aged a stage for each 80 points lost (at
// most four).
uint16_t compute_plant_frame(const Thing &plant, const AnimData &anims);

// inc_hardware_stats' and dec_hardware_stats' hospital value part
// (src/objects.cpp) for hospital 0: a machine, diagnosis machine or leisure
// object is worth its object_stats cost, added as one is placed (`sign` 1) and
// taken away as one is removed (-1). Nothing before the ledger is connected
// (at a level's start, where the cost is 0 in the game too).
void add_object_value(World &world, int kind, int sign);

// The game's NUMFX_NUMBER and NUMFX_MONEY and the colours setup_colors finds
// (LbPaletteFindColour) for color.black, color.red and color.dark_blue in the
// hospital palette and in the fax picture's (World::fax_palette).
constexpr uint8_t kNumfxNumber = 0;
constexpr uint8_t kNumfxMoney = 3;
constexpr uint8_t kColourBlack = 0;
constexpr uint8_t kColourRed = 12;
constexpr uint8_t kColourDarkBlue = 245;
constexpr uint8_t kFaxColourRed = 243;
constexpr uint8_t kFaxColourDarkBlue = 16;

// color.red and color.dark_blue now (color.black is 0 in both palettes).
inline uint8_t colour_red(const World &world)
{
    return world.fax_palette ? kFaxColourRed : kColourRed;
}
inline uint8_t colour_dark_blue(const World &world)
{
    return world.fax_palette ? kFaxColourDarkBlue : kColourDarkBlue;
}

// create_numfx (0x9d480): a floating number of `type` showing `value` on
// `tile` in palette colour `colour`, drawn `height` pixels up plus 0-15 at
// random: a thing (kThingNumfx) linked on the cell, unless no thing but the
// last 50 is free. Draws three random numbers when made.
void create_numfx(World &world, uint8_t type, int16_t value, uint16_t tile, uint8_t colour, uint8_t height);

// numfx_ages (0x9d560): one turn of floating number `id`; removed
// (remove_thing) when its lifetime runs out, a money one's twice as fast.
void numfx_ages(World &world, uint16_t id);

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
