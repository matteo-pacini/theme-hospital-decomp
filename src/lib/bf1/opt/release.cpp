#include "snd.h"

extern "C" {

// Lets looping sample `number` of source source_id finish its current loop
// and stop.
// FUNCTION: 0x000cecb0
void ReleaseLoopedSample(u32 source_id, s16 number)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            if (smp->SourceID == source_id && smp->SampleNumber == number
                && AIL_sample_status(smp->SampleHandle) == SMP_PLAYING) {
                AIL_set_sample_loop_count(smp->SampleHandle, 1);
                return;
            }
        }
    }
}

}
