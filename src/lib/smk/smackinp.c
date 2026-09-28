typedef unsigned long u32;
typedef long s32;
typedef unsigned short u16;
typedef unsigned char u8;

#define NULL ((void *)0)

#define SMACKTRACK1 0x02000
#define SMACKTRACKS 0xfe000

void radmemcpy(void *dst, const void *src, unsigned n);
#pragma aux radmemcpy = \
    "cld" \
    "mov bl,cl" \
    "shr ecx,2" \
    "rep movsd" \
    "mov cl,bl" \
    "and cl,3" \
    "rep movsb" \
    parm [edi] [esi] [ecx] \
    modify [ebx ecx esi edi];

u32 mult64anddiv(u32 m1, u32 m2, u32 d);
#pragma aux mult64anddiv = \
    "mul ecx" \
    "div ebx" \
    parm [eax] [ecx] [ebx] \
    value [eax] \
    modify exact [eax edx];

u32 square(u32 x);
#pragma aux square = \
    "mul eax" \
    parm [eax] \
    value [eax] \
    modify exact [eax edx];

void radmemset(void *dst, unsigned char c, unsigned n);
#pragma aux radmemset = \
    "cld" \
    "mov ah,al" \
    "mov bx,ax" \
    "shl eax,16" \
    "mov ax,bx" \
    "mov bl,cl" \
    "shr ecx,2" \
    "and bl,3" \
    "rep stosd" \
    "mov cl,bl" \
    "rep stosb" \
    parm [edi] [al] [ecx] \
    modify [eax ebx ecx edi];

typedef struct SmackSnd {
    u8 *buf;
    u8 *bufend;
    u8 *readpos;
    u8 *writepos;
    u32 f10;
    u32 f14;
    u32 bufsize;
    u32 f1c;
    u32 f20;
    u32 f24;
    u32 f28;
    u32 f2c;
    u32 f30;
    u32 f34;
    u32 f38;
    u32 f3c;
    u32 f40;
    u32 f44;
    u32 f48;
    u32 f4c;
    u32 f50;
    u32 f54;
    u32 f58;
    u32 f5c;
    u8 unknown60[0x84 - 0x60];
} SmackSnd;

typedef struct Smack {
    u32 Version;
    u32 Width;
    u32 Height;
    u32 Frames;
    u32 MSPerFrame;
    u32 SmackerType;
    u32 LargestInTrack[7];
    u32 tablesize;
    u32 codesize;
    u32 absize;
    u32 detailsize;
    u32 typesize;
    u32 TrackType[7];
    u32 extra;
    u32 NewPalette;
    u8 Palette[772];
    u32 PalType;
    u32 FrameNum;
    u32 FrameSize;
    u32 SndSize;
    s32 LastRectx;
    s32 LastRecty;
    s32 LastRectw;
    s32 LastRecth;
    u32 OpenFlags;
    u32 LeftOfs;
    u32 TopOfs;
    u32 unknown39c;
    u32 unknown3a0;
    u32 handle;
    u32 f3a8;
    void *addr32;
    u8 *f3b0;
    u32 f3b4;
    u32 f3b8;
    u8 *f3bc;
    u8 *f3c0;
    u32 f3c4;
    u32 *f3c8;
    u8 *f3cc;
    u8 *f3d0;
    u32 *trackbufs[7];
    u8 *f3f0;
    u32 f3f4;
    u32 f3f8;
    u32 f3fc;
    u32 f400;
    u32 f404;
    u32 f408;
    u32 f40c;
    u32 f410;
    SmackSnd *sounds[7];
    u32 f430;
    u32 f434;
    u32 f438;
    u8 *f43c;
    u8 *f440;
    u8 *f444;
    u8 *f448;
    u32 f44c;
    u32 f450;
    u32 f454;
    u32 f458;
    u32 f45c;
    u32 f460;
    u32 f464;
    u32 f468;
    u32 f46c;
    u32 f470;
    u32 f474;
    u32 f478;
    u32 f47c;
    u32 f480;
    u32 f484;
    u32 f488;
    u32 f48c;
    u32 f490;
    u32 f494;
    u32 f498;
    u32 f49c;
    u32 f4a0;
    u32 f4a4;
    u32 f4a8;
    u32 f4ac;
    u32 f4b0;
    u32 f4b4;
    u32 f4b8;
} Smack;

typedef struct SmackSum {
    u32 TotalTime;
    u32 MS100PerFrame;
    u32 TotalOpenTime;
    u32 TotalFrames;
    u32 SkippedFrames;
    u32 SoundSkips;
    u32 TotalBlitTime;
    u32 TotalReadTime;
    u32 TotalDecompTime;
    u32 TotalBackReadTime;
    u32 TotalReadSpeed;
    u32 SlowestFrameTime;
    u32 Slowest2FrameTime;
    u32 SlowestFrameNum;
    u32 Slowest2FrameNum;
    u32 AverageFrameSize;
    u32 unknown40;
    u32 unknown44;
    u32 HighestMemAmount;
    u32 TotalExtraMemory;
    u32 HighestExtraUsed;
} SmackSum;

extern u32 sounds;
extern u8 *trackbuf;
extern s32 forcerate;
extern u32 simspeed;
extern u8 pallookup[];
extern u32 (__pascal *SmackTimerReadAddr)(void);
extern void (__pascal *SmackTimerDoneAddr)(void);
extern void (__pascal *D_00151b98)(void);
extern void (__pascal *LowSoundCheckAddr)(void);
extern u8 (__pascal *LowSoundOpenAddr)(SmackSnd *snd, u32 flags);
extern void (__pascal *D_0018cdbc)(SmackSnd *snd);
extern u32 (__pascal *LowSoundPlayedAddr)(SmackSnd *snd);
extern void (__pascal *LowSoundPurgeAddr)(SmackSnd *snd);
extern void (__pascal *D_00151688)(SmackSnd *snd);
extern u8 (__pascal *D_0015168c)(SmackSnd *snd);
extern void (__pascal *D_00151690)(SmackSnd *snd, u32 volume, u32 pan);

void *__pascal RADMALLOC(u32 bytes);
void __pascal RADFREE(void *ptr);

u32 __cdecl radopen(const char __far *name, u32 mode);
void __cdecl radclose(u32 handle);
void __cdecl radread(u32 handle, void __far *buf, u32 size, u32 *amount);
void __cdecl radseekbegin(u32 handle, u32 pos);
u32 __cdecl radseekend(u32 handle, u32 pos);

void __cdecl Unsmack(u8 *data, void *addr32, u8 *deltas);
void __cdecl SmackDoPCM(u32 *in, u32 insize, u8 *work, u8 *outbase, u8 *outend, u8 *outpos);
u32 __cdecl SmackGetSizeTables(void);
void __cdecl SmackDoTables(u8 *tables, void *addr32, u32 codesize, u32 absize, u32 detailsize, u32 typesize);
u32 __cdecl SmackGetSizeDeltas(u32 width, u32 height);
void fn_0010f560(void *addr32, const void *trans);
#pragma aux fn_0010f560 "fn_0010f560_" parm caller [] modify [eax ecx edx];
u8 fn_0010f440(u8 *deltas, u32 *rect);
#pragma aux fn_0010f440 "fn_0010f440_" parm caller [] value [al] modify [eax ecx edx];

