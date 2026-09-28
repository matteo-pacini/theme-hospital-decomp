#include "snd.h"

extern volatile u8 samples_currently_fading;
extern volatile u8 switch_off_sample_fade_timer;
extern long sample_fade_handle;

extern "C" {
void fn_000ccd30(u32 user);

// NONMATCHING: jump table placement (target's lies before the function)
// FUNCTION: 0x000ccfb0
void FadeSample(u32 source_id, s16 number, s16 volume, u8 stop_flag, u8 step)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive && number <= NumberOfSamples) {
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            if (smp->SourceID == source_id && smp->SampleNumber == number
                && AIL_sample_status(smp->SampleHandle) != 2) {
                if (volume > 127)
                    volume = 127;
                if (volume == smp->SampleVolume)
                    return;
                smp->FadeState = 0;
                smp->FadeToVolume = volume;
                smp->FadeStep = step;
                smp->FadeState = (volume <= smp->SampleVolume) + 1;
                switch (stop_flag) {
                case 1:
                    smp->FadeStopFlag = 1;
                    break;
                case 2:
                    smp->FadeStopFlag = 2;
                    break;
                case 3:
                default:
                    smp->FadeStopFlag = 3;
                    break;
                case 4:
                    smp->FadeStopFlag = 4;
                    break;
                case 5:
                    smp->FadeStopFlag = 5;
                    break;
                }
                if (samples_currently_fading == 0) {
                    switch_off_sample_fade_timer = 0;
                    samples_currently_fading = 1;
                    sample_fade_handle = AIL_register_timer((void *)fn_000ccd30);
                    AIL_set_timer_frequency(sample_fade_handle, 20);
                    AIL_start_timer(sample_fade_handle);
                }
                return;
            }
        }
    }
}

// FUNCTION: 0x000cd100
void StopAllSampleFadeTimers(void)
{
    SampleInfo *smp;

    if (samples_currently_fading) {
        switch_off_sample_fade_timer = 1;
        AIL_release_timer_handle(sample_fade_handle);
        samples_currently_fading = 0;
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            smp->FadeState = 0;
            smp->FadeStep = 0;
        }
    }
}

}
