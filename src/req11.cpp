#include "hosp.h"

extern "C" {
extern Event D_001549d0[];
extern u8 D_00156910;
extern char **object_names;
extern char **req1_help;
}

s32 cost_of_all_selected_objects(void);
void clicked_buy_objects(const s32 idx, const u8 button);

// FUNCTION: 0x0009c6f0
void req11(Window *win)
{
    u16 i;
    u16 croom;
    u16 obj;
    WindowButton *button;
    s32 k;

    button = win->buttons;
    croom = crequester.unknown8;
    obj = crequester.unknown3;
    k = 0x3e;
    for (i = 1; button; button = button->next) {
        if (button->flags & 6) {
            debug.unknown8 = i;
            if (i >= resvars.fe[0x30] && i <= resvars.fe[0x39]) {
                clicked_buy_objects(i - resvars.fe[0x30], OldLeftButton ? 1 : 2);
            } else if (i >= resvars.fe[0x45] && i <= resvars.fe[0x4e]) {
                clicked_buy_objects(i - resvars.fe[0x45], 1);
            } else if (i >= resvars.fe[0x3b] && i <= resvars.fe[0x44]) {
                clicked_buy_objects(i - resvars.fe[0x3b], 2);
            } else if (i == resvars.fe[0x2f]) {
                if (crequester.unknown4 == 1) {
                    D_001549d0[player].type = 0x2b;
                } else if (crequester.unknown9) {
                    D_001549d0[player].type = EV_BACK_TO_PLACE_OBJECT;
                    D_001549d0[player].value = 0;
                }
                play_sound(10, SFX_YESX, -1, -1);
            } else if (i == resvars.fe[0x3a]) {
                if (crequester.unknown9) {
                    D_001549d0[player].type = EV_BACK_TO_PLACE_OBJECT;
                    D_001549d0[player].value = 1;
                } else {
                    while (--k >= 0)
                        gamevar.buy_count[k] = 0;
                    D_001549d0[player].type = 0x64;
                }
                play_sound(10, SFX_NO4, -1, -1);
            }
        }
        i++;
    }
}

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
    obj = crequester.unknown3;
    if (resvars.f1 == SCREEN_MODE_13 && loaded_requester != 10)
        return;
    compute_help_text(0, 0, 5);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
        window.unknown2cf - window.unknown2c7);
    ox = win->x - lbDisplay.graphics_window_x;
    oy = win->y - lbDisplay.graphics_window_y;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, lbDisplay.graphics_screen_height, 3);
    D_00156910 = 1;
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
    LbTextDraw(ox + resvars.f1c[0x19].x, oy + resvars.f1c[0x19].y, req1_help[3]);
    sprintf(buf, "%s%d", req1_help[5], myhosp->object_stats[obj].cost);
    LbTextDraw(ox + resvars.f1c[0x1a].x, oy + resvars.f1c[0x1a].y, buf);
    sprintf(buf, "%s%d", req1_help[6], total);
    LbTextDraw(ox + resvars.f1c[0x1b].x, oy + resvars.f1c[0x1b].y, buf);
    if (obj) {
        fr = &frame[start[objects[obj].f10].frame];
        scale_x = 0x790000 / fr->width;
        scale_y = 0x790000 / fr->length;
        if (scale_y >= 0x10000 || scale_x >= 0x10000) {
            draw_a_frame(start[objects[obj].f10].frame, ox + resvars.f1c[0x1c].x, oy + resvars.f1c[0x1c].y, 0,
                start[objects[obj].f10].flags);
        } else if (scale_y <= scale_x) {
            draw_a_frame_scaled(start[objects[obj].f10].frame, ox + resvars.f1c[0x1c].x, oy + resvars.f1c[0x1c].y,
                0, start[objects[obj].f10].flags, scale_y);
        } else {
            draw_a_frame_scaled(start[objects[obj].f10].frame, ox + resvars.f1c[0x1c].x, oy + resvars.f1c[0x1c].y,
                0, start[objects[obj].f10].flags, scale_x);
        }
    }
    for (i = 0; i < 10; i++) {
        item = req_indices[i];
        if (!item)
            break;
        set_font_from_cache(1);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, ox + resvars.f18[i + 0xb].x, oy + resvars.f18[i + 0xb].y,
                resvars.f18[i + 0xb].w, resvars.f18[i + 0xb].h)) {
            set_font_from_cache(2);
            if (crequester.unknown3 != item)
                play_sound(10, SFX_HLIGHT5, -1, -1);
            crequester.unknown3 = item;
        }
        LbTextDraw(ox + resvars.f1c[i + 0x34].x, oy + resvars.f1c[i + 0x34].y, object_names[objects[item].name]);
        sprintf(buf, "%d", gamevar.buy_count[item]);
        LbTextDraw(ox + resvars.f1c[i + 0x3e].x, oy + resvars.f1c[i + 0x3e].y, buf);
    }
    if (advisor.f11f & 1) {
        i = 1;
        othervar.f9bc++;
        if (othervar.f9bc % 10 == 0) {
            switch (advisor.f120) {
            case 0xa:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x30])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x12]]);
                break;
            case 9:
                for (button = win->buttons; button; button = button->next) {
                    if (i == resvars.fe[0x2f])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + button->x, (s16)oy + button->y, &button_sprite[resvars.f12[0x13]]);
                break;
            }
            othervar.f9bc = 0;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// FUNCTION: 0x0009d114
