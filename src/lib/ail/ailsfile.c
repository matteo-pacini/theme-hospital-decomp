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


/* VOC and WAV end-of-sample callbacks: __pascal (callee pops). */
void AIL_VOC_EOS(HSAMPLE S);
#pragma aux (__pascal) AIL_VOC_EOS "*";
void AIL_WAV_EOS(HSAMPLE S);
#pragma aux (__pascal) AIL_WAV_EOS "*";
void AILSFILE_end(void);

// Locks this module's code against paging, once.
// FUNCTION: 0x000e16e0
void AILSFILE_start(void)
{
    if (!locked) {
        AIL_vmm_lock_range(AILSFILE_start, AILSFILE_end);
        locked = 1;
    }
}

// Returns the 24-bit length field of the VOC block header at `block`.
// FUNCTION: 0x000e1710
U32 __cdecl AIL_VOC_block_len(void *block)
{
    return *(U32 *)block >> 8;
}

// Ends a VOC or WAV file sample: calls its end-of-file callback, releases the
// handle if AIL_allocate_file_sample allocated it, and marks the file finished
// (SSD_RELEASE = -1).
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

// Walks sample `S`'s VOC file from its current block to the next voice block
// (type 1, type 8 with the data block after it, or type 9) and sets the
// sample's address, rate and format from it, starting it if `play_flag` is
// set. Follows repeat loops (types 6 and 7; 0xFFFF repeats forever). With a
// marker requested, only voice blocks between the marker block of that number
// and the next marker are used. At the terminator block it calls
// AIL_VOC_terminate instead.
// FUNCTION: 0x000e1780
void __cdecl AIL_process_VOC_block(HSAMPLE S, S32 play_flag)
{
    S32 voice_block;
    void *block;

    voice_block = 0;
    do {
        block = (void *)S->system_data[VOC_BLK_PTR];
        switch (((BLOCK *)block)->block_ID) {
        case 0:
            AIL_VOC_terminate(S);
            return;
        case 1:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            AIL_set_sample_address(S, ((VOC_BLOCK1 *)block)->data, AIL_VOC_block_len(block) - 2);
            AIL_set_sample_playback_rate(S, 1000000UL / (256 - ((VOC_BLOCK1 *)block)->time_constant));
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
            if ((S32)((MARKER_BLOCK *)block)->marker == S->system_data[VOC_MARKER]) {
                S->system_data[VOC_MARKER_FOUND] = 1;
            } else {
                S->system_data[VOC_MARKER_FOUND] = 0;
            }
            break;
        case 6:
            S->system_data[VOC_REP_BLK] = (S32)block;
            S->system_data[VOC_N_REPS] = ((REPEAT_BLOCK *)block)->repeat_count;
            break;
        case 7:
            if (S->system_data[VOC_N_REPS] != 0xffff) {
                if (S->system_data[VOC_N_REPS]-- == 0) {
                    break;
                }
            }
            block = (void *)S->system_data[VOC_REP_BLK];
            break;
        case 8:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            if (((EXTENDED_BLOCK *)block)->voice_mode) {
                AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
                AIL_set_sample_playback_rate(S, 128000000UL / (65536UL - ((EXTENDED_BLOCK *)block)->time_constant));
            } else {
                AIL_set_sample_type(S, DIG_F_MONO_8, 0);
                AIL_set_sample_playback_rate(S, 256000000UL / (65536UL - ((EXTENDED_BLOCK *)block)->time_constant));
            }
            block = (U8 *)block + AIL_VOC_block_len(block) + 4;
            AIL_set_sample_address(S, ((VOC_BLOCK1 *)block)->data, AIL_VOC_block_len(block) - 2);
            if (play_flag) {
                AIL_start_sample(S);
            }
            voice_block = 1;
            break;
        case 9:
            if (!S->system_data[VOC_MARKER_FOUND]) {
                break;
            }
            AIL_set_sample_address(S, ((BLOCK9 *)block)->data, AIL_VOC_block_len(block) - 12);
            AIL_set_sample_playback_rate(S, ((BLOCK9 *)block)->sample_rate);
            if (((BLOCK9 *)block)->channels == 1 && ((BLOCK9 *)block)->format == 0) {
                AIL_set_sample_type(S, DIG_F_MONO_8, 0);
            } else if (((BLOCK9 *)block)->channels == 2 && ((BLOCK9 *)block)->format == 0) {
                AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
            } else if (((BLOCK9 *)block)->channels == 1 && ((BLOCK9 *)block)->format == 4) {
                AIL_set_sample_type(S, DIG_F_MONO_16, DIG_PCM_SIGN);
            } else if (((BLOCK9 *)block)->channels == 2 && ((BLOCK9 *)block)->format == 4) {
                AIL_set_sample_type(S, DIG_F_STEREO_16, DIG_PCM_SIGN);
            }
            if (play_flag) {
                AIL_start_sample(S);
            }
            voice_block = 1;
            break;
        }
        S->system_data[VOC_BLK_PTR] = (S32)((U8 *)block + AIL_VOC_block_len(block) + 4);
    } while (!voice_block);
}

// End-of-sample callback for VOC files: moves on to and starts the next voice
// block.
// FUNCTION: 0xe1a00
void AIL_VOC_EOS(HSAMPLE S)
{
    AIL_process_VOC_block(S, 1);
}

