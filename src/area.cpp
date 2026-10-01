#include "hosp.h"

struct TbIffPalette {
    u32 unknown0;
    u8 data[0x300];
};

struct AreaLayout {
    u8 unknown0[0x1de9];
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
};

extern "C" {
extern Event D_001549d0[];
extern u8 *palette;
extern u8 D_00156910;
extern void *smack_draw_callback;
void keybuffer_clear(void);
int LbScreenLock(void);
void FadeSample(int a, int b, int c, int d, u8 e);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 w);
}

extern Menu area;
extern ClickArea area_towns[];
extern TbIffPalette ppalette;
extern u8 BufferedInkey;
extern s32 PendingDisplay;
extern char *display_text;
extern char *paragraphs[];
extern s16 text_ofs;
extern s16 text_paragraph;
extern s16 yofs_text;
extern s16 text_delay;
extern char text_buf;

void play_area_smk(const u8 level_no);
void display_area_select(void);
void display_select_level(void);
void calculate_grey(u8 *palette);
s32 next_map(void);
long keybuffer_get(void);
u8 what_key_is_it_really(u8 key);
void swap_game_sprites(TbScreenMode mode);

// Runs one frame of the level briefing screen, by its state (area.unknown2): 0
// and 6 unload the game and play the level's intro video, 1 shows the
// briefing's map screens (next_map) until they are done, 3 types out the
// briefing text, 4 shows the debug level list, 5 the text while any level can
// be picked (process_mouse_area_select), and 2 loads level_number and starts
// the game (display_mode 0). Returns 0 if the loading screen could not be
// shown, else 1.
// FUNCTION: 0x00068a80
u8 draw_area_select(void)
{
    u8 was_locked;
    s32 next;
    char buf[128];

    switch (area.state) {
    case 0:
        return_from_screen(9, 0, 1);
        unload_game_files();
        if (animation_table[level_number + 7].cd_install_level > install_type) {
            sprintf(buf, "%s%s", cd_install_path, animation_table[level_number + 7].name);
            check_cd_available(buf);
        }
        if (level_number >= 12)
            play_sound(10, SFX_DICE122M, -1, -1);
        else
            play_sound(10, SFX_DICEYFIN, -1, -1);
        play_area_smk(level_number);
        lbDisplay.left_button = 0;
        LbPaletteGet(ppalette.data);
        LbGhostTableGenerate(ppalette.data, 0x19, "qdata/ghost2.dat");
        calculate_grey(ppalette.data);
        BufferedInkey = 0;
        keybuffer_clear();
        area.value = 0;
        area.state = 1;
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        othervar.screen_pic_loaded = 0;
        set_mouse_mode(9);
        break;
    case 1:
        next = next_map();
        if (next == 0)
            area.state = 3;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        break;
    case 2:
        set_mouse_mode(9);
        FadeSample(10, 0xd2, 0, 1, 1);
        LbScreenClear(0);
        if (!display_loading_screen())
            return 0;
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        area.state = 0;
        advisor.tutorial_step = 0x1d;
        display_mode = 0;
        othervar.f33 = 0;
        LbScreenClear(0);
        swap_game_sprites(resvars.game_mode);
        if (resvars.game_mode == SCREEN_MODE_1) {
            was_locked = lbDisplay.wscreen ? (u8)1 : (u8)0;
            if (was_locked)
                LbScreenUnlock();
            change_res(SCREEN_MODE_1);
            setup_menus();
            while (LbScreenLock() != 1)
                ;
        }
        resvars.loaded_mode = resvars.game_mode;
        D_00156910 = 1;
        clear_loading_screen(palette);
        setup_new_level((u16)level_number, 1);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        goto_column_row_in_window(window.current_window, level->camera_start[player] % 128, level->camera_start[player] / 128);
        othervar.show_level_brief = 1;
        draw_it();
        othervar.pending_palette = palette;
        init_ambients();
        break;
    case 3:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        display_area_select();
        break;
    case 4:
        display_select_level();
        break;
    case 5:
        display_area_select();
        break;
    case 6:
        unload_game_files();
        if (animation_table[level_number + 7].cd_install_level > install_type) {
            sprintf(buf, "%s%s", cd_install_path, animation_table[level_number + 7].name);
            check_cd_available(buf);
        }
        if (level_number >= 12)
            play_sound(10, SFX_DICE122M, -1, -1);
        else
            play_sound(10, SFX_DICEYFIN, -1, -1);
        play_area_smk(level_number);
        lbDisplay.left_button = 0;
        LbPaletteGet(ppalette.data);
        LbGhostTableGenerate(ppalette.data, 0x19, "qdata/ghost2.dat");
        calculate_grey(ppalette.data);
        BufferedInkey = 0;
        keybuffer_clear();
        area.value = 0;
        area.state = 1;
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        othervar.screen_pic_loaded = 0;
        set_mouse_mode(9);
        break;
    default:
        area.state = 0;
        break;
    }
    return 1;
}

// Handles a key on the briefing screen: Escape returns to the main menu, C
// switches the briefing to state 5 (pick any level). Consumes the key.
// FUNCTION: 0x00068f60
void process_keys_area_select(void)
{
    switch (BufferedInkey) {
    case KC_ESCAPE:
        area.state = 0;
        display_mode = 2;
        mainmenu.state = 0x1f;
        D_001549d0[player].value = 0;
        set_mouse_mode(MM_NORMAL);
        break;
    case KC_C:
        area.state = 5;
        break;
    }
    BufferedInkey = lbInkey = 0;
}

