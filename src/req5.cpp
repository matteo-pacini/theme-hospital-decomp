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
extern char D_00155ced[];
extern char **D_0016029c;
extern u8 D_001553c7;
extern char D_00120788[];
extern char D_0012078d[];
extern char D_00120790[];
extern s32 D_00188c3c;
extern s32 D_00188c40;
extern s32 D_00188c38;
extern u8 D_001553f0;
extern u8 D_001553c8;
extern u8 D_00188c7e;
extern u8 D_00188c7f;
extern u8 D_00155e19;
extern u8 D_00155434;
extern u8 D_00155435;
extern u16 D_00155436;
}

u16 find_next_thing(Thing *w, u8 kind, s8 sub1, s8 sub2);
void req5_draw_machine_usage_bar(long x, long y, long used, long total);
u8 is_thing_paged(ThingPager *pager, u16 id);

// FUNCTION: 0x00088260
void req5(Window *win)
{
    WindowButton *b;
    u16 i;

    b = win->buttons;
    for (i = 1; b; b = b->next) {
        if (b->flags & 6) {
            debug.unknown4 = i;
            if (i == resvars.fe[0x12]) {
                D_001549d0[player].type = 0x43;
                D_001549d0[player].value = 3;
                play_sound(10, 0x42, -1, -1);
            } else if (i == resvars.fe[0x13]) {
                D_001549d0[player].type = 0x76;
                D_001549d0[player].value = D_001581b2[3].value;
                play_sound(10, 0x43, -1, -1);
            } else if (i == resvars.fe[0x14]) {
                u8 kind;

                kind = thing[D_001581b2[3].value].subtype;
                if ((u32)allhosps[player].f35 >= (u32)allhosps[player].object_stats[kind].cost
                    && allhosps[player].f35 > 0) {
                    sprintf(D_00155ced, yesno_text[2], D_0016029c[objects[kind].name],
                            allhosps[D_001553c7].object_stats[kind].cost);
                    open_yes_no_requester(0x8a, D_001581b2[3].value, 0, 0x78, 0, 0);
                    play_sound(10, 0x43, -1, -1);
                }
            }
        }
        i++;
    }
}

// FUNCTION: 0x00088490
void req5_overdraw(Window *win)
{
    char buf[100];
    s32 max;
    s32 age;
    s32 y;
    s32 x;
    Thing *w;
    s32 count;

    count = 0;
    compute_help_text(0, 0, 0xb);
    w = &thing[D_001581b2[3].value];
    if (w->renderer == 6) {
        LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3,
                                  window.unknown2cf - window.unknown2c7);
        x = win->x - D_00188c3c;
        y = win->y - D_00188c40;
        age = w->mode;
        max = allhosps[w->owner].object_stats[w->subtype].unknown8;
        set_font_from_cache(1);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        if (is_thing_paged(&allhosps[player].ff6d, D_001581b2[3].value)) {
            sprintf(buf, D_00120788, D_0016029c[w->subtype]);
            LbTextDraw(x + resvars.f1c[0x61].x, y + resvars.f1c[0x61].y, buf);
        } else {
            sprintf(buf, D_0012078d, D_0016029c[w->subtype]);
            LbTextDraw(x + resvars.f1c[0x61].x, y + resvars.f1c[0x61].y, buf);
        }
        {
            RoomInfo *room;
            s32 pct;

            room = &room_info[w->unknown89];
            if (room > room_info) {
                count = room->count;
                sprintf(buf, D_00120790, count);
                LbTextDraw(x + resvars.f1c[0x62].x, y + resvars.f1c[0x62].y, buf);
            }
            pct = (max - age) * 100 / max;
            req5_draw_machine_usage_bar(x, y, pct, 100);
        }
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        set_font_from_cache(0);
        if (D_00188c7e) {
            if (over_hotspot(mouse_dx, mouse_dy, x + resvars.f18[0x54].x, y + resvars.f18[0x54].y, resvars.f18[0x54].w,
                             resvars.f18[0x54].h)) {
                D_001553f0 = 3;
                D_001553c8 = 1;
            }
        }
        if (D_00188c7f && !D_00155e19) {
            if (over_hotspot(mouse_dx, mouse_dy, x + resvars.f18[0x54].x, y + resvars.f18[0x54].y, resvars.f18[0x54].w,
                             resvars.f18[0x54].h)) {
                D_001581b2[3].value = find_next_thing(w, 6, -1, -1);
                D_001553f0 = 3;
                D_001553c8 = 1;
                D_00155435 = D_00155434;
                D_00155434 = 3;
                D_00155436 = D_001581b2[3].value;
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (D_00188c7f)
            D_00155e19 = 1;
        else
            D_00155e19 = 0;
    }
}

// FUNCTION: 0x00088880
void req5_draw_machine_usage_bar(long x, long y, long used, long total)
{
    s32 i;

    for (i = (used << 16) / ((total << 16) / resvars.f18[0x53].w) - 1; i >= 0; i--)
        reqspr_draw(x + resvars.f18[0x53].x + i, y + resvars.f18[0x53].y, &button_sprite[resvars.f12[4]]);
}

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
