#include "hosp.h"

extern s32 current_frame;
extern char *text1;
extern s16 text_ofs;
extern s16 text_paragraph;
extern s16 text_delay;
extern s16 yofs_text;
extern char *display_text;
extern char text_buf;
extern Menu area;

void play_midi(u8 track);
void concat_composite(char *dst, char **strs);
void letter_format_text(char *dst, char *src, s16 width);
void __stdcall lose_call(void *buf, u32 a, u32 b);

extern "C" {
extern s8 anim_choice;
extern s8 text_choice;
extern void *smack_draw_callback;
extern u8 *palette;
extern Event D_001549d0[];
extern char **win_lose;
extern char **win_text;
extern char **town_names;

u32 time(u32 *timer);
void setup_lores_screen(void);
void insert_hall_details(void);
int LbScreenLock(void);
void LbTextSetClipWindow(s32 x, s32 y, s32 width, s32 height);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 width);

int strcmp(const char *a, const char *b);
void strcat(char *dst, char *src);
void LbDrawLine(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
}

// Smacker frame callback for the losing animation: once its text frame is
// reached, draws the chosen losing line (text1) over the frame in the back
// buffer, with scan lines over the top half.
// FUNCTION: 0x0009ded0
void __stdcall lose_call(void *buf, u32 a, u32 b)
{
    s16 x;
    s16 width;
    s16 y;

    if (buf == lbDisplay.wscreen) {
        a = b;
        current_frame++;
        if (current_frame >= animation_table[anim_choice].text_frame) {
            y = 0;
            if (anim_choice != ANIM_LOSE3)
                set_font_from_cache(0x13);
            else
                set_font_from_cache(0x15);
            width = LbTextStringWidth(text1);
            if (width > lbDisplay.physical_screen_width) {
                if (anim_choice != ANIM_LOSE3)
                    set_font_from_cache(0x14);
                else
                    set_font_from_cache(0x16);
                width = LbTextStringWidth(text1);
                y = 4;
            }
            x = (lbDisplay.graphics_screen_width - width) / 2;
            LbTextDraw(x, y + 0x83, text1);
            for (y = 1; y < lbDisplay.graphics_screen_height / 2; y += 2)
                LbDrawLine(0, y, lbDisplay.graphics_screen_width, y, color.black);
        }
    }
}

// Breaks text in place into lines at most width pixels wide in the current
// font, turning spaces into newlines.
// FUNCTION: 0x0009e010
void format_text(char *text, s16 width)
{
    u8 saved;
    char *line;
    char *cursor;

    line = text;
    cursor = line;
    for (;;) {
        saved = *cursor;
        *cursor = 0;
        while (LbTextStringWidth(line) <= width) {
            *cursor = saved;
            if (!saved)
                return;
            cursor++;
            saved = *cursor;
            *cursor = 0;
        }
        *cursor = saved;
        do {
            --cursor;
        } while (*cursor != ' ');
        *cursor = '\n';
        ++cursor;
        line = cursor;
    }
}

// Turns the newlines in text into spaces.
// FUNCTION: 0x0009e0a0
void remove_newlines(char *text)
{
    while (*text) {
        if (*text == '\n')
            *text = ' ';
        text++;
    }
}

