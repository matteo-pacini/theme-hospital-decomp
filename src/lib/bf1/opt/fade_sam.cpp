#include "snd.h"

extern volatile u8 samples_currently_fading;
extern volatile u8 switch_off_sample_fade_timer;
extern long sample_fade_handle;

extern "C" {
extern volatile u8 D_0018a57d;

void fn_000ccd30(u32 user);
#pragma aux (__pascal) fn_000ccd30 "fn_000ccd30_"

// NONMATCHING: target range also holds FadeSample's jump table
// FUNCTION: 0x000ccd30
void fn_000ccd30(u32 user)
{
    volatile SampleInfo *smp;
    volatile u8 i;

    if (SoundInstalled && SoundAble && SoundActive) {
        if (switch_off_sample_fade_timer) {
            AIL_release_timer_handle(sample_fade_handle);
            return;
        }
        D_0018a57d = 0;
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            switch (smp->FadeState) {
            case 1:
                D_0018a57d = 1;
                if (AIL_sample_status(smp->SampleHandle) == 4) {
                    if ((u16)smp->FadeToVolume > 127)
                        smp->FadeToVolume = 127;
                    if ((u16)smp->FadeToVolume <= smp->SampleVolume) {
                        smp->FadeState = 0;
                        if (smp->FadeStep) {
                            AIL_end_sample(smp->SampleHandle);
                            smp->FadeStep = 0;
                        }
                    }
                    if (smp->FadeState == 1) {
                        for (i = 1; i <= smp->FadeStopFlag; i++) {
                            smp->SampleVolume++;
                            if (smp->SampleVolume >= 127) {
                                smp->SampleVolume = 127;
                                AIL_set_sample_volume(smp->SampleHandle, smp->SampleVolume);
                                goto next;
                            }
                            if ((u16)smp->FadeToVolume <= smp->SampleVolume) {
                                smp->SampleVolume = (u16)smp->FadeToVolume;
                                break;
                            }
                        }
                        AIL_set_sample_volume(smp->SampleHandle, smp->SampleVolume);
                    }
                } else {
                    smp->FadeState = 0;
                    smp->FadeStep = 0;
                }
                break;
            case 2:
                D_0018a57d = 1;
                if (AIL_sample_status(smp->SampleHandle) == 4) {
                    if ((u16)smp->FadeToVolume >= smp->SampleVolume) {
                        smp->FadeState = 0;
                        if (smp->FadeStep) {
                            AIL_end_sample(smp->SampleHandle);
                            smp->FadeStep = 0;
                        }
                    }
                    if (smp->FadeState == 2) {
                        for (i = 1; i <= smp->FadeStopFlag; i++) {
                            smp->SampleVolume--;
                            if (smp->SampleVolume <= 0) {
                                smp->SampleVolume = 0;
                                AIL_set_sample_volume(smp->SampleHandle, smp->SampleVolume);
                                goto next;
                            }
                            if ((u16)smp->FadeToVolume >= smp->SampleVolume) {
                                smp->SampleVolume = (u16)smp->FadeToVolume;
                                break;
                            }
                        }
                        AIL_set_sample_volume(smp->SampleHandle, smp->SampleVolume);
                    }
                } else {
                    smp->FadeState = 0;
                    smp->FadeStep = 0;
                }
                break;
            }
        next:;
        }
        if (D_0018a57d == 0) {
            switch_off_sample_fade_timer = 1;
            samples_currently_fading = 0;
        }
    } else {
        switch_off_sample_fade_timer = 1;
        samples_currently_fading = 0;
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            smp->FadeState = 0;
            smp->FadeStep = 0;
        }
    }
}

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
