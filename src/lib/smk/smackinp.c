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
    // 0x10: Bytes in the ring buffer not yet handed to the sound driver
    // (smkmss.c's pending).
    u32 pending;
    // 0x14: Bytes per second of the track (rate x channels x sample size).
    u32 bytespersec;
    u32 bufsize;
    // 0x1c: Bytes of audio per frame, 22.10 fixed point; added to clockfix and
    // lateclockfix each frame.
    u32 frameadd;
    // 0x20: 22.10 fixed-point bytes past clockbase at which the next frame is
    // due (SmackWait).
    u32 clockfix;
    // 0x24: Played-byte count that clockfix and lateclockfix are relative to.
    u32 clockbase;
    // 0x28: 22.10 fixed-point bytes past clockbase after which a frame is late
    // (doskip); starts at half a second of audio.
    u32 lateclockfix;
    // 0x2c: Total bytes put into the ring buffer (audio plus silence padding),
    // less bytes discarded when sound is turned off.
    u32 written;
    // 0x30: Frames whose audio was cut short to fit the ring buffer; nothing
    // reads it.
    u32 overflows;
    // 0x34: Times silence was inserted after the driver starved; reported as
    // SmackSum.SoundSkips.
    u32 soundskips;
    // 0x38: The track's LargestInTrack rounded down to 4: a starved track is
    // padded with silence up to this many bytes per frame.
    u32 minchunk;
    // 0x3c-0x44: the sound driver's next, sample and halfsize (smkmss.c).
    u32 f3c;
    u32 f40;
    u32 f44;
    // 0x48: Sample rate in Hz (smkmss.c's rate).
    u32 rate;
    // 0x4c: Set when the current frame has no audio for the track, so the
    // driver plays out what is pending (smkmss.c's flush).
    u32 flush;
    // 0x50: 16-bit samples (smkmss.c's bits16).
    u32 bits16;
    // 0x54: Stereo samples (smkmss.c's stereo).
    u32 stereo;
    // 0x58: Set by the driver when it ran out of data (smkmss.c's starved);
    // soundnext then pads with silence.
    u32 starved;
    // 0x5c: Set when soundnext handled starved; it clears starved (and itself)
    // on the next frame.
    u32 starvedseen;
    // 0x60: The sound driver's own fields (smkmss.c's played onwards).
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
    // 0x39c: Set when a file read comes up short; stops further decoding.
    u32 readerror;
    u32 unknown3a0;
    u32 handle;
    // 0x3a8: Size of framebuf less its 8 spare bytes: the whole file's frames,
    // or the largest frame when reading ahead.
    u32 framebufsize;
    // 0x3ac: Decoding tables built by SmackDoTables from the file's Huffman
    // trees; passed to Unsmack.
    void *addr32;
    // 0x3b0: Blit table for Unsmack and SmackGetRect, built by SmackToBuffer or
    // SmackToScreen; byte 0 is 1 for a buffer, 0 for the screen.
    u8 *deltas;
    // 0x3b4: Bytes allocated for deltas.
    u32 deltassize;
    // 0x3b8: File position of frame 0's data (after the header, frame tables
    // and Huffman tables).
    u32 firstframepos;
    // 0x3bc: Frame data buffer: the current frame, or all frames when the file
    // was loaded whole (OpenFlags 0x200).
    u8 *framebuf;
    // 0x3c0: The current frame's data within framebuf.
    u8 *framedata;
    // 0x3c4: Frame number SmackDoFrame last played with NewPalette set (-1
    // after open); a palette chunk on that frame does not set NewPalette again.
    u32 palframe;
    // 0x3c8: Per-frame data sizes read from the file (Frames entries, plus one
    // for a ring frame).
    u32 *framesizes;
    // 0x3cc: Per-frame type bytes read from the file (bit 0 palette, bits 1-7
    // tracks 0-6); also the start of the frame table allocation.
    u8 *frametypes;
    // 0x3d0: Start of the current frame's chunks; SndSize is measured from it.
    u8 *framestart;
    // 0x3d4: Each track's audio chunk in the current frame, or NULL.
    u32 *trackbufs[7];
    // 0x3f0: The current frame's video data (after its palette and audio
    // chunks), passed to Unsmack.
    u8 *videodata;
    // 0x3f4: SmackToBufferRect state: 0 nothing to report, 1 frame decoded, 2 a
    // rectangle is in rectx..recth, 3 all reported.
    u32 rectstate;
    // 0x3f8: Changed rectangle filled in by SmackGetRect: left.
    u32 rectx;
    // 0x3fc: See rectx: top.
    u32 recty;
    // 0x400: See rectx: width.
    u32 rectw;
    // 0x404: See rectx: height.
    u32 recth;
    // 0x408: Set by SmackWait (no sound timing) when the frame time has come;
    // cleared by setuptheframe.
    u32 framedue;
    // 0x40c: Timer reading x 100 at which the next frame is due when timing
    // without sound, or -1 before the first.
    u32 nextframetime;
    // 0x410: Frame duration in hundredths of a millisecond (from MSPerFrame), 0
    // for no frame rate; SmackSum.MS100PerFrame.
    u32 ms100perframe;
    SmackSnd *sounds[7];
    // 0x430: First opened sound track, used for timing, or -1 if none.
    u32 timingtrack;
    // 0x434: Set by soundnext each frame (with a frame rate); cleared once the
    // timing track has played up to the frame's time.
    u32 soundwait;
    // 0x438: Sound turned off by SmackSoundOnOff.
    u32 soundoff;
    // 0x43c: Read-ahead ring buffer (readaheadsize bytes plus 4K), or NULL.
    u8 *readahead;
    // 0x440: End of the valid data in readahead; reads wrap there.
    u8 *readaheadend;
    // 0x444: Read position in readahead.
    u8 *readaheadread;
    // 0x448: Write position in readahead.
    u8 *readaheadwrite;
    // 0x44c: Bytes buffered in readahead.
    u32 readaheadcount;
    // 0x450: Size of readahead (the extra buffer); SmackSum.TotalExtraMemory.
    u32 readaheadsize;
    // 0x454: File position of the next byte blockread delivers.
    u32 filepos;
    // 0x458: Bytes read from the file by blockread; SmackOpen also adds
    // backreadbytes read while opening.
    u32 readbytes;
    // 0x45c: Time spent in blockread; SmackOpen also moves in backreadtime
    // spent while opening.
    u32 readtime;
    // 0x460: Bytes read by backgroundload.
    u32 backreadbytes;
    // 0x464: Time spent in backgroundload; SmackSum.TotalBackReadTime.
    u32 backreadtime;
    // 0x468: Most bytes ever buffered in readahead; SmackSum.HighestExtraUsed.
    u32 highestextraused;
    // 0x46c: Bytes currently allocated through smkmalloc.
    u32 memamount;
    // 0x470: Timer reading at SmackDoFrame, for the slowest-frame times; 0 once
    // used.
    u32 framestarttime;
    // 0x474: Timer reading at SmackDoFrame, for the decompression time; 0 once
    // used.
    u32 decompstarttime;
    // 0x478: Timer reading at the first SmackToBufferRect of a frame, for the
    // blit time; 0 once used.
    u32 blitstarttime;
    // 0x47c: Timer reading at the first SmackDoFrame; SmackSum.TotalTime is
    // measured from it.
    u32 firstframetime;
    // 0x480: Time SmackOpen took; SmackSum.TotalOpenTime.
    u32 totalopentime;
    // 0x484: readtime at the end of SmackOpen; subtracted for
    // SmackSum.TotalReadTime.
    u32 openreadtime;
    // 0x488: Frames passed to SmackDoFrame; SmackSum.TotalFrames.
    u32 totalframes;
    // 0x48c: Frames whose video SmackDoFrame skipped as late;
    // SmackSum.SkippedFrames.
    u32 skippedframes;
    // 0x490: SmackSum.TotalBlitTime.
    u32 totalblittime;
    // 0x494: SmackSum.TotalDecompTime.
    u32 totaldecomptime;
    // 0x498: SmackSum.SlowestFrameTime.
    u32 slowestframetime;
    // 0x49c: SmackSum.Slowest2FrameTime.
    u32 slowest2frametime;
    // 0x4a0: Frame number (0-based) of slowestframetime.
    u32 slowestframenum;
    // 0x4a4: Frame number (0-based) of slowest2frametime.
    u32 slowest2framenum;
    // 0x4a8: Total frame data / frame count; SmackSum.AverageFrameSize.
    u32 averageframesize;
    // 0x4ac: Peak memamount; SmackSum.HighestMemAmount.
    u32 highestmemamount;
    // 0x4b0: Simulated read speed in bytes per second (simspeed, OpenFlags
    // 0x800), or 0.
    u32 simreadspeed;
    // 0x4b4: Largest total data size of one second's worth of consecutive
    // frames.
    u32 highest1secsize;
    // 0x4b8: First frame (0-based) of that one-second window.
    u32 highest1secframe;
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
    // 0x40: Smack.highest1secsize: most frame data in one second of frames.
    u32 Highest1SecRate;
    // 0x44: 1-based first frame of that one-second window.
    u32 Highest1SecFrame;
    u32 HighestMemAmount;
    u32 TotalExtraMemory;
    u32 HighestExtraUsed;
} SmackSum;

