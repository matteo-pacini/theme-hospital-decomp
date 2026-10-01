#ifndef BFLIB_H
#define BFLIB_H

#include "common.h"

/*
 * Bullfrog library, runtime and sound library declarations. The C
 * functions' symbols do not depend on their parameter types, so those follow
 * how the game uses them; the C++ ones must mangle to the names in
 * config/functions.txt and config/symbols.txt.
 */

enum TbScreenMode {
    SCREEN_MODE_1 = 1,
    SCREEN_MODE_13 = 13
};

enum TbPaletteFadeFlag {
    Lb_PALETTE_FADE_OPEN = 0,
    Lb_PALETTE_FADE_CLOSED = 1
};

enum TbError {
    Lb_FAIL = -1,
    Lb_OK = 0,
    Lb_SUCCESS = 1
};

struct TbSprite {
    u32 data;
    u8 width;
    u8 height;
};

struct TbScreenModeInfo {
    u16 width;
    u16 height;
    u16 BitsPerPixel;
    s32 Available;
    s32 VideoMode;
    char Desc[24];
};

struct TbDisplayStruct {
    union {
        u32 unknown0;
        u8 *physical_screen;
    };
    u8 *wscreen;
    union {
        u8 unknown8[0x10];
        u8 *ghost;
        struct {
            u8 *ghost_;
            u8 *fade_table;
            u8 *graphics_window;
            TbSprite *mouse_sprite;
        };
    };

    s32 physical_screen_width;
    s32 physical_screen_height;
    s32 graphics_screen_width;
    s32 graphics_screen_height;
    s32 graphics_window_x;
    s32 graphics_window_y;
    s32 graphics_window_width;
    s32 graphics_window_height;
    s32 mouse_window_x;
    s32 mouse_window_y;
    s32 mouse_window_width;
    s32 mouse_window_height;
    volatile s32 mouse_x;
    volatile s32 mouse_y;
    volatile s32 mmouse_x;
    volatile s32 mmouse_y;
    volatile s32 rmouse_x;
    volatile s32 rmouse_y;
    u16 draw_flags;
    u16 old_video_mode;
    u16 screen_mode;
    u8 vesa_is_set_up;
    volatile u8 left_button;
    volatile u8 right_button;
    volatile u8 middle_button;
    volatile u8 mleft_button;
    volatile u8 mright_button;
    volatile u8 mmiddle_button;
    volatile u8 rleft_button;
    volatile u8 rmiddle_button;
    volatile u8 rright_button;
    u8 fade_step;
    u8 draw_colour;
    u8 unknown72[6];
};

struct TbLoadFiles {
    union {
        struct {
            char FName[0x1c];
            u8 **Start;
            u8 **SEnd;
            s32 SLength;
            u16 Flags;
            /* A u16 (modify_data_load_filename reads it as one), split
               because wpp386 10.6 fails with "segment too large" when an
               initialiser reaches the last member of an anonymous struct
               in a union. */
            u8 Spare;
            u8 Spare_hi;
        };
        u8 unknown[0x2c];
    };
};

struct __iobuf {
    u8 unknown[0x1a];
};

struct __1rdtuTbModemDev {
    u8 unknown0[0x1c];
    u8 unknown1c;
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
void fn_000bbff0(char *dst, char *src);
#pragma aux fn_000bbff0 "strcpy_"
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
void fn_000c21e6(void *a, void *b, int c);
#pragma aux fn_000c21e6 "memcpy_"
int LbPaletteGet(u8 *palette, ...);
int LbPaletteSet(u8 *palette, ...);
u32 GetSongHandle(void);
void StopAllSamples(void);
int LbDrawBox(s32 x, s32 y, s32 w, s32 h, u8 colour);
int LbTextSetWindow(s32 x, s32 y, s32 w, s32 h);
int LbTextDraw(s32 x, s32 y, char *text);
u32 LbSqrL(u32 n);
s32 LbTextStringWidth(char *text);
s32 LbTextStringHeight(char *text);
u16 LbArcTan(s32 x, s32 y);
int fn_000cb2db(int n);
void LbScreenCopyBox(const u8 *src, u8 *dst, s32 height);
u8 play_smk(char *path, u32 a, u32 b);
void reqspr_draw(int x, int y, TbSprite *sprite);

int atoi(char *text);
void flushall(void);
void memset(void *dst, int value, u32 size);
void memmove(void *dst, void *src, u32 size);
int sprintf(char *buf, char *fmt, ...);
void exit(int code);
void vprintf(char *fmt, char **args);
void fflush(__iobuf *fp);
int tolower(int c);
int toupper(int c);

int printf(char *fmt, ...);
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

int int386x(int intno, REGS *in, REGS *out, SREGS *seg);
void segread(SREGS *seg);

int int386(int intno, REGS *in, REGS *out);

void AIL_stop_sequence(u32 handle, ...);
#pragma aux AIL_stop_sequence "_*"
void AIL_resume_sequence(u32 handle, ...);
#pragma aux AIL_resume_sequence "_*"
void AIL_end_sequence(u32 handle, ...);
#pragma aux AIL_end_sequence "_*"

}

extern __1rdtuTbModemDev modem_dev;

/* A cache block's header; the block's data follows it. */
struct TbCacheUsedBlock {
    TbCacheUsedBlock *next;
    TbCacheUsedBlock *prev;
    u32 tag;
    u32 locked;
    u32 size;
};

struct TbCacheFreeBlock {
    TbCacheFreeBlock *next;
    u32 size;
};

struct TbCache {
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
TbError LbNetwork_Send(u32 port, void *buf, u32 len);
u32 LbNetwork_Receive(u32 port, void *buf, u32 len);

/* The library's service enum has no tag: any typedef name mangles as $$TE$$. */
typedef enum {
    NS_IPX = 1,
    NS_MODEM = 2,
    NS_SERIAL = 3
} TbNetworkService;

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

typedef void (__stdcall *TbNetworkCallbackFunc)(TbNetworkCallbackData *, void *);

TbError LbNetwork_Init(TbNetworkService service, __tibcaGUID guid, u32 max_players, void *a, u32 b,
                       __7308jnTbNetworkPlayerInfo *info, void *init_data);
TbError LbNetwork_Create(char *session, char *name, u32 *player, void *data);
TbError LbNetwork_Join(TbNetworkSessionNameEntry *session, char *name, u32 *player, void *data);
TbError LbNetwork_EnableNewPlayers(u32 enable);
TbError LbNetwork_EnumerateSessions(TbNetworkCallbackFunc callback, void *data);
TbError LbNetwork_EnumeratePlayers(TbNetworkSessionNameEntry *session, TbNetworkCallbackFunc callback, void *data);

#endif
