#include "hosp.h"

struct Message {
    char text[200];
};

extern "C" {

extern u8 D_0015f982;
extern u8 D_0015f3f9;
extern TbScreenMode D_0015f3f4, D_0015f3f5, D_0015f3f6;
extern u8 D_0015f3f7, D_0015f3f8, D_0015f40a, D_0015f40b;
extern u8 *D_0015f40c, *D_0015f410, *D_0015f414, *D_0015f418, *D_0015f438;
extern u8 *D_0015f41c, *D_0015f428, *D_0015f3fa, *D_0015f402, *D_0015f406, *D_0015f42c;
extern u8 *D_0015f444;
extern int *D_0015f420;
extern int *D_0015f3fe;
extern View *D_0015f430;
extern View D_0014619c;
extern Point *D_0015f434;
extern Point D_00146c8c[];
extern Button *D_0015f43c;
extern Button D_00145378[];
extern u8 D_0013bb0c[], D_0013c0fc[], D_001361ac[], D_001310e4[], D_00145358[];
extern u8 D_0013c6a4[], D_00130288[], D_0013e5fc[], D_0014a620[], D_0014a9fe[], D_0012cc3c[];
extern u8 D_00147314[];
extern int D_0013c77c[], D_0014be34[];
extern int D_0013e634[], D_0013e6f4[];
extern TbLoadFiles D_0013e9c8[], D_0013e7b4[], D_0013e22c[], D_0013e2b8[];
extern TbLoadFiles D_0013ea78[], D_0013e944[], D_0013ec5c[];
extern u8 *D_00153f34;
extern TbSprite *D_00153f30;
extern u8 *D_00153f28;
extern char D_00120028[], D_0012002d[], D_0012003b[], D_00120049[];
extern char D_0012005b[], D_00120073[], D_00120087[], D_001200a9[], D_001200ce[], D_001200e9[];
extern char D_00120102[], D_00120129[], D_00120142[], D_0012015d[];
extern char *D_0013e80c;
extern u8 *D_001553d7;
extern u8 D_001581ec, D_001581ed, D_001581ee, D_001581ef;
extern s32 D_001581e8;
extern s32 D_001290cc;
extern char *D_001289a0;
extern Message D_00129004;
extern void *D_00158334;
extern u8 D_001547e5;
extern u8 D_00155df8, D_00155df9;

void fn_000bce44(TbLoadFiles *files);
void fn_0008bcb4(void);
void fn_000c20f0(void);
int fn_000bea80(int mode, ...);
void fn_0001102c(void);
void fn_00011058(void);
void fn_00011084(void);
void fn_000110b0(void);

}

// FUNCTION: 0x000107c0
u8 setup_host(void)
{
    char path[128];
    char name[52];

    D_0015f982 = 0;
    D_0015f3f9 = 0;
    fn_000bce44(D_0013e9c8);
    fn_000bce44(D_0013e7b4);
    if (!setup_any_screen(D_00153f34, SCREEN_MODE_13)) {
        return 1;
    }
    if (setup_lang() == 0) {
        D_0015f982 = 1;
        return 1;
    }
    if (install_type < 2) {
        sprintf(path, D_00120028, cd_install_path, D_0013e80c);
        check_cd_available(path);
        LbFileLoadAt(path, D_00153f34);
    } else {
        LbFileLoadAt(D_0013e80c, D_00153f34);
    }
    if (debug.unknown1fa) {
        load_game_files();
    }
    fn_000bce44(D_0013e22c);
    LbSpriteSetup(pointer, D_00153f30, D_00153f28);
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
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
    if (!display_loading_screen()) {
        return 0;
    }
    D_0015f982 = setup_sound();
    if (D_0015f982) {
        return 1;
    }
    fn_000bce44(D_0013ea78);
    fn_000bce44(D_0013e944);
    SWIKeyboardOpen();
    if (LbMouseSetup(pointer + 1) != 1) {
        D_0015f982 = 2;
        return 1;
    }
    LbMouseChangeSprite(0);
    LbDataLoadAll(D_0013ec5c);
    D_001553d7 = 0;
    D_001581ee = 0;
    if (D_001581ec) {
        sprintf(name, D_0012002d, D_001581ef);
        D_001581e8 = LbFileOpen(name, 0);
        if (D_001581e8) {
            D_001581ee |= 1;
        }
    } else if (D_001581ed) {
        sprintf(name, D_0012003b, D_001581ef);
        D_001581e8 = LbFileOpen(name, 2);
        if (D_001581e8) {
            D_001581ee |= 2;
        }
    }
    debug.unknown1e0 = -1;
    LbGhostTableGenerate(D_00153f34, 0x42, D_00120049);
    generate_bw_remap(D_00153f34, bw_remap);
    clear_loading_screen(D_00153f34);
    D_001290cc = LbFileOpen(D_001289a0, 2);
    if (D_001290cc == -1) {
        D_0015f982 = 9;
        sub_error_number = 7;
        return 1;
    }
    version = 3;
    return 0;
}

