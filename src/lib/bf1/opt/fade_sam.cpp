#include "snd.h"

extern volatile u8 samples_currently_fading;
extern volatile u8 switch_off_sample_fade_timer;
extern long sample_fade_handle;

extern "C" {
extern volatile u8 a_sample_is_fading;

void cbfadesample(u32 user);
#pragma aux (__pascal) cbfadesample "W?cbfadesample$n(ul)v"

// Sample fade timer callback (20 Hz): moves each fading sample's volume up
// or down by its rate towards its target, ending samples that were to stop
// once reached, and releases the timer when nothing is fading.
// The target range also holds FadeSample's jump table.
// FUNCTION: 0x000ccd30
void cbfadesample(u32 user)
{
    SampleInfo *smp;
    volatile u8 i;

    if (SoundInstalled && SoundAble && SoundActive) {
        if (switch_off_sample_fade_timer) {
            AIL_release_timer_handle(sample_fade_handle);
            return;
        }
        a_sample_is_fading = 0;
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            switch (smp->FadeState) {
            case 1:
                a_sample_is_fading = 1;
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
                            ++smp->SampleVolume;
                            if (smp->SampleVolume >= 127) {
                                smp->SampleVolume = 127;
                                break;
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
                a_sample_is_fading = 1;
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
                            --smp->SampleVolume;
                            if (smp->SampleVolume <= 0) {
                                smp->SampleVolume = 0;
                                break;
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
        }
        if (a_sample_is_fading == 0) {
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

// Fades the playing sample `number` of source source_id to volume (0-127),
// by rate (1-5) volume steps per 50 ms tick, ending it when the target is
// reached if stop_when_done is set. Starts the fade timer if needed.
// FUNCTION: 0x000ccfb0
void FadeSample(u32 source_id, s16 number, s16 volume, u8 rate, u8 stop_when_done)
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
                smp->FadeStep = stop_when_done;
                smp->FadeState = (volume <= smp->SampleVolume) + 1;
                switch (rate) {
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
                    sample_fade_handle = AIL_register_timer((void *)cbfadesample);
                    AIL_set_timer_frequency(sample_fade_handle, 20);
                    AIL_start_timer(sample_fade_handle);
                }
                return;
            }
        }
    }
}

// Stops all sample fades and releases the fade timer.
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
