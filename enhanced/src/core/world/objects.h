#pragma once

#include "core/world/object_tables.h"

#include <array>
#include <cstdint>

namespace th
{

class Game;
struct Event;
struct World;

// Room types (the game's ROOM_*, rooms[] in src/tables.cpp) used so far.
constexpr uint16_t kRoomCorridor = 3;

// The object kinds the corridor offers, in the buy objects requester's order
// (rooms[ROOM_CORRIDOR].buyable, src/tables.cpp): OBJ_RECEPTION_DESK,
// OBJ_BENCH, OBJ_DRINKS_MACHINE, OBJ_FIRE_EXTINGUISHER, OBJ_RADIATOR,
// OBJ_PLANT and OBJ_LITTER_BOMB.
inline constexpr std::array<uint8_t, 7> kCorridorObjects = {11, 4, 7, 43, 44, 45, 17};

// The requesters the commands of buying, placing objects and building rooms
// open (crequester.id): the buy objects requester (REQ_BUY_OBJECTS, window
// 10), the edit room requester (REQ_EDIT_ROOM, window 5), which lists what
// is left to place and leads building a room, and the build room requester
// (REQ_BUILD_ROOM, window 9).
enum class ObjectRequester : uint8_t { None, BuyObjects, EditRoom, BuildRoom };

// The edit room requester's buttons, counted from 1 in its window
// (hires_requester_buttons[0x4f] to [0x52], src/reqbut.cpp): cancel, buy
// more (furnish), pick up and confirm.
constexpr int kEditRoomCancel = 9;
constexpr int kEditRoomFurnish = 10;
constexpr int kEditRoomPickUp = 11;
constexpr int kEditRoomConfirm = 12;

// The player's objects bought and being placed: the game's gamevar buy list
// and placing fields, and the state of the requester they drive
// (crequester). The interface shows `requester` and reads the rest; the
// commands change them.
struct ObjectPlacing {
    // Per object kind, how many are bought and not yet placed, at most 99
    // (gamevar.buy_count, also buy_list.to_place). Nothing is paid until an
    // object is placed.
    std::array<uint8_t, kObjectKinds> buy_count{};
    // Kinds with any left to place (gamevar.buy_kind_count).
    uint8_t buy_kind_count = 0;

    // An object follows the pointer on the map (mouse mode MM_PLACE_OBJECT).
    bool placing = false;
    uint8_t kind = 0;   // gamevar.place_kind
    uint8_t dir = 0;    // place_dir, 0-3; kept from one object to the next
    uint8_t mode = 0;   // place_mode: 0 in a room, 1 against its wall, 2 anywhere (the corridor)
    uint16_t room = 0;  // place_room (room_info index), 0 for the corridor
    int16_t offset = 0; // place_offset, added to the pointer's cell
    // Where the object goes and whether it may (place_tile, place_ok), as
    // update_placement last found; recomputed when the pointer's cell
    // changes or `redraw` is set (place_redraw).
    uint16_t tile = 0;
    bool ok = false;
    bool redraw = false;
    // The pointer's cell update_placement last saw (othervar.prev_mouse_cell),
    // -1 for none.
    int mouse_cell = -1;

    ObjectRequester requester = ObjectRequester::None;
    // crequester.kind: the room type furnished or built (kRoomCorridor for
    // the corridor); in the build room requester, the room type under the
    // pointer.
    uint16_t room_type = 0;
    // crequester.mode: 3 buying; 6 or 0xa (corridor) placing; building a
    // room, 0 the blueprint, 2 the door, 4 windows; in the build room
    // requester, 1 until a department is chosen, then 4.
    uint8_t requester_mode = 0;
    // crequester.item: the buy objects requester's selected kind.
    uint16_t item = 0;
    // crequester.arg: 1 while the buy objects requester furnishes the
    // corridor; in the edit room requester, bit n disables its button n
    // (counted from 1); in the build room requester, the department shown.
    uint32_t arg = 0;
    // crequester.arg3: the kind being placed (0 while buying); while the
    // buy objects requester furnishes a room, the low byte of its cell.
    uint16_t arg3 = 0;
    // The buy objects requester's entries, or the build room requester's
    // room types (req_indices), 0 past the last.
    std::array<uint8_t, 10> offered{};
    // The edit room requester's instructions (othervar.edit_room_hint, a
    // misc_text index).
    uint8_t hint = 0;
    // pmenu.disabled as these commands set it: the menu bar is off while the
    // buy objects requester is up.
    bool menu_disabled = false;
};

// Carries out the commands of buying, selling and placing objects
// (milestone 4 step 2): EV_OPEN_FURNISH_CORRIDOR, EV_BUY_OBJECT,
// EV_SELL_OBJECT, EV_PLACE_NEXT_OBJECT, EV_CANCEL_BUY_OBJECTS and
// EV_PLACE_OBJECT (value: tile; arg: kind | facing << 8), as process_turn
// (src/io.cpp) for the local player. Returns false for other commands, and
// for EV_CANCEL_BUILD, whose object part (forgetting what was bought,
// closing the requesters, ending placing) it carries out but which has a
// part for held staff too.
bool apply_object_event(Game &game, const Event &event);

// init_place_object (src/build.cpp): starts placing an object of `kind` in
// `mode` (0 in room `room`, which becomes 1 for objects hugging walls; 2 in
// the corridor), with `offset` added to the pointer's cell. Shows the edit
// room requester with what is left to place.
void init_place_object(ObjectPlacing &p, int kind, int mode, int room, int offset);

// A click on an entry of the edit room requester's list (req14_overdraw):
// places `kind` next, in the same mode and room.
void select_object_to_place(ObjectPlacing &p, int kind);

// place_object_change_angle (src/build.cpp): faces the object being placed
// `angle`, or with -1 the next way it can face (0 if it cannot face
// `angle`). Returns whether its facing changed.
bool change_placing_angle(ObjectPlacing &p, int angle);

// The first kind with any left to place, 0 if none (next_object_to_build).
int next_object_to_build(const ObjectPlacing &p);

// draw_object_cursor's computation (src/cursor.cpp): for the pointer on
// `mouse_cell`, where the object being placed would go (p.tile: in room
// p.room for modes 0 and 1, against its wall for mode 1, which also sets
// p.dir) and whether it may (p.ok: it fits, with its second part, and leaves
// the room's objects usable from its door, or in the corridor what patients
// use reachable from the entrance; corridor objects go into rooms only if
// movable into them).
void update_placement(World &world, ObjectPlacing &p, int mouse_cell);

// cancel_top_level_requesters' part for the buy objects requester
// (src/utils.cpp), for a command opening another top-level requester: unless
// it is placing, closes it and forgets what was bought.
void cancel_buy_objects(ObjectPlacing &p);

} // namespace th
