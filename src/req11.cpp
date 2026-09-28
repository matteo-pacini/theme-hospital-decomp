#include "hosp.h"

struct Box {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern u8 *D_0015f402;
extern u16 *D_0015f406;
extern Box *D_0015f40c;
extern Point *D_0015f410;
extern HelpHotspot *D_0015f420;
extern Event D_001549d0[];
extern TbScreenMode D_0015f3f5;
extern u8 D_0015831b;
extern u16 D_0015831c;
extern u8 D_00155d74;
extern u16 D_00155c64;
extern char D_0015544e[];
extern u8 D_00156910;
extern u16 D_00188c74;
extern s32 D_00188c38;
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern char **D_0016029c;
extern char **D_001602d4;
extern char D_00121a50[];
extern char D_00121a55[];
extern char D_00121a5a[];
}

s32 cost_of_all_selected_objects(void);
void clicked_buy_objects(const s32 idx, const u8 button);

// FUNCTION: 0x0009c6f0
void req11(Window *win)
{
    u16 i;
    u16 croom;
    u16 obj;
    WindowButton *b;
    s32 k;

    b = win->buttons;
    croom = crequester.unknown8;
    obj = crequester.unknown3;
    k = 0x3e;
    for (i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown8 = i;
            if (i >= D_0015f402[0x30] && i <= D_0015f402[0x39]) {
                clicked_buy_objects(i - D_0015f402[0x30], OldLeftButton ? 1 : 2);
            } else if (i >= D_0015f402[0x45] && i <= D_0015f402[0x4e]) {
                clicked_buy_objects(i - D_0015f402[0x45], 1);
            } else if (i >= D_0015f402[0x3b] && i <= D_0015f402[0x44]) {
                clicked_buy_objects(i - D_0015f402[0x3b], 2);
            } else if (i == D_0015f402[0x2f]) {
                if (crequester.unknown4 == 1) {
                    D_001549d0[player].type = 0x2b;
                } else if (crequester.unknown9) {
                    D_001549d0[player].type = 0x5c;
                    D_001549d0[player].value = 0;
                }
                play_sound(10, 0x41, -1, -1);
            } else if (i == D_0015f402[0x3a]) {
                if (crequester.unknown9) {
                    D_001549d0[player].type = 0x5c;
                    D_001549d0[player].value = 1;
                } else {
                    while (--k >= 0)
                        gamevar.buy_count[k] = 0;
                    D_001549d0[player].type = 0x64;
                }
                play_sound(10, 0x42, -1, -1);
            }
        }
        i++;
    }
}

