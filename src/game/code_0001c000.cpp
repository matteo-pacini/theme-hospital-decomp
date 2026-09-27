#include "common.h"

struct Struct_0015f754 {
    u8 unknown0[0x62];
    s16 x;
    s16 y;
};

struct Struct_0015f410 {
    u8 unknown0[0x570];
    int f570;
    u8 unknown574[0x10];
    int f584;
    u8 unknown588[0xc];
    int f594;
};

struct Struct_001602a0 {
    u8 unknown0[0x6c];
    int f6c;
};

struct MemRegion {
    u32 addr;
    u32 size;
};

struct Event {
    u8 unknown0;
    u8 type;
    u16 value;
    u8 flags;
    union { u16 arg; u8 argb[2]; };
    u32 checksum;
    u8 unknownB[4];
};

struct Dept {
    int value;
    int level;
    u8 unknown8[0x10];
    u16 count;
    u8 unknown1a[0xc];
};

struct Player {
    u8 unknown0[0x2b];
    u8 f2b;
    u8 unknown2c[0x9];
    int f35;
    u8 unknown39[0x4];
    int f3d;
    u8 unknown41[0x198];
    u16 f1d9;
    u8 unknown1db[0x7];
    u8 f1e2;
    u8 unknown1e3[0x2];
    u8 f1e5;
    u8 unknown1e6[0xd];
    int f1f3;
    u8 unknown1f7[0x3];
    u8 f1fa;
    u8 unknown1fb[0x8];
    u16 f203;
    u16 f205;
    u16 f207;
    u8 unknown209[0x2];
    u8 f20b;
    u8 f20c;
    u8 f20d;
    u8 f20e;
    u8 unknown20f[0x8];
    u16 f217;
    u16 f219;
    u16 f21b;
    u16 f21d;
    u8 f21f;
    u8 unknown220[0xc];
    Dept dept[1];
    u8 unknown252[0xa42];
    u8 fc94;
    u8 fc95;
    u8 unknownc96[0x91];
    s16 fd27;
    u16 fd29;
    u8 unknownd2b[0x242];
    u8 ff6d[1];
    u8 unknownf6e[0x249];
    u16 f11b7;
    u8 unknown11b9[0x3];
    u8 f11bc;
    u8 unknown11bd[0x37];
    u16 f11f4;
    u16 f11f6;
    u8 unknown11f8[0x5e];
    u8 f1256;
    u16 f1257;
};

struct Object {
    u8 unknown0[0x5];
    u8 f5;
    u8 unknown6[0x1];
    u8 f7;
    u8 unknown8[0x4];
    u8 fc;
    u8 fd;
    u8 fe;
    u8 unknownf[0x7];
    u8 f16;
    u8 unknown17[0x20];
    int f37;
    union { int f3b; u16 f3b_w; };
    u8 unknown3f[0x12];
    u16 f51;
    u8 unknown53[0x8];
    u16 f5b;
    u8 unknown5d[0xf];
    int f6c;
    u8 unknown70[0x4];
    u8 f74;
    u8 f75;
    u8 f76;
    u8 unknown77[0x9];
    u16 f80;
    u8 unknown82[0xc];
    u8 f8e;
    u8 f8f;
    u8 unknown90[0x3];
    u8 f93;
    u8 unknown94[0x14];
    u8 fa8;
    u8 unknowna9[0x2];
    u8 fab;
    u8 unknownac[0x2];
    u8 fae;
};

struct Item {
    u8 f0;
    u8 unknown1[0x2];
    u8 f3;
    u8 unknown4[0x4];
    u8 f8;
    u8 unknown9[0x15];
    u8 f1e;
    u8 unknown1f[0x3];
    u8 f22;
};

struct Slot {
    u8 kind;
    u8 unknown1[0x1];
    u16 value;
    u16 f4;
    u8 f6;
    u8 unknown7[0x4];
};

struct SlotPos {
    s16 x;
    s16 y;
    u8 unknown4[0x1e];
};

struct Rect {
    int x0;
    int y0;
    int x1;
    int y1;
};

struct Link {
    int f0;
    u8 unknown4[0x20];
};

struct Record {
    u8 unknown0[0x4];
    u16 f4;
};

struct Stats {
    u8 unknown0[0x28024];
    u16 f28024[4];
};

struct Tile {
    u8 unknown0[0x7];
    u8 f7;
};

struct Stride25 {
    u16 f0;
    u8 unknown2[0x23];
};

struct Stride6c {
    u16 f0;
    u8 unknown2[0x6a];
};

struct Stride32 {
    u8 f0;
    u8 unknown1[0x31];
};

struct Blob63 {
    u8 unknown0[0x3f];
};

struct Panel {
    u8 unknown0[0x64f];
    u8 f64f;
    u8 f650;
    u8 unknown651[0x1];
};

