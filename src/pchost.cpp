#include "hosp.h"

extern "C" {

extern u8 error_number;
extern View ScreenBoxes;
extern Point D_00146c8c[];
extern Button screenbuttons[];
extern u8 D_0013bb0c[], D_0013c0fc[], window_list[], button_list[], screenlistpos[];
extern u8 hiresbuttons[], panel_popup_coord[], D_0012cc3c[];
extern u8 D_00147314[];
extern int hireshelp[], D_0014be34[];
extern u8 *palette;
extern TbSprite *end_pointers;
extern u8 *pointer_data;
extern char *D_0013e80c;
extern u8 D_001581ec, D_001581ed, D_001581ee, D_001581ef;
extern s32 D_001581e8;
extern void *heapptr;

void LbDataLoadAll(TbLoadFiles *files);
void setup_mcga_elements(void);
void FreeAudio(void);
int LbScreenSetupAnyMode(int mode, ...);
void fn_0001102c(void);
void use_hires_help_and_fonts(void);
void use_lores_help_and_fonts(void);
void fn_000110b0(void);

}

// Starts up the machine side: the 640x480 screen and palette, the language
// files, sound, keyboard, mouse, the data files and the faces file, and opens
// the PLAY.R<n> replay file for the -R/-P switches. Returns 1 on failure with
// error_number set, else 0 (also, early, if the loading screen cannot be
// shown).
// FUNCTION: 0x000107c0
u8 setup_host(void)
{
    char path[128];
    char name[52];

    error_number = 0;
    resvars.f5 = 0;
    LbDataLoadAll(screenfiles);
    LbDataLoadAll(palettefile);
    if (!setup_any_screen(palette, SCREEN_MODE_13)) {
        return 1;
    }
    if (setup_lang() == 0) {
        error_number = 1;
        return 1;
    }
    if (install_type < 2) {
        sprintf(path, "%s%s", cd_install_path, D_0013e80c);
        check_cd_available(path);
        LbFileLoadAt(path, palette);
    } else {
        LbFileLoadAt(D_0013e80c, palette);
    }
    if (debug.unknown1fa) {
        load_game_files();
    }
    LbDataLoadAll(hiresgamepointerfiles);
    LbSpriteSetup(pointer, end_pointers, pointer_data);
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
    resvars.f3 = 0x20;
    resvars.f4 = 0x40;
    resvars.f4 = 0x40;
    resvars.f3 = 0x20;
    resvars.f18_u8p = D_0013bb0c;
    resvars.f3c = &ScreenBoxes;
    resvars.f1c_u8p = D_0013c0fc;
    resvars.f40 = D_00146c8c;
    resvars.f20_u8p = window_list;
    resvars.f24 = button_list;
    resvars.f44 = screenlistpos;
    resvars.f28 = hiresbuttons;
    resvars.f2c_intp = hireshelp;
    resvars.f34 = panel_popup_coord;
    resvars.f1 = SCREEN_MODE_13;
    resvars.f38 = D_0012cc3c;
    resvars.f16 = 3;
    resvars.f17 = 0x20;
    resvars.f0 = SCREEN_MODE_13;
    resvars.fa_charpp = hires_font_filenames;
    resvars.f50 = D_00147314;
    resvars.f48 = screenbuttons;
    resvars.f5 = 0;
    if (!display_loading_screen()) {
        return 0;
    }
    error_number = setup_sound();
    if (error_number) {
        return 1;
    }
    LbDataLoadAll(memfiles);
    LbDataLoadAll(complexfiles);
    SWIKeyboardOpen();
    if (LbMouseSetup(pointer + 1) != 1) {
        error_number = 2;
        return 1;
    }
    LbMouseChangeSprite(0);
    LbDataLoad(thingfiles);
    othervar.f1f = 0;
    D_001581ee = 0;
    if (D_001581ec) {
        sprintf(name, "SAVE/PLAY.R%d", D_001581ef);
        D_001581e8 = LbFileOpen(name, 0);
        if (D_001581e8) {
            D_001581ee |= 1;
        }
    } else if (D_001581ed) {
        sprintf(name, "SAVE/PLAY.R%d", D_001581ef);
        D_001581e8 = LbFileOpen(name, 2);
        if (D_001581e8) {
            D_001581ee |= 2;
        }
    }
    debug.unknown1e0 = -1;
    LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
    generate_bw_remap(palette, bw_remap);
    clear_loading_screen(palette);
    face_file = LbFileOpen((char *)hires_faces, 2);
    if (face_file == -1) {
        error_number = 9;
        sub_error_number = 7;
        return 1;
    }
    version = 3;
    return 0;
}

