#include "ail.h"

#define EOD_CALLBACK 0
#define VOC_BLK_PTR 1
#define VOC_REP_BLK 2
#define VOC_N_REPS 3
#define VOC_MARKER 4
#define VOC_MARKER_FOUND 5
#define SSD_RELEASE 6

#define FILE_VOC 0
#define FILE_WAV 1

#pragma pack(1)
typedef struct {
    U8 block_ID;
    U8 block_len[3];
} BLOCK;

typedef struct {
    BLOCK hdr;
    U8 time_constant;
    U8 pack_method;
    U8 data[1];
} VOC_BLOCK1;

typedef struct {
    BLOCK hdr;
    S16 marker;
} MARKER_BLOCK;

typedef struct {
    BLOCK hdr;
    U16 repeat_count;
} REPEAT_BLOCK;

typedef struct {
    BLOCK hdr;
    U16 time_constant;
    U8 pack_method;
    U8 voice_mode;
} EXTENDED_BLOCK;

typedef struct {
    BLOCK hdr;
    U32 sample_rate;
    U8 bits_per_sample;
    U8 channels;
    U16 format;
    U8 reserved[4];
    U8 data[1];
} BLOCK9;

typedef struct {
    char ID[20];
    U16 data_offset;
    U16 version;
    U16 ID_code;
} VOC;

typedef struct {
    char FMT_string[4];
    U32 chunk_size;
    S16 format_tag;
    S16 channels;
    S32 sample_rate;
    S32 average_data_rate;
    S16 alignment;
    S16 bits_per_sample;
} FMT;

typedef struct {
    char DATA_string[4];
    U32 chunk_size;
    U8 data[1];
} DATA;
#pragma pack()

static S32 locked = 0;

extern char D_00126db8[];
extern char D_00126dc0[];
extern char D_00126dc8[];
extern char D_00126dd4[];
extern char D_00126ddc[];
extern char D_00126e04[];
extern char D_00126e24[];
extern char D_00126e4c[];

/* VOC end-of-sample callback (demo: AIL_VOC_EOS): __pascal, address-named. */
void fn_000e1a00(HSAMPLE S);
#pragma aux (__pascal) fn_000e1a00 "*_";
/* WAV end-of-sample callback: __pascal (callee pops), address-named. */
void fn_000e1b60(HSAMPLE S);
#pragma aux (__pascal) fn_000e1b60 "*_";
void AILSFILE_end(void);

// FUNCTION: 0x000e16e0
void AILSFILE_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILSFILE_start, AILSFILE_end);
        locked = 1;
    }
}

// FUNCTION: 0x000e1710
U32 __cdecl AIL_VOC_block_len(void *block)
{
    return *(U32 *)block >> 8;
}

// FUNCTION: 0x000e1720
void __cdecl AIL_VOC_terminate(HSAMPLE S)
{
    if (S->system_data[EOD_CALLBACK] != NULL) {
        ((AILSAMPLECB)S->system_data[EOD_CALLBACK])(S);
    }
    if (S->system_data[SSD_RELEASE] > 0) {
        AIL_release_sample_handle(S);
    }
    S->system_data[SSD_RELEASE] = -1;
}

