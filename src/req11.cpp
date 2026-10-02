#include "hosp.h"

extern "C" {
extern Event player_events[];
extern u8 redraw_panel;
extern char **object_names;
extern char **req1_help;
}

s32 cost_of_all_selected_objects(void);
void clicked_buy_objects(const s32 idx, const u8 button);

// Buy objects requester clicks: the item, plus and minus buttons buy or sell
// one (5 with Shift, 10 with Ctrl) of the selected object, the accept button
// places them (event EV_PLACE_NEXT_OBJECT, or back to placing), and the cancel button
// empties the list (event EV_CANCEL_BUY_OBJECTS) or returns to placing.
// FUNCTION: 0x0009c6f0
void req11(Window *win)
{
    u16 i;
    u16 croom;
    u16 obj;
    WindowButton *button;
    s32 k;

    button = win->buttons;
    croom = crequester.kind;
    obj = crequester.item;
    k = 0x3e;
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.last_action = i;
            if (i >= resvars.requester_buttons[0x30] && i <= resvars.requester_buttons[0x39]) {
                clicked_buy_objects(i - resvars.requester_buttons[0x30], OldLeftButton ? 1 : 2);
            } else if (i >= resvars.requester_buttons[0x45] && i <= resvars.requester_buttons[0x4e]) {
                clicked_buy_objects(i - resvars.requester_buttons[0x45], 1);
            } else if (i >= resvars.requester_buttons[0x3b] && i <= resvars.requester_buttons[0x44]) {
                clicked_buy_objects(i - resvars.requester_buttons[0x3b], 2);
            } else if (i == resvars.requester_buttons[0x2f]) {
                if (crequester.arg == 1) {
                    player_events[player].type = EV_PLACE_NEXT_OBJECT;
                } else if (crequester.arg3) {
                    player_events[player].type = EV_BACK_TO_PLACE_OBJECT;
                    player_events[player].value = 0;
                }
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
            } else if (i == resvars.requester_buttons[0x3a]) {
                if (crequester.arg3) {
                    player_events[player].type = EV_BACK_TO_PLACE_OBJECT;
                    player_events[player].value = 1;
                } else {
                    while (--k >= 0)
                        gamevar.buy_count[k] = 0;
                    player_events[player].type = EV_CANCEL_BUY_OBJECTS;
                }
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            }
        }
        i++;
    }
}

// Draws the buy objects requester: each listed object with its count, the
// hovered one selected (crequester.item) with its picture and price, the
// total, and the advisor's flashing hints.
// FUNCTION: 0x0009c97c
void req11_overdraw(Window *win)
{
    u8 i;
    u16 item;
    u16 obj;
    s32 total;
    u32 scale_x;
    u32 scale_y;
    CompFrame *fr;
    WindowButton *button;
    s32 oy;
    s32 ox;
    char buf[100];

    button = win->buttons;
    obj = crequester.item;
    if (resvars.game_mode == SCREEN_MODE_13 && loaded_requester != 10)
        return;
    compute_help_text(0, 0, 5);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    ox = win->x - lbDisplay.graphics_window_x;
    oy = win->y - lbDisplay.graphics_window_y;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, lbDisplay.graphics_screen_height, 3);
    redraw_panel = 1;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    total = 0;
    for (i = 0; i < 10; i++) {
        item = req_indices[i];
        if (!item)
            break;
        total += gamevar.buy_count[item] * myhosp->object_stats[item].cost;
    }
    LbTextDraw(ox + resvars.coords[0x19].x, oy + resvars.coords[0x19].y, req1_help[3]);
    sprintf(buf, "%s%d", req1_help[5], myhosp->object_stats[obj].cost);
    LbTextDraw(ox + resvars.coords[0x1a].x, oy + resvars.coords[0x1a].y, buf);
    sprintf(buf, "%s%d", req1_help[6], total);
    LbTextDraw(ox + resvars.coords[0x1b].x, oy + resvars.coords[0x1b].y, buf);
    if (obj) {
        fr = &frame[start[objects[obj].preview_anim].frame];
        scale_x = 0x790000 / fr->width;
        scale_y = 0x790000 / fr->length;
        if (scale_y >= 0x10000 || scale_x >= 0x10000) {
            draw_a_frame(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x1c].x,
                         oy + resvars.coords[0x1c].y, 0, start[objects[obj].preview_anim].flags);
        } else if (scale_y <= scale_x) {
            draw_a_frame_scaled(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x1c].x,
                                oy + resvars.coords[0x1c].y, 0, start[objects[obj].preview_anim].flags, scale_y);
        } else {
            draw_a_frame_scaled(start[objects[obj].preview_anim].frame, ox + resvars.coords[0x1c].x,
                                oy + resvars.coords[0x1c].y, 0, start[objects[obj].preview_anim].flags, scale_x);
        }
    }
    for (i = 0; i < 10; i++) {
        item = req_indices[i];
        if (!item)
            break;
        set_font_from_cache(1);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.boxes[i + 0xb].x,
                         oy + resvars.boxes[i + 0xb].y, resvars.boxes[i + 0xb].w, resvars.boxes[i + 0xb].h)) {
            set_font_from_cache(2);
            if (crequester.item != item)
                play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
            crequester.item = item;
        }
        LbTextDraw(ox + resvars.coords[i + 0x34].x, oy + resvars.coords[i + 0x34].y, object_names[objects[item].name]);
        sprintf(buf, "%d", gamevar.buy_count[item]);
        LbTextDraw(ox + resvars.coords[i + 0x3e].x, oy + resvars.coords[i + 0x3e].y, buf);
    }
    if (advisor.flags & 1) {
        i = 1;
        othervar.flash_counter++;
        if (othervar.flash_counter % 10 == 0) {
            switch (advisor.tutorial_highlight) {
            case 0xa:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x30])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x12]]);
                break;
            case 9:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.requester_buttons[0x2f])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.requester_sprites[0x13]]);
                break;
            }
            othervar.flash_counter = 0;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// Takes amount of object idx off the buy list (gamevar.buy_count), stopping
