#include "../snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

#define MK_FP(s, o) (((unsigned short)(s)):>((void __near *)(o)))
unsigned short FP_SEG(const volatile void __far *p);
#pragma aux FP_SEG = parm caller [eax dx] value [dx];

struct __38rogeSF_DATA {
    u16 count;
    u16 segment;
};

struct AWEInfo {
    s16 samples;
    u32 sample_offset;
    u32 preset_offset;
    u32 preset_size;
};

struct AWEDriver {
    u32 unknown0;
    u32 drvr;
};

struct AWECall {
    s16 AX;
    s16 BX;
    s16 CX;
    s16 DX;
    s16 SI;
    s16 DI;
};

extern long sbkHandle;
extern u8 __far *buffer;
extern u8 __far *preset;
extern __38rogeSF_DATA __far *data;

extern "C" {
extern u8 UseCurrentAwe32Soundfont;
extern u8 Awe32SoundfontLoaded;
extern char CurrentAwe32SoundfontPrefix[];
extern char D_0018c8b4[];
extern char D_00127ff0[];
extern char D_00127ffc[];
extern char D_00127ffe[];
extern char D_00128008[];
extern char D_00128014[];

int stricmp(const char *a, const char *b);
char *strcpy(char *dst, const char *src);
#pragma intrinsic(strcpy)
long filelength(int handle);
long lseek(int handle, long offset, int origin);
unsigned _dos_read(int handle, void __far *buf, unsigned count, unsigned *bytes);
s32 __cdecl AIL_call_driver(u32 drvr, s32 fn, AWECall *in, AWECall *out);

void __far *AllocDOSmem(s32 size);
s32 FreeDOSmem(void __far *p);
s32 CreateSelector(u32 segment);
s32 GetDOSseg(u32 selector);
s32 AWEGetTotalRAM(AWEDriver *mdi);
s32 AWEDefMemMap(AWEDriver *mdi, u16 count, void __far *map);
void __far *AWEGetSFInfo(AWEDriver *mdi, u16 bank, void __far *buf);
s32 AWEStreamSample(AWEDriver *mdi, u16 bank, void __far *buf);
s32 AWELoadPreset(AWEDriver *mdi, u16 bank, void __far *buf);
s32 AWEFreeMem(AWEDriver *mdi, u16 bank);
void LoadAwe32Soundfont(char *prefix);
void FreeAwe32Soundfont(void);
}

// FUNCTION: 0xef390
void __far *AllocDOSmem(s32 size)
{
    REGS r;
    u16 sel;

    r.x.eax = 0x100;
    r.x.ebx = (size + 15) >> 4;
    int386(0x31, &r, &r);
    if (r.x.cflag)
        sel = 0;
    else
        sel = r.x.edx;
    return MK_FP(sel, 0);
}

// FUNCTION: 0xef3d0
s32 FreeDOSmem(void __far *p)
{
    REGS r;

    r.x.eax = 0x101;
    r.x.edx = FP_SEG(p);
    int386(0x31, &r, &r);
    return (r.x.cflag == 0) - 1;
}

// FUNCTION: 0xef410
s32 CreateSelector(u32 segment)
{
    REGS r;

    r.x.eax = 2;
    r.x.ebx = segment;
    int386(0x31, &r, &r);
    if (r.x.cflag)
        return -1;
    return r.w.ax;
}

// FUNCTION: 0xef450
s32 GetDOSseg(u32 selector)
{
    REGS r;

    r.x.eax = 6;
    r.x.ebx = selector;
    int386(0x31, &r, &r);
    if (r.x.cflag)
        return -1;
    return (r.w.cx << 12) + (r.w.dx >> 4);
}

// FUNCTION: 0xef4a0
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
s32 AWEGetTotalRAM(AWEDriver *mdi)
{
    AWECall v;

    v.CX = 0x10;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    if (v.AX == 0)
        return v.DX << (v.SI + 16);
    return -1;
}

// FUNCTION: 0xef4f0
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
s32 AWEDefMemMap(AWEDriver *mdi, u16 count, void __far *map)
{
    AWECall v;

    if (!data) {
        data = (__38rogeSF_DATA __far *)AllocDOSmem(4);
        if (!data)
            return -1;
    }
    data->count = count;
    data->segment = GetDOSseg(FP_SEG(map));
    v.CX = 0x11;
    v.DX = GetDOSseg(FP_SEG(data));
    v.SI = 0;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    return v.AX;
}

// FUNCTION: 0xef5a0
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
void __far *AWEGetSFInfo(AWEDriver *mdi, u16 bank, void __far *buf)
{
    AWECall v;

    if (!data) {
        data = (__38rogeSF_DATA __far *)AllocDOSmem(4);
        if (!data)
            return 0;
    }
    data->count = bank;
    data->segment = GetDOSseg(FP_SEG(buf));
    v.CX = 0x17;
    v.DX = GetDOSseg(FP_SEG(data));
    v.SI = 0;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    if (v.AX == 0)
        return MK_FP(CreateSelector(v.DX), v.SI);
    return 0;
}