// FUNCTION: 0x00010b20
void reset_host(void)
{
    Message msg = D_00129004;

    if (lbDisplay.unknown4) {
        LbIsActive();
    }
    LbNetwork_Stop();
    LbMouseReset();
    SWIKeyboardClose();
    LbScreenReset();
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
        hosp_dos_printf(D_00120102);
        break;
    case 9:
        hosp_dos_printf(D_00120129);
        break;
    case 10:
        hosp_dos_printf(D_00120142);
        break;
    }
    if (sub_error_number) {
        sprintf(msg.text, D_0012015d, msg.text, sub_error_number);
    }
    if (msg.text[0]) {
        hosp_dos_printf(msg.text);
    }
    switch (D_0015f3f4) {
    case SCREEN_MODE_1:
        LbDataFreeAll(D_0013e2b8);
        break;
    case SCREEN_MODE_13:
        LbDataFreeAll(D_0013e22c);
        break;
    }
    unload_game_files();
    if (D_001581ee & 3) {
        LbFileClose(D_001581e8);
    }
    close_sound_heap();
    fn_000c20f0();
    LbMemoryFree(midi_buffer);
    LbMemoryFree(D_00158334);
    LbMemoryFree(text_starts);
    LbDataFreeAll(D_0013ec5c);
    LbDataFreeAll(D_0013ea78);
    LbDataFreeAll(D_0013e944);
    LbDataFreeAll(D_0013e9c8);
    LbDataFree(&langfile[D_001547e5]);
    LbDataFreeAll(D_0013e7b4);
    if (D_001290cc != -1) {
        LbFileClose(D_001290cc);
    }
    LbMemoryReset();
}

// FUNCTION: 0x00010dbc
void get_input(void)
{
    HospShift = 0;
    if (lbKeyOn[0x2a] || lbKeyOn[0x36]) {
        HospShift |= 1;
    }
    if (lbKeyOn[0x38] || lbKeyOn[0xb8]) {
        HospShift |= 4;
    }
    if (lbKeyOn[0x1d] || lbKeyOn[0x9d]) {
        HospShift |= 2;
    }
}

// FUNCTION: 0x00010e24
void load_game_files(void)
{
    if (D_0015f3f9 == 2) {
        return;
    }
    if (D_0015f3f5 == SCREEN_MODE_1) {
        fn_000bce44(loresblockfiles);
        fn_000bce44(mcgaspritefiles);
        fn_000bce44(mcgareqfiles);
        fn_000bce44(loresanifiles);
        setup_panel_icons(1);
        fn_0008bcb4();
    } else {
        fn_000bce44(hiresspritefiles);
        fn_000bce44(hiresblockfiles);
        fn_000bce44(reqfiles);
        loaded_requester = -1;
        fn_000bce44(hiresanifiles);
        setup_panel_icons(13);
    }
    D_0015f3f6 = D_0015f3f5;
    setup_game_sprites();
    D_0015f3f9 = 2;
}

// FUNCTION: 0x00010ee0
void unload_game_files(void)
{
    if (D_0015f3f9 == 2) {
        if (D_0015f3f5 == SCREEN_MODE_1) {
            LbDataFreeAll(loresblockfiles);
            LbDataFreeAll(mcgaspritefiles);
            LbDataFreeAll(mcgareqfiles);
            LbDataFreeAll(loresanifiles);
        } else {
            LbDataFreeAll(hiresspritefiles);
            LbDataFreeAll(hiresblockfiles);
            LbDataFreeAll(reqfiles);
            LbDataFreeAll(hiresanifiles);
        }
        D_0015f3f9 = 0;
        LbPackHeap();
    }
}

// FUNCTION: 0x00010f68
u8 setup_any_screen(const u8 *const palette, TbScreenMode mode)
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
    if (fn_000bea80(mode, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height, palette, variant, 0) != 1) {
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
    setup_any_screen(D_00153f34, SCREEN_MODE_1);
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
    statvar.unknown4 = lbScreenModeInfo[SCREEN_MODE_13].height - 0x30;
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
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
        LbFileClose(D_001290cc);
        D_001290cc = -1;
    }
    D_001290cc = LbFileOpen(D_001289a0, 2);
    if (D_001290cc == -1) {
        D_0015f982 = 9;
        sub_error_number = 7;
        reset_host();
        exit(1);
    }
    D_00155df9 = 1;
    D_00155df8 = 0;
    window.unknown30 = 0;
    window.unknown48 = lbScreenModeInfo[SCREEN_MODE_13].width;
    window.unknown3c = 0;
    window.unknown54 = lbScreenModeInfo[SCREEN_MODE_13].height;
    window.unknown2c0 = 0;
    window.unknown90 = (window.unknown30 + window.unknown48) / 2;
    window.unknown9c = (window.unknown3c + window.unknown54) / 2;
    window.unknown2eb = 0;
    setup_menus();
    LbDataFreeAll(D_0013e2b8);
    fn_000bce44(D_0013e22c);
    LbSpriteSetup(pointer, D_00153f30, D_00153f28);
}
