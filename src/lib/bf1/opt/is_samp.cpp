#include "snd.h"

extern "C" {

// FUNCTION: 0x000cdee0
u8 IsSamplePlaying(u32 source_id, s16 number, u32 handle)
{
    SampleInfo *smp;

    if (SoundInstalled && SoundAble && SoundActive) {
        if (number == 0 && source_id == 0) {
            if (AIL_sample_status(handle) == 4)
                return 1;
        } else if (number == 0) {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SourceID == source_id && AIL_sample_status(smp->SampleHandle) == 4)
                    return 1;
            }
        } else {
            for (smp = sample_id; smp <= end_sample_id; smp++) {
                if (smp->SourceID == source_id && smp->SampleNumber == number
                    && AIL_sample_status(smp->SampleHandle) == 4)
                    return 1;
            }
        }
    }
    return 0;
}

}
