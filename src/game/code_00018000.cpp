#include "common.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

struct Obj {
    u16 type;
    u16 x;
    s16 timer;
    u16 id;
    u8 active;
    u8 pad[2];
    u8 owner;
};

struct Player {
    u8 unknown[0x7c];
    u16 staff[0x10];
    u16 rooms[0x1e];
    u16 first[9];
};

struct Path {
    u8 unknown0[0xc];
    u8 result;
    u8 unknownd[3];
};

struct Object {
    u8 kind;
    s8 f1;
    s8 f2;
    s8 f3;
    u16 pos;
    u8 unknown6[6];
    u16 fc;
    u8 unknowne[0xd];
    u8 f1b;
    u8 owner;
    u8 unknown1d[6];
};

struct Walker {
    u8 unknown0[5];
    u8 x;
    u8 unknown6;
    u8 y;
    u8 unknown8[9];
    u8 f11;
};

struct SortEntry {
    signed int key : 27;
};

struct Cursor {
    u8 unknown0[4];
    u8 f4;
    u8 f5;
};

struct Staff {
    u8 unknown0[0x22];
};

struct RoomDef {
    u16 id;
    u8 unknown2[0xa];
    u8 fc;
    u8 unknownd[0x17];
    u8 f24;
    u8 unknown25[5];
};

struct Movie {
    char name[0x28];
    u16 f28;
    u8 unknown2a[8];
    u32 f32;
    u32 f36;
};

struct ResearchShapes {
    u8 shapes[4];
    u8 unknown4[0x21];
};

struct Shape {
    s8 count;
    u16 points[0x15];
    u8 unknown2b;
};

struct Bounds {
    s8 minX;
    s8 minY;
    s8 maxX;
    s8 maxY;
};

struct RoomInfo {
    u16 staffMask;
    u8 unknown2[7];
    u8 room;
    u8 unknowna[9];
};

struct Tile {
    s16 first;
    u8 unknown2[5];
    u8 f7;
};

struct Room {
    u8 type;
    u8 unknown1[5];
    u16 f6;
    u8 unknown8[6];
    u16 fe;
};

struct Entity {
    u8 unknown0[2];
    s16 f2;
    u8 unknown4;
    u8 x;
    u8 unknown6;
    u8 y;
    u8 unknown8[4];
    u8 fc;
    u8 fd;
    u8 fe;
    u8 unknownf[7];
    u8 owner;
    u8 unknown17[0x20];
    Room *room;
    u8 unknown3b[0xc];
    u16 f47;
    u16 f49;
    u8 unknown4b[4];
    u16 next;
    u8 unknown51[8];
    u16 f59;
    u8 unknown5b[3];
    u8 f5e;
    u8 unknown5f[7];
    u8 state;
    u8 unknown67[0xd];
    u32 flags;
    u8 unknown78[0x14];
    u16 f8c;
    u8 unknown8e[4];
    s16 f92;
    u8 unknown94[0x14];
    u8 fa8;
    u8 unknowna9[6];
};