u8 nexttrack(u32 *flags);
void timeframe(Smack *smk);
u8 doskip(Smack *smk);
void soundnext(Smack *smk);
u8 soundopen(Smack *smk, u8 trk);
void setuptheframe(Smack *smk);
u32 gotoframe(Smack *smk);
u32 readframe(Smack *smk);
u32 blockread(Smack *smk, u8 *buf, u32 size);
u32 backgroundload(Smack *smk);
void *smkmalloc(Smack *smk, u32 bytes);

/* config/functions.txt has no name for nexttrack. */
#pragma aux nexttrack "GetFirstTrack_";

// FUNCTION: 0xea4f0
// NONMATCHING: register allocation and instruction scheduling differ
u8 nexttrack(u32 *flags)
{
    u32 bit;
    u8 i;

    bit = SMACKTRACK1;
    for (i = 0; i < 7; i++) {
        if (*flags & bit) {
            *flags &= ~bit;
            return i;
        }
        bit += bit;
    }
    return 0xff;
}

// FUNCTION: 0xe7f40
u32 __pascal SmackWait(Smack *smk)
{
    u32 played;
    u32 t;
    SmackSnd *snd;

    if (smk == NULL)
        return 0;
    if (smk->unknown39c)
        return 0;
    if (sounds)
        LowSoundCheckAddr();
    if (smk->f430 != -1) {
        if (smk->f434 == 0 || smk->f438 != 0)
            return 0;
        played = LowSoundPlayedAddr(smk->sounds[smk->f430]);
        snd = smk->sounds[smk->f430];
        if (played >= (snd->f20 >> 10) + snd->f24) {
            smk->f434 = 0;
            return 0;
        }
    } else {
        if (smk->f408)
            return 0;
        t = SmackTimerReadAddr() * 100;
        if (smk->f40c == -1) {
            smk->f40c = t + smk->f410;
        } else if (t >= smk->f40c) {
            smk->f40c = smk->f410 + smk->f40c;
            if (t - smk->f40c > smk->f410)
                smk->f40c = t + smk->f410;
            if (smk->f410)
                smk->f408 = 1;
            return 0;
        }
    }
    backgroundload(smk);
    return 1;
}

// FUNCTION: 0x000e8070
void __pascal SmackSoundCheck(void)
{
    if (sounds)
        LowSoundCheckAddr();
}

#pragma aux (__pascal) SmackVolumePan "SMACKVOLUMEPAN";

// FUNCTION: 0xe8080
// NONMATCHING: smk and pan swap esi/edi; the target indexes sounds with
// shl/add instead of a scaled address
void __pascal SmackVolumePan(Smack *smk, u32 trackflag, u32 volume, u32 pan)
{
    u8 trk;
    SmackSnd *snd;

    if (smk == NULL || smk->f430 == -1)
        return;
    if (volume > 0x10000)
        volume = 0x10000;
    if (pan > 0x10000)
        pan = 0x10000;
    while (trackflag & SMACKTRACKS) {
        trk = nexttrack(&trackflag);
        if (trk < 7) {
            snd = smk->sounds[trk];
            if (snd != NULL && D_00151690 != NULL)
                D_00151690(snd, volume, pan);
        }
    }
}

u32 __pascal SmackSoundInTrack(Smack *smk, u32 trackflags);
#pragma aux (__pascal) SmackSoundInTrack "SMACKSOUNDINTRACK";

// FUNCTION: 0xe8120
// NONMATCHING: register allocation and instruction scheduling differ
u32 __pascal SmackSoundInTrack(Smack *smk, u32 trackflags)
{
    u8 trk;

    if (smk != NULL) {
        while (trackflags & SMACKTRACKS) {
            trk = nexttrack(&trackflags);
            if (((u8)(smk->TrackType[trk] >> 24) & 0x40) && (smk->TrackType[trk] & 0xffffff))
                return 1;
        }
    }
    return 0;
}

u32 __pascal SmackToBufferRect(Smack *smk, u32 SmackSurface);
#pragma aux (__pascal) SmackToBufferRect "fn_000e8190_";

// FUNCTION: 0xe8190
// NONMATCHING: register allocation and instruction scheduling differ
u32 __pascal SmackToBufferRect(Smack *smk, u32 SmackSurface)
{
    u32 left;
    u32 top;
    u32 right;
    u32 bottom;
    u32 area;
    u32 nleft;
    u32 nright;
    u32 nbottom;

    if (smk == NULL)
        return 0;
    if (sounds)
        LowSoundCheckAddr();
    if (smk->f3f4 == 0 || SmackSurface == 2)
        return 0;
    if (smk->f3f4 == 3) {
        smk->f3f4 = 0;
        if (SmackSurface != 3)
            return 0;
        goto empty;
    }
    if (smk->f478 == 0) {
        smk->f478 = SmackTimerReadAddr();
        if (smk->f474) {
            smk->f494 += smk->f478 - smk->f474;
            smk->f474 = 0;
        }
    }
    if (smk->f3f4 == 1) {
        if (!fn_0010f440(smk->f3b0, &smk->f3f8)) {
            smk->f3f4 = 0;
            if (!smk->NewPalette)
                return 0;
        empty:
            smk->LastRectx = 0;
            smk->LastRecty = 0;
            smk->LastRectw = 0;
            smk->LastRecth = 0;
            return 1;
        }
        smk->f3f4 = 2;
    }
    left = smk->f3f8;
    top = smk->f3fc;
    right = smk->f400 + left;
    bottom = top + smk->f404;
    if (SmackSurface == 1) {
        while (fn_0010f440(smk->f3b0, &smk->f3f8)) {
            if (left > smk->f3f8)
                left = smk->f3f8;
            if (smk->f3f8 + smk->f400 > right)
                right = smk->f3f8 + smk->f400;
            bottom = smk->f3fc + smk->f404;
        }
        smk->f3f4 = 3;
    } else {
        area = (smk->f400 * smk->f404 * 5) >> 2;
        for (;;) {
            if (!fn_0010f440(smk->f3b0, &smk->f3f8)) {
                smk->f3f4 = 3;
                break;
            }
            area += (smk->f400 * smk->f404 * 5) >> 2;
            nleft = smk->f3f8;
            if (left <= nleft)
                nleft = left;
            nright = smk->f3f8 + smk->f400;
            if (right > nright)
                nright = right;
            nbottom = smk->f3fc + smk->f404;
            if (area < (nbottom - top) * (nright - nleft))
                break;
            right = nright;
            left = nleft;
            bottom = nbottom;
        }
    }
    if (smk->OpenFlags & 0x300000) {
        top += top;
        bottom += bottom;
    }
    smk->LastRectx = left + smk->LeftOfs;
    smk->LastRecty = top + smk->TopOfs;
    smk->LastRectw = right - left;
    smk->LastRecth = bottom - top;
    return 1;
}

void __pascal SmackGoto(Smack *smk, u32 frame);
#pragma aux (__pascal) SmackGoto "SMACKGOTO";

// FUNCTION: 0xe8440
void __pascal SmackGoto(Smack *smk, u32 frame)
{
    if (smk != NULL && smk->unknown39c == 0) {
        if (frame)
            frame--;
        if (frame != smk->FrameNum) {
            smk->FrameNum = frame;
            gotoframe(smk);
            setuptheframe(smk);
        }
    }
}

