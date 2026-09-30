#include "snd.h"

u8 sample_queue_handle_stopped = 1;

extern "C" {

extern volatile u32 D_0018cd80;

void __cdecl AIL_init_sample(_SAMPLE *sample);
s32 __cdecl AIL_sample_user_data(_SAMPLE *sample, u32 index);
u32 __cdecl AIL_set_sample_file(_SAMPLE *sample, void *file, s32 block);
void __cdecl AIL_start_sample(_SAMPLE *sample);

/* End-of-sample callback: __pascal (callee pops), address-named. */
void fn_000e7460(_SAMPLE *sample);
#pragma aux fn_000e7460 "*_" parm routine [] modify [eax ebx ecx edx];

// FUNCTION: 0xe7460
void fn_000e7460(_SAMPLE *sample)
{
    if (sample_queue_handle_stopped)
        return;
    if (AIL_sample_user_data(sample_queue_handle, D_0018cd80) == 0)
        return;
    if (D_0018cd80 >= 8)
        return;
    AIL_init_sample(sample_queue_handle);
    AIL_register_EOS_callback(sample_queue_handle, fn_000e7460);
    AIL_set_sample_file(sample_queue_handle,
                        (void *)Sfx[AIL_sample_user_data(sample_queue_handle, D_0018cd80)].data, 1);
    D_0018cd80++;
    AIL_start_sample(sample_queue_handle);
}

// FUNCTION: 0xe7520
void fn_000e7520(s32 s0, s32 s1, s32 s2, s32 s3, s32 s4, s32 s5, s32 s6, s32 s7)
{
    if (SoundInstalled && SoundAble && SoundActive) {
        if (!sample_queue_handle_initiated) {
            sample_queue_handle = (_SAMPLE *)AIL_allocate_sample_handle(SoundDriver);
            sample_queue_handle_initiated = 1;
        }
        sample_queue_handle_stopped = 1;
        AIL_end_sample(sample_queue_handle);
        AIL_set_sample_user_data(sample_queue_handle, 0, s0);
        AIL_set_sample_user_data(sample_queue_handle, 1, s1);
        AIL_set_sample_user_data(sample_queue_handle, 2, s2);
        AIL_set_sample_user_data(sample_queue_handle, 3, s3);
        AIL_set_sample_user_data(sample_queue_handle, 4, s4);
        AIL_set_sample_user_data(sample_queue_handle, 5, s5);
        AIL_set_sample_user_data(sample_queue_handle, 6, s6);
        AIL_set_sample_user_data(sample_queue_handle, 7, s7);
        D_0018cd80 = 0;
        sample_queue_handle_stopped = 0;
        fn_000e7460(0);
    }
}

}

// FUNCTION: 0xe7650
void StopSampleQueueList(void)
{
    if (sample_queue_handle_initiated) {
        sample_queue_handle_stopped = 1;
        AIL_register_EOS_callback(sample_queue_handle, 0);
        AIL_end_sample(sample_queue_handle);
        AIL_set_sample_user_data(sample_queue_handle, 0, 0);
        AIL_set_sample_user_data(sample_queue_handle, 1, 0);
        AIL_set_sample_user_data(sample_queue_handle, 2, 0);
        AIL_set_sample_user_data(sample_queue_handle, 3, 0);
        AIL_set_sample_user_data(sample_queue_handle, 4, 0);
        AIL_set_sample_user_data(sample_queue_handle, 5, 0);
        AIL_set_sample_user_data(sample_queue_handle, 6, 0);
        AIL_set_sample_user_data(sample_queue_handle, 7, 0);
    }
}
