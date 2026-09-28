#include "ail.h"

#define REDBOOK_ERROR 0
#define REDBOOK_PLAYING 1
#define REDBOOK_PAUSED 2
#define REDBOOK_STOPPED 3

#pragma pack(1)
typedef struct {
    U32 edi;
    U32 esi;
    U32 ebp;
    U32 reserved;
    U32 ebx;
    U32 edx;
    U32 ecx;
    U32 eax;
    U16 flags;
    U16 es;
    U16 ds;
    U16 fs;
    U16 gs;
    U16 ip;
    U16 cs;
    U16 sp;
    U16 ss;
} RMI;

typedef struct {
    U8 length;
    U8 subunit;
    U8 command;
    U16 status;
    U8 reserved[8];
} RQ_HDR;

typedef struct {
    U8 media;
    U32 transfer;
    U16 count;
    U16 start;
    U32 volid;
} IOCTL_RQ;

typedef struct {
    U8 mode;
    U32 start;
    U32 count;
} PLAY_RQ;
#pragma pack()

typedef struct {
    U32 tracks;
    U32 pos[100];
} RB_CONTENTS;

typedef struct _REDBOOK {
    U32 drive;
    U32 paused;
    U32 pause_pos;
    U32 end;
    U32 dirty;
    RB_CONTENTS contents;
} REDBOOK;

extern S32 numopen;
extern U32 stupidhash[4];
extern U8 *ptrtoreal;
extern U32 segtoreal;
extern U32 seltoreal;

U32 rbpos(U32 drive, S32 track);

// FUNCTION: 0x000e60c0
void simint(S32 intno, RMI *rmi)
{
    union REGS in;
    union REGS out;

    in.x.eax = 0x300;
    in.x.ebx = intno;
    in.x.ecx = 0;
    in.x.edi = (U32)rmi;
    rmi->ss = 0;
    rmi->sp = 0;
    AIL_int386(0x31, &in, &out);
}

// FUNCTION: 0x000e6100
S32 calldrv(U32 drive, RQ_HDR *req, U32 real, U32 offset)
{
    RMI rmi;

    memset(&rmi, 0, sizeof(rmi));
    rmi.ecx = drive;
    rmi.es = real >> 16;
    rmi.eax = 0x1510;
    rmi.ebx = offset;
    req->status = 0xffff;
    req->length = 0xd;
    req->subunit = 0;
    simint(0x2f, &rmi);
    return !(req->status & 0x8000);
}

// FUNCTION: 0x000e6190
S32 mscdexread(U32 drive, RQ_HDR *req, U32 real, U32 offset)
{
    IOCTL_RQ *ioctl;

    ioctl = (IOCTL_RQ *)(req + 1);
    req->command = 3;
    ioctl->media = 0;
    ioctl->start = 0;
    ioctl->volid = 0;
    return calldrv(drive, req, real, offset);
}

// FUNCTION: 0x000e61c0
U32 RBtoMS(U32 rb)
{
    return ((rb >> 8) & 0xff) * 1000 + ((rb >> 16) & 0xff) * 60000
           + ((rb & 0xff) * 1000) / 75;
}

// FUNCTION: 0x000e6240
U32 RBseptoMS(U32 min, U32 sec, U32 frame)
{
    return (sec & 0xff) * 1000 + (min & 0xff) * 60000 + (frame * 1000) / 75;
}

// FUNCTION: 0x000e62b0
U32 MStoSector(U32 ms)
{
    return ((ms / 1000) % 60) * 75 + (ms / 60000) * 4500 + ((ms % 1000) * 75) / 1000 - 150;
}

// FUNCTION: 0x000e6340
U8 BCDtoBin(U8 bcd)
{
    return (bcd / 16) * 10 + (bcd & 0x0f);
}