u32 sounds = 0;
u8 *trackbuf = NULL;
u32 simspeed = 0;
s32 forcerate = 0;
extern u32 (__pascal *SmackTimerReadAddr)(void);
extern void (__pascal *SmackTimerDoneAddr)(void);
extern void (__pascal *SmackTimerSetupAddr)(void);
extern void (__pascal *LowSoundCheckAddr)(void);
u8 (__pascal *LowSoundOpenAddr)(SmackSnd *snd, u32 flags) = NULL;
extern void (__pascal *LowSoundCloseAddr)(SmackSnd *snd);
extern u32 (__pascal *LowSoundPlayedAddr)(SmackSnd *snd);
extern void (__pascal *LowSoundPurgeAddr)(SmackSnd *snd);
void (__pascal *LowSoundOffAddr)(SmackSnd *snd) = NULL;
u8 (__pascal *LowSoundOnAddr)(SmackSnd *snd) = NULL;
void (__pascal *LowSoundVolPanAddr)(SmackSnd *snd, u32 volume, u32 pan) = NULL;
char SmackerVersionString[] = "*** Smacker Version: 2.2k***";
u8 pallookup[64] = {
    0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c,
    0x41, 0x45, 0x49, 0x4d, 0x51, 0x55, 0x59, 0x5d, 0x61, 0x65, 0x69, 0x6d, 0x71, 0x75, 0x79, 0x7d,
    0x82, 0x86, 0x8a, 0x8e, 0x92, 0x96, 0x9a, 0x9e, 0xa2, 0xa6, 0xaa, 0xae, 0xb2, 0xb6, 0xba, 0xbe,
    0xc3, 0xc7, 0xcb, 0xcf, 0xd3, 0xd7, 0xdb, 0xdf, 0xe3, 0xe7, 0xeb, 0xef, 0xf3, 0xf7, 0xfb, 0xff,
};

void *__pascal RADMALLOC(u32 bytes);
void __pascal RADFREE(void *ptr);

u32 __cdecl radopen(const char __far *name, u32 mode);
void __cdecl radclose(u32 handle);
void __cdecl radread(u32 handle, void __far *buf, u32 size, u32 *amount);
void __cdecl radseekbegin(u32 handle, u32 pos);
u32 __cdecl radseekcur(u32 handle, u32 pos);

void __cdecl Unsmack(u8 *data, void *addr32, u8 *deltas);
void __cdecl SmackDoPCM(u32 *in, u32 outsize, u8 *work, u8 *outbase, u8 *outend, u8 *outpos);
u32 __cdecl SmackGetSizeTables(void);
void __cdecl SmackDoTables(u8 *tables, void *addr32, u32 codesize, u32 absize, u32 detailsize, u32 typesize);
u32 __cdecl SmackGetSizeDeltas(u32 width, u32 height);
void SmackRemapTables(void *addr32, const void *trans);
#pragma aux SmackRemapTables "_SmackRemapTables" parm caller [] modify [eax ecx edx];
u8 SmackGetRect(u8 *deltas, u32 *rect);
#pragma aux SmackGetRect "_SmackGetRect" parm caller [] value [al] modify [eax ecx edx];

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

/* config/functions.txt names nexttrack GetFirstTrack_. */
#pragma aux nexttrack "GetFirstTrack_";

#pragma aux (__pascal) SmackVolumePan "SMACKVOLUMEPAN";

u32 __pascal SmackSoundInTrack(Smack *smk, u32 trackflags);
#pragma aux (__pascal) SmackSoundInTrack "SMACKSOUNDINTRACK";

u32 __pascal SmackToBufferRect(Smack *smk, u32 SmackSurface);
#pragma aux (__pascal) SmackToBufferRect "SMACKTOBUFFERRECT";

void __pascal SmackGoto(Smack *smk, u32 frame);
#pragma aux (__pascal) SmackGoto "SMACKGOTO";

u32 __pascal SmackGetTrackData(Smack *smk, u8 *dest, u32 trackflag);
#pragma aux (__pascal) SmackGetTrackData "SMACKGETTRACKDATA";

void __pascal SmackColorRemap(Smack *smk, const u8 *remappal, u32 numcolors, u32 paltype);
#pragma aux (__pascal) SmackColorRemap "SMACKCOLORREMAP";

void __pascal SmackColorTrans(Smack *smk, const void *trans);
#pragma aux (__pascal) SmackColorTrans "SMACKCOLORTRANS";

u16 *PutWidths(u16 *p, u32 bpl, u32 width, u32 mult, u32 lines);

void __pascal SmackToScreen(Smack *smk, u32 left, u32 top, u32 pitch, const u16 *wintbl, u32 banked);
#pragma aux (__pascal) SmackToScreen "SMACKTOSCREEN";

void __pascal SmackSummary(Smack *smk, SmackSum *sum);
#pragma aux (__pascal) SmackSummary "SMACKSUMMARY";

u32 __pascal SmackSoundOnOff(Smack *smk, u32 on);
#pragma aux (__pascal) SmackSoundOnOff "SMACKSOUNDONOFF";

/* Binds readframe to readframe_, the name config/functions.txt gives it. */
#pragma aux readframe "readframe_";

// smkmalloc: RADMALLOC(bytes), adding them to smk's memory total and peak.
// FUNCTION: 0xeb200
void *smkmalloc(Smack *smk, u32 bytes)
{
    smk->memamount += bytes;
    if (smk->memamount > smk->highestmemamount)
        smk->highestmemamount = smk->memamount;
    return RADMALLOC(bytes);
}

// backgroundload: if frames remain and 4K of the read-ahead buffer is free, read 4K of the file into it (throttled to the simulated speed if set) and count the time; returns the bytes read.
// FUNCTION: 0xeb0f0
u32 backgroundload(Smack *smk)
{
    u32 amount;
    u32 start;
    u32 until;
    u32 now;

    amount = 0;
    if (smk->FrameNum < smk->Frames && (s32)(smk->readaheadsize - smk->readaheadcount) >= 0x1000) {
        start = SmackTimerReadAddr();
        radread(smk->handle, smk->readaheadwrite, 0x1000, &amount);
        smk->readaheadcount += amount;
        smk->readaheadwrite += amount;
        if (smk->readaheadwrite > smk->readaheadend) {
            smk->readaheadend = smk->readaheadwrite;
            smk->readaheadwrite = smk->readahead;
        }
        if (smk->readaheadcount > smk->highestextraused)
            smk->highestextraused = smk->readaheadcount;
        smk->backreadbytes += amount;
        if (smk->simreadspeed) {
            until = mult64anddiv(amount, 1000, smk->simreadspeed) + start;
            do
                now = SmackTimerReadAddr();
            while (now < until);
        } else {
            now = SmackTimerReadAddr();
        }
        smk->backreadtime += now - start;
    }
    return amount;
}

