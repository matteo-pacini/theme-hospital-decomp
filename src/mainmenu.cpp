#include "hosp.h"

extern Menu loadgame;
extern Menu nickname;
extern PlotBid plotbid;
extern s32 current_frame;

void help_hotspot_detected(u16 id, u8 a, s16 b, s16 c);
void calculate_land_amount(void);
void setup_overhead_map(void);
void draw_money_on_town(void);
void process_bid_panel(void);
void process_bid_time(void);
void tri_test(void);
u8 check_tri(u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 x, u16 y);
void display_main_menu(void);
void play_main_menu_intro(void);

extern "C" {
extern void *smack_draw_callback;
void LbMouseSetWindow(s32 x, s32 y, s32 w, s32 h);

int fn_000bed50(void);
void fn_0001102c(void);
void fn_00011058(void);
void fn_000110b0(void);
void fn_000905f4(void);
void fn_00066914(void);
void fn_000688e4(void);
void fn_00068a14(void);
void fn_000669fc(void);
void fn_0006680c(void);
void fn_0006310c(void);
u8 fn_00063174(void);
u8 fn_00063300(void);
u8 fn_0006344c(s16 *scroll, u16 *line);
void fn_00063680(void);
void fn_00063a64(void);

extern u8 D_001553e1[];
extern u8 D_001553e8;
extern u8 *D_001553d7;
extern u8 *D_001553dc;
extern u8 *D_00153f34;
extern u8 D_00155421;
extern u8 D_00155422;
extern u32 D_00155423;
extern u8 D_00155430;
extern u8 D_00155d82;
extern u16 D_00155d83;
extern u16 D_00155d85;
extern u8 *D_00155d8f;
extern s32 D_00155d93;
extern s32 D_00155d97;
extern u8 D_00155e1d;
extern u8 D_00155e3f;
extern u8 D_00155e42;
extern u8 D_00156910;
extern u8 D_0015f489;
extern char **D_0016033c;
extern s32 D_00188c38;
extern volatile s16 D_00188c6c, D_00188c70;
extern u16 D_00188c74;
extern u8 D_00188c7b, D_00188c7c, D_00188c7e, D_00188c7f, D_00188c81, D_00188c83;
extern char D_00120488[];
extern char D_0012049a[];
extern char D_0012049f[];
extern char D_001204a4[];
extern char D_001204a8[];
extern char D_001204ba[];
extern char D_0013f4fa[];
}

s8 compare_products(s32 a, s32 b, s32 c, s32 d);
#pragma aux compare_products = \
    0xf7 0xea 0x93 0x87 0xca 0xf7 0xea 0x29 0xc3 0x0f 0x95 0xc0 0x19 0xd1 \
    0x74 0x08 0x0f 0x9c 0xc4 0x0f 0x9f 0xc0 0x28 0xe0 \
    parm [eax] [ebx] [ecx] [edx] value [al] modify [eax ebx ecx edx];

