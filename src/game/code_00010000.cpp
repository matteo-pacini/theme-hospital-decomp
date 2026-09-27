#include "common.h"

enum ScreenMode {
    SCREEN_MODE_1 = 1,
    SCREEN_MODE_13 = 13
};

struct ScreenModeInfo {
    u16 width;
    u16 height;
    u8 unknown[34];
};

struct Sprite {
    u32 data;
    u8 width;
    s8 height;
};

struct RoomDef {
    u16 unknown0;
    u8 unknown2[2];
    u8 kind;
    u8 unknown5[3];
    u8 unknown8;
    u8 category;
    u8 unknown10[9];
};

struct HospitalRoom {
    s32 unknown0;
    u8 unknown4[8];
    s32 unknownC;
    s32 unknown10;
    s32 unknown14;
    u16 unknown18;
    u8 unknown1A;
    u8 unknown1B;
    u8 unknown1C;
    u8 unknown1D;
    u16 unknown1E;
    u16 unknown20;
    u8 unknown22[2];
    u16 unknown24;
};

struct Equipment {
    u8 unknown0;
    u8 unknown1[13];
};

struct Hospital {
    u8 unknown0[0x220];
    HospitalRoom rooms[0x2e];
    u8 unknown8f4[0x32];
    Equipment equipment[55];
    u8 unknownc28;
    u8 unknownc29[0x55];
    s32 unknownc7e;
    u8 unknownc82[0x535];
    u16 unknown11b7;
    u8 unknown11b9[0x35];
    u16 unknown11ee;
    u16 unknown11f0;
    u8 unknown11f2[0x64];
    u8 unknown1256;
    u16 unknown1257;
};

struct Object {
    u8 unknown0[0xe];
    u16 unknownE;
    u8 unknown10[0x12];
    u8 unknown22;
};

struct Level {
    u8 unknown0[0x64f];
    u8 unknown64f;
    u8 unknown650;
};

struct PlayerSlot {
    s32 active;
    u8 unknown4[0x20];
};

struct Announcement {
    u8 unknown0[3];
    s8 unknown3;
    u8 unknown4;
};

struct Settings {
    u8 unknown0[3];
    ScreenMode mode;
};

struct Player {
    u8 unknown0[0x9c];
    u16 unknown9c[0x1c];
    u16 unknownd4;
    u8 unknownd6[0x14];
};

struct Event {
    u8 unknown0;
    u8 type;
    u16 value;
    u8 unknown4;
    u16 room;
    u8 unknown7[8];
};

struct View {
    s32 unknown0;
    s32 unknown4;
    s32 unknown8;
    s32 unknownC;
    u8 unknown10[0x2d0];
    s32 unknown2e0;
    s32 unknown2e4;
    s32 unknown2e8;
    s32 unknown2ec;
};

struct Message {
    char text[200];
};

struct Resource {
    u8 unknown[0x2c];
};

struct Button {
    s16 x;
    s16 y;
    u16 width;
    u16 height;
    void (*callback)(int index);
    u16 unknownC;
    u16 unknownE;
    u16 sprite;
};

struct Point {
    s32 x;
    s32 y;
};

struct Widget {
    u8 unknown0[0xd1];
    u8 unknownd1;
};

struct EquipmentDef {
    u8 unknown0;
    u8 unknown1[0x24];
};

struct WeightEntry {
    u32 unknown0;
    s32 value;
};

