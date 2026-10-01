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

u32 time(u32 *t);
void fn_0001102c(void);
void insert_hall_details(void);
int LbScreenLock(void);
void LbTextSetClipWindow(s32 x, s32 y, s32 w, s32 h);
void LbTextSetJustifyWindow(s32 x, s32 y, s32 w);

int strcmp(const char *a, const char *b);
void fn_000c93bf(char *dst, char *src);
void fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
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

    if (buf == lbDisplay.unknown4) {
        a = b;
        current_frame++;
        if (current_frame >= animation_table[anim_choice].f2e) {
            y = 0;
            if (anim_choice != 2)
                set_font_from_cache(0x13);
            else
                set_font_from_cache(0x15);
            width = LbTextStringWidth(text1);
            if (width > lbDisplay.unknown18) {
                if (anim_choice != 2)
                    set_font_from_cache(0x14);
                else
                    set_font_from_cache(0x16);
                width = LbTextStringWidth(text1);
                y = 4;
            }
            x = (lbDisplay.graphics_screen_width - width) / 2;
            LbTextDraw(x, y + 0x83, text1);
            for (y = 1; y < lbDisplay.graphics_screen_height / 2; y += 2)
                fn_000c9be0(0, y, lbDisplay.graphics_screen_width, y, color.unknown8);
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

// Runs the end-of-level sequence (winlose.unknown2): 2 lose (a random losing
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

    had_screen = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    unload_game_files();
    stop_midi();
    othervar.fa4f = 0;
    switch (winlose.unknown2) {
    case 2:
        finish_composite_help();
        clear_loading_screen(0);
        safeseed = time(0);
        anim_choice = saferand() % 6;
        n = anim_choice;
        if (anim_choice == 2) {
            anim_choice = 1;
            n = anim_choice;
        }
        if (othervar.fa4f)
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
        set_mouse_mode(9);
        smack_draw_callback = (void *)lose_call;
        hosp_play_smk(anim_choice);
        if (statvar.unknownfd > 1) {
            mainmenu.unknown2 = 0x1f;
            display_mode = 2;
            fn_0001102c();
            quit_game = 0;
        } else {
            winlose.unknown2 = 4;
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
            mainmenu.unknown2 = 0x1f;
            display_mode = 2;
            clear_loading_screen(0);
            fn_0001102c();
            D_001549d0[player].value = 0;
            set_mouse_mode(MM_NORMAL);
        }
        break;
    case 1:
        finish_composite_help();
        clear_loading_screen(0);
        set_mouse_mode(0x2a);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        hosp_play_smk(6);
        load_ingame_pic(0x18, 0, 0x29, 0, SCREEN_MODE_13);
        winlose.unknown2 = 5;
        lbDisplay.left_button = 0;
        insert_hall_details();
        set_font_from_cache(0x17);
        lbDisplay.draw_flags = text_ofs = text_paragraph = 0;
        text_delay = 1;
        if (level_number != 0xc) {
            safeseed = time(0);
            pick = saferand() % 4;
            if (gamevar.f176 % 2 == 1)
                pick += 4;
            concat_composite(othervar.f96, &win_text[statvar.f543[pick]]);
            sprintf(buf, othervar.f96, myhosp, othervar.f74_s32, town_names[level_number + 1]);
            letter_format_text(othervar.f96, buf, resvars.f3c_Boxp[0xae].w);
            lbDisplay.rleft_button = 0;
        } else {
            pick = saferand() % 4 + 8;
            concat_composite(othervar.f96, &win_text[statvar.f543[pick]]);
            sprintf(buf, othervar.f96, myhosp, othervar.f74_s32);
            letter_format_text(othervar.f96, buf, resvars.f3c_Boxp[0xae].w);
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
        set_mouse_mode(9);
        smack_draw_callback = 0;
        hosp_play_smk(0x18);
        quit_game = 1;
        break;
    case 5:
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        reqspr_draw(resvars.f40[0xcf].x, resvars.f40[0xcf].y, &screen_sprites_tab_start[1]);
        reqspr_draw(resvars.f40[0xd0].x, resvars.f40[0xd0].y, &screen_sprites_tab_start[3]);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xcf].x, resvars.f40[0xcf].y, screen_sprites_tab_start[1].width,
                screen_sprites_tab_start[1].height))
            reqspr_draw(resvars.f40[0xcf].x, resvars.f40[0xcf].y, &screen_sprites_tab_start[2]);
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xd0].x, resvars.f40[0xd0].y, screen_sprites_tab_start[1].width,
                screen_sprites_tab_start[1].height))
            reqspr_draw(resvars.f40[0xd0].x, resvars.f40[0xd0].y, &screen_sprites_tab_start[4]);
        if (!--text_delay) {
            LbTextSetJustifyWindow(resvars.f3c_Boxp[0xae].x, resvars.f3c_Boxp[0xae].y, resvars.f3c_Boxp[0xae].w + 0x64);
            LbTextSetClipWindow(resvars.f3c_Boxp[0xae].x, 0, resvars.f3c_Boxp[0xae].w, lbDisplay.graphics_screen_width);
            set_font_from_cache(0x17);
            lbDisplay.draw_flags |= 0x20;
            yofs_text = 0;
            display_text = othervar.f96;
            if (display_text) {
                if (display_text[text_ofs]) {
                    text_buf = display_text[text_ofs];
                    display_text[text_ofs] = 0;
                    LbTextDraw(0, yofs_text, display_text);
                    display_text[text_ofs] = text_buf;
                    text_ofs++;
                    text_delay = 1;
                    play_sound(10, 0xee, -1, -1);
                }
            }
        }
        if (lbDisplay.left_button) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xcf].x, resvars.f40[0xcf].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                if (level_number != 0xc) {
                    D_001549d0[player].type = 0x38;
                    area.unknown2 = 0;
                    level_number++;
                    if (othervar.fa76 && resvars.f1 == SCREEN_MODE_13) {
                        othervar.fa75 = level_number;
                        level_number = othervar.fa76;
                        othervar.fa76 = 0;
                    }
                    myhosp->f29 = level_number;
                    myhosp->f2d = othervar.f74_s32;
                    display_mode = 3;
                } else {
                    D_001549d0[player].type = 0x38;
                    level_number = myhosp->f29 = 0;
                    myhosp->f2d = othervar.f74_s32;
                    display_mode = 3;
                    set_mouse_mode(9);
                    fade_out_menu();
                    clear_loading_screen(0);
                    smack_draw_callback = 0;
                    hosp_play_smk(0x18);
                    mainmenu.unknown2 = 0x1c;
                    display_mode = 2;
                    quit_game = 0;
                }
            } else if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xd0].x, resvars.f40[0xd0].y,
                           screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                gamevar.f176++;
                othervar.fa5f = 1;
                othervar.fa76 = 0;
                pick = display_loading_screen();
                load_game_files();
                if (pick)
                    clear_loading_screen(palette);
                restore_display_screen();
                if (othervar.f9b9 && myplayer.unknown11)
                    play_midi(othervar.f9ba);
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
        set_mouse_mode(0x2a);
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        lbDisplay.draw_flags = 0;
        current_frame = 0;
        smack_draw_callback = 0;
        hosp_play_smk(6);
        load_ingame_pic(0x18, 0, 0x29, 0, SCREEN_MODE_13);
        winlose.unknown2 = 5;
        set_font_from_cache(0x17);
        lbDisplay.draw_flags = text_ofs = text_paragraph = 0;
        text_delay = 1;
        pick = saferand() % 4 + 8;
        concat_composite(othervar.f96, &win_text[statvar.f543[pick]]);
        sprintf(buf, othervar.f96, myhosp, 100000);
        letter_format_text(othervar.f96, buf, resvars.f3c_Boxp[0xae].w);
        lbDisplay.left_button = 0;
        lbDisplay.rleft_button = 0;
        if (had_screen)
            LbScreenUnlock();
        for (done = 0; !done;) {
            while (LbScreenLock() != 1)
                ;
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            reqspr_draw(resvars.f40[0xcf].x, resvars.f40[0xcf].y, &screen_sprites_tab_start[1]);
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xcf].x, resvars.f40[0xcf].y,
                    screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height))
                reqspr_draw(resvars.f40[0xcf].x, resvars.f40[0xcf].y, &screen_sprites_tab_start[2]);
            if (!--text_delay) {
                LbTextSetJustifyWindow(resvars.f3c_Boxp[0xae].x, resvars.f3c_Boxp[0xae].y, resvars.f3c_Boxp[0xae].w + 0x64);
                LbTextSetClipWindow(resvars.f3c_Boxp[0xae].x, 0, resvars.f3c_Boxp[0xae].w, lbDisplay.graphics_screen_width);
                set_font_from_cache(0x17);
                lbDisplay.draw_flags |= 0x20;
                yofs_text = 0;
                display_text = othervar.f96;
                if (display_text) {
                    if (display_text[text_ofs]) {
                        text_buf = display_text[text_ofs];
                        display_text[text_ofs] = 0;
                        LbTextDraw(0, yofs_text, display_text);
                        display_text[text_ofs] = text_buf;
                        text_ofs++;
                        text_delay = 1;
                        play_sound(10, 0xee, -1, -1);
                    }
                }
            }
            if (lbDisplay.left_button) {
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.f40[0xcf].x, resvars.f40[0xcf].y,
                        screen_sprites_tab_start[1].width, screen_sprites_tab_start[1].height)) {
                    set_mouse_mode(9);
                    clear_loading_screen(0);
                    smack_draw_callback = 0;
                    hosp_play_smk(0x18);
                    mainmenu.unknown2 = 0x1f;
                    display_mode = 2;
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
        othervar.f96[0] = 0;
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
        fn_000c93bf(dst, *strs);
}
