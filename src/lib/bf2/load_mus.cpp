#include "snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

// FUNCTION: 0xf0960
void format_music(void)
{
    SoundBankEntry *p;

    if (Music && MusicData) {
        NumberOfSongs = 0;
        for (p = Music + 1; p < EndMusic; p++) {
            p->data += (u32)MusicData;
            NumberOfSongs++;
        }
    }
}

// FUNCTION: 0xf09c0
// NONMATCHING: RNC length computation allocates registers differently
u8 load_music_bank(long file, u8 bank)
{
    BankHead head[4];
    u8 *data;
    u8 *tab;
    u8 i;

    LbFilePosition(file);
    LbFileRead(file, head, sizeof(head));
    if (head[bank].data_offset != -1 && MusicData && Music) {
        data = MusicData;
        tab = (u8 *)Music;
        EndMusic = (SoundBankEntry *)(tab + head[bank].tab_size);
        LbFileSeek(file, head[bank].data_offset, 0);
        LbFileRead(file, data, 8);
        if (data[0] == 'R' && data[1] == 'N' && data[2] == 'C') {
            LbFileRead(file, MusicData + 8, RNC_LENGTH(data) - 8);
            UnpackM1(data, data);
        } else {
            LbFileRead(file, MusicData + 8, head[bank].data_size - 8);
        }
        LbFileSeek(file, head[bank].tab_offset, 0);
        LbFileRead(file, tab, 8);
        if (tab[0] == 'R' && tab[1] == 'N' && tab[2] == 'C') {
            LbFileRead(file, (u8 *)Music + 8, RNC_LENGTH(tab) - 8);
            UnpackM1(tab, tab);
        } else {
            LbFileRead(file, (u8 *)Music + 8, head[bank].tab_size - 8);
        }
        format_music();
        for (i = 1; i <= NumberOfSongs; i++)
            AIL_init_sequence(SongHandle, Music[i].data, 0);
        MusicAble = 1;
        return 1;
    }
    return 0;
}

// FUNCTION: 0xf0bc0
// NONMATCHING: register allocation differs around the file reads
s32 AllocateMusicBankMemory(void)
{
    s32 file;
    s32 length;
    s32 data_size;
    s32 tab_size;

    sprintf(D_0018d5c4, D_00128477, MusicDataPath);
    file = LbFileOpen(D_0018d5c4, 2);
    if (file == -1)
        return -1;
    memset(D_0018d5a4, 0, 0x20);
    LbFileSeek(file, 0, 2);
    length = LbFilePosition(file);
    LbFileSeek(file, 0, 2);
    LbFileSeek(file, length - 0x24, 0);
    LbFileRead(file, D_0018d5a4, 0x20);
    tab_size = 0;
    LbFileClose(file);
    data_size = 0;
    switch (MusicType[0]) {
    case 'G':
    case 'g':
        data_size = D_0018d5a4[2].data_size;
        tab_size = D_0018d5a4[2].tab_size;
        break;
    case 'R':
    case 'r':
        data_size = D_0018d5a4[1].data_size;
        tab_size = D_0018d5a4[1].tab_size;
        break;
    case 'F':
    case 'f':
        data_size = D_0018d5a4[0].data_size;
        tab_size = D_0018d5a4[0].tab_size;
        break;
    case 'W':
    case 'w':
        data_size = D_0018d5a4[3].data_size;
        tab_size = D_0018d5a4[3].tab_size;
        break;
    }
    if (data_size == 0 || tab_size == 0)
        return -2;
    MusicData = (u8 *)LbMemoryAlloc(data_size + 0x100);
    Music = (SoundBankEntry *)LbMemoryAlloc(tab_size + 0x100);
    if (!MusicData || !Music) {
        LbMemoryFree(MusicData);
        LbMemoryFree(Music);
        return 0;
    }
    memset(MusicData, 0, data_size);
    memset(Music, 0, tab_size);
    return 1;
}
