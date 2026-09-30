#include "hosp.h"

struct ReqButton {
    u8 unknown0[8];
    u8 flags;
    u8 unknown9[3];
    ReqButton *next;
};

struct Requester {
    s16 x;
    s16 y;
    u8 unknown4[0x16];
    ReqButton *buttons;
};

struct IconScreen {
    s32 v[0x156];
};

struct MidiFile {
    char name[0x1e];
    char title[0x1a];
};

extern MidiFile midi_files[];

u8 is_midi_playing(void);
void play_last_midi(void);

#define BSPR(i) (button_sprite + resvars.f12[i])

// FUNCTION: 0x0008dee0
extern "C" void fn_0008dee0(Requester *req)
{
    ReqButton *b;
    u16 idx;
    s32 song;

    b = req->buttons;
    idx = 1;
    switch (crequester.unknown0) {
    case 0:
        break;
    case 1:
        if (crequester.unknown8 == 0) {
            for (; b; b = b->next) {
                if (b->flags & 6) {
                    debug.unknown4 = idx;
                    song = GetSongHandle();
                    if (idx == resvars.fe[0x55]) {
                        if (!othervar.fa50) {
                            if (othervar.fa4f) {
                                AIL_end_sequence(song);
                                play_last_midi();
                            }
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x57]) {
                        if (othervar.fa4f) {
                            stop_midi();
                            othervar.fa4f = 0;
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x54]) {
                        if (othervar.fa4f) {
                            AIL_stop_sequence(song);
                            othervar.fa4f = 0;
                        } else {
                            AIL_resume_sequence(song);
                            othervar.fa4f = 1;
                            if (!is_midi_playing())
                                play_midi(othervar.f9ba);
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x56]) {
                        if (!othervar.fa50) {
                            if (othervar.fa4f)
                                AIL_end_sequence(song);
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x53]) {
                        setup_abort_all_requesters();
                        play_sound(10, 0x42, -1, -1);
                    } else if (idx == resvars.fe[0x58]) {
                        othervar.fa50 = !othervar.fa50 ? (u8)1 : (u8)0;
                        play_sound(10, 0x43, -1, -1);
                    }
                }
                idx++;
            }
        }
        break;
    }
}

// FUNCTION: 0x0008e184
extern "C" void fn_0008e184(Requester *req)
{
    u8 sel;
    u8 i;
    u16 y;
    u16 x;
    s32 song;
    s32 ox;
    s32 oy;
    s32 unused;

    unused = resvars.f12[0x2e] + 0xd;
    compute_help_text(0, 0, 0x14);
    LbScreenSetGraphicsWindow(window.unknown2c3, window.unknown2c7, window.unknown2cb - window.unknown2c3, window.unknown2cf - window.unknown2c7);
    ox = req->x - lbDisplay.graphics_window_x;
    oy = req->y - lbDisplay.graphics_window_y;
    if (lbDisplay.mleft_button)
        crequester.unknown4 = 1;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(2);
    lbDisplay.draw_flags = 0;
    if (othervar.f9b9)
        LbTextDraw(ox + resvars.f1c_IconScreenp->v[0xfe], oy + resvars.f1c_IconScreenp->v[0xff], midi_files[othervar.f9ba].title);
    if (othervar.fa50)
        reqspr_draw(ox + resvars.f1c_IconScreenp->v[0x140], oy + resvars.f1c_IconScreenp->v[0x141], BSPR(0x1f));
    if (othervar.fa4f)
        reqspr_draw(ox + resvars.f1c_IconScreenp->v[0x142], oy + resvars.f1c_IconScreenp->v[0x143], BSPR(0x27));
    i = 0;
    for (y = resvars.f1c_IconScreenp->v[0x101]; y < BSPR(0x21)->height * statvar.f5dd + resvars.f1c_IconScreenp->v[0x101]; y += BSPR(0x21)->height, i++) {
        sel = 0xff;
        x = resvars.f1c_IconScreenp->v[0x100];
        reqspr_draw(x + ox, y + oy, BSPR(0x20));
        set_all_refresh_zones_in_window(x + ox, y + oy, x + ox + BSPR(0x20)->width, y + oy + BSPR(0x20)->height, 3);
        for (; x < BSPR(0x21)->width * 7; x += BSPR(0x21)->width) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x21));
            set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy, x + ox + BSPR(0x20)->width + BSPR(0x21)->width, y + oy + BSPR(0x20)->height + BSPR(0x21)->height, 3);
            if (!lbDisplay.mleft_button && crequester.unknown4) {
                if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, ox + x + BSPR(0x20)->width, oy + y, BSPR(0x21)->width, BSPR(0x21)->height)) {
                    if (i != othervar.f9ba) {
                        othervar.fa50 = 0;
                        othervar.fa4f = 1;
                        play_midi(i);
                    }
                    crequester.unknown4 = 0;
                }
            }
        }
        if (othervar.fa50) {
            if (i == othervar.f9ba)
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
            else
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else if (!myplayer.unknown12[i]) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
        }
        set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy, x + ox + BSPR(0x20)->width + BSPR(0x22)->width, y + oy + BSPR(0x20)->height + BSPR(0x22)->height, 3);
        if (!lbDisplay.mleft_button && crequester.unknown4) {
            if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, ox + x + BSPR(0x20)->width, oy + y, BSPR(0x22)->width, BSPR(0x22)->height)) {
                song = GetSongHandle();
                myplayer.unknown12[i] = !myplayer.unknown12[i] ? (u8)1 : (u8)0;
                play_sound(10, 0x43, -1, -1);
                if (i == othervar.f9ba) {
                    AIL_end_sequence(song);
                    play_next_midi();
                }
                crequester.unknown4 = 0;
            }
        }
        if (othervar.f9b9 && i == othervar.f9ba)
            set_font_from_cache(2);
        else
            set_font_from_cache(1);
        LbTextDraw(ox + resvars.f1c_IconScreenp->v[0x104], y - BSPR(0x21)->height + oy + resvars.f1c_IconScreenp->v[0x105], midi_files[i].title);
    }
    x = resvars.f1c_IconScreenp->v[0x102];
    y -= resvars.f1c_IconScreenp->v[0x103];
    reqspr_draw(x + ox, y + oy, BSPR(0x24));
    set_all_refresh_zones_in_window(x + ox, y + oy, x + ox + BSPR(0x24)->width, y + oy + BSPR(0x24)->height, 3);
    for (; x < BSPR(0x25)->width * 7; x += BSPR(0x25)->width) {
        reqspr_draw(x + ox + BSPR(0x24)->width, y + oy, BSPR(0x25));
        set_all_refresh_zones_in_window(x + ox + BSPR(0x24)->width, y + oy, x + ox + BSPR(0x24)->width + BSPR(0x25)->width, y + oy + BSPR(0x24)->height + BSPR(0x25)->height, 3);
    }
    reqspr_draw(x + ox + BSPR(0x24)->width, y + oy, BSPR(0x26));
    set_all_refresh_zones_in_window(x + ox + BSPR(0x24)->width, y + oy, x + ox + BSPR(0x24)->width + BSPR(0x26)->width, y + oy + BSPR(0x24)->height + BSPR(0x26)->height, 3);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// FUNCTION: 0x0008ee10
void stop_midi(void)
{
    u32 song;

    song = GetSongHandle();
    othervar.fa4f = 0;
    AIL_end_sequence(song);
    othervar.fa50 = 0;
}