// FUNCTION: 0x000e1780
void __cdecl AIL_process_VOC_block(HSAMPLE S, S32 play_flag)
{
    S32 voice_block;
    void *b;

    voice_block = 0;
    do {
        b = (void *)S->system_data[VOC_BLK_PTR];
        switch (((BLOCK *)b)->block_ID) {
        case 0:
            AIL_VOC_terminate(S);
            return;
        case 1:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            AIL_set_sample_address(S, ((VOC_BLOCK1 *)b)->data, AIL_VOC_block_len(b) - 2);
            AIL_set_sample_playback_rate(S, 1000000UL / (256 - ((VOC_BLOCK1 *)b)->time_constant));
            AIL_set_sample_type(S, DIG_F_MONO_8, 0);
            if (play_flag) {
                AIL_start_sample(S);
            }
            voice_block = 1;
            break;
        case 4:
            if (S->system_data[VOC_MARKER] == -1) {
                break;
            }
            if ((S32)((MARKER_BLOCK *)b)->marker == S->system_data[VOC_MARKER]) {
                S->system_data[VOC_MARKER_FOUND] = 1;
            } else {
                S->system_data[VOC_MARKER_FOUND] = 0;
            }
            break;
        case 6:
            S->system_data[VOC_REP_BLK] = (S32)b;
            S->system_data[VOC_N_REPS] = ((REPEAT_BLOCK *)b)->repeat_count;
            break;
        case 7:
            if (S->system_data[VOC_N_REPS] != 0xffff) {
                if (S->system_data[VOC_N_REPS]-- == 0) {
                    break;
                }
            }
            b = (void *)S->system_data[VOC_REP_BLK];
            break;
        case 8:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            if (((EXTENDED_BLOCK *)b)->voice_mode) {
                AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
                AIL_set_sample_playback_rate(S, 128000000UL / (65536UL - ((EXTENDED_BLOCK *)b)->time_constant));
            } else {
                AIL_set_sample_type(S, DIG_F_MONO_8, 0);
                AIL_set_sample_playback_rate(S, 256000000UL / (65536UL - ((EXTENDED_BLOCK *)b)->time_constant));
            }
            b = (U8 *)b + AIL_VOC_block_len(b) + 4;
            AIL_set_sample_address(S, ((VOC_BLOCK1 *)b)->data, AIL_VOC_block_len(b) - 2);
            if (play_flag) {
                AIL_start_sample(S);
            }
            voice_block = 1;
            break;
        case 9:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            AIL_set_sample_address(S, ((BLOCK9 *)b)->data, AIL_VOC_block_len(b) - 12);
            AIL_set_sample_playback_rate(S, ((BLOCK9 *)b)->sample_rate);
            if (((BLOCK9 *)b)->channels == 1 && ((BLOCK9 *)b)->format == 0) {
                AIL_set_sample_type(S, DIG_F_MONO_8, 0);
            } else if (((BLOCK9 *)b)->channels == 2 && ((BLOCK9 *)b)->format == 0) {
                AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
            } else if (((BLOCK9 *)b)->channels == 1 && ((BLOCK9 *)b)->format == 4) {
                AIL_set_sample_type(S, DIG_F_MONO_16, DIG_PCM_SIGN);
            } else if (((BLOCK9 *)b)->channels == 2 && ((BLOCK9 *)b)->format == 4) {
                AIL_set_sample_type(S, DIG_F_STEREO_16, DIG_PCM_SIGN);
            }
            if (play_flag) {
                AIL_start_sample(S);
            }
            voice_block = 1;
            break;
        }
        S->system_data[VOC_BLK_PTR] = (S32)((U8 *)b + AIL_VOC_block_len(b) + 4);
    } while (!voice_block);
}

// FUNCTION: 0xe1a00
void fn_000e1a00(HSAMPLE S)
{
    AIL_process_VOC_block(S, 1);
}

// FUNCTION: 0x000e1a20
void __cdecl AIL_process_WAV_image(void *image, HSAMPLE S)
{
    FMT *f;
    DATA *d;

    f = (FMT *)((U8 *)image + 12);
    while (strnicmp(f->FMT_string, D_00126db8, 4)) {
        f = (FMT *)((U8 *)f + f->chunk_size + 8 + (f->chunk_size & 1));
    }
    if (f->channels == 1 && f->bits_per_sample == 8) {
        AIL_set_sample_type(S, DIG_F_MONO_8, 0);
    } else if (f->channels == 2 && f->bits_per_sample == 8) {
        AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
    } else if (f->channels == 1 && f->bits_per_sample == 16) {
        AIL_set_sample_type(S, DIG_F_MONO_16, DIG_PCM_SIGN);
    } else if (f->channels == 2 && f->bits_per_sample == 16) {
        AIL_set_sample_type(S, DIG_F_STEREO_16, DIG_PCM_SIGN);
    }
    AIL_set_sample_playback_rate(S, f->sample_rate);
    d = (DATA *)((U8 *)image + 12);
    while (strnicmp(d->DATA_string, D_00126dc0, 4)) {
        d = (DATA *)((U8 *)d + d->chunk_size + 8 + (d->chunk_size & 1));
    }
    AIL_set_sample_address(S, d->data, d->chunk_size);
}

