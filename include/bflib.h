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
    u8 unknown[34];
};

struct TbDisplayStruct {
    u32 unknown0;
    u8 *unknown4;
    union {
        u8 unknown8[0x10];
        u8 *ghost;
    };

    s32 unknown18;
    s32 unknown1c;
};

struct TbLoadFiles {
    u8 unknown[0x2c];
};

struct __iobuf {
    u8 unknown[0x1a];
};

struct __1rdtuTbModemDev {
    u8 unknown0[0x1c];
    u8 unknown1c;
};

struct __ctcl4TbNetworkVersion;

extern "C" {

extern TbDisplayStruct lbDisplay;
extern u8 lbKeyOn[];
extern u8 lbInkey;
extern TbSprite *lbFontPtr;
extern s8 lbInkeyToAscii[];
extern s8 lbInkeyToAsciiShift[];
extern TbScreenModeInfo lbScreenModeInfo[];
extern __iobuf __iob[];

int LbMemorySetup(void);
void LbMemoryReset(void);
void *LbMemoryAlloc(u32 size);
void LbMemoryFree(void *p);
void LbMemoryCopy(void *dst, void *src, u32 size);
void LbMemorySet(void *dst, int value, u32 size);
void LbDataLoadSetModifyFilenameFunction(char *dst, char *src);
int LbDataLoadAll(TbLoadFiles *files);
void LbDataFree(TbLoadFiles *files);
void LbDataFreeAll(TbLoadFiles *files);
int LbFileLengthRnc(char *path);
int LbFileLoadAt(char *path, void *buf);
void LbFileSaveAt(char *path, void *buf, u32 size);
int LbFileExists(char *path);
int LbFileOpen(char *name, int mode);
void LbFileClose(int handle);
int LbFileRead(int handle, void *buf, int len);
int LbFileSeek(int handle, int offset, int origin);
void LbFileWrite(int handle, void *buf, int len);
void LbSpriteSetup(TbSprite *start, TbSprite *end, void *data);
void LbSpriteDraw(int x, int y, TbSprite *sprite);
void LbMousePlace(void);
void LbMouseRemove(void);
void LbMouseChangeSprite(int sprite);
void LbMouseChangeSpriteAndHotspot(TbSprite *sprite, int x, int y);
int LbMouseSetup(TbSprite *sprite);
void LbMouseReset(void);
void LbMouseSetPosition(s32 x, s32 y);
void LbGhostTableGenerate(u8 *palette, int intensity, char *name);
void LbScreenSetGraphicsWindow(u32 x, u32 y, u32 w, u32 h);
void LbScreenClear(u8 colour, ...);
void LbScreenReset(void);
void LbIsActive(void);
void LbScreenSwap(void);
int LbDate(void);
u32 LbTimerClock(void);
void LbDrawBoxCoords(void *a, void *b, int c);
void LbPaletteGet(u8 *palette, ...);
void LbPaletteSet(u8 *palette, ...);
u32 GetSongHandle(void);
void StopAllSamples(void);
void LbDrawBox(u32 x, u32 y, u32 w, u32 h, u8 colour);
void LbTextSetWindow(u32 x, u32 y, u32 w, u32 h);
void LbTextDraw(int x, int y, char *text);
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

void LbPackHeap(void);
s32 GetCDSpeed(char *path, void *buf, s32 size, u32 count);
void LbNetwork_LibraryVersion(__ctcl4TbNetworkVersion *version);
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
    u8 unknown0[9];
    char name[0x20];
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
