#include "hosp.h"

extern Menu loadgame;
extern Menu nickname;
extern s32 current_frame;

void help_hotspot_detected(u16 hotspot, u8 help_group, s16 x, s16 y);
void tri_test(void);
u8 check_tri(u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 x, u16 y);
void display_main_menu(void);
void play_main_menu_intro(void);

extern "C" {
extern void *smack_draw_callback;

int LbScreenLock(void);
void setup_lores_screen(void);
void use_hires_help_and_fonts(void);
void use_hires_layout(void);
void fn_000905f4(void);
void play_main_menu_outro(void);
u8 play_intro(void);
u8 play_attract_mode(void);
u8 play_credits(s16 *scroll, u16 *line);

extern u8 *palette;
extern u8 D_00156910;
extern u8 anim_choice;
extern char **credit_text;
}

s8 compare_products(s32 a, s32 b, s32 c, s32 d);
#pragma aux compare_products = \
    0xf7 0xea 0x93 0x87 0xca 0xf7 0xea 0x29 0xc3 0x0f 0x95 0xc0 0x19 0xd1 \
    0x74 0x08 0x0f 0x9c 0xc4 0x0f 0x9f 0xc0 0x28 0xe0 \
    parm [eax] [ebx] [ecx] [edx] value [al] modify [eax ebx ecx edx];