// FUNCTION: 0x0009c97c
void req11_overdraw(Window *win)
{
    u8 i;
    u16 room;
    u16 obj;
    s32 total;
    u32 sx;
    u32 sy;
    CompFrame *fr;
    WindowButton *b;
    s32 oy;
    s32 ox;
    char buf[100];

    b = win->buttons;
    obj = crequester.unknown3;
    if (D_0015f3f5 == SCREEN_MODE_13 && loaded_requester != 10)
        return;
    compute_help_text(0, 0, 5);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
        window.unknown2cf - window.unknown2c7);
    ox = win->x - D_00188c3c;
    oy = win->y - D_00188c40;
    set_all_refresh_zones_in_window(ox, oy, win->width + ox, D_00188c38, 3);
    D_00156910 = 1;
    D_00188c74 = 0;
    set_font_from_cache(1);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    total = 0;
    for (i = 0; i < 10; i++) {
        room = req_indices[i];
        if (!room)
            break;
        total += gamevar.buy_count[room] * myhosp->object_stats[room].cost;
    }
    LbTextDraw(ox + D_0015f410[0x19].x, oy + D_0015f410[0x19].y, D_001602d4[3]);
    sprintf(buf, D_00121a50, D_001602d4[5], myhosp->object_stats[obj].cost);
    LbTextDraw(ox + D_0015f410[0x1a].x, oy + D_0015f410[0x1a].y, buf);
    sprintf(buf, D_00121a55, D_001602d4[6], total);
    LbTextDraw(ox + D_0015f410[0x1b].x, oy + D_0015f410[0x1b].y, buf);
    if (obj) {
        fr = &frame[start[objects[obj].f10].frame];
        sx = 0x790000 / fr->width;
        sy = 0x790000 / fr->length;
        if (sy >= 0x10000 || sx >= 0x10000) {
            draw_a_frame(start[objects[obj].f10].frame, ox + D_0015f410[0x1c].x, oy + D_0015f410[0x1c].y, 0,
                start[objects[obj].f10].flags);
        } else if (sy <= sx) {
            draw_a_frame_scaled(start[objects[obj].f10].frame, ox + D_0015f410[0x1c].x, oy + D_0015f410[0x1c].y,
                0, start[objects[obj].f10].flags, sy);
        } else {
            draw_a_frame_scaled(start[objects[obj].f10].frame, ox + D_0015f410[0x1c].x, oy + D_0015f410[0x1c].y,
                0, start[objects[obj].f10].flags, sx);
        }
    }
    for (i = 0; i < 10; i++) {
        room = req_indices[i];
        if (!room)
            break;
        set_font_from_cache(1);
        if (over_hotspot(mouse_dx, mouse_dy, ox + D_0015f40c[i + 0xb].x, oy + D_0015f40c[i + 0xb].y,
                D_0015f40c[i + 0xb].w, D_0015f40c[i + 0xb].h)) {
            set_font_from_cache(2);
            if (crequester.unknown3 != room)
                play_sound(10, 0x44, -1, -1);
            crequester.unknown3 = room;
        }
        LbTextDraw(ox + D_0015f410[i + 0x34].x, oy + D_0015f410[i + 0x34].y, D_0016029c[objects[room].name]);
        sprintf(buf, D_00121a5a, gamevar.buy_count[room]);
        LbTextDraw(ox + D_0015f410[i + 0x3e].x, oy + D_0015f410[i + 0x3e].y, buf);
    }
    if (D_0015831b & 1) {
        i = 1;
        D_00155d74++;
        if (D_00155d74 % 10 == 0) {
            switch (D_0015831c) {
            case 0xa:
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x30])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + b->x, (s16)oy + b->y, &button_sprite[D_0015f406[0x12]]);
                break;
            case 9:
                for (b = win->buttons; b; b = b->next) {
                    if (i == D_0015f402[0x2f])
                        break;
                    i++;
                }
                reqspr_draw((s16)ox + b->x, (s16)oy + b->y, &button_sprite[D_0015f406[0x13]]);
                break;
            }
            D_00155d74 = 0;
        }
    }
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0);
}

// FUNCTION: 0x0009d114
void req11_sell_object(const u16 idx, const s32 n)
{
    if (gamevar.buy_count[idx] >= n)
        gamevar.buy_count[idx] -= n;
    else
        gamevar.buy_count[idx] = 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x0009d164
void clicked_buy_objects(const s32 idx, const u8 button)
{
    s32 obj;
    s32 amount;

    amount = (HospShift & 2) ? 10 : ((HospShift & 1) ? 5 : 1);
    obj = crequester.unknown3;
    if (req_indices[idx]) {
        if (button == 1) {
            if (allhosps[player].object_stats[obj].cost * amount + cost_of_all_selected_objects()
                    <= (u32)allhosps[player].f35
                && allhosps[player].f35 > 0) {
                D_001549d0[player].type = 0x25;
                D_001549d0[player].value = obj;
                D_001549d0[player].arg = amount;
                if (gamevar.buy_count[obj] + amount < 0x63)
                    play_sound(10, 0x5b, -1, -1);
                else
                    play_sound(10, 8, -1, -1);
                return;
            } else {
                play_sound(10, 8, -1, -1);
            }
        } else {
            if (gamevar.buy_count[obj] >= amount) {
                D_001549d0[player].type = 0x63;
                D_001549d0[player].value = obj;
                D_001549d0[player].arg = amount;
                play_sound(10, 0x5c, -1, -1);
            } else {
                play_sound(10, 8, -1, -1);
            }
            return;
        }
    }
    play_sound(10, 8, -1, -1);
}

// FUNCTION: 0x0009d36c
void req11_help_object(void)
{
    u8 idx;

    idx = req_indices[D_0015f420[D_00155c64].unknownA];
    if (idx)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, object_help[idx]);
}

// FUNCTION: 0x0009d3cc
void req11_help_plus(void)
{
    u8 idx;

    idx = req_indices[D_0015f420[D_00155c64].unknownA];
    if (idx)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[0x29]);
}

// FUNCTION: 0x0009d424
void req11_help_minus(void)
{
    u8 idx;

    idx = req_indices[D_0015f420[D_00155c64].unknownA];
    if (idx)
        LbDataLoadSetModifyFilenameFunction(D_0015544e, help_text[0x2a]);
}