u32 __pascal SmackGetTrackData(Smack *smk, u8 *dest, u32 trackflag);
#pragma aux (__pascal) SmackGetTrackData "fn_000e8480_";

// FUNCTION: 0xe8480
u32 __pascal SmackGetTrackData(Smack *smk, u8 *dest, u32 trackflag)
{
    u16 trk;
    u32 *src;

    if (smk == NULL)
        return 0;
    if (smk->unknown39c)
        return 0;
    trk = nexttrack(&trackflag);
    if (trk >= 7)
        return 0;
    src = smk->trackbufs[trk];
    if (src == NULL)
        return 0;
    if ((u8)(smk->TrackType[trk] >> 24) & 0x80) {
        if (src[1] != 0 && src[1] != -1) {
            if (trackbuf == NULL) {
                trackbuf = smkmalloc(smk, 0x2000);
                if (trackbuf == NULL)
                    return 0;
            }
            SmackDoPCM(src + 2, src[1], trackbuf, dest, dest + 0x1000000, dest);
        }
        return src[1];
    }
    if (src[0] - 4 != 0 && src[0] - 4 != -1)
        radmemcpy(dest, src + 1, src[0] - 4);
    return src[0] - 4;
}

void __pascal SmackColorRemap(Smack *smk, const u8 *remappal, u32 numcolors, u32 paltype);
#pragma aux (__pascal) SmackColorRemap "fn_000e85b0_";

// FUNCTION: 0xe85b0
// NONMATCHING: register allocation and instruction scheduling differ
void __pascal SmackColorRemap(Smack *smk, const u8 *remappal, u32 numcolors, u32 paltype)
{
    u8 remap[256];
    u32 i;
    const u8 *pal;
    const u8 *end;
    u32 go;
    u32 ro;
    u32 bo;
    u32 r;
    u32 b;
    u32 g;
    u8 best;
    u32 size;
    u32 j;
    u32 bestdist;
    u32 dist;
    const u8 *p;

    if (smk == NULL)
        return;
    if (sounds)
        LowSoundCheckAddr();
    if (numcolors > 256)
        numcolors = 256;
    ro = 0;
    go = 1;
    bo = 2;
    size = 3;
    if (paltype & 4) {
        size = 4;
        if (paltype & 8) {
            ro = 1;
            go = 2;
            bo = 3;
        }
    }
    if (paltype & 2) {
        j = bo;
        bo = ro;
        ro = j;
    }
    if (paltype & 1) {
        size += size;
        ro += ro;
        go += go;
        bo += bo;
    }
    pal = smk->Palette;
    end = remappal + numcolors * size;
    i = 0;
    do {
        p = remappal + i * size;
        r = *pal++;
        g = *pal++;
        b = *pal++;
        bestdist = (u32)-1;
        for (j = 0; j < numcolors; j++, p += size) {
            if (p >= end)
                p = remappal;
            dist = square(r - p[ro]);
            dist += square(g - p[go]);
            dist += square(b - p[bo]);
            if (dist < bestdist) {
                best = (s32)(p - remappal) / (s32)size;
                if (dist == 0)
                    break;
                bestdist = dist;
            }
        }
        remap[i++] = best;
    } while (i < 256);
    fn_0010f560(smk->addr32, remap);
}

void __pascal SmackColorTrans(Smack *smk, const void *trans);
#pragma aux (__pascal) SmackColorTrans "fn_000e8840_";

// FUNCTION: 0xe8840
void __pascal SmackColorTrans(Smack *smk, const void *trans)
{
    if (smk != NULL) {
        if (sounds)
            LowSoundCheckAddr();
        fn_0010f560(smk->addr32, trans);
        if (sounds)
            LowSoundCheckAddr();
    }
}

// FUNCTION: 0xe8890
// NONMATCHING: stack frame layout and register allocation differ
void __pascal SmackToBuffer(Smack *smk, u32 left, u32 top, u32 pitch, u32 destheight, const void *buf, u32 flags)
{
    u32 bpl;
    u32 mult;
    u32 height;
    u32 step;
    u32 row;
    u8 *t;
    u32 *p;

    if (smk == NULL)
        return;
    if (buf == NULL) {
        if (smk->f3b0 != NULL) {
            RADFREE(smk->f3b0);
            smk->f3b0 = NULL;
            smk->f46c -= smk->f3b4;
        }
        return;
    }
    mult = 3;
    bpl = pitch;
    height = smk->Height;
    if (smk->OpenFlags & 0x100000) {
        height >>= 1;
        bpl += bpl;
    } else if (smk->OpenFlags & 0x200000) {
        height >>= 1;
        mult = 7;
    }
    t = smk->f3b0;
    if (t == NULL || *t != 1) {
        if (t != NULL) {
            RADFREE(t);
            smk->f46c -= smk->f3b4;
        }
        smk->f3b4 = SmackGetSizeDeltas(smk->Width, height) + 0x20;
        smk->f3b0 = smkmalloc(smk, smk->f3b4);
        if (smk->f3b0 == NULL)
            return;
        radmemset(smk->f3b0, 0, smk->f3b4);
    }
    smk->TopOfs = top;
    smk->LeftOfs = left;
    t = smk->f3b0;
    t[0] = 1;
    t[1] = (smk->OpenFlags & 0x200000) != 0;
    p = (u32 *)(t + 4);
    row = flags ? destheight - top - 1 : top;
    *p++ = (u32)buf + left + pitch * row;
    step = mult * bpl;
    *p++ = smk->Width >> 2;
    *p++ = height >> 2;
    if (flags) {
        p[0] = -(step + 4);
        p[1] = -step;
        p[2] = -bpl;
        p[3] = -((mult + 2) * bpl - (bpl - smk->Width));
    } else {
        p[0] = step - 4;
        p[1] = step;
        p[2] = bpl;
        p[3] = step + (bpl - smk->Width);
    }
    p += 4;
    *p++ = smk->Width;
    *p = height;
}

u16 *PutWidths(u16 *p, u32 width, u32 pitch, u32 height, u32 lines);

void __pascal SmackToScreen(Smack *smk, u32 left, u32 top, u32 pitch, const u16 *wintbl, u32 banked);
#pragma aux (__pascal) SmackToScreen "fn_000e8ab0_";

