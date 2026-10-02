#ifndef BFLIB_H
#define BFLIB_H

#include "common.h"

/*
 * Bullfrog library, runtime and sound library declarations. The C
 * functions' symbols do not depend on their parameter types, so those follow
 * how the game uses them; the C++ ones must mangle to the names in
 * config/functions.txt and config/symbols.txt.
 */

enum TbScreenMode { SCREEN_MODE_1 = 1, SCREEN_MODE_13 = 13 };

enum TbPaletteFadeFlag { Lb_PALETTE_FADE_OPEN = 0, Lb_PALETTE_FADE_CLOSED = 1 };

enum TbError { Lb_FAIL = -1, Lb_OK = 0, Lb_SUCCESS = 1 };

struct TbSprite {
    // RLE pixel data: an offset into the sprite data until LbSpriteSetup makes
    // it a pointer (LbSpriteReset undoes that).
    u32 data;
    u8 width;
    u8 height;
};

struct TbScreenModeInfo {
    u16 width;
    u16 height;
    u16 BitsPerPixel;
    s32 Available;
    // BIOS video mode number; 0x100 and up is a VESA mode.
    s32 VideoMode;
    char Desc[24];
};

/*
 * The library's display and mouse state (lbDisplay).
 */
struct TbDisplayStruct {
    // 0x0: Video memory the screen is shown from.
    u8 *physical_screen;
    // 0x4: Buffer the drawing routines write to.
    u8 *wscreen;
    // 0x8: 256x256 colour-blend table (ghost[a << 8 | b]) for translucent
    // drawing.
    u8 *ghost;
    // 0xc: Table of 64 colour remaps, one per fade_step, used for sprites drawn
    // with draw_flags 0x800.
    u8 *fade_table;
    // 0x10: Address in wscreen of the graphics window's top-left pixel.
    u8 *graphics_window;
    // 0x14: Pointer sprite, or 0.
    TbSprite *mouse_sprite;
    // 0x18: Width of the physical screen in pixels.
    s32 physical_screen_width;
    // 0x1c: Height of the physical screen in pixels.
    s32 physical_screen_height;
    // 0x20: Width (line pitch) of wscreen.
    s32 graphics_screen_width;
    // 0x24: Height of wscreen.
    s32 graphics_screen_height;
    // 0x28: Graphics window left edge.
    s32 graphics_window_x;
    // 0x2c: Graphics window top edge.
    s32 graphics_window_y;
    // 0x30: Graphics window width.
    s32 graphics_window_width;
    // 0x34: Graphics window height.
    s32 graphics_window_height;
    // 0x38: Left edge of the area the pointer is kept in.
    s32 mouse_window_x;
    // 0x3c: Top edge of the pointer area.
    s32 mouse_window_y;
    // 0x40: Width of the pointer area.
    s32 mouse_window_width;
    // 0x44: Height of the pointer area.
    s32 mouse_window_height;
    // 0x48: Pointer x when a button was last pressed.
    volatile s32 mouse_x;
    // 0x4c: Pointer y when a button was last pressed.
    volatile s32 mouse_y;
    // 0x50: Current pointer x, updated by the mouse handler.
    volatile s32 mmouse_x;
    // 0x54: Current pointer y.
    volatile s32 mmouse_y;
    // 0x58: Pointer x when a button was last released.
    volatile s32 rmouse_x;
    // 0x5c: Pointer y when a button was last released.
    volatile s32 rmouse_y;
    // 0x60: Drawing mode bits for the sprite, text and line routines.
    u16 draw_flags;
    // 0x62: BIOS video mode saved before the first mode change.
    u16 old_video_mode;
    // 0x64: Current screen mode.
    u16 screen_mode;
    // 0x66: Nonzero while a VESA mode is set.
    u8 vesa_is_set_up;
    // 0x67: Set by the mouse handler on a left press; cleared by the game when
    // handled.
    volatile u8 left_button;
    // 0x68: Set on a right press.
    volatile u8 right_button;
    // 0x69: Set on a middle press.
    volatile u8 middle_button;
    // 0x6a: 1 while the left button is held.
    volatile u8 mleft_button;
    // 0x6b: 1 while the right button is held.
    volatile u8 mright_button;
    // 0x6c: 1 while the middle button is held.
    volatile u8 mmiddle_button;
    // 0x6d: Set on a left release.
    volatile u8 rleft_button;
    // 0x6e: Set on a middle release.
    volatile u8 rmiddle_button;
    // 0x6f: Set on a right release.
    volatile u8 rright_button;
    // 0x70: Index of the fade_table remap in use (low 6 bits).
    u8 fade_step;
    // 0x71: Colour for one-colour drawing, such as text drawn with draw_flags
    // 0x40.
    u8 draw_colour;
    u8 unknown72[6];
};

