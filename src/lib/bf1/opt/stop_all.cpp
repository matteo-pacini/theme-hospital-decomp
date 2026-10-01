#include "snd.h"

extern "C" {
void StopAllSampleFadeTimers(void);
void StopSampleQueueList(void);

// Stops every sample, its fades and the sample queue.
// FUNCTION: 0x000c8320
void StopAllSamples(void)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        StopAllSampleFadeTimers();
        for (smp = sample_id; smp <= end_sample_id; smp++) {
            AIL_end_sample(smp->SampleHandle);
            smp->SampleNumber = 0;
            smp->SourceID = 0;
            smp->FadeState = 0;
            smp->FadeStep = 0;
        }
        StopSampleQueueList();
    }
}

}
