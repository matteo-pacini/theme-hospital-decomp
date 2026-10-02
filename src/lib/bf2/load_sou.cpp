#include "snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

u8 load_sound_bank(long file, u8 type);

extern "C" {

extern volatile s8 CurrentSoundBank;

// Stops all samples and loads sample bank `bank` of SOUND.DAT for the
// current SoundType, unless it is already loaded. Returns 0 on success, 1
// otherwise (also when sound effects are off).
// NONMATCHING: bank-offset multiply uses eax instead of ecx; volatile load order
// FUNCTION: 0xeff70
u32 LoadSounds(u8 bank)
{
    u16 counts[9];
    u32 offset;
    s32 type;
    s32 file;
    u16 number;

    number = bank;
    sprintf(SoundProgressMessage, "BF42 - load sound bank %d\n", bank);
    SoundProgressLog(SoundProgressMessage);
    if (SoundInstalled && SoundAble && CurrentSoundBank != number && !DisableLoadSounds) {
        StopAllSamples();
        file = LbFileOpen(full_sound_data_path, 2);
        if (file != -1) {
            LbFileSeek(file, LbFileLength(full_sound_data_path) - 4, 0);
            LbFileRead(file, &offset, 4);
            LbFileSeek(file, offset, 0);
            LbFileRead(file, counts, sizeof(counts));
            number++;
            switch (SoundType) {
            case 0x66c:
                type = 0;
                break;
            case 0x668:
                type = 1;
                break;
            case 0x656:
                type = 2;
                break;
            case 0x654:
                type = 3;
                break;
            case 0x64b:
                type = 4;
                break;
            case 0x64a:
                type = 5;
                break;
            case 0x336:
                type = 6;
                break;
            case 0x32b:
                type = 7;
                break;
            case 0x320:
                type = 8;
                break;
            }
            if (number > counts[type]) {
                LbFileClose(file);
                sprintf(SoundProgressMessage, "BF43 - load sound bank - failed - bank not found\n");
                SoundProgressLog(SoundProgressMessage);
                return 1;
            }
            number--;
            LbFileSeek(file, number * 0x90, 1);
            if (!load_sound_bank(file, type)) {
                LbFileClose(file);
                sprintf(SoundProgressMessage, "BF44 - load sound bank - failed - cannot allocate\n");
                SoundProgressLog(SoundProgressMessage);
                return 1;
            }
            CurrentSoundBank = bank;
            LbFileClose(file);
            sprintf(SoundProgressMessage, "BF45 - load sound bank - passed\n");
            SoundProgressLog(SoundProgressMessage);
            return 0;
        }
        sprintf(SoundProgressMessage, "BF46 - load sound bank - failed - no sound.dat\n");
    } else if (!SoundInstalled) {
        sprintf(SoundProgressMessage, "BF47 - load sound bank - failed - Sound not installed\n");
    } else if (!SoundAble) {
        sprintf(SoundProgressMessage, "BF47 - load sound bank - failed - SoundAble = 0\n");
    } else if (DisableLoadSounds) {
        sprintf(SoundProgressMessage, "BF47 - load sound bank - failed - LoadSounds disabled\n");
    } else {
        sprintf(SoundProgressMessage, "BF47 - load sound bank - failed - already loaded\n");
    }
    SoundProgressLog(SoundProgressMessage);
    return 1;
}

}

// Turns the loaded sample table's offsets into pointers into the sample
// data and counts the samples (NumberOfSamples).
// FUNCTION: 0xf0220
void format_sounds(void)
{
    SoundBankEntry *entry;

    if (Sfx && SfxData) {
        NumberOfSamples = 0;
        for (entry = Sfx + 1; entry < EndSfxs; entry++) {
            entry->data += (u32)SfxData;
            NumberOfSamples++;
        }
    }
}

