#include "ail.h"

#define AUDIO_TYPE_MIDI 1
#define AUDIO_TYPE_DIGITAL 2

#define QSTAT_DONE 2
#define QSTAT_PLAYING 3

typedef struct _AUDIO_TYPE {
    void *data;
    S32 type;
    void *handle;
    S32 status;
    struct _AUDIO_TYPE *next;
} AUDIO_TYPE;

extern HDIGDRIVER dig;
extern HMDIDRIVER mdi;
extern HAUDIO audios;

extern char D_001270e8[];
extern char D_001270f8[];
extern char D_00127108[];
extern char D_00127114[];
extern char D_0012711c[];
extern char D_00127124[];
extern char D_0012712c[];
extern char D_00127140[];
extern char D_0012715c[];

void fn_000e6d00(void);
void fn_000e6d40(void);

// FUNCTION: 0x000e6e90
HAUDIO __cdecl AIL_API_quick_load(char *filename)
{
    HAUDIO audio;

    audio = AIL_mem_alloc_lock(sizeof(AUDIO_TYPE));
    if (audio == NULL) {
        strcpy(AIL_error, D_001270e8);
        return NULL;
    }
    memset(audio, 0, sizeof(AUDIO_TYPE));
    audio->data = AIL_file_read(filename, NULL);
    if (audio->data == NULL) {
        AIL_quick_unload(audio);
        strcpy(AIL_error, D_001270f8);
        return NULL;
    }
    if (!strnicmp(audio->data, D_00127108, 8)) {
        audio->type = AUDIO_TYPE_DIGITAL;
    } else if (!strnicmp((char *)audio->data + 8, D_00127114, 4)) {
        audio->type = AUDIO_TYPE_DIGITAL;
    } else if (!strnicmp(audio->data, D_0012711c, 4)
               && !strnicmp((char *)audio->data + 8, D_00127124, 4)) {
        audio->type = AUDIO_TYPE_MIDI;
    } else {
        AIL_quick_unload(audio);
        strcpy(AIL_error, D_0012712c);
        return NULL;
    }
    if (audio->type == AUDIO_TYPE_MIDI && mdi == NULL) {
        AIL_quick_unload(audio);
        strcpy(AIL_error, D_00127140);
        return NULL;
    }
    if (audio->type == AUDIO_TYPE_DIGITAL && dig == NULL) {
        AIL_quick_unload(audio);
        strcpy(AIL_error, D_0012715c);
        return NULL;
    }
    audio->status = QSTAT_DONE;
    audio->next = audios;
    return audio;
}

// FUNCTION: 0x000e7120
void __cdecl AIL_API_quick_unload(HAUDIO audio)
{
    HAUDIO a;

    if (audio == NULL) {
        return;
    }
    AIL_lock();
    if (audio->handle != NULL) {
        switch (audio->type) {
        case AUDIO_TYPE_MIDI:
            AIL_end_sequence(audio->handle);
            break;
        case AUDIO_TYPE_DIGITAL:
            AIL_end_sample(audio->handle);
            break;
        }
        audio->handle = NULL;
    }
    AIL_unlock();
    if (audio->data != NULL) {
        AIL_mem_free_lock(audio->data);
        audio->data = NULL;
    }
    if (audio == audios) {
        audios = audio->next;
    } else {
        for (a = audios; a != NULL; a = a->next) {
            if (a->next == audio) {
                a->next = audio->next;
                break;
            }
        }
    }
    AIL_mem_free_lock(audio);
}

// FUNCTION: 0x000e71c0
S32 __cdecl AIL_API_quick_play(HAUDIO audio, U32 loop_count)
{
    if (audio == NULL) {
        return 0;
    }
    if (audio->handle != NULL) {
        AIL_quick_halt(audio);
    }
    switch (audio->type) {
    case AUDIO_TYPE_MIDI:
        audio->handle = AIL_allocate_sequence_handle(mdi);
        if (audio->handle == NULL) {
            return 0;
        }
        if (AIL_init_sequence(audio->handle, audio->data, 0) != 1) {
            AIL_release_sequence_handle(audio->handle);
            return 0;
        }
        AIL_set_sequence_user_data(audio->handle, 0, (S32)audio);
        AIL_register_sequence_callback(audio->handle, (AILSEQUENCECB)fn_000e6d00);
        AIL_set_sequence_loop_count(audio->handle, loop_count);
        audio->status = QSTAT_PLAYING;
        AIL_start_sequence(audio->handle);
        return 1;
    case AUDIO_TYPE_DIGITAL:
        audio->handle = AIL_allocate_sample_handle(dig);
        if (audio->handle == NULL) {
            return 0;
        }
        AIL_init_sample(audio->handle);
        if (!AIL_set_sample_file(audio->handle, audio->data, -1)) {
            AIL_release_sample_handle(audio->handle);
            return 0;
        }
        AIL_set_sample_user_data(audio->handle, 0, (S32)audio);
        AIL_register_EOS_callback(audio->handle, (AILSAMPLECB)fn_000e6d40);
        AIL_set_sample_loop_count(audio->handle, loop_count);
        audio->status = QSTAT_PLAYING;
        AIL_start_sample(audio->handle);
        return 1;
    }
    return 0;
}
