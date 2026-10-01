#include "hosp.h"

extern "C" {
}

void reset_dept_object(void);

// Before buying more for an existing room: takes the room type's required
// objects that room lacks off the shopping list (gamevar.buy_list),
// remembering them in othervar.shop_palette_adjust.
// FUNCTION: 0x00054e50
void adjust_build_palette_for_shop(RoomInfo *room)
{
    u8 counts[0x3f];
    Thing *room_obj;
    s32 i;
    s32 obj_type;

    memset(counts, 0, 0x3f);
    for (room_obj = &thing[room->first_object]; room_obj > thing; room_obj = &thing[room_obj->room_obj_next])
        counts[room_obj->subtype]++;
    memset(othervar.shop_palette_adjust, 0, 0x3f);
    for (i = rooms[room->type].compulsory_count - 1; i >= 0; i--) {
        obj_type = rooms[room->type].objects[i];
        if (counts[obj_type] == 0) {
            gamevar.buy_list.to_place[obj_type]--;
            othervar.shop_palette_adjust[obj_type]++;
        } else {
            counts[obj_type]--;
        }
    }
}

// Puts back the shopping list counts that adjust_build_palette_for_shop
// took off.
// FUNCTION: 0x00054f58
void post_shop_readjust_build_palette(void)
{
    s32 i;

    for (i = 0x3d; i >= 0; i--)
        gamevar.buy_list.to_place[i] += othervar.shop_palette_adjust[i];
}

// Clears the requester's list of entries (req_indices).
// FUNCTION: 0x00054f98
void clear_req_indices(void)
{
    memset(req_indices, 0, 10);
}

// Lists the extra objects room type `room` can buy (up to ten available to
// this player) in req_indices and opens the object list (crequester.mode
// 3).
// FUNCTION: 0x00054fc0
void set_buyable_items(u16 room)
{
    u8 obj_type;
    u16 count;
    u16 i;

    count = 0;
    crequester.mode = 3;
    crequester.item = 0;
    for (i = 0; i < rooms[room].buyable_count && count < 10; i++) {
        obj_type = rooms[room].buyable[i];
        if (allhosps[player].equipment[obj_type].available == 1 && objects[obj_type].available_for_level == 1)
            req_indices[count++] = obj_type;
    }
    while (count < 10)
        req_indices[count++] = 0;
    reset_dept_object();
}

// Sets the shopping list (gamevar.buy_list) to just room type `room`'s
// required objects.
// FUNCTION: 0x000550cc
void buy_compulsory_items(const u16 room)
{
    u16 i;

    for (i = 0; i < 0x3e; i++)
        gamevar.buy_list.to_place[i] = 0;
    for (i = 0; i < rooms[room].compulsory_count; i++) {
        u8 obj;

        obj
 = rooms[room].objects[i];
        gamevar.buy_list.to_place[obj]++;
    }
}

// Lists department dept's room types this player can build (up to ten) in
// req_indices.
// FUNCTION: 0x00055164
void req1_reset_rooms(u16 dept)
{
    u16 i;
    u16 count;
    u16 room;

    count = 0;
    for (i = 0; i < departments[dept].count; i++) {
        room = departments[dept].rooms[i];
        if (room_available(room))
            req_indices[count++] = room;
    }
    while (count < 10)
        req_indices[count++] = 0;
}

// Returns 1 if this player has every object room type `room` requires, else
// 0.
// FUNCTION: 0x00055214
u16 room_available(u16 room)
{
    u16 i;

    for (i = 0; i < rooms[room].compulsory_count; i++) {
        if (allhosps[player].equipment[rooms[room].objects[i]].available == 0
            || objects[rooms[room].objects[i]].available_for_level == 0)
            return 0;
    }
    return 1;
}

// Adds amount (may be negative) of object obj to the shopping list, capped
// at 99.
// FUNCTION: 0x000552e8
void req1_buy_object(const u16 obj, const s32 amount)
{
    if (gamevar.buy_list.to_place[obj] + amount < 0x63)
        gamevar.buy_list.to_place[obj] += amount;
    else
        gamevar.buy_list.to_place[obj] = 0x63;
}

// Returns the cost of everything on the shopping list at this player's
// prices.
// FUNCTION: 0x0005533c
s32 cost_of_all_selected_objects(void)
{
    s32 total;
    u8 i;

    total = 0;
    for (i = 0; i < 0x3e; i++)
        total += gamevar.buy_list.to_place[i] * allhosps[player].object_stats[i].cost;
    return total;
}

// Selects the current room type's first extra object (crequester.item).
// FUNCTION: 0x000553b4
void reset_dept_object(void)
{
    crequester.item = rooms[crequester.kind].buyable[0];
}
