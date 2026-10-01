#include "snd.h"

u8 sample_queue_handle_stopped = 1;

extern "C" {

extern volatile u32 current_sample_queue_count;

void __cdecl AIL_init_sample(_SAMPLE *S);
s32 __cdecl AIL_sample_user_data(_SAMPLE *S, u32 index);
u32 __cdecl AIL_set_sample_file(_SAMPLE *S, void *file_image, s32 block);
void __cdecl AIL_start_sample(_SAMPLE *S);

/* End-of-sample callback: __pascal (callee pops), address-named. */
void cb_sample_queue_callback(_SAMPLE *sample);
#pragma aux cb_sample_queue_callback "*_" parm routine [] modify [eax ebx ecx edx];

// End-of-sample callback of the sample queue: starts the queue's next
// sample (up to eight), if any.
// FUNCTION: 0xe7460
void cb_sample_queue_callback(_SAMPLE *sample)
{
    if (sample_queue_handle_stopped)
        return;
    if (AIL_sample_user_data(sample_queue_handle, current_sample_queue_count) == 0)
        return;
    if (current_sample_queue_count >= 8)
        return;
    AIL_init_sample(sample_queue_handle);
    AIL_register_EOS_callback(sample_queue_handle, cb_sample_queue_callback);
    AIL_set_sample_file(sample_queue_handle,
                        (void *)Sfx[AIL_sample_user_data(sample_queue_handle, current_sample_queue_count)].data, 1);
    current_sample_queue_count++;
    AIL_start_sample(sample_queue_handle);
}

// Plays the sound bank samples sfx0-sfx7 one after another on the queue's
// voice (a 0 ends the list), replacing any list playing.
// FUNCTION: 0xe7520
void PlaySampleList(s32 sfx0, s32 sfx1, s32 sfx2, s32 sfx3, s32 sfx4, s32 sfx5, s32 sfx6, s32 sfx7)
{
    if (SoundInstalled && SoundAble && SoundActive) {
        if (!sample_queue_handle_initiated) {
            sample_queue_handle = (_SAMPLE *)AIL_allocate_sample_handle(SoundDriver);
            sample_queue_handle_initiated = 1;
        }
        sample_queue_handle_stopped = 1;
        AIL_end_sample(sample_queue_handle);
        AIL_set_sample_user_data(sample_queue_handle, 0, sfx0);
        AIL_set_sample_user_data(sample_queue_handle, 1, sfx1);
        AIL_set_sample_user_data(sample_queue_handle, 2, sfx2);
        AIL_set_sample_user_data(sample_queue_handle, 3, sfx3);
        AIL_set_sample_user_data(sample_queue_handle, 4, sfx4);
        AIL_set_sample_user_data(sample_queue_handle, 5, sfx5);
        AIL_set_sample_user_data(sample_queue_handle, 6, sfx6);
        AIL_set_sample_user_data(sample_queue_handle, 7, sfx7);
        current_sample_queue_count = 0;
        sample_queue_handle_stopped = 0;
        cb_sample_queue_callback(0);
    }
}

}

// Stops the sample queue and clears its list.
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
