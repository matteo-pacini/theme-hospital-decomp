#include "bflib.h"

typedef char *va_list[1];
#define va_start(ap, last) ((ap)[0] = (char *)&(last) + sizeof(last))
#define va_end(ap) ((ap)[0] = 0)

extern "C" {

int vsprintf(char *buf, char *fmt, va_list args);
u32 LbScreenIsInitialised(void);
int LbIsIKeyboardInstalled(void);
void LbKeyboardClose(void);
void LbDrawCheapText(int x, int y, char *text, int colour);
int getch(void);
char *strcat(char *dst, const char *src);
void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(strcat, memcpy)

// Does nothing under DOS. Returns 1.
// FUNCTION: 0x000d0800
int LbWindowsControl(void)
{
    return 1;
}

}

// printf-style debug output to the console (512-byte buffer).
// FUNCTION: 0x000d080c
void LbDebugString(char *fmt, ...)
{
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    printf(buf);
}

// Reports a failed library assertion (expr at file:line) on screen or the
// console and waits for K (exit the program) or C (continue). Returns 0.
// FUNCTION: 0x000d0848
int LbAssertMessage(const char *expr, const char *file, unsigned long line)
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
        LbDrawBox(0, 0, lbDisplay.physical_screen_width, 72, 0);
        y += 12;
        LbDrawCheapText(x, y, "Warning a library assertion occured:", 1);
        sprintf(text, "%s", expr);
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        sprintf(text, "In   : %s\nline : %u", file, line);
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        sprintf(text, "Enter 'K' to Kill the Application or 'C' to Continue : ");
        y += 12;
        LbDrawCheapText(x, y, text, 1);
        LbScreenSwap();
    } else {
        sprintf(title, "\nWarning a library assertion occured:");
        sprintf(text, "%s\n%s\n\nIn   : %s\nline : %u", title, expr, file, line);
        strcat(text, "\n\nEnter 'K' to Kill the Application or 'C' to Continue : ");
        printf("%s\n", text);
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