extern "C" {

extern int D_0013c77c[];
extern int D_0013e634[];
extern int D_0013e6f4[];
extern int D_0014be34[];
extern int *D_0015f420;
extern int *D_0015f3fe;
extern Sprite D_0013f71c[];
extern u8 D_0013fc94[];
extern u32 D_00153f44;
extern u32 D_00153f40;
extern u32 D_00153f48;
extern u32 D_00153f34;
extern u32 D_00153f1c, D_00153f20, D_00153f24;
extern u32 D_00153f4c, D_00153f50, D_00153f54;
extern u32 D_00153f58, D_00153f5c, D_00153f60;
extern u32 D_00153f80, D_00153f84, D_00153f88;
extern u32 D_00153f8c, D_00153f90, D_00153f94;
extern u32 D_001603ec, D_001603f4, D_001603f8;
extern u8 D_00155e3f;
extern char D_00120021[];
extern char D_0013f486[];
extern char D_0015f900[];

extern u8 D_001580f4;
extern u8 D_001896a9, D_001896b6, D_001896c2, D_001896c4, D_00189729, D_00189744;
extern u8 D_0015f3f9;
extern ScreenMode D_0015f3f5, D_0015f3f6;
extern ScreenMode D_0015f3f4;
extern u8 D_0015f982;
extern ScreenModeInfo D_0014d5e8[];
extern s32 D_001547a0;
extern char D_0013dd5c[], D_0013df40[], D_0013e124[], D_0013e1a8[];
extern char D_0013e33c[], D_0013e418[], D_0013e4f4[], D_0013e578[];

extern u32 D_00156b58;
extern u32 D_00153f00;
extern u8 D_00155261;
extern u16 D_00188c74;
extern u8 D_001548b5;
extern u32 D_00188c2c, D_00188c30, D_00188c34, D_00188c38;
extern u8 D_00156910;
extern Hospital *D_001547b4;
extern RoomDef D_0012d900[];

extern volatile s32 D_00188c64, D_00188c68;
extern u8 D_00155e1d;
extern u32 D_00153f08, D_00153f2c;
extern u8 D_001553e8;
extern u8 D_00155e33[];
extern s32 D_00154b58;
extern u16 D_00155e37;
extern u8 D_001548b4;
extern u8 D_0015831b;
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c7f;
extern u8 D_00155f4d, D_00158100, D_00155e14, D_00155e15, D_00155e16;
extern u16 D_0015f8f4;
extern u32 D_00155c5a, D_00155dab;
extern u16 D_00155da9;
extern u16 D_00155444;
extern u16 D_0015814c;
extern Event D_001549d0[];
extern Hospital *D_001547bc;
extern u16 D_001549ae, D_001549b0;
extern u8 D_00155e07, D_00155e39;
extern View *D_0015f430;

extern u16 D_001549b2;
extern u8 D_00158182, D_00158186, D_0015f9a2;
extern u16 D_00158184;
extern char D_0015f768[], D_0015f774[];
extern u8 D_001896d4, D_00189754, D_001896dc, D_0018975c;
extern u8 D_001896d9, D_00189759, D_001896d7, D_00189757;
extern u32 D_0015692c;
extern u16 D_00155e30;
extern u8 D_0015814e;
extern u8 D_00129e27;
extern Player *D_00154820;

extern u16 D_00155e54;
extern u8 D_00155f36, D_00155f37, D_0015f3f7, D_0015f3f8, D_0015f40a, D_0015f40b;
extern u8 *D_0015f40c, *D_0015f410, *D_0015f414, *D_0015f418, *D_0015f438;
extern u8 *D_0015f41c, *D_0015f428, *D_0015f3fa, *D_0015f402, *D_0015f406, *D_0015f42c;
extern u8 D_0013bb0c[], D_0013c0fc[], D_001361ac[], D_001310e4[], D_00145358[];
extern u8 D_0013c6a4[], D_00130288[], D_0013e5fc[], D_0014a620[], D_0014a9fe[], D_0012cc3c[];
extern View D_0014619c;
extern Point *D_0015f434;
extern Point D_00146c8c[];
extern u8 D_00155c6e;
extern u32 D_00153f3c;
extern Widget *D_001897d0;
extern char D_0012017c[], D_0012017f[], D_00120182[], D_00120187[], D_0012018b[], D_0012018f[];
extern char D_00120192[], D_00120195[], D_00120198[], D_0012019b[], D_0012019e[], D_001201a1[];
extern char **D_00160304, **D_001602a4;
extern EquipmentDef D_00129634[];
extern u16 D_00155376;

void fn_00099d00(int a, int b, int c);
void fn_000c84f0(u32 x, u32 y, u32 w, u32 h);
void fn_000b7648(int a, int b, int c, int d, u8 e);
void fn_0001b388(u32 a);
void fn_000b714c(int colour);
int fn_00058e34(u16 room, u8 player);
void fn_000c889c(int x, int y, char *text);
s16 fn_00052bcc(void);
void fn_00013540(void);
void fn_00013a7c(void);
extern s32 D_001290cc;
extern char *D_001289a0;
extern u8 D_0015f983, D_00155df9, D_00155df8, D_0015677c, D_001567a7;
extern s32 D_001564ec, D_00156504, D_001564f8, D_00156510, D_0015654c, D_00156558;
extern char D_0013e2b8[], D_0013e22c[];
extern u32 D_00153f28, D_00153f30;

extern Message D_00129004;
extern u32 D_00188c18;
extern char D_0012005b[], D_00120073[], D_00120087[], D_001200a9[], D_001200ce[], D_001200e9[];
extern char D_00120102[], D_00120129[], D_00120142[], D_0012015d[];
extern u8 D_001581ee;
extern s32 D_001581e8;
extern void *D_00158338, *D_00158334, *D_001549a0;
extern char D_0013ec5c[], D_0013ea78[], D_0013e944[], D_0013e9c8[], D_0013e7b4[];
extern Resource D_0013e810[];
extern u8 D_001547e5;

extern u8 D_00155240[], D_001553b8[], D_00156924[], D_001549cc[], D_001547c4[], D_0015fd94[];
extern u8 D_0015fd64[], D_0015f79c[];
extern Button *D_0015f43c;
extern u8 *D_0015f444;
extern Button D_00145378[];
extern u8 D_00147314[];
extern volatile s16 D_00188c5c, D_00188c60;
extern u32 D_00153f14;

extern u32 D_001549c0, D_001549c4;
extern u16 D_00155e2a;
extern u8 D_00155378, D_00155c68, D_0015536d, D_001553ea, D_001553c6, D_001553e7, D_0015543e;
extern s16 D_001547e0[2];
extern s16 D_00155c64[2];
extern u32 D_00156b54;
extern u16 D_0015543a, D_0015543c, D_0015536e;
extern Level *D_001547c0;
extern u8 D_0012b7c8[];
extern PlayerSlot D_001609c4[];
extern Announcement D_0015532d;
extern Object *D_001547a4;
extern u8 D_00155e4c;

extern char D_00120174[];
extern u8 D_0015813e;
extern u16 D_001549ac;
extern Settings D_001553e1;
extern u8 D_0012d8d8[];
extern u32 D_001606bc;
extern u8 D_0018978c;
extern u8 D_001603dc[];
extern u16 D_0015542a;
extern u8 D_00158146, D_001569f8;
extern u32 D_00153f64;
extern u8 *D_001549a4;
extern u32 D_001549a8;
extern u8 D_00155379;
extern u16 D_00155e1b;
extern u32 D_00153f38, D_001553dc;
extern u8 D_001553db, D_0015819a, D_00155d70, D_001553c8;

void fn_00090610(void);
void fn_000110b0(void);
void fn_00010dbc(void);
u8 fn_00012068(ScreenMode mode);
void fn_00012400(void);
int fn_000bed50(void);
u8 fn_00012500(u8 flag);
void fn_000859b4(void);
void fn_0008add8(void);
void fn_00074220(int a, int b);
void fn_0004e47c(u8 *a);
void fn_00028e08(void);
void fn_00059d00(void);
void fn_0005dbb0(void);
void fn_00089718(void);
void fn_00089204(void);
void fn_000a2960(void);
void fn_00089468(void);
void fn_00088bdc(void);
void fn_000a6ca4(void);
void fn_0004c730(u8 *a);
void fn_00012390(void);
void fn_0009a1f8(void);
void fn_00012148(void);
void fn_00061bd0(void);
void fn_00062730(void);
void fn_000627cc(void);
u8 fn_00068a80(void);
void fn_00068f60(void);
void fn_00068fec(void);
void fn_00063680(void);
void fn_00063a64(void);
void fn_00063ae8(void);
void fn_0007ff88(void);
void fn_00081b38(void);
void fn_00081f14(void);
void fn_0009d5b0(void);
void fn_0009d9d4(void);
void fn_0009da30(void);
void fn_00012274(void);
void fn_000a3158(void);
void fn_0008ee50(void);
void fn_0008f188(void);
void fn_0008f448(void);
void fn_0006fed8(void);
void fn_000bcfc0(void);
void fn_00019bec(void);
void fn_000bd118(void);
void fn_00011b24(void);
void fn_000ab120(void);
void fn_000bee60(void);
void fn_0005cc70(u8 *a, u32 b);
void fn_00047c58(void);
void fn_0001bc48(u32 a, int b, int c);
void fn_00019770(void);
void fn_000c2274(u32 a, ...);
void fn_000125b8(void);
void fn_00087b4c(void);
void fn_0001bdb8(int a);
void fn_000b7a1c(void);
void fn_000becd0(u8 a, ...);
void fn_00021818(int a, int b);
void fn_0008ee10(void);
u32 fn_000b0100(void);
void fn_00052e70(void);
void fn_00052c44(void);
void fn_00051534(void);
void fn_00051d98(void);
void fn_00058b00(void);
void fn_00060b54(int a, int b);
void fn_0001afd8(void);
void fn_000a2f00(int a, int b);
u8 fn_00029a54(Object *obj);
int fn_0001b4e0(s16 px, s16 py, s16 x, s16 y, s16 w, s16 h);
void fn_000fe040(int a, int b, u32 c);
extern u16 D_0015f78a, D_0015f788, D_001581fe, D_001581fc;
extern u32 D_0015f78e;
extern u8 D_00155e03, D_00155d7d, D_001581ec, D_001581ed, D_001581ef, D_001581f0, D_00155f4c;
extern u8 D_001553ce, D_0015642c, D_0015831a;
extern WeightEntry D_001290d4[];
extern s32 D_00155e50;

extern char D_00120004[];
extern u8 D_0015f980;
extern char D_00120028[], D_0012002d[], D_0012003b[], D_00120049[];
extern char *D_0013e80c;
extern u32 D_001553d7;
extern s32 D_00156b04;
extern u8 D_0015469c[];
extern u16 D_001549c8;

s16 fn_0002d7c4(void);
void fn_0009b5b8(char *path);
void fn_000bc23c(char *path, u32 a);
void fn_00010e24(void);
u8 fn_000b7a44(void);
u8 fn_00088980(void);
void fn_00074d60(void);
void fn_000bd278(int a);
void fn_000bcd18(char *name);
void fn_000be7a0(u32 a, int b, char *c);
void fn_0001a5f4(u32 a, u8 *b);
void fn_000b7b3c(u32 a);
extern char D_001548a0[];
extern u8 D_0015642e, D_00156a2e, D_00156b1e, D_0015f99f;

int fn_000bacd8(void);
void fn_0007a318(void);
u8 fn_000bc00f(char *text);
void fn_0001b7e0(void *dst, char *text, int flags);
void fn_0001bdf8(void);
u8 fn_000107c0(void);
void fn_000112b0(void);
void fn_000bc07c(void);
void fn_00010330(void);
void fn_000bc0b0(void *dst, int value, u32 size);
void fn_000bc104(void *dst, int value, u32 size);
void fn_000b0f80(void);
void fn_000baa28(void);
void fn_000bed5c(void);
void fn_000c02fc(void);
void fn_000bd688(void);
void fn_00074dc8(void);
void fn_000becfc(void);
void fn_000bbff0(char *dst, char *src);
void fn_0001b7a4(char *fmt, ...);
void fn_000a292c(void);
void fn_000c20f0(void);
void fn_000bcf5c(Resource *res);
void fn_000badfc(void);
void fn_00010ee0(void);
void fn_000be3bc(s32 handle);
s32 fn_000be2e0(char *name, int mode);
void fn_00010b20(void);
void fn_000c2111(int code);
void fn_00089df0(void);
u8 fn_000864f4(void);
void fn_000951c8(void);
void fn_000c8320(void);
void fn_00094b28(void);
void fn_000c21e6(char *a, char *b, int c);
void fn_000589f0(void);
void fn_000a4208(void);
void fn_0009e0d8(void);
void fn_0001387c(void);
void fn_0001397c(void);
u8 fn_00013d8c(u8 forward, u8 start);
int fn_000bf778(void);
u32 fn_000c24b0(void);
void fn_000c5990(u32 a, ...);
void fn_000c5a10(u32 a, ...);
u8 fn_00086b10(void);
void fn_00052038(void);
void fn_00012690(void);
void fn_00044530(void);
void fn_000b26d0(void);
void fn_00078fc4(int a, int b, int c, int d);
void fn_000914e0(void);
void fn_0007e210(void);
void fn_00056d50(void);
void fn_000b4e88(void);

int fn_000bd4d0(u32 font);
void fn_000bd7dc(s32 width, s32 height);
void fn_00011b90(void);
void fn_000a6b20(void);
void fn_000be948(u32 a, u32 b, u32 c, u32 d);
void fn_000868f8(void);
void fn_0001c7c0(void);
void fn_0009ae08(int a, int b);
void fn_00088ebc(int a, int b, int c, int d);
u32 fn_000c21c0(void);
void fn_000c8390(u32 a, u32 b, u32 c, u32 d, u32 e);
void fn_000ab49c(u32 a, u32 b, u32 c, u32 d, u8 e);
void *fn_000bac40(u32 size);
void fn_000bac20(void);
void fn_000bce44(char *name);
void fn_000bcf90(char *name);
void fn_00031ffc(int n);
void fn_0008bcb4(void);
int fn_000bea80(int mode, ...);
void fn_000baca4(void *p);
int fn_000bc13a(char *buf, char *fmt, ...);
int fn_000bc170(char *path);
u8 fn_000bbbb0(char *path, void *buf, u32 size, int count);
void fn_000bc86c(u32 a, u32 b, u32 c);
u8 fn_00010f68(u32 a, ScreenMode mode);
void fn_00011084(void);
void fn_00010760(void);
void fn_00011058(void);

// NONMATCHING: stack slots of i and the return value are swapped
// FUNCTION: 0x00010010
int fn_00010010(u16 argc, char **argv)
{
    int i;
    char *arg;
    s16 c;

    if (fn_000bacd8() == -1) {
        fn_0001b7a4(D_00120004, 4900000);
        return 1;
    }
    fn_00010330();
    fn_0007a318();
    for (i = 1; (u16)i < argc; i++) {
        arg = argv[(u16)i];
        if (*arg == '-' || *arg == '/') {
            arg++;
            while (c = (s8)*arg++) {
                switch (c) {
                case 'B':
                case 'b':
                    fn_000bbff0(D_001548a0, arg);
                    goto next_arg;
                case 'C':
                case 'c':
                    D_001547e5 = fn_000bc00f(arg);
                    goto next_arg;
                case 'E':
                case 'e':
                    D_0015642e = 1;
                    break;
                case 'G':
                case 'g':
                    D_00156a2e = fn_000bc00f(arg);
                    goto next_arg;
                case 'S':
                case 's':
                    D_00156b1e = 1;
                    D_00158100 = 1;
                    goto next_arg;
                case 'L':
                case 'l':
                    D_00158100 = fn_000bc00f(arg);
                    D_0015f99f = 1;
                    D_00156b1e = 1;
                    goto next_arg;
                case 'N':
                case 'n':
                    fn_0001b7e0(D_001547c4, arg, 0);
                    goto next_arg;
                case 'P':
                case 'p':
                    D_001581ed = 1;
                    D_001581ef = fn_000bc00f(arg);
                    goto next_arg;
                case 'Q':
                case 'q':
                    D_00155f4c = 1;
                    goto next_arg;
                case 'R':
                case 'r':
                    D_001581ec = 1;
                    D_001581ef = fn_000bc00f(arg);
                    goto next_arg;
                default:
                    break;
                }
            }
        }
    next_arg:;
    }
    if (D_00155f4c == 0) {
        fn_0001bdf8();
    }
    if (fn_000107c0() == 0) {
        fn_000112b0();
    }
    fn_00010b20();
    fn_000bc07c();
    fn_000bac20();
    fn_000badfc();
    return 0;
}

// FUNCTION: 0x00010330
void fn_00010330(void)
{
    int i;

    fn_000bc0b0(D_00155240, 0, 0x178);
    fn_000bc0b0(D_001553b8, 0, 0xa96);
    fn_000bc0b0(D_00156924, 0, 0x21c);
    fn_000bc0b0(D_001549cc, 0, 0x3c);
    fn_000bc0b0(D_001547c4, 0, 0x5c);
    fn_000bc0b0(D_0015fd94, 0, 0x4e);
    fn_000bc0b0(D_0015fd64, 0, 0x2d);
    fn_000bc104(D_0015f79c, 0, 0x155);
    D_0015f3f6 = SCREEN_MODE_13;
    D_0015f3f7 = 0x20;
    D_0015f3f8 = 0x40;
    D_0015f40c = D_0013bb0c;
    D_0015f430 = &D_0014619c;
    D_0015f410 = D_0013c0fc;
    D_0015f434 = D_00146c8c;
    D_0015f414 = D_001361ac;
    D_0015f418 = D_001310e4;
    D_0015f438 = D_00145358;
    D_0015f41c = D_0013c6a4;
    D_0015f43c = D_00145378;
    D_0015f420 = D_0013c77c;
    D_0015f444 = D_00147314;
    D_0015f3fa = D_0013e5fc;
    D_0015f402 = D_0014a620;
    D_0015f406 = D_0014a9fe;
    D_0015f3fe = D_0013e634;
    D_0015f428 = D_00130288;
    D_0015f40a = 3;
    D_0015f40b = 0x20;
    D_0015f42c = D_0012cc3c;
    D_0015f3f9 = 0;
    D_0015831b &= 0xfe;
    D_0015f78a = 0;
    D_0015f788 = 0;
    D_0015f78e = 0;
    D_00155e03 = 0;
    D_00155d7d = 0;
    D_001581ed = D_001581ec = D_001581ee = D_001581ef = 0;
    D_001581f0 = 0;
    D_00155f4d = 1;
    D_00154b58 = 0;
    D_00155f4c = 0;
    for (i = 0; D_001290d4[i].value != 0; i++) {
        D_00155e50 += D_001290d4[i].value;
    }
    D_001553ce = 1;
    D_0015642c = 0x40;
    D_001581fe = 0;
    D_001581fc = 0;
    D_0015831a = 0;
    D_00155e54 = 0x1b0;
    fn_000b0f80();
    fn_000baa28();
}

// FUNCTION: 0x00010584
void fn_00010584(void)
{
    void *buf;
    int count;
    char path[144];

    buf = fn_000bac40(0x10000);
    if (buf != 0) {
        fn_000bc13a(path, D_00120021, D_0015f900, D_0013f486);
        count = fn_000bc170(path) / 12;
        D_00155e3f = fn_000bbbb0(path, buf, 0x10000, count);
        fn_000baca4(buf);
    } else {
        D_00155e3f = 2;
    }
}

// FUNCTION: 0x00010618
void fn_00010618(void)
{
    u32 i = 0;

    fn_00010760();
    for (; D_0013f71c[i].height >= 0; i++) {
        if (i >= 0x50) {
            D_0013f71c[i].data = ((u8)D_0013f71c[i].data + 0x51) * 6 + D_00153f44;
        } else {
            D_0013f71c[i].data = (u8)D_0013f71c[i].data * 6 + D_00153f44;
        }
    }
    fn_000bc86c(D_00153f44, D_00153f48, D_00153f40);
}

// FUNCTION: 0x000106c0
void fn_000106c0(void)
{
    fn_00010618();
    fn_000bc86c(D_00153f84, D_00153f88, D_00153f80);
    fn_000bc86c(D_001603f4, D_001603f8, D_001603ec);
    fn_000bc86c(D_00153f90, D_00153f94, D_00153f8c);
    fn_000bc86c(D_00153f20, D_00153f24, D_00153f1c);
    fn_000bc86c(D_00153f50, D_00153f54, D_00153f4c);
    fn_000bc86c(D_00153f5c, D_00153f60, D_00153f58);
}

// FUNCTION: 0x00010760
void fn_00010760(void)
{
    int i;

    for (i = 0; D_0013f71c[i].height >= 0; i++) {
        D_0013f71c[i].data = D_0013fc94[i];
    }
}

// FUNCTION: 0x000107c0
u8 fn_000107c0(void)
{
    char path[128];
    char name[52];

    D_0015f982 = 0;
    D_0015f3f9 = 0;
    fn_000bce44(D_0013e9c8);
    fn_000bce44(D_0013e7b4);
    if (!fn_00010f68(D_00153f34, SCREEN_MODE_13)) {
        return 1;
    }
    if (fn_0002d7c4() == 0) {
        D_0015f982 = 1;
        return 1;
    }
    if (D_0015f980 < 2) {
        fn_000bc13a(path, D_00120028, D_0015f900, D_0013e80c);
        fn_0009b5b8(path);
        fn_000bc23c(path, D_00153f34);
    } else {
        fn_000bc23c(D_0013e80c, D_00153f34);
    }
    if (D_00156b1e) {
        fn_00010e24();
    }
    fn_000bce44(D_0013e22c);
    fn_000bc86c(D_00153f2c, D_00153f30, D_00153f28);
    D_00155f36 = 0x1c;
    D_00155f37 = 0x1f;
    D_0015f3f7 = 0x20;
    D_0015f3f8 = 0x40;
    D_0015f3f8 = 0x40;
    D_0015f3f7 = 0x20;
    D_0015f40c = D_0013bb0c;
    D_0015f430 = &D_0014619c;
    D_0015f410 = D_0013c0fc;
    D_0015f434 = D_00146c8c;
    D_0015f414 = D_001361ac;
    D_0015f418 = D_001310e4;
    D_0015f438 = D_00145358;
    D_0015f41c = D_0013c6a4;
    D_0015f420 = D_0013c77c;
    D_0015f428 = D_00130288;
    D_0015f3f5 = SCREEN_MODE_13;
    D_0015f42c = D_0012cc3c;
    D_0015f40a = 3;
    D_0015f40b = 0x20;
    D_0015f3f4 = SCREEN_MODE_13;
    D_0015f3fe = D_0013e634;
    D_0015f444 = D_00147314;
    D_0015f43c = D_00145378;
    D_0015f3f9 = 0;
    if (!fn_000b7a44()) {
        return 0;
    }
    D_0015f982 = fn_00088980();
    if (D_0015f982) {
        return 1;
    }
    fn_000bce44(D_0013ea78);
    fn_000bce44(D_0013e944);
    fn_00074d60();
    if (fn_000bd4d0(D_00153f2c + 6) != 1) {
        D_0015f982 = 2;
        return 1;
    }
    fn_000bd278(0);
    fn_000bcd18(D_0013ec5c);
    D_001553d7 = 0;
    D_001581ee = 0;
    if (D_001581ec) {
        fn_000bc13a(name, D_0012002d, D_001581ef);
        D_001581e8 = fn_000be2e0(name, 0);
        if (D_001581e8) {
            D_001581ee |= 1;
        }
    } else if (D_001581ed) {
        fn_000bc13a(name, D_0012003b, D_001581ef);
        D_001581e8 = fn_000be2e0(name, 2);
        if (D_001581e8) {
            D_001581ee |= 2;
        }
    }
    D_00156b04 = -1;
    fn_000be7a0(D_00153f34, 0x42, D_00120049);
    fn_0001a5f4(D_00153f34, D_0015469c);
    fn_000b7b3c(D_00153f34);
    D_001290cc = fn_000be2e0(D_001289a0, 2);
    if (D_001290cc == -1) {
        D_0015f982 = 9;
        D_0015f983 = 7;
        return 1;
    }
    D_001549c8 = 3;
    return 0;
}

// FUNCTION: 0x00010b20
void fn_00010b20(void)
{
    Message msg = D_00129004;

    if (D_00188c18) {
        fn_000bed5c();
    }
    fn_000c02fc();
    fn_000bd688();
    fn_00074dc8();
    fn_000becfc();
    switch (D_0015f982) {
    case 1:
        fn_000bbff0(msg.text, D_0012005b);
        break;
    case 2:
        fn_000bbff0(msg.text, D_00120073);
        break;
    case 3:
        fn_000bbff0(msg.text, D_00120087);
        break;
    case 4:
        fn_000bbff0(msg.text, D_001200a9);
        break;
    case 5:
        fn_000bbff0(msg.text, D_001200ce);
        break;
    case 6:
        fn_000bbff0(msg.text, D_001200e9);
        break;
    case 8:
        fn_0001b7a4(D_00120102);
        break;
    case 9:
        fn_0001b7a4(D_00120129);
        break;
    case 10:
        fn_0001b7a4(D_00120142);
        break;
    }
    if (D_0015f983) {
        fn_000bc13a(msg.text, D_0012015d, msg.text, D_0015f983);
    }
    if (msg.text[0]) {
        fn_0001b7a4(msg.text);
    }
    switch (D_0015f3f4) {
    case SCREEN_MODE_1:
        fn_000bcf90(D_0013e2b8);
        break;
    case SCREEN_MODE_13:
        fn_000bcf90(D_0013e22c);
        break;
    }
    fn_00010ee0();
    if (D_001581ee & 3) {
        fn_000be3bc(D_001581e8);
    }
    fn_000a292c();
    fn_000c20f0();
    fn_000baca4(D_00158338);
    fn_000baca4(D_00158334);
    fn_000baca4(D_001549a0);
    fn_000bcf90(D_0013ec5c);
    fn_000bcf90(D_0013ea78);
    fn_000bcf90(D_0013e944);
    fn_000bcf90(D_0013e9c8);
    fn_000bcf5c(&D_0013e810[D_001547e5]);
    fn_000bcf90(D_0013e7b4);
    if (D_001290cc != -1) {
        fn_000be3bc(D_001290cc);
    }
    fn_000badfc();
}

// FUNCTION: 0x00010dbc
void fn_00010dbc(void)
{
    D_001580f4 = 0;
    if (D_001896b6 || D_001896c2) {
        D_001580f4 |= 1;
    }
    if (D_001896c4 || D_00189744) {
        D_001580f4 |= 4;
    }
    if (D_001896a9 || D_00189729) {
        D_001580f4 |= 2;
    }
}

// FUNCTION: 0x00010e24
void fn_00010e24(void)
{
    if (D_0015f3f9 == 2) {
        return;
    }
    if (D_0015f3f5 == SCREEN_MODE_1) {
        fn_000bce44(D_0013e1a8);
        fn_000bce44(D_0013dd5c);
        fn_000bce44(D_0013e578);
        fn_000bce44(D_0013e418);
        fn_00031ffc(1);
        fn_0008bcb4();
    } else {
        fn_000bce44(D_0013df40);
        fn_000bce44(D_0013e124);
        fn_000bce44(D_0013e4f4);
        D_001547a0 = -1;
        fn_000bce44(D_0013e33c);
        fn_00031ffc(13);
    }
    D_0015f3f6 = D_0015f3f5;
    fn_000106c0();
    D_0015f3f9 = 2;
}

// FUNCTION: 0x00010ee0
void fn_00010ee0(void)
{
    if (D_0015f3f9 == 2) {
        if (D_0015f3f5 == SCREEN_MODE_1) {
            fn_000bcf90(D_0013e1a8);
            fn_000bcf90(D_0013dd5c);
            fn_000bcf90(D_0013e578);
            fn_000bcf90(D_0013e418);
        } else {
            fn_000bcf90(D_0013df40);
            fn_000bcf90(D_0013e124);
            fn_000bcf90(D_0013e4f4);
            fn_000bcf90(D_0013e33c);
        }
        D_0015f3f9 = 0;
        fn_000bac20();
    }
}

// FUNCTION: 0x00010f68
u8 fn_00010f68(u32 a, ScreenMode mode)
{
    int variant;

    if (mode == D_0015f3f4) {
        return 1;
    }
    if (mode == SCREEN_MODE_13) {
        variant = 1;
        fn_00011058();
    } else if (mode == SCREEN_MODE_1) {
        variant = 2;
        fn_00011084();
    }
    if (fn_000bea80(mode, D_0014d5e8[mode].width, D_0014d5e8[mode].height, a, variant, 0) != 1) {
        if (mode == SCREEN_MODE_13) {
            D_0015f982 = 4;
        } else {
            D_0015f982 = 3;
        }
        return 0;
    }
    D_0015f3f4 = mode;
    return 1;
}

// FUNCTION: 0x0001102c
void fn_0001102c(void)
{
    fn_00011084();
    fn_00010f68(D_00153f34, SCREEN_MODE_1);
}

// FUNCTION: 0x00011058
void fn_00011058(void)
{
    D_0015f420 = D_0013c77c;
    D_0015f3fe = D_0013e634;
}

// FUNCTION: 0x00011084
void fn_00011084(void)
{
    D_0015f420 = D_0014be34;
    D_0015f3fe = D_0013e6f4;
}

// FUNCTION: 0x000110b0
void fn_000110b0(void)
{
    D_00155e54 = D_0014d5e8[SCREEN_MODE_13].height - 0x30;
    D_00155f36 = 0x1c;
    D_00155f37 = 0x1f;
    D_0015f3f5 = SCREEN_MODE_13;
    D_0015f3f4 = SCREEN_MODE_13;
    D_0015f40c = D_0013bb0c;
    D_0015f430 = &D_0014619c;
    D_0015f410 = D_0013c0fc;
    D_0015f434 = D_00146c8c;
    D_0015f3f7 = 0x20;
    D_0015f3f8 = 0x40;
    D_0015f414 = D_001361ac;
    D_0015f418 = D_001310e4;
    D_0015f438 = D_00145358;
    D_0015f41c = D_0013c6a4;
    D_0015f420 = D_0013c77c;
    D_0015f428 = D_00130288;
    D_0015f3fa = D_0013e5fc;
    D_0015f402 = D_0014a620;
    D_0015f406 = D_0014a9fe;
    D_0015f3fe = D_0013e634;
    D_0015f40a = 3;
    D_0015f40b = 0x20;
    D_0015f42c = D_0012cc3c;
    if (D_001290cc != -1) {
        fn_000be3bc(D_001290cc);
        D_001290cc = -1;
    }
    D_001290cc = fn_000be2e0(D_001289a0, 2);
    if (D_001290cc == -1) {
        D_0015f982 = 9;
        D_0015f983 = 7;
        fn_00010b20();
        fn_000c2111(1);
    }
    D_00155df9 = 1;
    D_00155df8 = 0;
    D_001564ec = 0;
    D_00156504 = D_0014d5e8[SCREEN_MODE_13].width;
    D_001564f8 = 0;
    D_00156510 = D_0014d5e8[SCREEN_MODE_13].height;
    D_0015677c = 0;
    D_0015654c = (D_001564ec + D_00156504) / 2;
    D_00156558 = (D_001564f8 + D_00156510) / 2;
    D_001567a7 = 0;
    fn_00089df0();
    fn_000bcf90(D_0013e2b8);
    fn_000bce44(D_0013e22c);
    fn_000bc86c(D_00153f2c, D_00153f30, D_00153f28);
}

// FUNCTION: 0x000112b0
void fn_000112b0(void)
{
    u8 redraw;
    u8 quit;
    int i;
    char path[128];

    quit = 0;
    fn_000bc13a(path, D_00120174, D_0015f900, D_0013f486);
    fn_0009b5b8(path);
    fn_00010584();
    fn_00090610();
    if (D_00156b1e) {
        D_001549ae = 0;
        D_00155261 = 0;
        fn_000110b0();
    } else {
        D_001549ae = 2;
        D_0015813e = 0x17;
    }
    fn_0001c7c0();
    while (D_001549ac == 0) {
        D_00155c5a = fn_000c21c0();
        fn_00010dbc();
        if (D_001553e1.mode) {
            if (!fn_00012068(D_001553e1.mode)) {
                break;
            }
            D_001553e1.mode = (ScreenMode)0;
        }
        fn_00012400();
        if (fn_000bed50() != 1) {
            redraw = D_0012d8d8[D_001549ae];
            if (D_001549ae == 13) {
                redraw = fn_00012500(0);
            } else {
                fn_000ab49c(0, 0, D_00188c2c, D_00188c30, 3);
            }
        } else {
            if (D_0012d8d8[D_001549ae]) {
                fn_00011b90();
            }
            redraw = D_0012d8d8[D_001549ae];
            D_001606bc++;
            switch (D_001549ae) {
            case 0:
                fn_000a6b20();
                fn_000859b4();
                fn_0008add8();
                fn_00074220(D_001580f4, D_0018978c);
                fn_0004e47c(D_001603dc);
                fn_00028e08();
                fn_00059d00();
                fn_0005dbb0();
                fn_00089718();
                fn_00089204();
                fn_000a2960();
                fn_00089468();
                fn_00088bdc();
                fn_000a6ca4();
                fn_0004c730(D_001603dc);
                fn_0001c7c0();
                fn_00012390();
                fn_0009a1f8();
                break;
            case 1:
                fn_00012148();
                fn_00074220(D_001580f4, D_0018978c);
                fn_000a2960();
                fn_0009a1f8();
                fn_0001c7c0();
                break;
            case 2:
                fn_00061bd0();
                fn_00062730();
                fn_000627cc();
                fn_000a2960();
                fn_0001c7c0();
                break;
            case 3:
                if (!fn_00068a80()) {
                    goto done;
                }
                fn_00068f60();
                fn_00068fec();
                fn_000a2960();
                fn_0001c7c0();
                break;
            case 4:
                fn_00063680();
                fn_0004e47c(D_001603dc);
                fn_00063a64();
                fn_00063ae8();
                fn_000a2960();
                fn_0001c7c0();
                break;
            case 5:
                fn_0007ff88();
                fn_00081b38();
                fn_00081f14();
                fn_000a2960();
                break;
            case 6:
                fn_0009d5b0();
                fn_0009d9d4();
                fn_0009da30();
                fn_000a2960();
                fn_00089204();
                break;
            case 7:
                if (D_0015542a != 0) {
                    switch (D_0015542a) {
                    case 1:
                    case 2:
                        fn_0009e0d8();
                        break;
                    default:
                        D_0015542a = 0;
                        break;
                    }
                }
                fn_0001c7c0();
                break;
            case 8:
                fn_00012274();
                fn_00089204();
                fn_000a2960();
                fn_0009a1f8();
                fn_0001c7c0();
                break;
            case 9:
                fn_000a3158();
                fn_000a2960();
                fn_00089204();
                fn_0009a1f8();
                fn_0001c7c0();
                break;
            case 10:
                fn_0008ee50();
                fn_0008f188();
                fn_0008f448();
                fn_000a2960();
                fn_00089204();
                fn_0001c7c0();
                break;
            case 11:
                fn_000a6b20();
                fn_000859b4();
                fn_0004e47c(D_001603dc);
                fn_0006fed8();
                fn_0005dbb0();
                fn_000a2960();
                fn_00088bdc();
                fn_00089204();
                fn_000a6ca4();
                fn_0004c730(D_001603dc);
                fn_0001c7c0();
                fn_0009a1f8();
                break;
            case 12:
                fn_000c21e6(D_0015f768, D_0015f774, 10);
                fn_0009e0d8();
                fn_0001c7c0();
                break;
            case 13:
                redraw = fn_00012500(1);
                break;
            case 99:
                fn_0001c7c0();
                break;
            case 14:
                fn_00063680();
                fn_0004e47c(D_001603dc);
                fn_00063a64();
                fn_00063ae8();
                fn_000a2960();
                fn_0001c7c0();
                if (D_00158146 >= 0x45 && D_00155f4d > 1) {
                    redraw = 0;
                }
                break;
            default:
                break;
            }
            fn_000bed5c();
        }
        if (D_001569f8 != 0) {
            if (D_001569f8 == 1) {
                D_001569f8 = 0;
            }
            fn_000bcfc0();
            while (fn_000bed50() != 1) {
            }
            fn_00019bec();
            fn_000bed5c();
            fn_000bd118();
        }
        fn_00011b24();
        if ((D_001549ae == 0 || D_001549ae == 11) && D_0015f3f4 == SCREEN_MODE_13 && D_00155e39 == 0) {
            fn_000ab120();
        } else {
            if (D_00155e39) {
                D_00155e39--;
            }
            fn_000bee60();
        }
        if (redraw && !D_00155261 && D_0015536d == 0 && D_00153f64 && D_001549ac != 1) {
            fn_0005cc70(D_001549a4 + 0xaf, D_001549a8);
            fn_00047c58();
            if (D_00155379 || D_00155c68 || D_00155e1b) {
                for (i = 0; D_00155e1b + 2 > i; i++) {
                    fn_0005cc70(D_001549a4 + 0xaf, D_001549a8);
                    fn_00047c58();
                    fn_00011b90();
                }
                D_00155c68 = 0;
                D_00155e1b = 0;
            }
        }
        if (D_001553d7) {
            fn_0001bc48(D_001553d7, 10, 1);
            D_00153f38 = D_001553d7;
            D_001553d7 = 0;
            fn_00019770();
        }
        if (D_001553db) {
            fn_0001bc48(0, 10, 1);
            D_001553db = 0;
        }
        if (D_001553dc) {
            fn_000c2274(D_001553dc);
            D_00153f38 = D_001553dc;
            D_001553dc = 0;
            fn_00019770();
        }
        if (D_00155e14 || D_00155e15 || D_00155e16) {
            if (D_001549ae == 0 && D_0015819a == 0x63 && D_00155d70 == 0 && D_0015536e != 0x25 && !D_00155e4c
                && D_0015536e != 3) {
                fn_000125b8();
            }
        }
        switch (D_001549ac) {
        case 1:
            fn_00087b4c();
            if (D_0015819a != 0x63) {
                fn_0001bdb8(D_0015819a);
            }
            D_0015813e = 0x1f;
            D_001549ae = 2;
            fn_000b7a1c();
            fn_000becd0(0);
            fn_000bee60();
            D_001549ac = 0;
            fn_00021818(0x63, -1);
            fn_0008ee10();
            if (D_00156b1e) {
                D_001549ac = 1;
            }
            break;
        case 2:
            D_0015f99f = 1;
            D_001549ac = 0;
            D_001553c8 = 1;
            break;
        default:
            break;
        }
    }
done:
    fn_000c5a10(fn_000c24b0());
}

// FUNCTION: 0x00011b24
void fn_00011b24(void)
{
    u32 i;
    u32 now;

    while ((now = fn_000c21c0()) - D_00156b58 < 0x28) {
        for (i = 0; i < 1000; i++) {
            D_00153f00 = D_00153f00 * 11 % (D_00153f00 - 10);
        }
    }
    D_00156b58 = now;
}

// NONMATCHING: call arguments load the pointer into the argument register itself in target
// FUNCTION: 0x00012148
void fn_00012148(void)
{
    switch (D_001549b0) {
    case 0:
        fn_00052038();
        break;
    case 1:
        fn_00012690();
        break;
    case 2:
        fn_00044530();
        break;
    case 3:
        fn_000b26d0();
        break;
    case 4:
        fn_00078fc4((u16)D_0015f430->unknown2e0, (u16)D_0015f430->unknown2e4, (u16)D_0015f430->unknown2ec,
                    (u16)D_0015f430->unknown2e8);
        break;
    case 5:
        fn_000914e0();
        break;
    case 6:
        fn_0007e210();
        break;
    case 7:
        fn_00056d50();
        break;
    case 13:
        fn_000b4e88();
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    default:
        D_001549d0[D_00154b58].type = 0x34;
        break;
    }
}

// FUNCTION: 0x00012274
void fn_00012274(void)
{
    switch (D_001549b2) {
    case 0:
        if (D_00155f4d > 1) {
            D_001549b2 = 2;
            D_00158184 = fn_000864f4();
        } else {
            D_00158182 = 0;
            D_001549b2++;
            D_00158184 = 1;
            fn_000951c8();
        }
        D_00158186 = 0;
        D_0015f9a2 = 0;
        D_00155261 = 1;
        fn_000c8320();
        break;
    case 1:
        fn_00094b28();
        break;
    case 2:
        fn_000b4e88();
        break;
    case 3:
        D_001549b2 = 4;
        break;
    case 4:
        fn_000c21e6(D_0015f768, D_0015f774, 10);
        fn_000589f0();
        if (D_001549b2 == 4) {
            fn_000a4208();
        }
        break;
    case 5:
    case 6:
    case 7:
        fn_0009e0d8();
        break;
    }
}

// FUNCTION: 0x00012390
void fn_00012390(void)
{
    if (D_00155261) {
        D_00188c74 = 4;
        fn_000c8390(0, 0, D_00188c34, D_00188c38, D_001548b5);
        fn_000ab49c(0, 0, D_00188c2c, D_00188c30, 3);
        D_00188c74 = 0;
        D_00156910 = 1;
    }
}

// FUNCTION: 0x00013a7c
void fn_00013a7c(void)
{
    u8 room;

    if (D_001896d4 || D_00189754) {
        room = fn_00013d8c(0, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            fn_00088ebc(10, 0xa8, -1, -1);
        } else {
            fn_00088ebc(10, 8, -1, -1);
        }
    }
    if (D_001896dc || D_0018975c) {
        room = fn_00013d8c(1, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            fn_00088ebc(10, 0xa8, -1, -1);
        } else {
            fn_00088ebc(10, 8, -1, -1);
        }
    }
    if (D_001896d9 || D_00189759) {
        fn_0001387c();
    } else if (D_001896d7 || D_00189757) {
        fn_0001397c();
    } else if (!D_00188c7e) {
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
    }
}

// NONMATCHING: case 5 builds the event address in ecx instead of eax
// FUNCTION: 0x00013bc4
void fn_00013bc4(u8 action)
{
    u8 room;

    D_0015692c = action;
    switch (action) {
    case 0:
        fn_0001387c();
        D_00155e30 = 0x32;
        break;
    case 1:
        fn_0001397c();
        D_00155e30 = 0x32;
        break;
    case 2:
        room = fn_00013d8c(0, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            fn_00088ebc(10, 0xa8, -1, -1);
        } else {
            fn_00088ebc(10, 8, -1, -1);
        }
        D_00155e30 = 0x32;
        break;
    case 3:
        room = fn_00013d8c(1, D_00155444);
        if (room != 0xfa) {
            D_00155444 = room;
            fn_00088ebc(10, 0xa8, -1, -1);
        } else {
            fn_00088ebc(10, 8, -1, -1);
        }
        D_00155e30 = 0x32;
        break;
    case 4:
        D_0015814e = 1;
        fn_00088ebc(10, 0x42, -1, -1);
        break;
    case 5:
        D_001549d0[D_00154b58].type = 0x72;
        D_001549d0[D_00154b58].value = D_00155444;
        fn_00088ebc(10, 0x43, -1, -1);
        break;
    }
}

// NONMATCHING: index/pointer register choice (pointer in edx in target)
// FUNCTION: 0x00013d8c
u8 fn_00013d8c(u8 forward, u8 start)
{
    u8 i;

    switch (forward) {
    case 0:
        if (start - 1 > 0) {
            for (i = start - 1; i > 0; i--) {
                if ((i < 0x24 && D_001547bc->rooms[i].unknown1D)
                    || (i >= 0x24 && D_001547b4[D_00154b58].rooms[i].unknown1C == 1)
                    || (i == 0x2c && (D_001547b4[D_00154b58].unknownc28 != 0 && D_00129e27 != 0)
                        && D_00154820[D_00154b58].unknownd4)) {
                    return i;
                }
            }
        }
        break;
    default:
        if (start + 1 < 0x2e) {
            for (i = start + 1; i < 0x2e; i++) {
                if ((i < 0x24 && D_001547bc->rooms[i].unknown1D)
                    || (i >= 0x24 && D_001547b4[D_00154b58].rooms[i].unknown1C == 1)
                    || (i == 0x2c && (D_001547b4[D_00154b58].unknownc28 != 0 && D_00129e27 != 0)
                        && D_00154820[D_00154b58].unknownd4)) {
                    return i;
                }
            }
        }
        break;
    }
    return 0xfa;
}

// NONMATCHING: address built in edx and value in eax (known unsolved store pattern)
// FUNCTION: 0x00013fb0
void fn_00013fb0(u16 a, u16 b)
{
    if (D_0012d900[b].kind == 0x27) {
        D_001547b4[a].rooms[b].unknown1B = D_0012d900[57].unknown10[3];
        D_001547b4[a].rooms[b].unknown18 = *(u16 *)&D_0012d900[57].unknown10[5];
    }
}

// NONMATCHING: index/pointer register choice (pointer in edx in target)
// FUNCTION: 0x00011b90
void fn_00011b90(void)
{
    u8 count;
    u8 i;
    u32 now;
    Object *obj;

    now = fn_000c21c0();
    D_001549c0++;
    D_00155e2a = 0;
    if (D_00155378) {
        D_001547e0[1] = 0x7d00;
    }
    if (now - D_00156b54 < 1000 / D_001547e0[1] && D_00155c68 == 0 && D_00155f4d <= 1) {
        D_0015536d = 1;
        return;
    }
    D_00156b54 = now;
    D_0015536d = 0;
    if (D_00155261) {
        return;
    }
    D_001549c4++;
    D_001553ea = (D_001549c4 & 0xf) == 0;
    *(u16 *)D_001553b8 = fn_000b0100() % 100;
    D_001553c6 = fn_000b0100() % 6;
    D_001553e7 = (fn_000b0100() & 3) + 1;
    D_0015543a = D_001549c4;
    D_0015543c = D_001549c4 + *(u16 *)((u8 *)D_0012d900 + 0x464);
    D_0015543e = (D_001549c4 & 0x1f) == 0;
    if (D_00155c64[1] > 0) {
        D_00155c64[1]--;
    }
    if (D_001549c4 % 25 == 0) {
        fn_00052e70();
        if (D_001549c4 % 50 == 0) {
            fn_00052c44();
            if ((int)D_001547c0->unknown64f > (int)D_0012b7c8[D_001547c0->unknown650]) {
                fn_00051534();
                if (D_001547c0->unknown650 >= 12) {
                    fn_00051d98();
                } else if (D_001547c0->unknown650 % 3 == 0) {
                    fn_00058b00();
                }
            }
        }
    }
    count = 1;
    if (D_00155f4d > 1) {
        count = 4;
    }
    for (i = 0; i < count; i++) {
        if (D_00155f4d > 1 && D_001609c4[i].active == 0) {
            continue;
        }
        D_001547b4[i].unknown11ee = D_001547b4[i].unknown11f0;
        D_001547b4[i].unknown11f0 = 0;
        if ((D_00155f4d > 1 ? 1 : D_001547b4[i].unknown1256) != 0 || (D_0015831b & 1)) {
            continue;
        }
        D_001547b4[i].unknown1257--;
        if (!D_001547b4[i].unknown1257) {
            if (i != D_00154b58) {
                fn_00060b54(0xc, i);
            } else {
                fn_00088ebc(10, 0xd9, -1, -1);
            }
            if (D_00155f4d <= 1) {
                D_001547b4[i].unknown1256 = 1;
            }
        }
    }
    fn_0001afd8();
    if (D_0015532d.unknown3) {
        fn_000a2f00(D_0015532d.unknown3, D_0015532d.unknown4);
    }
    obj = &D_001547a4[D_001547b4[D_00154b58].unknown11b7];
    if (obj > D_001547a4) {
        if (obj->unknown22 && fn_00029a54(&D_001547a4[D_001547b4[D_00154b58].unknown11b7])) {
            if (D_0015536e == 0xe) {
                if (D_001547a4[D_001547b4[D_00154b58].unknown11b7].unknownE == 0) {
                    D_001549d0[D_00154b58].type = 0x83;
                }
            } else {
                D_001549d0[D_00154b58].type = 0x7b;
            }
        }
    }
    if (D_00155f4d > 1) {
        switch (D_00155e4c) {
        case 1:
            D_001549d0[D_00154b58].type = 0x7e;
            D_001549d0[D_00154b58].value = 1;
            break;
        case 2:
            D_001549d0[D_00154b58].type = 0x90;
            break;
        default:
            break;
        }
    }
}

// FUNCTION: 0x00012068
u8 fn_00012068(ScreenMode mode)
{
    s32 width = D_00188c64;
    s32 height = D_00188c68;

    if (D_0015f3f4 == SCREEN_MODE_13 && mode != SCREEN_MODE_13) {
        width = width / 2;
        height = height * 5 / 12;
    } else if (D_0015f3f4 != SCREEN_MODE_13 && mode == SCREEN_MODE_13) {
        width <<= 1;
        height = height * 12 / 5;
    }
    if (!fn_00010f68(D_00153f34, mode)) {
        return 0;
    } else {
        u32 font;

        if (D_00155e1d) {
            font = D_00153f08;
        } else {
            font = D_00153f2c;
        }
        fn_000bd4d0(font + D_001553e8 * 6);
        fn_000bd7dc(width, height);
        return 1;
    }
}

// FUNCTION: 0x00012400
void fn_00012400(void)
{
    if (D_00155f4d > 1) {
        if (fn_000bf778() && D_00155e33[D_00154b58] != 0) {
            D_00155e39 = 0xf;
            D_001549d0[D_00154b58].type = 0x8e;
            if (D_00155e07) {
                fn_000c5990(fn_000c24b0());
            }
        } else if (!fn_000bf778() && D_00155e33[D_00154b58] == 0) {
            D_001549d0[D_00154b58].type = 0x8d;
            if (fn_00086b10()) {
                D_00155e33[D_00154b58] = 2;
            } else {
                D_00155e33[D_00154b58] = 1;
            }
            if (D_001549ae != 0xd) {
                D_00155e37 = D_001549ae;
                D_001549ae = 0xd;
            }
            if (D_00155e07) {
                fn_000c5a10(fn_000c24b0());
            }
        }
    }
}

// FUNCTION: 0x00012500
u8 fn_00012500(u8 flag)
{
    u8 result;

    if (D_00155e33[D_00154b58] == 1) {
        fn_00011b90();
        result = 1;
    } else {
        result = 0;
    }
    if (flag) {
        switch (D_00155e37) {
        case 0:
        case 11:
            fn_000a6b20();
            break;
        default:
            fn_000be948(0, 0, D_00188c2c, D_00188c30);
            fn_000c8390(0, 0, D_00188c2c, D_00188c30, D_001548b4);
            break;
        }
        fn_000868f8();
    }
    fn_0001c7c0();
    return result;
}

// FUNCTION: 0x000125b8
void fn_000125b8(void)
{
    if (D_0015831b & 1) {
        return;
    }
    D_00188c7b = D_00188c7c = 0;
    D_00188c7e = D_00188c7f = 0;
    if (D_00155f4d <= 1) {
        fn_0009ae08(1, D_00158100);
        D_00155e14 = 0;
    } else if (D_00155e16) {
        if (D_00155e16 == 3) {
            fn_00088ebc(10, 0xaf, -1, -1);
        }
        fn_0009ae08(3, D_00155e16);
        D_00155e16 = 0;
    } else {
        fn_0009ae08(2, D_0015f8f4);
        D_00155e15 = 0;
    }
}

// NONMATCHING: u8 local stack slot order and index/pointer register choice
// FUNCTION: 0x00012690
void fn_00012690(void)
{
    u8 i;
    u8 lowFee;
    u8 noStaff;
    u8 unavailable;
    u8 unused18;
    u8 unused1c;
    u8 unused20;
    u8 unused24;
    u8 unused28;
    u8 count;
    u8 unused30;
    u16 value;
    s16 offset;
    u16 room;
    int n;
    int result;
    RoomDef *def;
    u8 list[18];
    char buf[100];

    n = 0;
    unused28 = 0;
    unused24 = 0;
    unused20 = 0;
    unused1c = 0;
    unused18 = 0;
    noStaff = 0;
    unavailable = 0;
    lowFee = 0;
    unused30 = 0;
    fn_00099d00(0, 0, 0xe);
    fn_000c84f0(0, 0, D_00188c34, D_00188c38);
    fn_000be948(0, 0, D_00188c34, D_00188c38);
    D_00155c6e = 0;
    switch (D_0015814e) {
    case 0:
        fn_000b7a1c();
        fn_000b7648(2, 1, 0x1b, 0, 13);
        D_0015814e = 2;
        D_0015814c = 0;
        D_00155da9 = 0x100;
        D_00155e30 = 0;
        break;
    case 1:
        D_001549d0[D_00154b58].type = 0x70;
        D_001549d0[D_00154b58].value = 0;
        break;
    case 2:
        fn_0001b388(D_00153f3c);
        break;
    default:
        break;
    }
    fn_000b714c(0xe);
    offset = D_001897d0 ? D_001897d0->unknownd1 : 0;
    fn_000c84f0(D_0015f430->unknown0, D_0015f430->unknown4, D_0015f430->unknown8, D_0015f430->unknownC);
    D_00188c74 = 0x100;
    room = D_00155444;
    def = &D_0012d900[room];
    result = fn_00058e34(room, D_00154b58);
    for (i = 0; i + 0x11 != 0x1d; i++) {
        switch (i) {
        case 6:
            if (room >= 0x24 || room == 1) {
                fn_000bc13a(buf, D_0012017c, D_001547bc->unknownc7e);
            } else {
                fn_000bc13a(buf, D_0012017f, D_001547bc->rooms[room].unknown0);
            }
            break;
        case 7:
            if (D_00155e30) {
                D_00155e30--;
                fn_000bc13a(buf, D_00120182, D_001547bc->rooms[room].unknown10 / 100);
            } else {
                fn_000bc13a(buf, D_00120187, D_001547bc->rooms[room].unknownC);
            }
            break;
        case 8:
            fn_000bc13a(buf, D_0012018b, D_001547bc->rooms[room].unknown14);
            break;
        case 9:
            fn_000bc13a(buf, D_0012018f, D_001547bc->rooms[room].unknown1E);
            break;
        case 10:
            fn_000bc13a(buf, D_00120192, D_001547bc->rooms[room].unknown20);
            break;
        case 11:
            fn_000bc13a(buf, D_00120195, D_001547bc->rooms[room].unknown24);
            break;
        default:
            fn_000bc13a(buf, D_00120198, D_00160304[i]);
            fn_0001b7e0(buf, buf, 1);
            break;
        }
        fn_000c889c(D_0015f434[i + 0x11].x, D_0015f434[i + 0x11].y, buf);
    }
    fn_000c84f0(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    for (i = 0; i < 0x12; i++) {
        list[i] = 0;
    }
    n = 0;
    list[8] = D_00155444;
    count = 0;
    if (D_00155444 + 1 < 0x2e) {
        for (i = D_00155444 + 1; i < 0x2e; i++) {
            if ((i < 0x24 && D_001547bc->rooms[i].unknown1D)
                || (i >= 0x24 && D_001547b4[D_00154b58].rooms[i].unknown1C == 1)
                || (i == 0x2c && (D_001547b4[D_00154b58].unknownc28 != 0 && D_00129e27 != 0)
                    && D_00154820[D_00154b58].unknownd4)) {
                if (count >= 7) {
                    break;
                }
                list[count + 9] = i;
                n++;
                count++;
            }
        }
    }
    if (D_00155444 - 1 > 0) {
        n = 7;
        count = 7;
        for (i = D_00155444 - 1; i > 0; i--) {
            if (((i < 0x24 && D_001547bc->rooms[i].unknown1D)
                 || (i >= 0x24 && D_001547b4[D_00154b58].rooms[i].unknown1C == 1))
                && i < 0x2c) {
                if (count == 0) {
                    break;
                }
                list[count] = i;
                n++;
                count--;
            }
        }
    }
    count = 0;
    n = 0;
    for (i = 0; i < 0x11; i++) {
        if (list[i]) {
            fn_000bc13a(buf, D_0012019b, D_001602a4[list[i]]);
            fn_0001b7e0(buf, buf, 0);
            if (i == 8) {
                fn_000b714c(0xf);
            } else {
                fn_000b714c(0xe);
                if (D_00188c7b) {
                    if (fn_0001b4e0(D_00188c5c, D_00188c60, D_0015f434[i].x, D_0015f434[i].y,
                                    D_00188c34 - D_0015f434[i].y, offset)) {
                        D_00155444 = list[i];
                        fn_00088ebc(10, 0xa8, -1, -1);
                        D_00188c7b = 0;
                    }
                }
            }
            fn_000c889c(D_0015f434[i].x, D_0015f434[i].y, buf);
        }
    }
    fn_000c84f0(0, 0, D_00188c34, D_00188c38);
    fn_000be948(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    fn_000b714c(0xe);
    D_00188c74 = 0x100;
    fn_000c84f0(D_0015f434[0x1e].x, D_0015f434[0x1e].y, 0x36, 0x1a);
    fn_000c889c(0, (0x1a - (D_001897d0 ? D_001897d0->unknownd1 : 0)) / 2, D_00160304[7]);
    D_00188c74 = 0;
    fn_000c84f0(0, 0, D_00188c34, D_00188c38);
    if (!(room >= 0x24 || room == 1)) {
        if (D_001547b4[D_00154b58].rooms[room].unknown1C == 1) {
            if (def->kind == 0x27) {
                value = D_001547b4[D_00154b58].rooms[room].unknown1B;
                fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x2a);
                fn_000b714c(0x2e);
                fn_000bc13a(buf, D_0012019e, value);
                fn_000c84f0(0x134, D_0015f434[0x21].y, 0x1c, 0x32);
                D_00188c74 = 0x100;
                fn_000c889c(0, 0, buf);
                if (value < 100) {
                    lowFee = 1;
                }
                fn_000c84f0(0, 0, D_00188c34, D_00188c38);
                D_00188c74 = 0;
            } else if (def->category == 10) {
                fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x30);
            } else if (def->unknown8 != 1 && def->unknown8 != 5 && def->unknown8 != 4) {
                fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x24);
            }
            fn_000fe040(D_0015f434[0x20].x, D_0015f434[0x20].y, D_00153f14 + 0x42);
        } else {
            fn_000fe040(D_0015f434[0x20].x, D_0015f434[0x20].y, D_00153f14 + 0x48);
            if (def->unknown8 != 1 && def->unknown8 != 5 && def->unknown8 != 4) {
                if (def->kind == 0x27) {
                    fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x2a);
                    fn_000b714c(0x2e);
                    fn_000bc13a(buf, D_001201a1);
                    fn_000c84f0(0x134, D_0015f434[0x21].y, 0x1c, 0x32);
                    D_00188c74 = 0x100;
                    fn_000c889c(0, 0, buf);
                    unavailable = 1;
                    fn_000c84f0(0, 0, D_00188c34, D_00188c38);
                    D_00188c74 = 0;
                } else if (D_001547b4[D_00154b58].equipment[def->kind].unknown0 == 1
                           && D_00129634[def->kind].unknown0 == 1) {
                    fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x24);
                    if (D_00154820[D_00154b58].unknown9c[def->category] == 0) {
                        noStaff = 1;
                    }
                    if (def->unknown0 == 0x40) {
                        fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x3c);
                    }
                    if (def->category == 10) {
                        fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x30);
                    }
                } else {
                    fn_000fe040(D_0015f434[0x1f].x, D_0015f434[0x1f].y, D_00153f14 + 0x36);
                    unavailable = 1;
                }
            }
        }
    } else if (room >= 0x24) {
        if (D_001547b4[D_00154b58].rooms[room].unknown1C == 1) {
            fn_000fe040(D_0015f434[0x20].x, D_0015f434[0x20].y, D_00153f14 + 0x42);
        } else {
            fn_000fe040(D_0015f434[0x20].x, D_0015f434[0x20].y, D_00153f14 + 0x48);
        }
    }
    if (D_0012d900[room].unknown5[0] != 0) {
        if (room == (D_00155f4d > 1 ? D_001547b4[D_00154b58].unknown1256 : D_00155376)) {
            fn_000b714c(0xf);
        } else {
            fn_000b714c(0xe);
        }
        fn_0001b7e0(buf, D_00160304[6], 1);
        fn_000c889c(D_0015f434[0x1d].x, D_0015f434[0x1d].y, buf);
    }
    fn_00013540();
    fn_00013a7c();
    D_00188c74 = 0;
    if (fn_00052bcc()) {
        D_0015814e = 1;
    }
    fn_000b714c(0);
}