struct TbLoadFiles {
    union {
        struct {
            char FName[0x1c];
            // Variable that receives the loaded buffer; 0 ends a list.
            u8 **Start;
            // Variable that receives the buffer's end, or 0.
            u8 **SEnd;
            // File length; for a name starting with '*', the size to allocate.
            s32 SLength;
            // Bit 0: allocate the buffer in low (DOS) memory.
            u16 Flags;
            /* A u16 (modify_data_load_filename reads it as one), split
               because wpp386 10.6 fails with "segment too large" when an
               initialiser reaches the last member of an anonymous struct
               in a union. */
            u8 Spare;
            u8 Spare_hi;
        };
        // Spare read as a whole: the lowest install_type that has the file on
        // the hard disk; above it modify_data_load_filename loads it from the CD.
        struct {
            u8 unknown_[0x2a];
            u16 install_level;
        };
    };
};

struct __iobuf {
    u8 unknown[0x1a];
};

struct __ctcl4TbNetworkVersion {
    char text[0x50];
    float version;
};

extern "C" {

extern TbDisplayStruct lbDisplay;
extern u8 lbKeyOn[];
extern u8 lbInkey;
extern TbSprite *lbFontPtr;
extern s8 lbInkeyToAscii[];
extern s8 lbInkeyToAsciiShift[];
extern TbScreenModeInfo lbScreenModeInfo[];
extern s32 lbSinTable[];
extern __iobuf __iob[];

int LbMemorySetup(void);
int LbMemoryReset(void);
void *LbMemoryAlloc(u32 size);
int LbMemoryFree(void *p);
void LbMemoryCopy(void *dst, void *src, u32 size);
void LbMemorySet(void *dst, u8 value, u32 size);
/* The C library's strcpy. */
void lib_strcpy(char *dst, char *src);
#pragma aux lib_strcpy "strcpy_"
int LbDataLoad(TbLoadFiles *files);
int LbDataFree(TbLoadFiles *files);
int LbDataFreeAll(TbLoadFiles *files);
int LbFileLengthRnc(char *path);
int LbFileLoadAt(char *path, void *buf);
int LbFileSaveAt(char *path, void *buf, u32 size);
int LbFileExists(char *path);
int LbFileOpen(char *name, u8 mode);
int LbFileClose(int handle);
int LbFileRead(int handle, void *buf, int len);
int LbFileSeek(int handle, int offset, u8 origin);
int LbFileWrite(int handle, void *buf, int len);
int LbSpriteSetup(TbSprite *start, TbSprite *end, void *data);
void LbSpriteDraw(int x, int y, TbSprite *sprite);
int LbMousePlace(void);
int LbMouseRemove(void);
int LbMouseChangeSprite(TbSprite *sprite);
int LbMouseChangeSpriteAndHotspot(TbSprite *sprite, s32 x, s32 y);
int LbMouseSetup(TbSprite *sprite);
int LbMouseReset(void);
int LbMouseSetPosition(s32 x, s32 y);
int LbGhostTableGenerate(u8 *palette, u8 intensity, char *name);
int LbScreenSetGraphicsWindow(s32 x, s32 y, s32 w, s32 h);
int LbScreenClear(u8 colour, ...);
int LbScreenReset(void);
int LbScreenUnlock(void);
int LbScreenSwap(void);
int LbIsActive(void);
s32 LbTimerClock(void);
/* The C library's memcpy. */
void lib_memcpy(void *a, void *b, int c);
#pragma aux lib_memcpy "memcpy_"
int LbPaletteGet(u8 *palette, ...);
int LbPaletteSet(u8 *palette, ...);
u32 GetSongHandle(void);
void StopAllSamples(void);
int LbDrawBox(s32 x, s32 y, s32 w, s32 h, u8 colour);
int LbTextSetWindow(s32 x, s32 y, s32 width, s32 height);
int LbTextDraw(s32 x, s32 y, char *text);
u32 LbSqrL(u32 n);
s32 LbTextStringWidth(char *text);
s32 LbTextStringHeight(char *text);
u16 LbArcTan(s32 x, s32 y);
int abs(int i);
void LbScreenCopyBox(const u8 *src, u8 *dst, s32 height);
u8 play_smk(char *name, u32 smk_flags, u32 flags);

// play_smk's flags (AnimTable.play_flags).
#define SMK_NO_SOUND 0x1
// Esc, Enter, Space and the left button do not stop the video.
#define SMK_NO_SKIP 0x2
#define SMK_DOUBLE_HEIGHT 0x4
// Double height with every other row left blank.
#define SMK_INTERLACE 0x8
#define SMK_DOUBLE_WIDTH 0x80
// Decode straight into VGA memory (320x200 mode only).
#define SMK_DIRECT_VGA 0x100
#define SMK_LOOP 0x400

// play_smk's result when the file opened: played to the end, or skipped.
#define SMK_PLAYED 1
#define SMK_SKIPPED 2

void reqspr_draw(int x, int y, TbSprite *sprite);

int atoi(char *p);
void flushall(void);
void memset(void *dst, int value, u32 size);
void memmove(void *dst, void *src, u32 size);
int sprintf(char *dest, char *format, ...);
void exit(int status);
void vprintf(char *format, char **arg);
void fflush(__iobuf *fp);
int tolower(int c);
int toupper(int c);

int printf(char *format, ...);
u32 clock(void);
void *malloc(u32 size);
#pragma aux malloc "_nmalloc_"
void free(void *p);
#pragma aux free "_nfree_"

struct DWORDREGS {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 esi;
    u32 edi;
    u32 cflag;
};

struct WORDREGS {
    u16 ax, _1;
    u16 bx, _2;
    u16 cx, _3;
    u16 dx, _4;
    u16 si, _5;
    u16 di, _6;
    u32 cflag;
};

struct BYTEREGS {
    u8 al, ah;
    u16 _1;
    u8 bl, bh;
    u16 _2;
    u8 cl, ch;
    u16 _3;
    u8 dl, dh;
    u16 _4;
};

union REGS {
    DWORDREGS x;
    WORDREGS w;
    BYTEREGS h;
};

struct SREGS {
    u16 es, cs, ss, ds, fs, gs;
};

int int386x(int intno, REGS *in_regs, REGS *out_regs, SREGS *segregs);
void segread(SREGS *seg);

int int386(int intno, REGS *in_regs, REGS *out_regs);

void AIL_stop_sequence(u32 handle, ...);
#pragma aux AIL_stop_sequence "_*"
void AIL_resume_sequence(u32 handle, ...);
#pragma aux AIL_resume_sequence "_*"
void AIL_end_sequence(u32 handle, ...);
#pragma aux AIL_end_sequence "_*"
}