// blockread: read size bytes of the file into buf (all directly when there is no read-ahead buffer), from the read-ahead buffer first, then directly in up to 16K chunks, refilling the buffer for the tail; a short read sets the read-error flag; returns the bytes delivered.
// FUNCTION: 0xeae00
u32 blockread(Smack *smk, u8 *buf, u32 size)
{
    u32 n;
    u32 amount;
    u32 start;
    u32 total;
    u32 chunk;
    u32 now;

    total = 0;
    amount = 0;
    start = SmackTimerReadAddr();
    if (smk->readahead == NULL) {
        radread(smk->handle, buf, size, &amount);
        total = amount;
        goto timing;
    }
    for (;;) {
        while (smk->readaheadcount) {
            n = smk->readaheadend - smk->readaheadread;
            if (n > size)
                n = size;
            if (smk->readaheadcount < n)
                n = smk->readaheadcount;
            radmemcpy(buf, smk->readaheadread, n);
            smk->readaheadcount -= n;
            smk->filepos += n;
            smk->readaheadread += n;
            if (smk->readaheadread >= smk->readaheadend) {
                smk->readaheadread = smk->readahead;
                smk->readaheadend = smk->readahead + smk->readaheadsize;
                if (smk->readaheadcount == 0)
                    smk->readaheadwrite = smk->readaheadread;
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
            smk->filepos += n;
            if (sounds)
                LowSoundCheckAddr();
            if (chunk != n && size != 0) {
                smk->readerror = 1;
                break;
            }
        }
        if (size == 0)
            break;
        chunk = ((size + 0xfff) >> 12) << 12;
        radread(smk->handle, smk->readahead, chunk, &n);
        amount += n;
        smk->readaheadread = smk->readahead;
        smk->readaheadwrite = smk->readahead + n;
        smk->readaheadcount = n;
        if (chunk != n && size > n) {
            smk->readerror = 1;
            break;
        }
    }
timing:
    smk->readbytes += amount;
    if (smk->simreadspeed) {
        amount = start + mult64anddiv(amount, 1000, smk->simreadspeed);
        do
            now = SmackTimerReadAddr();
        while (now < amount);
    } else {
        now = SmackTimerReadAddr();
    }
    smk->readtime += now - start;
    return total;
}

// readframe: read the current frame's data into the frame buffer (nothing if the whole file is in memory); returns the bytes read.
// FUNCTION: 0xeadb0
u32 readframe(Smack *smk)
{
    if (smk->OpenFlags & 0x200)
        return 0;
    smk->framedata = smk->framebuf;
    smk->FrameSize = smk->framesizes[smk->FrameNum];
    return blockread(smk, smk->framebuf, smk->FrameSize);
}

// gotoframe: reposition on FrameNum: on frame 0 clear the palette and, once sound has been queued, reset the sound buffers; then point at the frame (whole file in memory, returning 0), or reuse read-ahead data or seek to the frame, and read it; returns the bytes read.
// FUNCTION: 0xeab30
// NONMATCHING: in the else branch the target stores readaheadwrite and
// readaheadread before readaheadend and adds readaheadsize to readahead in
// place; written in that order, ours adds to a copy of readahead and loads
// readaheadsize late (further off than this order). The inlined readframe
// loads framesizes before FrameNum (the target loads FrameNum first)
u32 gotoframe(Smack *smk)
{
    u32 pos;
    u32 i;
    u32 n;
    SmackSnd *snd;

    if (smk->FrameNum == 0) {
        radmemset(smk->Palette, 0, sizeof(smk->Palette));
        if (smk->timingtrack != -1 && smk->sounds[smk->timingtrack]->written) {
            n = 1;
            for (i = smk->timingtrack + 1; i < 7; i++) {
                if (smk->sounds[i] != NULL)
                    n++;
            }
            if (n == sounds) {
                for (i = 0; i < 7; i++) {
                    snd = smk->sounds[i];
                    if (snd != NULL) {
                        LowSoundPurgeAddr(snd);
                        snd->pending = 0;
                        snd->clockbase = 0;
                        snd->readpos = snd->writepos = snd->buf;
                        snd->written = 0;
                        snd->lateclockfix -= snd->clockfix;
                        snd->clockfix = 0;
                    }
                }
            }
        }
    }
    pos = 0;
    for (i = 0; i < smk->FrameNum; i++)
        pos += smk->framesizes[i];
    if (smk->OpenFlags & 0x200) {
        smk->framedata = smk->framebuf + pos;
        return 0;
    }
    pos += smk->firstframepos;
    if (pos >= smk->filepos && smk->filepos + smk->readaheadcount >= pos) {
        n = pos - smk->filepos;
        smk->readaheadcount -= n;
        smk->readaheadread += n;
        if (smk->readaheadread >= smk->readaheadend) {
            smk->readaheadread = smk->readahead + (smk->readaheadread - smk->readaheadend);
            smk->readaheadend = smk->readahead + smk->readaheadsize;
        }
    } else {
        smk->readaheadcount = 0;
        smk->readaheadend = smk->readahead + smk->readaheadsize;
        smk->readaheadwrite = smk->readahead;
        smk->readaheadread = smk->readahead;
        radseekbegin(smk->handle, pos);
        n = pos & 0xfff;
        if (n) {
            smk->readahead = NULL;
            n = blockread(smk, smk->readaheadread, 0x1000 - n);
            smk->readaheadcount = n;
            smk->readaheadwrite += n;
            smk->readahead = smk->readaheadread;
        }
    }
    smk->filepos = pos;
    if (smk->OpenFlags & 0x200)
        return 0;
    smk->framedata = smk->framebuf;
    smk->FrameSize = smk->framesizes[smk->FrameNum];
    return blockread(smk, smk->framebuf, smk->FrameSize);
}

// setuptheframe: prepare the current frame (wrapping past the last one): locate its per-track audio chunks and video data and decode its palette chunk, setting NewPalette when the palette changed.
// FUNCTION: 0xea7a0
// NONMATCHING: register allocation: the inlined readframe indexes framesizes
// (eax) by FrameNum (edx), ours the reverse, and in the palette loop the
// target keeps c in cl, src in edx and dst in ebp
void setuptheframe(Smack *smk)
{
    u8 oldpal[0x300];
    u32 n;
    u8 *prev;
    u16 i;
    u8 copy_count;
    u8 keep_count;
    s32 past_end;
    u8 *chunk;
    u8 *src;
    u8 *dst;
    u8 flags;
    u8 c;

    smk->framedue = 0;
    past_end = smk->FrameNum - smk->Frames;
    if (past_end >= 0) {
        if (smk->SmackerType & 1) {
            if (past_end == 0) {
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
    chunk = smk->framedata;
    smk->framestart = chunk;
    flags = smk->frametypes[smk->FrameNum];
    if (flags & 1)
        chunk += *chunk * 4;
    smk->trackbufs[0] = (flags & 0x02) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[0] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[1] = (flags & 0x04) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[1] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[2] = (flags & 0x08) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[2] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[3] = (flags & 0x10) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[3] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[4] = (flags & 0x20) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[4] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[5] = (flags & 0x40) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[5] != NULL)
        chunk += *(u32 *)chunk;
    smk->trackbufs[6] = (flags & 0x80) ? (u32 *)chunk : NULL;
    if (smk->trackbufs[6] != NULL)
        chunk += *(u32 *)chunk;
    smk->videodata = chunk;
    smk->SndSize = chunk - smk->framestart;
    if (flags & 1) {
        dst = smk->Palette;
        i = 0;
        src = smk->framedata;
        radmemcpy(oldpal, dst, 0x300);
        prev = oldpal;
        src++;
        while (i < 0x100) {
            c = *src;
            if (c & 0x80) {
                keep_count = (c & 0x7f) + 1;
                n = keep_count * 3;
                radmemcpy(dst, prev + i * 3, n);
                dst += n;
                i += keep_count;
                src++;
            } else if (c & 0x40) {
                copy_count = (c & 0x3f) + 1;
                n = copy_count * 3;
                radmemcpy(dst, prev + src[1] * 3, n);
                src += 2;
                i += copy_count;
                dst += n;
            } else {
                *dst++ = pallookup[c];
                src++;
                *dst++ = pallookup[*src++];
                *dst++ = pallookup[*src++];
                i++;
            }
        }
        if (smk->palframe != smk->FrameNum) {
            smk->NewPalette = 1;
            return;
        }
    }
    smk->NewPalette = 0;
}

// soundopen: open sound track trk (rate, stereo and 16-bit from TrackType): allocate its ring buffer and open it through the low-level sound driver; returns 1 on success.
// FUNCTION: 0xea520
u8 soundopen(Smack *smk, u8 trk)
{
    u32 rate;
    u8 stereo;
    u8 bits16;
    u32 bps;

    if (LowSoundOpenAddr == NULL)
        return 0;
    rate = ((smk->TrackType[trk] >> 24) & 0x40) ? smk->TrackType[trk] & 0xffffff : 0;
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
    smk->sounds[trk]->frameadd = mult64anddiv(bps << 10, smk->ms100perframe, 100000);
    smk->sounds[trk]->lateclockfix = bps << 9;
    smk->sounds[trk]->bufsize = ((smk->sounds[trk]->frameadd >> 9) + bps + 0xfff) & ~0xfff;
    if (smk->sounds[trk]->bufsize < 0x2000)
        smk->sounds[trk]->bufsize = 0x2000;
    smk->sounds[trk]->bufend =
        (smk->sounds[trk]->buf = smk->sounds[trk]->writepos = smk->sounds[trk]->readpos =
             smkmalloc(smk, smk->sounds[trk]->bufsize))
        + smk->sounds[trk]->bufsize;
    if (smk->sounds[trk]->buf == NULL)
        return 0;
    smk->sounds[trk]->bytespersec = bps;
    smk->sounds[trk]->rate = rate;
    smk->sounds[trk]->bits16 = bits16;
    smk->sounds[trk]->stereo = stereo;
    if (!LowSoundOpenAddr(smk->sounds[trk], smk->OpenFlags)) {
        RADFREE(smk->sounds[trk]->buf);
        smk->memamount -= smk->sounds[trk]->bufsize;
        return 0;
    }
    smk->sounds[trk]->minchunk = smk->LargestInTrack[trk] & ~3;
    if (smk->timingtrack == -1)
        smk->timingtrack = trk;
    sounds++;
    return 1;
}

// nexttrack: index (0-6) of the lowest SMACKTRACK bit set in *flags, clearing it; 0xff if none.
// FUNCTION: 0xea4f0
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
        bit *= 2;
    }
    return 0xff;
}

// SmackOpen: open Smacker file name (with flag 0x1000 name is an open file handle; with 0x2000000 the Smack is built in the memory at name, whose first dword holds the handle when 0x1000 is also set): read the header and frame tables, build the decoding tables, set up read-ahead or load the whole file, prepare the first frame and open the requested sound tracks; NULL on failure. When allocating the Smack fails or the header is bad, the file is closed through the null or freed smk.
// FUNCTION: 0xe9c80
// NONMATCHING: the spill slots of largest, start, handle, left, fps and
// nextsize are ordered differently, the inlined readframe loads FrameNum
// before framesizes, and the target's inlined nexttrack keeps the mask in eax and
// the index in dl, the reverse of its out-of-line GetFirstTrack (ours inlines
// the GetFirstTrack allocation)
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
    u32 size;
    u8 *tables;
    u32 i;
    u32 window;
    u32 highest;
    u32 *frame_size;
    u8 trk;

    largest = 0;
    total = 0;
    if (name == NULL)
        return NULL;
    SmackTimerSetupAddr();
    start = SmackTimerReadAddr();
    if (flags & 0x1000) {
        handle = (flags & 0x2000000) ? *(u32 *)name : (u32)name;
        if (handle != -1)
            tmp = radseekcur(handle, 0);
    } else {
        handle = radopen(name, 0);
        tmp = 0;
    }
    if (handle == -1) {
    fail:
        SmackTimerDoneAddr();
        return NULL;
    }
    smk = (Smack *)name;
    if (!(flags & 0x2000000)) {
        smk = RADMALLOC(sizeof(Smack));
        if (smk == NULL) {
        fail_close:
            if (flags & 0x1000)
                goto fail;
            radclose(smk->handle);
            SmackTimerDoneAddr();
            return NULL;
        }
    }
    radmemset(smk, 0, sizeof(Smack));
    smk->memamount = sizeof(Smack);
    smk->handle = handle;
    smk->firstframepos = tmp;
    if (flags & 0x800)
        smk->simreadspeed = simspeed;
    tmp = 0;
    smk->Version = 0;
    blockread(smk, (u8 *)smk, (u32)((u8 *)&smk->extra - (u8 *)smk) + 4);
    if (smk->Version != 0x324b4d53) {
    fail_free:
        if (!(flags & 0x2000000))
            RADFREE(smk);
        goto fail_close;
    }
    if (flags & 0x80)
        smk->MSPerFrame = forcerate;
    smk->ms100perframe = (s32)smk->MSPerFrame < 0 ? -smk->MSPerFrame : smk->MSPerFrame * 100;
    fps = smk->ms100perframe ? 100000 / smk->ms100perframe : 100000;
    if (fps == 0)
        ++fps;
    tmp = smk->Frames + (smk->SmackerType & 1);
    smk->frametypes = smkmalloc(smk, tmp * 5 + 4);
    if (smk->frametypes == NULL)
        goto fail_free;
    smk->framesizes = (u32 *)(smk->frametypes + tmp);
    blockread(smk, (u8 *)smk->framesizes, tmp * 4);
    blockread(smk, smk->frametypes, tmp);
    smk->firstframepos += tmp * 4 + tmp;
    size = SmackGetSizeTables() + smk->codesize + smk->absize + smk->detailsize + smk->typesize;
    smk->addr32 = smkmalloc(smk, size);
    if (smk->addr32 == NULL) {
        RADFREE(smk->frametypes);
        goto fail_free;
    }
    size = (smk->tablesize + 0x1003) & ~3;
    tables = smkmalloc(smk, size < 0x2000 ? 0x2000 : size);
    if (tables == NULL) {
    fail_tables:
        RADFREE(smk->addr32);
        RADFREE(smk->frametypes);
        goto fail_free;
    }
    blockread(smk, tables + 0x1000, smk->tablesize);
    SmackDoTables(tables, smk->addr32, smk->codesize, smk->absize, smk->detailsize, smk->typesize);
    RADFREE(tables);
    smk->memamount -= smk->tablesize + 0x1000;
    smk->firstframepos += (u32)((u8 *)&smk->extra - (u8 *)smk) + 4 + smk->tablesize;
    tmp = smk->Frames - 1 + (smk->SmackerType & 1);
    highest = 0;
    frame_size = smk->framesizes;
    if (tmp < fps) {
        highframe = 0;
        i = 0;
        for (;;) {
            highest += *frame_size;
            if (*frame_size > largest)
                largest = *frame_size;
            total += *frame_size;
            frame_size++;
            if (++i > tmp)
                break;
        }
    } else {
        window = 0;
        for (i = 0; i < fps; i++)
            window += *frame_size++;
        frame_size = smk->framesizes;
        left = tmp - fps;
        i = 0;
        nextsize = frame_size + fps;
        for (;;) {
            if (window > highest && i != 0) {
                highest = window;
                highframe = i;
            }
            window -= *frame_size;
            window += *nextsize;
            if (*frame_size > largest)
                largest = *frame_size;
            total += *frame_size;
            frame_size++;
            nextsize++;
            if (++i > left)
                break;
        }
        for (; i <= tmp; i++) {
            if (*frame_size > largest)
                largest = *frame_size;
            total += *frame_size;
            frame_size++;
        }
    }
    smk->highest1secframe = highframe;
    smk->highest1secsize = highest;
    smk->averageframesize = total / i;
    if (flags & 0x200) {
        smk->framebufsize = total;
    } else {
        if (extrabuf == -1) {
            extrabuf = smk->ms100perframe ? smk->highest1secsize : 0x3000;
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
            smk->framebufsize = total;
        } else {
            smk->readaheadsize = extrabuf;
            smk->framebufsize = largest;
            if (extrabuf) {
                smk->readaheadwrite = smk->readaheadread = smk->readahead = smkmalloc(smk, extrabuf + 0x1000);
                if (smk->readahead == NULL)
                    goto fail_tables;
                smk->readaheadend = smk->readahead + smk->readaheadsize;
            }
        }
    }
    smk->framebuf = smkmalloc(smk, smk->framebufsize + 8);
    if (smk->framebuf == NULL) {
        if (smk->readahead != NULL)
            RADFREE(smk->readahead);
        goto fail_tables;
    }
    smk->filepos = smk->firstframepos;
    if (flags & 0x200) {
        blockread(smk, smk->framebuf, total);
        if (!(flags & 0x1000))
            radclose(smk->handle);
        smk->handle = -1;
        smk->framedata = smk->framebuf;
    } else {
        tmp = smk->firstframepos & 0xfff;
        if (tmp) {
            radread(smk->handle, smk->readahead, 0x1000 - tmp, &tmp);
            smk->readaheadwrite = smk->readahead + tmp;
            smk->readaheadcount = tmp;
        }
        readframe(smk);
    }
    smk->palframe = -1;
    setuptheframe(smk);
    smk->timingtrack = -1;
    smk->OpenFlags = flags & 0xffc01fff;
    while (flags & SMACKTRACKS) {
        tmp = nexttrack(&flags);
        smk->sounds[tmp] = smkmalloc(smk, sizeof(SmackSnd));
        if (smk->sounds[tmp] != NULL) {
            if (soundopen(smk, tmp)) {
                smk->OpenFlags |= 1 << (tmp + 13);
            } else {
                RADFREE(smk->sounds[tmp]);
                smk->memamount -= sizeof(SmackSnd);
                smk->sounds[tmp] = NULL;
            }
        }
    }
    smk->nextframetime = -1;
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
    smk->totalopentime = SmackTimerReadAddr() - start;
    smk->readtime += smk->backreadtime;
    smk->backreadtime = 0;
    smk->readbytes += smk->backreadbytes;
    smk->openreadtime = smk->readtime;
    return smk;
}

