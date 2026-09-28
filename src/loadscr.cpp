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
s16 button_index(s32 id);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_glass_text(SlistButton *b, char *text);

extern "C" {
extern Button *D_0015f43c;
extern TbScreenMode D_0015f3f5;
extern u8 *D_00153f34;
extern u8 *D_001553dc;
extern u8 D_00188c7b;
extern u8 D_00188c81;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern u16 D_00161138;
extern u8 D_00155e1d;
extern u8 D_00156910;
extern s32 D_00188c38;
extern char D_001563c5[];
extern char D_00121a60[];
extern char D_00121a71[];
extern char D_00121a79[];
extern char D_00121a80[];
extern char D_00121a92[];
extern char D_00121aa4[];
extern char D_00121ab5[];
extern char D_00121ac7[];
extern char D_00121acd[];

void fn_000c93bf(char *dst, char *src);
void fn_00074cd8(void);
void fn_000110b0(void);
void delay(u32 ms);
}

// NONMATCHING: stack slots of the ?: temporaries
// FUNCTION: 0x0009d5b0
void draw_load_game(void)
{
    s32 level;
    s32 map;
    s32 handle;
    u8 width;
    u8 height;

    set_font_from_cache(1);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    width = lbFontPtr ? lbFontPtr[34].width : 0;
    switch (loadgame.unknown2) {
    case 0:
        return_from_screen(9, 0, 1);
        D_00188c7b = 0;
        load_ingame_pic(0x11, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, D_00121a60);
        calculate_grey(ppalette.data);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        for (D_00161138 = 0; D_00161138 < 0x1e; D_00161138++) {
            loadable[D_00161138] = 0;
            sprintf(tmp, D_00121a71, D_001563c5);
            fn_000c93bf(tmp, myplayer.unknown0);
            sprintf(tmp, D_00121a79, tmp, D_00161138);
            handle = LbFileOpen(tmp, 2);
            if (handle != -1) {
                if (LbFileLengthRnc(tmp) == statvar.unknown0)
                    loadable[D_00161138] = 1;
                LbFileClose(handle);
            }
        }
        loadgame.unknown2 = 3;
        loadgame.unknown0 = 0xfa;
        fn_00074cd8();
        BufferedInkey = 0;
        D_00188c81 = 0;
        delay(800);
        init_zoom_boxes(0x2000);
        break;
    case 3:
        map = next_map();
        if (map == 0)
            loadgame.unknown2 = 4;
        PendingDisplay = map;
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
            LbGhostTableGenerate(D_00153f34, 0x42, D_00121a80);
            D_0015f3f5 = SCREEN_MODE_13;
            load_game_files();
            level = read_level_number(load_game_number);
            if (level != -1) {
                fn_00090804((u16)level, 1);
                if (fn_00021c38((s8)load_game_number, -1)) {
                    clear_loading_screen(D_00153f34);
                    D_00155e1d = 0;
                    fn_000110b0();
                    load_game_files();
                    set_mouse_mode(0);
                    LbGhostTableGenerate(D_00153f34, 0x42, D_00121a92);
                    display_mode = 0;
                    D_00156910 = 1;
                    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
                    draw_it();
                    D_001553dc = D_00153f34;
                    break;
                }
            }
        }
        D_001553dc = D_00153f34;
        LbGhostTableGenerate(ppalette.data, 0x1e, D_00121aa4);
        loadgame.unknown2 = 4;
        break;
    case 2:
        loadgame.unknown0 = 0;
        fade_out_menu();
        LbGhostTableGenerate(D_00153f34, 0x42, D_00121ab5);
        display_mode = 2;
        mainmenu.unknown2 = 0;
        set_mouse_mode(0);
        loadgame.unknown2 = 0;
        break;
    default:
        loadgame.unknown2 = 0;
        break;
    }
}

// FUNCTION: 0x0009d988
void process_loadgame_buttons(u8 n)
{
    if (n == 9) {
        loadgame.unknown2 = 2;
    } else if (loadable[n]) {
        load_game_number = n;
        loadgame.unknown2 = 1;
    }
}

// FUNCTION: 0x0009d9d4
void process_keys_load_game(void)
{
    switch (BufferedInkey) {
    case 1:
        play_sound(10, 0x42, -1, -1);
        loadgame.unknown2 = 2;
        BufferedInkey = 0;
        break;
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x0009da30
void process_mouse_load_game(void)
{
    u8 i;
    s16 x;
    s16 base;
    s16 y;
    s32 mask;
    s32 bit;

    if (loadgame.unknown2 == 4) {
        reqspr_draw((u16)D_0015f43c[0x9e].x, (u16)D_0015f43c[0x9e].y, &screen_sprites_tab_start[2]);
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            base = button_index(bit);
            if (!base)
                continue;
            for (i = 0;; i++) {
                if (D_0015f43c[base + i].width == 0 || D_0015f43c[base + i].height == 0)
                    break;
                x = D_0015f43c[base + i].x;
                y = D_0015f43c[base + i].y;
                if (D_00188c81) {
                    if (over_hotspot(D_00188c6c, D_00188c70, x, y, D_0015f43c[i + base].width - x + 1,
                            D_0015f43c[base + i].height - y + 1)) {
                        play_sound(10, 0x41, -1, -1);
                        D_00188c81 = 0;
                        if (D_0015f43c[base + i].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, D_0015f43c[base + i].width - x, D_0015f43c[base + i].height - y, 1);
                        if (D_0015f43c[base + i].callback)
                            D_0015f43c[i + base].callback(i);
                        break;
                    }
                }
                if (over_hotspot(mouse_dx, mouse_dy, x, y, D_0015f43c[i + base].width - x + 1,
                        D_0015f43c[base + i].height - y + 1)) {
                    if (D_0015f43c[base + i].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, D_0015f43c[base + i].width - x, D_0015f43c[base + i].height - y, 1);
                    if (i != loadgame.unknown0) {
                        play_sound(10, 0x44, -1, -1);
                        loadgame.unknown0 = i;
                    }
                }
            }
        }
    }
    D_00188c81 = 0;
}

// FUNCTION: 0x0009dddc
void display_loadgame_screen(void)
{
    s32 i;

    for (i = 0; (!D_0015f43c[i + 0x95].sprite && D_0015f43c[i + 0x95].width != 0)
                && D_0015f43c[i + 0x95].height != 0; i++) {
        if (loadable[i])
            sprintf(tmp, D_00121ac7, load_text[0], i);
        else
            sprintf(tmp, D_00121acd, load_text[1]);
        draw_glass_text((SlistButton *)&D_0015f43c[i + 0x95], tmp);
    }
}