// Handles the mouse on the briefing screen: in state 5 a click on one of the
// 21 level areas plays that level's video and starts it. Otherwise a click, or
// 600 frames without mouse movement, starts the selected level (level 1 if none
// is set).
// FUNCTION: 0x00068fec
void process_mouse_area_select(void)
{
    u16 i;
    u8 kind;

    switch (area.state) {
    case 3:
        break;
    case 5:
        if (lbDisplay.left_button) {
            for (i = 0; i < 0x15; i++) {
                if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.boxes[i + 45].x, resvars.boxes[i + 45].y, resvars.boxes[i + 45].w, resvars.boxes[i + 45].h)) {
                    set_mouse_mode(0x15);
                    play_area_smk(i + 1);
                    area.state = 2;
                    level_number = i + 1;
                    allhosps->level = level_number;
                    break;
                }
            }
            lbDisplay.left_button = 0;
        }
        break;
    case 0:
    case 2:
    case 4:
        return;
    case 1:
    default:
        break;
    }
    i = 0;
    kind = 0x63;
    while (area_towns[i].w > 0 && area_towns[i].h > 0) {
        kind = area_towns[i].kind;
        if (lbDisplay.left_button) {
            if (allhosps->level == 0 || level_number == 0) {
                level_number = 1;
                allhosps->level = level_number;
            }
            area.state = 2;
            lbDisplay.left_button = 0;
        }
        i++;
    }
    if (othervar.last_mouse_sum != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
        othervar.idle_countdown = 600;
        othervar.last_mouse_sum = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
    }
    if (!(s16)othervar.idle_countdown--) {
        if (allhosps->level == 0 || level_number == 0) {
            level_number = 1;
            allhosps->level = level_number;
        }
        area.state = 2;
    }
}

// Draws the briefing text: the paragraphs already shown, then the current one
// (text_paragraph) revealed one more character per call, moving on to the
// next paragraph with a pause (text_delay) once it is complete.
// FUNCTION: 0x00069268
void display_area_select(void)
{
    u8 unused;
    u8 flag;
    u16 i;
    char buf[500];

    unused = 0;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x27);
    lbDisplay.draw_flags = 0;
    LbTextSetJustifyWindow(resvars.glass_frames_AreaLayoutp->x, resvars.glass_frames_AreaLayoutp->y, resvars.glass_frames_AreaLayoutp->x1 - resvars.glass_frames_AreaLayoutp->x);
    LbTextSetClipWindow(resvars.glass_frames_AreaLayoutp->x, resvars.glass_frames_AreaLayoutp->y, resvars.glass_frames_AreaLayoutp->x1 - resvars.glass_frames_AreaLayoutp->x, resvars.glass_frames_AreaLayoutp->y1 - resvars.glass_frames_AreaLayoutp->y);
    lbDisplay.draw_flags |= 0x20;
    yofs_text = 0;
    for (i = 0; i < text_paragraph; i++) {
        LbTextDraw(0, yofs_text, paragraphs[i]);
        yofs_text = (lbFontPtr ? lbFontPtr[34].height : 0) + LbTextStringHeight(paragraphs[i]) + yofs_text;
    }
    display_text = paragraphs[text_paragraph];
    if (display_text) {
        if (display_text[text_ofs]) {
            text_buf = display_text[text_ofs];
            display_text[text_ofs] = 0;
            LbTextDraw(0, yofs_text, display_text);
            display_text[text_ofs] = text_buf;
            text_ofs++;
            text_delay = 1;
        } else {
            text_ofs = 0;
            text_paragraph++;
            LbTextDraw(0, yofs_text, display_text);
            yofs_text += LbTextStringHeight(display_text);
            text_delay = 0x1e;
        }
    }
    lbDisplay.draw_flags = 0;
}

// Debug level list: draws level.l0 to level.l119 in six columns, greyed when
// the file is missing, and starts the level clicked (state 2). Levels above 14
// use town 1.
// FUNCTION: 0x000694e8
void display_select_level(void)
{
    u8 char_width;
    u8 line_height;
    u8 found;
    u16 row;
    u16 col;
    s32 handle;
    char buf[144];

    setup_colors();
    LbScreenClear(color.dark_blue);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x27);
    lbDisplay.draw_flags = 0;
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    char_width = lbFontPtr ? lbFontPtr[34].width : 0;
    for (col = 0; col < 6; col++) {
        for (row = 0; row < 20; row++) {
            found = 0;
            if (install_type < 3) {
                sprintf(buf, "%slevels/level.l%d", cd_install_path, col * 20 + row);
                check_cd_available(buf);
            } else {
                sprintf(buf, "levels/level.l%d", col * 20 + row);
            }
            handle = LbFileOpen(buf, 2);
            if (handle != -1) {
                lbDisplay.draw_colour = color.dark_blue;
                lbDisplay.draw_flags |= 0x40;
                found = 1;
                LbFileClose(handle);
            } else {
                lbDisplay.draw_colour = color.green;
                lbDisplay.draw_flags |= 0x40;
            }
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, col * 100, line_height * row, 100, line_height)) {
                lbDisplay.draw_colour = color.black;
                *(volatile u16 *)&lbDisplay.draw_flags |= 0x40;
                if (lbDisplay.left_button) {
                    lbDisplay.left_button = 0;
                    area.state = 2;
                    level_number = (u8)col * 20 + (u8)row;
                    if (level_number > 14)
                        allhosps->level = 1;
                    else
                        allhosps->level = level_number;
                }
            }
            sprintf(buf, "level-%d", col * 20 + row);
            LbTextDraw(col * 100, line_height * row, buf);
        }
    }
}

// Plays level level_no's briefing video (animation_table index level_no + 7)
// full screen, then restores the saved screen.
// FUNCTION: 0x00069770
void play_area_smk(const u8 level_no)
{
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    smack_draw_callback = 0;
    hosp_play_smk(level_no + 7);
    copy_screen_from_wscreen(bscreen);
}
