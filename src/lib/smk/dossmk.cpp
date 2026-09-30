#include "bflib.h"

struct Smack {
    u32 Version;
    u32 Width;
    u32 Height;
    u32 Frames;
    u8 unknown10[0x58];
    u32 NewPalette;
    u8 Palette[772];
};

#define SMACKTRACKS 0xfe000
#define SMACKAUTOEXTRA 0xffffffff


extern u8 palette[];

extern "C" {

Smack *__pascal SmackOpen(const char *name, u32 flags, u32 extrabuf);
void __pascal SmackClose(Smack *smk);
u32 __pascal SmackDoFrame(Smack *smk);
void __pascal SmackNextFrame(Smack *smk);
u32 __pascal SmackWait(Smack *smk);
void __pascal SmackToBuffer(Smack *smk, u32 left, u32 top, u32 pitch, u32 destheight, const void *buf, u32 flags);

void *(*smack_malloc)(u32 bytes) = LbMemoryAlloc;
void (*smack_free)(void *ptr) = (void (*)(void *))LbMemoryFree;
extern void *smack_draw_callback;

u32 GetSoundDriver(void);
int LbScreenWaitVbi(void);
void SmackSoundUseMSS(u32 digdriver);
#pragma aux (__pascal) SmackSoundUseMSS "SMACKSOUNDUSEMSS";

}

void copy_to_screen(u8 *buf, u32 width, u32 height, u32 flags);
long play_smk_direct(char *name, long a2, long flags);
long play_smk_via_buffer(char *name, long a2, long flags);

extern "C" {

// FUNCTION: 0xca350
void set_smack_malloc(void *(*fn)(u32 bytes))
{
    smack_malloc = fn;
}

// FUNCTION: 0xca358
void set_smack_free(void (*fn)(void *ptr))
{
    smack_free = fn;
}

// FUNCTION: 0xca360
u8 play_smk(char *name, u32 a2, u32 flags)
{
    lbDisplay.left_button = 0;
    if (smack_draw_callback != 0 || (flags & 0x8c)) {
        flags &= ~0x100;
        return play_smk_via_buffer(name, a2, flags);
    }
    if (lbDisplay.screen_mode != 1)
        flags &= ~0x100;
    return play_smk_direct(name, a2, flags);
}

}

// FUNCTION: 0xca398
long play_smk_direct(char *name, long a2, long flags)
{
    Smack *smk;
    u32 frame;
    u8 new_palette;
    int i;
    u32 extra;
    u32 tracks;
    u32 x;
    u32 y;

    extra = SMACKAUTOEXTRA;
    // Both settings are the same value; the original still tests the flag.
    if (flags & 0x200)
        extra = SMACKAUTOEXTRA;
    if (!(flags & 1) && GetSoundDriver()) {
        SmackSoundUseMSS(GetSoundDriver());
    } else {
        flags |= 1;
    }
    tracks = (flags & 1) == 0 ? SMACKTRACKS : 0;
    smk = SmackOpen(name, tracks, extra);
    if (smk == 0)
        return 0;
    if (flags & 0x100) {
        x = (320 - smk->Width) >> 1;
        y = (200 - smk->Height) >> 1;
        SmackToBuffer(smk, x, y, 320, 200, (void *)0xa0000, 0);
    } else {
        x = (lbDisplay.graphics_screen_width - smk->Width) >> 1;
        y = (lbDisplay.graphics_screen_height - smk->Height) >> 1;
        SmackToBuffer(smk, x, y, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height, lbDisplay.unknown4, 0);
    }
    for (frame = 0; (flags & 0x400) || frame < smk->Frames; frame++) {
        if (flags & 0x100) {
            if (smk->NewPalette) {
                for (i = 0; i < 0x300; i++)
                    palette[i] = smk->Palette[i] >> 2;
                LbPaletteSet(palette);
            }
            SmackDoFrame(smk);
        } else {
            new_palette = 0;
            if (smk->NewPalette) {
                new_palette = 1;
                for (i = 0; i < 0x300; i++)
                    palette[i] = smk->Palette[i] >> 2;
            }
            SmackDoFrame(smk);
            if (new_palette) {
                LbScreenWaitVbi();
                LbPaletteSet(palette);
            }
            LbScreenSwap();
        }
        SmackNextFrame(smk);
        do {
            if (!(flags & 2)) {
                if (lbKeyOn[1] || lbKeyOn[0x1c] || lbKeyOn[0x39] || lbDisplay.left_button) {
                    SmackClose(smk);
                    return 2;
                }
            }
        } while (SmackWait(smk));
    }
    SmackClose(smk);
    return 1;
}