// Shuts down everything setup_host started, freeing the data files and
// memory, and prints the message for error_number (and sub_error_number).
// FUNCTION: 0x00010b20
void reset_host(void)
{
    char error_string[200] = "";

    if (lbDisplay.unknown4) {
        LbScreenUnlock();
    }
    LbNetwork_Stop();
    LbMouseReset();
    SWIKeyboardClose();
    LbScreenReset();
    switch (error_number) {
    case 1:
        fn_000bbff0(error_string, "Language Setup Failed.\n");
        break;
    case 2:
        fn_000bbff0(error_string, "No Mouse Detected.\n");
        break;
    case 3:
        fn_000bbff0(error_string, "Failed to initialise MCGA screen\n");
        break;
    case 4:
        fn_000bbff0(error_string, "Failed to initialise VRES256 screen\n");
        break;
    case 5:
        fn_000bbff0(error_string, "Memory allocation failure\n");
        break;
    case 6:
        fn_000bbff0(error_string, "Heap allocation failure\n");
        break;
    case 8:
        hosp_dos_printf("Speech Setup Failed. Sample too Large\n");
        break;
    case 9:
        hosp_dos_printf("Error loading data file\n");
        break;
    case 10:
        hosp_dos_printf("Error couldn't change res\n");
        break;
    }
    if (sub_error_number) {
        sprintf(error_string, "%s(Reason code: %d)\n", error_string, sub_error_number);
    }
    if (error_string[0]) {
        hosp_dos_printf(error_string);
    }
    switch (resvars.f0) {
    case SCREEN_MODE_1:
        LbDataFreeAll(lorespointerfiles);
        break;
    case SCREEN_MODE_13:
        LbDataFreeAll(hiresgamepointerfiles);
        break;
    }
    unload_game_files();
    if (D_001581ee & 3) {
        LbFileClose(D_001581e8);
    }
    close_sound_heap();
    FreeAudio();
    LbMemoryFree(midi_buffer);
    LbMemoryFree(heapptr);
    LbMemoryFree(text_starts);
    LbDataFreeAll(thingfiles);
    LbDataFreeAll(memfiles);
    LbDataFreeAll(complexfiles);
    LbDataFreeAll(screenfiles);
    LbDataFree(&langfile[myplayer.f21]);
    LbDataFreeAll(palettefile);
    if (face_file != -1) {
        LbFileClose(face_file);
    }
    LbMemoryReset();
}

// Sets HospShift from the shift, alt and control keys held (MOD_SHIFT,
// MOD_ALT, MOD_CTRL).
// FUNCTION: 0x00010dbc
void get_input(void)
{
    HospShift = 0;
    if (lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT]) {
        HospShift |= MOD_SHIFT;
    }
    if (lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT]) {
        HospShift |= MOD_ALT;
    }
    if (lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL]) {
        HospShift |= MOD_CTRL;
    }
}

// Loads the game's block, sprite, requester and animation files for the
// current resolution (resvars.f1), unless already loaded, and lays out the
// panel.
// FUNCTION: 0x00010e24
void load_game_files(void)
{
    if (resvars.f5 == 2) {
        return;
    }
    if (resvars.f1 == SCREEN_MODE_1) {
        LbDataLoadAll(loresblockfiles);
        LbDataLoadAll(mcgaspritefiles);
        LbDataLoadAll(mcgareqfiles);
        LbDataLoadAll(loresanifiles);
        setup_panel_icons(1);
        setup_mcga_elements();
    } else {
        LbDataLoadAll(hiresspritefiles);
        LbDataLoadAll(hiresblockfiles);
        LbDataLoadAll(reqfiles);
        loaded_requester = -1;
        LbDataLoadAll(hiresanifiles);
        setup_panel_icons(13);
    }
    resvars.f2 = resvars.f1;
    setup_game_sprites();
    resvars.f5 = 2;
}

