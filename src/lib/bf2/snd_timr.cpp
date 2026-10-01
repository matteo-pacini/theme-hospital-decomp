#include "snd.h"

struct SndTimer {
    s32 handle;
    u8 used;
};

extern "C" {

extern SndTimer sound_timer[5];

s32 __cdecl AIL_register_timer(void *callback);
void __cdecl AIL_set_timer_frequency(s32 timer, u32 hertz);
void __cdecl AIL_start_timer(s32 timer);

// Frees all five Bullfrog sound timer slots.
// FUNCTION: 0xeb5b0
void InitAllBullfrogSoundTimers(void)
{
    if (MusicInstalled || SoundInstalled)
        memset(sound_timer, 0, sizeof(sound_timer));
}

// Starts a Miles timer calling callback frequency times a second. Returns
// its slot number (1-5), or 0 if none is free or audio is not installed.
// FUNCTION: 0xeb5e0
u32 SetupBullfrogSoundTimer(u16 frequency, void *callback)
{
    SndTimer *timer;
    u32 i;

    if (MusicInstalled || SoundInstalled) {
        for (timer = sound_timer, i = 0; timer < &sound_timer[5]; timer++, i++) {
            if (timer->used == 0) {
                timer->handle = AIL_register_timer(callback);
                AIL_set_timer_frequency(timer->handle, frequency);
                AIL_start_timer(timer->handle);
                timer->used = 1;
                return i + 1;
            }
        }
    }
    return 0;
}

// Stops and frees sound timer slot number (1-5).
// FUNCTION: 0xeb660
void ReleaseBullfrogSoundTimer(s32 number)
{
    if (MusicInstalled || SoundInstalled) {
        if (number <= 5 && sound_timer[number - 1].used == 1) {
            AIL_release_timer_handle(sound_timer[number - 1].handle);
            sound_timer[number - 1].used = 0;
        }
    }
}

}