// Runs the end-of-level sequence (winlose.state): 2 lose (a random losing
// animation and line, then the hall of fame or, in a network game, the main
// menu; 4 waits for a key), 1 win (the animation and a typed-out letter, 5:
// accept to go to the next level or the bonus level, or decline to keep
// playing), 3 the game's ending, 6 a win letter that returns to the main
// menu.
// FUNCTION: 0x0009e0d8
void play_winlose(void)
{
    u8 had_screen;
    u8 pick;
    s32 n;
    u8 done;
    char buf[0x800];

    had_screen = lbDisplay.wscreen ? (u8)1 : (u8)0;
    unload_game_files();
    stop_midi();
    othervar.music_playing = 0;
    switch (winlose.state) {
    case 2:
        finish_composite_help();
        clear_loading_screen(0);
        safeseed = time(0);
        anim_choice = saferand() % 6;
        n = anim_choice;
        if (anim_choice == ANIM_LOSE3) {
            anim_choice = ANIM_LOSE2;
            n = anim_choice;
        }
        if (othervar.music_playing)
            stop_midi();
        text_choice = 0;
        for (; n != 0; n--) {
            while (strcmp(win_lose[text_choice++], "."))
                ;
        }
        while (strcmp(win_lose[text_choice + n], ".") != 0)
            n++;
        text_choice += saferand() % n;
        text1 = win_lose[text_choice];
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        set_mouse_mode(MM_SCREEN);
        smack_draw_callback = (void *)lose_call;
        hosp_play_smk(anim_choice);
        if (statvar.num_players > 1) {
            mainmenu.state = MMS_UNLOAD_GAME;
            display_mode = DM_MAIN_MENU;
            setup_lores_screen();
            quit_game = 0;
        } else {
            winlose.state = 4;
            lbDisplay.left_button = 0;
            insert_hall_details();
            lbDisplay.rleft_button = 0;
            lbInkey = 0;
        }
        break;
    case 4:
        if (lbDisplay.rleft_button || lbInkey) {
            lbDisplay.left_button = 0;
            lbInkey = 0;
            mainmenu.state = MMS_UNLOAD_GAME;
            display_mode = DM_MAIN_MENU;
            clear_loading_screen(0);
            setup_lores_screen();
            D_001549d0[player].value = 0;
            set_mouse_mode(MM_NORMAL);
        }
        break;
    case 1:
        finish_composite_help();
        clear_loading_screen(0);
        set_mouse_mode(MM_WINLOSE);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        hosp_play_smk(ANIM_WIN_LEVEL);
        load_ingame_pic(SCR_NONE, 0, 0x29, 0, SCREEN_MODE_13);
        winlose.state = 5;
        lbDisplay.left_button = 0;
        insert_hall_details();
        set_font_from_cache(0x17);
        lbDisplay.draw_flags = text_ofs = text_paragraph = 0;
        text_delay = 1;
        if (level_number != 0xc) {
            safeseed = time(0);
            pick = saferand() % 4;
            if (gamevar.win_declined_count % 2 == 1)
                pick += 4;
            concat_composite(othervar.help_msg, &win_text[statvar.winlose_text_start[pick]]);
            sprintf(buf, othervar.help_msg, myhosp, othervar.next_level_offer_s, town_names[level_number + 1]);
            letter_format_text(othervar.help_msg, buf, resvars.screen_boxes_Boxp[0xae].w);
            lbDisplay.rleft_button = 0;
        } else {
            pick = saferand() % 4 + 8;
            concat_composite(othervar.help_msg, &win_text[statvar.winlose_text_start[pick]]);
            sprintf(buf, othervar.help_msg, myhosp, othervar.next_level_offer_s);
            letter_format_text(othervar.help_msg, buf, resvars.screen_boxes_Boxp[0xae].w);
            lbDisplay.rleft_button = 0;
        }
        break;
    case 3:
        finish_composite_help();
        clear_loading_screen(0);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        set_mouse_mode(MM_SCREEN);
        smack_draw_callback = 0;
        hosp_play_smk(ANIM_WIN_GAME);
        quit_game = 1;
        break;
    case 5:
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        reqspr_draw(resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y, &screen_sprites_tab_start[1]);
        reqspr_draw(resvars.screen_coords[0xd0].x, resvars.screen_coords[0xd0].y, &screen_sprites_tab_start[3]);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y, screen_sprites_tab_start[1].width,
                screen_sprites_tab_start[1].height))
            reqspr_draw(resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y, &screen_sprites_tab_start[2]);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xd0].x, resvars.screen_coords[0xd0].y, screen_sprites_tab_start[1].width,
                screen_sprites_tab_start[1].height))
            reqspr_draw(resvars.screen_coords[0xd0].x, resvars.screen_coords[0xd0].y, &screen_sprites_tab_start[4]);
        if (!--text_delay) {
            LbTextSetJustifyWindow(resvars.screen_boxes_Boxp[0xae].x, resvars.screen_boxes_Boxp[0xae].y, resvars.screen_boxes_Boxp[0xae].w + 0x64);
            LbTextSetClipWindow(resvars.screen_boxes_Boxp[0xae].x, 0, resvars.screen_boxes_Boxp[0xae].w, lbDisplay.graphics_screen_width);
            set_font_from_cache(0x17);
            lbDisplay.draw_flags |= 0x20;
            yofs_text = 0;
            display_text = othervar.help_msg;
            if (display_text) {
                if (display_text[text_ofs]) {
                    text_buf = display_text[text_ofs];
                    display_text[text_ofs] = 0;
                    LbTextDraw(0, yofs_text, display_text);
                    display_text[text_ofs] = text_buf;
                    text_ofs++;
                    text_delay = 1;
                    play_sound(SNDSRC_GENERAL, SFX_WINLOSE_TYPE, -1, -1);
                }
            }
        }
        if (lbDisplay.left_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                if (level_number != 0xc) {
                    D_001549d0[player].type = EV_CANCEL_BUILD;
                    area.state = 0;
                    level_number++;
                    if (othervar.bonus_level && resvars.game_mode == SCREEN_MODE_13) {
                        othervar.level_after_bonus = level_number;
                        level_number = othervar.bonus_level;
                        othervar.bonus_level = 0;
                    }
                    myhosp->level = level_number;
                    myhosp->score = othervar.next_level_offer_s;
                    display_mode = DM_AREA_SELECT;
                } else {
                    D_001549d0[player].type = EV_CANCEL_BUILD;
                    level_number = myhosp->level = 0;
                    myhosp->score = othervar.next_level_offer_s;
                    display_mode = DM_AREA_SELECT;
                    set_mouse_mode(MM_SCREEN);
                    fade_out_menu();
                    clear_loading_screen(0);
                    smack_draw_callback = 0;
                    hosp_play_smk(ANIM_WIN_GAME);
                    mainmenu.state = MMS_CREDITS;
                    display_mode = DM_MAIN_MENU;
                    quit_game = 0;
                }
            } else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xd0].x, resvars.screen_coords[0xd0].y,
                           screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                gamevar.win_declined_count++;
                othervar.win_declined = 1;
                othervar.bonus_level = 0;
                pick = display_loading_screen();
                load_game_files();
                if (pick)
                    clear_loading_screen(palette);
                restore_display_screen();
                if (othervar.midi_playing && myplayer.music_on)
                    play_midi(othervar.midi_track);
            } else {
                do {
                    text_ofs++;
                } while (display_text[text_ofs]);
                text_ofs--;
            }
            lbDisplay.left_button = 0;
        }
        break;
    case 6:
        finish_composite_help();
        set_mouse_mode(MM_WINLOSE);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        hosp_play_smk(ANIM_WIN_LEVEL);
        load_ingame_pic(SCR_NONE, 0, 0x29, 0, SCREEN_MODE_13);
        winlose.state = 5;
        set_font_from_cache(0x17);
        lbDisplay.draw_flags = text_ofs = text_paragraph = 0;
        text_delay = 1;
        pick = saferand() % 4 + 8;
        concat_composite(othervar.help_msg, &win_text[statvar.winlose_text_start[pick]]);
        sprintf(buf, othervar.help_msg, myhosp, 100000);
        letter_format_text(othervar.help_msg, buf, resvars.screen_boxes_Boxp[0xae].w);
        lbDisplay.left_button = 0;
        lbDisplay.rleft_button = 0;
        if (had_screen)
            LbScreenUnlock();
        for (done = 0; !done;) {
            while (LbScreenLock() != 1)
                ;
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            reqspr_draw(resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y, &screen_sprites_tab_start[1]);
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height))
                reqspr_draw(resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y, &screen_sprites_tab_start[2]);
            if (!--text_delay) {
                LbTextSetJustifyWindow(resvars.screen_boxes_Boxp[0xae].x, resvars.screen_boxes_Boxp[0xae].y, resvars.screen_boxes_Boxp[0xae].w + 0x64);
                LbTextSetClipWindow(resvars.screen_boxes_Boxp[0xae].x, 0, resvars.screen_boxes_Boxp[0xae].w, lbDisplay.graphics_screen_width);
                set_font_from_cache(0x17);
                lbDisplay.draw_flags |= 0x20;
                yofs_text = 0;
                display_text = othervar.help_msg;
                if (display_text) {
                    if (display_text[text_ofs]) {
                        text_buf = display_text[text_ofs];
                        display_text[text_ofs] = 0;
                        LbTextDraw(0, yofs_text, display_text);
                        display_text[text_ofs] = text_buf;
                        text_ofs++;
                        text_delay = 1;
                        play_sound(SNDSRC_GENERAL, SFX_WINLOSE_TYPE, -1, -1);
                    }
                }
            }
            if (lbDisplay.left_button) {
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_coords[0xcf].x, resvars.screen_coords[0xcf].y,
                        screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                    set_mouse_mode(MM_SCREEN);
                    clear_loading_screen(0);
                    smack_draw_callback = 0;
                    hosp_play_smk(ANIM_WIN_GAME);
                    mainmenu.state = MMS_UNLOAD_GAME;
                    display_mode = DM_MAIN_MENU;
                    quit_game = 0;
                    done = 1;
                } else {
                    do {
                        text_ofs++;
                    } while (display_text[text_ofs]);
                    text_ofs--;
                }
                lbDisplay.left_button = 0;
            }
            LbScreenUnlock();
            LbScreenSwap();
        }
        if (had_screen) {
            while (LbScreenLock() != 1)
                ;
        }
        othervar.help_msg[0] = 0;
        break;
    }
}

