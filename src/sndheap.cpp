#include "hosp.h"

struct SampleInfo {
    void *SampleHandle;
    s32 SampleVolume;
    u16 SamplePitch;
    u16 SamplePan;
    u16 FadeToVolume;
    u32 SourceID;
    s16 SampleNumber;
    u8 unknown14[3];
    u8 flags;
    u8 unknown18[5];
};

struct SampleTable {
    u32 offset;
    s32 length;
    u32 unknown8;
    TbCacheUsedBlock *block;
    u32 unknown10;
};

/* One entry of the sound bank's directory. */
struct SoundBankEntry {
    u32 offset;
    u32 base;
    u32 size;
    u32 unknownc;
};

/* One sample's header in the sound bank. */
struct SoundBankSample {
    char name[0x12];
    u32 offset;
    u8 unknown16[4];
    u32 length;
    u8 unknown1e;
    u8 unknown1f;
};

extern u8 *memstart;
extern u32 memsize;
extern s32 sound_file;
extern s32 samples_in_bank;
extern SampleTable *sample_table;

s32 parse_sound_file(s32 file, u8 *buf, s32 *count, s32 size, s32 rate);

extern "C" {
extern u8 D_00155db1;
extern u8 D_00155e1d;
extern TbCacheUsedBlock *D_00155dbe;
int LbFilePosition(int handle);
char IsSamplePlaying(s32 a, s32 b, void *handle);
SampleInfo *GetNumberOfSongs(void);
SampleInfo *GetCurrentSoundMasterVolume(void);
SampleInfo *PlaySampleFromAddress(u32 source, s16 sample, u32 volume, u32 pitch, u16 pan, s8 loop, u8 type, void *data, u32 length);
void *fn_00022b60(TbCacheUsedBlock *block);
void fn_000a2ed0(TbCacheUsedBlock *block);
}

// FUNCTION: 0x000a27a0
s32 init_sound_heap(u8 *mem, s32 size, char *path, s32 rate)
{
    s32 len;
    s32 i;
    s32 unused14;
    s32 unused18;

    memstart = mem;
    memsize = size;
    memset(mem, 0, size);
    if (sound_file != -1)
        close_sound_heap();
    sound_file = LbFileOpen(path, 2);
    if (sound_file == -1) {
        sub_error_number = 2;
        return 0;
    }
    len = parse_sound_file(sound_file, mem, &samples_in_bank, size, rate);
    if (len == 0) {
        sub_error_number = 3;
        close_sound_heap();
        return 0;
    }
    sample_table = (SampleTable *)mem;
    mem += len;
    size -= len;
    if (size <= 0) {
        sub_error_number = 4;
        close_sound_heap();
        return 0;
    }
    if (!cache.Initialise(mem, size)) {
        sub_error_number = 5;
        close_sound_heap();
        return 0;
    }
    {
        u16 j;
        SampleTable *st;

        for (j = 1; j < samples_in_bank; j++)
            st = &sample_table[j];
    }
    for (i = 1; i < samples_in_bank; i++) {
        if (sample_table[i].length >= 0x4b000) {
            sub_error_number = 6;
            return 0;
        }
    }
    return 1;
}

// FUNCTION: 0x000a292c
void close_sound_heap(void)
{
    if (sound_file != -1) {
        LbFileClose(sound_file);
        sound_file = -1;
    }
}

// FUNCTION: 0x000a2960
void process_sound_heap(void)
{
    SampleTable *st;
    SampleInfo *s;
    SampleInfo *end;

    if (!myplayer.unknown0[0xf] || !statvar.f6c[0x78])
        return;
    cache.UnlockAll();
    if (D_00155db1)
        fn_000a2ed0(D_00155dbe);
    if (D_00155e1d)
        fn_000a2ed0(D_00155dbe);
    end = GetCurrentSoundMasterVolume();
    for (s = GetNumberOfSongs(); s <= end; s++) {
        if (s->SampleHandle == 0 || !(s->flags & 1))
            continue;
        if (IsSamplePlaying(0, 0, s->SampleHandle)) {
            st = &sample_table[s->SampleNumber];
            if (st->block)
                fn_000a2ed0(st->block);
        } else {
            s->flags &= ~1;
        }
    }
}