// Frees the files load_game_files loaded, if loaded.
// FUNCTION: 0x00010ee0
void unload_game_files(void)
{
    if (resvars.f5 == 2) {
        if (resvars.f1 == SCREEN_MODE_1) {
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
        resvars.f5 = 0;
        LbPackHeap();
    }
}

// Switches to screen mode `mode` (SCREEN_MODE_13 640x480 or SCREEN_MODE_1
// 320x200) with `palette`, using that mode's help layout and fonts. Returns
// 1 on success or if already in it; 0 with error_number 4 or 3 on failure.
// FUNCTION: 0x00010f68
u8 setup_any_screen(const u8 *const palette, TbScreenMode mode)
{
    int variant;

    if (mode == resvars.f0) {
        return 1;
    }
    if (mode == SCREEN_MODE_13) {
        variant = 1;
        use_hires_help_and_fonts();
    } else if (mode == SCREEN_MODE_1) {
        variant = 2;
        use_lores_help_and_fonts();
    }
    if (LbScreenSetupAnyMode(mode, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height, palette, variant, 0) != 1) {
        if (mode == SCREEN_MODE_13) {
            error_number = 4;
        } else {
            error_number = 3;
        }
        return 0;
    }
    resvars.f0 = mode;
    return 1;
}

// Switches to 320x200 with the low-resolution help layout and fonts.
// FUNCTION: 0x0001102c
void fn_0001102c(void)
{
    use_lores_help_and_fonts();
    setup_any_screen(palette, SCREEN_MODE_1);
}

// Uses the 640x480 help layout and fonts.
// FUNCTION: 0x00011058
void use_hires_help_and_fonts(void)
{
    resvars.f2c_intp = hireshelp;
    resvars.fa_charpp = hires_font_filenames;
}

// Uses the 320x200 help layout and fonts.
// FUNCTION: 0x00011084
void use_lores_help_and_fonts(void)
{
    resvars.f2c_intp = D_0014be34;
    resvars.fa_charpp = lores_font_filenames;
}

// Points resvars at the 640x480 layout tables, reopens the faces file
// (exiting the program if it cannot), resets the view window and rebuilds
// the menus and pointer sprites. The screen mode itself is not changed.
// FUNCTION: 0x000110b0
void fn_000110b0(void)
{
    statvar.unknown4 = lbScreenModeInfo[SCREEN_MODE_13].height - 0x30;
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
    resvars.f1 = SCREEN_MODE_13;
    resvars.f0 = SCREEN_MODE_13;
    resvars.f18_u8p = D_0013bb0c;
    resvars.f3c = &ScreenBoxes;
    resvars.f1c_u8p = D_0013c0fc;
    resvars.f40 = D_00146c8c;
    resvars.f3 = 0x20;
    resvars.f4 = 0x40;
    resvars.f20_u8p = window_list;
    resvars.f24 = button_list;
    resvars.f44 = screenlistpos;
    resvars.f28 = hiresbuttons;
    resvars.f2c_intp = hireshelp;
    resvars.f34 = panel_popup_coord;
    resvars.f6_names = hires_req_filenames;
    resvars.fe = hires_requester_buttons;
    resvars.f12 = hires_requester_sprites;
    resvars.fa_charpp = hires_font_filenames;
    resvars.f16 = 3;
    resvars.f17 = 0x20;
    resvars.f38 = D_0012cc3c;
    if (face_file != -1) {
        LbFileClose(face_file);
        face_file = -1;
    }
    face_file = LbFileOpen((char *)hires_faces, 2);
    if (face_file == -1) {
        error_number = 9;
        sub_error_number = 7;
        reset_host();
        exit(1);
    }
    othervar.fa41 = 1;
    othervar.fa40 = 0;
    window.unknown30 = 0;
    window.unknown48 = lbScreenModeInfo[SCREEN_MODE_13].width;
    window.unknown3c = 0;
    window.unknown54 = lbScreenModeInfo[SCREEN_MODE_13].height;
    window.unknown2c0 = 0;
    window.unknown90 = (window.unknown30 + window.unknown48) / 2;
    window.unknown9c = (window.unknown3c + window.unknown54) / 2;
    window.unknown2eb = 0;
    setup_menus();
    LbDataFreeAll(lorespointerfiles);
    LbDataLoadAll(hiresgamepointerfiles);
    LbSpriteSetup(pointer, end_pointers, pointer_data);
}