void req11_sell_object(const u16 idx, const s32 amount)
{
    if (gamevar.buy_count[idx] >= amount)
        gamevar.buy_count[idx] -= amount;
    else
        gamevar.buy_count[idx] = 0;
}

// FUNCTION: 0x0009d164
void clicked_buy_objects(const s32 idx, const u8 button)
{
    s32 obj;
    s32 amount;

    amount = (HospShift & MOD_CTRL) ? 10 : ((HospShift & MOD_SHIFT) ? 5 : 1);
    obj = crequester.unknown3;
    if (req_indices[idx]) {
        if (button == 1) {
            if (cost_of_all_selected_objects() + allhosps[player].object_stats[obj].cost * amount
                    <= (u32)allhosps[player].f35
                && allhosps[player].f35 > 0) {
                D_001549d0[player].type = EV_BUY_OBJECT;
                D_001549d0[player].value = obj;
                D_001549d0[player].arg = amount;
                if (gamevar.buy_count[obj] + amount < 0x63)
                    play_sound(10, SFX_ADDITEMJ, -1, -1);
                else
                    play_sound(10, SFX_WRONG2, -1, -1);
                return;
            } else {
                play_sound(10, SFX_WRONG2, -1, -1);
            }
        } else {
            if (gamevar.buy_count[obj] >= amount) {
                D_001549d0[player].type = EV_SELL_OBJECT;
                D_001549d0[player].value = obj;
                D_001549d0[player].arg = amount;
                play_sound(10, SFX_DELITEMJ, -1, -1);
            } else {
                play_sound(10, SFX_WRONG2, -1, -1);
            }
            return;
        }
    }
    play_sound(10, SFX_WRONG2, -1, -1);
}

// FUNCTION: 0x0009d36c
void req11_help_object(void)
{
    u8 idx;

    idx = req_indices[resvars.f2c_HelpHotspotp[othervar.f8ac].unknownA];
    if (idx)
        fn_000bbff0(othervar.f96, object_help[idx]);
}

// FUNCTION: 0x0009d3cc
void req11_help_plus(void)
{
    u8 idx;

    idx = req_indices[resvars.f2c_HelpHotspotp[othervar.f8ac].unknownA];
    if (idx)
        fn_000bbff0(othervar.f96, help_text[0x29]);
}

// FUNCTION: 0x0009d424
void req11_help_minus(void)
{
    u8 idx;

    idx = req_indices[resvars.f2c_HelpHotspotp[othervar.f8ac].unknownA];
    if (idx)
        fn_000bbff0(othervar.f96, help_text[0x2a]);
}
