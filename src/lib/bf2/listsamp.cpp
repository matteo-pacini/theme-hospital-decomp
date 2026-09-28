#include "snd.h"

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