// SmackSimulate: simulate a read speed of speed bytes per second for files opened with flag 0x800.
// FUNCTION: 0x000e9c70
void __pascal SmackSimulate(u32 speed)
{
    simspeed = speed;
}

// SmackFrameRate: frame rate used instead of the file's for files opened with flag 0x80 (rate > 0 in frames per second; otherwise used as a header MSPerFrame value).
// FUNCTION: 0xe9c50
void __pascal SmackFrameRate(s32 rate)
{
    if (rate > 0)
        rate = 100000 / -rate;
    forcerate = rate;
}

// SmackSoundOnOff: turn smk's sound tracks on (padding them with silence to catch up) or off (stopping and emptying them); returns 0 if a track cannot be restarted, else 1.
// FUNCTION: 0xe9a20
// NONMATCHING: register allocation; the target keeps behind in ebp and
// spills the sounds[] cursor (one more dword of frame), loads LowSoundOnAddr into
// a register before testing it, and does not keep 0 in a register in the
// second loop
u32 __pascal SmackSoundOnOff(Smack *smk, u32 on)
{
    u32 i;
    SmackSnd *snd;
    u32 behind;
    u32 need;

    if (smk == NULL)
        return 0;
    if (on) {
        if (smk->soundoff) {
            smk->soundoff = 0;
            for (i = 0; i < 7; i++) {
                snd = smk->sounds[i];
                if (snd == NULL)
                    continue;
                if (LowSoundOnAddr != NULL && !LowSoundOnAddr(snd))
                    return 0;
                snd->clockbase = LowSoundPlayedAddr(snd);
                behind = snd->written - snd->clockbase;
                snd->lateclockfix -= snd->clockfix;
                snd->clockfix = 0;
                if (smk->FrameNum == 0)
                    continue;
                need = (snd->minchunk - ((snd->clockfix >> 10) + snd->clockbase)) & ~3;
                if (behind >= need)
                    continue;
                behind = need - behind;
                if (snd->pending + behind > snd->bufsize)
                    behind = snd->bufsize - snd->pending;
                behind &= ~3;
                need = snd->bits16 ? 0 : 0x80;
                if (snd->writepos + behind >= snd->bufend) {
                    radmemset(snd->writepos, need, snd->bufend - snd->writepos);
                    radmemset(snd->buf, need, behind - (snd->bufend - snd->writepos));
                    snd->writepos = snd->buf + behind - (snd->bufend - snd->writepos);
                } else {
                    radmemset(snd->writepos, need, behind);
                    snd->writepos += behind;
                }
                snd->pending += behind;
                snd->written += behind;
            }
            LowSoundCheckAddr();
            return 1;
        }
    } else if (!smk->soundoff) {
        smk->soundoff = 1;
        for (i = 0; i < 7; i++) {
            if (smk->sounds[i] != NULL) {
                if (LowSoundOffAddr != NULL)
                    LowSoundOffAddr(smk->sounds[i]);
                snd = smk->sounds[i];
                snd->readpos = snd->writepos = snd->buf;
                snd->written -= snd->pending;
                snd->pending = 0;
            }
        }
    }
    LowSoundCheckAddr();
    return 1;
}

