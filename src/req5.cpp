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
extern char **D_0016029c;
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
                    sprintf(othervar.f935, yesno_text[2], D_0016029c[objects[kind].name],
                            allhosps[othervar.ff].object_stats[kind].cost);
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
        x = win->x - lbDisplay.graphics_window_x;
        y = win->y - lbDisplay.graphics_window_y;
        age = w->mode;
        max = allhosps[w->owner].object_stats[w->subtype].unknown8;
        set_font_from_cache(1);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (is_thing_paged(&allhosps[player].ff6d, D_001581b2[3].value)) {
            sprintf(buf, "(%s)", D_0016029c[w->subtype]);
            LbTextDraw(x + resvars.f1c[0x61].x, y + resvars.f1c[0x61].y, buf);
        } else {
            sprintf(buf, "%s", D_0016029c[w->subtype]);
            LbTextDraw(x + resvars.f1c[0x61].x, y + resvars.f1c[0x61].y, buf);
        }
        {
            RoomInfo *room;
            s32 pct;

            room = &room_info[w->unknown89];
            if (room > room_info) {
                count = room->count;
                sprintf(buf, "%d", count);
                LbTextDraw(x + resvars.f1c[0x62].x, y + resvars.f1c[0x62].y, buf);
            }
            pct = (max - age) * 100 / max;
            req5_draw_machine_usage_bar(x, y, pct, 100);
        }
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        set_font_from_cache(0);
        if (lbDisplay.mleft_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x54].x, y + resvars.f18[0x54].y, resvars.f18[0x54].w,
                             resvars.f18[0x54].h)) {
                othervar.f38 = 3;
                othervar.f10 = 1;
            }
        }
        if (lbDisplay.mright_button && !othervar.fa61) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x + resvars.f18[0x54].x, y + resvars.f18[0x54].y, resvars.f18[0x54].w,
                             resvars.f18[0x54].h)) {
                D_001581b2[3].value = find_next_thing(w, 6, -1, -1);
                othervar.f38 = 3;
                othervar.f10 = 1;
                othervar.f7d = othervar.f7c;
                othervar.f7c = 3;
                othervar.f7e = D_001581b2[3].value;
                play_sound(10, 0xbb, -1, -1);
            }
        }
        if (lbDisplay.mright_button)
            othervar.fa61 = 1;
        else
            othervar.fa61 = 0;
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