// Copies src to dst word-wrapped to width pixels in the current font, '/'
// forcing a new line.
// FUNCTION: 0x0009ef00
void letter_format_text(char *dst, char *src, s16 width)
{
    s32 unused;
    u32 line_width;
    char *word_end;
    u32 word_width;

    unused = 0;
    line_width = 0;
    while (*src) {
        while (*src == ' ')
            src++;
        if (!*src)
            continue;
        if (*src == '/') {
            *dst++ = '\n';
            line_width = 0;
            src++;
            continue;
        }
        word_end = src;
        word_width = 0;
        while (!(*word_end == ' ' || *word_end == '/' || (s8)*word_end == 0)) {
            word_width += lbFontPtr ? lbFontPtr[*word_end - 31].width : 0;
            word_end++;
        }
        if (word_width + line_width > width) {
            *dst++ = '\n';
            line_width = 0;
        }
        while (!(*src == ' ' || *src == '/' || (s8)*src == 0))
            *dst++ = *src++;
        *dst++ = ' ';
        line_width += word_width + (lbFontPtr ? lbFontPtr[1].width : 0);
    }
    *dst = 0;
}

// Appends the strings from strs up to the "." entry to dst.
// FUNCTION: 0x0009f06c
void concat_composite(char *dst, char **strs)
{
    for (; strcmp(*strs, ".") != 0; strs++)
        strcat(dst, *strs);
}