// SmackClose: close smk's sound tracks and file and free its buffers and the Smack itself (unless caller-owned).
// FUNCTION: 0xe98d0
void __pascal SmackClose(Smack *smk)
{
    u32 i;
    u32 keep;

    if (smk == NULL)
        return;
    if (smk->timingtrack != -1) {
        for (i = 0; i < 7; i++) {
            if (smk->sounds[i] != NULL) {
                LowSoundCloseAddr(smk->sounds[i]);
                RADFREE(smk->sounds[i]->buf);
                RADFREE(smk->sounds[i]);
                smk->sounds[i] = NULL;
                sounds--;
            }
        }
        smk->timingtrack = -1;
    }
    if (sounds == 0 && trackbuf != NULL) {
        RADFREE(trackbuf);
        trackbuf = NULL;
    }
    if (smk->handle != -1 && !(smk->OpenFlags & 0x1000))
        radclose(smk->handle);
    if (smk->deltas != NULL)
        RADFREE(smk->deltas);
    if (smk->addr32 != NULL)
        RADFREE(smk->addr32);
    if (smk->frametypes != NULL)
        RADFREE(smk->frametypes);
    if (smk->framebuf != NULL)
        RADFREE(smk->framebuf);
    if (smk->readahead != NULL)
        RADFREE(smk->readahead);
    keep = smk->OpenFlags & 0x2000000;
    radmemset(smk, 0, sizeof(Smack));
    if (!keep)
        RADFREE(smk);
    SmackTimerDoneAddr();
}