// NONMATCHING: index/pointer register choice (pointer in edx in target)
// FUNCTION: 0x00013540
void fn_00013540(void)
{
    u8 i;

    if (D_00188c7b) {
        D_0015814c = 0;
        for (i = 0; D_0015f43c[i + 0x12].width != 0 && D_0015f43c[i + 0x12].height != 0; i++) {
            if (fn_0001b4e0(D_00188c5c, D_00188c60, D_0015f43c[i + 0x12].x, D_0015f43c[i + 0x12].y,
                            (s16)D_0015f43c[i + 0x12].width, (s16)D_0015f43c[i + 0x12].height)) {
                D_0015f43c[i + 0x12].callback(i);
                if (D_0015f43c[i + 0x12].sprite) {
                    fn_000fe040(D_0015f43c[i + 0x12].unknownC, D_0015f43c[i + 0x12].unknownE,
                                D_00153f14 + D_0015f43c[i + 0x12].sprite * 6);
                }
                break;
            }
        }
        D_00155da9 = 0x100;
        D_00155dab = D_00155c5a;
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0x12].width != 0 && D_0015f43c[i + 0x12].height != 0; i++) {
            if (fn_0001b4e0((s16)D_00188c64, (s16)D_00188c68, D_0015f43c[i + 0x12].x, D_0015f43c[i + 0x12].y,
                            (s16)D_0015f43c[i + 0x12].width, (s16)D_0015f43c[i + 0x12].height)) {
                if (i == 0 || i == 1) {
                    D_0015f43c[i + 0x12].callback(i);
                }
                if (D_0015f43c[i + 0x12].sprite) {
                    fn_000fe040(D_0015f43c[i + 0x12].unknownC, D_0015f43c[i + 0x12].unknownE,
                                D_00153f14 + D_0015f43c[i + 0x12].sprite * 6);
                }
                break;
            }
        }
    }
    D_00188c7b = 0;
}