// Sets sample `S`'s format, rate and address from the "fmt " and "data" chunks
// of the RIFF WAVE file at `image`. Only the channel count and sample size pick
// the type (format_tag is not checked); combinations other than 1 or 2 channels
// of 8 or 16 bits leave it unchanged.
// FUNCTION: 0x000e1a20
void __cdecl AIL_process_WAV_image(void *image, HSAMPLE S)
{
    FMT *fmt;
    DATA *data;

    fmt = (FMT *)((U8 *)image + 12);
    while (strnicmp(fmt->FMT_string, "fmt ", 4)) {
        fmt = (FMT *)((U8 *)fmt + fmt->chunk_size + 8 + (fmt->chunk_size & 1));
    }
    if (fmt->channels == 1 && fmt->bits_per_sample == 8) {
        AIL_set_sample_type(S, DIG_F_MONO_8, 0);
    } else if (fmt->channels == 2 && fmt->bits_per_sample == 8) {
        AIL_set_sample_type(S, DIG_F_STEREO_8, 0);
    } else if (fmt->channels == 1 && fmt->bits_per_sample == 16) {
        AIL_set_sample_type(S, DIG_F_MONO_16, DIG_PCM_SIGN);
    } else if (fmt->channels == 2 && fmt->bits_per_sample == 16) {
        AIL_set_sample_type(S, DIG_F_STEREO_16, DIG_PCM_SIGN);
    }
    AIL_set_sample_playback_rate(S, fmt->sample_rate);
    data = (DATA *)((U8 *)image + 12);
    while (strnicmp(data->DATA_string, "data", 4)) {
        data = (DATA *)((U8 *)data + data->chunk_size + 8 + (data->chunk_size & 1));
    }
    AIL_set_sample_address(S, data->data, data->chunk_size);
}

// End-of-sample callback for WAV files: same as AIL_VOC_terminate.
// FUNCTION: 0xe1b60
void AIL_WAV_EOS(HSAMPLE S)
{
    if (S->system_data[EOD_CALLBACK] != NULL) {
        ((AILSAMPLECB)S->system_data[EOD_CALLBACK])(S);
    }
    if (S->system_data[SSD_RELEASE] > 0) {
        AIL_release_sample_handle(S);
    }
    S->system_data[SSD_RELEASE] = -1;
}

// Allocates a sample on `dig` and sets it up from the VOC or WAV file image
// `file_image`; for a VOC, `block` selects the data after the marker of that
// number (-1 plays from the start). The handle is released when the file ends.
// Returns it, or NULL: for a NULL image, if no handle is free, or with AIL_error
// set if the type is unknown or the file has no data to play.
// FUNCTION: 0x000e1ba0
HSAMPLE __cdecl AIL_API_allocate_file_sample(HDIGDRIVER dig, void *file_image, S32 block)
{
    HSAMPLE S;
    U32 type;

    AILSFILE_start();
    if (file_image == NULL) {
        return NULL;
    }
    if (!strnicmp(file_image, "Creative", 8)) {
        type = FILE_VOC;
    } else if (!strnicmp((char *)file_image + 8, "WAVE", 4)) {
        type = FILE_WAV;
    } else {
        strcpy(AIL_error, "Unrecognized digital audio file type\n");
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
        AIL_register_EOS_callback(S, (AILSAMPLECB)AIL_VOC_EOS);
        AIL_process_VOC_block(S, 0);
        break;
    case FILE_WAV:
        S->system_data[SSD_RELEASE] = 1;
        AIL_register_EOS_callback(S, (AILSAMPLECB)AIL_WAV_EOS);
        AIL_process_WAV_image(file_image, S);
        break;
    }
    if (S->system_data[SSD_RELEASE] == -1) {
        strcpy(AIL_error, "Invalid or missing data block\n");
        return NULL;
    }
    return S;
}

// Sets up sample `S` from the VOC or WAV file image `file_image`, with `block`
// as in AIL_allocate_file_sample. Unlike that, it installs no end-of-sample
// callback, so a VOC plays only its first voice block, and `S` is never
// released. Returns 1, or 0: for a NULL image, or with AIL_error set for an
// unrecognised image or no data to play.
// FUNCTION: 0x000e1d30
S32 __cdecl AIL_API_set_sample_file(HSAMPLE S, void *file_image, S32 block)
{
    U32 type;

    if (file_image == NULL) {
        return 0;
    }
    if (!strnicmp(file_image, "Creative", 8)) {
        type = FILE_VOC;
    } else if (!strnicmp((char *)file_image + 8, "WAVE", 4)) {
        type = FILE_WAV;
    } else {
        strcpy(AIL_error, "Unrecognized digital audio file type\n");
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
        strcpy(AIL_error, "Invalid or missing data block\n");
        return 0;
    }
    return 1;
}

// Sets the callback run when the file behind sample `S` ends; returns the
// previous one (NULL for a NULL `S`).
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

// Unlocks this module's code. Its address also marks the end of the locked code
// range.
// FUNCTION: 0xe1ea0
void AILSFILE_end(void)
{
    if (locked) {
        AIL_vmm_unlock_range(AILSFILE_start, AILSFILE_end);
        locked = 0;
    }
}
