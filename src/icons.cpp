#include "hosp.h"

void reset_icon_value(Thing *const peep, const s8 icon);

// Watcom 10.6 cannot initialise IconTable's union of an array and an
// anonymous struct; the table is defined with this layout of the same size,
// bound to its real name with #pragma aux.
struct IconTableData {
    u8 unknown0;
    u16 unknown1;
    u16 f3;
    s8 f5;
};

IconTableData icon_table_data[] = {
    { 255, 0, 0, 0 },
    { 20, 2000, 65535, 1 },
    { 19, 2001, 65535, 1 },
    { 18, 2002, 65535, 1 },
    { 17, 2003, 65535, 1 },
    { 16, 2004, 65535, 1 },
    { 30, 1957, 65535, 1 },
    { 11, 2026, 65535, 1 },
    { 35, 2289, 120, 1 },
    { 29, 2288, 120, 1 },
    { 30, 1995, 120, -1 },
    { 55, 1998, 120, -1 },
    { 60, 1993, 120, -1 },
    { 10, 2287, 65535, 1 },
    { 9, 2285, 65535, 1 },
    { 9, 2286, 65535, 1 },
    { 8, 2283, 120, 1 },
    { 60, 1994, 120, -1 },
    { 60, 1997, 120, -1 },
    { 15, 2009, 120, 0 },
    { 15, 2010, 120, 0 },
    { 15, 2022, 120, 0 },
    { 15, 2023, 120, 0 },
    { 1, 2024, 120, 0 },
    { 19, 2025, 120, 0 },
    { 15, 2027, 120, 0 },
    { 13, 2284, 2, 0 },
    { 14, 2503, 120, 1 },
    { 50, 2282, 120, 1 },
    { 0, 0, 0, 0 },
};
#pragma aux icon_table_data "W?icon_table$n[]x$IconTable$$"

// FUNCTION: 0x000b0300
void set_icon_value(Thing *const peep, const s8 icon)
{
    if (peep->type != THING_PATIENT || peep->health > 0) {
        if (icon_table[icon].unknown0 <= icon_table[fn_000cb2db(peep->icon_id)].unknown0) {
            peep->icon_frame = start[icon_table[icon].unknown1].frame;
            if (icon_table[icon].f5 < 0) {
                peep->icon_id = -icon;
                peep->icon_timer = icon_table[icon].f3;
            } else if (icon_table[icon].f5 == 0) {
                peep->icon_id = icon;
                peep->icon_timer = icon_table[icon].f3;
            } else if (icon_table[icon].f5 > 0) {
                peep->icon_id = icon;
                peep->icon_timer = 0xffff;
            }
        }
    }
}

// FUNCTION: 0x000b0428
void reset_icon_value(Thing *const peep, const s8 icon)
{
    if (peep->icon_id == fn_000cb2db(icon)) {
        peep->icon_id = 0;
        peep->icon_timer = 0;
        peep->icon_frame = 0;
    }
}

// FUNCTION: 0x000b0474
void peep_icon(Thing *const peep)
{
    if (peep->type != THING_PATIENT || peep->health > 0) {
        if (peep->icon_id < 0)
            play_sound(10, SFX_HLIGHTP2, -1, -1);
        peep->icon_id = fn_000cb2db(peep->icon_id);
        if (icon_table[peep->icon_id].unknown1 > 0 && icon_table[peep->icon_id].f5 == -1)
            peep->icon_timer = icon_table[peep->icon_id].f3;
    }
}

// FUNCTION: 0x000b0540
void update_icon_counter(Thing *const peep)
{
    if (peep->icon_id > 0) {
        if ((s16)peep->icon_timer > 0) {
            peep->icon_timer--;
            peep->icon_frame = frame[peep->icon_frame].next;
        }
        if ((s16)peep->icon_timer == 0)
            reset_icon_value(peep, peep->icon_id);
        else if ((s16)peep->icon_timer < 0)
            peep->icon_frame = frame[peep->icon_frame].next;
    }
}

extern "C" {

// FUNCTION: 0x000b05f0
void clear_peep_icon(Thing *peep)
{
    peep->icon_id = 0;
    peep->icon_timer = 0;
    peep->icon_frame = 0;
}

}
