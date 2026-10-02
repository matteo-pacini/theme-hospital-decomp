#include "hosp.h"

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    s16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

extern "C" {
extern Slot D_001581b2[];
extern Event D_001549d0[];
extern char **object_names;
}

u16 find_next_thing(Thing *cand, u8 kind, s8 sub1, s8 sub2);
void req5_draw_machine_usage_bar(long x, long y, long filled, long total);
u8 is_thing_paged(ThingPager *pager, u16 id);

// Machine window clicks (machine D_001581b2[3].value): close, call a
// handyman (EV_THINGPAGE_REQUEST), and replace it if affordable (asks to
// confirm).
// FUNCTION: 0x00088260
void req5(Window *win)
{
    WindowButton *button;
    u16 i;

    button = win->buttons;
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_button = i;
            if (i == resvars.requester_buttons[0x12]) {
                D_001549d0[player].type = EV_CLOSE_QUERY;
                D_001549d0[player].value = 3;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            } else if (i == resvars.requester_buttons[0x13]) {
                D_001549d0[player].type = EV_THINGPAGE_REQUEST;
                D_001549d0[player].value = D_001581b2[3].value;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            } else if (i == resvars.requester_buttons[0x14]) {
                u8 kind;

                kind = thing[D_001581b2[3].value].subtype;
                if ((u32)allhosps[player].balance >= (u32)allhosps[player].object_stats[kind].cost &&
                    allhosps[player].balance > 0) {
                    sprintf(othervar.yesno_msg, yesno_text[2], object_names[objects[kind].name],
                            allhosps[othervar.cur_player].object_stats[kind].cost);
                    open_yes_no_requester(EV_REPLACE_MACHINE, D_001581b2[3].value, 0, EV_CLOSE_YES_NO, 0, 0);
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                }
            }
        }
        i++;
    }
}

// Draws the machine window: its name (in brackets while a handyman is
// paged), the room's patient count and the strength left as a bar. A left
// click on the view centres the map on it, a right click moves to the next
// machine.
// FUNCTION: 0x00088490
void req5_overdraw(Window *win)
{
    char buf[100];
    s32 max;
    s32 uses;
    s32 y;
    s32 x;
    Thing *machine;
    s32 count;

    count = 0;
    compute_help_text(0, 0, 0xb);
    machine = &thing[D_001581b2[3].value];
    if (machine->type == THING_OBJECT) {
        LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                                  window.clip_y1 - window.clip_y0);
        x = win->x - lbDisplay.graphics_window_x;
        y = win->y - lbDisplay.graphics_window_y;
        uses = machine->times_used;
        max = allhosps[machine->owner].object_stats[machine->subtype].strength;
        set_font_from_cache(1);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (is_thing_paged(&allhosps[player].repair_pager, D_001581b2[3].value)) {
            sprintf(buf, "(%s)", object_names[machine->subtype]);
            LbTextDraw(x + resvars.coords[0x61].x, y + resvars.coords[0x61].y, buf);
        } else {
            sprintf(buf, "%s", object_names[machine->subtype]);
            LbTextDraw(x + resvars.coords[0x61].x, y + resvars.coords[0x61].y, buf);
        }
        {
            RoomInfo *room;
            s32 pct;

            room = &room_info[machine->room_id];
            if (room > room_info) {
                count = room->count;
                sprintf(buf, "%d", count);
                LbTextDraw(x + resvars.coords[0x62].x, y + resvars.coords[0x62].y, buf);
            }
            pct = (max - uses) * 100 / max;
            req5_draw_machine_usage_bar(x, y, pct, 100);
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(0);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x54].x,
                             y + resvars.boxes[0x54].y, resvars.boxes[0x54].w, resvars.boxes[0x54].h)) {
                othervar.center_on_slot = 3;
                othervar.full_redraw = 1;
            }
        }
        if (lbDisplay.mright_button && !othervar.right_button_was_down) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.boxes[0x54].x,
                             y + resvars.boxes[0x54].y, resvars.boxes[0x54].w, resvars.boxes[0x54].h)) {
                D_001581b2[3].value = find_next_thing(machine, THING_OBJECT, -1, -1);
                othervar.center_on_slot = 3;
                othervar.full_redraw = 1;
                othervar.prev_panel_info = othervar.panel_info;
                othervar.panel_info = 3;
                othervar.panel_thing = D_001581b2[3].value;
                play_sound(SNDSRC_GENERAL, SFX_CAMCLICK, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.right_button_was_down = 1;
        else
            othervar.right_button_was_down = 0;
    }
}

// Draws the machine window's bar filled to filled / total of its width.
// FUNCTION: 0x00088880
void req5_draw_machine_usage_bar(long x, long y, long filled, long total)
{
    s32 i;

    for (i = (filled << 16) / ((total << 16) / resvars.boxes[0x53].w) - 1; i >= 0; i--)
        reqspr_draw(x + resvars.boxes[0x53].x + i, y + resvars.boxes[0x53].y,
                    &button_sprite[resvars.requester_sprites[4]]);
}

// Returns 1 if thing id is among pager's requests, else 0.
// FUNCTION: 0x00088918
u8 is_thing_paged(ThingPager *pager, u16 id)
{
    s32 i;

    for (i = pager->count - 1; i >= 0; i--) {
        if (pager->pages[i].id == id)
            return 1;
    }
    return 0;
}
