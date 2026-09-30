#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {
extern char D_00124cf0[];
extern char D_00124d15[];
extern char D_00124d18[];
extern char D_00124d2c[];
extern char D_00124d64[];
extern char D_00124d8a[];
extern char D_00124da5[];
extern char D_00124ddf[];

int vsprintf(char *buf, char *fmt, va_list args);
u32 LbScreenIsInitialised(void);
int LbIsIKeyboardInstalled(void);
void LbKeyboardClose(void);
void LbDrawCheapText(int x, int y, char *text, int colour);
int getch(void);
char *strcat(char *dst, const char *src);
void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(strcat, memcpy)

// FUNCTION: 0x000d0800
int fn_000d0800(void)
{
    return 1;
}

// FUNCTION: 0x000d080c
void fn_000d080c(char *fmt, ...)
{
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    printf(buf);
}

// FUNCTION: 0x000d0848
int fn_000d0848(char *expr, char *file, int line)
{
    char text[512];
    int cont;
    u16 y;
    u16 x;
    int screen;
    u8 palette[768];
    u8 old_colours[6];
    u8 key;
    char title[512];
    int keyboard;
    int restore;

    cont = 0;
    keyboard = LbIsIKeyboardInstalled();
    screen = LbScreenIsInitialised();
    restore = 0;
    if (screen) {
        LbPaletteGet(palette);
        memcpy(old_colours, palette, 6);
        memset(palette, 0, 3);
        memset(palette + 3, 0x3f, 3);
        LbPaletteSet(palette);
        restore = 1;
        x = 0;
        y = 0;
        LbDrawBox(0, 0, lbDisplay.unknown18, 72, 0);
        y += 12;
        LbDrawCheapText(x, y, D_00124cf0, 1);
        sprintf(text, D_00124d15, expr);
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        sprintf(text, D_00124d18, file, line);
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        sprintf(text, D_00124d2c);
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        LbScreenSwap();
    } else {
        sprintf(title, D_00124d64);
        sprintf(text, D_00124d8a, title, expr, file, line);
        strcat(text, D_00124da5);
        printf(D_00124ddf, text);
    }
    key = 0;
    while (!cont) {
        if (keyboard) {
            if (lbKeyOn[0x25])
                key = 'k';
            if (lbKeyOn[0x2e])
                key = 'c';
        } else {
            key = getch();
        }
        if (key == 'c' || key == 'C') {
            cont = 1;
        } else if (key == 'k' || key == 'K') {
            if (keyboard)
                LbKeyboardClose();
            if (screen)
                LbScreenReset();
            exit(1);
        }
    }
    if (restore) {
        memcpy(palette, old_colours, 6);
        LbPaletteSet(palette);
    }
    return 0;
}

}
