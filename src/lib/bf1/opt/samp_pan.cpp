#include "snd.h"

extern "C" {

// FUNCTION: 0x000cdfc0
void SetSamplePan(u32 source_id, s16 number, u16 pan, u32 handle)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        if (number == 0 && source_id == 0) {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SampleHandle == handle && AIL_sample_status(smp->SampleHandle) == 4
                    && pan != smp->SamplePan) {
                    AIL_set_sample_pan(smp->SampleHandle, pan);
                    smp->SamplePan = pan;
                    return;
                }
            }
        } else {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SourceID == source_id && smp->SampleNumber == number
                    && AIL_sample_status(smp->SampleHandle) == 4 && pan != smp->SamplePan) {
                    AIL_set_sample_pan(smp->SampleHandle, pan);
                    smp->SamplePan = pan;
                    return;
                }
            }
        }
    }
}

}