/* A cache block's header; the block's data follows it. */
struct TbCacheUsedBlock {
    TbCacheUsedBlock *next;
    TbCacheUsedBlock *prev;
    // The owner's id, returned by ThrowOutOldest (sounds: sample + 1).
    u32 tag;
    // Nonzero: ThrowOutOldest skips the block.
    u32 locked;
    // Block size, this header included.
    u32 size;
};

struct TbCacheFreeBlock {
    TbCacheFreeBlock *next;
    u32 size;
};

struct TbCache {
    // Number of used blocks.
    s32 count;
    TbCacheFreeBlock *free;
    TbCacheUsedBlock *oldest;
    TbCacheUsedBlock *newest;
    u32 end;
    void *mem;
    u32 size;

    void UsedAndLocked(long &used, long &locked);
    int Initialise(void *mem, u32 size);
    void UnlockAll(void);
    TbCacheUsedBlock *Allocate(u32 size, u32 tag);
    void Free(TbCacheUsedBlock *block);
    void MakeNewest(TbCacheUsedBlock *block);
    u32 ThrowOutOldest(void);
};

extern TbCache cache;

/* One allocation made through _CMemory, in conventional memory if seg != 0. */
struct _CMemoryBlock {
    _CMemoryBlock *next;
    void *ptr;
    u32 size;
    u16 seg;
    u16 pad;
};