// Runs one frame of the main menu's state machine (mainmenu.state): loads
// and plays the intro animation, draws the menu with the hovered segment lit,
// and carries out a chosen entry (new game, load, network, tutorial or quit)
// by setting display_mode or quit_game. Also runs the attract loop and the
// credits.
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
    switch (mainmenu.state) {
    case MMS_ENTER:
        return_from_screen(9, 0, 0);
    case MMS_UNLOAD_GAME:
        lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = 0;
        unload_game_files();
        mainmenu.state = MMS_SHOW_MENU;
        break;
    case MMS_SHOW_MENU:
        clear_loading_screen(0);
        LbMouseReset();
        LbMouseChangeSprite(0);
        setup_lores_screen();
        use_hires_help_and_fonts();
        lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = 0;
        load_ingame_pic(SCR_MAIN_MENU, 1, 0x14, 0, SCREEN_MODE_1);
        LbScreenUnlock();
        LbScreenClear(0);
        LbScreenSwap();
        while (LbScreenLock() != 1) {
        }
        play_main_menu_intro();
        set_mouse_mode(MM_MAIN_MENU);
        LbMouseSetup(&screen_pointers_tab_start[othervar.pointer_sprite]);
        setup_colors();
        mainmenu.state = MMS_HOVER_CONTINUE;
        break;
    case MMS_LOAD_GAME:
        LbMouseChangeSprite(0);
        play_main_menu_outro();
        fade_out_menu();
        othervar.pending_screen_mode = SCREEN_MODE_13;
        display_mode = DM_LOAD_GAME;
        mainmenu.state = MMS_ENTER;
        loadgame.state = 0;
        break;
    case MMS_NEW_GAME:
        LbMouseChangeSprite(0);
        play_main_menu_outro();
        fade_out_menu();
        display_mode = DM_NICKNAME;
        nickname.state = 3;
        mainmenu.state = MMS_ENTER;
        othervar.pending_screen_mode = SCREEN_MODE_13;
        level_number = 0;
        break;
    case MMS_CONTINUE:
        if (save_game_valid(0x63) == 0) {
            mainmenu.state = MMS_HOVER_CONTINUE;
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            break;
        }
        play_main_menu_outro();
        fade_out_menu();
        othervar.pending_screen_mode = SCREEN_MODE_13;
        mainmenu.state = MMS_LOAD_CONTINUE;
        break;
    case MMS_LOAD_CONTINUE:
        return_from_screen(0, 0, 1);
        LbScreenClear(0);
        if (!display_loading_screen())
            break;
        level_number = read_level_number(0x63);
        if (level_number != -1) {
            use_hires_layout();
            load_game_files();
            setup_new_level((u16)level_number, 1);
            if (load_game(0x63, -1)) {
                othervar.screen_pic_loaded = 0;
                set_mouse_mode(MM_NORMAL);
                LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
                display_mode = DM_GAME;
                mainmenu.state = MMS_ENTER;
                load_game_files();
                clear_loading_screen(palette);
                D_00156910 = 1;
                set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height,
                                                3);
                draw_it();
                othervar.pending_palette = palette;
                break;
            }
        }
        mainmenu.state = MMS_CONTINUE_FAILED;
        break;
    case MMS_CONTINUE_FAILED:
        setup_lores_screen();
        mainmenu.state = MMS_ENTER;
        break;
    case MMS_NETWORK:
        LbMouseChangeSprite(0);
        play_main_menu_outro();
        fade_out_menu();
        display_mode = DM_NETWORK_SELECT;
        mainmenu.state = MMS_ENTER;
        othervar.pending_screen_mode = SCREEN_MODE_13;
        break;
    case 0xb:
    case MMS_QUIT:
        LbMouseChangeSprite(0);
        play_main_menu_outro();
        fade_out_menu();
        quit_game = 3;
        break;
    case MMS_HOVER_CONTINUE:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb3].x, resvars.screen_coords[0xb3].y, &screen_sprites_tab_start[2]);
        draw_help_text();
        break;
    case MMS_HOVER_LOAD:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb4].x, resvars.screen_coords[0xb4].y, &screen_sprites_tab_start[3]);
        draw_help_text();
        break;
    case MMS_HOVER_NEW_GAME:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb5].x, resvars.screen_coords[0xb5].y, &screen_sprites_tab_start[1]);
        draw_help_text();
        break;
    case MMS_HOVER_NETWORK:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb6].x, resvars.screen_coords[0xb6].y, &screen_sprites_tab_start[4]);
        draw_help_text();
        break;
    case MMS_HOVER_QUIT:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb7].x, resvars.screen_coords[0xb7].y, &screen_sprites_tab_start[5]);
        draw_help_text();
        break;
    case 0x11:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb8].x, resvars.screen_coords[0xb8].y, &screen_sprites_tab_start[6]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case 0x12:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xb9].x, resvars.screen_coords[0xb9].y, &screen_sprites_tab_start[7]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case 0x13:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xba].x, resvars.screen_coords[0xba].y, &screen_sprites_tab_start[8]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case 0x14:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xbb].x, resvars.screen_coords[0xbb].y, &screen_sprites_tab_start[9]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case 0x15:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xbc].x, resvars.screen_coords[0xbc].y, &screen_sprites_tab_start[10]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case 0x16:
        display_main_menu();
        reqspr_draw(resvars.screen_coords[0xbd].x, resvars.screen_coords[0xbd].y, &screen_sprites_tab_start[11]);
        play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
        break;
    case MMS_INTRO:
        lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = 0;
        LbMouseChangeSprite(0);
        play_intro();
        fade_out_menu();
        othervar.pending_screen_mode = SCREEN_MODE_13;
        mainmenu.state = MMS_TITLE;
        break;
    case MMS_TITLE:
        load_ingame_pic(SCR_TITLE, 1, 9, 0, SCREEN_MODE_13);
        othervar.saved_screen_ptr = lbDisplay.wscreen;
        othervar.saved_screen_width = lbDisplay.graphics_screen_width;
        othervar.saved_screen_height = lbDisplay.graphics_screen_height;
        lbDisplay.wscreen = bscreen;
        lbDisplay.graphics_screen_width = 0x280;
        lbDisplay.graphics_screen_height = 0x1e0;
        LbScreenSetGraphicsWindow(0, 0, 0x280, 0x1e0);
        fn_000905f4();
        lbDisplay.wscreen = othervar.saved_screen_ptr;
        lbDisplay.graphics_screen_width = othervar.saved_screen_width;
        lbDisplay.graphics_screen_height = othervar.saved_screen_height;
        LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
        fn_000905f4();
        mainmenu.state = MMS_ATTRACT;
        break;
    case MMS_ATTRACT:
        while (lbInkey + lbDisplay.mleft_button + lbDisplay.mright_button) {
            lbDisplay.left_button = lbDisplay.right_button = lbInkey = 0;
        }
        done = 0;
        timer = 0x3e8;
        sprintf(path, "%s%s", cd_install_path, animation_table[ANIM_ATTRACT].name);
        while (!done) {
            copy_screen_to_wscreen(bscreen);
            if (exit_quick_requester()) {
                done = 1;
            } else if (LbFileExists(path)) {
                if (--timer == 0) {
                    while (lbInkey + lbDisplay.mleft_button + lbDisplay.mright_button) {
                        lbDisplay.left_button = lbDisplay.right_button = lbInkey = 0;
                    }
                    fade_out_menu();
                    count++;
                    if (count % 2 == 1)
                        done = play_attract_mode();
                    else
                        done = play_intro();
                    fade_out_menu();
                    load_ingame_pic(SCR_TITLE, 1, 9, 0, SCREEN_MODE_13);
                    othervar.saved_screen_ptr = lbDisplay.wscreen;
                    othervar.saved_screen_width = lbDisplay.graphics_screen_width;
                    othervar.saved_screen_height = lbDisplay.graphics_screen_height;
                    lbDisplay.wscreen = bscreen;
                    lbDisplay.graphics_screen_width = 0x280;
                    lbDisplay.graphics_screen_height = 0x1e0;
                    LbScreenSetGraphicsWindow(0, 0, 0x280, 0x1e0);
                    fn_000905f4();
                    lbDisplay.wscreen = othervar.saved_screen_ptr;
                    lbDisplay.graphics_screen_width = othervar.saved_screen_width;
                    lbDisplay.graphics_screen_height = othervar.saved_screen_height;
                    LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
                    fn_000905f4();
                    timer = 0x3e8;
                    if (!done)
                        hosp_fade_palette(othervar.fade_palette, 10, Lb_PALETTE_FADE_CLOSED);
                }
            }
            if (lbDisplay.mleft_button + lbDisplay.mright_button + lbInkey + lbDisplay.left_button +
                lbDisplay.right_button) {
                lbDisplay.left_button = lbDisplay.rleft_button = lbDisplay.right_button = lbInkey = 0;
                done = 1;
            }
            if (done) {
                mainmenu.state = MMS_TO_NICKNAME;
                lbDisplay.left_button = lbDisplay.right_button = 0;
            }
            LbScreenUnlock();
            LbScreenSwap();
            while (LbScreenLock() != 1) {
            }
        }
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        break;
    case MMS_TO_NICKNAME:
        display_mode = DM_NICKNAME;
        nickname.state = 0;
        mainmenu.state = MMS_ENTER;
        break;
    case MMS_CREDITS:
        load_ingame_pic(SCR_CREDITS, 1, 9, 0, SCREEN_MODE_13);
        mainmenu.state = MMS_CREDITS_SCROLL;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0x72, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        set_mouse_mode(MM_SCREEN);
        break;
    case MMS_CREDITS_SCROLL:
        lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
        lbInkey = 0;
        scroll = 0;
        line = 0;
        play_credits(&scroll, &line);
        done = 0;
        while (!done) {
            copy_screen_to_wscreen(bscreen);
            if (play_credits(&scroll, &line))
                done = 1;
            LbScreenUnlock();
            LbScreenSwap();
            while (LbScreenLock() != 1) {
            }
        }
        mainmenu.state = MMS_CREDITS_DONE;
        break;
    case MMS_CREDITS_DONE:
        mainmenu.state = MMS_UNLOAD_GAME;
        fade_out_menu();
        setup_lores_screen();
        break;
    default:
        mainmenu.state = MMS_ENTER;
        break;
    }
}