extern "C" {

extern u8 D_001553b4;
extern Obj *D_00154824;
extern Obj *D_00154828;
extern Player *D_00154820;
extern u32 D_00154b58;
extern s16 D_0015f580;

u16 fn_000b01a0(void);
void fn_00088d90(u8 sound);
void fn_00035760(Obj *obj);

struct Slot {
    u32 used;
    u8 unknown[0x20];
};

extern u8 D_00155f4d;
extern Slot D_001609c4[];
extern Obj D_0015f5a4[][9];

void fn_000c9372(void *dst, void *src, u32 n);
void fn_00035624(Obj *obj);
void fn_00018438(u16 type, u8 owner);

struct HospitalItem {
    u8 f0;
    u8 unknown1[0xd];
};

struct HospitalRoom {
    u8 f0;
    u8 unknown1[0x25];
};

struct Hospital {
    u8 unknown0[0x2a];
    u8 f2a;
    u8 unknown2b[0x1b7];
    u8 f1e2;
    u8 unknown1e3[0xe];
    u16 f1f1;
    u32 f1f3;
    u8 unknown1f7[3];
    u8 f1fa;
    u8 unknown1fb[0x41];
    HospitalRoom roomStats[0x2c];
    u8 f8c4;
    u8 unknown8c5[0x61];
    HospitalItem items[0x3e];
    u8 unknownc8a[0x5cf];
};

extern Hospital *D_001547b4;
extern u32 D_001549c4;
extern u16 D_0015f764;

void fn_000a4168(int n);

extern Entity *D_001549a4;

void fn_000417ac(Entity *e, int a, int b);
void fn_00040524(Entity *e);

extern u8 D_00158192;

void fn_0004b058(Entity *e);
void fn_000194f0(Entity *e);
void fn_0003e330(Entity *e);
void fn_0003c8d8(Entity *e, int a);
void fn_00015b68(Entity *e);
void fn_000bc0b0(void *dst, int c, u32 n);

extern Tile *D_00153f78;
extern Tile *D_00153f7c;

void fn_000427d8(Entity *e);
void fn_0002c3bc(Entity *e);

extern u32 D_00153f38;
extern u32 D_001548ac;
extern u8 D_001548b3, D_001548b4, D_001548b5, D_001548b6, D_001548b7;
extern u8 D_001548b8, D_001548b9, D_001548ba, D_001548bb, D_001548bc;
extern u8 D_001548bd, D_001548be, D_001548bf, D_001548c0;

u8 fn_000c960c(u32 palette, int r, int g, int b);
void fn_00055454(Path *path, s8 a, s8 b, s8 c, s8 d);

extern Object *D_001547a4;
extern Object *D_001547a8;

u32 fn_000b0100(void);

u32 fn_000c9abc(u32 n);

extern u8 D_00155e1d;
extern u8 D_0012cf18[];
extern u8 D_0012cf00[];
extern char D_0012cedc[];
extern char D_0012cec4[];
extern char *D_001291c4;
extern Cursor *D_00153f08;
extern Cursor *D_00153f2c;
extern u8 D_001553e8;
extern u8 D_001553e9;

void fn_000bd400(Cursor *c, int x, int y);

struct Researchable {
    u8 f0;
    u8 unknown1[0x24];
};

extern u8 D_00155e46;
extern Researchable D_00129634[];

void fn_00050ef0(int a, int b, int c, u16 d);

extern u32 D_00188c18;
extern u32 D_00188c30;
extern char D_0014dbbe[];
extern char D_001603dc[];
extern u8 D_0015819a;
extern u16 D_0015f984[];
extern Staff *D_0015f414;

void fn_000c9b40(u32 a, u32 b, u32 c);
void fn_000caa79(char *fmt, va_list args);
void fn_000caa8a(char *s);
void fn_0004c9dc(void *a, void *b);
void fn_0009ab54(void);
void fn_000c9400(u32 a, int b, int c);

extern s16 D_001291ce[];
extern u16 D_001549ae;
extern u8 D_001569fb;
extern u8 D_00155c69;
extern u32 D_00155c5a;
extern u32 D_00155c6a;
extern Entity *D_001549a8;

u16 fn_0004e154(void *a);

extern RoomDef D_0012c198[];
extern u8 D_001581a1;
extern u8 D_001553c7;

void fn_0002812c(u8 a);
void fn_0001bdb8(u8 i);

void fn_0001aa8c(u8 idx);
void fn_000c9be0(int x0, int y0, int x1, int y1, u8 color);
int fn_0001b874(u8 c);
int fn_000caa9a(int c);
void fn_000abbc8(int a, int x, int y);

int fn_000caaa8(int c);

enum Toggle {
    TOGGLE_0,
    TOGGLE_1
};

enum ScreenMode {
    SCREEN_MODE_1 = 1,
    SCREEN_MODE_13 = 13
};

extern s32 D_00188c34;
extern s32 D_00188c38;
extern u8 D_001567a7;
extern s32 D_00156498;
extern s32 D_0015649c;
extern char **D_0015f3fa;
extern ScreenMode D_0015f3f4;
extern s32 D_001547a0;
extern char D_001201ca[];
extern char D_001201cf[];
extern s32 D_001603ec, D_001603f0, D_001603f4, D_001603f8;

void fn_000ac264(u8 a, int x, int y);
void fn_000bbff0(char *dst, char *src);
void fn_000c93bf(char *dst, char *src);
int fn_000bc170(char *path);
int fn_000bc23c(char *path, s32 dst);
void fn_000bc86c(s32 a, s32 b, s32 c);
void fn_0004e0c0(void *a, Staff *b, u16 c);

extern char D_001201d4[];
extern char D_00120202[];
extern char D_00120218[];
extern char D_0012022d[];
extern char D_0012023e[];
extern u8 D_001581ec;
extern u8 D_001581ed;
extern u8 D_001581ef;
extern s32 D_001290cc;
extern u8 D_00155e45;
extern u32 D_001289a0;
extern u32 D_001289a4;
extern u8 D_0015f982;
extern u8 D_0015f983;
extern u32 D_0015f44c, D_0015f450, D_0015f454, D_0015f458, D_0015f45c, D_0015f460;
extern u32 D_0014bcc4, D_0014bcd0, D_0014bcd8, D_0014bce0, D_0014bce4, D_0014bce8;
extern u32 D_0013c60c, D_0013c618, D_0013c620, D_0013c628, D_0013c62c, D_0013c630;

void fn_000be3bc(s32 handle);
s32 fn_000be2e0(u32 name, int mode);
void fn_00010b20(void);
void fn_000c2111(int code);

struct PlayerInput {
    u8 f0;
    u8 f1;
    u16 f2;
    u8 f4;
    u16 f5;
    u8 unknown7[8];
};

extern PlayerInput D_001549d0[];
extern u16 D_0015536e;
extern u8 D_00155d7d;
extern u16 D_00158198;
extern u8 D_001581a0;
extern u16 D_001552b5;
extern char **D_00160324;
extern char D_001201c2[];
extern char D_00155ced[];

int fn_000bc13a(char *buf, char *fmt, ...);
void fn_0006fd98(int a, int b, int c, int d, u8 e, u8 f);
void fn_0006c5b8(void);

extern Toggle D_0015f3f5;
extern Movie D_0013eedc[];
extern u8 D_0015f980;
extern char D_0015f900[];
extern char D_001201c5[];
extern char D_0015fde4[];
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c7f, D_00188c81, D_00188c83;

void fn_000bed5c(void);
u8 fn_00012068(ScreenMode mode);
void fn_000becd0(u8 a, ...);
void fn_000bee60(void);
void fn_0009b5b8(char *s);
int fn_000be290(char *path);
u8 fn_000ca360(char *path, u32 a, u32 b);
void fn_000bac20(void);
int fn_000bed50(void);

extern ResearchShapes D_0012961a[];
extern Shape D_00129f34[];
extern Bounds D_00155f4f[][4];

void fn_0001a7dc(u16 pos, s8 *x, s8 *y);

extern s16 D_001291d0[];

#define ABS(a) ((a) < 0 ? -(a) : (a))

extern u8 D_00155df2[];
extern u8 D_00155df3, D_00155df4, D_00155df5, D_00155df6;
extern u8 D_00155378, D_00155379;
extern s16 D_001547e0[2];

extern u8 D_00160400;

extern char D_001201ac[];
extern u8 *D_00156b08;
extern s32 D_00156b04;
extern s32 D_00188c2c;

void *fn_000bac40(u32 size);
void fn_000c2210(u8 *palette, ...);
void fn_000bc0d0(void *dst, void *src, u32 n);
void fn_000bc2dc(char *path, void *buf, u32 size);

#define PUT16BE(p, v) \
    (*(u16 *)(p) = (v), tmp = *(u8 *)((u32)(p) + 1), \
     *(u8 *)((u32)(p) + 1) = *(p), *(p) = tmp, (*(u16 **)&(p))++)

struct UnknownD0 {
    u8 unknown0[0xd1];
    u8 fd1;
};

extern char D_0012d8e8[];
extern UnknownD0 *D_001897d0;
extern RoomInfo D_0012d900[];
extern char **D_00160370;
extern char **D_00160318;
extern char **D_001602a4;
extern u8 D_0015f762;
extern u16 D_0012dd7c;

u16 fn_00055214(int room);
void fn_000c889c(u16 a, u16 b, char *text);

extern char D_0012d8ea[];
extern char D_001201a8[];

// NONMATCHING: register allocation: value/address registers swapped in two stores
// FUNCTION: 0x0001824c
u16 fn_0001824c(u16 type, u16 x, u16 y, u8 owner)
{
    Obj *p;

    if (D_001553b4) {
        return 0;
    }
    for (p = D_00154824 + 1; p < D_00154828; p++) {
        if (p->type == 0) {
            switch (type) {
            case 3:
                if (owner == D_00154b58) {
                    fn_00088d90(fn_000b01a0() % 3 + 0xaf);
                }
                break;
            case 5:
                switch (x) {
                case 3:
                    if (owner == D_00154b58) {
                        fn_00088d90(fn_000b01a0() % 3 + 0xb2);
                    }
                    break;
                }
                break;
            }
            p->type = type;
            p->id = D_0015f580++;
            p->active = 1;
            p->x = x;
            p->timer = y;
            p->owner = owner;
            fn_00035760(p);
            if (D_00154820[owner].first[type] == 0) {
                D_00154820[owner].first[type] = p - D_00154824;
            }
            return p->id;
        }
    }
    return 0;
}

// NONMATCHING: register allocation: address computation for first[] store
// FUNCTION: 0x00018438
void fn_00018438(u16 type, u8 owner)
{
    int unused = 0;
    u16 min = 0xffff;
    Obj *p;

    for (p = D_00154824 + 1; p < D_00154828; p++) {
        if (p->type != type || p->owner != owner) {
            continue;
        }
        if (min > p->id) {
            D_00154820[owner].first[type] = p - D_00154824;
            min = p->id;
        }
    }
}

// FUNCTION: 0x000184f4
void fn_000184f4(u16 type)
{
    u8 i;
    int n = 1;
    Obj *p;

    if (D_00155f4d > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (D_00155f4d > 1 && D_001609c4[i].used == 0) {
            continue;
        }
        p = D_00154824 + D_00154820[i].first[type];
        if (p->type == 0) {
            return;
        }
        fn_000c9372(&D_0015f5a4[i][type], p, 12);
        fn_00035624(p);
        p->type = 0;
        D_00154820[i].first[type] = 0;
        fn_00018438(type, i);
    }
}

// FUNCTION: 0x00018620
void fn_00018620(u16 type)
{
    u8 i;
    u16 unused = 0xffff;
    int n = 1;
    Obj *p;

    if (D_00155f4d > 1) {
        n = 4;
    }
    for (i = 0; i < n; i++) {
        if (D_00155f4d > 1 && D_001609c4[i].used == 0) {
            continue;
        }
        for (p = D_00154824 + 1; p < D_00154828; p++) {
            if (p->type == 0) {
                fn_000c9372(p, &D_0015f5a4[i][type], 12);
                fn_00035760(p);
                break;
            }
        }
        fn_00018438(type, i);
    }
}

// NONMATCHING: register allocation: imul destination in first[] load
// FUNCTION: 0x00018710
u16 fn_00018710(u16 type, u16 id, u8 owner)
{
    u16 result = 0;
    Obj *p;

    for (p = D_00154824 + 1; p < D_00154828; p++) {
        if ((int)p->type != (int)type || p->owner != owner) {
            continue;
        }
        if (id == 0 || id && id == p->id) {
            p->type = 0;
            result = p->x;
            fn_00035624(p);
            if (D_00154820[owner].first[type] == p - D_00154824) {
                D_00154820[owner].first[type] = 0;
                fn_00018438(type, owner);
            }
            break;
        }
    }
    return result;
}

// FUNCTION: 0x00018838
int fn_00018838(u16 type, u16 x, u8 owner)
{
    int count = 0;
    Obj *p;

    for (p = D_00154824 + 1; p < D_00154828; p++) {
        if (p->type != type || p->owner != owner) {
            continue;
        }
        if (type == 4) {
            count++;
        } else if (type == 8) {
            count++;
        } else if (type == 2) {
            count++;
        } else if (type == 3) {
            count++;
        } else if (p->x == x) {
            count++;
        }
    }
    return count;
}

// NONMATCHING: register allocation in the first[] and Hospital address computations
// FUNCTION: 0x00018908
void fn_00018908(void)
{
    u8 owner1;
    u8 changed = 0;
    u8 owner2;
    Obj *p;

    for (p = D_00154824 + 1; p < D_00154828; p++) {
        if (p->timer) {
            p->timer--;
            if (p->timer > 0) {
                continue;
            }
            if (p->type == 6) {
                owner1 = p->owner;
                D_001547b4[owner1].f1f3 = D_001549c4 + 0x5dc;
                D_001547b4[owner1].f1fa = 2;
            } else if (p->type == 3) {
                D_0015f764 = 0x63;
                fn_000a4168(0xb);
            } else if (p->type == 5) {
                if (p->x == 1) {
                    D_001547b4[p->owner].f1e2 = 0;
                }
            }
            owner2 = p->owner;
            if (D_00154820[owner2].first[p->type] == p - D_00154824) {
                D_00154820[owner2].first[p->type] = 0;
                changed = 1;
            }
            fn_00035624(p);
            p->type = 0;
        }
    }
    if (changed) {
        for (p = D_00154824 + 1; p < D_00154828; p++) {
            if (p->type && D_00154820[p->owner].first[p->type] == 0) {
                D_00154820[p->owner].first[p->type] = p - D_00154824;
            }
        }
    }
}

// NONMATCHING: stack slots of the return value and ?: temporary; one imul register
// FUNCTION: 0x00018b78
int fn_00018b78(Entity *e, char **strs)
{
    char buf3[0xc8];
    char buf2[0x7d0];
    char buf1[0xc8];
    RoomInfo *info;
    int room;
    int unused30;
    int staff;
    int i;
    u16 b;
    u16 unused1c;
    s16 pct;
    u16 v14;
    u16 a;
    u16 sep;

    room = 0;
    staff = 0;
    unused30 = 0;
    unused1c = 0;
    sep = *(u16 *)D_0012d8e8;
    if (e <= D_001549a4) {
        return 0;
    }
    v14 = D_001897d0 ? D_001897d0->fd1 : 0;
    info = &D_0012d900[e->fa8];
    if (D_00154820[e->owner].rooms[info->room] == 0 && fn_00055214((u16)info->room)) {
        room = info->room;
    }
    for (i = 0; i < 9; i++) {
        if ((info->staffMask & (1 << i)) && D_00154820[e->owner].staff[i] == 0) {
            staff = i;
            break;
        }
    }
    if (room && staff) {
        fn_000bc13a(buf1, strs[0], D_00160370[D_0012c198[room].id], D_00160318[staff]);
    } else if (room) {
        fn_000bc13a(buf1, strs[1], D_00160370[D_0012c198[room].id]);
    } else if (staff) {
        fn_000bc13a(buf1, strs[2], D_00160318[staff]);
    } else {
        fn_000bc13a(buf1, strs[3]);
    }
    if (D_001547b4[e->owner].f8c4 != 1) {
        D_0015f762 &= ~4;
    }
    a = 0;
    b = 0;
    if (e->flags & 0x1000) {
        pct = e->f92 * 100 / D_0012dd7c;
        if (pct < 0) {
            pct = 0;
        } else if (pct > 99) {
            pct = 99;
        }
        fn_000bc13a(buf2, strs[6], pct, D_001602a4[e->fa8]);
    } else {
        fn_000bc13a(buf2, strs[4], D_001602a4[e->fa8]);
    }
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf1);
    fn_000bc13a(buf3, strs[5]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    fn_000c889c(a, b, buf2);
    return 0;
}

// FUNCTION: 0x00018f20
void fn_00018f20(u8 n)
{
    switch (n) {
    case 0:
        fn_000a4168(0);
        break;
    case 1:
        fn_000a4168(1);
        break;
    case 2:
        fn_000a4168(2);
        break;
    }
}

// NONMATCHING: register allocation: e->room->f6 argument loaded via eax instead of ebx
// FUNCTION: 0x00018f88
void fn_00018f88(Entity *e)
{
    Entity *q;

    if (e->room != 0) {
        for (q = D_001549a4 + e->room->fe; q > D_001549a4; q = D_001549a4 + q->next) {
            if (q->fc == 4 && (u16)(e - D_001549a4) == q->f49) {
                return;
            }
        }
        fn_000417ac(e, 0x5a, e->room->f6);
        e->state = 0x26;
        fn_00040524(e);
    }
}

// FUNCTION: 0x0001905c
int fn_0001905c(Entity *e)
{
    Room *r = e->room;

    if (r != 0) {
        if (r->type == 0x1d) {
            return 0;
        }
    } else if (e->state == 0x36) {
        return 0;
    }
    return 1;
}

// NONMATCHING: register allocation in the D_00154820/D_001547b4 address computations
// FUNCTION: 0x000190c4
int fn_000190c4(u8 kind, char **strs)
{
    char buf3[0xc8];
    char buf2[0x7d0];
    char buf1[0xc8];
    int i;
    RoomInfo *info;
    u16 v28;
    u16 sep;
    int unused18;
    int unused14;
    int staff;
    int room;
    u16 b;
    u16 a;

    room = 0;
    staff = 0;
    unused14 = 0;
    unused18 = 0;
    sep = *(u16 *)D_0012d8ea;
    if (kind <= 1) {
        return 0;
    }
    v28 = D_001897d0 ? D_001897d0->fd1 : 0;
    info = &D_0012d900[kind];
    if (D_00154820[D_00154b58].rooms[info->room] == 0 && fn_00055214((u16)info->room)) {
        room = info->room;
    }
    for (i = 0; i < 9; i++) {
        if ((info->staffMask & (1 << i)) && D_00154820[D_00154b58].staff[i] == 0) {
            staff = i;
            break;
        }
    }
    if (D_001547b4[D_00154b58].roomStats[kind].f0 == 1) {
        fn_000bc13a(buf1, strs[0]);
    } else if (room && staff) {
        fn_000bc13a(buf1, strs[1], D_00160370[D_0012c198[room].id], D_00160318[staff]);
    } else if (room) {
        fn_000bc13a(buf1, strs[2], D_00160370[D_0012c198[room].id]);
    } else if (staff) {
        fn_000bc13a(buf1, strs[3], D_00160318[staff]);
    } else {
        fn_000bc13a(buf1, D_001201a8);
    }
    buf2[0] = 0;
    a = 0;
    b = 0;
    fn_000bc13a(buf3, strs[4], D_001602a4[kind]);
    fn_000c93bf(buf2, buf3);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000bc13a(buf3, strs[kind * 3 - 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    fn_000bc13a(buf3, strs[kind * 3 - 1 + 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    fn_000bc13a(buf3, strs[kind * 3 + 1]);
    fn_000c93bf(buf2, (char *)&sep);
    fn_000c93bf(buf2, buf3);
    fn_000c889c(a, b, buf2);
    return 0;
}

// FUNCTION: 0x00019464
void fn_00019464(u8 n)
{
    switch (n) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000194a0
void fn_000194a0(Entity *e)
{
    e->state = 0;
    e->flags = 0;
    fn_0004b058(e);
    fn_000194f0(e);
    fn_0003e330(e);
    fn_0003c8d8(e, 0);
}

// NONMATCHING: one register choice
// FUNCTION: 0x000194f0
void fn_000194f0(Entity *e)
{
    u8 was44 = e->fd == 0x44 || e->state == 0x44;
    u8 state;
    Entity *q;

    if (e->f8c) {
        state = 8;
    } else if (e->f59) {
        state = 0x24;
    } else {
        fn_0004b058(e);
        state = 0x26;
    }
    fn_000427d8(e);
    if (e->room) {
        for (q = D_001549a4 + e->room->fe; q > D_001549a4; q = D_001549a4 + q->next) {
            if (q->fc == 4 && (u16)(e - D_001549a4) == q->f49) {
                q->f49 = 0;
                fn_0002c3bc(q);
                break;
            }
        }
        switch (e->fe) {
        case 4:
        case 5:
            fn_000417ac(e, 0xb4, 0);
            break;
        case 9:
        case 10:
            e->f5e = 0;
            fn_000417ac(e, 0x276, 0);
            break;
        }
        fn_000417ac(e, 0x5a, e->room->f6);
        fn_00040524(e);
        e->state = state;
    } else if (was44) {
        e->fd = 0x44;
    } else {
        e->fd = state;
    }
}

// FUNCTION: 0x0001974c
int fn_0001974c(void)
{
    return 1;
}

// FUNCTION: 0x00019770
void fn_00019770(void)
{
    D_001548ac = D_00153f38;
    D_001548b3 = fn_000c960c(D_00153f38, 0x3f, 0x3f, 0x3f);
    D_001548b4 = fn_000c960c(D_00153f38, 0, 0, 0);
    D_001548b5 = fn_000c960c(D_00153f38, 0, 0, 0x10);
    D_001548b6 = fn_000c960c(D_00153f38, 0, 0x3f, 0);
    D_001548b7 = fn_000c960c(D_00153f38, 0x3f, 0x30, 0x17);
    D_001548b8 = fn_000c960c(D_00153f38, 0x3f, 0x17, 0x17);
    D_001548b9 = fn_000c960c(D_00153f38, 0x3f, 0, 0);
    D_001548ba = fn_000c960c(D_00153f38, 0x1b, 0x37, 0x39);
    D_001548bb = fn_000c960c(D_00153f38, 0x35, 0x1f, 0x42);
    D_001548bc = fn_000c960c(D_00153f38, 0x17, 0x43, 0x10);
    D_001548bd = fn_000c960c(D_00153f38, 0x26, 0x29, 0x2f);
    D_001548be = fn_000c960c(D_00153f38, 0, 0x26, 0x56);
    D_001548bf = fn_000c960c(D_00153f38, 0x24, 0x50, 0x2f);
    D_001548c0 = fn_000c960c(D_00153f38, 0x3f, 0x1e, 0x1e);
}

// NONMATCHING: stack slots of x/y and the return value; register choice for e loads
// FUNCTION: 0x00019918
u16 fn_00019918(Entity *e, u16 pos)
{
    int x = pos % 128;
    int y = pos / 128;
    Path path;

    if (e->fc == 6) {
        fn_00055454(&path, e->next % 128, e->next / 128, x, y);
    } else {
        fn_00055454(&path, e->x, e->y, x, y);
    }
    return path.result;
}

// FUNCTION: 0x000199e8
void fn_000199e8(Entity *e)
{
    fn_00015b68(e);
    fn_000bc0b0(e, 0, sizeof(Entity));
}

// NONMATCHING: register allocation around the shift mask
// FUNCTION: 0x00019a1c
void fn_00019a1c(u16 player)
{
    Hospital *h = D_001547b4 + player;
    Tile *t;

    if (h->f2a != 2) {
        h->f1f1 = 0;
        for (t = D_00153f78; t < D_00153f7c; t++) {
            if (t->f7 & (1 << player)) {
                h->f1f1++;
            }
        }
    }
}

// FUNCTION: 0x00019aac
Entity *fn_00019aac(Tile *t, u16 kind)
{
    Entity *e;

    if (!(t < D_00153f78 || t >= D_00153f7c)) {
        for (e = D_001549a4 + t->first; e > D_001549a4; e = D_001549a4 + e->f2) {
            if (e->fc == kind) {
                return e;
            }
        }
    }
    return 0;
}

// FUNCTION: 0x00019b44
Entity *fn_00019b44(Tile *t, u16 id)
{
    Entity *e;

    if (!(t < D_00153f78 || t >= D_00153f7c)) {
        for (e = D_001549a4 + t->first; e > D_001549a4; e = D_001549a4 + e->f2) {
            if (e->fc == 8 && e->f59 == id) {
                return e;
            }
        }
    }
    return 0;
}

// NONMATCHING: register allocation in the byte-swap stores
// FUNCTION: 0x00019bec
void fn_00019bec(void)
{
    u8 palette[0x300];
    char path[0x80];
    int i;
    u8 *p;
    int n;
    s32 handle;
    int j;
    char *fmt;
    u8 tmp;

    fmt = D_001201ac;
    if (D_00156b08 == 0) {
        D_00156b08 = (u8 *)fn_000bac40(400000);
        if (D_00156b08 == 0) {
            return;
        }
    }
    if (D_00156b04 == -1) {
        for (i = 0; i < 10000; i++) {
            fn_000bc13a(path, fmt, i);
            handle = fn_000be2e0((u32)path, 2);
            if (handle == -1) {
                D_00156b04 = i - 1;
                break;
            }
            fn_000be3bc(handle);
        }
    }
    n = ++D_00156b04;
    fn_000bc13a(path, fmt, n);
    if (n >= 10000) {
        return;
    }
    p = D_00156b08;
    *p = 'm';
    p++;
    *p = 'h';
    p++;
    *p = 'w';
    p++;
    *p = 'a';
    p++;
    *p = 'n';
    p++;
    *p = 'h';
    p++;
    PUT16BE(p, 4);
    PUT16BE(p, D_00188c2c);
    PUT16BE(p, D_00188c30);
    PUT16BE(p, 0x100);
    PUT16BE(p, 0x100);
    PUT16BE(p, 0x100);
    PUT16BE(p, 0x100);
    p += 0xc;
    fn_000c2210(palette);
    for (i = 0; i < 0x100; i++) {
        for (j = 0; j < 3; j++) {
            *p = palette[i * 3 + j] << 2;
            p++;
        }
    }
    fn_000bc0d0(p, (void *)D_00188c18, D_00188c2c * D_00188c30);
    p += D_00188c2c * D_00188c30;
    fn_000bc2dc(path, D_00156b08, p - D_00156b08);
}

// NONMATCHING: list index scaled with shl instead of lea (register allocation)
// FUNCTION: 0x00019ef8
Object *fn_00019ef8(u8 kind, u8 mode, u8 owner)
{
    u8 n = 0;
    Object *p;
    Object *list[30];

    for (p = D_001547a4 + 1; p < D_001547a8; p++) {
        if (p->f3 < 0 || p->owner != owner) {
            continue;
        }
        if (p->kind != kind || !(mode == 0x65 || p->f3 == 0)) {
            continue;
        }
        if (n < 30) {
            list[n++] = p;
        }
    }
    if (n == 0) {
        return 0;
    }
    return list[fn_000b0100() % n];
}

// FUNCTION: 0x00019fe0
Object *fn_00019fe0(u8 kind, u8 mode, u8 owner)
{
    Object *found = 0;
    Object *p;

    for (p = D_001547a4 + 1; p < D_001547a8; p++) {
        if (p->f3 >= 0) {
            if (p->owner == owner) {
                if (p->kind == kind) {
                    if (p->f3 == 0) {
                        return p;
                    }
                    if (mode == 0x65) {
                        return p;
                    }
                    if (mode == 0x64) {
                        found = p;
                    }
                }
            }
        }
    }
    if (mode == 0x64 && found > 0) {
        return found;
    }
    return D_001547a4;
}

// FUNCTION: 0x0001a0a0
Object *fn_0001a0a0(Entity *e, u8 kind)
{
    u16 dist;
    Object *p;
    u16 pos;
    Object *best = 0;
    u16 bestDist = 9999;

    pos = e->x + (e->y << 7);
    for (p = D_001547a4 + 1; p < D_001547a8; p++) {
        if (!(p->f3 == 0 || p->f3 == 1)) {
            continue;
        }
        if (p->owner == e->owner) {
            if (p->kind == kind) {
                dist = (pos % 128 - p->pos % 128) * (pos % 128 - p->pos % 128)
                    + (pos / 128 - p->pos / 128) * (pos / 128 - p->pos / 128);
                if (dist < bestDist) {
                    best = p;
                    bestDist = dist;
                }
            }
        }
    }
    return best;
}

// NONMATCHING: register allocation in the first items[] address computation
// FUNCTION: 0x0001a238
void fn_0001a238(u32 flags)
{
    u32 unused1;
    u32 unused2;
    u32 i;

    if (D_00155f4d <= 1) {
        if (D_00155e46 != 0) {
            fn_00088d90(fn_000b01a0() % 2 + 0xc4);
            if (flags & 2) {
                for (i = 1; i < 0x3e; i++) {
                    if (D_00129634[i].f0) {
                        D_001547b4[D_00154b58].items[i].f0 = 1;
                    }
                }
            }
            if (flags & 4) {
                for (i = 1; i < 0x3e; i++) {
                    D_00129634[i].f0 = 1;
                    D_001547b4[D_00154b58].items[i].f0 = 1;
                }
            }
            if (flags & 8) {
                fn_00050ef0(0xf, 1000, 0, D_00154b58);
            }
            if (flags & 0x10) {
                fn_00050ef0(0xf, -1000, 0, D_00154b58);
            }
        }
    }
}

// FUNCTION: 0x0001a374
void fn_0001a374(u8 mode)
{
    switch (mode) {
    case 0:
        D_001553e8 = 1;
        break;
    case 1:
    case 2:
    case 7:
        D_001553e8 = 3;
        break;
    case 10:
        D_001553e8 = 0xd;
        break;
    case 9:
        D_001553e8 = 0;
        break;
    case 11:
        D_001553e8 = 0xe;
        break;
    case 12:
        D_001553e8 = 4;
        break;
    case 14:
        D_001553e8 = 9;
        break;
    case 15:
        D_001553e8 = 0xc;
        break;
    case 16:
        D_001553e8 = 9;
        break;
    case 18:
        D_001553e8 = 0xf;
        break;
    case 20:
        D_001553e8 = 0xf;
        break;
    case 21:
        D_001553e8 = 1;
        break;
    case 22:
        D_001553e8 = 0x1a;
        break;
    case 23:
        D_001553e8 = 0x10;
        break;
    case 24:
        D_001553e8 = 0xb;
        break;
    case 25:
        D_001553e8 = 3;
        break;
    case 26:
        D_001553e8 = 0x18;
        break;
    case 27:
        D_001553e8 = 5;
        break;
    case 28:
        D_001553e8 = 0x12;
        break;
    case 29:
        D_001553e8 = 1;
        break;
    case 30:
        D_001553e8 = 0x16;
        break;
    case 31:
        D_001553e8 = 9;
        break;
    case 32:
        D_001553e8 = 9;
        break;
    case 33:
        D_001553e8 = 0x20;
        break;
    case 34:
        D_001553e8 = 9;
        break;
    case 36:
        D_001553e8 = 7;
        break;
    case 38:
        D_001553e8 = 0x14;
        break;
    case 40:
        D_001553e8 = 0x1c;
        break;
    case 41:
        D_001553e8 = 0x1e;
        break;
    default:
        D_001553e8 = 1;
        break;
    }
    D_0015536e = mode;
    if (!D_00160400) {
        fn_0001aa8c(D_001553e8);
    } else {
        D_00160400 = 0;
    }
}

// FUNCTION: 0x0001a5f4
void fn_0001a5f4(u32 palette, u8 *out)
{
    u32 v;
    u8 *src;
    u8 i;
    u8 c;

    for (i = 0xff, src = (u8 *)palette; i != 0; i--, src += 3) {
        v = fn_000c9abc(src[0] * src[0] + src[1] * src[1]);
        v = fn_000c9abc(v * v + src[2] * src[2]);
        v = (v * 0x3f + 0x36) / 0x6d;
        c = fn_000c960c(palette, (u8)v, (u8)v, (u8)v);
        *out++ = c;
    }
}

// FUNCTION: 0x0001a6cc
u8 fn_0001a6cc(u16 from, u16 to)
{
    Path path;

    fn_00055454(&path, from % 128, from / 128, to % 128, to / 128);
    return path.result != 0;
}

// FUNCTION: 0x0001a764
u8 fn_0001a764(Walker *a, Walker *b)
{
    if (b->y > a->y) {
        return 0;
    }
    if (b->x < a->x) {
        return 1;
    }
    if (b->y < a->y) {
        return 2;
    }
    if (b->x > a->x) {
        return 3;
    }
    return a->f11;
}

// FUNCTION: 0x0001a7dc
void fn_0001a7dc(u16 pos, s8 *x, s8 *y)
{
    s8 lx = pos % 128;
    s8 ly = pos / 128;

    if (lx < -0x40) {
        lx += 0x80;
        ly += 0xff;
    }
    if (lx > 0x40) {
        lx += 0x80;
        ++ly;
    }
    *x = lx;
    *y = ly;
}

// NONMATCHING: stack slot order of shape/x/y; register allocation of bounds stores
// FUNCTION: 0x0001a854
void fn_0001a854(void)
{
    s8 x;
    s8 y;
    s8 maxY;
    s8 maxX;
    s8 minY;
    s8 minX;
    Shape *shape;
    u8 i;
    u8 j;
    u8 k;

    for (i = 0; i < 0x3e; i++) {
        for (j = 0; j < 4; j++) {
            shape = D_00129f34 + D_0012961a[i].shapes[j];
            minX = 100;
            minY = 100;
            maxX = -100;
            maxY = -100;
            for (k = 0; k < shape->count; k++) {
                fn_0001a7dc(shape->points[k], &x, &y);
                minX > x ? minX = x : 0;
                minY > y ? minY = y : 0;
                maxX < x ? maxX = x : 0;
                maxY < y ? maxY = y : 0;
            }
            D_00155f4f[i][j].minX = minX;
            D_00155f4f[i][j].minY = minY;
            D_00155f4f[i][j].maxX = maxX;
            D_00155f4f[i][j].maxY = maxY;
        }
    }
}

// FUNCTION: 0x0001aa10
int fn_0001aa10(SortEntry *a, SortEntry *b)
{
    return -(a->key - b->key);
}

// FUNCTION: 0x0001aa50
int fn_0001aa50(SortEntry *a, SortEntry *b)
{
    return a->key - b->key;
}

// NONMATCHING: register allocation in the cursors[idx] address computations
// FUNCTION: 0x0001aa8c
void fn_0001aa8c(u8 idx)
{
    u8 *kinds;
    Cursor *cursors;

    if (D_00155e1d) {
        kinds = D_0012cf18;
        cursors = D_00153f08;
        D_001291c4 = D_0012cedc;
    } else {
        kinds = D_0012cf00;
        cursors = D_00153f2c;
        D_001291c4 = D_0012cec4;
    }
    D_001553e9 = D_001553e8;
    D_001553e8 = idx;
    switch (kinds[idx]) {
    case 1:
        fn_000bd400(cursors + D_001553e8, 0, 0);
        break;
    case 2:
        fn_000bd400(cursors + D_001553e8, cursors[idx].f4 / 2, cursors[idx].f5 / 2);
        break;
    }
}

// FUNCTION: 0x0001ab90
u16 fn_0001ab90(Entity *e, u16 target)
{
    u16 best = 0xffff;
    u16 cand;
    u16 result = 0;
    u16 pos;
    u16 dist;
    u8 i;

    pos = e->x + (e->y << 7);
    for (i = 0; i < 4; i++) {
        cand = target + D_001291d0[i];
        dist = ABS(pos % 128 - cand % 128) + ABS(pos / 128 - cand / 128);
        if (dist < best && fn_00019918(e, cand)) {
            best = dist;
            result = cand;
        }
    }
    return result;
}

// FUNCTION: 0x0001ad58
u8 fn_0001ad58(Entity *e, u16 pos)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if ((s16)e->x + ((s16)e->y << 7) == pos + D_001291ce[i + 1]) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: 0x0001add0
void fn_0001add0(s16 speed)
{
    u8 i;

    for (i = 0; i < 5; i++) {
        D_00155df2[i] = 0;
    }
    switch (speed) {
    case -1:
        D_00155378 = 0;
        D_001547e0[1]--;
        D_00155379 = 0;
        if (D_001547e0[1] < 1) {
            D_001547e0[1] = 1;
        } else if (D_001547e0[1] > 0x7d00) {
            D_001547e0[1] = 0x7d00;
        }
        D_00155df3 = 1;
        break;
    case 1:
        D_00155378 = 0;
        D_001547e0[1]++;
        D_00155379 = 0;
        if (D_001547e0[1] < 1) {
            D_001547e0[1] = 1;
        } else if (D_001547e0[1] > 0x7d00) {
            D_001547e0[1] = 0x7d00;
        }
        D_00155df5 = 1;
        break;
    case 10:
        D_00155378 = 0;
        D_001547e0[1] = 1;
        D_00155379 = 0;
        D_00155df2[0] = 1;
        break;
    case 20:
        D_00155378 = 0;
        D_001547e0[1] = 5;
        D_00155379 = 0;
        D_00155df3 = 1;
        break;
    case 30:
        D_00155378 = 0;
        D_001547e0[1] = 0x16;
        D_00155379 = 0;
        D_00155df4 = 1;
        break;
    case 40:
        D_00155378 = 0;
        D_001547e0[1] = 0x19;
        D_00155379 = 0;
        D_00155df5 = 1;
        break;
    case 50:
        D_00155378 = 1;
        D_001547e0[1] = 0x7d00;
        D_00155379 = 1;
        D_00155df6 = 1;
        break;
    }
}

// FUNCTION: 0x0001afd8
void fn_0001afd8(void)
{
    u8 idle = fn_0004e154(D_001603dc) == 0 && D_001549ae == 0 && !D_001569fb;

    if (idle && !D_00155c69) {
        D_00155c6a = D_00155c5a;
    }
    D_00155c69 = idle;
}

// FUNCTION: 0x0001b050
int fn_0001b050(Object *r)
{
    Entity *q;
    int score;
    int base;
    int count = 0;

    if (r <= D_001547a4 || r >= D_001547a8) {
        return 0;
    }
    base = (r->f1 + r->f2) * 5;
    for (q = D_001549a4 + r->fc; q > D_001549a4; q = D_001549a4 + q->f47) {
        count++;
    }
    count -= D_0012c198[r->kind].fc;
    count += r->f1b / 2;
    score = base + count * 5;
    if (score < 0) {
        score = 0;
    }
    return score;
}

// FUNCTION: 0x0001b150
Entity *fn_0001b150(Room *room, u16 kind, u16 sub)
{
    Entity *q;

    if (room) {
        for (q = D_001549a4 + room->fe; q > D_001549a4; q = D_001549a4 + q->next) {
            if (q->fc == kind) {
                if (q->fe == sub) {
                    return q;
                }
            }
        }
    }
    return 0;
}

// NONMATCHING: register allocation: value/address registers swapped in two stores
// FUNCTION: 0x0001b1f8
void fn_0001b1f8(void)
{
    if (D_0015536e == 0xe) {
        D_001549d0[D_00154b58].f1 = 0x7b;
        return;
    }
    switch (D_0015819a) {
    case 10:
        if (D_001581a1) {
            D_001549d0[D_00154b58].f1 = 0x5c;
            D_001549d0[D_00154b58].f2 = 1;
            D_00155d7d = 1;
            return;
        }
        break;
    case 5:
        if (D_00158198 == 0) {
            if (D_00155d7d == 1) {
                fn_000bc13a(D_00155ced, D_001201c2, D_00160324[4]);
                fn_0006fd98(0x79, 0, 0, 0x78, 1, 1);
                return;
            }
            D_001549d0[D_00154b58].f1 = 0x79;
            return;
        }
        if (D_001581a0 != 3) {
            D_001549d0[D_00154b58].f1 = 0x45;
            return;
        }
        break;
    case 12:
        fn_0006c5b8();
        return;
    }
    D_001549d0[D_00154b58].f1 = 0x38;
    if (D_001552b5) {
        D_001549d0[D_00154b58].f2 = D_001552b5;
    } else {
        D_001549d0[D_00154b58].f2 = 0;
    }
    D_001549d0[D_00154b58].f5 = D_0015536e;
}

// FUNCTION: 0x0001b388
void fn_0001b388(u32 a)
{
    fn_000c9b40(a, D_00188c18, D_00188c30);
}

// FUNCTION: 0x0001b3b8
void fn_0001b3b8(u32 a)
{
    fn_000c9b40(D_00188c18, a, D_00188c30);
}

// FUNCTION: 0x0001b3e8
void fn_0001b3e8(u32 n)
{
    if (D_0015819a != n) {
        switch (D_0015819a) {
        case 9:
        case 11:
            fn_0001bdb8(D_0015819a);
            break;
        case 10:
            if (!D_001581a1) {
                fn_0002812c(D_001553c7);
                fn_0001bdb8(D_0015819a);
            }
            break;
        }
    }
}

// FUNCTION: 0x0001b474
Entity *fn_0001b474(u32 n)
{
    Entity *e;

    for (e = D_001549a4 + 1; e < D_001549a8 - n; e++) {
        if (e->fc == 0) {
            return e;
        }
    }
    return 0;
}

// FUNCTION: 0x0001b4e0
int fn_0001b4e0(s16 x, s16 y, s16 rx, s16 ry, s16 w, s16 h)
{
    return (x >= rx && x < rx + w) && (y >= ry && y < ry + h);
}

// FUNCTION: 0x0001b554
void fn_0001b554(s16 x, s16 y, s16 w, s16 h, u8 color)
{
    fn_000c9be0(x, y, x + w, y, color);
    fn_000c9be0(x, y, x, y + h, color);
    fn_000c9be0(x + w, y, x + w, y + h, color);
    fn_000c9be0(x, y + h, x + w, y + h, color);
}

// NONMATCHING: register allocation in the Movie table loads
// FUNCTION: 0x0001b610
u8 fn_0001b610(u8 idx)
{
    u8 wasPlaying = D_00188c18 != 0;
    u8 ok;

    ok = 1;
    if (wasPlaying) {
        fn_000bed5c();
    }
    if (D_0015f3f5 == TOGGLE_1 && D_0015f3f4 == SCREEN_MODE_1 && idx != 0x16 && idx != 0x17) {
        if (!fn_00012068(SCREEN_MODE_13)) {
            fn_00010b20();
            fn_000c2111(1);
        }
        fn_000becd0(0);
        fn_000bee60();
    }
    if (D_0013eedc[idx].f28 > D_0015f980) {
        fn_000bc13a(D_0015fde4, D_001201c5, D_0015f900, D_0013eedc[idx].name);
        fn_0009b5b8(D_0015fde4);
    } else {
        fn_000bbff0(D_0015fde4, D_0013eedc[idx].name);
    }
    if (fn_000be290(D_0015fde4)) {
        ok = fn_000ca360(D_0015fde4, D_0013eedc[idx].f32, D_0013eedc[idx].f36);
    } else {
        ok = 0;
    }
    fn_000bac20();
    if (wasPlaying) {
        while (fn_000bed50() != 1) {
        }
    }
    D_00188c7b = D_00188c81 = 0;
    D_00188c7c = D_00188c83 = 0;
    D_00188c7e = D_00188c7f = 0;
    return ok;
}

// FUNCTION: 0x0001b7a4
void fn_0001b7a4(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fn_000caa79(fmt, args);
    va_end(args);
    fn_000caa8a(D_0014dbbe);
}

// NONMATCHING: register allocation: stored character kept in eax in target
// FUNCTION: 0x0001b7e0
void fn_0001b7e0(char *dst, s8 *src, u8 mode)
{
    if (mode == 0) {
        for (; *src; src++, dst++) {
            *dst = fn_0001b874(*src);
        }
        *dst = 0;
    } else if (mode == 1) {
        for (; *src; src++, dst++) {
            *dst = fn_000caa9a(*src);
        }
        *dst = 0;
    }
}

// FUNCTION: 0x0001b874
int fn_0001b874(u8 c)
{
    if (c < 0x80) {
        return fn_000caaa8(c);
    }
    if (c > 0xa5) {
        return c;
    }
    switch (c) {
    case 0x81:
        return 0x9a;
    case 0x82:
        return 0x90;
    case 0x83:
        return 0x41;
    case 0x84:
        return 0x8e;
    case 0x85:
        return 0x41;
    case 0x86:
        return 0x8f;
    case 0x87:
        return 0x80;
    case 0x88:
        return 0x45;
    case 0x89:
        return 0x45;
    case 0x8a:
        return 0x45;
    case 0x8b:
        return 0x49;
    case 0x8c:
        return 0x49;
    case 0x8d:
        return 0x49;
    case 0x93:
        return 0x4f;
    case 0x94:
        return 0x99;
    case 0x95:
        return 0x4f;
    case 0x96:
        return 0x55;
    case 0x97:
        return 0x55;
    case 0x98:
        return 0x59;
    case 0xa0:
        return 0x41;
    case 0xa1:
        return 0x49;
    case 0xa2:
        return 0x4f;
    case 0xa3:
        return 0x55;
    case 0xa4:
        return 0xa5;
    default:
        return c;
    }
}

// FUNCTION: 0x0001bab4
void fn_0001bab4(u8 i)
{
    int y;
    int x;

    if (i < 12) {
        fn_000ac264(D_001567a7, D_00188c34 / 2, D_00188c38 / 2);
        if (D_00156498 < 0) {
            x = 0;
        } else if (D_00156498 >= 0x80) {
            x = 0x7f;
        } else {
            x = D_00156498;
        }
        if (D_0015649c >= 0x80) {
            y = 0x7f;
        } else if (D_0015649c < 0) {
            y = 0;
        } else {
            y = D_0015649c;
        }
        D_0015f984[i] = x + (y << 7);
    }
}

// NONMATCHING: register allocation in the D_0015f984 loads
// FUNCTION: 0x0001bb8c
void fn_0001bb8c(u8 i)
{
    if (i < 12) {
        if (D_0015f984[i]) {
            fn_000abbc8(0, D_0015f984[i] % 128, D_0015f984[i] / 128);
        }
    }
}

// FUNCTION: 0x0001bc08
void fn_0001bc08(void)
{
    u8 i;

    for (i = 0; i < 12; i++) {
        D_0015f984[i] = 0;
    }
}

// FUNCTION: 0x0001bc48
void fn_0001bc48(u32 a, u8 b, s8 c)
{
    if (D_00155f4d > 1) {
        fn_000c9400(a, 1, c);
    } else {
        fn_000c9400(a, b, c);
    }
}

// NONMATCHING: register allocation in the D_0015f3fa[idx] load
// FUNCTION: 0x0001bc94
void fn_0001bc94(u8 idx, u8 flag)
{
    char *name;
    char path[20];
    int size;

    name = D_0015f3fa[idx];

    if (D_0015f3f4 == SCREEN_MODE_13) {
        if (idx != D_001547a0) {
            fn_000bbff0(path, name);
            fn_000c93bf(path, D_001201ca);
            size = fn_000bc170(path);
            if (D_001603f8 - D_001603f4 < size) {
                return;
            }
            if (fn_000bc23c(path, D_001603f4) != size) {
                return;
            }
            fn_000bbff0(path, name);
            fn_000c93bf(path, D_001201cf);
            size = fn_000bc170(path);
            if (D_001603f0 - D_001603ec < size) {
                return;
            }
            if (fn_000bc23c(path, D_001603ec) != size) {
                return;
            }
            fn_000bc86c(D_001603f4, D_001603f8, D_001603ec);
            D_001547a0 = idx;
        }
    }
    if (flag) {
        fn_0004e0c0(D_001603dc, D_0015f414, idx);
    }
}

// FUNCTION: 0x0001bdb8
void fn_0001bdb8(u8 i)
{
    fn_0004c9dc(D_001603dc, D_0015f414 + i);
    D_0015819a = 0x63;
    fn_0009ab54();
}

// FUNCTION: 0x0001bdf8
void fn_0001bdf8(void)
{
    fn_0001b7a4(D_001201d4);
    fn_0001b7a4(D_00120202);
    fn_0001b7a4(D_00120218, 3);
    if (D_001581ec) {
        fn_0001b7a4(D_0012022d, D_001581ef);
    }
    if (D_001581ed) {
        fn_0001b7a4(D_0012023e, D_001581ef);
    }
}

// FUNCTION: 0x0001bee4
void fn_0001bee4(void)
{
    if (D_001290cc != -1) {
        fn_000be3bc(D_001290cc);
        D_001290cc = -1;
    }
    if (D_0015f3f4 == SCREEN_MODE_1 || D_00155e45) {
        D_001290cc = fn_000be2e0(D_001289a4, 2);
        if (D_001290cc == -1) {
            D_0015f982 = 9;
            D_0015f983 = 8;
            fn_00010b20();
            fn_000c2111(1);
        }
        D_0015f454 = D_0014bcc4;
        D_0015f458 = D_0014bcd0;
        D_0015f45c = D_0014bcd8;
        D_0015f460 = D_0014bce0;
        D_0015f44c = D_0014bce4;
        D_0015f450 = D_0014bce8;
    } else if (D_0015f3f4 == SCREEN_MODE_13) {
        D_001290cc = fn_000be2e0(D_001289a0, 2);
        if (D_001290cc == -1) {
            D_0015f982 = 9;
            D_0015f983 = 7;
            fn_00010b20();
            fn_000c2111(1);
        }
        D_0015f454 = D_0013c60c;
        D_0015f458 = D_0013c618;
        D_0015f45c = D_0013c620;
        D_0015f460 = D_0013c628;
        D_0015f44c = D_0013c62c;
        D_0015f450 = D_0013c630;
    }
}

}
