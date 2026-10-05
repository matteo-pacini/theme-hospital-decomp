#pragma once

#include "core/data/anims.h"
#include "core/world/world.h"

#include <array>
#include <cstdint>

namespace th
{

// People's path finding and walking, as the game's nav.cpp, orthonav.cpp,
// wallhug.cpp, node.cpp and the walking step of goto.cpp, with their limits.
//
// Directions are the game's: 0 -y (north-east on screen), 1 +x, 2 +y, 3 -x.

// Per direction, the cell step (the game's around_xy).
constexpr std::array<std::array<int8_t, 2>, 4> kAroundXy = {{{0, -1}, {1, 0}, {0, 1}, {-1, 0}}};

// The animations a person's kind walks and stands with: start indices of
// AnimData::starts, to which the facing is added; 9999 for none. Copied
// from the game's anim_starts and staff_anim_starts (src/anitable.cpp):
// anims[0] (walk), anims[5] (staff standing) and anims[9] (patients standing).
struct PersonAnims {
    uint16_t walk;
    uint16_t stand;
};
extern const std::array<PersonAnims, 15> kPatientAnims;
extern const std::array<PersonAnims, 5> kStaffAnims;

// wallhug_trivial's and wallhug_tricky's result when no route is found.
constexpr uint32_t kWallhugFailed = 0x70000000;

// A route of the wall-hugging path finder (the game's wallhug_path).
struct WallhugPath {
    // Waypoints in route.
    uint8_t count = 0;
    Waypoint start;
    Waypoint end;
    // On success, the corners from start to end, the last being end.
    std::array<Waypoint, 0xfd> route{};
};

// wall_in_way (0x43ff0): whether a step from cell (x, y) in direction `dir`
// is blocked by the map's edge, a wall between the cells, or either cell
// being impassable (an object's footprint: MapCell.flags bit 0 without bit
// 1). A `dir` of 4 or more is a step that stays on (x, y). x and y are
// unsigned in the game, so -1 passes its edge tests; the cell indices it
// then reads ((int16_t)x + ((int16_t)y << 7)) can fall before the map, where
// the game reads whatever is in memory there: here such a cell reads as
// empty.
bool wall_in_way(const World &world, int32_t x, int32_t y, uint32_t dir);

// line_of_sight (0x48abc): whether the stepped line (bresenham) from `from`
// to `to` crosses nothing wall_in_way blocks.
bool line_of_sight(const World &world, Waypoint from, Waypoint to);

// wallhug_trivial (0x491d8): finds a route from path.start to path.end by
// walking the stepped line and, at each wall, sending a left- and a
// right-handed wall follower round it until one is back on the line,
// unsmoothed. Returns the steps taken, or kWallhugFailed. It gives up at the
// game's limits even when a route exists: 60000 steps in all, 252 corners
// for either follower, or 250 waypoints (docs/BUGS.md). On failure the route
// holds what was found, ending with the cell reached.
uint32_t wallhug_trivial(const World &world, WallhugPath &path);

// wallhug_tricky (0x48e6c): wallhug_trivial, then the game's smoothing
// (wallhug_cleanup, 0x48c78: waypoints skipped where a line of sight allows,
// and detours shortened, for up to ten rounds). Returns as wallhug_trivial.
uint32_t wallhug_tricky(const World &world, WallhugPath &path);

// orthonav_init (0x55454): plans `path` from (x0, y0) to (x1, y1) with
// wallhug_tricky and keeps its first four waypoints. If no route is found,
// count is 0 and the rest of `path`, the destination included, is left as
// it was (docs/BUGS.md).
void orthonav_init(const World &world, OrthonavPath &path, int x0, int y0, int x1, int y1);

// free_orthog_line (0x55568): whether the row or column from (x1, y1) to
// (x2, y2) (which share x or y) crosses nothing wall_in_way blocks.
bool free_orthog_line(const World &world, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

// orthonav_whatnext's results besides a direction 0-3.
constexpr int kNavArrived = 4;
constexpr int kNavBlockedByPeople = -1;
constexpr int kNavWalledIn = -2;
constexpr int kNavNoPath = -3;

// orthonav_whatnext (0x56028): the direction of the next step from cell
// (x, y) along `path` for the person `walker` facing `dir`, sidestepping
// walls and the patients and staff in the way (person_in_way,
// person_overlap): 0-3, or kNavArrived, kNavBlockedByPeople, kNavWalledIn
// or kNavNoPath (no route was found). Reached waypoints are dropped. A
// detour, or being blocked by people or walls, sets path.x1 to 0xff so the
// path is planned again next time. Draws from world.seed as the game does.
int orthonav_whatnext(World &world, uint16_t walker, OrthonavPath &path, int x, int y, int dir);

// nav_init (0x444e0): plans the person's path from its cell to (dest_x,
// dest_y).
void nav_init(const World &world, Thing &person);

// person_head_for (0x44210): the person's next step towards (dest_x,
// dest_y), as orthonav_whatnext, planning the path first when it has no
// waypoints and again when the destination is not the path's.
int person_head_for(World &world, uint16_t person);

// set_destination_coords (0x4195c): sends the person to `tile` (column + row
// * 128), so it decides its first step on its next walk_turn.
void set_destination_coords(World &world, uint16_t person, int tile);

// insert_node (0x15a20): links `id` at the head of the list of things of
// cell (x, y). It must be in no list; its position is not set.
void insert_node(World &world, uint16_t id, MapPos x, MapPos y);

// remove_node (0x15b68): unlinks `id` from the list of its cell.
void remove_node(World &world, uint16_t id);

// move_node (0x15ac4): moves `id` to (x, y), moving it to the new cell's
// list when the cell changes. Returns true if it did; nothing moves if (x,
// y) is off the map.
bool move_node(World &world, uint16_t id, MapPos x, MapPos y);

// Sets the frame of a person to the start of animation `anim` (a
// PersonAnims entry) for its facing, as the game's PLAY_FRAME (goto.cpp):
// 9999 (check_frame) plays animation 0.
void play_frame(Thing &person, const AnimData &anims, uint16_t anim);

// What a turn of walk_turn came to.
enum class WalkResult {
    // Still on the way (it stepped this turn).
    Walking,
    // At the destination (orthonav_whatnext's kNavArrived); countdown is 0,
    // so a further walk_turn decides again at once.
    Arrived,
    // People in the way (kNavBlockedByPeople): it stands this turn,
    // showing its standing frame, and decides again next turn.
    BlockedByPeople,
    // Walled in (kNavWalledIn) or no route (kNavNoPath): the game's callers
    // then call person_nav_failure (or, for some, take another course);
    // nothing has moved.
    WalledIn,
    NoPath,
};

// One game turn of a walking patient or member of staff (kThingPatient or
// kThingStaff, with nonzero speed and anim_speed), as the game's
// person_goto_destination (0x763d0) and staff_goto_object (0x77508) without
// what is not walking (blueprints, needs, reactions, the complex): its
// animation advances when `game_turn` is a multiple of anim_speed; when its
// countdown runs out it decides its step (person_head_for) and waits 256 /
// speed turns for the next; turning to a new direction, or setting off from
// its standing frame, starts the walk animation for it. It then moves speed
// / 256 cell in its facing (move_node).
// The game's other walkers differ in the frames they show: the wandering
// staff, the grim reaper and the mayor restart the walk only on turning.
WalkResult walk_turn(World &world, const AnimData &anims, uint16_t person, uint32_t game_turn);

} // namespace th