// FUNCTION: 0x000e6380
// NONMATCHING: the target returns 1 through a jump to a shared epilogue
S32 rbcontents(U32 drive, RB_CONTENTS *info)
{
    RQ_HDR *req;
    IOCTL_RQ *ioctl;
    U8 *disk;
    U8 *track;
    S32 i;

    req = (RQ_HDR *)ptrtoreal;
    ioctl = (IOCTL_RQ *)(ptrtoreal + 0xd);
    disk = ptrtoreal + 0x1a;
    track = ptrtoreal + 0x21;
    ioctl->count = 7;
    ioctl->transfer = segtoreal + (disk - (U8 *)req);
    disk[0] = 0xa;
    if (mscdexread(drive, req, segtoreal, 0)) {
        ioctl->count = 7;
        ioctl->transfer = segtoreal + (track - (U8 *)req);
        track[0] = 0xb;
        for (i = disk[1]; i <= disk[2]; i++) {
            track[1] = i;
            if (!mscdexread(drive, req, segtoreal, 0)) {
                return 0;
            }
            info->pos[i - disk[1]] = RBtoMS(*(U32 *)(track + 2)) + 1;
        }
        info->tracks = disk[2] - disk[1] + 1;
        info->pos[info->tracks] = RBtoMS(*(U32 *)(disk + 3));
        return 1;
    }
    return 0;
}

// FUNCTION: 0x000e64a0
U32 rbopen(U32 which)
{
    RMI rmi;

    memset(&rmi, 0, sizeof(rmi));
    rmi.eax = 0x1500;
    rmi.ebx = 0;
    simint(0x2f, &rmi);
    if (which >= rmi.ebx) {
        return 0xff;
    }
    if (numopen == 0) {
        AIL_mem_alloc_DOS(8, (void **)&ptrtoreal, &segtoreal, &seltoreal);
        if (ptrtoreal == NULL) {
            return 0xff;
        }
    }
    rmi.eax = 0x150d;
    rmi.ebx = 0;
    rmi.es = segtoreal >> 16;
    simint(0x2f, &rmi);
    numopen++;
    return ptrtoreal[which];
}

// FUNCTION: 0x000e6580
void rbclose(void)
{
    if (numopen && --numopen == 0) {
        AIL_mem_free_DOS(ptrtoreal, segtoreal, seltoreal);
    }
}

// FUNCTION: 0x000e65d0
// NONMATCHING: config/functions.txt sizes this function to also cover the
// tail of rbpos, which the target places after it
S32 rbwrite(U32 drive, U8 cmd)
{
    RQ_HDR *req;
    IOCTL_RQ *ioctl;
    U8 *buf;

    req = (RQ_HDR *)ptrtoreal;
    ioctl = (IOCTL_RQ *)(ptrtoreal + 0xd);
    buf = ptrtoreal + 0x1a;
    ioctl->count = 1;
    ioctl->transfer = segtoreal + (buf - ptrtoreal);
    buf[0] = cmd;
    req->command = 0xc;
    ioctl->media = 0;
    ioctl->start = 0;
    ioctl->volid = 0;
    return calldrv(drive, req, segtoreal, 0);
}

// FUNCTION: 0x000e6650
// NONMATCHING: drive and play swap esi/edi
S32 rbplay(U32 drive, U32 start, U32 len)
{
    RQ_HDR *req;
    PLAY_RQ *play;
    U32 first;
    U32 last;

    req = (RQ_HDR *)ptrtoreal;
    first = MStoSector(start);
    play = (PLAY_RQ *)(req + 1);
    last = MStoSector(start + len);
    req->command = 0x84;
    play->start = first;
    play->count = last - first;
    play->mode = 0;
    return calldrv(drive, req, segtoreal, 0);
}

// FUNCTION: 0x000e66a0
S32 rbstop(U32 drive)
{
    ((RQ_HDR *)ptrtoreal)->command = 0x85;
    return calldrv(drive, (RQ_HDR *)ptrtoreal, segtoreal, 0);
}

