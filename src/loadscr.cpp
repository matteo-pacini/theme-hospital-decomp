#include "hosp.h"

struct TbIffPalette {
    u32 unknown0;
    u8 data[0x300];
};

struct SlistButton {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 unknown8[10];
};

extern Menu loadgame;
extern TbIffPalette ppalette;
extern u8 BufferedInkey;
extern s32 PendingDisplay;
extern s32 CurrentDisplay;
extern s8 loadable[];
extern char tmp[];
extern u8 load_game_number;

void calculate_grey(u8 *palette);
void init_zoom_boxes(s32 mask);
s32 next_map(void);
long keybuffer_get(void);
u8 what_key_is_it_really(u8 key);
s8 draw_zoom_boxes(void);
void display_loadgame_screen(void);
s16 button_index(s32 mask);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_glass_text(SlistButton *button, char *text);

extern "C" {
extern u8 *palette;
extern u16 col;
extern u8 D_00156910;

void strcat(char *dst, char *src);
void keybuffer_clear(void);
void use_hires_layout(void);
void delay(u32 ms);
}

// Runs one frame of the load game screen, by loadgame.state: 0 sets it up
// (finding which of the player's save files are valid), 3 opens its panels, 4
// shows it, 1 loads the chosen save and starts the game (back to 4 if that
// fails), 2 returns to the main menu.
// FUNCTION: 0x0009d5b0
void draw_load_game(void)
{
    u8 height;
    u8 width;
    s32 handle;
    s32 next;
    s32 level_no;

    set_font_from_cache(1);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    width = lbFontPtr ? lbFontPtr[34].width : 0;
    switch (loadgame.state) {
    case 0:
        return_from_screen(9, 0, 1);
        lbDisplay.left_button = 0;
        load_ingame_pic(SCR_LOAD_GAME, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost1.dat");
        calculate_grey(ppalette.data);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        for (col = 0; col < 0x1e; col++) {
            loadable[col] = 0;
            sprintf(tmp, "%ssave\\", statvar.save_root);
            strcat(tmp, myplayer.name_chars);
            sprintf(tmp, "%s.h%d", tmp, col);
            handle = LbFileOpen(tmp, 2);
            if (handle != -1) {
                if (LbFileLengthRnc(tmp) == statvar.save_game_size)
                    loadable[col] = 1;
                LbFileClose(handle);
            }
        }
        loadgame.state = 3;
        loadgame.value = 0xfa;
        keybuffer_clear();
        BufferedInkey = 0;
        lbDisplay.rleft_button = 0;
        delay(800);
        init_zoom_boxes(0x2000);
        break;
    case 3:
        next = next_map();
        if (next == 0)
            loadgame.state = 4;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        break;
    case 4:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        display_loadgame_screen();
        break;
    case 1:
        fade_out_menu();
        if (display_loading_screen()) {
            LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
            resvars.game_mode = SCREEN_MODE_13;
            load_game_files();
            level_no = read_level_number(load_game_number);
            if (level_no != -1) {
                setup_new_level((u16)level_no, 1);
                if (load_game((s8)load_game_number, -1)) {
                    clear_loading_screen(palette);
                    othervar.screen_pic_loaded = 0;
                    use_hires_layout();
                    load_game_files();
                    set_mouse_mode(MM_NORMAL);
                    LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
                    display_mode = DM_GAME;
                    D_00156910 = 1;
                    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
                    draw_it();
                    othervar.pending_palette = palette;
                    break;
                }
            }
        }
        othervar.pending_palette = palette;
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost1.dat");
        loadgame.state = 4;
        break;
    case 2:
        loadgame.value = 0;
        fade_out_menu();
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        display_mode = DM_MAIN_MENU;
        mainmenu.state = MMS_ENTER;
        set_mouse_mode(MM_NORMAL);
        loadgame.state = 0;
        break;
    default:
        loadgame.state = 0;
        break;
    }
}

// Load game button slot: 9 goes back, others load that save if it is valid.
// FUNCTION: 0x0009d988
void process_loadgame_buttons(u8 slot)
{
    if (slot == 9) {
        loadgame.state = 2;
    } else if (loadable[slot]) {
        load_game_number = slot;
        loadgame.state = 1;
    }
}

// Load game screen keys: Escape goes back to the main menu.
// FUNCTION: 0x0009d9d4
void process_keys_load_game(void)
{
    switch (BufferedInkey) {
    case KC_ESCAPE:
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        loadgame.state = 2;
        BufferedInkey = 0;
        break;
    }
}

// Highlights the load game screen's buttons under the mouse and runs the one
// clicked.
// FUNCTION: 0x0009da30
void process_mouse_load_game(void)
{
    u8 i;
    s16 x;
    s16 base;
    s16 y;
    s32 mask;
    s32 bit;

    if (loadgame.state == 4) {
        reqspr_draw((u16)resvars.screen_buttons[0x9e].x, (u16)resvars.screen_buttons[0x9e].y, &screen_sprites_tab_start[2]);
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            base = button_index(bit);
            if (!base)
                continue;
            for (i = 0;; i++) {
                if (resvars.screen_buttons[base + i].width == 0 || resvars.screen_buttons[base + i].height == 0)
                    break;
                x = resvars.screen_buttons[base + i].x;
                y = resvars.screen_buttons[base + i].y;
                if (lbDisplay.rleft_button) {
                    if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, y, resvars.screen_buttons[base + i].width - x + 1,
                            resvars.screen_buttons[base + i].height - y + 1)) {
                        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                        lbDisplay.rleft_button = 0;
                        if (resvars.screen_buttons[base + i].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, resvars.screen_buttons[base + i].width - x, resvars.screen_buttons[base + i].height - y, 1);
                        if (resvars.screen_buttons[base + i].callback)
                            resvars.screen_buttons[base + i].callback(i);
                        break;
                    }
                }
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, y, resvars.screen_buttons[base + i].width - x + 1,
                        resvars.screen_buttons[base + i].height - y + 1)) {
                    if (resvars.screen_buttons[base + i].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, resvars.screen_buttons[base + i].width - x, resvars.screen_buttons[base + i].height - y, 1);
                    if (i != loadgame.value) {
                        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                        loadgame.value = i;
                    }
                }
            }
        }
    }
    lbDisplay.rleft_button = 0;
}

// Writes each save slot's label on its button: its number if it holds a valid
// save, else the empty text.
// FUNCTION: 0x0009dddc
void display_loadgame_screen(void)
{
    s32 i;

    for (i = 0; (!resvars.screen_buttons[i + 0x95].sprite && resvars.screen_buttons[i + 0x95].width != 0)
                && resvars.screen_buttons[i + 0x95].height != 0; i++) {
        if (loadable[i])
            sprintf(tmp, "%s %d", load_text[0], i);
        else
            sprintf(tmp, "%s", load_text[1]);
        draw_glass_text((SlistButton *)&resvars.screen_buttons[i + 0x95], tmp);
    }
}
