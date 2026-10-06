#pragma once

#include "core/world/object_tables.h"

#include <array>
#include <cstdint>
#include <vector>

namespace th
{

class Game;
struct AnimData;
struct Event;
struct ObjectPlacing;
struct World;

// Thing.type of a room's door (THING_DOOR); the second half of a double door
// becomes an object (kThingObject).
constexpr uint8_t kThingDoor = 8;

// RoomInfo.state values (RS_*): a room being built, and one being furnished
// or edited.
constexpr int8_t kRoomStateBlueprint = 3;
constexpr int8_t kRoomStateEdited = 4;

// Room types (ROOM_*).
constexpr int kRoomTypes = 31;

// The game's rooms[] (RoomControl in include/hosp.h; src/tables.cpp), by
// room type, without the fields only staff and patients read. The price is
// the level's (Balance::rooms, key rooms.Cost).
struct RoomTypeInfo {
    uint16_t name;      // room_names index (language section 13)
    uint8_t wall_style; // recolour_all_rooms's wall colours: 3-6, else none
    uint8_t main_object;
    // objects[0..compulsory_count) must be placed; the rest are not used.
    uint8_t compulsory_count;
    std::array<uint8_t, 10> objects;
    // What the buy objects requester offers while furnishing the room.
    uint8_t buyable_count;
    std::array<uint8_t, 10> buyable;
    uint8_t floor_block; // Map.floor_block of the finished room's cells
    uint8_t door_kind;   // 0 a double door, 1 a single door
    uint8_t min_width;
    uint8_t min_height;
};

inline constexpr std::array<RoomTypeInfo, kRoomTypes> kRoomTypeInfo = {{
    {0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0},             // 0
    {0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0},             // 1
    {0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0},             // 2
    {28, 1, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 7, {11, 4, 7, 43, 44, 45, 17, 0, 0, 0}, 0, 0, 0, 0},       // 3
    {1, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0},             // 4
    {0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0},             // 5
    {3, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 4, 4},             // 6
    {4, 3, 1, 3, {1, 6, 2, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 17, 1, 4, 4},        // 7
    {5, 3, 18, 3, {18, 16, 61, 0, 0, 0, 0, 0, 0, 0}, 6, {43, 44, 45, 50, 56, 60, 0, 0, 0, 0}, 17, 1, 5, 5},  // 8
    {6, 3, 8, 2, {8, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 5, {43, 44, 45, 50, 8, 0, 0, 0, 0, 0}, 20, 0, 6, 6},        // 9
    {7, 3, 30, 4, {30, 33, 29, 35, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 20, 0, 6, 6},   // 10
    {8, 3, 39, 1, {39, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 4, 4},      // 11
    {9, 4, 13, 2, {13, 16, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 4, 4},     // 12
    {10, 4, 14, 3, {14, 15, 16, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 5, 5},   // 13
    {11, 4, 22, 1, {22, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 4, 4},     // 14
    {12, 4, 42, 1, {42, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 4, 4},     // 15
    {13, 4, 27, 2, {27, 28, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 6, 6},    // 16
    {14, 5, 9, 1, {9, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 4, 4},       // 17
    {15, 5, 23, 2, {23, 15, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 0, 5, 5},    // 18
    {16, 5, 25, 2, {25, 15, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 4, 4},    // 19
    {17, 5, 26, 1, {26, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 4, 4},     // 20
    {18, 5, 24, 1, {24, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 4, 4},     // 21
    {19, 6, 37, 2, {37, 36, 0, 0, 0, 0, 0, 0, 0, 0}, 7, {43, 44, 45, 50, 36, 56, 60, 0, 0, 0}, 16, 1, 4, 4}, // 22
    {20, 5, 46, 2, {46, 15, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 5, 5},    // 23
    {21, 5, 47, 1, {47, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 16, 1, 4, 4},     // 24
    {22, 6, 19, 1, {19, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 7, {43, 44, 45, 19, 10, 57, 21, 0, 0, 0}, 16, 1, 4, 4},  // 25
    {23, 6, 21, 3, {21, 5, 4, 0, 0, 0, 0, 0, 0, 0}, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 20, 1, 4, 4},         // 26
    {24, 6, 20, 2, {20, 16, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 20, 1, 5, 5},    // 27
    {25, 6, 55, 3, {55, 1, 2, 0, 0, 0, 0, 0, 0, 0}, 8, {43, 44, 45, 50, 40, 1, 2, 41, 0, 0}, 20, 1, 5, 5},   // 28
    {26, 6, 51, 2, {51, 32, 0, 0, 0, 0, 0, 0, 0, 0}, 6, {43, 44, 45, 50, 51, 32, 0, 0, 0, 0}, 20, 1, 4, 4},  // 29
    {27, 5, 54, 2, {54, 15, 0, 0, 0, 0, 0, 0, 0, 0}, 4, {43, 44, 45, 50, 0, 0, 0, 0, 0, 0}, 18, 1, 5, 5},    // 30
}};

// The game's departments[] (DepartmentControl; src/tables.cpp): the build
// room requester's departments, from 3 (diagnosis), 4 (treatment), 5
// (clinics) and 6 (facilities), and their room types. Only listed ones are
// shown.
struct DepartmentInfo {
    uint8_t count;
    uint8_t listed;
    std::array<uint8_t, 10> rooms;
};

inline constexpr std::array<DepartmentInfo, 7> kDepartments = {{
    {0, 0, {}},
    {0, 0, {}},
    {0, 0, {}},
    {9, 1, {7, 27, 12, 13, 14, 15, 16, 8, 9, 0}},
    {4, 1, {8, 9, 10, 11, 0, 0, 0, 0, 0, 0}},
    {8, 1, {17, 18, 19, 20, 21, 23, 24, 30, 0, 0}},
    {4, 1, {25, 28, 29, 22, 0, 0, 0, 0, 0, 0}},
}};

// The game's blueprint[] (Blueprint in include/hosp.h; src/tables.cpp): the
// markers ("small things") a blueprint puts on the map, and what each one
// builds. A marker is kept negated at the end of its cell's thing list.
struct BlueprintPiece {
    uint16_t anim;       // anim start of its picture
    uint16_t kind;       // the wall block, door kind or object kind it builds
    uint8_t build_type;  // how build_all_rooms builds it; doors: facing + 1
    uint8_t flags;       // bit 0 the cell's north edge, bit 1 its west edge
    uint8_t wall_layer;  // 0 wall_n, 1 wall_w, for build type 10
    uint8_t object_kind; // the door it shows (OBJ_DOOR, OBJ_SWING_DOOR...)
};

inline constexpr std::array<BlueprintPiece, 107> kBlueprints = {{
    {0, 0, 0, 0, 0, 0},      {60, 80, 0, 1, 0, 0},    {61, 80, 0, 2, 0, 0},     {62, 80, 0, 3, 0, 0},
    {63, 3, 1, 0, 1, 3},     {64, 3, 2, 0, 1, 3},     {63, 3, 3, 0, 1, 3},      {64, 3, 4, 0, 1, 3},
    {65, 126, 5, 1, 1, 0},   {66, 126, 5, 2, 1, 0},   {67, 3, 5, 3, 1, 3},      {68, 3, 5, 3, 1, 3},
    {69, 126, 5, 3, 1, 0},   {70, 126, 5, 3, 1, 0},   {273, 153, 10, 1, 0, 0},  {271, 152, 10, 2, 1, 0},
    {272, 151, 10, 4, 0, 0}, {270, 154, 10, 5, 1, 0}, {274, 155, 10, 3, 0, 0},  {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},      {0, 0, 0, 0, 0, 0},      {0, 0, 0, 0, 0, 0},       {65, 126, 5, 1, 1, 0},
    {66, 126, 5, 2, 1, 0},   {67, 3, 15, 3, 1, 3},    {68, 3, 15, 3, 1, 3},     {69, 126, 15, 3, 1, 0},
    {70, 126, 15, 3, 1, 0},  {275, 157, 10, 6, 0, 0}, {277, 9999, 10, 7, 0, 0}, {276, 158, 10, 8, 0, 0},
    {0, 11, 15, 0, 1, 11},   {0, 11, 16, 0, 1, 11},   {0, 11, 17, 0, 1, 11},    {0, 11, 18, 0, 1, 11},
    {63, 53, 1, 0, 1, 53},   {64, 53, 2, 0, 1, 53},   {63, 53, 3, 0, 1, 53},    {64, 53, 4, 0, 1, 53},
    {89, 0, 60, 0, 0, 0},    {90, 0, 60, 0, 0, 0},    {975, 0, 60, 1, 0, 0},    {976, 0, 60, 1, 0, 0},
    {135, 0, 60, 0, 0, 0},   {136, 0, 60, 0, 0, 0},   {137, 0, 60, 0, 0, 0},    {138, 0, 60, 0, 0, 0},
    {139, 0, 60, 0, 0, 0},   {140, 0, 60, 0, 0, 0},   {141, 0, 60, 0, 0, 0},    {142, 0, 60, 0, 0, 0},
    {222, 0, 60, 1, 0, 0},   {223, 0, 60, 1, 0, 0},   {224, 0, 60, 0, 0, 0},    {225, 0, 60, 0, 0, 0},
    {226, 0, 60, 0, 0, 0},   {227, 0, 60, 0, 0, 0},   {228, 0, 60, 0, 0, 0},    {380, 0, 60, 0, 0, 0},
    {381, 0, 60, 0, 0, 0},   {375, 0, 60, 0, 0, 0},   {376, 0, 60, 0, 0, 0},    {63, 52, 1, 0, 1, 52},
    {64, 52, 2, 0, 1, 52},   {63, 52, 3, 0, 1, 52},   {64, 52, 4, 0, 1, 52},    {63, 199, 21, 1, 0, 52},
    {64, 101, 22, 2, 0, 52}, {63, 63, 23, 0, 0, 52},  {64, 64, 24, 0, 0, 52},   {63, 65, 25, 0, 0, 52},
    {64, 66, 26, 0, 0, 52},  {1650, 0, 60, 0, 0, 0},  {1651, 0, 60, 0, 0, 0},   {1652, 0, 60, 0, 0, 0},
    {1653, 0, 60, 0, 0, 0},  {1654, 0, 60, 0, 0, 0},  {1655, 0, 60, 0, 0, 0},   {1656, 0, 60, 0, 0, 0},
    {1657, 0, 60, 0, 0, 0},  {1658, 0, 60, 0, 0, 0},  {1659, 0, 60, 0, 0, 0},   {1681, 0, 60, 0, 0, 0},
    {1682, 0, 60, 0, 0, 0},  {1686, 0, 60, 0, 0, 0},  {1687, 0, 60, 0, 0, 0},   {2558, 0, 60, 0, 0, 0},
    {2559, 0, 60, 0, 0, 0},  {1690, 0, 60, 0, 0, 0},  {1691, 0, 60, 0, 0, 0},   {1692, 0, 60, 0, 0, 0},
    {1693, 0, 60, 0, 0, 0},  {1694, 0, 60, 0, 0, 0},  {1695, 0, 60, 0, 0, 0},   {1696, 0, 60, 0, 0, 0},
    {1697, 0, 60, 0, 0, 0},  {1698, 0, 60, 0, 0, 0},  {1699, 0, 60, 0, 0, 0},   {1700, 0, 60, 0, 0, 0},
    {1701, 0, 60, 0, 0, 0},  {1705, 0, 60, 0, 0, 0},  {1704, 0, 60, 0, 0, 0},   {1707, 0, 60, 0, 0, 0},
    {1706, 0, 60, 0, 0, 0},  {1708, 0, 60, 0, 0, 0},  {0, 0, 255, 0, 0, 0},
}};

// The game's doortype[] (DoorInfo in include/hosp.h; src/tables.cpp): the
// pieces of a door or window, four entries (one per wall: north, south,
// west, east) per kind: 0-3 double doors, 4-7 single doors, 8-11 windows,
// 20-23 plain wall (taking a window out).
struct DoorType {
    uint8_t count;                       // cells covered
    std::array<int16_t, 3> tiles;        // their offsets from the door's cell
    uint16_t cursor_anim;                // anim start drawn while placing it
    std::array<uint16_t, 3> pieces;      // kBlueprints index of each piece
    int16_t door_offset;                 // RoomInfo.door_tile from the door's cell
    int16_t door_inside_offset;          // RoomInfo.door_inside_tile from it
    std::array<uint8_t, 3> floor_blocks; // set on its cells' floors (less `offset`)
    int16_t offset;                      // subtracted for the floor and preview cells
    uint8_t is_entrance;                 // a door: sets the room's door tiles
};

inline constexpr std::array<DoorType, 24> kDoorTypes = {{
    {3, {0, 1, 2}, 63, {69, 36, 67}, -127, 1, {24, 23, 25}, 0, 1},      // 0
    {3, {0, 1, 2}, 63, {71, 38, 67}, 1, -127, {28, 23, 29}, 128, 1},    // 1
    {3, {0, 128, 256}, 64, {72, 39, 68}, 127, 128, {31, 23, 30}, 0, 1}, // 2
    {3, {0, 128, 256}, 64, {70, 37, 68}, 128, 127, {27, 23, 26}, 1, 1}, // 3
    {1, {0, 1, 0}, 63, {4, 4, 0}, -128, 0, {25, 0, 0}, 0, 1},           // 4
    {1, {0, 1, 0}, 63, {6, 6, 0}, 0, -128, {29, 0, 0}, 128, 1},         // 5
    {1, {0, 128, 0}, 64, {7, 7, 0}, -1, 0, {30, 0, 0}, 0, 1},           // 6
    {1, {0, 128, 0}, 64, {5, 5, 0}, 0, -1, {26, 0, 0}, 1, 1},           // 7
    {1, {0, 1, 0}, 65, {8, 8, 0}, -128, 0, {32, 0, 0}, 0, 0},           // 8
    {1, {0, 1, 0}, 65, {8, 8, 0}, 0, -128, {34, 0, 0}, 128, 0},         // 9
    {1, {0, 128, 0}, 66, {9, 9, 0}, -1, 0, {35, 0, 0}, 0, 0},           // 10
    {1, {0, 128, 0}, 66, {9, 9, 0}, 0, -1, {33, 0, 0}, 1, 0},           // 11
    {1, {0, 1, 0}, 278, {19, 19, 0}, -128, 128, {25, 0, 0}, 0, 1},      // 12
    {1, {0, 1, 0}, 280, {21, 21, 0}, 128, -128, {29, 0, 0}, 128, 1},    // 13
    {1, {0, 128, 0}, 281, {22, 22, 0}, -1, 1, {30, 0, 0}, 0, 1},        // 14
    {1, {0, 128, 0}, 279, {20, 20, 0}, 1, -1, {26, 0, 0}, 1, 1},        // 15
    {1, {0, 1, 0}, 0, {32, 32, 0}, -128, 0, {32, 0, 0}, 0, 0},          // 16
    {1, {0, 1, 0}, 0, {34, 34, 0}, 0, -128, {34, 0, 0}, 128, 0},        // 17
    {1, {0, 128, 0}, 0, {35, 35, 0}, -1, 0, {35, 0, 0}, 0, 0},          // 18
    {1, {0, 128, 0}, 0, {33, 33, 0}, 0, -1, {33, 0, 0}, 1, 0},          // 19
    {1, {0, 1, 0}, 60, {1, 1, 0}, -128, 0, {23, 0, 0}, 0, 0},           // 20
    {1, {0, 1, 0}, 60, {1, 1, 0}, 0, -128, {23, 0, 0}, 128, 0},         // 21
    {1, {0, 128, 0}, 61, {2, 2, 0}, -1, 0, {23, 0, 0}, 0, 0},           // 22
    {1, {0, 128, 0}, 61, {2, 2, 0}, 0, -1, {23, 0, 0}, 1, 0},           // 23
}};

// What dragging the blueprint does (BuildState::drag).
enum BlueprintDrag : uint8_t { kDragNone = 0, kDragCorner = 1, kDragEdge = 2, kDragMove = 3 };

// The game's BuildState (include/hosp.h; allhosps[player].build): the room
// the player is building, its blueprint and the door being placed.
struct BuildState {
    // The blueprint's corners, inclusive cells.
    uint8_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    // The corner held fixed while a corner or edge is dragged, or the cell
    // grabbed while the whole blueprint is moved; for an edge, `pivot` and
    // `anchor` are the ends of the opposite edge.
    uint8_t anchor_x = 0, anchor_y = 0;
    uint8_t pivot_x = 0, pivot_y = 0;
    // The room built or furnished (a World::rooms index; also
    // allhosps.edited_room), 0 for none.
    uint16_t room = 0;
    uint8_t drag = kDragNone;
    uint8_t room_type = 0;
    // 1 while the blueprint is laid out (drawn every frame), else 0.
    uint8_t draw_mode = 0;
    // The door being placed: its cell and kDoorTypes index, as the last
    // EV_TEMP_PLACE_DOOR found them.
    uint16_t door_tile = 0;
    uint8_t door_type = 0;
    // The kind of door being placed (RoomTypeInfo::door_kind), 2 (a window)
    // once the door is placed.
    uint8_t door_kind = 0;
    // Objects lifted off the map while the room went back to a blueprint,
    // to be placed again (no charge) or refunded; 0 for none.
    std::array<uint16_t, 20> objects{};
    // 1 while doors and windows are placed, 2 while waiting for the room to
    // empty before it is built.
    uint8_t state = 0;
    // The floor put back when the blueprint is taken off the map.
    uint8_t saved_floor_block = 0;
};

// The game's mouse modes of building (gamevar.mouse_mode): MM_BLUEPRINT,
// MM_PLACE_DOOR and 0x10 (waiting for the room to empty).
enum class BuildMouse : uint8_t { None, Blueprint, PlaceDoor, WaitClear };

// The player's building state: the game's BuildState and the parts of
// gamevar and othervar the build commands set.
struct Building {
    BuildState build;
    BuildMouse mouse = BuildMouse::None;
    // The door previewed can not go there (othervar.door_invalid[player]).
    bool door_invalid = false;
    // The last EV_TEMP_PLACE_DOOR found a place for the door, and which of
    // its pieces it found invalid (temp_place_door draws them red).
    bool door_shown = false;
    std::array<bool, 3> door_piece_invalid{};
    // The pointer the blueprint shows (PTR_RESIZE_ROOM and on).
    uint8_t pointer = 0;
    // The list of objects to place before the buy objects requester opened
    // on the room (othervar.saved_build_palette), and the compulsory ones
    // taken off it (othervar.shop_palette_adjust).
    std::array<uint8_t, kObjectKinds> saved_buy_count{};
    std::array<uint8_t, kObjectKinds> shop_adjust{};
    // The type of the last room opened (othervar.last_built_room).
    uint8_t last_built_room = 0;
    // Sound effects (SFX_*) the commands played, for the interface to play
    // and clear.
    std::vector<uint8_t> sounds;
};

// Carries out the build tool's commands for the local player, as
// process_turn (src/io.cpp): EV_OPEN_BUILD_ROOM, EV_REQ1_RESET_ROOMS,
// EV_CLOSE_REQUESTERS, EV_INIT_BLUEPRINT, EV_MODIFY_BLUEPRINT,
// EV_HOVER_BLUEPRINT, EV_COMMIT_BLUEPRINT, EV_UNCOMMIT_BLUEPRINT,
// EV_TEMP_PLACE_DOOR, EV_PLACE_BP_DOOR, EV_COMMIT_DOORS, EV_OPEN_ROOM,
// EV_EDIT_BLUEPRINT, EV_DELETE_ROOM (which goes on as EV_CANCEL_BUILD, run
// here through apply_staff_event and apply_object_event),
// EV_FURNISH_EDITED_ROOM, EV_BACK_TO_PLACE_OBJECT, and EV_PLACE_OBJECT while
// furnishing a room.
// Returns false for other commands; for EV_CANCEL_BUILD, whose part for
// the lifted objects and the mouse mode it carries out but which has parts
// for staff and objects too; and for EV_OPEN_HIRE_STAFF and
// EV_OPEN_FURNISH_CORRIDOR, for which it only closes the build room
// requester (cancel_top_level_requesters).
bool apply_build_event(Game &game, const Event &event);

// process_blueprint_zones (src/blueprnt.cpp), after each turn's things:
// builds the room waiting for its floor to empty (EV_COMMIT_DOORS) once it
// has.
void build_turn(Game &game);

// which_room_number (src/build.cpp): the room (World::rooms index) whose
// floor holds `tile`, 0 for none.
uint16_t room_at(const World &world, uint16_t tile);

// check_room_access (src/blueprnt.cpp): whether the door of each of
// `owner`'s rooms but its entrances and receptions, built or furnished, and
// both sides of each of its reception desks, can be reached from its first
// entrance, each room from the one before.
bool check_room_access(const World &world, int owner);

// find_small_thing (src/objects.cpp): the blueprint marker (kBlueprints
// index) at the end of `tile`'s thing list, 0 for none.
uint16_t find_small_thing(const World &world, int tile);

// draw_blueprint_area's tests (src/blueprnt.cpp) for the player's blueprint:
// whether floor cell (x, y) is drawn as invalid, and whether the outline
// piece on cell (x, y) (corners from (x0, y0) to (x1 + 1, y1 + 1)) is.
bool blueprint_floor_invalid(const World &world, int x, int y);
bool blueprint_outline_invalid(const World &world, int x, int y);

// The game's object placing in a room (src/build.cpp), for update_placement
// (core/world/objects.h): place_object_restrict_to_room moves `pos` so that
// an object of `kind` facing `dir` tested at pos + offset lies inside room
// p.room; place_object_wall_hug puts an object of `kind` against the room's
// wall nearest `pos`, facing p.dir set to the wall's angle (with a swoosh
// when it turns).
int place_object_restrict_to_room(const World &world, const ObjectPlacing &p, int pos, int offset, int kind, int dir);
int place_object_wall_hug(World &world, ObjectPlacing &p, int pos, int kind);

// place_object_nav_check (src/build.cpp): whether an object of `kind` at
// `pos` facing `dir` (and its second part at `pos2`, unless 0) in room
// `room` leaves each object of the room usable from its door.
bool place_object_nav_check(World &world, uint16_t room, int pos, int kind, int dir, int pos2, int kind2, int dir2);

// corridor_object_room_nav_check (src/build.cpp): place_object_nav_check
// for an object moved into room `room`, and the people in it still able to
// reach its door.
bool corridor_object_room_nav_check(World &world, int pos, int kind, int dir, uint16_t room);

// does_room_contain_compulsory_objects (src/build.cpp).
bool room_has_compulsory_objects(const World &world, uint16_t room);

// set_edit_room_requester_mode (src/req14.cpp) for a room's modes: 0
// blueprint, 2 placing the door, 4 windows, 6 placing objects (confirm
// enabled once the room has its compulsory objects), 8 picking them up.
void set_room_requester_mode(World &world, int mode);

// place_existing_object (src/objects.cpp; defined in core/world/world.cpp):
// puts object `id`, off the map, on `tile` facing `dir`, linked into the
// map, the room holding `tile` and World::first_of_kind, with its second
// part placed by place_object. Does not check that it fits.
void place_existing_object(World &world, const AnimData &anims, uint16_t id, int tile, int dir);

// move_peep_from_blueprint (src/blueprnt.cpp): sends person `id`, standing
// in its owner's blueprint, to a cell just outside it that is reachable
// from the entrance, nearest sides first (state PS_BP_GOTO or SS_BP_GOTO,
// its state and the cell saved). Returns whether it found one. Draws from
// the random generator.
bool move_peep_from_blueprint(World &world, uint16_t id);

} // namespace th
