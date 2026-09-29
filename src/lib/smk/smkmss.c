typedef unsigned long U32;
typedef long S32;
typedef unsigned char U8;
typedef struct _SAMPLE *HSAMPLE;
typedef struct _DIG_DRIVER *HDIGDRIVER;

#define NULL ((void *)0)

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

U32 mult64anddiv(U32 m1, U32 m2, U32 d);
#pragma aux mult64anddiv = \
    "mul ecx" \
    "div ebx" \
    parm [eax] [ecx] [ebx] \
    value [eax] \
    modify exact [eax edx];

S32 __cdecl AIL_set_preference(U32 number, S32 value);
HSAMPLE __cdecl AIL_allocate_sample_handle(HDIGDRIVER dig);
void __cdecl AIL_release_sample_handle(HSAMPLE S);
void __cdecl AIL_init_sample(HSAMPLE S);
void __cdecl AIL_set_sample_type(HSAMPLE S, S32 format, U32 flags);
void __cdecl AIL_end_sample(HSAMPLE S);
void __cdecl AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate);
void __cdecl AIL_set_sample_volume(HSAMPLE S, S32 volume);
void __cdecl AIL_set_sample_pan(HSAMPLE S, S32 pan);
HDIGDRIVER __cdecl AIL_install_DIG_driver_image(void *driver_image, U32 size, void *IO);
S32 __cdecl AIL_minimum_sample_buffer_size(HDIGDRIVER dig, S32 playback_rate, S32 format);
S32 __cdecl AIL_sample_buffer_ready(HSAMPLE S);
void __cdecl AIL_load_sample_buffer(HSAMPLE S, U32 buff_num, void *buffer, U32 len);
S32 __cdecl AIL_sample_buffer_info(HSAMPLE S, U32 *pos0, U32 *len0, U32 *pos1, U32 *len1);
void __cdecl AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value);
U32 __cdecl AIL_ms_count(void);
S32 __cdecl AIL_vmm_lock(void *start, U32 size);
S32 __cdecl AIL_vmm_unlock(void *start, U32 size);

typedef struct SmackSnd {
    U8 *buf;
    U8 *bufend;
    U8 *readpos;
    U8 *writepos;
    U32 pending;
    U32 bytespersec;
    U32 f18;
    U32 f1c;
    U32 f20;
    U32 f24;
    U32 f28;
    U32 f2c;
    U32 f30;
    U32 f34;
    U32 f38;
    struct SmackSnd *next;
    HSAMPLE sample;
    U32 halfsize;
    U32 rate;
    U32 flush;
    U32 bits16;
    U32 stereo;
    U32 starved;
    U32 f5c;
    U32 played;
    U8 *halves[2];
    U32 f6c;
    U32 f70;
    U32 minbuf;
    U32 loaded;
    U32 loadtime;
    U32 f80;
} SmackSnd;

extern SmackSnd *D_00151664;
extern HDIGDRIVER D_00151668;
extern U32 D_0015166c;
extern void *D_00151670;
extern void *D_0018cd90;
extern void *D_0018cd94;
extern U32 D_0018cd98;
extern U32 D_0018cd9c;
extern U32 D_0018cda0;

extern U32 (__pascal *SmackTimerReadAddr)(void);
extern void (__pascal *SmackTimerDoneAddr)(void);
extern void (__pascal *D_00151b98)(void);
extern void (__pascal *LowSoundCheckAddr)(void);
extern U8 (__pascal *LowSoundOpenAddr)(SmackSnd *snd, U32 flags);
extern void (__pascal *D_0018cdbc)(SmackSnd *snd);
extern U32 (__pascal *LowSoundPlayedAddr)(SmackSnd *snd);
extern void (__pascal *LowSoundPurgeAddr)(SmackSnd *snd);
extern void (__pascal *D_00151688)(SmackSnd *snd);
extern U8 (__pascal *D_0015168c)(SmackSnd *snd);
extern void (__pascal *D_00151690)(SmackSnd *snd, U32 volume, U32 pan);

void *__pascal RADMALLOC(U32 bytes);
void __pascal RADFREE(void *ptr);