// NONMATCHING: index/pointer register choice (value in eax, address in edx in target)
// FUNCTION: 0x0001387c
void fn_0001387c(void)
{
    if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
        D_001549d0[D_00154b58].type = 0x6e;
        D_001549d0[D_00154b58].value = D_00155da9 >> 8;
        D_001549d0[D_00154b58].room = D_00155444;
        if (D_00155da9 < 0xa00) {
            D_00155da9 += 0x10;
        } else {
            D_00155da9 = 0xa00;
        }
        if (D_001547bc->rooms[D_00155444].unknown10 < 20000) {
            fn_00088ebc(10, 0x43, -1, -1);
        } else if (!D_0015814c) {
            fn_00088ebc(10, 8, -1, -1);
            D_0015814c = 1;
        }
    }
}

// NONMATCHING: index/pointer register choice (value in eax, address in edx in target)
// FUNCTION: 0x0001397c
void fn_0001397c(void)
{
    if (D_00155c5a - D_00155dab > 0xfa || D_00155da9 == 0x100) {
        D_001549d0[D_00154b58].type = 0x6f;
        D_001549d0[D_00154b58].value = D_00155da9 >> 8;
        D_001549d0[D_00154b58].room = D_00155444;
        if (D_00155da9 < 0xa00) {
            D_00155da9 += 0x10;
        } else {
            D_00155da9 = 0xa00;
        }
        if (D_001547bc->rooms[D_00155444].unknown10 > 5000) {
            fn_00088ebc(10, 0x43, -1, -1);
        } else if (!D_0015814c) {
            fn_00088ebc(10, 8, -1, -1);
            D_0015814c = 1;
        }
    }
}

}
