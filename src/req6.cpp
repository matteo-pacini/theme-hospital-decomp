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

#define BSPR(i) (button_sprite + resvars.requester_sprites[i])

// Jukebox clicks: previous and next track (unless looping), stop, play or
// pause, loop the current track on/off (othervar.loop_track), and close.
// FUNCTION: 0x0008dee0
extern "C" void req6(Requester *req)
{
    ReqButton *button;
    u16 idx;
    s32 song;

    button = req->buttons;
    idx = 1;
    switch (crequester.mode) {
    case 0:
        break;
    case 1:
        if (crequester.kind == 0) {
            for (; button; button = button->next) {
                if (button->flags & 6) {
                    debug.last_button = idx;
                    song = GetSongHandle();
                    if (idx == resvars.requester_buttons[0x55]) {
                        if (!othervar.loop_track) {
                            if (othervar.music_playing) {
                                AIL_end_sequence(song);
                                play_last_midi();
                            }
                        }
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    } else if (idx == resvars.requester_buttons[0x57]) {
                        if (othervar.music_playing) {
                            stop_midi();
                            othervar.music_playing = 0;
                        }
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    } else if (idx == resvars.requester_buttons[0x54]) {
                        if (othervar.music_playing) {
                            AIL_stop_sequence(song);
                            othervar.music_playing = 0;
                        } else {
                            AIL_resume_sequence(song);
                            othervar.music_playing = 1;
                            if (!is_midi_playing())
                                play_midi(othervar.midi_track);
                        }
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    } else if (idx == resvars.requester_buttons[0x56]) {
                        if (!othervar.loop_track) {
                            if (othervar.music_playing)
                                AIL_end_sequence(song);
                        }
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    } else if (idx == resvars.requester_buttons[0x53]) {
                        setup_abort_all_requesters();
                        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                    } else if (idx == resvars.requester_buttons[0x58]) {
                        othervar.loop_track = !othervar.loop_track ? (u8)1 : (u8)0;
                        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    }
                }
                idx++;
            }
        }
        break;
    }
}

// Draws the jukebox: the playing track's title, the loop and play lights,
// and the track list; clicking a title plays it, clicking its light turns
// the track on or off in the playlist (myplayer.track_enabled), skipping it if
// it is playing.
// FUNCTION: 0x0008e184
extern "C" void req6_overdraw(Requester *req)
{
    u8 sel;
    u8 i;
    u16 y;
    u16 x;
    s32 song;
    s32 ox;
    s32 oy;
    s32 unused;

    unused = resvars.requester_sprites[0x2e] + 0xd;
    compute_help_text(0, 0, 0x14);
    LbScreenSetGraphicsWindow(window.clip_x0, window.clip_y0, window.clip_x1 - window.clip_x0,
                              window.clip_y1 - window.clip_y0);
    ox = req->x - lbDisplay.graphics_window_x;
    oy = req->y - lbDisplay.graphics_window_y;
    if (lbDisplay.mleft_button)
        crequester.arg = 1;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(2);
    lbDisplay.draw_flags = 0;
    if (othervar.midi_playing)
        LbTextDraw(ox + resvars.coords_IconScreenp->v[0xfe], oy + resvars.coords_IconScreenp->v[0xff],
                   midi_files[othervar.midi_track].title);
    if (othervar.loop_track)
        reqspr_draw(ox + resvars.coords_IconScreenp->v[0x140], oy + resvars.coords_IconScreenp->v[0x141], BSPR(0x1f));
    if (othervar.music_playing)
        reqspr_draw(ox + resvars.coords_IconScreenp->v[0x142], oy + resvars.coords_IconScreenp->v[0x143], BSPR(0x27));
    i = 0;
    for (y = resvars.coords_IconScreenp->v[0x101];
         y < BSPR(0x21)->height * statvar.midi_count + resvars.coords_IconScreenp->v[0x101];
         y += BSPR(0x21)->height, i++) {
        sel = 0xff;
        x = resvars.coords_IconScreenp->v[0x100];
        reqspr_draw(x + ox, y + oy, BSPR(0x20));
        set_all_refresh_zones_in_window(x + ox, y + oy, x + ox + BSPR(0x20)->width, y + oy + BSPR(0x20)->height, 3);
        for (; x < BSPR(0x21)->width * 7; x += BSPR(0x21)->width) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x21));
            set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy,
                                            x + ox + BSPR(0x20)->width + BSPR(0x21)->width,
                                            y + oy + BSPR(0x20)->height + BSPR(0x21)->height, 3);
            if (!lbDisplay.mleft_button && crequester.arg) {
                if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, ox + x + BSPR(0x20)->width, oy + y,
                                 BSPR(0x21)->width, BSPR(0x21)->height)) {
                    if (i != othervar.midi_track) {
                        othervar.loop_track = 0;
                        othervar.music_playing = 1;
                        play_midi(i);
                    }
                    crequester.arg = 0;
                }
            }
        }
        if (othervar.loop_track) {
            if (i == othervar.midi_track)
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
            else
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else if (!myplayer.track_enabled[i]) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
        }
        set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy,
                                        x + ox + BSPR(0x20)->width + BSPR(0x22)->width,
                                        y + oy + BSPR(0x20)->height + BSPR(0x22)->height, 3);
        if (!lbDisplay.mleft_button && crequester.arg) {
            if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, ox + x + BSPR(0x20)->width, oy + y,
                             BSPR(0x22)->width, BSPR(0x22)->height)) {
                song = GetSongHandle();
                myplayer.track_enabled[i] = !myplayer.track_enabled[i] ? (u8)1 : (u8)0;
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                if (i == othervar.midi_track) {
                    AIL_end_sequence(song);
                    play_next_midi();
                }
                crequester.arg = 0;
            }
        }
        if (othervar.midi_playing && i == othervar.midi_track)
            set_font_from_cache(2);
        else
            set_font_from_cache(1);
        LbTextDraw(ox + resvars.coords_IconScreenp->v[0x104],
                   y - BSPR(0x21)->height + oy + resvars.coords_IconScreenp->v[0x105], midi_files[i].title);
    }
    x = resvars.coords_IconScreenp->v[0x102];
    y -= resvars.coords_IconScreenp->v[0x103];
    reqspr_draw(x + ox, y + oy, BSPR(0x24));
    set_all_refresh_zones_in_window(x + ox, y + oy, x + ox + BSPR(0x24)->width, y + oy + BSPR(0x24)->height, 3);
    for (; x < BSPR(0x25)->width * 7; x += BSPR(0x25)->width) {
        reqspr_draw(x + ox + BSPR(0x24)->width, y + oy, BSPR(0x25));
        set_all_refresh_zones_in_window(x + ox + BSPR(0x24)->width, y + oy,
                                        x + ox + BSPR(0x24)->width + BSPR(0x25)->width,
                                        y + oy + BSPR(0x24)->height + BSPR(0x25)->height, 3);
    }
    reqspr_draw(x + ox + BSPR(0x24)->width, y + oy, BSPR(0x26));
    set_all_refresh_zones_in_window(x + ox + BSPR(0x24)->width, y + oy, x + ox + BSPR(0x24)->width + BSPR(0x26)->width,
                                    y + oy + BSPR(0x24)->height + BSPR(0x26)->height, 3);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0);
}

// Stops the music and clears the playing and loop flags (othervar.music_playing,
// fa50).
// FUNCTION: 0x0008ee10
void stop_midi(void)
{
    u32 song;

    song = GetSongHandle();
    othervar.music_playing = 0;
    AIL_end_sequence(song);
    othervar.loop_track = 0;
}