// FUNCTION: 0xca568
long play_smk_via_buffer(char *name, long a2, long flags)
{
    Smack *smk;
    u8 *buf;
    u32 frame;
    int i;
    u32 extra;
    u32 tracks;

    extra = SMACKAUTOEXTRA;
    // Both settings are the same value; the original still tests the flag.
    if (flags & 0x200)
        extra = SMACKAUTOEXTRA;
    if (!(flags & 1) && GetSoundDriver()) {
        SmackSoundUseMSS(GetSoundDriver());
    } else {
        flags |= 1;
    }
    tracks = (flags & 1) == 0 ? SMACKTRACKS : 0;
    smk = SmackOpen(name, tracks, extra);
    if (smk == 0)
        return 0;
    buf = (u8 *)LbMemoryAlloc(smk->Width * smk->Height);
    if (!buf) {
        SmackClose(smk);
        return 0;
    }
    SmackToBuffer(smk, 0, 0, smk->Width, smk->Height, buf, 0);
    for (frame = 0; (flags & 0x400) || frame < smk->Frames; frame++) {
        if (smk->NewPalette) {
            for (i = 0; i < 0x300; i++)
                palette[i] = smk->Palette[i] >> 2;
        }
        SmackDoFrame(smk);
        LbScreenWaitVbi();
        if (smk->NewPalette)
            LbPaletteSet(palette);
        copy_to_screen(buf, smk->Width, smk->Height, flags);
        SmackNextFrame(smk);
        while (SmackWait(smk)) {
            if (!(flags & 2)) {
                if (lbKeyOn[1] || lbKeyOn[0x1c] || lbKeyOn[0x39] || lbDisplay.left_button) {
                    SmackClose(smk);
                    LbMemoryFree(buf);
                    return 2;
                }
            }
        }
    }
    SmackClose(smk);
    LbMemoryFree(buf);
    return 1;
}

extern "C" {

// FUNCTION: 0xca6cc
void *__pascal RADMALLOC(u32 bytes)
{
    if (bytes != 0) {
        if (bytes != 0xffffffff)
            return smack_malloc(bytes);
        bytes = 0;
    }
    return (void *)bytes;
}

// FUNCTION: 0xca6e8
void __pascal RADFREE(void *ptr)
{
    smack_free(ptr);
}

}

typedef void (__stdcall *SmackDrawCallback)(u8 *screen, u32 pitch, u32 height);

extern "C" {
void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(memcpy)
}

/* Doubles each pixel across and each row down (two rows written per pass). */
void copy_line_double_both(u8 *dst, u8 *src, u32 width, u32 pitch);
#pragma aux copy_line_double_both = \
    0xc1 0xe9 0x02 0x8b 0x16 0x88 0xf0 0x88 0xf4 0xc1 0xe0 0x10 0x88 0xd0 0x88 0xd4 \
    0x89 0x07 0x89 0x04 0x1f 0xc1 0xea 0x10 0x88 0xf0 0x88 0xf4 0xc1 0xe0 0x10 0x88 \
    0xd0 0x88 0xd4 0x89 0x47 0x04 0x89 0x44 0x1f 0x04 0x83 0xc6 0x04 0x83 0xc7 0x08 \
    0x49 0x75 0xd0 \
    parm [edi] [esi] [ecx] [ebx] modify [eax ecx edx esi edi];