// soundnext: append the current frame's audio for every open track to its ring buffer (decoding compressed audio, padding underruns with silence) and advance the timing track's clock.
// FUNCTION: 0xe94c0
// NONMATCHING: register allocation; the target spills silence (one more
// dword of frame) and keeps the inlined nexttrack's mask in edx
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

    if (smk->timingtrack == -1 || smk->soundoff)
        return;
    if (smk->ms100perframe)
        smk->soundwait = 1;
    first = smk->sounds[smk->timingtrack];
    first->clockfix += first->frameadd;
    first->lateclockfix += first->frameadd;
    if (first->clockfix > 0xfa00000 && first->lateclockfix > 0x2000000) {
        first->clockfix -= 0xfa00000;
        first->lateclockfix -= 0xfa00000;
        first->clockbase += 0x3e800;
    }
    tracks = smk->OpenFlags;
    trk = nexttrack(&tracks);
    src = smk->trackbufs[trk];
    size = src == NULL ? 0 : (smk->TrackType[trk] & 0x80000000) ? src[1] : src[0] - 4;
    if (smk->sounds[trk]->flush = size == 0)
        goto done;
    if (sounds)
        LowSoundCheckAddr();
    for (;;) {
        snd = smk->sounds[trk];
        if (size + snd->pending > snd->bufsize) {
            size = snd->bufsize - snd->pending;
            snd->overflows++;
        }
        if (snd->starvedseen) {
            snd->starved = 0;
            snd->starvedseen = 0;
        }
        fill = snd->starved;
        if (fill) {
            snd->starvedseen = 1;
            if (size < snd->minchunk && snd->written) {
                fill = snd->minchunk - size;
                if (snd->pending + size + fill > snd->bufsize)
                    fill = snd->bufsize - snd->pending - size;
                if (fill) {
                    snd->soundskips++;
                    silence = snd->bits16 ? 0 : 0x80;
                    if (fill + (u32)snd->writepos > (u32)snd->bufend) {
                        radmemset(snd->writepos, silence, snd->bufend - snd->writepos);
                        n = fill - (snd->bufend - snd->writepos);
                        dst = snd->buf;
                    } else {
                        n = fill;
                        dst = snd->writepos;
                    }
                    radmemset(dst, silence, n);
                    snd->clockbase += fill;
                    snd->writepos += fill;
                    if (snd->writepos >= snd->bufend)
                        snd->writepos = snd->buf + (snd->writepos - snd->bufend);
                }
            } else {
                fill = 0;
            }
        }
        if (smk->TrackType[trk] & 0x80000000) {
            SmackDoPCM(src + 2, size, trackbuf, snd->buf, snd->bufend, snd->writepos);
        } else if (size + (u32)snd->writepos > (u32)snd->bufend) {
            radmemcpy(snd->writepos, src + 1, snd->bufend - snd->writepos);
            n = size - (snd->bufend - snd->writepos);
            radmemcpy(snd->buf, src + 1 + (snd->bufend - snd->writepos), n);
        } else {
            radmemcpy(snd->writepos, src + 1, size);
        }
        snd->written += size + fill;
        snd->pending += size + fill;
        snd->writepos += size;
        if (snd->writepos >= snd->bufend)
            snd->writepos = snd->buf + (snd->writepos - snd->bufend);
        trk = nexttrack(&tracks);
        if (trk == 0xff)
            break;
        src = smk->trackbufs[trk];
        size = src == NULL ? 0 : (smk->TrackType[trk] & 0x80000000) ? src[1] : src[0] - 4;
    }
done:
    if ((first->clockfix >> 10) + first->clockbase > first->written) {
        first->lateclockfix -= first->clockfix;
        first->clockfix = 0;
        first->clockbase = first->written;
    }
}

// doskip: 1 if the timing sound track has already played past this frame, i.e. the frame is late, else 0 (also 0 when this frame queued no audio on that track).
// FUNCTION: 0xe9440
u8 doskip(Smack *smk)
{
    SmackSnd *snd;
    u32 played;

    if (smk->timingtrack != -1 && smk->soundoff == 0) {
        if (smk->soundwait == 0)
            return 1;
        snd = smk->sounds[smk->timingtrack];
        if (snd->flush == 0) {
            played = LowSoundPlayedAddr(snd);
            snd = smk->sounds[smk->timingtrack];
            if (played >= (snd->lateclockfix >> 10) + snd->clockbase) {
                smk->soundwait = 0;
                return 1;
            }
        }
    }
    return 0;
}

// SmackDoFrame: queue the current frame's audio and decompress its video, skipping the video if the frame is late (unless opened with 0x400); returns 1 if it was late, 0 otherwise and after a read error.
// FUNCTION: 0xe9350
u32 __pascal SmackDoFrame(Smack *smk)
{
    u32 now;
    u8 skip;
    u8 nodraw;

    if (smk == NULL)
        return 0;
    if (smk->readerror)
        return 0;
    now = SmackTimerReadAddr();
    smk->decompstarttime = now;
    smk->framestarttime = now;
    if (smk->firstframetime == 0)
        smk->firstframetime = now;
    if (smk->NewPalette)
        smk->palframe = smk->FrameNum;
    smk->totalframes++;
    soundnext(smk);
    if (smk->ms100perframe == 0) {
        skip = 0;
        nodraw = 0;
    } else {
        skip = doskip(smk);
        nodraw = skip;
        if (smk->OpenFlags & 0x400)
            nodraw = 0;
    }
    if (smk->deltas != NULL) {
        if (nodraw) {
            smk->skippedframes++;
        } else {
            smk->rectstate = 1;
            Unsmack(smk->videodata, smk->addr32, smk->deltas);
        }
    }
    if (sounds)
        LowSoundCheckAddr();
    return skip;
}

// timeframe: account the finished frame's time: slowest two frames, blit and decompression time.
// FUNCTION: 0xe9230
void timeframe(Smack *smk)
{
    u32 now;
    u32 elapsed;

    now = (u32)-1;
    if (smk->framestarttime) {
        now = SmackTimerReadAddr();
        elapsed = now - smk->framestarttime;
        smk->framestarttime = 0;
        if (elapsed > smk->slowestframetime) {
            if (smk->slowestframenum != smk->FrameNum) {
                smk->slowest2frametime = smk->slowestframetime;
                smk->slowest2framenum = smk->slowestframenum;
                smk->slowestframenum = smk->FrameNum;
            }
            smk->slowestframetime = elapsed;
        } else if (elapsed > smk->slowest2frametime && smk->slowestframenum != smk->FrameNum) {
            smk->slowest2frametime = elapsed;
            smk->slowest2framenum = smk->FrameNum;
        }
    }
    if (smk->blitstarttime) {
        if (now == -1)
            now = SmackTimerReadAddr();
        smk->totalblittime += now - smk->blitstarttime;
        smk->blitstarttime = 0;
    }
    if (smk->decompstarttime) {
        if (now == -1)
            now = SmackTimerReadAddr();
        smk->totaldecomptime += now - smk->decompstarttime;
        smk->decompstarttime = 0;
    }
}

