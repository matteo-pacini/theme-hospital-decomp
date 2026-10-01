#include "hosp.h"

extern "C" {
}

void reset_dept_object(void);

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
    memset(othervar.f8f6, 0, 0x3f);
    for (i = rooms[room->type].fc - 1; i >= 0; i--) {
        obj_type = rooms[room->type].objects[i];
        if (counts[obj_type] == 0) {
            gamevar.unknown33.unknown0[obj_type]--;
            othervar.f8f6[obj_type]++;
        } else {
            counts[obj_type]--;
        }
    }
}

// FUNCTION: 0x00054f58
void post_shop_readjust_build_palette(void)
{
    s32 i;

    for (i = 0x3d; i >= 0; i--)
        gamevar.unknown33.unknown0[i] += othervar.f8f6[i];
}

// FUNCTION: 0x00054f98
void clear_req_indices(void)
{
    memset(req_indices, 0, 10);
}

// FUNCTION: 0x00054fc0
void set_buyable_items(u16 room)
{
    u8 obj_type;
    u16 count;
    u16 i;

    count = 0;
    crequester.unknown0 = 3;
    crequester.unknown3 = 0;
    for (i = 0; i < rooms[room].f17 && count < 10; i++) {
        obj_type = rooms[room].f18[i];
        if (allhosps[player].equipment[obj_type].unknown0 == 1 && objects[obj_type].unknown1c == 1)
            req_indices[count++] = obj_type;
    }
    while (count < 10)
        req_indices[count++] = 0;
    reset_dept_object();
}

// FUNCTION: 0x000550cc
void buy_compulsory_items(const u16 room)
{
    u16 i;

    for (i = 0; i < 0x3e; i++)
        gamevar.unknown33.unknown0[i] = 0;
    for (i = 0; i < rooms[room].fc; i++) {
        u8 obj;

        obj
 = rooms[room].objects[i];
        gamevar.unknown33.unknown0[obj]++;
    }
}

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

// FUNCTION: 0x00055214
u16 room_available(u16 room)
{
    u16 i;

    for (i = 0; i < rooms[room].fc; i++) {
        if (allhosps[player].equipment[rooms[room].objects[i]].unknown0 == 0
            || objects[rooms[room].objects[i]].unknown1c == 0)
            return 0;
    }
    return 1;
}

// FUNCTION: 0x000552e8
void req1_buy_object(const u16 obj, const s32 amount)
{
    if (gamevar.unknown33.unknown0[obj] + amount < 0x63)
        gamevar.unknown33.unknown0[obj] += amount;
    else
        gamevar.unknown33.unknown0[obj] = 0x63;
}

// FUNCTION: 0x0005533c
s32 cost_of_all_selected_objects(void)
{
    s32 total;
    u8 i;

    total = 0;
    for (i = 0; i < 0x3e; i++)
        total += gamevar.unknown33.unknown0[i] * allhosps[player].object_stats[i].cost;
    return total;
}

// FUNCTION: 0x000553b4
void reset_dept_object(void)
{
    crequester.unknown3 = rooms[crequester.unknown8].f18[0];
}