/* Copies one row to two consecutive screen rows. */
void copy_line_double_rows(u8 *dst, u8 *src, u32 width, u32 pitch);
#pragma aux copy_line_double_rows = \
    0xc1 0xe9 0x02 0x8b 0x06 0x89 0x07 0x89 0x04 0x1f 0x83 0xc6 0x04 0x83 0xc7 0x04 \
    0x49 0x75 0xf0 \
    parm [edi] [esi] [ecx] [ebx] modify [eax ecx esi edi];

/* Doubles each pixel across. */
void copy_line_double_width(u8 *dst, u8 *src, u32 width);
#pragma aux copy_line_double_width = \
    0x55 0xc1 0xe8 0x02 0x89 0xc5 0x8b 0x16 0x89 0xd1 0xc1 0xe9 0x10 0x88 0xf0 0x88 \
    0xeb 0x88 0xf4 0x88 0xef 0xc1 0xe0 0x10 0xc1 0xe3 0x10 0x88 0xd0 0x88 0xcb 0x88 \
    0xd4 0x88 0xcf 0x89 0x07 0x89 0x5f 0x04 0x83 0xc6 0x04 0x83 0xc7 0x08 0x4d 0x75 \
    0xd5 0x5d \
    parm [edi] [esi] [eax] modify [eax ebx ecx edx esi edi];

// FUNCTION: 0xca6f8
void copy_to_screen(u8 *buf, u32 width, u32 height, u32 flags)
{
    u8 *dst;
    u32 pitch;
    u32 screen_height;
    u32 i;

    if (lbDisplay.screen_mode == 1 && !smack_draw_callback) {
        dst = (u8 *)0xa0000;
        pitch = 320;
        screen_height = 200;
    } else {
        pitch = lbDisplay.graphics_screen_width;
        dst = lbDisplay.unknown4;
        screen_height = lbDisplay.graphics_screen_height;
    }
    if ((flags & 4) || (flags & 8))
        dst += ((screen_height - height * 2) >> 1) * pitch;
    else
        dst += ((screen_height - height) >> 1) * pitch;
    if (flags & 0x80)
        dst += (pitch - width * 2) >> 1;
    else
        dst += (pitch - width) >> 1;
    if (flags & 4) {
        if (flags & 0x80) {
            for (i = 0; i < height; i++) {
                copy_line_double_both(dst, buf, width, pitch);
                dst += pitch * 2;
                buf += width;
            }
        } else {
            for (i = 0; i < height; i++) {
                copy_line_double_rows(dst, buf, width, pitch);
                dst += pitch * 2;
                buf += width;
            }
        }
    } else if (flags & 0x80) {
        if (flags & 8) {
            for (i = 0; i < height; i++) {
                copy_line_double_width(dst, buf, width);
                dst += pitch * 2;
                buf += width;
            }
        } else {
            for (i = 0; i < height; i++) {
                copy_line_double_width(dst, buf, width);
                dst += pitch;
                buf += width;
            }
        }
    } else if (flags & 8) {
        for (i = 0; i < height; i++) {
            memcpy(dst, buf, width);
            dst += pitch * 2;
            buf += width;
        }
    } else {
        for (i = 0; i < height; i++) {
            memcpy(dst, buf, width);
            dst += pitch;
            buf += width;
        }
    }
    if (lbDisplay.screen_mode != 1 || smack_draw_callback) {
        if (smack_draw_callback)
            ((SmackDrawCallback)smack_draw_callback)(lbDisplay.unknown4, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbScreenSwap();
    } else {
        if (smack_draw_callback)
            ((SmackDrawCallback)smack_draw_callback)((u8 *)0xa0000, 320, 200);
    }
}