extern "C" {

extern Event D_001549d0[];
extern u32 D_00154b58;
u16 fn_00021818(s8 a, s8 b);
s16 fn_00021c38(s8 a, s8 b);

extern MemRegion D_001290d4[];
extern u32 D_00160288;
extern u16 D_0015fe86[];
extern u32 D_0015fe78, D_0015fe7c, D_0015fe80;
extern u32 D_0016028c, D_00160290;
extern u8 D_00160508;

s8 fn_000caac0(int port, void *buf, int len);
int fn_000cade4(int port, void *buf, int len);

extern int D_00188c18;
extern u16 D_001549ae;
extern Struct_001602a0 *D_001602a0;
extern Struct_0015f754 *D_0015f754;
extern Struct_0015f410 *D_0015f410;
extern int D_00188c2c, D_00188c30;
extern char D_00120260[];

int fn_000bc13a(char *buf, char *fmt, ...);
int fn_000bed50(void);
void fn_000bed5c(void);
void fn_00033a40(void);
void fn_000c84f0(int x, int y, int w, int h);
void fn_000b714c(int n);
void fn_000c889c(int a, int b, char *s);
void fn_000ab49c(int a, int b, int c, int d, u8 e);
void fn_000ab120(void);


extern u8 D_001581ee;
extern int D_001581e8;
extern u8 D_001549cc[];
extern u16 D_0015536e;
extern u8 D_00155240[];
extern u32 D_00154898;
extern char D_00156a3a[];
extern char D_0012026a[];
extern u8 D_00156b02;
extern int D_00188c64, D_00188c68;
extern u8 D_0012e37c;
extern u8 D_00156438, D_00156439, D_0015643a, D_0015643b;
extern Stats *D_00158330;
extern u8 D_001567a7, D_001567a8;
extern u8 D_001553c7, D_001553c8;
extern Event D_00154a0c[];
extern u8 D_00155f4d;
extern char D_0012027f[], D_0012028f[], D_001202a4[], D_001202aa[];
extern u32 D_001549c0;
extern Link D_001609c4[];
extern u8 D_00160294;
extern Player *D_001547b4;
extern Object *D_001549a4;
extern Item *D_001547a4;
extern u16 D_00158198, D_0015819c, D_0015819e;
extern u8 D_0015819a, D_001581a0, D_001581a1, D_001581a2;
extern u8 D_00155c69;
extern u16 D_0012ddae;
extern u8 D_00155dc6[];
extern u16 D_001552b5, D_001552b7, D_001552b9;
extern u8 D_00188c7b;
extern u8 D_00155e07;
extern u16 D_001549ac, D_001549b0;
extern u8 D_00156a06;
extern u8 D_001581ef, D_001581f0;
extern char D_001563c5[];
extern u8 D_00155e14;
extern u16 D_00158184;
extern u8 D_00155261;
extern u16 D_00155e21;
extern u8 D_001547d5, D_00155f35, D_00155d71;
extern u8 D_001553e9;
extern u32 D_00153f34;
extern u8 D_00156910;
extern u32 D_001553d7;
extern u8 D_00158146, D_0015814e, D_00158152, D_00158156, D_0015815e;
extern u8 D_00158162, D_00158166, D_0015816a, D_0015816e, D_0015817a;
extern u8 D_0015817e, D_00158182, D_00158186;
extern u16 D_00155419, D_0015541b, D_0015541d;
extern u8 D_00155284;
extern u8 D_00155265, D_0015525e, D_00155266;
extern u16 D_0015526f;
extern Slot D_001581b2[];
extern u8 D_00155d70;
extern SlotPos *D_0015f414;
extern Rect *D_0015f40c;
extern u8 D_001553ec[];
extern u8 D_00155d7d;

void fn_000be458(int fh, void *buf, int len);
void fn_000be490(int fh, void *buf, int len);
void fn_000bbff0(char *dst, char *src);
void fn_000bd7dc(int w, int h);
void fn_000abbc8(int a, int b, int c);
u32 fn_00022790(void);
u8 fn_00086b60(void);
void fn_000c0f68(void *buf, int len);
void fn_0008debc(char *s);
s8 fn_000c0b74(void *p);
void fn_000bf7a0(int a);
void fn_0009b1f0(void);
void fn_00086834(int a);
void fn_00086890(int a);
void fn_000228e0(void);
void fn_00027b44(u8 a);
void fn_00046514(u8 a, int b, int c);
u8 fn_00046b54(u8 a);
void fn_00088ebc(int a, int b, int c, int d);
void fn_00046e50(u8 a);
void fn_0001a374(int a);
void fn_00054fc0(u16 a);
void fn_0002341c(u8 a, u8 b);
void fn_00047cf8(u8 a);
u16 fn_00029c4c(int a, int b, int c, u8 d);
void fn_00050ef0(int a, int b, int c, u16 d);
u16 fn_00037098(int a, u16 b, int c, u8 d);
void fn_000247c8(void);
void fn_0004b7e0(int a, u16 b);
void fn_0001afd8(void);
u16 fn_00018710(int a, int b, u8 c);
void fn_0006bda8(Object *o);
u16 fn_00018838(int a, int b, u8 c);
void fn_000184f4(int a);
void fn_000a410c(u8 a, int b, s16 c, int d);
void fn_00038870(int a, u16 b, int c);
void fn_0006b1bc(int a);
void fn_0006fdfc(int a);
void fn_0008bd24(void);
void fn_0001add0(s16 a);
void fn_00022b90(int a, u8 b);
void fn_00055164(int a);
void fn_000464bc(u8 a, u8 b);
void fn_000550cc(u16 a);
void fn_0001bdb8(int a);
void fn_0009ff68(void);
void fn_0009fcbc(int a);
void fn_000485e0(u8 a, u8 b);
void fn_000c02fc(void);
void fn_0008ee10(void);
void fn_0007fbb8(int a);
void fn_000be3bc(int fh);
int fn_000be2e0(char *name, int mode);
Record *fn_00019fe0(int a, int b, u8 c);
void fn_000abb80(int a, u16 b);
void fn_000c8320(void);
int fn_000c24b0(void);
void fn_000c5910(int a, ...);
void fn_000c5990(int a, ...);
void fn_00087b4c(void);
void fn_0001aa8c(int a);
void fn_000552e8(int a, u16 b);
u8 fn_00027af0(void);
void fn_000275c8(u8 a, u8 b, u16 c, int d);
void fn_0001bc48(int a, int b, int c);
void fn_0001b3e8(int a);
void fn_0006aab0(void);
void fn_0001bc94(int a, int b);
u16 fn_0006aae0(int a, u16 b, u16 c);
void fn_0006b174(int a);
void fn_0001438c(void);
void fn_00052f7c(u8 a);
void fn_00064db0(int a);
void fn_00059c38(int a, u8 b);
void fn_00054f98(void);
void fn_0004ace0(int a, u16 b);
void fn_0004ae60(int a, u16 b);
void fn_0002812c(u8 a);
void fn_000ac78c(int a, int b, int c, int d, int e, int f, int g);
void fn_000736d4(int a);
void fn_00027e1c(int a, u8 b);
void fn_0002a804(Object *o, int b);
void fn_00047308(u8 a);
int fn_0002b998(Object *o, u16 b, int c, int d, u8 e);
void fn_00027920(Item *p);
extern u8 D_0012e383;
extern u8 D_00155e09;
extern Stride32 D_00154b5c[];
extern u8 D_00155402;
extern u8 D_00155430;
extern Blob63 D_00155c6f, D_00155273;
extern u8 D_0015819b, D_00154690;
extern Panel *D_001547c0;
extern u8 D_0012b7c8[];
extern u32 D_0015f8e9, D_0015f8ed;
extern u8 D_001548b5;
extern u8 D_00158324;
extern u16 D_00155376;
extern u8 D_00155e1d;
extern u8 D_00155d7b, D_00155d7c;
extern u16 D_00155d69;
extern u16 D_0015543a;
extern int D_001549c4;
extern Stride6c D_0015f5c2[];
extern Object *D_001549a8;
extern Stride25 D_00129636[];
extern Tile *D_00153f78;
extern u32 D_0015542c;
extern u8 D_00155e4d;
extern u8 D_00158100;
extern u16 D_0015542a;
extern u8 D_0015818e;
extern u8 D_00188c7c, D_00188c81, D_00188c83;
extern u16 D_001553fd;
extern u16 D_00155d57;
extern u8 D_00155e4c;

void fn_000b73a0(u8 a, int b, int c);
void fn_00085ce8(u8 a);
void fn_00085f3c(u8 a);
void fn_00085e50(u8 a, u8 b);
void fn_000b2c74(u8 a, u8 b, u8 c);
void fn_0004b654(int a, u16 b);
void fn_0004b6b0(int a, u16 b);
void fn_0002c700(int a, u16 b);
void fn_000688c0(void);
void fn_00068480(u8 a);
void fn_0002cb2c(Object *o, int b, int c, u8 d);
void fn_00054e50(Item *p);
void fn_0009ffa8(u8 a);
void fn_0009d114(int a, u16 b);
void fn_00017d78(Object *o, u8 b);
u8 fn_0002d11c(Object *o);
void fn_0002c3bc(Object *o);
void fn_0002becc(Object *o);
void fn_00075484(u16 a, int b);
void fn_0009d480(int a, int b, u16 c, int d, u8 e);
void fn_0007a710(u16 a, s8 b, int c);
void fn_000199e8(Object *o);
u32 fn_000b0100(void);
void fn_000b0428(Object *o, int b);
int fn_00058e34(u16 a, u8 b);
void fn_00087c44(int a, u8 b);
void fn_0001665c(Object *o);
void fn_0007baac(u8 *p, int b, int c);
void fn_000b0300(Object *o, int b);
void fn_000483c4(u8 a);
void fn_0002994c(u8 a);
void fn_000a04e4(u8 a, int b, u8 c);
void fn_00058370(int a, int b, int c, u8 d, int e);
void fn_00018620(int a);
void fn_000af510(int a, u8 b);
void fn_000427d8(Object *o);
void fn_0003e330(Object *o);
void fn_0003c8d8(Object *o, int b);
void fn_000a4208(void);
void fn_00075010(Object *o);
void fn_0009e0d8(void);
void fn_000987c8(Object *o);
void fn_00098770(Object *o);
void fn_00027a90(void);
void fn_000194a0(Object *o);
void fn_0003a3e0(Object *o);
void fn_00060b54(int a, u16 b);
void fn_00022a58(void);
// FUNCTION: 0x0001c030
int fn_0001c030(int n)
{
    if (!(n >= 0x11 || n == 0xc || n == 0xd || n == 7 || n == 9)) {
        return 1;
    }
    return 0;
}

// NONMATCHING: register choice for two call arguments (history-dependent allocator)
// FUNCTION: 0x0001c084
void fn_0001c084(int n)
{
    u8 flag = D_00188c18 != 0;

    if (D_001549ae == 0) {
        char buf[32];

        if (!flag) {
            while (fn_000bed50() != 1) {
            }
        }
        fn_000bc13a(buf, D_00120260, D_001602a0->f6c, n);
        fn_00033a40();
        fn_000c84f0(D_0015f754->x, D_0015f754->y, D_00188c2c - D_0015f754->x, D_00188c30 - D_0015f754->y);
        fn_000b714c(0x2c);
        fn_000c889c(D_0015f410->f570, D_0015f410->f584 + D_0015f410->f594, buf);
        fn_000ab49c(0, D_00188c30 - 0x5c, D_00188c2c, D_00188c30, 3);
        fn_000bed5c();
        fn_000ab120();
        if (flag) {
            while (fn_000bed50() != 1) {
            }
        }
    }
}

// FUNCTION: 0x0001c1b4
u16 fn_0001c1b4(u16 *p, int len)
{
    u32 sum = 0;

    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    if (len > 0) {
        sum += *(u8 *)p;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ~sum;
}

// NONMATCHING: stored checksum ends up in dx with the index in eax, the reverse of the original
// FUNCTION: 0x0001c234
void fn_0001c234(void)
{
    int i;
    u32 p;
    u32 base;
    u32 chunk;
    u32 size;

    D_00160288 = 0;
    for (i = 0; D_001290d4[i].addr != 0; i++) {
        if (fn_0001c030(i)) {
            p = base = D_001290d4[i].addr;
            size = D_001290d4[i].size;
            while (base + size > p) {
                chunk = 0x400;
                if (p + chunk > base + size) {
                    chunk = base + size - p;
                }
                if (D_00160288 < 0x200) {
                    D_0015fe86[D_00160288++] = fn_0001c1b4((u16 *)p, chunk);
                }
                p += chunk;
            }
        }
    }
    D_00160288 <<= 1;
}

// NONMATCHING: checksum index scaled with add instead of lea (register allocation)
// FUNCTION: 0x0001c318
int fn_0001c318(s8 port, u32 base, int len)
{
    int tries;
    int chunk;
    int off;

    if (base == D_001290d4[0].addr) {
        D_0015fe78 = D_0015fe80 = 0;
    }
    if (port < 0) {
        off = 0;
        while (off < len) {
            chunk = 0x400;
            if (off + chunk > len) {
                chunk = len - off;
            }
            if (D_0015fe86[D_0015fe78++] != fn_0001c1b4((u16 *)(base + off), chunk)) {
                tries = 0;
                do {
                } while (fn_000caac0(D_00160508, &off, 4) == -1 && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                tries = 0;
                do {
                } while (fn_000cade4(D_00160508, (void *)(base + off), chunk) != chunk && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                if ((D_0015fe7c++ & 0xf) == 0) {
                    fn_0001c084((D_00160290 + chunk) * 100 / D_0016028c);
                }
            }
            off += chunk;
            D_00160290 += chunk;
        }
        tries = 0;
        off = -1;
        do {
        } while (fn_000caac0(D_00160508, &off, 4) == -1 && tries++ < 8);
        if (tries >= 8) {
            return 0;
        }
    } else {
        int end = 0;

        off = 0;
        while (off >= 0) {
            tries = 0;
            do {
            } while (fn_000cade4(port, &off, 4) != 4 && tries++ < 8);
            if (tries >= 8) {
                return 0;
            }
            if (off >= 0) {
                chunk = 0x400;
                if (off + chunk > len) {
                    chunk = len - off;
                }
                tries = 0;
                do {
                } while (fn_000caac0(port, (void *)(base + off), chunk) == -1 && tries++ < 8);
                if (tries >= 8) {
                    return 0;
                }
                end = off + chunk;
            }
            if ((D_0015fe7c++ & 0xf) == 0) {
                fn_0001c084((D_00160290 + end) * 100 / D_0016028c);
            }
        }
        D_00160290 += len;
        fn_0001c084(D_00160290 * 100 / D_0016028c);
    }
    D_0015fe80++;
    return 1;
}

// NONMATCHING: eax/edx swapped when comparing two table entries (register allocation)
// FUNCTION: 0x0001c5e4
void fn_0001c5e4(void)
{
    int value;
    int i;
    int tries;

    if (D_00160508 == D_00154b58) {
        value = 1;
        for (i = 0; i < 4; i++) {
            if (D_001549d0[i].checksum != 0) {
                if (D_001549d0[D_00154b58].checksum != D_001549d0[i].checksum) {
                    fn_0001c084(0);
                    D_00160290 = 0;
                    tries = 0;
                    do {
                    } while (fn_000caac0(i, &value, 4) == -1 && tries++ < 8);
                    if (tries >= 8) {
                        return;
                    }
                    if (!fn_00021818(0, i)) {
                        return;
                    }
                    fn_0001c084(100);
                }
            }
        }
        for (i = 0; i < 4; i++) {
            if (i == D_00154b58 || D_001549d0[i].checksum == 0) {
                continue;
            }
            tries = 0;
            do {
            } while (fn_000caac0(i, &value, 4) == -1 && tries++ < 8);
            if (tries >= 8) {
                return;
            }
        }
    } else {
        if (D_001549d0[D_00154b58].checksum != D_001549d0[D_00160508].checksum) {
            fn_0001c084(0);
            D_00160290 = 0;
            value = 0;
            while (value == 0) {
                fn_000cade4(D_00160508, &value, 4);
            }
            fn_00021c38(0, 1);
            fn_0001c084(100);
        }
        value = 0;
        while (value == 0) {
            fn_000cade4(D_00160508, &value, 4);
        }
    }
}


// NONMATCHING: eax/edx/ebx choice differs in many indexed loads and stores
// FUNCTION: 0x0001c7c0
void fn_0001c7c0(void)
{
    char buf[100];
    Object *obj2;
    int w;
    Object *obj1;
    int h;
    u16 j;
    u16 i;
    u8 changed;

    if (D_001581ee & 2) {
        u32 version;

        fn_000be458(D_001581e8, D_001549cc, 0xf);
        fn_000be458(D_001581e8, &w, 4);
        fn_000be458(D_001581e8, &h, 4);
        fn_000be458(D_001581e8, &D_0015536e, 2);
        fn_000be458(D_001581e8, D_00155240, 0x178);
        fn_000be458(D_001581e8, &version, 4);
        if (D_00154898 != version) {
            fn_000bbff0(D_00156a3a, D_0012026a);
            D_00156b02 = 200;
        }
        fn_000bd7dc(w, h);
    } else if (D_001581ee & 1) {
        w = D_00188c64;
        h = D_00188c68;
        fn_000be490(D_001581e8, D_001549cc, 0xf);
        fn_000be490(D_001581e8, &w, 4);
        fn_000be490(D_001581e8, &h, 4);
        fn_000be490(D_001581e8, &D_0015536e, 2);
        fn_000be490(D_001581e8, D_00155240, 0x178);
        fn_000be490(D_001581e8, &D_00154898, 4);
    }
    if (D_001549d0[D_00154b58].flags && !D_0012e37c) {
        if (D_001549d0[D_00154b58].flags & 1) {
            D_00156438 = 1;
        } else if (D_001549d0[D_00154b58].flags & 2) {
            D_00156439 = 1;
        }
        if (D_001549d0[D_00154b58].flags & 4) {
            D_0015643a = 1;
        } else if (D_001549d0[D_00154b58].flags & 8) {
            D_0015643b = 1;
        }
        if (D_001549d0[D_00154b58].flags & 0x10) {
            fn_000abbc8(D_001567a7, D_00158330->f28024[D_00154b58] % 128, D_00158330->f28024[D_00154b58] / 128);
            D_001553c8 = 1;
        }
        D_00154a0c[D_00154b58].flags = D_001549d0[D_00154b58].flags;
        D_001549d0[D_00154b58].flags = 0;
    }
    if (D_00155f4d > 1) {
        D_001549d0[D_00154b58].checksum = fn_00022790();
        fn_000bc13a(buf, D_0012027f, 0xf);
        if (!fn_00086b60()) {
            fn_000c0f68(D_001549cc, 0xf);
            fn_0008debc(D_0012028f);
            if (fn_000c0b74(D_001549cc + D_00154b58 * 0xf) == -1) {
            }
            fn_0008debc(D_001202a4);
            if (D_001549c0 == 4) {
                fn_000bf7a0(0);
            }
            changed = 0;
            for (j = 0; j < 4; j++) {
                if (D_001609c4[j].f0 && D_001549d0[j].checksum) {
                    if (D_001549d0[D_00160508].checksum != D_001549d0[j].checksum) {
                        if (D_001549d0[j].type != 0x8e && D_001549d0[j].type != 0x8d) {
                            D_001549d0[j].type = 0;
                            D_001549d0[j].value = 0;
                            D_001549d0[j].arg = 0;
                        }
                        changed = 1;
                    }
                }
            }
            if (changed) {
                fn_0009b1f0();
                D_00160294 = 1;
                fn_0001c5e4();
                D_00160294 = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            u8 handled = 0;

            switch (D_001549d0[i].type) {
            case 0x8d:
                fn_00086834(i);
                handled = 1;
                break;
            case 0x8e:
                fn_00086890(i);
                handled = 1;
                break;
            }
            if (handled) {
                D_001549d0[i].type = 0;
                D_001549d0[i].value = 0;
                D_001549d0[i].arg = 0;
                return;
            }
        }
    }
    for (i = 0; i < 4; i++) {
        D_001553c7 = i;
        if (D_001547b4[i].f2b != 2) {
            switch (D_001549d0[i].type) {
            case 0x1c:
            case 0x7e:
                goto dispatch;
            }
            continue;
        }
    dispatch:
        switch (D_001549d0[i].type) {
        case 0:
            break;
        case 1:
            fn_00027b44(i);
            break;
        case 2:
            fn_00046514(i, 1, D_001549d0[i].value);
            break;
        case 0x12:
            if (fn_00046b54(i)) {
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0x41, -1, -1);
                }
            } else {
                if (i == D_00154b58) {
                    fn_00088ebc(10, 8, -1, -1);
                }
            }
            break;
        case 0x13:
            fn_00046e50(i);
            if (i == D_00154b58) {
                fn_0001a374(1);
            }
            break;
        case 0x14:
            if (i == D_00154b58) {
                D_0015819c = 1;
                fn_00054fc0(3);
            }
            break;
        case 3:
            fn_0002341c(i, (u8)D_001549d0[i].value);
            break;
        case 4:
            if (i == D_00154b58) {
                fn_0001a374(0x10);
            }
            fn_00047cf8(i);
            break;
        case 5: {
            u8 empty = !D_001547b4[i].f11f4;

            if (fn_00029c4c(D_001549d0[i].value, D_001549d0[i].arg, 0, i)) {
                if (empty) {
                    fn_00050ef0(2, -(D_001549a4[D_001549d0[i].arg].f6c * 12) / 10, 0, D_001553c7);
                }
                if (i == D_00154b58) {
                    fn_00088ebc(10, 7, -1, -1);
                    fn_0001a374(0);
                }
            }
            break;
        }
        case 7:
            if (fn_00037098(D_001549d0[i].value, D_001549d0[i].argb[0], D_001549d0[i].argb[1], i) != 0) {
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0x46, -1, -1);
                    fn_000247c8();
                }
            }
            break;
        case 8:
            fn_00046514(i, 0, D_001549d0[i].value);
            break;
        case 9:
            fn_0004b7e0(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0xa:
            if (i == D_00154b58) {
                fn_0001afd8();
            }
            switch (D_001549d0[i].value) {
            case 1:
                j = fn_00018710(1, D_001549d0[i].arg, i);
                if (j) {
                    if (i == D_00154b58 && D_00155c69) {
                        fn_0006bda8(&D_001549a4[j]);
                    }
                }
                break;
            case 2:
                if (fn_00018838(2, 0, i)) {
                    fn_000184f4(2);
                    if (i == D_00154b58) {
                        fn_000a410c(i, 2, 0, 0);
                    }
                }
                break;
            case 3:
                fn_00018710(3, 0, i);
                if (D_001547b4[i].f205) {
                    D_001547b4[i].f203 = D_001547b4[i].f205 * D_0012ddae;
                    fn_000a410c(i, 5, 0, 0);
                }
                break;
            case 4:
                j = fn_00018710(4, D_001549d0[i].arg, i);
                if (j) {
                    fn_000a410c(i, 7, j, 0);
                }
                break;
            case 5:
                j = fn_00018710(5, 0, i);
                switch (j) {
                case 1:
                    D_001547b4[i].f1e2 = 0;
                    if (i == D_00154b58) {
                        fn_000a410c(i, 3, 0, 400);
                    }
                    break;
                case 2:
                    D_001547b4[i].f1fa = 0;
                    if (i == D_00154b58) {
                        fn_000a410c(i, 9, 0, 400);
                    }
                    break;
                case 3:
                    D_001547b4[i].f20e = 0;
                    fn_000a410c(i, 6, D_001547b4[i].f20c, 400);
                    break;
                }
                break;
            case 6:
                j = fn_00018710(6, 0, i);
                fn_000a410c(i, 8, j, 0);
                break;
            case 7:
                j = fn_00018710(7, 0, i);
                fn_000a410c(i, 4, j, 0);
                break;
            case 8:
                j = fn_00018710(8, D_001549d0[i].arg, i);
                if (j) {
                    fn_000a410c(i, 10, j, 0);
                }
                break;
            }
            break;
        case 0xb:
            fn_00038870(D_001549d0[i].value, D_001549d0[i].arg, 0);
            if (i == D_00154b58) {
                fn_00088ebc(10, 0x46, -1, -1);
            }
            break;
        case 0xc:
            if (i == D_00154b58) {
                if (D_00155dc6[D_001549d0[i].value] == 1) {
                    if (D_001547b4[i].f11f4) {
                        fn_00029c4c(D_001547b4[i].f11f6, D_001547b4[i].f11f4, 1, i);
                    } else if (D_001552b5) {
                        fn_0006b1bc(D_001552b5);
                    }
                    fn_0006fdfc(0x63);
                    fn_0001a374(0);
                    D_00188c7b = 0;
                }
                fn_00021818((s8)D_001549d0[i].value, -1);
                fn_0008bd24();
            }
            break;
        case 0xd:
            if (i == D_00154b58) {
                fn_00021c38((s8)D_001549d0[i].value, -1);
            }
            break;
        case 0xe:
            fn_0001add0((s16)D_001549d0[i].value);
            break;
        case 0x17:
            break;
        case 0x18:
            break;
        case 0x1a:
            fn_00022b90(D_001549d0[i].value, i);
            break;
        case 0x20:
            if (i == D_00154b58) {
                D_0015819c = D_001549d0[i].value;
                D_00158198 = 4;
                fn_00055164(D_0015819c);
            }
            break;
        case 0x35:
            fn_000464bc(i, D_001549d0[i].value);
            if (i == D_00154b58) {
                D_001581a0 = D_001549d0[i].value;
                fn_00054fc0(D_001581a0);
                fn_000550cc(D_001581a0);
                fn_0001bdb8(D_0015819a);
                fn_0001a374(1);
                fn_0009ff68();
                fn_0009fcbc(0);
                D_0012e383 = 0;
            }
            fn_000485e0(i, D_001549d0[i].value);
            break;
        case 0x1c:
            if (i == D_00154b58) {
                if (D_0015819a == 4) {
                    fn_0001bdb8(4);
                    D_0012e383 = 0;
                }
            }
            if (D_00155f4d > 1) {
                fn_000c02fc();
                if (D_00155e07) {
                    fn_0008ee10();
                }
                D_00155f4d = 1;
                D_00154b58 = 0;
                fn_0007fbb8(-1);
            }
            D_001549ac = 1;
            if (D_00156a06 == 1) {
            }
            if (D_001581ee & 2) {
                if (D_001581f0) {
                    D_001549ac = 2;
                    for (;;) {
                        char name[50];

                        if (D_001581ef < 10) {
                            D_001581ef++;
                        } else {
                            D_001581ef = 1;
                        }
                        fn_000be3bc(D_001581e8);
                        fn_000bc13a(name, D_001202aa, D_001563c5, D_001581ef);
                        D_001581e8 = fn_000be2e0(name, 2);
                        if (D_001581e8 > 0) {
                            D_001581ee |= 2;
                            break;
                        }
                    }
                }
            }
            break;
        case 0x1e:
            if (i == D_00154b58) {
                if (fn_00021c38((s8)D_001549d0[i].value, -1) == 0) {
                    fn_000abb80(0, fn_00019fe0(1, 0x65, D_001553c7)->f4);
                    D_001553c8 = 1;
                }
            }
            break;
        case 0x1d:
            fn_000abbc8(D_001567a7, D_00158330->f28024[D_00154b58] % 128, D_00158330->f28024[D_00154b58] / 128);
            D_00155e14 = 1;
            D_001553c8 = 1;
            break;
        case 0x21:
            break;
        case 0x22:
            D_00158184--;
            if (D_001549ae != 8 || D_00158184 == 0) {
                D_00155261 = !D_00155261;
                if (i == D_00154b58) {
                    int handle;

                    if (D_00155261) {
                        fn_000c8320();
                        D_00155e21 = 0x32;
                        if (D_00155e07) {
                            handle = fn_000c24b0();
                            fn_000c5910(handle);
                            D_00155e07 = 0;
                        }
                    } else {
                        D_00155e21 = 0x32;
                        if (D_001547d5 && D_00155f35 && D_00155d71) {
                            handle = fn_000c24b0();
                            fn_000c5990(handle);
                            D_00155e07 = 1;
                        }
                    }
                }
            }
            break;
        case 0x26:
            if (i == D_00154b58) {
                fn_0001bdb8(D_0015819a);
                fn_00087b4c();
                fn_0001aa8c(D_001553e9);
                D_0012e383 = 0;
            }
            break;
        case 0x27:
            if (i == D_00154b58) {
                fn_0001bdb8(D_0015819a);
                fn_0001aa8c(D_001553e9);
                D_0012e383 = 0;
            }
            break;
        case 0x25:
            if (i == D_00154b58) {
                fn_000552e8(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case 0x2b:
            if (i == D_00154b58) {
                u16 k;

                fn_0001bdb8(D_0015819a);
                k = fn_00027af0();
                if (k) {
                    fn_000275c8(k, 2, 0, 0);
                }
                D_0012e383 = 0;
            }
            break;
        case 0x29:
            fn_0001bc48(0, 0x32, 1);
            break;
        case 0x2a:
            if (i == D_00154b58) {
                fn_0001bc48(D_00153f34, 0x32, 1);
            }
            break;
        case 0x2e:
            if (i == D_00154b58) {
                fn_0001b3e8(0xb);
                if (D_0015819a == 0x63) {
                    fn_0006aab0();
                    fn_00087b4c();
                    fn_0001bc94(D_0015819a, 1);
                    fn_0001aa8c(1);
                    D_0012e383 = 1;
                }
            }
            break;
        case 0x2f:
            if (i == D_00154b58) {
                D_00158198 = 2;
                D_0015819c = D_001549d0[i].value;
                D_0015819e = fn_0006aae0(1, 0, D_0015819c);
            }
            break;
        case 0x30:
            if (i == D_00154b58) {
                if (D_001549a4[D_001549d0[i].arg].fd != 0x44) {
                    fn_0001bdb8(D_0015819a);
                    D_001552b7 = D_001549d0[i].value;
                    D_001552b5 = D_001549d0[i].arg;
                    D_001552b9 = 0xffff;
                    fn_0001a374(3);
                    D_0012e383 = 0;
                }
            }
            fn_0006b174(D_001549d0[i].arg);
            break;
        case 0x32:
            if (i == D_00154b58) {
                D_0015819e = fn_0006aae0((u8)D_001549d0[i].value, D_0015819e, D_0015819c);
            }
            break;
        case 0x34:
            if (D_001549ae != 0) {
                D_001549ae = 0;
                D_001553c8 = 1;
                D_00156910 = 1;
                D_001553d7 = D_00153f34;
            }
            break;
        case 0x33:
            if (i == D_00154b58 && D_0015536e == 0xe) {
                fn_00088ebc(10, 8, -1, -1);
                break;
            }
            if (D_001547b4[i].f11f4) {
                fn_00029c4c(D_001547b4[i].f11f6, D_001547b4[i].f11f4, 1, i);
            }
            fn_0001438c();
            if (i == D_00154b58) {
                D_001549ae = 1;
                D_001549b0 = D_001549d0[i].value;
                switch (D_001549b0) {
                case 0:
                    D_00158152 = 0;
                    break;
                case 1:
                    D_0015814e = 0;
                    break;
                case 2:
                    D_00158156 = 0;
                    break;
                case 3:
                    D_0015815e = 0;
                    break;
                case 4:
                    D_0015817e = 0;
                    break;
                case 5:
                    D_0015817a = 0;
                    break;
                case 6:
                    fn_00052f7c(D_00154b58);
                    D_0015816e = 0;
                    break;
                case 7:
                    D_00158166 = 0;
                    fn_00052f7c(D_00154b58);
                    break;
                case 8:
                    D_00158162 = 0;
                    break;
                case 9:
                    D_0015816a = 0;
                    break;
                case 0xd:
                    D_00158186 = 0;
                    break;
                case 0xf:
                    D_00158182 = 0;
                    break;
                }
                D_001553c8 = 1;
                D_00156910 = 1;
            }
            break;
        case 0x36:
            if (D_00155f4d > 1) {
                D_00158146 = 1;
            }
            fn_00064db0(D_001549d0[i].value);
            fn_00059c38(3, i);
            break;
        case 0x37:
            if (i == D_00154b58) {
                fn_0001b3e8(9);
                if (D_0015819a == 0x63) {
                    D_0015819a = 9;
                    D_00158198 = 1;
                    fn_00087b4c();
                    fn_0001bc94(D_0015819a, 1);
                    D_0015819e = 0;
                    fn_0001aa8c(1);
                    D_001581a0 = 0;
                    D_001581a1 = 0;
                    fn_00054f98();
                    D_0012e383 = 1;
                }
            }
            break;
        case 0x3b:
            if (i == D_00154b58) {
                if (D_0015819a == 0x63 && &D_001547a4[D_001549d0[i].value] > D_001547a4) {
                    D_0015819a = 6;
                    D_00158198 = 0;
                    D_0015819c = 0;
                    D_0015819e = 0;
                    D_001581a0 = 0;
                    D_001581a1 = 0;
                    D_001581a2 = 0;
                    fn_00087b4c();
                    fn_0001bc94(D_0015819a, 1);
                    fn_0001aa8c(1);
                    D_0015541b = 0;
                    D_00155419 = D_001549d0[i].value;
                    D_0015541d = D_001549d0[i].arg;
                    D_0012e383 = 1;
                }
            }
            break;
        case 0x3c:
            fn_0004ace0(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x3d:
            fn_0004ae60(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x38:
            if (D_001547b4[i].f11f4) {
                fn_00029c4c(D_001547b4[i].f11f6, D_001547b4[i].f11f4, 1, i);
            } else if (D_001549d0[i].value) {
                fn_0006b1bc(D_001549d0[i].value);
            }
            fn_0002812c(i);
            if (i == D_00154b58) {
                if (!D_00155284) {
                    fn_0001bdb8(0);
                    fn_0001bdb8(0xb);
                    fn_0001bdb8(0xd);
                    fn_0001bdb8(6);
                    fn_0001bdb8(9);
                    fn_0001bdb8(0xa);
                    fn_0001bdb8(0xc);
                    fn_0001bdb8(5);
                    fn_00087b4c();
                    D_001552b5 = 0;
                    D_001552b7 = 0;
                    D_00155265 = 0;
                    D_0015525e = 0;
                    fn_0001a374(0);
                    D_0012e383 = 0;
                } else {
                    fn_000275c8(0x11, D_00155266, D_0015526f, 0);
                }
            }
            break;
        case 0x42:
            if (i == D_00154b58) {
                Object *obj;
                int kind = 0;
                u8 slot = 0;

                obj = &D_001549a4[D_001549d0[i].value];
                if (obj->f16 == D_00154b58) {
                    switch (obj->fc) {
                    case 2:
                        slot = 1;
                        D_001581b2[slot].kind = 2;
                        D_001581b2[slot].f4 = 1;
                        D_001581b2[slot].f6 = 0;
                        kind = 0x22;
                        break;
                    case 4:
                        switch (obj->fe) {
                        case 2:
                            slot = 2;
                            D_001581b2[slot].kind = 1;
                            kind = 0x21;
                            break;
                        default:
                            slot = 2;
                            D_001581b2[slot].kind = 0;
                            kind = 0x20;
                            break;
                        }
                        break;
                    case 6:
                        slot = 3;
                        D_001581b2[slot].kind = 3;
                        break;
                    }
                }
                if (slot) {
                    fn_00087b4c();
                    D_00155d70 = slot;
                    fn_0001bc94(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    fn_0001aa8c(1);
                }
                if (kind) {
                    int x = D_0015f414[slot].x;
                    int y = D_0015f414[slot].y;

                    fn_000ac78c(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                    D_001553ec[slot] = D_001567a8 - 1;
                    D_001567a7 = 0;
                }
            }
            break;
        case 0x43:
            if (i == D_00154b58) {
                fn_000736d4((u8)D_001549d0[i].value);
            }
            break;
        case 0x44:
            fn_00027e1c(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == D_00154b58) {
                fn_00088ebc(10, 0xba, -1, -1);
            }
            break;
        case 0x47:
            fn_0002a804(&D_001549a4[D_001549d0[i].value], D_001549d0[i].arg);
            if (i == D_00154b58) {
                fn_0006fdfc(1);
                fn_00087b4c();
                fn_0001a374(0);
                D_00188c7b = 0;
                fn_00088ebc(10, 0xaa, -1, -1);
            }
            break;
        case 0x45:
            fn_00047308(i);
            break;
        case 0x48:
            if (D_001549d0[i].arg) {
                if (D_001547b4[i].f35 >= D_001549d0[i].value && D_001547b4[i].f35 > 0) {
                    fn_0002b998(&D_001549a4[D_001549d0[i].arg], D_001549d0[i].value, 0, 1, i);
                }
            } else {
                if (D_001547b4[i].f35 >= fn_0002b998(0, D_001549d0[i].value, 0, 0, i) && D_001547b4[i].f35 > 0) {
                    fn_0002b998(0, D_001549d0[i].value, 0, 1, i);
                }
            }
            break;
        case 0x46:
            D_00155d7d = 1;
            fn_00027920(&D_001547a4[D_001549d0[i].value]);
            break;
        case 0x49:
            if (i == D_00154b58) {
                if (D_0015819a == 0x63) {
                    D_0015819a = 0xd;
                    D_00158198 = 1;
                    D_0015819c = 0;
                    D_0015819e = 0;
                    D_001581a0 = 0;
                    D_001581a1 = 0;
                    D_001581a2 = 0;
                    fn_00087b4c();
                    fn_0001bc94(D_0015819a, 1);
                    fn_0001aa8c(1);
                    D_0012e383 = 1;
                }
            }
            break;
        case 0x4a:
            D_001547b4[i].f1e2 = 10;
            D_001547b4[i].f1d9 = 100;
            D_001547b4[i].f1e5 = 0;
            D_00155e09 = D_001547b4[i].f1d9;
            if (i == D_00154b58) {
                fn_000b73a0(0, 1, 1);
            }
            break;
        case 0x4b:
            if (D_001549d0[i].value & (1 << D_00154b58)) {
                fn_00085ce8(i);
            }
            D_00154b5c[i].f0 = 0;
            break;
        case 0x4c:
            fn_00085f3c(i);
            break;
        case 0x4d:
            fn_00085e50(D_001549d0[i].value, i);
            break;
        case 0x4e:
            D_00154b5c[i].f0 = 0;
            if (i == D_00154b58) {
                D_00155402 = 1;
            }
            break;
        case 0x4f:
            if (D_001547b4[i].fc95 == 0) {
                D_001547b4[i].fc94 = D_001549d0[i].value;
            }
            break;
        case 0x50:
            fn_000b2c74(D_001549d0[i].value, D_001549d0[i].arg, i);
            break;
        case 0x51:
            fn_0004b654(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == D_00154b58) {
                fn_00088ebc(10, 0xaa, -1, -1);
            }
            break;
        case 0x53:
            if (D_001547a4[D_001549d0[i].value].f1e < 0x1e) {
                D_001547a4[D_001549d0[i].value].f1e++;
            }
            break;
        case 0x54:
            if (D_001547a4[D_001549d0[i].value].f1e) {
                D_001547a4[D_001549d0[i].value].f1e--;
            }
            break;
        case 0x55:
            fn_0004b6b0(D_001549d0[i].value, D_001549d0[i].arg);
            break;
        case 0x56:
            fn_0002c700(D_001549d0[i].value, D_001549d0[i].arg);
            if (i == D_00154b58) {
                D_001549ae = 0;
                fn_0001bdb8(0xc);
                fn_00087b4c();
                D_001567a7 = 0;
                D_0012e383 = 0;
            }
            break;
        case 0x58:
            fn_0009b1f0();
            fn_000688c0();
            D_001549ae = 0xe;
            D_00158146 = 0xb;
            D_00155430 = D_001549d0[i].value;
            break;
        case 0x59:
            fn_00068480(i);
            break;
        case 0x5a:
            if (D_001549d0[i].arg) {
                fn_0002cb2c(&D_001549a4[D_001549d0[i].arg], D_001549d0[i].value, 0, i);
            } else {
                fn_0002cb2c(0, D_001549d0[i].value, 0, i);
            }
            break;
        case 0x5b:
            if (i == D_00154b58) {
                fn_00087b4c();
                fn_0001bdb8(5);
                D_00155c6f = D_00155273;
                fn_00054e50(&D_001547a4[D_001547b4[i].f11b7]);
                fn_00054fc0(D_001547a4[D_001547b4[i].f11b7].f0);
                D_0015819a = 10;
                fn_0001bc94(D_0015819a, 1);
                D_001581a0 = D_001547b4[i].f11bc;
                D_001581a1 = D_001547a4[D_001547b4[i].f11b7].f8;
                D_00158198 = 3;
                D_0015819b = D_00154690;
                D_0012e383 = 0;
            }
            break;
        case 0x5c:
            if (i == D_00154b58) {
                fn_0009ffa8(D_001549d0[i].value);
            }
            break;
        case 0x5e:
            obj2 = &D_001549a4[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 1;
            break;
        case 0x5f:
            obj2 = &D_001549a4[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 4;
            break;
        case 0x60:
            obj2 = &D_001549a4[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 2;
            break;
        case 0x61:
            obj2 = &D_001549a4[D_001549d0[i].value];
            obj2->f8e &= 0xf0;
            obj2->f8e |= 8;
            break;
        case 0x62:
            if (i == D_00154b58) {
                fn_0001b3e8(10);
                if (D_0015819a == 0x63) {
                    D_0015819c = 1;
                    fn_00054fc0(3);
                    fn_00087b4c();
                    D_0015819a = 10;
                    fn_0001bc94(D_0015819a, 1);
                    D_001547b4[i].f11b7 = 0;
                    D_001581a0 = 3;
                    D_001581a1 = 0;
                    D_00158198 = 3;
                    D_0015819b = D_00154690;
                    D_0012e383 = 1;
                }
            }
            break;
        case 0x63:
            if (i == D_00154b58) {
                fn_0009d114(D_001549d0[i].value, D_001549d0[i].arg);
            }
            break;
        case 0x64:
            if (i == D_00154b58) {
                fn_0001bdb8(10);
                fn_00087b4c();
                fn_0001aa8c(D_001553e9);
                D_0012e383 = 0;
            }
            break;
        case 0x65:
            D_001547c0->f64f = D_0012b7c8[D_001547c0->f650] - 1;
            break;
        case 0x66:
            fn_00050ef0(0x12, 5000, 0, i);
            D_001547b4[i].f3d += 5000;
            break;
        case 0x67:
            if (D_001547b4[i].f35 >= 5000 && D_001547b4[i].f35 > 0) {
                fn_00050ef0(0x13, -5000, 0, i);
                D_001547b4[i].f3d -= 5000;
            }
            break;
        case 0x57:
            obj1 = &D_001549a4[D_001549d0[i].value];
            D_001547b4[obj1->f16].dept[obj1->fa8].count++;
            if (obj1->f74 & 0x20) {
                D_001547b4[obj1->f16].f207 = 0;
            }
            fn_00017d78(obj1, D_001549d0[i].arg);
            if (i == D_00154b58) {
                fn_000736d4(1);
                fn_00088ebc(10, 0xaa, -1, -1);
            }
            break;
        case 0x68:
            obj2 = &D_001549a4[D_001549d0[i].value];
            D_001547b4[i].f11f4 = D_001549d0[i].value;
            if (!(obj2->f8e & 0x20)) {
                if (fn_0002d11c(obj2)) {
                    fn_0002c3bc(obj2);
                    fn_0002becc(&D_001549a4[D_001549d0[i].value]);
                }
            }
            if (i == D_00154b58) {
                fn_000736d4(2);
            }
            break;
        case 0x69:
            fn_0002becc(&D_001549a4[D_001549d0[i].value]);
            break;
        case 0x6a: {
            Object *o = &D_001549a4[D_001549d0[i].value];

            switch (D_001549d0[i].argb[0]) {
            case 1:
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xd3, D_00188c64, D_00188c68);
                }
                break;
            case 2:
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xd4, D_00188c64, D_00188c68);
                }
                break;
            case 3:
                fn_00075484(o->f5 + (o->f7 << 7), 0x902);
                break;
            default:
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0x3f, D_00188c64, D_00188c68);
                }
                break;
            }
            D_0015f8ed++;
            D_001547b4[i].fd29++;
            if (D_001549d0[i].argb[1]) {
                D_0015f8e9++;
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0x40, D_00188c64, D_00188c68);
                }
                fn_0009d480(0, ++D_001547b4[i].fd27, (o->f7 << 7) + o->f5, D_001548b5, 0x20);
                fn_0007a710(o->f5 + (o->f7 << 7), (s8)o->f16, 4);
                fn_000199e8(o);
            }
            break;
        }
        case 0x6b:
            obj2 = &D_001549a4[D_001549d0[i].value];
            if (obj2 > D_001549a4) {
                if (fn_000b0100() % (obj2->f8f + 1)) {
                    fn_0002a804(&D_001549a4[D_001549d0[i].value], 1);
                } else {
                    obj2->f8e &= 0xdf;
                    D_00158324 &= 0xfe;
                    obj2->f80 = 0;
                    fn_000b0428(&D_001549a4[D_001549d0[i].value], 9);
                }
                if (i == D_00154b58) {
                    D_001549ae = 0;
                    fn_0001bdb8(0xc);
                    fn_00087b4c();
                    D_001567a7 = 0;
                }
                D_0012e383 = 0;
            }
            break;
        case 0x6c:
            obj2 = &D_001549a4[D_001549d0[i].value];
            if (obj2 > D_001549a4) {
                obj2->f8e &= 0xdf;
                D_00158324 &= 0xfe;
                obj2->f6c += D_001549d0[i].arg;
                obj2->f51 = 100;
                obj2->f80 = 0;
                fn_000b0428(&D_001549a4[D_001549d0[i].value], 9);
            }
            if (i == D_00154b58) {
                D_001549ae = 0;
                fn_0001bdb8(0xc);
                fn_00087b4c();
                D_001567a7 = 0;
                D_0012e383 = 0;
            }
            break;
        case 0x6d:
            obj2 = &D_001549a4[D_001549d0[i].value];
            if (obj2 > D_001549a4) {
                obj2->f8e &= 0xdf;
                D_00158324 &= 0xfe;
                fn_0002b998(obj2, D_001549d0[i].arg, 0, 1, i);
                obj2->f51 = 100;
                obj2->f80 = 0;
                fn_000b0428(&D_001549a4[D_001549d0[i].value], 9);
            }
            if (i == D_00154b58) {
                D_001549ae = 0;
                fn_0001bdb8(0xc);
                fn_00087b4c();
                D_001567a7 = 0;
                D_0012e383 = 0;
            }
            break;
        case 0x6e: {
            int dept = D_001549d0[i].arg;
            int rate = fn_00058e34(dept, i);

            if (D_001547b4[i].dept[dept].level < 20000) {
                D_001547b4[i].dept[dept].level += D_001549d0[i].value * 100;
                if (D_001547b4[i].dept[dept].level < 5000) {
                    D_001547b4[i].dept[dept].level = 5000;
                } else if (D_001547b4[i].dept[dept].level > 20000) {
                    D_001547b4[i].dept[dept].level = 20000;
                }
                D_001547b4[i].dept[dept].value = D_001547b4[i].dept[dept].level * rate / 10000;
            }
            break;
        }
        case 0x6f: {
            int dept = D_001549d0[i].arg;
            int rate = fn_00058e34(dept, i);

            if (D_001547b4[i].dept[dept].level > 5000) {
                D_001547b4[i].dept[dept].level -= D_001549d0[i].value * 100;
                if (D_001547b4[i].dept[dept].level < 5000) {
                    D_001547b4[i].dept[dept].level = 5000;
                } else if (D_001547b4[i].dept[dept].level > 20000) {
                    D_001547b4[i].dept[dept].level = 20000;
                }
                D_001547b4[i].dept[dept].value = D_001547b4[i].dept[dept].level * rate / 10000;
            }
            break;
        }
        case 0x70:
            if (D_001553c7 == D_00154b58) {
                D_00188c81 = D_00188c83 = D_00188c7b = D_00188c7c = 0;
                fn_000b73a0(D_001549d0[i].value, 1, 1);
            }
            break;
        case 0x71:
            switch (D_001549d0[i].value) {
            case 1:
                if (D_001549d0[i].arg <= D_001547b4[i].f219) {
                    D_001547b4[i].f21b = D_001549d0[i].arg;
                } else {
                    D_001547b4[i].f21b = D_001547b4[i].f219;
                }
                break;
            case 2:
                if (D_001549d0[i].arg >= D_001547b4[i].f21b) {
                    D_001547b4[i].f219 = D_001549d0[i].arg;
                } else {
                    D_001547b4[i].f219 = D_001547b4[i].f21b;
                }
                break;
            case 3:
                D_001547b4[i].f217 = D_001549d0[i].arg;
                break;
            case 4:
                D_001547b4[i].f21d = D_001549d0[i].arg;
                break;
            case 5:
                D_001547b4[i].f21f = D_001549d0[i].arg;
                break;
            }
            break;
        case 0x72:
            if (D_001549d0[i].value == (D_00155f4d > 1 ? D_001547b4[i].f1256 : D_00155376)) {
                if (D_00155f4d > 1) {
                    D_001547b4[i].f1256 = 0;
                } else {
                    D_00155376 = 0;
                }
            } else {
                if (D_00155f4d > 1) {
                    D_001547b4[i].f1256 = D_001549d0[i].value;
                } else {
                    D_00155376 = D_001549d0[i].value;
                }
            }
            break;
        case 0x73:
            fn_00087c44(D_001549d0[i].value, D_001549d0[i].argb[0]);
            break;
        case 0x74:
            obj1 = &D_001549a4[D_001549d0[i].value];
            obj1->fab = 0;
            D_001547b4[obj1->f16].dept[obj1->fa8].count++;
            fn_0001665c(obj1);
            obj1->f75 |= 0x80;
            if (obj1->f74 & 0x20) {
                D_001547b4[obj1->f16].f207 = 0;
            }
            break;
        case 0x76:
            fn_0007baac(D_001547b4[i].ff6d, D_001549d0[i].value, 0xff);
            fn_000b0300(&D_001549a4[D_001549d0[i].value], 0x1c);
            break;
        case 0x77:
            if (i == D_00154b58) {
                if (D_00155e1d) {
                    fn_000b73a0(0, 1, 1);
                }
                D_00155d7c = D_0015536e;
                D_001549ae = 0xb;
                fn_00087b4c();
                D_00155d7b = D_0015819a;
                if (D_0015819a != 0x63) {
                    fn_0001bdb8(D_0015819a);
                }
                D_0015819a = 4;
                fn_0001bc94(D_0015819a, 1);
                D_0012e383 = 1;
            }
            break;
        case 0x78:
            if (i == D_00154b58) {
                fn_0006fdfc((u8)D_001549d0[i].value);
                if (D_00155d69) {
                    fn_0001a374((u8)D_00155d69);
                    D_00188c7b = 0;
                }
            }
            break;
        case 0x79:
            fn_0006fdfc(0);
            fn_0002812c(i);
            fn_000483c4(i);
            D_001549d0[i].type = 0x38;
            D_001549d0[i].arg = D_0015536e;
            goto dispatch;
        case 0x7b:
            fn_0002994c(i);
            D_001549d0[i].type = 0x38;
            D_001549d0[i].value = 0;
            goto dispatch;
        case 0x7c:
            fn_00017d78(&D_001549a4[D_001549d0[i].value], D_001549d0[i].arg);
            break;
        case 0x7d: {
            u8 quiet = 0;

            switch (D_001549d0[i].value) {
            case 0:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->fab = 0;
                    D_001547b4[obj1->f16].dept[obj1->fa8].count++;
                    fn_0001665c(obj1);
                    obj1->f76 |= 0x10;
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            case 1:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    fn_000b0428(obj1, 0x1b);
                    obj1->f74 |= 0x80;
                    obj1->f3b_w = D_0015543a;
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            case 2:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->fab = 0x2c;
                    obj1->f74 &= 0xef;
                    fn_000b0428(obj1, 0x1b);
                    obj1->fd = 0x26;
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            case 0xa:
                fn_000a04e4(D_001547b4[i].f20b, 2, i);
                D_001547b4[i].f20b = 0;
                D_001547b4[i].f20d = 0;
                fn_00050ef0(0x21, -D_001547b4[i].f203, 0, i);
                fn_00058370(0, 0xe, 0, i, 0);
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xce, -1, -1);
                }
                break;
            case 0xb:
                fn_000a04e4(D_001547b4[i].f20b, 1, i);
                D_001547b4[i].f20d = 1;
                D_001547b4[i].f207 = 100;
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xcd, -1, -1);
                }
                quiet = D_001549d0[i].arg;
                break;
            case 0x15:
                D_001547b4[i].f1fa = 0;
                break;
            case 0x14:
                D_001547b4[i].f1f3 = D_001549c4 + 1500;
                D_001547b4[i].f1fa = 2;
                break;
            case 0x1e:
                if (D_00155f4d > 1) {
                    fn_00018620(2);
                }
                break;
            case 0x1f:
                fn_000af510(D_0015f5c2[D_00154b58].f0, i);
                D_001549d0[i].type = 0x4a;
                goto dispatch;
            case 0x28:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->fab = 0;
                    D_001547b4[obj1->f16].dept[obj1->fa8].count++;
                    fn_0001665c(obj1);
                    obj1->f76 |= 8;
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            case 0x29:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    fn_000b0428(obj1, 0x1b);
                    fn_000427d8(obj1);
                    fn_0003e330(obj1);
                    fn_0003c8d8(obj1, 0);
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            case 0x2a:
                obj1 = &D_001549a4[D_001549d0[i].arg];
                if (obj1->fc == 2 && (obj1->f74 & 0x10)) {
                    obj1->f74 &= 0xef;
                    fn_000b0428(obj1, 0x1b);
                    obj1->f74 |= 0x80;
                    obj1->f3b_w = D_0015543a;
                }
                if (obj1->fc == 2) {
                    obj1->f5b = 0;
                }
                break;
            }
            if (i == D_00154b58 && quiet == 0) {
                fn_000a4208();
                D_001553d7 = D_00153f34;
                D_001553c8 = 1;
                D_00156910 = 1;
            }
            break;
        }
        case 0x7e:
            fn_0009b1f0();
            switch (D_001549d0[i].value) {
            case 0:
                break;
            case 1:
                if (D_001547b4[i].f2b == 2) {
                    Object *o;

                    for (o = D_001549a4 + 1; o < D_001549a8; o++) {
                        if (o->f16 == i) {
                            switch (o->fc) {
                            case 6:
                                if (D_00129636[o->fe].f0) {
                                    o->f37 = o->f3b * 2;
                                    fn_00075010(o);
                                }
                                break;
                            case 4:
                                fn_0002a804(o, 0);
                                break;
                            case 2:
                                if (!(D_00153f78[(s16)o->f5 + ((s16)o->f7 << 7)].f7 & 0x10)) {
                                    o->f74 &= 0xdf;
                                    o->fae = 0;
                                    o->f75 |= 1;
                                }
                                break;
                            }
                        }
                    }
                    D_001547b4[i].f2b = 0;
                    D_001549d0[i].checksum = 0;
                    if (i == D_00154b58) {
                        fn_0006fdfc(0);
                        fn_000c02fc();
                        D_001549ac = 1;
                        D_0015542c = 100000;
                        if (D_00155e4d) {
                            D_001547b4[D_00154b58].f2b = 0;
                            D_00158100 = 0xc;
                            D_0015542a = 6;
                            D_0015818e = D_0015542a;
                        } else {
                            D_0015542a = 2;
                            D_0015818e = D_0015542a;
                        }
                        fn_0009e0d8();
                        D_00155f4d = 1;
                        D_00154b58 = 0;
                    }
                    fn_0007fbb8((s8)i);
                }
                break;
            case 2:
                break;
            default:
                break;
            }
            break;
        case 0x7f:
            obj1 = &D_001549a4[D_001549d0[i].value];
            obj1->f74 &= 0xef;
            fn_000b0428(obj1, 0x1b);
            obj1->f74 |= 0x80;
            obj1->f3b_w = D_0015543a;
            break;
        case 0x80:
            obj1 = &D_001549a4[D_001549d0[i].value];
            obj1->fab = 0x2c;
            obj1->f74 &= 0xef;
            fn_000b0428(obj1, 0x1b);
            obj1->fd = 0x26;
            break;
        case 0x81:
            obj2 = &D_001549a4[D_001549d0[i].value];
            if (obj2 > D_001549a4) {
                obj2->f93 = 1 << (u8)i;
            }
            break;
        case 0x82:
            obj1 = &D_001549a4[D_001549d0[i].value];
            if (obj1->f74 & 1) {
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xdf, -1, -1);
                }
                fn_000987c8(obj1);
                if (obj1->f74 & 0x20) {
                    fn_000b0300(obj1, 0xd);
                }
            } else {
                if (i == D_00154b58) {
                    fn_00088ebc(10, 0xde, -1, -1);
                }
                fn_00098770(obj1);
            }
            break;
        case 0x83:
            D_001547a4[D_001547b4[i].f11b7].f3 = 4;
            D_001547a4[D_001547b4[i].f11b7].f22 = 0;
            if (i == D_00154b58) {
                fn_00027a90();
                fn_0001a374(0);
            }
            break;
        case 0x84:
            if (i == D_00154b58) {
                D_0015541b = 0;
                D_001553fd = 0;
                D_001549ae = 0;
                fn_00087b4c();
                fn_0001bdb8(6);
                D_001567a7 = 0;
                fn_0001a374(0);
                D_0012e383 = 0;
            }
            break;
        case 0x85:
            break;
        case 0x86:
            fn_000194a0(&D_001549a4[D_001549d0[i].value]);
            break;
        case 0x87:
            if (D_001553c7 == D_00154b58) {
                D_00188c81 = D_00188c83 = D_00188c7b = D_00188c7c = 0;
                fn_000b73a0(D_001549d0[i].value, 1, 1);
                if (i == D_00154b58) {
                    Object *obj;
                    int kind = 0;
                    u8 slot = 0;

                    obj = &D_001549a4[D_001549d0[i].arg];
                    if (obj->f16 == D_00154b58) {
                        switch (obj->fc) {
                        case 2:
                            slot = 1;
                            D_001581b2[slot].kind = 2;
                            D_001581b2[slot].f4 = 1;
                            D_001581b2[slot].f6 = 0;
                            kind = 0x22;
                            break;
                        case 4:
                            switch (obj->fe) {
                            case 2:
                                slot = 2;
                                D_001581b2[slot].kind = 1;
                                kind = 0x21;
                                break;
                            default:
                                slot = 2;
                                D_001581b2[slot].kind = 0;
                                kind = 0x20;
                                break;
                            }
                            break;
                        case 6:
                            slot = 3;
                            D_001581b2[slot].kind = 3;
                            break;
                        }
                    }
                    if (slot) {
                        fn_00087b4c();
                        fn_0001b3e8(0x63);
                        if (D_0015819a != 0x63) {
                            break;
                        }
                        D_00155d70 = slot;
                        fn_0001bc94(D_001581b2[slot].kind, 1);
                        D_001581b2[slot].value = D_001549d0[i].arg;
                        fn_0001aa8c(1);
                    }
                    if (kind) {
                        int x = D_0015f414[slot].x;
                        int y = D_0015f414[slot].y;

                        fn_000ac78c(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                        D_001553ec[slot] = D_001567a8 - 1;
                        D_001567a7 = 0;
                    }
                }
            }
            break;
        case 0x88:
            if (i == D_00154b58) {
                Object *obj;
                int kind = 0;
                u8 slot = 0;

                fn_0006fdfc(1);
                fn_00087b4c();
                if (D_00155d69) {
                    fn_0001a374((u8)D_00155d69);
                    D_00188c7b = 0;
                }
                obj = &D_001549a4[D_001549d0[i].value];
                if (obj->f16 == D_00154b58) {
                    switch (obj->fe) {
                    case 2:
                        slot = 2;
                        D_001581b2[slot].kind = 1;
                        kind = 0x21;
                        break;
                    default:
                        slot = 2;
                        D_001581b2[slot].kind = 0;
                        kind = 0x20;
                        break;
                    }
                }
                if (slot) {
                    fn_00087b4c();
                    D_00155d70 = slot;
                    fn_0001bc94(D_001581b2[slot].kind, 1);
                    D_001581b2[slot].value = D_001549d0[i].value;
                    fn_0001aa8c(1);
                }
                if (kind) {
                    int x = D_0015f414[slot].x;
                    int y = D_0015f414[slot].y;

                    fn_000ac78c(0, 0, 0, x + D_0015f40c[kind].x0, y + D_0015f40c[kind].y0, x + D_0015f40c[kind].x1, y + D_0015f40c[kind].y1);
                    D_001553ec[slot] = D_001567a8 - 1;
                    D_001567a7 = 0;
                }
                fn_00088ebc(10, 0x42, -1, -1);
            }
            break;
        case 0x89:
            D_001547c0->f650 = 0xb;
            D_001547c0->f64f = D_0012b7c8[D_001547c0->f650] - 2;
            break;
        case 0x8a:
            if (i == D_00154b58) {
                fn_0006fdfc(0);
            }
            fn_0003a3e0(&D_001549a4[D_001549d0[i].value]);
            break;
        case 0x8b:
            D_001547b4[i].f1257 = 0;
            if (D_00155f4d <= 1) {
                D_001547b4[i].f1256 = 1;
            }
            if (i != D_00154b58) {
                fn_00060b54(0xc, i);
            }
            break;
        case 0x8c:
            if (i == D_00154b58) {
                fn_0006fdfc((u8)D_001549d0[i].value);
                if (D_00155d69) {
                    fn_0001a374((u8)D_00155d69);
                    D_00188c7b = 0;
                }
                if (D_00155d57 < 10) {
                    D_00155dc6[D_00155d57] = 1;
                }
                fn_0008bd24();
            }
            break;
        case 0x90:
            if (i == D_00154b58) {
                D_00155e4c = 0;
            } else {
                D_00155e4c = 1;
            }
            break;
        case 0x91:
            fn_0006fdfc(0);
            fn_00022a58();
            break;
        default:
            break;
        }
        D_00154a0c[i].type = D_001549d0[i].type;
        D_00154a0c[i].value = D_001549d0[i].value;
        D_00154a0c[i].arg = D_001549d0[i].arg;
        D_00154a0c[i].checksum = D_001549d0[i].checksum;
        D_001549d0[i].type = 0;
        D_001549d0[i].value = 0;
        D_001549d0[i].arg = 0;
    }
    D_001553c7 = D_00154b58;
    fn_000228e0();
}
}
