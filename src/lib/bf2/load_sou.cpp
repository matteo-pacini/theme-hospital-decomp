#include "snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

// FUNCTION: 0xeff20
void LoadSounds(s32 volume)
{
    if (SoundAble && SoundInstalled && volume != D_00150824 && volume <= 127 && volume >= 0
        && !SoundMasterVolumeOverride) {
        AIL_set_digital_master_volume(SoundDriver, volume);
        D_00150824 = volume;
    }
}

// FUNCTION: 0xf0220
void format_sounds(void)
{
    SoundBankEntry *p;

    if (Sfx && SfxData) {
        NumberOfSamples = 0;
        for (p = Sfx + 1; p < EndSfxs; p++) {
            p->data += (u32)SfxData;
            NumberOfSamples++;
        }
    }
}

// FUNCTION: 0xf0280
// NONMATCHING: RNC length computation allocates registers differently
u8 load_sound_bank(long file, u8 bank)
{
    BankHead head[9];
    u8 *data;
    u8 *tab;

    LbFilePosition(file);
    LbFileRead(file, head, sizeof(head));
    if (head[bank].data_offset != -1) {
        SoundAble = 0;
        if (SfxData && Sfx) {
            memset(SfxData, 0, largest_dat_size);
            memset(Sfx, 0, largest_tab_size);
            tab = (u8 *)Sfx;
            EndSfxs = (SoundBankEntry *)(tab + head[bank].tab_size);
            data = SfxData;
            LbFileSeek(file, head[bank].data_offset, 0);
            LbFileRead(file, SfxData, 8);
            if (data[0] == 'R' && data[1] == 'N' && data[2] == 'C') {
                LbFileRead(file, SfxData + 8, RNC_LENGTH(data) - 8);
                UnpackM1(data, data);
            } else {
                LbFileRead(file, SfxData + 8, head[bank].data_size - 8);
            }
            LbFileSeek(file, head[bank].tab_offset, 0);
            LbFileRead(file, Sfx, 8);
            if (tab[0] == 'R' && tab[1] == 'N' && tab[2] == 'C') {
                LbFileRead(file, (u8 *)Sfx + 8, RNC_LENGTH(tab) - 8);
                UnpackM1(tab, tab);
            } else {
                LbFileRead(file, (u8 *)Sfx + 8, head[bank].tab_size - 8);
            }
            format_sounds();
            SoundAble = 1;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0xf04a0
// NONMATCHING: register allocation differs around the file reads
s32 AllocateSoundBankMemory(u16 type)
{
    s32 file;
    s32 length;
    s32 data_size;
    s32 tab_size;

    sprintf(D_0018d4c4, D_001282de, SoundDataPath);
    file = LbFileOpen(D_0018d4c4, 2);
    if (file == -1)
        return -1;
    length = LbFileLength(D_0018d4c4);
    memset(D_0018d474, 0, 0x48);
    LbFileSeek(file, 0, 2);
    LbFileSeek(file, length - 0x4c, 0);
    data_size = 0;
    LbFileRead(file, D_0018d474, 0x48);
    tab_size = 0;
    LbFileClose(file);
    switch (type) {
    case 0x66c:
        data_size = D_0018d474[0].data_size;
        tab_size = D_0018d474[0].tab_size;
        break;
    case 0x668:
        data_size = D_0018d474[1].data_size;
        tab_size = D_0018d474[1].tab_size;
        break;
    case 0x656:
        data_size = D_0018d474[2].data_size;
        tab_size = D_0018d474[2].tab_size;
        break;
    case 0x654:
        data_size = D_0018d474[3].data_size;
        tab_size = D_0018d474[3].tab_size;
        break;
    case 0x64b:
        data_size = D_0018d474[4].data_size;
        tab_size = D_0018d474[4].tab_size;
        break;
    case 0x64a:
        data_size = D_0018d474[5].data_size;
        tab_size = D_0018d474[5].tab_size;
        break;
    case 0x336:
        data_size = D_0018d474[6].data_size;
        tab_size = D_0018d474[6].tab_size;
        break;
    case 0x32b:
        data_size = D_0018d474[7].data_size;
        tab_size = D_0018d474[7].tab_size;
        break;
    case 0x320:
        data_size = D_0018d474[8].data_size;
        tab_size = D_0018d474[8].tab_size;
        break;
    }
    if (data_size == 0 || tab_size == 0)
        return -2;
    SfxData = (u8 *)LbMemoryAlloc(data_size + 0x100);
    sprintf(SoundProgressMessage, D_001282eb, data_size + 0x100);
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
