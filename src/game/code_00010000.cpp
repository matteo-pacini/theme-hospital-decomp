#include "common.h"

struct Sprite {
    u32 data;
    u8 width;
    s8 height;
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

void *fn_000bac40(u32 size);
void fn_000baca4(void *p);
int fn_000bc13a(char *buf, char *fmt, ...);
int fn_000bc170(char *path);
u8 fn_000bbbb0(char *path, void *buf, u32 size, int count);
void fn_000bc86c(u32 a, u32 b, u32 c);
void fn_00010f68(u32 a, int b);
void fn_00011084(void);
void fn_00010760(void);

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

// FUNCTION: 0x0001102c
void fn_0001102c(void)
{
    fn_00011084();
    fn_00010f68(D_00153f34, 1);
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

}
