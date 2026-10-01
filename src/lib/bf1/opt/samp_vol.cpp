#include "snd.h"

extern "C" {

// Sets the volume (0-127) of a playing sample, chosen by handle (source_id
// and number both 0) or by source and number.
// FUNCTION: 0x000ce0b0
void SetSampleVolume(u32 source_id, s16 number, s16 volume, u32 handle)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        if (number == 0 && source_id == 0) {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SampleHandle == handle && AIL_sample_status(smp->SampleHandle) == 4
                    && volume >= 0 && volume <= 127 && volume != smp->SampleVolume) {
                    AIL_set_sample_volume(smp->SampleHandle, volume);
                    smp->SampleVolume = volume;
                    return;
                }
            }
        } else {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SourceID == source_id && smp->SampleNumber == number
                    && AIL_sample_status(smp->SampleHandle) == 4
                    && volume >= 0 && volume <= 127 && volume != smp->SampleVolume) {
                    AIL_set_sample_volume(smp->SampleHandle, volume);
                    smp->SampleVolume = volume;
                    return;
                }
            }
        }
    }
}

}
