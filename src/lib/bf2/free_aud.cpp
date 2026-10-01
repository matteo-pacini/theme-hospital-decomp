#include "snd.h"

extern "C" {

extern u8 StreamedSoundAble;

void FreeStreamedSound(void);
void FreeRedbook(void);
void FreeMusic(void);
void FreeSound(void);

// Shuts down streamed sound, Red Book audio, music and sound effects.
// FUNCTION: 0xc20f0
void FreeAudio(void)
{
    if (StreamedSoundAble)
        FreeStreamedSound();
    FreeRedbook();
    FreeMusic();
    FreeSound();
}

}