void __pascal MSSLowSoundCheck(void);
void MSSInitSample(SmackSnd *snd);
U8 *__pascal MSSCopyWrap(U8 *dst, U8 *src, U8 *start, U8 *end, U32 n);
void __pascal MSSLowSoundVolPan(SmackSnd *snd, U32 volume, U32 pan);
void __pascal MSSSmackTimerSetup(void);
U32 __pascal MSSSmackTimerRead(void);
void __pascal MSSLowSoundClose(SmackSnd *snd);
void __pascal MSSLowSoundPurge(SmackSnd *snd);
U8 __pascal MSSLowSoundOpen(SmackSnd *snd, U32 flags);
U32 __pascal MSSLowSoundPlayed(SmackSnd *snd);

U8 __pascal SmackSoundUseMSS(void *digdriver);
#pragma aux (__pascal) SmackSoundUseMSS "fn_000e7910_";

// FUNCTION: 0xe7910
U8 __pascal SmackSoundUseMSS(void *digdriver)
{
    if (SmackTimerReadAddr == NULL) {
        LowSoundCheckAddr = MSSLowSoundCheck;
        LowSoundOpenAddr = MSSLowSoundOpen;
        D_0018cdbc = MSSLowSoundClose;
        LowSoundPlayedAddr = MSSLowSoundPlayed;
        LowSoundPurgeAddr = MSSLowSoundPurge;
        D_00151688 = MSSLowSoundPurge;
        D_0015168c = NULL;
        D_00151690 = MSSLowSoundVolPan;
        D_00151b98 = MSSSmackTimerSetup;
        SmackTimerReadAddr = MSSSmackTimerRead;
        SmackTimerDoneAddr = MSSSmackTimerSetup;
    }
    if (D_00151664 == NULL) {
        D_00151668 = digdriver;
        return 1;
    }
    return 0;
}

#pragma aux (__pascal) MSSLowSoundVolPan "fn_000e79b0_";

// FUNCTION: 0xe79b0
void __pascal MSSLowSoundVolPan(SmackSnd *snd, U32 volume, U32 pan)
{
    if (volume > 0x8000)
        volume = 0x8000;
    AIL_set_sample_volume(snd->sample, (mult64anddiv(volume, 1270, 0x8000) + 5) / 10);
    AIL_set_sample_pan(snd->sample, (mult64anddiv(pan, 1270, 0x10000) + 5) / 10);
}

#pragma aux (__pascal) MSSSmackTimerSetup "fn_000e7a20_";

// FUNCTION: 0xe7a20
void __pascal MSSSmackTimerSetup(void)
{
}

// FUNCTION: 0x000e7a30
U32 __pascal MSSSmackTimerRead(void)
{
    return AIL_ms_count();
}

#pragma aux (__pascal) MSSLowSoundClose "fn_000e7a40_";

// FUNCTION: 0xe7a40
void __pascal MSSLowSoundClose(SmackSnd *snd)
{
    if (D_00151664 == NULL)
        return;
    AIL_end_sample(snd->sample);
    AIL_release_sample_handle(snd->sample);
    AIL_vmm_unlock(snd->halves[0], snd->halfsize * 2);
    RADFREE(snd->halves[0]);
    if (snd == D_00151664) {
        D_00151664 = snd->next;
    } else {
        volatile SmackSnd *p = D_00151664;
        while (p->next != NULL) {
            if (snd == p->next) {
                p->next = snd->next;
                break;
            }
            p = p->next;
        }
    }
}

#pragma aux (__pascal) MSSLowSoundPurge "fn_000e7ac0_";

// FUNCTION: 0xe7ac0
void __pascal MSSLowSoundPurge(SmackSnd *snd)
{
    AIL_end_sample(snd->sample);
    snd->played = 0;
    snd->loaded = 0;
    snd->loadtime = 0;
    MSSInitSample(snd);
}

#pragma aux (__pascal) MSSLowSoundOpen "fn_000e7af0_";