// NONMATCHING: D_00188c7e and lbInkey are loaded in the opposite order; the target order is what a
// struct-field D_00188c7e (0x188c7e lies inside the demo's 0x78-byte lbDisplay) would give
// FUNCTION: 0x00061bd0
void draw_main_menu(void)
{
    u8 count;
    u8 done;
    s16 timer;
    u16 line;
    s16 scroll;
    char path[0x80];

    timer = 0x3e8;
    count = 0;
    switch (mainmenu.unknown2) {
    case 0:
        return_from_screen(9, 0, 0);
    case 0x1f:
        D_00188c7b = D_00188c7c = D_00188c81 = 0;
        unload_game_files();
        mainmenu.unknown2 = 0xe;
        break;
    case 0xe:
        clear_loading_screen(0);
        LbMouseReset();
        LbMouseChangeSprite(0);
        fn_0001102c();
        fn_00011058();
        D_00188c7b = D_00188c7c = D_00188c81 = 0;
        load_ingame_pic(0xd, 1, 0x14, 0, SCREEN_MODE_1);
        LbIsActive();
        LbScreenClear(0);
        LbScreenSwap();
        while (fn_000bed50() != 1) {
        }
        play_main_menu_intro();
        set_mouse_mode(0x14);
        LbMouseSetup(&screen_pointers_tab_start[D_001553e8]);
        setup_colors();
        mainmenu.unknown2 = 3;
        break;
    case 0xf:
        LbMouseChangeSprite(0);
        fn_0006310c();
        fade_out_menu();
        D_001553e1[3] = 0xd;
        display_mode = 6;
        mainmenu.unknown2 = 0;
        loadgame.unknown2 = 0;
        break;
    case 8:
        LbMouseChangeSprite(0);
        fn_0006310c();
        fade_out_menu();
        display_mode = 0xa;
        nickname.unknown2 = 3;
        mainmenu.unknown2 = 0;
        D_001553e1[3] = 0xd;
        level_number = 0;
        break;
    case 9:
        if (save_game_valid(0x63) == 0) {
            mainmenu.unknown2 = 3;
            play_sound(10, 8, -1, -1);
            break;
        }
        fn_0006310c();
        fade_out_menu();
        D_001553e1[3] = 0xd;
        mainmenu.unknown2 = 0xd;
        break;
    case 0xd:
        return_from_screen(0, 0, 1);
        LbScreenClear(0);
        if (!display_loading_screen())
            break;
        level_number = read_level_number(0x63);
        if (level_number != -1) {
            fn_000110b0();
            load_game_files();
            fn_00090804((u16)level_number, 1);
            if (fn_00021c38(0x63, -1)) {
                D_00155e1d = 0;
                set_mouse_mode(0);
                LbGhostTableGenerate(D_00153f34, 0x42, D_00120488);
                display_mode = 0;
                mainmenu.unknown2 = 0;
                load_game_files();
                clear_loading_screen(D_00153f34);
                D_00156910 = 1;
                set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
                draw_it();
                D_001553dc = D_00153f34;
                break;
            }
        }
        mainmenu.unknown2 = 0x1b;
        break;
    case 0x1b:
        fn_0001102c();
        mainmenu.unknown2 = 0;
        break;
    case 0xa:
        LbMouseChangeSprite(0);
        fn_0006310c();
        fade_out_menu();
        display_mode = 5;
        mainmenu.unknown2 = 0;
        D_001553e1[3] = 0xd;
        break;
    case 0xb:
    case 0xc:
        LbMouseChangeSprite(0);
        fn_0006310c();
        fade_out_menu();
        quit_game = 3;
        break;
    case 3:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb3].x, resvars.f40[0xb3].y, &screen_sprites_tab_start[2]);
        draw_help_text();
        break;
    case 4:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb4].x, resvars.f40[0xb4].y, &screen_sprites_tab_start[3]);
        draw_help_text();
        break;
    case 5:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb5].x, resvars.f40[0xb5].y, &screen_sprites_tab_start[1]);
        draw_help_text();
        break;
    case 6:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb6].x, resvars.f40[0xb6].y, &screen_sprites_tab_start[4]);
        draw_help_text();
        break;
    case 7:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb7].x, resvars.f40[0xb7].y, &screen_sprites_tab_start[5]);
        draw_help_text();
        break;
    case 0x11:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb8].x, resvars.f40[0xb8].y, &screen_sprites_tab_start[6]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x12:
        display_main_menu();
        reqspr_draw(resvars.f40[0xb9].x, resvars.f40[0xb9].y, &screen_sprites_tab_start[7]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x13:
        display_main_menu();
        reqspr_draw(resvars.f40[0xba].x, resvars.f40[0xba].y, &screen_sprites_tab_start[8]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x14:
        display_main_menu();
        reqspr_draw(resvars.f40[0xbb].x, resvars.f40[0xbb].y, &screen_sprites_tab_start[9]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x15:
        display_main_menu();
        reqspr_draw(resvars.f40[0xbc].x, resvars.f40[0xbc].y, &screen_sprites_tab_start[10]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x16:
        display_main_menu();
        reqspr_draw(resvars.f40[0xbd].x, resvars.f40[0xbd].y, &screen_sprites_tab_start[11]);
        play_sound(10, 0x44, -1, -1);
        break;
    case 0x17:
        D_00188c7b = D_00188c7c = D_00188c81 = 0;
        LbMouseChangeSprite(0);
        fn_00063174();
        fade_out_menu();
        D_001553e1[3] = 0xd;
        mainmenu.unknown2 = 0x18;
        break;
    case 0x18:
        load_ingame_pic(0x12, 1, 9, 0, SCREEN_MODE_13);
        D_00155d8f = lbDisplay.unknown4;
        D_00155d93 = D_00188c34;
        D_00155d97 = D_00188c38;
        lbDisplay.unknown4 = bscreen;
        D_00188c34 = 0x280;
        D_00188c38 = 0x1e0;
        LbScreenSetGraphicsWindow(0, 0, 0x280, 0x1e0);
        fn_000905f4();
        lbDisplay.unknown4 = D_00155d8f;
        D_00188c34 = D_00155d93;
        D_00188c38 = D_00155d97;
        LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
        fn_000905f4();
        mainmenu.unknown2 = 0x19;
        break;
    case 0x19:
        while (D_00188c7e + lbInkey + D_00188c7f) {
            D_00188c7b = D_00188c7c = lbInkey = 0;
        }
        done = 0;
        timer = 0x3e8;
        sprintf(path, D_0012049a, cd_install_path, D_0013f4fa);
        while (!done) {
            copy_screen_to_wscreen(bscreen);
            if (exit_quick_requester()) {
                done = 1;
            } else if (LbFileExists(path)) {
                if (--timer == 0) {
                    while (D_00188c7e + lbInkey + D_00188c7f) {
                        D_00188c7b = D_00188c7c = lbInkey = 0;
                    }
                    fade_out_menu();
                    count++;
                    if (count % 2 == 1)
                        done = fn_00063300();
                    else
                        done = fn_00063174();
                    fade_out_menu();
                    load_ingame_pic(0x12, 1, 9, 0, SCREEN_MODE_13);
                    D_00155d8f = lbDisplay.unknown4;
                    D_00155d93 = D_00188c34;
                    D_00155d97 = D_00188c38;
                    lbDisplay.unknown4 = bscreen;
                    D_00188c34 = 0x280;
                    D_00188c38 = 0x1e0;
                    LbScreenSetGraphicsWindow(0, 0, 0x280, 0x1e0);
                    fn_000905f4();
                    lbDisplay.unknown4 = D_00155d8f;
                    D_00188c34 = D_00155d93;
                    D_00188c38 = D_00155d97;
                    LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
                    fn_000905f4();
                    timer = 0x3e8;
                    if (!done)
                        hosp_fade_palette(D_001553d7, 10, Lb_PALETTE_FADE_CLOSED);
                }
            }
            if (D_00188c7e + D_00188c7f + lbInkey + D_00188c7b + D_00188c7c) {
                D_00188c7b = D_00188c81 = D_00188c7c = lbInkey = 0;
                done = 1;
            }
            if (done) {
                mainmenu.unknown2 = 0x1a;
                D_00188c7b = D_00188c7c = 0;
            }
            LbIsActive();
            LbScreenSwap();
            while (fn_000bed50() != 1) {
            }
        }
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        break;
    case 0x1a:
        display_mode = 0xa;
        nickname.unknown2 = 0;
        mainmenu.unknown2 = 0;
        break;
    case 0x1c:
        load_ingame_pic(0x17, 1, 9, 0, SCREEN_MODE_13);
        mainmenu.unknown2 = 0x1d;
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        LbTextSetWindow(0, 0x72, D_00188c34, D_00188c38);
        D_00188c74 = 0;
        set_mouse_mode(9);
        break;
    case 0x1d:
        D_00188c7b = D_00188c7c = D_00188c81 = D_00188c83 = 0;
        lbInkey = 0;
        scroll = 0;
        line = 0;
        fn_0006344c(&scroll, &line);
        done = 0;
        while (!done) {
            copy_screen_to_wscreen(bscreen);
            if (fn_0006344c(&scroll, &line))
                done = 1;
            LbIsActive();
            LbScreenSwap();
            while (fn_000bed50() != 1) {
            }
        }
        mainmenu.unknown2 = 0x1e;
        break;
    case 0x1e:
        mainmenu.unknown2 = 0x1f;
        fade_out_menu();
        fn_0001102c();
        break;
    default:
        mainmenu.unknown2 = 0;
        break;
    }
}

// FUNCTION: 0x00062730
void process_keys_main_menu(void)
{
    switch (mainmenu.unknown2) {
    case 0x19:
        if (lbInkey)
            mainmenu.unknown2 = 0x1a;
    case 0x17:
    case 0x1a:
        return;
    default:
        break;
    }
    switch (lbInkey) {
    case 1:
    case 0x10:
        fade_out_menu();
        quit_game = 3;
        lbInkey = 0;
        break;
    default:
        break;
    }
}

// NONMATCHING: switch compare tree splits the case ranges differently
// FUNCTION: 0x000627cc
void process_mouse_main_menu(void)
{
    switch (mainmenu.unknown2) {
    case 0:
    case 1:
    case 2:
    case 8:
    case 9:
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
        return;
    case 0x19:
        if (D_00188c7b + D_00188c7c)
            mainmenu.unknown2 = 0x1a;
    case 0x17:
    case 0x18:
    case 0x1a:
        return;
    default:
        break;
    }
    if (D_00188c81) {
        D_00188c81 = 0;
        if (check_tri(0x2c, 0x13, 0x55, 0x3a, 0x7f, 0x13, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 8;
        if (check_tri(0x55, 0x3a, 0x7f, 0x68, 0x7f, 0x13, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 0xf;
        if (check_tri(0x2c, 0x13, 0x2c, 0x68, 0x55, 0x3a, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 9;
        if (check_tri(0x2c, 0x68, 0x7f, 0x68, 0x55, 0x3a, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 0xa;
        if (check_tri(0x30, 0x77, 0x30, 0x96, 0x74, 0x79, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 0xc;
        if (check_tri(0x74, 0x79, 0x30, 0x96, 0x74, 0x96, D_00188c6c, D_00188c70))
            mainmenu.unknown2 = 0xc;
    }
    D_00188c7b = D_00188c81 = D_00188c7c = D_00188c83 = 0;
}

// FUNCTION: 0x00062a50
void display_main_menu(void)
{
    copy_screen_to_wscreen(bscreen);
    tri_test();
    set_font_from_cache(0);
}

// FUNCTION: 0x00062a7c
void tri_test(void)
{
    u8 unused;

    unused = 0;
    if (check_tri(0x2c, 0x13, 0x55, 0x3a, 0x7f, 0x13, mouse_dx, mouse_dy)) {
        if (mainmenu.unknown2 != 5) {
            switch (mainmenu.unknown2) {
            case 3:
                mainmenu.unknown2 = 0x11;
                break;
            case 4:
                mainmenu.unknown2 = 0x13;
                break;
            case 6:
                mainmenu.unknown2 = 0x12;
                break;
            case 7:
                mainmenu.unknown2 = 0x12;
                break;
            default:
                mainmenu.unknown2 = 5;
                break;
            }
        } else {
            help_hotspot_detected(1, 1, 0, 0);
        }
    } else if (check_tri(0x55, 0x3a, 0x7f, 0x68, 0x7f, 0x13, mouse_dx, mouse_dy)) {
        if (mainmenu.unknown2 != 4) {
            switch (mainmenu.unknown2) {
            case 5:
                mainmenu.unknown2 = 0x13;
                break;
            case 3:
                mainmenu.unknown2 = 0x12;
                break;
            case 6:
                mainmenu.unknown2 = 0x14;
                break;
            case 7:
                mainmenu.unknown2 = 0x14;
                break;
            default:
                mainmenu.unknown2 = 4;
                break;
            }
        } else {
            help_hotspot_detected(2, 1, 0, 0);
        }
    } else if (check_tri(0x2c, 0x13, 0x2c, 0x68, 0x55, 0x3a, mouse_dx, mouse_dy)) {
        if (mainmenu.unknown2 != 3) {
            switch (mainmenu.unknown2) {
            case 5:
                mainmenu.unknown2 = 0x11;
                break;
            case 4:
                mainmenu.unknown2 = 0x12;
                break;
            case 6:
                mainmenu.unknown2 = 0x16;
                break;
            case 7:
                mainmenu.unknown2 = 0x16;
                break;
            default:
                mainmenu.unknown2 = 3;
                break;
            }
        } else {
            help_hotspot_detected(3, 1, 0, 0);
        }
    } else if (check_tri(0x2c, 0x68, 0x7f, 0x68, 0x55, 0x3a, mouse_dx, mouse_dy)) {
        if (mainmenu.unknown2 != 6) {
            switch (mainmenu.unknown2) {
            case 5:
                mainmenu.unknown2 = 0x12;
                break;
            case 4:
                mainmenu.unknown2 = 0x14;
                break;
            case 3:
                mainmenu.unknown2 = 0x16;
                break;
            case 7:
                mainmenu.unknown2 = 0x15;
                break;
            default:
                mainmenu.unknown2 = 6;
                break;
            }
        } else {
            help_hotspot_detected(4, 1, 0, 0);
        }
    } else if (check_tri(0x30, 0x77, 0x30, 0x96, 0x74, 0x79, mouse_dx, mouse_dy)
               || check_tri(0x74, 0x79, 0x30, 0x96, 0x74, 0x96, mouse_dx, mouse_dy)) {
        if (mainmenu.unknown2 != 7) {
            switch (mainmenu.unknown2) {
            case 5:
                mainmenu.unknown2 = 0x12;
                break;
            case 4:
                mainmenu.unknown2 = 0x14;
                break;
            case 3:
                mainmenu.unknown2 = 0x16;
                break;
            case 6:
                mainmenu.unknown2 = 0x15;
                break;
            default:
                mainmenu.unknown2 = 7;
                break;
            }
        } else {
            help_hotspot_detected(5, 1, 0, 0);
        }
    } else {
        switch (mainmenu.unknown2) {
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            mainmenu.unknown2 = 3;
            break;
        default:
            break;
        }
    }
    switch (mainmenu.unknown2) {
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
        break;
    default:
        break;
    }
}

// FUNCTION: 0x00062f34
u8 check_tri(u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 x, u16 y)
{
    s32 outside;

    outside = compare_products(x2 - x1, y2 - y1, x - x1, y - y1) > 0
              || compare_products(x3 - x2, y3 - y2, x - x2, y - y2) > 0
              || compare_products(x1 - x3, y1 - y3, x - x3, y - y3) > 0;
    return outside == 0;
}

// FUNCTION: 0x000630a4
void play_main_menu_intro(void)
{
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_mouse_mode(9);
    smack_draw_callback = 0;
    hosp_play_smk(0x16);
}

// FUNCTION: 0x0006310c
void fn_0006310c(void)
{
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_mouse_mode(9);
    smack_draw_callback = 0;
    hosp_play_smk(0x17);
}

// FUNCTION: 0x00063174
u8 fn_00063174(void)
{
    u8 aborted;
    char path[0x80];
    u8 active;
    u8 result;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    aborted = 0;
    sprintf(path, D_0012049f, cd_install_path, animation_table[D_00155e3f > 4 ? 0x1a : 0x19].name);
    if (!LbFileExists(path))
        return aborted;
    fade_out_menu();
    if (active)
        LbIsActive();
    LbScreenClear(0);
    fn_0001102c();
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_mouse_mode(9);
    smack_draw_callback = 0;
    if (D_00155e3f > 4)
        result = hosp_play_smk(0x1a);
    else
        result = hosp_play_smk(0x19);
    if (result == 2)
        aborted = 1;
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
    if (!setup_any_screen(D_00153f34, SCREEN_MODE_13)) {
        reset_host();
        return 1;
    }
    if (active) {
        while (fn_000bed50() != 1) {
        }
    }
    set_mouse_mode(9);
    return aborted;
}

// FUNCTION: 0x00063300
u8 fn_00063300(void)
{
    u8 aborted;
    u8 active;

    aborted = 0;
    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if (active)
        LbIsActive();
    fade_out_menu();
    if (change_res(SCREEN_MODE_13)) {
        LbScreenClear(0);
        LbScreenSwap();
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        D_00188c74 = 0;
        set_mouse_mode(9);
        D_0015f489 = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        D_00155d82 = saferand();
        D_00155d83 = 0;
        D_00155d85 = resvars.f18_s32p[0xab] / 2 + 100;
        mainmenu.unknown0 = 0;
        aborted = 0;
        if (hosp_play_smk(0x1b) == 2)
            aborted = 1;
        fade_out_menu();
        change_res(SCREEN_MODE_13);
        LbScreenClear(0);
    }
    if (active) {
        while (fn_000bed50() != 1) {
        }
    }
    return aborted;
}

// FUNCTION: 0x0006344c
u8 fn_0006344c(s16 *scroll, u16 *line)
{
    u8 i;
    u8 height;
    s16 y;
    u8 done;
    char text[0x64];

    done = 0;
    set_font_from_cache(0x2a);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    D_00188c74 = 0x100;
    if (height % 3)
        height = (height / 3 + 1) * 3;
    y = -*scroll;
    for (i = 0; y < D_00188c38; y += height, i++) {
        if (*scroll >= -0x32) {
            sprintf(text, D_001204a4, D_0016033c[i + *line]);
            switch (text[0]) {
            case '!':
                break;
            case '.':
                done = 1;
                break;
            case ':':
                text[0] = ' ';
                LbTextDraw(0, y, text);
                break;
            case ',':
                text[0] = ' ';
                LbTextDraw(0, y, text);
                break;
            default:
                LbTextDraw(0, y, text);
                break;
            }
        }
    }
    if (D_00188c81 + D_00188c83) {
        done = 1;
        D_00188c7b = D_00188c7c = D_00188c81 = D_00188c83 = 0;
        lbInkey = 0;
    }
    *scroll += 3;
    if (*scroll % height == 0) {
        *scroll = 0;
        (*line)++;
    }
    return done;
}

// FUNCTION: 0x00063680
void fn_00063680(void)
{
    u32 i;
    u32 total;

    switch (town.unknown2) {
    case 0:
        D_00188c7b = D_00188c81 = 0;
        setup_abort_all_requesters();
        D_00155422 = myhosp->fc94;
        D_00155423 = 0x32;
        load_ingame_pic(1, 1, 0xa, level_number, SCREEN_MODE_13);
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        if (!town.unknown0)
            town.unknown0 = 0x1f;
        calculate_land_amount();
        set_mouse_mode(0xa);
        set_font_from_cache(0);
        setup_overhead_map();
        town.unknown2 = 7;
        D_00155421 = 1;
        fn_00066914();
        draw_money_on_town();
        set_font_from_cache(0);
        D_00155e42 = 0x1e;
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        break;
    case 1:
        D_00188c7b = D_00188c81 = 0;
        LbGhostTableGenerate(D_00153f34, 0x42, D_001204a8);
        display_mode = 0;
        fn_000688e4();
        town.unknown2 = 0;
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        fn_00068a14();
        break;
    case 7:
        compute_help_text(0, 0, 0x13);
        fn_00066914();
        draw_money_on_town();
        set_font_from_cache(0);
        draw_help_text();
        set_mouse_mode(0xa);
        if (D_00188c81) {
            if (over_hotspot(D_00188c6c, D_00188c70, 0xd, 0x1a6, 0xc0, 0x2e)) {
                fn_00074220(0, 0x41);
                LbGhostTableGenerate(D_00153f34, 0x42, D_001204ba);
                town.unknown2 = 0;
            }
        }
        break;
    case 0xb:
        D_00188c7b = D_00188c81 = 0;
        D_00155422 = 1;
        D_00155423 = 0;
        D_00155422 = myhosp->fc94;
        D_00155423 = 0x32;
        LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        town.unknown0 = 3;
        town.unknown2 = 0xc;
        calculate_land_amount();
        set_font_from_cache(0xc);
        LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        break;
    case 0x45:
        fn_000669fc();
        process_bid_panel();
        process_bid_time();
        break;
    case 0xc:
        D_00155421 = 1;
        fn_0006680c();
        town.unknown2 = 0x45;
        total = 0;
        for (i = 0; i < 0x4000; i++) {
            if (level->plots[i] == D_00155430)
                total += gbv.f7c;
        }
        plotbid.price = total;
        if (total > 0) {
            plotbid.step = total / 10;
            if ((u32)plotbid.step < 10)
                plotbid.step = 10;
        }
        for (i = 0; i < 4; i++)
            plotbid.bids[i] = 0;
        plotbid.bidder = 0x63;
        plotbid.timer = 0x64;
        break;
    default:
        town.unknown2 = 0;
        break;
    }
    if (D_00155e42)
        D_00155e42--;
}

// FUNCTION: 0x00063a64
void fn_00063a64(void)
{
    switch (town.unknown2) {
    case 0:
    case 1:
    case 4:
    case 5:
    case 6:
    case 0xb:
    case 0xc:
    case 0x45:
        return;
    default:
        break;
    }
    switch (lbInkey) {
    case 1:
        town.unknown2 = 1;
        break;
    default:
        break;
    }
    lbInkey = 0;
}
