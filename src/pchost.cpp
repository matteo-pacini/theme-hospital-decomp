#include "hosp.h"

extern "C" {

extern u8 error_number;
extern View ScreenBoxes;
extern Point ScreenCoords[];
extern Button screenbuttons[];
extern u8 HiresBoxes[], HiresCoords[], window_list[], button_list[], screenlistpos[];
extern u8 hiresbuttons[], panel_popup_coord[], litter_pos[];
extern u8 hiresglass[];
extern int hireshelp[], loreshelp[];
extern u8 *palette;
extern TbSprite *end_pointers;
extern u8 *pointer_data;
extern char *palette_filename;
extern u8 record_replay, play_replay, replay_mode, replay_number;
extern s32 replay_file;
extern void *heapptr;

void LbDataLoadAll(TbLoadFiles *files);
void setup_mcga_elements(void);
void FreeAudio(void);
int LbScreenSetupAnyMode(int mode, ...);
void setup_lores_screen(void);
void use_hires_help_and_fonts(void);
void use_lores_help_and_fonts(void);
void use_hires_layout(void);
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
    resvars.game_files_state = 0;
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
        sprintf(path, "%s%s", cd_install_path, palette_filename);
        check_cd_available(path);
        LbFileLoadAt(path, palette);
    } else {
        LbFileLoadAt(palette_filename, palette);
    }
    if (debug.direct_start) {
        load_game_files();
    }
    LbDataLoadAll(hiresgamepointerfiles);
    LbSpriteSetup(pointer, end_pointers, pointer_data);
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
    resvars.tile_height = 0x20;
    resvars.tile_width = 0x40;
    resvars.tile_width = 0x40;
    resvars.tile_height = 0x20;
    resvars.boxes_u8p = HiresBoxes;
    resvars.screen_boxes = &ScreenBoxes;
    resvars.coords_u8p = HiresCoords;
    resvars.screen_coords = ScreenCoords;
    resvars.window_list_u8p = window_list;
    resvars.button_list = button_list;
    resvars.screen_list_pos = screenlistpos;
    resvars.buttons = hiresbuttons;
    resvars.help_hotspots_intp = hireshelp;
    resvars.panel_popup_coords = panel_popup_coord;
    resvars.game_mode = SCREEN_MODE_13;
    resvars.litter_pos = litter_pos;
    resvars.advisor_min_segments = 3;
    resvars.advisor_max_segments = 0x20;
    resvars.screen_mode = SCREEN_MODE_13;
    resvars.font_filenames_charpp = hires_font_filenames;
    resvars.glass_frames = hiresglass;
    resvars.screen_buttons = screenbuttons;
    resvars.game_files_state = 0;
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
    othervar.fade_palette = 0;
    replay_mode = 0;
    if (record_replay) {
        sprintf(name, "SAVE/PLAY.R%d", replay_number);
        replay_file = LbFileOpen(name, 0);
        if (replay_file) {
            replay_mode |= 1;
        }
    } else if (play_replay) {
        sprintf(name, "SAVE/PLAY.R%d", replay_number);
        replay_file = LbFileOpen(name, 2);
        if (replay_file) {
            replay_mode |= 2;
        }
    }
    debug.screenshot_number = -1;
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

    if (lbDisplay.wscreen) {
        LbScreenUnlock();
    }
    LbNetwork_Stop();
    LbMouseReset();
    SWIKeyboardClose();
    LbScreenReset();
    switch (error_number) {
    case 1:
        lib_strcpy(error_string, "Language Setup Failed.\n");
        break;
    case 2:
        lib_strcpy(error_string, "No Mouse Detected.\n");
        break;
    case 3:
        lib_strcpy(error_string, "Failed to initialise MCGA screen\n");
        break;
    case 4:
        lib_strcpy(error_string, "Failed to initialise VRES256 screen\n");
        break;
    case 5:
        lib_strcpy(error_string, "Memory allocation failure\n");
        break;
    case 6:
        lib_strcpy(error_string, "Heap allocation failure\n");
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
    switch (resvars.screen_mode) {
    case SCREEN_MODE_1:
        LbDataFreeAll(lorespointerfiles);
        break;
    case SCREEN_MODE_13:
        LbDataFreeAll(hiresgamepointerfiles);
        break;
    }
    unload_game_files();
    if (replay_mode & 3) {
        LbFileClose(replay_file);
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
    LbDataFree(&langfile[myplayer.language]);
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
// current resolution (resvars.game_mode), unless already loaded, and lays out the
// panel.
// FUNCTION: 0x00010e24
void load_game_files(void)
{
    if (resvars.game_files_state == 2) {
        return;
    }
    if (resvars.game_mode == SCREEN_MODE_1) {
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
    resvars.loaded_mode = resvars.game_mode;
    setup_game_sprites();
    resvars.game_files_state = 2;
}

// Frees the files load_game_files loaded, if loaded.
// FUNCTION: 0x00010ee0
void unload_game_files(void)
{
    if (resvars.game_files_state == 2) {
        if (resvars.game_mode == SCREEN_MODE_1) {
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
        resvars.game_files_state = 0;
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

    if (mode == resvars.screen_mode) {
        return 1;
    }
    if (mode == SCREEN_MODE_13) {
        variant = 1;
        use_hires_help_and_fonts();
    } else if (mode == SCREEN_MODE_1) {
        variant = 2;
        use_lores_help_and_fonts();
    }
    if (LbScreenSetupAnyMode(mode, lbScreenModeInfo[mode].width, lbScreenModeInfo[mode].height, palette, variant, 0) !=
        1) {
        if (mode == SCREEN_MODE_13) {
            error_number = 4;
        } else {
            error_number = 3;
        }
        return 0;
    }
    resvars.screen_mode = mode;
    return 1;
}

// Switches to 320x200 with the low-resolution help layout and fonts.
// FUNCTION: 0x0001102c
void setup_lores_screen(void)
{
    use_lores_help_and_fonts();
    setup_any_screen(palette, SCREEN_MODE_1);
}

// Uses the 640x480 help layout and fonts.
// FUNCTION: 0x00011058
void use_hires_help_and_fonts(void)
{
    resvars.help_hotspots_intp = hireshelp;
    resvars.font_filenames_charpp = hires_font_filenames;
}

// Uses the 320x200 help layout and fonts.
// FUNCTION: 0x00011084
void use_lores_help_and_fonts(void)
{
    resvars.help_hotspots_intp = loreshelp;
    resvars.font_filenames_charpp = lores_font_filenames;
}

// Points resvars at the 640x480 layout tables, reopens the faces file
// (exiting the program if it cannot), resets the view window and rebuilds
// the menus and pointer sprites. The screen mode itself is not changed.
// FUNCTION: 0x000110b0
void use_hires_layout(void)
{
    statvar.view_height = lbScreenModeInfo[SCREEN_MODE_13].height - 0x30;
    statvar.unknowne6 = 0x1c;
    statvar.unknowne7 = 0x1f;
    resvars.game_mode = SCREEN_MODE_13;
    resvars.screen_mode = SCREEN_MODE_13;
    resvars.boxes_u8p = HiresBoxes;
    resvars.screen_boxes = &ScreenBoxes;
    resvars.coords_u8p = HiresCoords;
    resvars.screen_coords = ScreenCoords;
    resvars.tile_height = 0x20;
    resvars.tile_width = 0x40;
    resvars.window_list_u8p = window_list;
    resvars.button_list = button_list;
    resvars.screen_list_pos = screenlistpos;
    resvars.buttons = hiresbuttons;
    resvars.help_hotspots_intp = hireshelp;
    resvars.panel_popup_coords = panel_popup_coord;
    resvars.req_filenames_names = hires_req_filenames;
    resvars.requester_buttons = hires_requester_buttons;
    resvars.requester_sprites = hires_requester_sprites;
    resvars.font_filenames_charpp = hires_font_filenames;
    resvars.advisor_min_segments = 3;
    resvars.advisor_max_segments = 0x20;
    resvars.litter_pos = litter_pos;
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
    othervar.hires_checked = 1;
    othervar.lores_checked = 0;
    window.zone0_x = 0;
    window.zone0_x1 = lbScreenModeInfo[SCREEN_MODE_13].width;
    window.zone0_y = 0;
    window.zone0_y1 = lbScreenModeInfo[SCREEN_MODE_13].height;
    window.anim_mode0 = 0;
    window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
    window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
    window.current_window = 0;
    setup_menus();
    LbDataFreeAll(lorespointerfiles);
    LbDataLoadAll(hiresgamepointerfiles);
    LbSpriteSetup(pointer, end_pointers, pointer_data);
}
