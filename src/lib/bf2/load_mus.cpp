#include "snd.h"

/* Built without inline expansion: callees in this file stay calls. */
#pragma inline_depth(0)

u8 load_music_bank(long file, u8 bank);

extern "C" {


// FUNCTION: 0xf0740
u32 LoadMusic(u16 bank)
{
    u16 counts[4];
    u32 offset;
    s32 type;
    s32 file;

    sprintf(SoundProgressMessage, "BF48 - load music bank %d\n", bank);
    SoundProgressLog(SoundProgressMessage);
    if (MusicInstalled && MusicAble && DisableLoadMusic == 0) {
        StopMusic();
        file = LbFileOpen(D_0018d5c4, 2);
        if (file != -1) {
            LbFileSeek(file, 0, 2);
            LbFileSeek(file, LbFilePosition(file) - 4, 0);
            LbFileRead(file, &offset, 4);
            LbFileSeek(file, offset, 0);
            LbFileRead(file, counts, sizeof(counts));
            bank++;
            switch (MusicType[0]) {
            case 'G':
            case 'g':
                type = 0;
                break;
            case 'R':
            case 'r':
                type = 1;
                break;
            case 'F':
            case 'f':
                type = 2;
                break;
            case 'W':
            case 'w':
                type = 3;
                break;
            }
            if (bank > counts[type]) {
                LbFileClose(file);
                sprintf(SoundProgressMessage, "BF49 - load music bank - failed - bank not found\n");
                SoundProgressLog(SoundProgressMessage);
                return 1;
            }
            bank--;
            LbFileSeek(file, bank * 64, 1);
            if (!load_music_bank(file, type)) {
                LbFileClose(file);
                sprintf(SoundProgressMessage, "BF50 - load music bank - failed - cannot allocate\n");
                SoundProgressLog(SoundProgressMessage);
                return 1;
            }
            LbFileClose(file);
            sprintf(SoundProgressMessage, "BF51 - load music bank - passed\n");
            SoundProgressLog(SoundProgressMessage);
            return 0;
        }
        sprintf(SoundProgressMessage, "BF52 - load music bank - failed - no music.dat\n");
    } else if (!MusicInstalled) {
        sprintf(SoundProgressMessage, "BF53 - load music bank - failed - music not installed\n");
    } else if (!MusicAble) {
        sprintf(SoundProgressMessage, "BF53 - load music bank - failed - MusicAble = 0\n");
    } else if (DisableLoadMusic) {
        sprintf(SoundProgressMessage, "BF53 - load music bank - failed - LoadMusic Disabled = 0\n");
    } else {
        return 1;
    }
    SoundProgressLog(SoundProgressMessage);
    return 1;
}

}

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
// NONMATCHING: the tab_offset seek and the else branch compute head[bank] into
// the argument register and load file into eax first; ours goes through eax
u8 load_music_bank(long file, u8 bank)
{
    BankHead head[4];
    u8 *data;
    u8 *tab;
    u8 i;
    u32 n;

    LbFilePosition(file);
    LbFileRead(file, head, sizeof(head));
    if (head[bank].data_offset != -1 && MusicData && Music) {
        data = MusicData;
        tab = (u8 *)Music;
        EndMusic = (SoundBankEntry *)(tab + head[bank].tab_size);
        LbFileSeek(file, head[bank].data_offset, 0);
        LbFileRead(file, data, 8);
        if (data[0] == 'R' && data[1] == 'N' && data[2] == 'C') {
            n = data[4];
            n = (n << 8) + data[5];
            n = (n << 8) + data[6];
            n = (n << 8) + data[7];
            LbFileRead(file, MusicData + 8, n - 8);
            UnpackM1(data, data);
        } else {
            LbFileRead(file, MusicData + 8, head[bank].data_size - 8);
        }
        LbFileSeek(file, head[bank].tab_offset, 0);
        LbFileRead(file, tab, 8);
        if (tab[0] == 'R' && tab[1] == 'N' && tab[2] == 'C') {
            n = tab[4];
            n = (n << 8) + tab[5];
            n = (n << 8) + tab[6];
            n = (n << 8) + tab[7];
            LbFileRead(file, (u8 *)Music + 8, n - 8);
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
s32 AllocateMusicBankMemory(void)
{
    s32 file;
    s32 length;
    s32 data_size;
    s32 tab_size;

    sprintf(D_0018d5c4, "%s/MUSIC.DAT", MusicDataPath);
    file = LbFileOpen(D_0018d5c4, 2);
    if (file == -1)
        return -1;
    tab_size = 0;
    data_size = 0;
    memset(D_0018d5a4, 0, 0x20);
    LbFileSeek(file, 0, 2);
    length = LbFilePosition(file);
    LbFileSeek(file, 0, 2);
    LbFileSeek(file, length - 0x24, 0);
    LbFileRead(file, D_0018d5a4, 0x20);
    LbFileClose(file);
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
    if (data_size != 0 && tab_size != 0) {
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
    return -2;
}

// FUNCTION: 0xf0d80
void DeInitMusicBank(void)
{
    if (MusicData)
        LbMemoryFree(MusicData);
    if (Music)
        LbMemoryFree(Music);
}
