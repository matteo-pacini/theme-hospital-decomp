#include "hosp.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

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

void note_in_error_log(char *text);
void write_to_player_log(char *text);

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

extern "C" {

extern char D_0012083c[], D_0012085a[], D_00120861[], D_00120874[];
extern u8 D_00155e07, D_00155e08, D_00155d71, D_00155d72, D_0015642d;
extern s32 D_00188c3c, D_00188c40, D_00188c38;
extern volatile s32 D_00188c5c, D_00188c60;
extern u8 D_00188c7e;
extern u16 D_00188c74;

char *getenv(char *name);
int vsprintf(char *buf, char *fmt, va_list args);

}

// FUNCTION: 0x0008dcd0
u16 check_frame(u16 anim, Thing *w)
{
    char buf[100];

    if (anim == 9999) {
        sprintf(buf, D_0012083c, (u16)(w - thing));
        note_in_error_log(buf);
        return 0;
    }
    return anim;
}

// FUNCTION: 0x0008dd40
void nav_debug_mode(void)
{
    set_mouse_mode(0xd);
    debug.nav_debug = 1;
}

// FUNCTION: 0x0008dd68
void write_to_player_log(char *text)
{
    char path[64];
    s32 handle;

    sprintf(path, D_00120861, getenv(D_0012085a));
    handle = LbFileOpen(path, 1);
    if (handle != -1) {
        LbFileSeek(handle, 0, 2);
        LbFileWrite(handle, text, fn_000cd716(text));
        LbFileClose(handle);
    }
}

// FUNCTION: 0x0008dde4
void note_in_error_log(char *text)
{
    char buf[128];

    sprintf(buf, D_00120874, text);
    write_to_player_log(buf);
}

// FUNCTION: 0x0008de24
extern "C" void fn_0008de24(void)
{
    if (resvars.f0 == SCREEN_MODE_13)
        resvars.f0 = SCREEN_MODE_1;
    if (!change_res(SCREEN_MODE_13))
        exit(1);
}

// FUNCTION: 0x0008de64
void debug_msg(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(debug.unknown116, fmt, args);
    va_end(args);
    debug.unknown1de = 200;
}

// FUNCTION: 0x0008dea0
extern "C" void fn_0008dea0(char *text)
{
}

// FUNCTION: 0x0008debc
extern "C" void fn_0008debc(char *text)
{
    char buf[132];

    if (0)
        sprintf(buf, text);
}

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
                        if (!D_00155e08) {
                            if (D_00155e07) {
                                AIL_end_sequence(song);
                                play_last_midi();
                            }
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x57]) {
                        if (D_00155e07) {
                            stop_midi();
                            D_00155e07 = 0;
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x54]) {
                        if (D_00155e07) {
                            AIL_stop_sequence(song);
                            D_00155e07 = 0;
                        } else {
                            AIL_resume_sequence(song);
                            D_00155e07 = 1;
                            if (!is_midi_playing())
                                play_midi(D_00155d72);
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x56]) {
                        if (!D_00155e08) {
                            if (D_00155e07)
                                AIL_end_sequence(song);
                        }
                        play_sound(10, 0x43, -1, -1);
                    } else if (idx == resvars.fe[0x53]) {
                        setup_abort_all_requesters();
                        play_sound(10, 0x42, -1, -1);
                    } else if (idx == resvars.fe[0x58]) {
                        D_00155e08 = !D_00155e08 ? (u8)1 : (u8)0;
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
    ox = req->x - D_00188c3c;
    oy = req->y - D_00188c40;
    if (D_00188c7e)
        crequester.unknown4 = 1;
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(2);
    D_00188c74 = 0;
    if (D_00155d71)
        LbTextDraw(ox + resvars.f1c_IconScreenp->v[0xfe], oy + resvars.f1c_IconScreenp->v[0xff], midi_files[D_00155d72].title);
    if (D_00155e08)
        reqspr_draw(ox + resvars.f1c_IconScreenp->v[0x140], oy + resvars.f1c_IconScreenp->v[0x141], BSPR(0x1f));
    if (D_00155e07)
        reqspr_draw(ox + resvars.f1c_IconScreenp->v[0x142], oy + resvars.f1c_IconScreenp->v[0x143], BSPR(0x27));
    i = 0;
    for (y = resvars.f1c_IconScreenp->v[0x101]; y < BSPR(0x21)->height * D_0015642d + resvars.f1c_IconScreenp->v[0x101]; y += BSPR(0x21)->height, i++) {
        sel = 0xff;
        x = resvars.f1c_IconScreenp->v[0x100];
        reqspr_draw(x + ox, y + oy, BSPR(0x20));
        set_all_refresh_zones_in_window(x + ox, y + oy, x + ox + BSPR(0x20)->width, y + oy + BSPR(0x20)->height, 3);
        for (; x < BSPR(0x21)->width * 7; x += BSPR(0x21)->width) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x21));
            set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy, x + ox + BSPR(0x20)->width + BSPR(0x21)->width, y + oy + BSPR(0x20)->height + BSPR(0x21)->height, 3);
            if (!D_00188c7e && crequester.unknown4) {
                if (over_hotspot(D_00188c5c, D_00188c60, ox + x + BSPR(0x20)->width, oy + y, BSPR(0x21)->width, BSPR(0x21)->height)) {
                    if (i != D_00155d72) {
                        D_00155e08 = 0;
                        D_00155e07 = 1;
                        play_midi(i);
                    }
                    crequester.unknown4 = 0;
                }
            }
        }
        if (D_00155e08) {
            if (i == D_00155d72)
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
            else
                reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else if (!myplayer.unknown12[i]) {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x23));
        } else {
            reqspr_draw(x + ox + BSPR(0x20)->width, y + oy, BSPR(0x22));
        }
        set_all_refresh_zones_in_window(x + ox + BSPR(0x20)->width, y + oy, x + ox + BSPR(0x20)->width + BSPR(0x22)->width, y + oy + BSPR(0x20)->height + BSPR(0x22)->height, 3);
        if (!D_00188c7e && crequester.unknown4) {
            if (over_hotspot(D_00188c5c, D_00188c60, ox + x + BSPR(0x20)->width, oy + y, BSPR(0x22)->width, BSPR(0x22)->height)) {
                song = GetSongHandle();
                myplayer.unknown12[i] = !myplayer.unknown12[i] ? (u8)1 : (u8)0;
                play_sound(10, 0x43, -1, -1);
                if (i == D_00155d72) {
                    AIL_end_sequence(song);
                    play_next_midi();
                }
                crequester.unknown4 = 0;
            }
        }
        if (D_00155d71 && i == D_00155d72)
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
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0);
}