// at 0.
// FUNCTION: 0x0009d114
void req11_sell_object(const u16 idx, const s32 amount)
{
    if (gamevar.buy_count[idx] >= amount)
        gamevar.buy_count[idx] -= amount;
    else
        gamevar.buy_count[idx] = 0;
}

// Buys (button 1) or sells back (2) amount of the selected object, 1, 5 with
// Shift or 10 with Ctrl, through EV_BUY_OBJECT / EV_SELL_OBJECT, if the
// player can afford it or has that many; else plays the error sound. idx is
// the list entry clicked: an empty one only plays the error sound.
// FUNCTION: 0x0009d164
void clicked_buy_objects(const s32 idx, const u8 button)
{
    s32 obj;
    s32 amount;

    amount = (HospShift & MOD_CTRL) ? 10 : ((HospShift & MOD_SHIFT) ? 5 : 1);
    obj = crequester.item;
    if (req_indices[idx]) {
        if (button == 1) {
            if (cost_of_all_selected_objects() + allhosps[player].object_stats[obj].cost * amount <=
                    (u32)allhosps[player].balance &&
                allhosps[player].balance > 0) {
                player_events[player].type = EV_BUY_OBJECT;
                player_events[player].value = obj;
                player_events[player].arg = amount;
                if (gamevar.buy_count[obj] + amount < 0x63)
                    play_sound(SNDSRC_GENERAL, SFX_ADDITEMJ, -1, -1);
                else
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                return;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
        } else {
            if (gamevar.buy_count[obj] >= amount) {
                player_events[player].type = EV_SELL_OBJECT;
                player_events[player].value = obj;
                player_events[player].arg = amount;
                play_sound(SNDSRC_GENERAL, SFX_DELITEMJ, -1, -1);
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            return;
        }
    }
    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
}

// Help text for an object entry of the buy objects requester, into
// othervar.help_msg.
// FUNCTION: 0x0009d36c
void req11_help_object(void)
{
    u8 idx;

    idx = req_indices[resvars.help_hotspots_HelpHotspotp[othervar.help_hotspot].req_slot];
    if (idx)
        lib_strcpy(othervar.help_msg, object_help[idx]);
}

// Help text for the plus button of a buy objects entry, into othervar.help_msg.
// FUNCTION: 0x0009d3cc
void req11_help_plus(void)
{
    u8 idx;

    idx = req_indices[resvars.help_hotspots_HelpHotspotp[othervar.help_hotspot].req_slot];
    if (idx)
        lib_strcpy(othervar.help_msg, help_text[0x29]);
}

// Help text for the minus button of a buy objects entry, into othervar.help_msg.
// FUNCTION: 0x0009d424
void req11_help_minus(void)
{
    u8 idx;

    idx = req_indices[resvars.help_hotspots_HelpHotspotp[othervar.help_hotspot].req_slot];
    if (idx)
        lib_strcpy(othervar.help_msg, help_text[0x2a]);
}