// SmackSummary: fill *sum with smk's playback statistics.
// FUNCTION: 0xe9110
void __pascal SmackSummary(Smack *smk, SmackSum *sum)
{
    u32 total;

    timeframe(smk);
    sum->TotalTime = SmackTimerReadAddr() - smk->firstframetime;
    sum->MS100PerFrame = smk->ms100perframe;
    sum->TotalOpenTime = smk->totalopentime;
    sum->TotalFrames = smk->totalframes;
    sum->SkippedFrames = smk->skippedframes;
    sum->TotalBlitTime = smk->totalblittime;
    sum->TotalReadTime = smk->readtime - smk->openreadtime;
    sum->TotalBackReadTime = smk->backreadtime;
    sum->TotalDecompTime = smk->totaldecomptime;
    total = smk->readtime + smk->backreadtime;
    sum->TotalReadSpeed = total ? mult64anddiv(smk->readbytes + smk->backreadbytes, 1000, total) : 0;
    sum->SlowestFrameTime = smk->slowestframetime;
    sum->SlowestFrameNum = smk->slowestframenum + 1;
    sum->Slowest2FrameTime = smk->slowest2frametime;
    sum->Slowest2FrameNum = smk->slowest2framenum + 1;
    sum->AverageFrameSize = smk->averageframesize;
    sum->HighestMemAmount = smk->highestmemamount;
    sum->TotalExtraMemory = smk->readaheadsize;
    sum->HighestExtraUsed = smk->highestextraused;
    sum->Highest1SecRate = smk->highest1secsize;
    sum->Highest1SecFrame = smk->highest1secframe + 1;
    sum->SoundSkips = smk->timingtrack != -1 ? smk->sounds[smk->timingtrack]->soundskips : 0;
}

// SmackNextFrame: advance to the next frame, reading and preparing its data (after a read error only the frame number advances).
// FUNCTION: 0xe9050
void __pascal SmackNextFrame(Smack *smk)
{
    if (smk == NULL)
        return;
    if (smk->readerror) {
        smk->FrameNum++;
    } else {
        if (sounds)
            LowSoundCheckAddr();
        timeframe(smk);
        smk->framedata += smk->framesizes[smk->FrameNum];
        smk->FrameNum++;
        if (smk->FrameNum < smk->Frames)
            readframe(smk);
        setuptheframe(smk);
        if (sounds)
            LowSoundCheckAddr();
    }
}

// PutWidths: write a blit table's row parameters at p: bpl, width in dwords, mult * bpl - 4, mult * bpl and bpl * lines - width; returns the end.
// FUNCTION: 0xe9010
u16 *PutWidths(u16 *p, u32 bpl, u32 width, u32 mult, u32 lines)
{
    // The casts add expression nodes that keep this function over the
    // -oe=25 inlining limit: the original never inlines it.
    *p++ = (u16)bpl;
    *p++ = (u16)(width >> 2);
    *p++ = (u16)((u16)(mult * bpl) - 4);
    *p++ = (u16)(mult * bpl);
    *p++ = (u16)(bpl * lines - width);
    return p;
}

// SmackToScreen: build the blit table for decoding straight to the screen at left, top with pitch, splitting rows at 64K boundaries through the bank numbers in wintbl when banked (pitch 320: unbanked VGA memory at 0xA0000).
// FUNCTION: 0xe8ab0
// NONMATCHING: register allocation: the original keeps pitch in edi, stride
// in esi and height (later p) in ebp, and spills prevbuf; ours spills p and
// one more local, so the frame and most operands differ
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
    u8 *split_buf;
    u32 stride;
    u32 height;
    u32 w;
    u32 i;
    u16 n;
    u8 *old_table;
    u16 *p;

    prevbuf = NULL;
    if (smk == NULL)
        return;
    if (pitch == 320) {
        banked = 0;
        wintbl = (const u16 *)0xa0000;
    }
    old_table = smk->deltas;
    if (old_table != NULL) {
        if (*old_table == 1) {
            prevstep = *(u32 *)(old_table + 0x18);
            prevbuf = *(u8 **)(old_table + 4);
        }
        RADFREE(smk->deltas);
        smk->memamount -= smk->deltassize;
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
    smk->deltassize = ((top * pitch + stride * height) >> 16) * (smk->Width * lines) + 0x400;
    smk->deltas = smkmalloc(smk, smk->deltassize);
    if (smk->deltas == NULL)
        return;
    left &= ~3;
    smk->TopOfs = top;
    smk->LeftOfs = left;
    split_buf = smk->deltas + 0x400;
    p = (u16 *)smk->deltas;
    *(u8 *)p = 0;
    ((u8 *)p)[1] = (smk->OpenFlags & 0x200000) != 0;
    offset = left + pitch * top;
    bank = offset >> 16;
    p += 2;
    *(*(u32 **)&p)++ = banked;
    *p++ = bpl - smk->Width;
    *p++ = !banked ? 0xfffe : wintbl[bank++];
    p = PutWidths(p, bpl, smk->Width, mult, lines);
    if (!banked) {
        *(*(u32 **)&p)++ = (u32)wintbl + offset;
        *p++ = height >> 2;
        *p = 0xffff;
        return;
    }
    offset &= 0xffff;
    *(*(u32 **)&p)++ = offset + 0xa0000;
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
                *(u8 **)p = split_buf;
                p += 2;
                *p++ = 1;
                *p++ = 0xfffd;
                *(u8 **)p = split_buf;
                if (prevbuf != NULL) {
                    for (i = 0; i < lines; i++) {
                        radmemcpy(split_buf, prevbuf, smk->Width);
                        split_buf += smk->Width;
                        prevbuf += prevstep;
                    }
                } else {
                    split_buf += smk->Width * lines;
                }
                p += 2;
                *(*(u32 **)&p)++ = offset + 0xa0000;
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
                    *p++ = w >> 2;
                    *p++ = wintbl[bank++];
                    *p++ = (smk->Width - w) >> 2;
                    *p++ = lines - n - 1;
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

// SmackToBuffer: set up decoding into buf at left, top with pitch (flags nonzero: rows bottom-up in a destheight-row buffer); a NULL buf frees the table.
// FUNCTION: 0xe8890
// NONMATCHING: register allocation: the target reads the delta header byte
// into bl before comparing it, and multiplies pitch * row in a fresh register
void __pascal SmackToBuffer(Smack *smk, u32 left, u32 top, u32 pitch, u32 destheight, const void *buf, u32 flags)
{
    u32 bpl;
    u32 height;
    u32 mult;
    u32 step;
    u32 row;
    u8 *table;
    u32 *p;
    u32 size;

    if (smk == NULL)
        return;
    if (buf == NULL) {
        if (smk->deltas != NULL) {
            RADFREE(smk->deltas);
            smk->deltas = NULL;
            smk->memamount -= smk->deltassize;
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
    table = smk->deltas;
    if (table != NULL) {
        if (*table == 1)
            goto have;
        RADFREE(table);
        smk->memamount -= smk->deltassize;
    }
    size = SmackGetSizeDeltas(smk->Width, height) + 0x20;
    smk->deltassize = size;
    smk->deltas = smkmalloc(smk, size);
    if (smk->deltas == NULL)
        return;
    radmemset(smk->deltas, 0, size);
have:
    smk->TopOfs = top;
    smk->LeftOfs = left;
    table = smk->deltas;
    table[0] = 1;
    table[1] = (smk->OpenFlags & 0x200000) ? 1 : 0;
    p = (u32 *)(table + 4);
    row = flags ? destheight - top - 1 : top;
    *p++ = (u32)buf + (left + pitch * row);
    step = mult * bpl;
    *p++ = smk->Width >> 2;
    *p++ = height >> 2;
    if (flags) {
        *p++ = -(step + 4);
        *p++ = -step;
        *p++ = -bpl;
        *p++ = -((mult + 2) * bpl - (bpl - smk->Width));
    } else {
        *p++ = step - 4;
        *p++ = step;
        *p++ = bpl;
        *p++ = step + (bpl - smk->Width);
    }
    p++;
    *p++ = smk->Width;
    *p = height;
}

// SmackColorTrans: remap the video's colours through the 256-entry table trans.
// FUNCTION: 0xe8840
void __pascal SmackColorTrans(Smack *smk, const void *trans)
{
    if (smk != NULL) {
        if (sounds)
            LowSoundCheckAddr();
        SmackRemapTables(smk->addr32, trans);
        if (sounds)
            LowSoundCheckAddr();
    }
}

// SmackColorRemap: remap the video's colours to the nearest (squared RGB distance) of numcolors entries of remappal, laid out as paltype describes (1: 16-bit components, 2: BGR order, 4: 4-byte entries, with 8 the colour in bytes 1-3). With numcolors 0, best is stored uninitialised.
// FUNCTION: 0xe85b0
// NONMATCHING: register allocation: r, g, b and ro, bo get each other's spill
// slots and are loaded through different registers; the target computes
// b - p[bo], which written here spills p[bo] (one more dword of frame), so the
// equal square(p[bo] - b) is used instead
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
        size *= 2;
        ro *= 2;
        go *= 2;
        bo *= 2;
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
            dist += square(p[bo] - b);
            if (dist < bestdist) {
                best = (s32)(p - remappal) / (s32)size;
                if (dist == 0)
                    break;
                bestdist = dist;
            }
        }
        remap[i++] = best;
    } while (i < 256);
    SmackRemapTables(smk->addr32, remap);
}