// FUNCTION: 0xe8ab0
// NONMATCHING: register allocation, stack slots and block order differ
void __pascal SmackToScreen(Smack *smk, u32 left, u32 top, u32 pitch, const u16 *wintbl, u32 banked)
{
    u8 *prevbuf;
    u32 bank;
    u32 offset;
    u32 extra;
    u32 prevstep4;
    u32 step4;
    u32 prevstep;
    u32 remaining;
    u32 mult;
    u32 bpl;
    u32 rows;
    u32 lines;
    u8 *tmp;
    u32 stride;
    u32 height;
    u32 w;
    u32 i;
    u16 n;
    u8 *t;
    u16 *p;

    prevbuf = NULL;
    if (smk == NULL)
        return;
    if (pitch == 320) {
        banked = 0;
        wintbl = (const u16 *)0xa0000;
    }
    t = smk->f3b0;
    if (t != NULL) {
        if (*t == 1) {
            prevstep = *(u32 *)(t + 0x18);
            prevbuf = *(u8 **)(t + 4);
        }
        RADFREE(smk->f3b0);
        smk->f46c -= smk->f3b4;
    }
    height = smk->Height;
    lines = 4;
    mult = 3;
    stride = pitch;
    bpl = pitch;
    extra = 0;
    if (smk->OpenFlags & 0x100000) {
        bpl = pitch * 2;
        stride = pitch * 2;
        height >>= 1;
    } else if (smk->OpenFlags & 0x200000) {
        lines = 8;
        mult = 7;
        extra = pitch;
        stride = pitch * 2;
        height >>= 1;
    }
    extra += stride * 3 + smk->Width;
    smk->f3b4 = smk->Width * lines * ((top * pitch + stride * height) >> 16) + 0x400;
    smk->f3b0 = smkmalloc(smk, smk->f3b4);
    if (smk->f3b0 == NULL)
        return;
    left &= ~3;
    smk->TopOfs = top;
    smk->LeftOfs = left;
    tmp = smk->f3b0 + 0x400;
    t = smk->f3b0;
    t[0] = 0;
    t[1] = (smk->OpenFlags & 0x200000) != 0;
    offset = left + pitch * top;
    bank = offset >> 16;
    *(u32 *)(t + 4) = banked;
    *(u16 *)(t + 8) = bpl - smk->Width;
    *(u16 *)(t + 10) = banked ? wintbl[bank++] : 0xfffe;
    p = PutWidths((u16 *)(t + 12), bpl, smk->Width, mult, lines);
    if (!banked) {
        *(u32 *)p = (u32)wintbl + offset;
        p[2] = height >> 2;
        p[3] = 0xffff;
        return;
    }
    offset &= 0xffff;
    *(u32 *)p = offset + 0xa0000;
    p += 2;
    rows = 0;
    remaining = smk->Width * height;
    if (remaining > 0) {
        step4 = stride * 4;
        prevstep4 = prevstep * 4;
        do {
            if (offset > 0x10000) {
                *p++ = rows;
                rows = 0;
                *p = wintbl[bank++];
                p = PutWidths(p + 1, bpl, smk->Width, mult, lines);
                offset &= 0xffff;
                *(u32 *)p = offset + 0xa0000;
                p += 2;
                continue;
            }
            if (offset + extra > 0x10000) {
                *p++ = rows;
                *p = 0xfffe;
                rows = 0;
                p = PutWidths(p + 1, smk->Width, smk->Width, mult, lines);
                *(u8 **)p = tmp;
                p += 2;
                *p++ = 1;
                *p = 0xfffd;
                *(u8 **)(p + 1) = tmp;
                p++;
                if (prevbuf != NULL) {
                    for (i = 0; i < lines; i++) {
                        radmemcpy(tmp, prevbuf, smk->Width);
                        tmp += smk->Width;
                        prevbuf += prevstep;
                    }
                } else {
                    tmp += smk->Width * lines;
                }
                p += 4;
                *(u32 *)(p - 2) = offset + 0xa0000;
                n = 0;
                while (offset + bpl <= 0x10000) {
                    n++;
                    offset += bpl;
                }
                *p++ = n;
                if (offset > 0x10000) {
                    *p++ = 0;
                    *p++ = wintbl[bank++];
                    *p++ = 0;
                    *p++ = lines - n;
                } else {
                    w = 0x10000 - offset;
                    if (w > smk->Width)
                        w = smk->Width;
                    p[0] = w >> 2;
                    p[1] = wintbl[bank];
                    p[2] = (smk->Width - w) >> 2;
                    bank++;
                    p[3] = lines - n - 1;
                    p += 4;
                }
                offset = ((lines - n) * bpl + offset) & 0xffff;
                p = PutWidths(p, bpl, smk->Width, mult, lines);
            } else {
                if (prevbuf != NULL)
                    prevbuf += prevstep4;
                rows++;
                offset += step4;
            }
            remaining -= smk->Width * 4;
        } while (remaining > 0);
    }
    *p++ = rows;
    *p = 0xffff;
}

// FUNCTION: 0xe9010
u16 *PutWidths(u16 *p, u32 width, u32 pitch, u32 height, u32 lines)
{
    *p++ = width;
    *p++ = pitch >> 2;
    *p++ = height * width - 4;
    *p++ = height * width;
    *p++ = width * lines - pitch;
    return p;
}

// FUNCTION: 0xe9050
// NONMATCHING: register allocation and instruction scheduling differ
void __pascal SmackNextFrame(Smack *smk)
{
    if (smk == NULL)
        return;
    if (smk->unknown39c) {
        smk->FrameNum++;
        return;
    }
    if (sounds)
        LowSoundCheckAddr();
    timeframe(smk);
    smk->f3c0 += smk->f3c8[smk->FrameNum++];
    if (smk->FrameNum < smk->Frames)
        readframe(smk);
    setuptheframe(smk);
    if (sounds)
        LowSoundCheckAddr();
}

void __pascal SmackSummary(Smack *smk, SmackSum *sum);
#pragma aux (__pascal) SmackSummary "SMACKSUMMARY";

// FUNCTION: 0xe9110
void __pascal SmackSummary(Smack *smk, SmackSum *sum)
{
    u32 total;

    timeframe(smk);
    sum->TotalTime = SmackTimerReadAddr() - smk->f47c;
    sum->MS100PerFrame = smk->f410;
    sum->TotalOpenTime = smk->f480;
    sum->TotalFrames = smk->f488;
    sum->SkippedFrames = smk->f48c;
    sum->TotalBlitTime = smk->f490;
    sum->TotalReadTime = smk->f45c - smk->f484;
    sum->TotalBackReadTime = smk->f464;
    sum->TotalDecompTime = smk->f494;
    total = smk->f45c + smk->f464;
    sum->TotalReadSpeed = total ? mult64anddiv(smk->f458 + smk->f460, 1000, total) : 0;
    sum->SlowestFrameTime = smk->f498;
    sum->SlowestFrameNum = smk->f4a0 + 1;
    sum->Slowest2FrameTime = smk->f49c;
    sum->Slowest2FrameNum = smk->f4a4 + 1;
    sum->AverageFrameSize = smk->f4a8;
    sum->HighestMemAmount = smk->f4ac;
    sum->TotalExtraMemory = smk->f450;
    sum->HighestExtraUsed = smk->f468;
    sum->unknown40 = smk->f4b4;
    sum->unknown44 = smk->f4b8 + 1;
    sum->SoundSkips = smk->f430 != -1 ? smk->sounds[smk->f430]->f34 : 0;
}

// FUNCTION: 0xe9230
void timeframe(Smack *smk)
{
    u32 t;
    u32 d;

    t = (u32)-1;
    if (smk->f470) {
        t = SmackTimerReadAddr();
        d = t - smk->f470;
        smk->f470 = 0;
        if (d > smk->f498) {
            if (smk->f4a0 != smk->FrameNum) {
                smk->f49c = smk->f498;
                smk->f4a4 = smk->f4a0;
                smk->f4a0 = smk->FrameNum;
            }
            smk->f498 = d;
        } else if (d > smk->f49c && smk->f4a0 != smk->FrameNum) {
            smk->f49c = d;
            smk->f4a4 = smk->FrameNum;
        }
    }
    if (smk->f478) {
        if (t == -1)
            t = SmackTimerReadAddr();
        smk->f490 += t - smk->f478;
        smk->f478 = 0;
    }
    if (smk->f474) {
        if (t == -1)
            t = SmackTimerReadAddr();
        smk->f494 += t - smk->f474;
        smk->f474 = 0;
    }
}

