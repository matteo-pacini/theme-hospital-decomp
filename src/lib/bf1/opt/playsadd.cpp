#include "snd.h"

extern "C" {
extern char D_00124c90[];

// FUNCTION: 0x000ce990
SampleInfo *PlaySampleFromAddress(u32 source_id, s16 number, s32 volume, s32 pan, u16 pitch,
                                  s8 loop, u8 type, void *address, u32 extra)
{
    SampleInfo *smp;
    SampleInfo *busy;
    SampleInfo *s;
    u8 found;

    if (SoundInstalled && SoundAble && SoundActive) {
        sprintf(SoundProgressMessage, D_00124c90, source_id, (s32)number, volume, pan, pitch, type, extra);
        SoundProgressLog(SoundProgressMessage);
        switch (type) {
        case 1:
            smp = 0;
            for (s = sample_id; s <= end_sample_id; s++) {
                if (AIL_sample_status(s->SampleHandle) == 2) {
                    smp = s;
                    busy = 0;
                    break;
                }
            }
            break;
        case 2:
            busy = 0;
            for (s = sample_id; s <= end_sample_id; s++) {
                if (s->SourceID == source_id && s->SampleNumber == number
                    && AIL_sample_status(s->SampleHandle) != 2) {
                    busy = s;
                    break;
                }
            }
            if (busy)
                break;
            smp = 0;
            for (s = sample_id; s <= end_sample_id; s++) {
                if (AIL_sample_status(s->SampleHandle) == 2) {
                    smp = s;
                    break;
                }
            }
            break;
        case 3:
            found = 0;
            for (s = sample_id; s <= end_sample_id; s++) {
                if (s->SourceID == source_id && s->SampleNumber == number) {
                    smp = s;
                    busy = 0;
                    found = 1;
                    break;
                }
            }
            if (!found && extra) {
                smp = 0;
                for (s = sample_id; s <= end_sample_id; s++) {
                    if (s->SourceID == source_id && s->ExtraData == extra) {
                        smp = s;
                        busy = 0;
                        found = 1;
                        break;
                    }
                }
            }
            if (!found) {
                smp = 0;
                for (s = sample_id; s <= end_sample_id; s++) {
                    if (AIL_sample_status(s->SampleHandle) == 2) {
                        smp = s;
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