// Main menu keys: Escape or Q quits the game (quit_game = 3); any key ends
// the attract loop.
// FUNCTION: 0x00062730
void process_keys_main_menu(void)
{
    switch (mainmenu.state) {
    case MMS_ATTRACT:
        if (lbInkey)
            mainmenu.state = MMS_TO_NICKNAME;
    case MMS_INTRO:
    case MMS_TO_NICKNAME:
        return;
    default:
        break;
    }
    switch (lbInkey) {
    case KC_ESCAPE:
    case KC_Q:
        fade_out_menu();
        quit_game = 3;
        lbInkey = 0;
        break;
    default:
        break;
    }
}

// Main menu clicks: a release inside one of the menu's triangles selects
// that entry by setting mainmenu.state. Clears the button state.
// FUNCTION: 0x000627cc
void process_mouse_main_menu(void)
{
    switch (mainmenu.state) {
    case MMS_ENTER:
    case MMS_NEW_GAME:
    case MMS_NETWORK:
    case MMS_LOAD_CONTINUE:
    case MMS_LOAD_GAME:
    case 1:
    case 2:
    case MMS_CONTINUE:
    case 0xb:
    case MMS_QUIT:
    case MMS_SHOW_MENU:
        return;
    case MMS_ATTRACT:
        if (lbDisplay.left_button + lbDisplay.right_button)
            mainmenu.state = MMS_TO_NICKNAME;
    case MMS_INTRO:
    case MMS_TITLE:
    case MMS_TO_NICKNAME:
        return;
    default:
        break;
    }
    if (lbDisplay.rleft_button) {
        lbDisplay.rleft_button = 0;
        if (check_tri(0x2c, 0x13, 0x55, 0x3a, 0x7f, 0x13, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_NEW_GAME;
        if (check_tri(0x55, 0x3a, 0x7f, 0x68, 0x7f, 0x13, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_LOAD_GAME;
        if (check_tri(0x2c, 0x13, 0x2c, 0x68, 0x55, 0x3a, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_CONTINUE;
        if (check_tri(0x2c, 0x68, 0x7f, 0x68, 0x55, 0x3a, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_NETWORK;
        if (check_tri(0x30, 0x77, 0x30, 0x96, 0x74, 0x79, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_QUIT;
        if (check_tri(0x74, 0x79, 0x30, 0x96, 0x74, 0x96, lbDisplay.rmouse_x, lbDisplay.rmouse_y))
            mainmenu.state = MMS_QUIT;
    }
    lbDisplay.left_button = lbDisplay.rleft_button = lbDisplay.right_button = lbDisplay.rright_button = 0;
}

// Draws the main menu background and updates the hovered entry.
// FUNCTION: 0x00062a50
void display_main_menu(void)
{
    copy_screen_to_wscreen(bscreen);
    tri_test();
    set_font_from_cache(0);
}

// Hit-tests the mouse against the main menu's triangles and sets the hover
// state in mainmenu.state, or shows the entry's help when it is already
// hovered.
// FUNCTION: 0x00062a7c
void tri_test(void)
{
    u8 unused;

    unused = 0;
    if (check_tri(0x2c, 0x13, 0x55, 0x3a, 0x7f, 0x13, lbDisplay.mmouse_x, lbDisplay.mmouse_y)) {
        if (mainmenu.state != MMS_HOVER_NEW_GAME) {
            switch (mainmenu.state) {
            case MMS_HOVER_CONTINUE:
                mainmenu.state = 0x11;
                break;
            case MMS_HOVER_LOAD:
                mainmenu.state = 0x13;
                break;
            case MMS_HOVER_NETWORK:
                mainmenu.state = 0x12;
                break;
            case MMS_HOVER_QUIT:
                mainmenu.state = 0x12;
                break;
            default:
                mainmenu.state = MMS_HOVER_NEW_GAME;
                break;
            }
        } else {
            help_hotspot_detected(1, 1, 0, 0);
        }
    } else if (check_tri(0x55, 0x3a, 0x7f, 0x68, 0x7f, 0x13, lbDisplay.mmouse_x, lbDisplay.mmouse_y)) {
        if (mainmenu.state != MMS_HOVER_LOAD) {
            switch (mainmenu.state) {
            case MMS_HOVER_NEW_GAME:
                mainmenu.state = 0x13;
                break;
            case MMS_HOVER_CONTINUE:
                mainmenu.state = 0x12;
                break;
            case MMS_HOVER_NETWORK:
                mainmenu.state = 0x14;
                break;
            case MMS_HOVER_QUIT:
                mainmenu.state = 0x14;
                break;
            default:
                mainmenu.state = MMS_HOVER_LOAD;
                break;
            }
        } else {
            help_hotspot_detected(2, 1, 0, 0);
        }
    } else if (check_tri(0x2c, 0x13, 0x2c, 0x68, 0x55, 0x3a, lbDisplay.mmouse_x, lbDisplay.mmouse_y)) {
        if (mainmenu.state != MMS_HOVER_CONTINUE) {
            switch (mainmenu.state) {
            case MMS_HOVER_NEW_GAME:
                mainmenu.state = 0x11;
                break;
            case MMS_HOVER_LOAD:
                mainmenu.state = 0x12;
                break;
            case MMS_HOVER_NETWORK:
                mainmenu.state = 0x16;
                break;
            case MMS_HOVER_QUIT:
                mainmenu.state = 0x16;
                break;
            default:
                mainmenu.state = MMS_HOVER_CONTINUE;
                break;
            }
        } else {
            help_hotspot_detected(3, 1, 0, 0);
        }
    } else if (check_tri(0x2c, 0x68, 0x7f, 0x68, 0x55, 0x3a, lbDisplay.mmouse_x, lbDisplay.mmouse_y)) {
        if (mainmenu.state != MMS_HOVER_NETWORK) {
            switch (mainmenu.state) {
            case MMS_HOVER_NEW_GAME:
                mainmenu.state = 0x12;
                break;
            case MMS_HOVER_LOAD:
                mainmenu.state = 0x14;
                break;
            case MMS_HOVER_CONTINUE:
                mainmenu.state = 0x16;
                break;
            case MMS_HOVER_QUIT:
                mainmenu.state = 0x15;
                break;
            default:
                mainmenu.state = MMS_HOVER_NETWORK;
                break;
            }
        } else {
            help_hotspot_detected(4, 1, 0, 0);
        }
    } else if (check_tri(0x30, 0x77, 0x30, 0x96, 0x74, 0x79, lbDisplay.mmouse_x, lbDisplay.mmouse_y) ||
               check_tri(0x74, 0x79, 0x30, 0x96, 0x74, 0x96, lbDisplay.mmouse_x, lbDisplay.mmouse_y)) {
        if (mainmenu.state != MMS_HOVER_QUIT) {
            switch (mainmenu.state) {
            case MMS_HOVER_NEW_GAME:
                mainmenu.state = 0x12;
                break;
            case MMS_HOVER_LOAD:
                mainmenu.state = 0x14;
                break;
            case MMS_HOVER_CONTINUE:
                mainmenu.state = 0x16;
                break;
            case MMS_HOVER_NETWORK:
                mainmenu.state = 0x15;
                break;
            default:
                mainmenu.state = MMS_HOVER_QUIT;
                break;
            }
        } else {
            help_hotspot_detected(5, 1, 0, 0);
        }
    } else {
        switch (mainmenu.state) {
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            mainmenu.state = MMS_HOVER_CONTINUE;
            break;
        default:
            break;
        }
    }
    switch (mainmenu.state) {
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

// Returns 1 if point (x, y) lies inside or on the triangle (x1, y1), (x2, y2),
// (x3, y3), given in counterclockwise order on screen (y down).
// FUNCTION: 0x00062f34
u8 check_tri(u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 x, u16 y)
{
    s32 outside;

    outside = compare_products(x2 - x1, y2 - y1, x - x1, y - y1) > 0 ||
              compare_products(x3 - x2, y3 - y2, x - x2, y - y2) > 0 ||
              compare_products(x1 - x3, y1 - y3, x - x3, y - y3) > 0;
    return outside == 0;
}

// Plays the main menu's opening animation (smacker 0x16) full screen.
// FUNCTION: 0x000630a4
void play_main_menu_intro(void)
{
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_mouse_mode(MM_SCREEN);
    smack_draw_callback = 0;
    hosp_play_smk(ANIM_MAIN_MENU1);
}

// Plays the main menu's closing animation (smacker 0x17) full screen.
// FUNCTION: 0x0006310c
void play_main_menu_outro(void)
{
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_mouse_mode(MM_SCREEN);
    smack_draw_callback = 0;
    hosp_play_smk(ANIM_MAIN_MENU2);
}

// Plays the intro movie from the CD (the high-quality version if the CD is
// fast enough), then restores the 320x200 mode. Returns 1 if the player
// skipped it or the screen mode could not be restored, 0 otherwise or if
// the movie is missing.
// FUNCTION: 0x00063174
u8 play_intro(void)
{
    u8 aborted;
    char path[0x80];
    u8 was_locked;
    u8 result;

    was_locked = lbDisplay.wscreen ? (u8)1 : (u8)0;
    aborted = 0;
    sprintf(path, "%s%s", cd_install_path, animation_table[othervar.cd_speed > 4 ? 0x1a : 0x19].name);
    if (!LbFileExists(path))
        return aborted;
    fade_out_menu();
    if (was_locked)
        LbScreenUnlock();
    LbScreenClear(0);
    setup_lores_screen();
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_mouse_mode(MM_SCREEN);
    smack_draw_callback = 0;
    if (othervar.cd_speed > 4)
        result = hosp_play_smk(ANIM_INTRO_SM4);
    else
        result = hosp_play_smk(ANIM_INTRO_SM2);
    if (result == SMK_SKIPPED)
        aborted = 1;
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
    if (!setup_any_screen(palette, SCREEN_MODE_13)) {
        reset_host();
        return 1;
    }
    if (was_locked) {
        while (LbScreenLock() != 1) {
        }
    }
    set_mouse_mode(MM_SCREEN);
    return aborted;
}

// Plays the attract-mode movie (smacker 0x1b) in 320x200. Returns 1 if the
// player skipped it.
// FUNCTION: 0x00063300
u8 play_attract_mode(void)
{
    u8 aborted;
    u8 was_locked;

    aborted = 0;
    was_locked = lbDisplay.wscreen ? (u8)1 : (u8)0;
    if (was_locked)
        LbScreenUnlock();
    fade_out_menu();
    if (change_res(SCREEN_MODE_13)) {
        LbScreenClear(0);
        LbScreenSwap();
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        set_mouse_mode(MM_SCREEN);
        anim_choice = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        othervar.f9ca = saferand();
        othervar.f9cb = 0;
        othervar.f9cd = resvars.boxes_s32p[0xab] / 2 + 100;
        mainmenu.value = 0;
        aborted = 0;
        if (hosp_play_smk(ANIM_ATTRACT) == SMK_SKIPPED)
            aborted = 1;
        fade_out_menu();
        change_res(SCREEN_MODE_13);
        LbScreenClear(0);
    }
    if (was_locked) {
        while (LbScreenLock() != 1) {
        }
    }
    return aborted;
}

// Draws one frame of the scrolling credits from credit_text. *scroll is the
// pixel offset into the current top line and *line its index; both advance.
// Returns 1 at the '.' end marker or on a click.
// FUNCTION: 0x0006344c
u8 play_credits(s16 *scroll, u16 *line)
{
    u8 i;
    u8 height;
    s16 y;
    u8 done;
    char text[0x64];

    done = 0;
    set_font_from_cache(0x2a);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    lbDisplay.draw_flags = 0x100;
    if (height % 3)
        height = (height / 3 + 1) * 3;
    y = -*scroll;
    for (i = 0; y < lbDisplay.graphics_screen_height; y += height, i++) {
        if (*scroll >= -0x32) {
            sprintf(text, "%s", credit_text[i + *line]);
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
    if (lbDisplay.rleft_button + lbDisplay.rright_button) {
        done = 1;
        lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
        lbInkey = 0;
    }
    *scroll += 3;
    if (*scroll % height == 0) {
        *scroll = 0;
        (*line)++;
    }
    return done;
}