// FUNCTION: 0xe9350
u32 __pascal SmackDoFrame(Smack *smk)
{
    u32 t;
    u8 skip;
    u8 nodraw;

    if (smk == NULL)
        return 0;
    if (smk->unknown39c)
        return 0;
    t = SmackTimerReadAddr();
    smk->f474 = t;
    smk->f470 = t;
    if (smk->f47c == 0)
        smk->f47c = t;
    if (smk->NewPalette)
        smk->f3c4 = smk->FrameNum;
    smk->f488++;
    soundnext(smk);
    if (smk->f410 == 0) {
        skip = 0;
        nodraw = 0;
    } else {
        skip = doskip(smk);
        nodraw = skip;
        if (smk->OpenFlags & 0x400)
            nodraw = 0;
    }
    if (smk->f3b0 != NULL) {
        if (nodraw) {
            smk->f48c++;
        } else {
            smk->f3f4 = 1;
            Unsmack(smk->f3f0, smk->addr32, smk->f3b0);
        }
    }
    if (sounds)
        LowSoundCheckAddr();
    return skip;
}

// FUNCTION: 0xe9440
u8 doskip(Smack *smk)
{
    SmackSnd *snd;
    u32 played;

    if (smk->f430 != -1 && smk->f438 == 0) {
        if (smk->f434 == 0)
            return 1;
        snd = smk->sounds[smk->f430];
        if (snd->f4c == 0) {
            played = LowSoundPlayedAddr(snd);
            snd = smk->sounds[smk->f430];
            if (played >= (snd->f28 >> 10) + snd->f24) {
                smk->f434 = 0;
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: 0xe94c0
// NONMATCHING: register allocation and instruction scheduling differ
void soundnext(Smack *smk)
{
    SmackSnd *first;
    u32 tracks;
    u8 trk;
    u32 *src;
    u32 size;
    u32 fill;
    u8 silence;
    SmackSnd *snd;
    u32 n;
    u8 *dst;

    if (smk->f430 == -1 || smk->f438)
        return;
    if (smk->f410)
        smk->f434 = 1;
    first = smk->sounds[smk->f430];
    first->f20 += first->f1c;
    first->f28 += first->f1c;
    if (first->f20 > 0xfa00000 && first->f28 > 0x2000000) {
        first->f20 -= 0xfa00000;
        first->f28 -= 0xfa00000;
        first->f24 += 0x3e800;
    }
    tracks = smk->OpenFlags;
    trk = nexttrack(&tracks);
    src = smk->trackbufs[trk];
    if (src == NULL)
        size = 0;
    else if ((u8)(smk->TrackType[trk] >> 24) & 0x80)
        size = src[1];
    else
        size = src[0] - 4;
    smk->sounds[trk]->f4c = size == 0;
    if (size == 0)
        goto done;
    for (;;) {
        if (sounds)
            LowSoundCheckAddr();
        snd = smk->sounds[trk];
        if (snd->f10 + size > snd->bufsize) {
            size = snd->bufsize - snd->f10;
            snd->f30++;
        }
        if (snd->f5c) {
            snd->f58 = 0;
            snd->f5c = 0;
        }
        fill = snd->f58;
        if (fill) {
            snd->f5c = 1;
            if (size < snd->f38 && snd->f2c) {
                fill = snd->f38 - size;
                if (snd->f10 + size + fill > snd->bufsize)
                    fill = snd->bufsize - snd->f10 - size;
                if (fill) {
                    snd->f34++;
                    silence = snd->f50 ? 0 : 0x80;
                    if (fill + (u32)snd->writepos > (u32)snd->bufend) {
                        radmemset(snd->writepos, silence, snd->bufend - snd->writepos);
                        n = fill - (snd->bufend - snd->writepos);
                        dst = snd->buf;
                    } else {
                        n = fill;
                        dst = snd->writepos;
                    }
                    radmemset(dst, silence, n);
                    snd->f24 += fill;
                    snd->writepos += fill;
                    if (snd->writepos >= snd->bufend)
                        snd->writepos = snd->buf + (snd->writepos - snd->bufend);
                }
            } else {
                fill = 0;
            }
        }
        if ((u8)(smk->TrackType[trk] >> 24) & 0x80) {
            SmackDoPCM(src + 2, size, trackbuf, snd->buf, snd->bufend, snd->writepos);
        } else if (size + (u32)snd->writepos > (u32)snd->bufend) {
            radmemcpy(snd->writepos, src + 1, snd->bufend - snd->writepos);
            n = size - (snd->bufend - snd->writepos);
            radmemcpy(snd->buf, src + 1 + (snd->bufend - snd->writepos), n);
        } else {
            radmemcpy(snd->writepos, src + 1, size);
        }
        snd->f2c += size + fill;
        snd->f10 += size + fill;
        snd->writepos += size;
        if (snd->writepos >= snd->bufend)
            snd->writepos = snd->buf + (snd->writepos - snd->bufend);
        trk = nexttrack(&tracks);
        if (trk == 0xff)
            break;
        src = smk->trackbufs[trk];
        if (src == NULL)
            size = 0;
        else if ((u8)(smk->TrackType[trk] >> 24) & 0x80)
            size = src[1];
        else
            size = src[0] - 4;
    }
done:
    if ((first->f20 >> 10) + first->f24 > first->f2c) {
        first->f28 -= first->f20;
        first->f20 = 0;
        first->f24 = first->f2c;
    }
}

// FUNCTION: 0xe98d0
void __pascal SmackClose(Smack *smk)
{
    u32 i;
    u32 keep;

    if (smk == NULL)
        return;
    if (smk->f430 != -1) {
        for (i = 0; i < 7; i++) {
            if (smk->sounds[i] != NULL) {
                D_0018cdbc(smk->sounds[i]);
                RADFREE(smk->sounds[i]->buf);
                RADFREE(smk->sounds[i]);
                smk->sounds[i] = NULL;
                sounds--;
            }
        }
        smk->f430 = -1;
    }
    if (sounds == 0 && trackbuf != NULL) {
        RADFREE(trackbuf);
        trackbuf = NULL;
    }
    if (smk->handle != -1 && !(smk->OpenFlags & 0x1000))
        radclose(smk->handle);
    if (smk->f3b0 != NULL)
        RADFREE(smk->f3b0);
    if (smk->addr32 != NULL)
        RADFREE(smk->addr32);
    if (smk->f3cc != NULL)
        RADFREE(smk->f3cc);
    if (smk->f3bc != NULL)
        RADFREE(smk->f3bc);
    if (smk->f43c != NULL)
        RADFREE(smk->f43c);
    keep = smk->OpenFlags & 0x2000000;
    radmemset(smk, 0, sizeof(Smack));
    if (!keep)
        RADFREE(smk);
    SmackTimerDoneAddr();
}

u32 __pascal SmackSoundOnOff(Smack *smk, u32 on);
#pragma aux (__pascal) SmackSoundOnOff "SMACKSOUNDONOFF";

// FUNCTION: 0xe9a20
// NONMATCHING: register allocation and instruction scheduling differ
u32 __pascal SmackSoundOnOff(Smack *smk, u32 on)
{
    u32 i;
    SmackSnd *snd;
    u32 behind;
    u32 need;
    u32 fill;
    u32 silence;

    if (smk == NULL)
        return 0;
    if (on) {
        if (smk->f438) {
            smk->f438 = 0;
            for (i = 0; i < 7; i++) {
                snd = smk->sounds[i];
                if (snd == NULL)
                    continue;
                if (D_0015168c != NULL && !D_0015168c(snd))
                    return 0;
                snd->f24 = LowSoundPlayedAddr(snd);
                behind = snd->f2c - snd->f24;
                snd->f28 -= snd->f20;
                snd->f20 = 0;
                if (smk->FrameNum == 0)
                    continue;
                need = (snd->f38 - ((snd->f20 >> 10) + snd->f24)) & ~3;
                if (behind >= need)
                    continue;
                fill = need - behind;
                if (snd->f10 + fill > snd->bufsize)
                    fill = snd->bufsize - snd->f10;
                fill &= ~3;
                silence = snd->f50 ? 0 : 0x80;
                if (snd->writepos + fill >= snd->bufend) {
                    radmemset(snd->writepos, silence, snd->bufend - snd->writepos);
                    radmemset(snd->buf, silence, fill - (snd->bufend - snd->writepos));
                    snd->writepos = snd->buf + fill - (snd->bufend - snd->writepos);
                } else {
                    radmemset(snd->writepos, silence, fill);
                    snd->writepos += fill;
                }
                snd->f10 += fill;
                snd->f2c += fill;
            }
            LowSoundCheckAddr();
            return 1;
        }
    } else if (!smk->f438) {
        smk->f438 = 1;
        for (i = 0; i < 7; i++) {
            if (smk->sounds[i] != NULL) {
                if (D_00151688 != NULL)
                    D_00151688(smk->sounds[i]);
                snd = smk->sounds[i];
                snd->writepos = snd->readpos = snd->buf;
                snd->f2c -= snd->f10;
                snd->f10 = 0;
            }
        }
    }
    LowSoundCheckAddr();
    return 1;
}

// FUNCTION: 0xe9c50
void __pascal SmackFrameRate(s32 rate)
{
    if (rate > 0)
        rate = 100000 / -rate;
    forcerate = rate;
}

// FUNCTION: 0x000e9c70
void __pascal SmackSimulate(u32 speed)
{
    simspeed = speed;
}

// FUNCTION: 0xe9c80
// NONMATCHING: register allocation and instruction scheduling differ
Smack *__pascal SmackOpen(const char *name, u32 flags, u32 extrabuf)
{
    Smack *smk;
    u32 tmp;
    u32 largest;
    u32 start;
    u32 handle;
    u32 highframe;
    u32 left;
    u32 fps;
    u32 *nextsize;
    u32 total;
    u32 frames;
    u32 size;
    u8 *tables;
    u32 i;
    u32 window;
    u32 highest;
    u32 *sz;
    u8 trk;

    largest = 0;
    total = 0;
    if (name == NULL)
        return NULL;
    D_00151b98();
    start = SmackTimerReadAddr();
    if (flags & 0x1000) {
        handle = (flags & 0x2000000) ? *(u32 *)name : (u32)name;
        if (handle != -1)
            tmp = radseekend(handle, 0);
    } else {
        handle = radopen(name, 0);
        tmp = 0;
    }
    if (handle == -1)
        goto fail;
    smk = (Smack *)name;
    if (!(flags & 0x2000000)) {
        smk = RADMALLOC(sizeof(Smack));
        if (smk == NULL)
            goto fail_close;
    }
    radmemset(smk, 0, sizeof(Smack));
    smk->f46c = sizeof(Smack);
    smk->handle = handle;
    smk->f3b8 = tmp;
    if (flags & 0x800)
        smk->f4b0 = simspeed;
    tmp = 0;
    smk->Version = 0;
    blockread(smk, (u8 *)smk, (u32)((u8 *)&smk->extra - (u8 *)smk) + 4);
    if (smk->Version != 0x324b4d53)
        goto fail_free;
    if (flags & 0x80)
        smk->MSPerFrame = forcerate;
    smk->f410 = (s32)smk->MSPerFrame < 0 ? -smk->MSPerFrame : smk->MSPerFrame * 100;
    fps = smk->f410 ? 100000 / smk->f410 : 100000;
    if (fps == 0)
        fps = 1;
    frames = smk->Frames + (smk->SmackerType & 1);
    smk->f3cc = smkmalloc(smk, frames * 5 + 4);
    if (smk->f3cc == NULL)
        goto fail_free;
    smk->f3c8 = (u32 *)(smk->f3cc + frames);
    blockread(smk, (u8 *)smk->f3c8, frames * 4);
    blockread(smk, smk->f3cc, frames);
    smk->f3b8 += frames * 4 + frames;
    size = SmackGetSizeTables() + smk->codesize + smk->absize + smk->detailsize + smk->typesize;
    smk->addr32 = smkmalloc(smk, size);
    if (smk->addr32 == NULL) {
        RADFREE(smk->f3cc);
        goto fail_free;
    }
    size = (smk->tablesize + 0x1003) & ~3;
    if (size < 0x2000)
        size = 0x2000;
    tables = smkmalloc(smk, size);
    if (tables == NULL)
        goto fail_tables;
    blockread(smk, tables + 0x1000, smk->tablesize);
    SmackDoTables(tables, smk->addr32, smk->codesize, smk->absize, smk->detailsize, smk->typesize);
    RADFREE(tables);
    smk->f46c -= smk->tablesize + 0x1000;
    smk->f3b8 += (u32)((u8 *)&smk->extra - (u8 *)smk) + 4 + smk->tablesize;
    frames = smk->Frames - 1 + (smk->SmackerType & 1);
    highest = 0;
    sz = smk->f3c8;
    if (frames < fps) {
        highframe = 0;
        i = 0;
        for (;;) {
            highest += *sz;
            if (*sz > largest)
                largest = *sz;
            total += *sz++;
            if (++i > frames)
                break;
        }
    } else {
        window = 0;
        for (i = 0; i < fps; i++)
            window += *sz++;
        sz = smk->f3c8;
        left = frames - fps;
        i = 0;
        nextsize = sz + fps;
        for (;;) {
            if (window > highest && i != 0) {
                highest = window;
                highframe = i;
            }
            window -= *sz;
            window += *nextsize;
            if (*sz > largest)
                largest = *sz;
            total += *sz++;
            nextsize++;
            if (++i > left)
                break;
        }
        while (i <= frames) {
            if (*sz > largest)
                largest = *sz;
            total += *sz++;
            i++;
        }
    }
    smk->f4b8 = highframe;
    smk->f4b4 = highest;
    smk->f4a8 = total / i;
    if (flags & 0x200) {
        smk->f3a8 = total;
    } else {
        if (extrabuf == -1) {
            extrabuf = smk->f410 ? smk->f4b4 : 0x3000;
            if (extrabuf > total >> 2)
                extrabuf = total >> 2;
            if (extrabuf < 0x3000)
                extrabuf = 0x3000;
        }
        extrabuf = ((extrabuf + 0xfff) >> 12) << 12;
        if (extrabuf < 0x2000)
            extrabuf = 0x2000;
        if (total * 4 / 5 <= extrabuf) {
            flags |= 0x200;
            smk->f3a8 = total;
        } else {
            smk->f450 = extrabuf;
            smk->f3a8 = largest;
            if (extrabuf) {
                smk->f43c = smk->f444 = smk->f448 = smkmalloc(smk, extrabuf + 0x1000);
                if (smk->f43c == NULL)
                    goto fail_tables;
                smk->f440 = smk->f43c + smk->f450;
            }
        }
    }
    smk->f3bc = smkmalloc(smk, smk->f3a8 + 8);
    if (smk->f3bc == NULL) {
        if (smk->f43c != NULL)
            RADFREE(smk->f43c);
        goto fail_tables;
    }
    smk->f454 = smk->f3b8;
    if (flags & 0x200) {
        blockread(smk, smk->f3bc, total);
        if (!(flags & 0x1000))
            radclose(smk->handle);
        smk->handle = -1;
        smk->f3c0 = smk->f3bc;
    } else {
        tmp = smk->f3b8 & 0xfff;
        if (tmp) {
            radread(smk->handle, smk->f43c, 0x1000 - tmp, &tmp);
            smk->f448 = smk->f43c + tmp;
            smk->f44c = tmp;
        }
        readframe(smk);
    }
    smk->f3c4 = -1;
    setuptheframe(smk);
    smk->f430 = -1;
    smk->OpenFlags = flags & 0xffc01fff;
    while (flags & SMACKTRACKS) {
        tmp = nexttrack(&flags);
        smk->sounds[tmp] = smkmalloc(smk, sizeof(SmackSnd));
        if (smk->sounds[tmp] != NULL) {
            if (soundopen(smk, tmp)) {
                smk->OpenFlags |= 1 << (tmp + 13);
            } else {
                RADFREE(smk->sounds[tmp]);
                smk->f46c -= sizeof(SmackSnd);
                smk->sounds[tmp] = NULL;
            }
        }
    }
    smk->f40c = -1;
    if (flags & 0x100) {
        while (backgroundload(smk))
            ;
    }
    switch (flags & 0x300000) {
    case 0x300000:
        smk->SmackerType &= ~6;
        break;
    case 0x100000:
        smk->SmackerType = (smk->SmackerType & ~6) | 2;
        break;
    case 0x200000:
        smk->SmackerType = (smk->SmackerType & ~6) | 4;
        break;
    }
    switch (smk->SmackerType & 6) {
    case 2:
        smk->OpenFlags |= 0x100000;
        break;
    case 4:
        smk->OpenFlags |= 0x200000;
        break;
    }
    if (smk->OpenFlags & 0x300000)
        smk->Height += smk->Height;
    smk->f480 = SmackTimerReadAddr() - start;
    smk->f45c += smk->f464;
    smk->f464 = 0;
    smk->f458 += smk->f460;
    smk->f484 = smk->f45c;
    return smk;

fail_tables:
    RADFREE(smk->addr32);
    RADFREE(smk->f3cc);
fail_free:
    if (!(flags & 0x2000000))
        RADFREE(smk);
fail_close:
    if (!(flags & 0x1000))
        radclose(smk->handle);
fail:
    SmackTimerDoneAddr();
    return NULL;
}

// FUNCTION: 0xea520
// NONMATCHING: register allocation and instruction scheduling differ
u8 soundopen(Smack *smk, u8 trk)
{
    u32 rate;
    u8 stereo;
    u8 bits16;
    u32 bps;

    if (LowSoundOpenAddr == NULL)
        return 0;
    rate = ((u8)(smk->TrackType[trk] >> 24) & 0x40) ? smk->TrackType[trk] & 0xffffff : 0;
    if (rate == 0)
        return 0;
    stereo = (smk->TrackType[trk] >> 28) & 1;
    bits16 = (smk->TrackType[trk] >> 29) & 1;
    bps = (bits16 + 1) * rate * (stereo + 1);
    if (trackbuf == NULL) {
        trackbuf = smkmalloc(smk, 0x2000);
        if (trackbuf == NULL)
            return 0;
    }
    radmemset(smk->sounds[trk], 0, sizeof(SmackSnd));
    smk->sounds[trk]->f1c = mult64anddiv(bps << 10, smk->f410, 100000);
    smk->sounds[trk]->f28 = bps << 9;
    smk->sounds[trk]->bufsize = ((smk->sounds[trk]->f1c >> 9) + bps + 0xfff) & ~0xfff;
    if (smk->sounds[trk]->bufsize < 0x2000)
        smk->sounds[trk]->bufsize = 0x2000;
    smk->sounds[trk]->buf = smk->sounds[trk]->writepos = smk->sounds[trk]->readpos =
        smkmalloc(smk, smk->sounds[trk]->bufsize);
    smk->sounds[trk]->bufend = smk->sounds[trk]->buf + smk->sounds[trk]->bufsize;
    if (smk->sounds[trk]->buf == NULL)
        return 0;
    smk->sounds[trk]->f14 = bps;
    smk->sounds[trk]->f48 = rate;
    smk->sounds[trk]->f50 = bits16;
    smk->sounds[trk]->f54 = stereo;
    if (!LowSoundOpenAddr(smk->sounds[trk], smk->OpenFlags)) {
        RADFREE(smk->sounds[trk]->buf);
        smk->f46c -= smk->sounds[trk]->bufsize;
        return 0;
    }
    smk->sounds[trk]->f38 = smk->LargestInTrack[trk] & ~3;
    if (smk->f430 == -1)
        smk->f430 = trk;
    sounds++;
    return 1;
}

// FUNCTION: 0xea7a0
// NONMATCHING: register allocation and instruction scheduling differ
void setuptheframe(Smack *smk)
{
    u8 oldpal[0x300];
    u32 n;
    u8 *prev;
    u16 i;
    u8 cnt;
    s32 d;
    u8 *p;
    u8 *src;
    u8 *dst;
    u8 flags;
    u8 c;

    smk->f408 = 0;
    d = smk->FrameNum - smk->Frames;
    if (d >= 0) {
        if (smk->SmackerType & 1) {
            if (d == 0) {
                readframe(smk);
                goto frame;
            }
            smk->FrameNum = 1;
        } else {
            smk->FrameNum = 0;
        }
        gotoframe(smk);
    }
frame:
    p = smk->f3c0;
    smk->f3d0 = p;
    flags = smk->f3cc[smk->FrameNum];
    if (flags & 1)
        p += *p * 4;
    smk->trackbufs[0] = (flags & 0x02) ? (u32 *)p : NULL;
    if (smk->trackbufs[0] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[1] = (flags & 0x04) ? (u32 *)p : NULL;
    if (smk->trackbufs[1] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[2] = (flags & 0x08) ? (u32 *)p : NULL;
    if (smk->trackbufs[2] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[3] = (flags & 0x10) ? (u32 *)p : NULL;
    if (smk->trackbufs[3] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[4] = (flags & 0x20) ? (u32 *)p : NULL;
    if (smk->trackbufs[4] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[5] = (flags & 0x40) ? (u32 *)p : NULL;
    if (smk->trackbufs[5] != NULL)
        p += *(u32 *)p;
    smk->trackbufs[6] = (flags & 0x80) ? (u32 *)p : NULL;
    if (smk->trackbufs[6] != NULL)
        p += *(u32 *)p;
    smk->f3f0 = p;
    smk->SndSize = p - smk->f3d0;
    if (flags & 1) {
        dst = smk->Palette;
        i = 0;
        src = smk->f3c0;
        radmemcpy(oldpal, dst, 0x300);
        prev = oldpal;
        src++;
        while (i < 0x100) {
            c = *src;
            if (c & 0x80) {
                cnt = (c & 0x7f) + 1;
                n = cnt * 3;
                radmemcpy(dst, prev + i * 3, n);
                dst += n;
                i += cnt;
                src++;
            } else if (c & 0x40) {
                cnt = (c & 0x3f) + 1;
                n = cnt * 3;
                radmemcpy(dst, prev + src[1] * 3, n);
                src += 2;
                i += cnt;
                dst += n;
            } else {
                *dst++ = pallookup[*src++];
                *dst++ = pallookup[*src++];
                *dst++ = pallookup[*src++];
                i++;
            }
        }
        if (smk->f3c4 != smk->FrameNum) {
            smk->NewPalette = 1;
            return;
        }
    }
    smk->NewPalette = 0;
}

// FUNCTION: 0xeab30
// NONMATCHING: register allocation and instruction scheduling differ
u32 gotoframe(Smack *smk)
{
    u32 pos;
    u32 i;
    u32 n;
    SmackSnd *snd;

    if (smk->FrameNum == 0) {
        radmemset(smk->Palette, 0, sizeof(smk->Palette));
        if (smk->f430 != -1 && smk->sounds[smk->f430]->f2c) {
            n = 1;
            for (i = smk->f430 + 1; i < 7; i++) {
                if (smk->sounds[i] != NULL)
                    n++;
            }
            if (n == sounds) {
                for (i = 0; i < 7; i++) {
                    snd = smk->sounds[i];
                    if (snd != NULL) {
                        LowSoundPurgeAddr(snd);
                        snd->f10 = 0;
                        snd->f24 = 0;
                        snd->f2c = 0;
                        snd->readpos = snd->writepos = snd->buf;
                        snd->f28 -= snd->f20;
                        snd->f20 = 0;
                    }
                }
            }
        }
    }
    pos = 0;
    for (i = 0; i < smk->FrameNum; i++)
        pos += smk->f3c8[i];
    if (smk->OpenFlags & 0x200) {
        smk->f3c0 = smk->f3bc + pos;
        return 0;
    }
    pos += smk->f3b8;
    if (pos >= smk->f454 && smk->f454 + smk->f44c >= pos) {
        n = pos - smk->f454;
        smk->f44c -= n;
        smk->f444 += n;
        if (smk->f444 >= smk->f440) {
            smk->f444 = smk->f43c + (smk->f444 - smk->f440);
            smk->f440 = smk->f43c + smk->f450;
        }
    } else {
        smk->f44c = 0;
        smk->f448 = smk->f43c;
        smk->f444 = smk->f43c;
        smk->f440 = smk->f43c + smk->f450;
        radseekbegin(smk->handle, pos);
        if (pos & 0xfff) {
            smk->f43c = NULL;
            n = blockread(smk, smk->f444, 0x1000 - (pos & 0xfff));
            smk->f44c = n;
            smk->f448 += n;
            smk->f43c = smk->f444;
        }
    }
    smk->f454 = pos;
    return readframe(smk);
}

/* config/functions.txt has no name for readframe. */
#pragma aux readframe "readframe_";

// FUNCTION: 0xeadb0
u32 readframe(Smack *smk)
{
    if (smk->OpenFlags & 0x200)
        return 0;
    smk->f3c0 = smk->f3bc;
    smk->FrameSize = smk->f3c8[smk->FrameNum];
    return blockread(smk, smk->f3bc, smk->FrameSize);
}

// FUNCTION: 0xeae00
// NONMATCHING: register allocation and instruction scheduling differ
u32 blockread(Smack *smk, u8 *buf, u32 size)
{
    u32 amount;
    u32 n;
    u32 start;
    u32 total;
    u32 chunk;

    total = 0;
    amount = 0;
    start = SmackTimerReadAddr();
    if (smk->f43c == NULL) {
        radread(smk->handle, buf, size, &amount);
        total = amount;
        goto timing;
    }
    for (;;) {
        while (smk->f44c) {
            n = smk->f440 - smk->f444;
            if (n > size)
                n = size;
            if (n > smk->f44c)
                n = smk->f44c;
            radmemcpy(buf, smk->f444, n);
            smk->f44c -= n;
            smk->f454 += n;
            smk->f444 += n;
            if (smk->f444 >= smk->f440) {
                smk->f444 = smk->f43c;
                smk->f440 = smk->f43c + smk->f450;
                if (smk->f44c == 0)
                    smk->f448 = smk->f444;
            }
            total += n;
            size -= n;
            buf += n;
            if (size == 0)
                break;
        }
        if (sounds)
            LowSoundCheckAddr();
        while (size > 0x1000) {
            chunk = size > 0x4000 ? 0x4000 : (size >> 12) << 12;
            radread(smk->handle, buf, chunk, &n);
            size -= n;
            buf += n;
            total += n;
            amount += n;
            smk->f454 += n;
            if (sounds)
                LowSoundCheckAddr();
            if (chunk != n && size != 0) {
                smk->unknown39c = 1;
                break;
            }
        }
        if (size == 0)
            break;
        chunk = ((size + 0xfff) >> 12) << 12;
        radread(smk->handle, smk->f43c, chunk, &n);
        amount += n;
        smk->f444 = smk->f43c;
        smk->f448 = smk->f43c + n;
        smk->f44c = n;
        if (chunk != n && size > n) {
            smk->unknown39c = 1;
            break;
        }
    }
timing:
    smk->f458 += amount;
    if (smk->f4b0) {
        amount = start + mult64anddiv(amount, 1000, smk->f4b0);
        do
            n = SmackTimerReadAddr();
        while (n < amount);
    } else {
        n = SmackTimerReadAddr();
    }
    smk->f45c += n - start;
    return total;
}

// FUNCTION: 0xeb0f0
u32 backgroundload(Smack *smk)
{
    u32 amount;
    u32 start;
    u32 until;
    u32 now;

    amount = 0;
    if (smk->FrameNum < smk->Frames && (s32)(smk->f450 - smk->f44c) >= 0x1000) {
        start = SmackTimerReadAddr();
        radread(smk->handle, smk->f448, 0x1000, &amount);
        smk->f44c += amount;
        smk->f448 += amount;
        if (smk->f448 > smk->f440) {
            smk->f440 = smk->f448;
            smk->f448 = smk->f43c;
        }
        if (smk->f44c > smk->f468)
            smk->f468 = smk->f44c;
        smk->f460 += amount;
        if (smk->f4b0) {
            until = mult64anddiv(amount, 1000, smk->f4b0) + start;
            do
                now = SmackTimerReadAddr();
            while (now < until);
        } else {
            now = SmackTimerReadAddr();
        }
        smk->f464 += now - start;
    }
    return amount;
}

// FUNCTION: 0xeb200
void *smkmalloc(Smack *smk, u32 bytes)
{
    smk->f46c += bytes;
    if (smk->f46c > smk->f4ac)
        smk->f4ac = smk->f46c;
    return RADMALLOC(bytes);
}
