#include "snd.h"

extern "C" {

// Stops sample `number` of source source_id if it is playing.
// FUNCTION: 0x000cccc0
void StopSample(u32 source_id, s16 number)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            if (smp->SourceID == source_id && smp->SampleNumber == number
                && AIL_sample_status(smp->SampleHandle) != 2) {
                AIL_end_sample(smp->SampleHandle);
                return;
            }
        }
    }
}

}