// FUNCTION: 0xef670
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
s32 AWEStreamSample(AWEDriver *mdi, u16 bank, void __far *buf)
{
    AWECall v;

    if (!data) {
        data = (__38rogeSF_DATA __far *)AllocDOSmem(4);
        if (!data)
            return -1;
    }
    data->count = bank;
    data->segment = GetDOSseg(FP_SEG(buf));
    v.CX = 0x18;
    v.DX = GetDOSseg(FP_SEG(data));
    v.SI = 0;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    return v.AX;
}

// FUNCTION: 0xef720
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
s32 AWELoadPreset(AWEDriver *mdi, u16 bank, void __far *buf)
{
    AWECall v;

    if (!data) {
        data = (__38rogeSF_DATA __far *)AllocDOSmem(4);
        if (!data)
            return -1;
    }
    data->count = bank;
    data->segment = GetDOSseg(FP_SEG(buf));
    v.CX = 0x19;
    v.DX = GetDOSseg(FP_SEG(data));
    v.SI = 0;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    return v.AX;
}

// FUNCTION: 0xef7d0
// NONMATCHING: s16 driver results read with mov/sar instead of movsx (sound modules were built with -4)
s32 AWEFreeMem(AWEDriver *mdi, u16 bank)
{
    AWECall v;

    FreeDOSmem(data);
    data = 0;
    v.CX = 0x12;
    v.DX = bank;
    AIL_call_driver(mdi->drvr, 0x506, &v, &v);
    return v.AX;
}

// FUNCTION: 0xef830
// NONMATCHING: s16 loads and register allocation differ (sound modules were built with -4)
void LoadAwe32Soundfont(char *prefix)
{
    char path[0x14];
    unsigned nread;
    AWEInfo __far *info;
    s32 size;
    s32 i;

    if (UseCurrentAwe32Soundfont) {
        Awe32SoundfontLoaded = 1;
        return;
    }
    if (!MusicInstalled || !MusicAble || stricmp(D_0018c8b4, D_00127ff0) || stricmp(MusicType, D_00127ffc))
        return;
    if (Awe32SoundfontLoaded == 1) {
        FreeAwe32Soundfont();
        Awe32SoundfontLoaded = 0;
    }
    strcpy(CurrentAwe32SoundfontPrefix, prefix);
    sprintf(path, D_00127ffe, MusicDataPath, CurrentAwe32SoundfontPrefix);
    sbkHandle = LbFileOpen(path, 2);
    if (sbkHandle == -1)
        return;
    size = filelength(sbkHandle);
    buffer = (u8 __far *)AllocDOSmem(0x200);
    if (!buffer)
        return;
    if (AWEGetTotalRAM((AWEDriver *)MusicDriver) == -1)
        return;
    ((u32 __far *)buffer)[0] = 0;
    ((u32 __far *)buffer)[1] = size;
    if (AWEDefMemMap((AWEDriver *)MusicDriver, 2, buffer) != 0)
        return;
    if (_dos_read(sbkHandle, buffer, 0x200, &nread) != 0)
        return;
    info = (AWEInfo __far *)AWEGetSFInfo((AWEDriver *)MusicDriver, 1, buffer);
    if (!info)
        return;
    lseek(sbkHandle, info->sample_offset, 0);
    for (i = 0; i < info->samples; i++) {
        if (_dos_read(sbkHandle, buffer, 0x200, &nread) != 0)
            return;
        if (AWEStreamSample((AWEDriver *)MusicDriver, 1, buffer) == 0)
            continue;
        return;
    }
    preset = (u8 __far *)AllocDOSmem(info->preset_size);
    if (!preset)
        return;
    lseek(sbkHandle, info->preset_offset, 0);
    if (_dos_read(sbkHandle, preset, info->preset_size, &nread) != 0)
        return;
    if (AWELoadPreset((AWEDriver *)MusicDriver, 1, preset) != 0)
        return;
    Awe32SoundfontLoaded = 1;
    LbFileClose(sbkHandle);
}

// FUNCTION: 0xefad0
void FreeAwe32Soundfont(void)
{
    if (!UseCurrentAwe32Soundfont && MusicInstalled && MusicAble && !stricmp(D_0018c8b4, D_00128008)
        && !stricmp(MusicType, D_00128014)) {
        AWEFreeMem((AWEDriver *)MusicDriver, 1);
        FreeDOSmem(buffer);
        FreeDOSmem(preset);
        Awe32SoundfontLoaded = 0;
    }
}