// Reads the bank whose nine per-type headers are at the open sound file's
// current position, taking the version for `type` (SoundType index 0-8),
// into the sample buffers, unpacking RNC data. Returns 1, or 0 if that type
// is missing (data_offset -1) or the buffers are not allocated; SoundAble
// is cleared while loading and stays 0 in the latter case.
// FUNCTION: 0xf0280
// NONMATCHING: the else branches compute head[type] into the argument register
// (ebx/edx) and load file into eax first; ours goes through eax
u8 load_sound_bank(long file, u8 type)
{
    BankHead head[9];
    u8 *data;
    u8 *tab;
    u32 packed_size;

    LbFilePosition(file);
    LbFileRead(file, head, sizeof(head));
    if (head[type].data_offset != -1) {
        SoundAble = 0;
        if (SfxData && Sfx) {
            memset(SfxData, 0, largest_dat_size);
            memset(Sfx, 0, largest_tab_size);
            tab = (u8 *)Sfx;
            EndSfxs = (SoundBankEntry *)(tab + head[type].tab_size);
            data = SfxData;
            LbFileSeek(file, head[type].data_offset, 0);
            LbFileRead(file, SfxData, 8);
            if (data[0] == 'R' && data[1] == 'N' && data[2] == 'C') {
                packed_size = data[4];
                packed_size = (packed_size << 8) + data[5];
                packed_size = (packed_size << 8) + data[6];
                packed_size = (packed_size << 8) + data[7];
                LbFileRead(file, SfxData + 8, packed_size - 8);
                UnpackM1(data, data);
            } else {
                LbFileRead(file, SfxData + 8, head[type].data_size - 8);
            }
            LbFileSeek(file, head[type].tab_offset, 0);
            LbFileRead(file, Sfx, 8);
            if (tab[0] == 'R' && tab[1] == 'N' && tab[2] == 'C') {
                packed_size = tab[4];
                packed_size = (packed_size << 8) + tab[5];
                packed_size = (packed_size << 8) + tab[6];
                packed_size = (packed_size << 8) + tab[7];
                LbFileRead(file, (u8 *)Sfx + 8, packed_size - 8);
                UnpackM1(tab, tab);
            } else {
                LbFileRead(file, (u8 *)Sfx + 8, head[type].tab_size - 8);
            }
            format_sounds();
            SoundAble = 1;
            return 1;
        }
    }
    return 0;
}

// Sets full_sound_data_path to <SoundDataPath>/SOUND.DAT and allocates the
// sample buffers for sound type `type`'s banks, sized from the end of that
// file. Returns 1, 0 if memory ran out, -1 if there is no
// SOUND.DAT, -2 if it has no bank of that type.
// FUNCTION: 0xf04a0
s32 AllocateSoundBankMemory(u16 type)
{
    s32 file;
    s32 length;
    s32 data_size;
    s32 tab_size;

    sprintf(full_sound_data_path, "%s/SOUND.DAT", SoundDataPath);
    file = LbFileOpen(full_sound_data_path, 2);
    if (file == -1)
        return -1;
    tab_size = 0;
    data_size = 0;
    length = LbFileLength(full_sound_data_path);
    memset(sound_bank_size_info, 0, 0x48);
    LbFileSeek(file, 0, 2);
    LbFileSeek(file, length - 0x4c, 0);
    LbFileRead(file, sound_bank_size_info, 0x48);
    LbFileClose(file);
    switch (type) {
    case 0x66c:
        data_size = sound_bank_size_info[0].data_size;
        tab_size = sound_bank_size_info[0].tab_size;
        break;
    case 0x668:
        data_size = sound_bank_size_info[1].data_size;
        tab_size = sound_bank_size_info[1].tab_size;
        break;
    case 0x656:
        data_size = sound_bank_size_info[2].data_size;
        tab_size = sound_bank_size_info[2].tab_size;
        break;
    case 0x654:
        data_size = sound_bank_size_info[3].data_size;
        tab_size = sound_bank_size_info[3].tab_size;
        break;
    case 0x64b:
        data_size = sound_bank_size_info[4].data_size;
        tab_size = sound_bank_size_info[4].tab_size;
        break;
    case 0x64a:
        data_size = sound_bank_size_info[5].data_size;
        tab_size = sound_bank_size_info[5].tab_size;
        break;
    case 0x336:
        data_size = sound_bank_size_info[6].data_size;
        tab_size = sound_bank_size_info[6].tab_size;
        break;
    case 0x32b:
        data_size = sound_bank_size_info[7].data_size;
        tab_size = sound_bank_size_info[7].tab_size;
        break;
    case 0x320:
        data_size = sound_bank_size_info[8].data_size;
        tab_size = sound_bank_size_info[8].tab_size;
        break;
    }
    if (data_size != 0 && tab_size != 0) {
        SfxData = (u8 *)LbMemoryAlloc(data_size + 0x100);
        sprintf(SoundProgressMessage, "  Sound Data size = %d  ", data_size + 0x100);
        SoundProgressLog(SoundProgressMessage);
        Sfx = (SoundBankEntry *)LbMemoryAlloc(tab_size + 0x100);
        if (!SfxData || !Sfx) {
            LbMemoryFree(SfxData);
            LbMemoryFree(Sfx);
            return 0;
        }
        largest_dat_size = data_size;
        largest_tab_size = tab_size;
        memset(SfxData, 0, data_size);
        memset(Sfx, 0, largest_tab_size);
        return 1;
    }
    return -2;
}

// Frees the sample buffers.
// FUNCTION: 0xf0710
void DeInitSoundBank(void)
{
    if (SfxData)
        LbMemoryFree(SfxData);
    if (Sfx)
        LbMemoryFree(Sfx);
}