// FUNCTION: 0x000a2a3c
SampleInfo *play_sample_using_heap(u32 source, s16 sample, u32 volume, u32 pitch, u32 pan, s8 loop, u8 type)
{
    SampleTable *st;
    u32 tag;
    void *data;
    SampleInfo *info;

    if (sound_file == -1)
        return 0;
    if (sample <= 0 || sample >= samples_in_bank)
        return 0;
    st = &sample_table[sample];
    if (st->block) {
        cache.MakeNewest(st->block);
    } else {
        for (;;) {
            st->block = cache.Allocate(st->length, sample + 1);
            if (st->block)
                break;
            tag = cache.ThrowOutOldest();
            if (tag == 0)
                return 0;
            sample_table[tag - 1].block = 0;
        }
        LbFileSeek(sound_file, st->offset, 0);
        LbFileRead(sound_file, fn_00022b60(st->block), st->length);
        cache.MakeNewest(st->block);
    }
    data = fn_00022b60(st->block);
    info = PlaySampleFromAddress(source, sample, volume, pitch, pan, loop, type, fn_00022b60(st->block), st->unknown8);
    if (info == 0)
        return 0;
    info->flags |= 1;
    fn_000a2ed0(st->block);
    return info;
}

// NONMATCHING: frame layout (target slots from the top: base, idx, entry, st, return value, i, table, pos)
// FUNCTION: 0x000a2bd0
s32 parse_sound_file(s32 file, u8 *buf, s32 *count, s32 size, s32 rate)
{
    SampleTable *st;
    SoundBankEntry *entry;
    s32 idx;
    u32 base;
    s32 pos;
    u32 table;
    s32 i;
    u8 header[0x12];
    SoundBankEntry dir[9];
    SoundBankSample rec;

    switch (rate) {
    case 0x66c:
        idx = 0;
        break;
    case 0x668:
        idx = 1;
        break;
    case 0x656:
        idx = 2;
        break;
    case 0x654:
        idx = 3;
        break;
    case 0x64b:
        idx = 4;
        break;
    case 0x64a:
        idx = 5;
        break;
    case 0x336:
        idx = 6;
        break;
    case 0x32b:
        idx = 7;
        break;
    case 0x320:
        idx = 8;
        break;
    default:
        return 0;
    }
    LbFileSeek(file, 0, 2);
    pos = LbFilePosition(file);
    LbFileSeek(file, pos - 4, 0);
    LbFileRead(file, &table, 4);
    LbFileSeek(file, table, 0);
    LbFileRead(file, header, 0x12);
    LbFileRead(file, dir, 0x90);
    entry = dir;
    entry += idx;
    if (entry->offset == 0 || entry->size == 0)
        return 0;
    *count = entry->size >> 5;
    if (*count * sizeof(SampleTable) >= size)
        return 0;
    LbFileSeek(file, entry->offset, 0);
    base = entry->base;
    st = (SampleTable *)buf;
    for (i = 0; i < *count; i++, st++) {
        LbFileRead(file, &rec, 0x20);
        st->offset = rec.offset + base;
        st->length = rec.length;
        st->block = 0;
        st->unknown8 = rec.unknown1e;
    }
    return *count * sizeof(SampleTable);
}

// FUNCTION: 0x000a2e74
void clear_sound_heap(void)
{
    u32 tag;

    cache.UnlockAll();
    StopAllSamples();
    while ((tag = cache.ThrowOutOldest()) != 0)
        sample_table[tag - 1].block = 0;
}

// FUNCTION: 0x000a2ed0
void fn_000a2ed0(TbCacheUsedBlock *block)
{
    block->locked = 1;
}