// SmackGetTrackData: copy the current frame's data of track trackflag to dest (decoding compressed audio); returns its size.
// FUNCTION: 0xe8480
u32 __pascal SmackGetTrackData(Smack *smk, u8 *dest, u32 trackflag)
{
    u16 trk;
    u32 *src;

    if (smk == NULL)
        return 0;
    if (smk->readerror)
        return 0;
    trk = nexttrack(&trackflag);
    if (trk >= 7)
        return 0;
    src = smk->trackbufs[trk];
    if (src == NULL)
        return 0;
    if ((smk->TrackType[trk] >> 24) & 0x80) {
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

// SmackGoto: move to frame (1-based) and prepare it.
// FUNCTION: 0xe8440
void __pascal SmackGoto(Smack *smk, u32 frame)
{
    if (smk != NULL && smk->readerror == 0) {
        if (frame)
            frame--;
        if (frame != smk->FrameNum) {
            smk->FrameNum = frame;
            gotoframe(smk);
            setuptheframe(smk);
        }
    }
}

// SmackToBufferRect: next changed rectangle of the decoded frame into LastRect* (SmackSurface 1 merges all into one; 0 and 3 merge while the bounding box stays within 5/4 of the rectangles' area, 3 then also giving one final empty rectangle; 2 none). A frame with only a new palette gives an empty rectangle. Returns 1 if a rectangle was produced.
// FUNCTION: 0xe8190
// NONMATCHING: register allocation; the target computes &smk->rectx into ecx
// once before loading left/top, and loads smk->NewPalette into eax so that its
// return 0 jumps to the final epilogue
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
    if (smk->rectstate == 0 || SmackSurface == 2)
        return 0;
    if (smk->rectstate == 3) {
        smk->rectstate = 0;
        if (SmackSurface != 3)
            return 0;
        goto empty;
    }
    if (smk->blitstarttime == 0) {
        smk->blitstarttime = SmackTimerReadAddr();
        if (smk->decompstarttime) {
            smk->totaldecomptime += smk->blitstarttime - smk->decompstarttime;
            smk->decompstarttime = 0;
        }
    }
    if (smk->rectstate == 1) {
        if (!SmackGetRect(smk->deltas, &smk->rectx)) {
            smk->rectstate = 0;
            if (!smk->NewPalette)
                return 0;
        empty:
            smk->LastRectx = 0;
            smk->LastRecty = 0;
            smk->LastRectw = 0;
            smk->LastRecth = 0;
            return 1;
        }
        smk->rectstate = 2;
    }
    left = smk->rectx;
    top = smk->recty;
    right = smk->rectw + left;
    bottom = top + smk->recth;
    if (SmackSurface == 1) {
        while (SmackGetRect(smk->deltas, &smk->rectx)) {
            if (left > smk->rectx)
                left = smk->rectx;
            if (smk->rectx + smk->rectw > right)
                right = smk->rectx + smk->rectw;
            bottom = smk->recty + smk->recth;
        }
        smk->rectstate = 3;
    } else {
        area = (smk->rectw * smk->recth * 5) >> 2;
        while (SmackGetRect(smk->deltas, &smk->rectx)) {
            area += (smk->rectw * smk->recth * 5) >> 2;
            nleft = smk->rectx;
            if (left <= nleft)
                nleft = left;
            nright = smk->rectx + smk->rectw;
            if (right > nright)
                nright = right;
            nbottom = smk->recty + smk->recth;
            if (area < (nbottom - top) * (nright - nleft))
                goto rect;
            right = nright;
            left = nleft;
            bottom = nbottom;
        }
        smk->rectstate = 3;
    }
rect:
    if (smk->OpenFlags & 0x300000) {
        top *= 2;
        bottom *= 2;
    }
    smk->LastRectx = left + smk->LeftOfs;
    smk->LastRecty = top + smk->TopOfs;
    smk->LastRectw = right - left;
    smk->LastRecth = bottom - top;
    return 1;
}

// SmackSoundInTrack: 1 if any track in trackflags carries sound.
// FUNCTION: 0xe8120
u32 __pascal SmackSoundInTrack(Smack *smk, u32 trackflags)
{
    u8 trk;

    if (smk != NULL) {
        while (trackflags & SMACKTRACKS) {
            trk = nexttrack(&trackflags);
            if (((smk->TrackType[trk] >> 24) & 0x40) && (smk->TrackType[trk] & 0xffffff))
                return 1;
        }
    }
    return 0;
}

// SmackVolumePan: set the volume and pan (each 0-0x10000) of the tracks in trackflag.
// FUNCTION: 0xe8080
void __pascal SmackVolumePan(Smack *smk, u32 trackflag, u32 volume, u32 pan)
{
    u8 trk;

    if (smk == NULL || smk->timingtrack == -1)
        return;
    if (volume > 0x10000)
        volume = 0x10000;
    if (pan > 0x10000)
        pan = 0x10000;
    while (trackflag & SMACKTRACKS) {
        trk = nexttrack(&trackflag);
        if (trk < 7) {
            if (smk->sounds[trk] != NULL && LowSoundVolPanAddr != NULL)
                LowSoundVolPanAddr(smk->sounds[trk], volume, pan);
        }
    }
}

// SmackSoundCheck: service the low-level sound driver if a track is open.
// FUNCTION: 0x000e8070
void __pascal SmackSoundCheck(void)
{
    if (sounds)
        LowSoundCheckAddr();
}

// SmackWait: 1 while the next frame is not yet due (by the timing sound track, else the frame rate), reading ahead in the meantime; 0 when it is.
// FUNCTION: 0xe7f40
u32 __pascal SmackWait(Smack *smk)
{
    u32 played;
    u32 now;
    SmackSnd *snd;

    if (smk == NULL)
        return 0;
    if (smk->readerror)
        return 0;
    if (sounds)
        LowSoundCheckAddr();
    if (smk->timingtrack != -1) {
        if (smk->soundwait == 0 || smk->soundoff != 0)
            return 0;
        played = LowSoundPlayedAddr(smk->sounds[smk->timingtrack]);
        snd = smk->sounds[smk->timingtrack];
        if (played >= (snd->clockfix >> 10) + snd->clockbase) {
            smk->soundwait = 0;
            return 0;
        }
    } else {
        if (smk->framedue)
            return 0;
        now = SmackTimerReadAddr() * 100;
        if (smk->nextframetime == -1) {
            smk->nextframetime = now + smk->ms100perframe;
        } else if (now >= smk->nextframetime) {
            smk->nextframetime = smk->ms100perframe + smk->nextframetime;
            if (now - smk->nextframetime > smk->ms100perframe)
                smk->nextframetime = now + smk->ms100perframe;
            if (smk->ms100perframe)
                smk->framedue = 1;
            return 0;
        }
    }
    backgroundload(smk);
    return 1;
}
