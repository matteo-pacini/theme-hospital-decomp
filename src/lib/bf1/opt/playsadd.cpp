#include "snd.h"

extern "C" {

// Plays the sample file at address as sample `number` of source source_id,
// at volume and pan (0-127), pitch (percent of its rate) and loop count
// loop + 1 (0 forever). type chooses the voice: 1 any free one, 2 a free
// one unless that sample is already playing, 3 the voice of the same sample
// (or of the same extra data, if any) or a free one. Returns the voice, or
// 0 if none was used.
// FUNCTION: 0x000ce990
SampleInfo *PlaySampleFromAddress(u32 source_id, s16 number, s32 volume, s32 pan, u16 pitch,
                                  s8 loop, u8 type, void *address, u32 extra)
{
    SampleInfo *smp;
    SampleInfo *busy;
    SampleInfo *cand;
    u8 found;

    if (SoundInstalled && SoundAble && SoundActive) {
        sprintf(SoundProgressMessage, "PLAYSADD    id-%08d    num-%03d    vol-%03d    pan-%03d    ptch-%03d    type-%d    pri-%03d\n", source_id, (s32)number, volume, pan, pitch, type, extra);
        SoundProgressLog(SoundProgressMessage);
        switch (type) {
        case 1:
            smp = 0;
            for (cand = sample_id; cand <= end_sample_id; cand++) {
                if (AIL_sample_status(cand->SampleHandle) == SMP_DONE) {
                    smp = cand;
                    busy = 0;
                    break;
                }
            }
            break;
        case 2:
            busy = 0;
            for (cand = sample_id; cand <= end_sample_id; cand++) {
                if (cand->SourceID == source_id && cand->SampleNumber == number
                    && AIL_sample_status(cand->SampleHandle) != SMP_DONE) {
                    busy = cand;
                    break;
                }
            }
            if (busy)
                break;
            smp = 0;
            for (cand = sample_id; cand <= end_sample_id; cand++) {
                if (AIL_sample_status(cand->SampleHandle) == SMP_DONE) {
                    smp = cand;
                    break;
                }
            }
            break;
        case 3:
            found = 0;
            for (cand = sample_id; cand <= end_sample_id; cand++) {
                if (cand->SourceID == source_id && cand->SampleNumber == number) {
                    smp = cand;
                    busy = 0;
                    found = 1;
                    break;
                }
            }
            if (!found && extra) {
                smp = 0;
                for (cand = sample_id; cand <= end_sample_id; cand++) {
                    if (cand->SourceID == source_id && cand->ExtraData == extra) {
                        smp = cand;
                        busy = 0;
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) {
                smp = 0;
                for (cand = sample_id; cand <= end_sample_id; cand++) {
                    if (AIL_sample_status(cand->SampleHandle) == SMP_DONE) {
                        smp = cand;
                        busy = 0;
                        break;
                    }
                }
            }
            break;
        }
        if (!smp || busy)
            return 0;
        AIL_init_sample(smp->SampleHandle);
        if (!address)
            return 0;
        AIL_set_sample_file(smp->SampleHandle, address, 1);
        AIL_set_sample_volume(smp->SampleHandle, volume);
        if (StereoSound)
            AIL_set_sample_pan(smp->SampleHandle, pan);
        if (pitch != 100)
            AIL_set_sample_playback_rate(smp->SampleHandle, pitch * ((s32 *)smp->SampleHandle)[0x13] / 100);
        AIL_set_sample_loop_count(smp->SampleHandle, loop + 1);
        AIL_start_sample(smp->SampleHandle);
        smp->SourceID = source_id;
        smp->SampleNumber = number;
        smp->SampleVolume = volume;
        smp->SamplePan = pan;
        smp->SamplePitch = pitch;
        smp->FadeState = 0;
        smp->FadeStep = 0;
        smp->unknown17 = 0;
        smp->ExtraData = extra;
        return smp;
    }
    return 0;
}

}
