#include "hosp.h"

extern const char *hiresscreens[];
extern const char *screenpalettes[];
extern TbLoadFiles hiresratfestpanelfiles[];

extern "C" {
extern TbScreenMode D_0015f3f4;
extern u8 D_00155e45;
extern u8 *D_00153f34;
extern u16 D_00188c74;
extern u8 D_001547e5;
extern u8 D_001553c8;
extern char *D_0013e80c;
extern char D_00120c60[];

int __far fn_0009b590(unsigned deverr, unsigned errcode, unsigned __far *devhdr);
void fn_000ce546(int (__far *handler)(unsigned, unsigned, unsigned __far *));
char *strchr(const char *s, int c);
int LbFileFindFirst(char *spec, void *find, u32 attributes);
int fn_000bed50(void);
void fn_0001102c(void);
}

// FUNCTION: 0x0009b5b8
void check_cd_available(char *path)
{
    s32 buf;
    s32 handle;
    char *text;
    s32 unused;
    u8 had_screen;
    TbScreenMode mode;
    u8 flag;
    u8 find[0xe4];
    char fname[0x80];

    had_screen = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    mode = D_0015f3f4;
    flag = D_00155e45;
    unused = 0;
    if (*path == '*')
        return;
    fn_000ce546(fn_0009b590);
    handle = LbFileOpen(path, 2);
    if (handle != -1) {
        if (LbFileRead(handle, &buf, 4) != -1) {
            LbFileClose(handle);
            return;
        }
    }
    if (strchr(path, '*')) {
        if (LbFileFindFirst(path, find, 0x21) != -1)
            return;
    }
    if (had_screen)
        LbIsActive();
    if (D_0015f3f4 != SCREEN_MODE_13 || D_00155e45)
        setup_any_screen(D_00153f34, SCREEN_MODE_13);
    LbFileLoadAt((char *)screenpalettes[21], D_00153f34);
    while (fn_000bed50() != 1)
        ;
    LbFileLoadAt((char *)hiresscreens[21], lbDisplay.unknown4);
    D_00188c74 = 0;
    set_font_from_cache(0x2f);
    text = ((char **)((u8 *)hiresratfestpanelfiles + 0x66))[D_001547e5];
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbTextDraw((lbDisplay.unknown18 - LbTextStringWidth(text)) / 2, 0x1a4, text);
    if (lbKeyOn[0x21] && lbKeyOn[0x38])
        LbTextDraw(0, 0x1c2, path);
    LbIsActive();
    LbMouseChangeSprite(0);
    LbScreenSwap();
    LbPaletteSet(D_00153f34);
    while (!LbFileExists(path)) {
        if (lbKeyOn[1]) {
            reset_host();
            exit(1);
        }
    }
    if (D_0015f3f4 != mode)
        setup_any_screen(D_00153f34, mode);
    if (flag)
        fn_0001102c();
    if (had_screen) {
        while (fn_000bed50() != 1)
            ;
    }
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    D_001553c8 = 1;
    if (install_type < 2) {
        sprintf(fname, D_00120c60, cd_install_path, D_0013e80c);
        check_cd_available(fname);
        LbFileLoadAt(fname, D_00153f34);
    } else {
        LbFileLoadAt(D_0013e80c, D_00153f34);
    }
}