struct _CMemory {
    _CMemoryBlock blocks[100];
    _CMemoryBlock *free_list;
    _CMemoryBlock *used_list;

    _CMemory();
    ~_CMemory();
    void *Alloc(u32 size, u32 low);
};

extern _CMemory _lbMemory;

struct CDosHeapItem {
    u32 size;
    u8 *ptr;
    CDosHeapItem *next;
};

struct CDosHeapStats {
    u32 total;
    u32 count;

    void Init(void);
    void RecordAlloc(CDosHeapItem *item);
    void RecordFree(CDosHeapItem *item);
    u32 GetTotalAllocation(void);
    u32 GetAllocCount(void);
};

struct CDosHeap {
    CDosHeapItem items[200];
    CDosHeapItem *used_list;
    CDosHeapItem *free_list;
    CDosHeapStats stats;
    u32 reporting;
    u32 errors;

    CDosHeap();
    void *Alloc(u32 size);
    CDosHeapItem *GetNextRecord(void);
    void Error(char *fmt, ...);
    void Init(void *mem, u32 size);
    void Free(void *p);
    CDosHeapItem *RemoveAllocMatch(void *p);
    u32 ReleaseFreeBlock(CDosHeapItem *item);
    u32 MergeRecord(CDosHeapItem *item);
    void Repack(void);
    void Report(char *fmt, ...);
    void SetReporting(u32 on);
    void SetErrors(u32 on);
    void ReleaseRecord(CDosHeapItem *item);
};

extern CDosHeap _lbHeap;

void LbPackHeap(void);
s32 GetCDSpeed(char *path, void *buf, s32 size, u32 count);
void LbNetwork_LibraryVersion(__ctcl4TbNetworkVersion *version);
void LbNetwork_ChangeExchangeTimeout(u32 timeout);
TbError LbNetwork_Stop(void);
TbError LbNetwork_Exchange(void *buf);
TbError LbNetwork_ChangeExchangeBuffer(void *buf, u32 size);
TbError LbNetwork_Send(u32 player, void *buf, u32 len);
u32 LbNetwork_Receive(u32 player, void *buf, u32 len);

/* The library's service enum has no tag: any typedef name mangles as $$TE$$. */
typedef enum { NS_IPX = 1, NS_MODEM = 2, NS_SERIAL = 3 } TbNetworkService;

struct __tibcaGUID {
    u32 data[4];
};

struct __7308jnTbNetworkPlayerInfo {
    char name[0x20];
    s32 active;
};

extern __7308jnTbNetworkPlayerInfo player_info[];

struct TbNetworkCallbackData;
struct TbNetworkSessionNameEntry {
    union {
        u8 unknown0[9];
        struct {
            u8 flags;
            u32 id;
            u32 in_use;
        };
    };
    char name[0x20];

    TbNetworkSessionNameEntry();
};

struct TbNetworkPlayerNameEntry {
    u8 unknown0[0xd];
    char name[0x10];
};

typedef void(__stdcall *TbNetworkCallbackFunc)(TbNetworkCallbackData *, void *);

TbError LbNetwork_Init(TbNetworkService service, __tibcaGUID guid, u32 max_players, void *exchange_buf,
                       u32 exchange_size, __7308jnTbNetworkPlayerInfo *info, void *init_data);
TbError LbNetwork_Create(char *session, char *name, u32 *player, void *data);
TbError LbNetwork_Join(TbNetworkSessionNameEntry *session, char *name, u32 *player, void *data);
TbError LbNetwork_EnableNewPlayers(u32 allow);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *data);
TbError LbNetwork_EnumeratePlayers(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data);

#endif