// FUNCTION: 0x000e66c0
// NONMATCHING: the target loads only the high byte of the status word
// (mov ah,[esi+4])
U32 rbstatus(U32 drive)
{
    RQ_HDR *req;
    IOCTL_RQ *ioctl;
    U8 *buf;
    U16 status;

    req = (RQ_HDR *)ptrtoreal;
    ioctl = (IOCTL_RQ *)(ptrtoreal + 0xd);
    buf = ptrtoreal + 0x1a;
    ioctl->count = 5;
    ioctl->transfer = segtoreal + (buf - ptrtoreal);
    buf[0] = 6;
    mscdexread(drive, req, segtoreal, 0);
    status = req->status;
    if ((status & 0x8000) || (buf[1] & 1)) {
        return 0;
    }
    return ((status & 0x200) == 0) + 1;
}

// FUNCTION: 0x000e6850
// NONMATCHING: the target keeps rb in esi; ours spills it to the stack
HREDBOOK __cdecl AIL_API_redbook_open(U32 which)
{
    REDBOOK *rb;

    rb = AIL_mem_alloc_lock(sizeof(REDBOOK));
    if (rb != NULL) {
        which = rbopen(which);
        if (which == 0xff) {
            AIL_mem_free_lock(rb);
            rb = NULL;
        } else {
            memset(rb, 0, sizeof(REDBOOK));
            rb->dirty = 1;
            rb->paused = 0;
            rb->drive = which;
        }
    }
    return rb;
}

// FUNCTION: 0x000e68d0
void __cdecl AIL_API_redbook_close(HREDBOOK hand)
{
    if (hand != NULL) {
        rbclose();
        AIL_mem_free_lock(hand);
    }
}

// FUNCTION: 0x000e68f0
void __cdecl AIL_API_redbook_eject(HREDBOOK hand)
{
    if (hand != NULL) {
        rbwrite(hand->drive, 0);
        hand->paused = 0;
    }
}

// FUNCTION: 0x000e6910
void __cdecl AIL_API_redbook_retract(HREDBOOK hand)
{
    if (hand != NULL) {
        rbwrite(hand->drive, 5);
        hand->paused = 0;
    }
}

// FUNCTION: 0x000e6930
U32 __cdecl AIL_API_redbook_status(HREDBOOK hand)
{
    U32 status;

    if (hand == NULL) {
        return REDBOOK_ERROR;
    }
    status = rbstatus(hand->drive);
    if (status != REDBOOK_ERROR) {
        if (hand->dirty) {
            if (!rbcontents(hand->drive, &hand->contents)) {
                goto error;
            }
            hand->dirty = 0;
        }
        if (status == REDBOOK_PLAYING) {
            return status;
        }
        return hand->paused ? REDBOOK_PAUSED : REDBOOK_STOPPED;
    }
error:
    hand->paused = 0;
    hand->dirty = 1;
    return REDBOOK_ERROR;
}

// FUNCTION: 0x000e6990
U32 __cdecl AIL_API_redbook_tracks(HREDBOOK hand)
{
    if (!AIL_redbook_status(hand)) {
        return 0;
    }
    return hand->contents.tracks;
}

// FUNCTION: 0x000e69b0
void __cdecl AIL_API_redbook_track_info(HREDBOOK hand, U32 tracknum, U32 *startmsec,
                                        U32 *endmsec)
{
    if (!AIL_redbook_status(hand) || tracknum < 1 || tracknum > hand->contents.tracks) {
        if (startmsec != NULL) {
            *startmsec = 0;
        }
        if (endmsec != NULL) {
            *endmsec = 0;
        }
        return;
    }
    if (startmsec != NULL) {
        *startmsec = hand->contents.pos[tracknum - 1];
    }
    if (endmsec != NULL) {
        *endmsec = hand->contents.pos[tracknum] - 1;
    }
}