// FUNCTION: 0xe7af0
U8 __pascal MSSLowSoundOpen(SmackSnd *snd, U32 flags)
{
    if (D_00151668 == NULL) {
        if (D_0015166c && D_00151670 != NULL) {
            AIL_set_preference(6, 0);
            AIL_set_preference(7, (flags & 0x20) || snd->stereo ? 1 : 0);
            AIL_set_preference(8, snd->bits16 != 0);
            D_00151668 = AIL_install_DIG_driver_image(D_00151670, D_0018cd98, NULL);
            if (D_00151668 == NULL) {
                D_00151668 = AIL_install_DIG_driver_image(D_0018cd94, D_0018cd9c, NULL);
                if (D_00151668 == NULL)
                    D_00151668 = AIL_install_DIG_driver_image(D_0018cd90, D_0018cda0, NULL);
            }
        }
        if (D_00151668 == NULL) {
            D_00151668 = (HDIGDRIVER)-1;
            return 0;
        }
    }
    if (D_00151668 == (HDIGDRIVER)-1)
        return 0;
    snd->minbuf = (AIL_minimum_sample_buffer_size(D_00151668, snd->rate,
        snd->bits16 ? (snd->stereo ? 3 : 1) : (snd->stereo ? 2 : 0)) + 3) & ~3;
    snd->halfsize = ((snd->bytespersec >> 2) + 0xfff) & ~0xfff;
    if (snd->minbuf > snd->halfsize)
        snd->halfsize = snd->minbuf;
    snd->halves[0] = RADMALLOC(snd->halfsize * 2);
    if (snd->halves[0] == NULL)
        return 0;
    AIL_vmm_lock(snd->halves[0], snd->halfsize * 2);
    snd->sample = AIL_allocate_sample_handle(D_00151668);
    if (snd->sample == NULL) {
        RADFREE(snd->halves[0]);
        return 0;
    }
    snd->halves[1] = (U8 *)((U32)snd->halves[0] + snd->halfsize);
    MSSInitSample(snd);
    snd->next = D_00151664;
    D_00151664 = snd;
    return 1;
}

/* config/functions.txt has no name for MSSInitSample. */
#pragma aux MSSInitSample "fn_000e7cd0_";

// FUNCTION: 0xe7cd0
void MSSInitSample(SmackSnd *snd)
{
    AIL_init_sample(snd->sample);
    AIL_set_sample_type(snd->sample,
        snd->bits16 ? (snd->stereo ? 3 : 1) : (snd->stereo ? 2 : 0), snd->bits16 != 0);
    AIL_set_sample_playback_rate(snd->sample, snd->rate);
    AIL_set_sample_user_data(snd->sample, 0, (S32)snd);
    AIL_set_sample_volume(snd->sample, 0x7f);
}

#pragma aux (__pascal) MSSLowSoundPlayed "fn_000e7d60_";

// FUNCTION: 0xe7d60
U32 __pascal MSSLowSoundPlayed(SmackSnd *snd)
{
    U32 played;

    MSSLowSoundCheck();
    played = snd->loadtime == 0
        ? snd->loaded
        : mult64anddiv(snd->bytespersec, SmackTimerReadAddr() - snd->loadtime, 1000);
    if (played > snd->loaded) {
        snd->loadtime = 0;
        played = snd->loaded;
    }
    return snd->played + played;
}

/* config/functions.txt has no name for MSSLowSoundCheck. */
#pragma aux (__pascal) MSSLowSoundCheck "fn_000e7db0_";

// FUNCTION: 0xe7db0
void __pascal MSSLowSoundCheck(void)
{
    SmackSnd *snd;
    U32 half;
    U32 n;
    U32 now;

    for (snd = D_00151664; snd != NULL; snd = snd->next) {
        if (AIL_sample_buffer_info(snd->sample, NULL, NULL, NULL, NULL))
            snd->starved = 1;
        if (snd->pending > snd->minbuf || (snd->flush && snd->pending > 3)) {
            half = AIL_sample_buffer_ready(snd->sample);
            if (half <= 1) {
                now = SmackTimerReadAddr();
                n = snd->pending & ~3;
                if (n > snd->halfsize)
                    n = snd->halfsize & ~3;
                snd->readpos = MSSCopyWrap(snd->halves[half], snd->readpos, snd->buf, snd->bufend, n);
                if (snd->loadtime == 0 || AIL_sample_buffer_info(snd->sample, NULL, NULL, NULL, NULL)) {
                    snd->loadtime = now;
                    snd->played += snd->loaded;
                    snd->loaded = 0;
                }
                AIL_load_sample_buffer(snd->sample, half, snd->halves[half], n);
                snd->loaded += n;
                snd->pending -= n;
            }
        }
    }
}

#pragma aux (__pascal) MSSCopyWrap "SMACKWRAPCOPY";

// FUNCTION: 0xe7ec0
U8 *__pascal MSSCopyWrap(U8 *dst, U8 *src, U8 *start, U8 *end, U32 n)
{
    U32 tail;

    tail = end - src;
    if (tail <= n) {
        if (tail) {
            radmemcpy(dst, src, tail);
            dst += tail;
            n -= tail;
        }
        src = start;
    }
    if (n) {
        radmemcpy(dst, src, n);
        src += n;
    }
    return src;
}
