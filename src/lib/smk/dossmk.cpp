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

extern void *(*smack_malloc)(u32 bytes);
extern void (*smack_free)(void *ptr);
extern void *smack_draw_callback;
extern u8 D_00188c7b;
extern u16 D_00188c78;
extern u32 D_00188c34;
extern u32 D_00188c38;

u32 GetStereoSound(void);
int LbScreenWaitVbi(void);
void fn_000e7910(u32 digdriver);
#pragma aux fn_000e7910 parm routine [];

}

void copy_to_screen(u8 *buf, u32 width, u32 height, u32 flags);
long play_smk_direct(char *name, long a2, long flags);
long play_smk_via_buffer(char *name, long a2, long flags);

extern "C" {

// FUNCTION: 0xca350
// NONMATCHING: config/functions.txt merges this with the smack_free setter
// at 0xca358
void fn_000ca350(void *(*fn)(u32 bytes))
{
    smack_malloc = fn;
}

void fn_000ca358(void (*fn)(void *ptr))
{
    smack_free = fn;
}

// FUNCTION: 0xca360
u8 play_smk(char *name, u32 a2, u32 flags)
{
    D_00188c7b = 0;
    if (smack_draw_callback != 0 || (flags & 0x8c)) {
        flags &= ~0x100;
        return play_smk_via_buffer(name, a2, flags);
    }
    if (D_00188c78 != 1)
        flags &= ~0x100;
    return play_smk_direct(name, a2, flags);
}

}

// FUNCTION: 0xca398
// NONMATCHING: ebp frame here; the original keeps a local in ebp
long play_smk_direct(char *name, long a2, long flags)
{
    Smack *smk;
    u32 frame;
    u8 new_palette;
    int i;

    if ((flags & 1) || !GetStereoSound()) {
        flags |= 1;
    } else {
        fn_000e7910(GetStereoSound());
    }
    smk = SmackOpen(name, (flags & 1) ? 0 : SMACKTRACKS, SMACKAUTOEXTRA);
    if (smk == 0)
        return 0;
    if (flags & 0x100) {
        SmackToBuffer(smk, (320 - smk->Width) >> 1, (200 - smk->Height) >> 1, 320, 200, (void *)0xa0000, 0);
    } else {
        SmackToBuffer(smk, (D_00188c34 - smk->Width) >> 1, (D_00188c38 - smk->Height) >> 1, D_00188c34, D_00188c38, lbDisplay.unknown4, 0);
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
                if (lbKeyOn[1] || lbKeyOn[0x1c] || lbKeyOn[0x39] || D_00188c7b) {
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
// NONMATCHING: ebp frame here; the original keeps buf in ebp
long play_smk_via_buffer(char *name, long a2, long flags)
{
    Smack *smk;
    u8 *buf;
    u32 frame;
    int i;

    if ((flags & 1) || !GetStereoSound()) {
        flags |= 1;
    } else {
        fn_000e7910(GetStereoSound());
    }
    smk = SmackOpen(name, (flags & 1) ? 0 : SMACKTRACKS, SMACKAUTOEXTRA);
    if (smk == 0)
        return 0;
    buf = (u8 *)LbMemoryAlloc(smk->Width * smk->Height);
    if (buf == 0) {
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
                if (lbKeyOn[1] || lbKeyOn[0x1c] || lbKeyOn[0x39] || D_00188c7b) {
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
