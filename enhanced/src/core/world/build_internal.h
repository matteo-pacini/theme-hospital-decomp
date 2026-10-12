// Private to the build*.cpp files: what the parts of build.cpp share.
#pragma once

#include "core/world/game.h"
#include "core/world/world.h"

namespace th
{

inline namespace build_detail
{

// The local player; the engine has no other.
constexpr int kPlayer = 0;

constexpr uint8_t kObjSwingDoor = 52;
constexpr uint8_t kObjSwingDoor2 = 53;

// OS_STATIC, OS_IN_USE and OS_REMOVED (include/enums.h).
constexpr uint8_t kObjectRemoved = 0x12;

// SFX_* (include/enums.h).
constexpr uint8_t kSfxWrong2 = 0x08;

// The game indexes cells as column + (row << 7) and offsets without
// checking the result against the map's bounds; here cells off the map are
// skipped (writes) or read as empty.
inline bool in_map(int index)
{
    return index >= 0 && index < kMapCells;
}

inline int cell_index(int x, int y)
{
    return x + (y << 7);
}

inline uint8_t wall_at(const World &w, int index, int side)
{
    if (!in_map(index))
        return 0;
    const MapCell &c = w.map[size_t(index)];
    return side == 0 ? c.wall_n : c.wall_w;
}

inline void set_flags(World &w, int index, uint8_t bits)
{
    if (in_map(index))
        w.map[size_t(index)].flags |= bits;
}

inline const ShapeInfo &shape_of(int kind, int dir)
{
    return kShapes[kObjects[size_t(kind)].shapes[size_t(dir)]];
}

inline void play(World &w, uint8_t sfx)
{
    w.building.sounds.push_back(sfx);
}

// The thing ids of the people (patients and staff) in room `room`, the
// game's room->first_person list, which the engine does not keep: people
// carry their room in Thing.room.
template <typename Each> void for_each_person_in(const World &w, uint16_t room, Each &&each)
{
    for (size_t i = 1; i < w.things.size(); i++) {
        const Thing &t = w.things[i];
        if ((t.type == kThingPatient || t.type == kThingStaff) && t.room == room)
            each(uint16_t(i));
    }
}

// build_blueprint.cpp
int find_bluep_who(const World &w, int tile);
void insert_blank_wall(World &w, int index, int side);
bool is_tile_valid(const World &w, int tile, int owner, uint8_t *flags = nullptr);
bool is_valid_no_room(const World &w, int tile, uint8_t *flags = nullptr);
bool is_tile_near_half_wall(const World &w, int x, int y, int x1, int y1);
bool is_room_size_valid(const World &w, int x0, int y0, int x1, int y1, uint8_t *flags = nullptr);
bool is_valid_door(const World &w, int tile, int type);
bool check_room_access_with_blueprint(World &w);
void mark_blueprint_on_map(World &w, int x0, int y0, int x1, int y1, bool overlay);

RoomInfo &built_room(World &w);
bool commit_blueprint(Game &game);
void uncommit_blueprint(World &w);
void modify_blueprint(World &w, bool dragging, uint16_t tile);

// build_door.cpp
void temp_place_door(World &w, uint16_t pos);
void place_bp_door(Game &game, bool swap);
uint16_t place_door(World &w, const AnimData &anims, int tile, int kind, int dir);

// build_walls.cpp
void build_all_rooms(Game &game);
void recolour_all_rooms(World &w);
void set_buyable_items(World &w, int room_type);

// build_room.cpp
void remove_object_from_map(World &w, uint16_t id);
void clear_build_objects(Game &game);
void delete_blueprint_room(Game &game);
void edit_blueprint(World &w);
bool is_room_empty(const World &w, uint16_t r);
void setup_place_object_requester(World &w);
void place_next_in_room(World &w);
void init_edit_room(World &w, uint16_t r);
void commit_doors(Game &game);
void open_room(Game &game);
void allocate_roominfo(Game &game, int type);
void cancel_build(Game &game);
bool try_move_peep_to_wall(World &w, Thing &peep, int side);

// build_events.cpp
void set_room_requester_mode_furnishing(World &world);

} // namespace build_detail

} // namespace th