// FUNCTION: 0xe1b60
void fn_000e1b60(HSAMPLE S)
{
    if (S->system_data[EOD_CALLBACK] != NULL) {
        ((AILSAMPLECB)S->system_data[EOD_CALLBACK])(S);
    }
    if (S->system_data[SSD_RELEASE] > 0) {
        AIL_release_sample_handle(S);
    }
    S->system_data[SSD_RELEASE] = -1;
}

// FUNCTION: 0x000e1ba0
HSAMPLE __cdecl AIL_API_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block)
{
    HSAMPLE S;
    U32 type;

    AILSFILE_start();
    if (file_image == NULL) {
        return NULL;
    }
    if (!strnicmp(file_image, D_00126dc8, 8)) {
        type = FILE_VOC;
    } else if (!strnicmp((char *)file_image + 8, D_00126dd4, 4)) {
        type = FILE_WAV;
    } else {
        strcpy(AIL_error, D_00126ddc);
        return NULL;
    }
    S = AIL_allocate_sample_handle(dig);
    if (S == NULL) {
        return NULL;
    }
    AIL_init_sample(S);
    S->system_data[EOD_CALLBACK] = NULL;
    switch (type) {
    case FILE_VOC:
        S->system_data[VOC_BLK_PTR] = (S32)((U8 *)file_image + ((VOC *)file_image)->data_offset);
        S->system_data[VOC_MARKER] = block;
        S->system_data[VOC_MARKER_FOUND] = (block == -1);
        S->system_data[SSD_RELEASE] = 1;
        AIL_register_EOS_callback(S, (AILSAMPLECB)fn_000e1a00);
        AIL_process_VOC_block(S, 0);
        break;
    case FILE_WAV:
        S->system_data[SSD_RELEASE] = 1;
        AIL_register_EOS_callback(S, (AILSAMPLECB)fn_000e1b60);
        AIL_process_WAV_image(file_image, S);
        break;
    }
    if (S->system_data[SSD_RELEASE] == -1) {
        strcpy(AIL_error, D_00126e04);
        return NULL;
    }
    return S;
}

// FUNCTION: 0x000e1d30
S32 __cdecl AIL_API_set_sample_file(HSAMPLE S, void *file_image, S32 block)
{
    U32 type;

    if (file_image == NULL) {
        return 0;
    }
    if (!strnicmp(file_image, D_00126dc8, 8)) {
        type = FILE_VOC;
    } else if (!strnicmp((char *)file_image + 8, D_00126dd4, 4)) {
        type = FILE_WAV;
    } else {
        strcpy(AIL_error, D_00126e24);
        return 0;
    }
    switch (type) {
    case FILE_VOC:
        S->system_data[VOC_BLK_PTR] = (S32)((U8 *)file_image + ((VOC *)file_image)->data_offset);
        S->system_data[VOC_MARKER] = block;
        S->system_data[VOC_MARKER_FOUND] = (block == -1);
        S->system_data[SSD_RELEASE] = 0;
        AIL_process_VOC_block(S, 0);
        break;
    case FILE_WAV:
        S->system_data[SSD_RELEASE] = 0;
        AIL_process_WAV_image(file_image, S);
        break;
    }
    if (S->system_data[SSD_RELEASE] == -1) {
        strcpy(AIL_error, D_00126e4c);
        return 0;
    }
    return 1;
}

// FUNCTION: 0xe1e80
AILSAMPLECB __cdecl AIL_API_register_EOF_callback(HSAMPLE S, AILSAMPLECB EOFILE)
{
    AILSAMPLECB old;

    if (S == NULL) {
        return NULL;
    }
    old = (AILSAMPLECB)S->system_data[EOD_CALLBACK];
    S->system_data[EOD_CALLBACK] = (S32)EOFILE;
    return old;
}

// FUNCTION: 0xe1ea0
void AILSFILE_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILSFILE_start, AILSFILE_end);
        locked = 0;
    }
}