// FUNCTION: 0x000e6a10
// NONMATCHING: the target keeps n in ebp and reloads hand from the stack
U32 __cdecl AIL_API_redbook_id(HREDBOOK hand)
{
    U32 start;
    U32 end;
    U32 n;
    U32 hash;
    U32 i;
    S32 k;

    if (!AIL_redbook_status(hand)) {
        return 0;
    }
    n = AIL_redbook_tracks(hand);
    hash = stupidhash[1] ^ n;
    k = 1;
    hash = (hash << 8) | (hash >> 24);
    for (i = 0; i < n; i++) {
        AIL_redbook_track_info(hand, i + 1, &start, &end);
        hash = hash ^ stupidhash[++k & 3] ^ start;
        hash = (hash << 8) | (hash >> 24);
        hash = hash ^ stupidhash[++k & 3] ^ end;
        hash = (hash << 8) | (hash >> 24);
    }
    return hash;
}

// FUNCTION: 0x000e6ac0
// NONMATCHING: jumps into rbpos, which the target names rbpos_+0x30
U32 __cdecl AIL_API_redbook_position(HREDBOOK hand)
{
    if (AIL_redbook_status(hand) != REDBOOK_PLAYING) {
        return 0;
    }
    return rbpos(hand->drive, 0);
}

// FUNCTION: 0x000e6ae0
// NONMATCHING: config/functions.txt sizes this function to also cover rbpos,
// which follows it
U32 __cdecl AIL_API_redbook_track(HREDBOOK hand)
{
    if (AIL_redbook_status(hand) != REDBOOK_PLAYING) {
        return 0;
    }
    return rbpos(hand->drive, 1);
}

U32 rbpos(U32 drive, S32 track)
{
    RQ_HDR *req;
    IOCTL_RQ *ioctl;
    U8 *buf;

    req = (RQ_HDR *)ptrtoreal;
    ioctl = (IOCTL_RQ *)(ptrtoreal + 0xd);
    buf = ptrtoreal + 0x1a;
    ioctl->count = 0xb;
    ioctl->transfer = segtoreal + (buf - ptrtoreal);
    buf[0] = 0xc;
    if (!mscdexread(drive, req, segtoreal, 0)) {
        return 0;
    }
    if (track) {
        return BCDtoBin(buf[2]);
    }
    return RBseptoMS(buf[8], buf[9], buf[10]);
}

// FUNCTION: 0x000e6b90
U32 __cdecl AIL_API_redbook_play(HREDBOOK hand, U32 startmsec, U32 endmsec)
{
    U32 status;

    status = AIL_redbook_status(hand);
    if (status != REDBOOK_ERROR) {
        if (status != REDBOOK_STOPPED) {
            rbstop(hand->drive);
        }
        if (startmsec | endmsec) {
            hand->end = endmsec;
            rbplay(hand->drive, startmsec, endmsec - startmsec);
        }
    }
    return AIL_redbook_status(hand);
}

// FUNCTION: 0x000e6be0
U32 __cdecl AIL_API_redbook_stop(HREDBOOK hand)
{
    if (AIL_redbook_status(hand)) {
        hand->paused = 0;
        rbstop(hand->drive);
        rbstop(hand->drive);
    }
    return AIL_redbook_status(hand);
}

// FUNCTION: 0x000e6c20
U32 __cdecl AIL_API_redbook_pause(HREDBOOK hand)
{
    if (AIL_redbook_status(hand) == REDBOOK_PLAYING) {
        hand->pause_pos = AIL_redbook_position(hand);
        hand->paused = 1;
        rbstop(hand->drive);
    }
    return AIL_redbook_status(hand);
}

// FUNCTION: 0x000e6c60
U32 __cdecl AIL_API_redbook_resume(HREDBOOK hand)
{
    if (AIL_redbook_status(hand) == REDBOOK_PAUSED) {
        hand->paused = 0;
        AIL_redbook_play(hand, hand->pause_pos, hand->end);
    }
    return AIL_redbook_status(hand);
}
