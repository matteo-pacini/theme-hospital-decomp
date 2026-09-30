#include "snd.h"

struct SndTimer {
    s32 handle;
    u8 used;
};

extern "C" {

extern SndTimer D_0018cf88[5];

s32 __cdecl AIL_register_timer(void *callback);
void __cdecl AIL_set_timer_frequency(s32 timer, u32 hertz);
void __cdecl AIL_start_timer(s32 timer);

// FUNCTION: 0xeb5b0
void InitAllBullfrogSoundTimers(void)
{
    if (MusicInstalled || SoundInstalled)
        memset(D_0018cf88, 0, sizeof(D_0018cf88));
}

// FUNCTION: 0xeb5e0
u32 SetupBullfrogSoundTimer(u16 frequency, void *callback)
{
    SndTimer *timer;
    u32 i;

    if (MusicInstalled || SoundInstalled) {
        for (timer = D_0018cf88, i = 0; timer < &D_0018cf88[5]; timer++, i++) {
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

// FUNCTION: 0xeb660
void ReleaseBullfrogSoundTimer(s32 number)
{
    if (MusicInstalled || SoundInstalled) {
        if (number <= 5 && D_0018cf88[number - 1].used == 1) {
            AIL_release_timer_handle(D_0018cf88[number - 1].handle);
            D_0018cf88[number - 1].used = 0;
        }
    }
}

}
