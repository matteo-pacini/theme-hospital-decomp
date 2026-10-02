#include "hosp.h"

struct TbIffPalette {
    u32 unknown0;
    u8 data[0x300];
};

struct BalanceFileInfo {
    BalanceFileInfo *next;
    BalanceFileInfo *prev;
    char name[10];
};

struct TbFileFind {
    char Filename[0x100];
};

struct SlistButton;

struct NetBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

#define LbTextCharHeight(c) (lbFontPtr ? lbFontPtr[(c) - 31].height : 0)
#define SLIST(n) ((SlistButton *)&resvars.screen_buttons[n])

extern Menu nickname;
extern Menu area;
extern TbIffPalette ppalette;
extern u8 BufferedInkey;
extern s32 PendingDisplay;
extern s32 CurrentDisplay;
extern s32 DesiredDisplay;
extern BalanceFileInfo *current_balance_file;

void calculate_grey(u8 *palette);
void init_zoom_boxes(s32 mask);
s32 next_map(void);
s8 draw_zoom_boxes(void);
long keybuffer_get(void);
u8 what_key_is_it_really(u8 key);
u8 load_player_details(void);
u8 setup_balance_files(u8 net_game);
void free_balance_files(void);
void next_balance_file(void);
void prev_balance_file(void);
void display_nick_name_screen(void);
void draw_new_prefs_screen(void);
s16 character_height(void);
s16 button_index(s32 mask);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_balance_file_explanation(void);
void balance_filename(char *name);
void draw_glass_text(SlistButton *button, char *text);
void draw_glass_text_hilight(SlistButton *button, char *text);
char *read_line_from_buf(char *line, s32 max, char *src);

extern TbFileFind balance_file;
extern char balance_file_name[];
extern char *upstr, *downstr;

u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"

extern "C" {

extern u8 *palette;
extern char **misc_text;
extern s8 __IsTable[];
#pragma aux __IsTable "*"

void keybuffer_clear(void);
void delay(int ms);
int LbFileFindFirst(char *path, void *ff, u32 attr);
int LbFileFindNext(void *ff);
char *strtok(char *str, char *charset);
char *strncpy(char *dst, char *src, u32 len);
int stricmp(char *s, char *t);
}

// Runs one frame of the name-entry and preferences screens' state machine
// (nickname.state): zooms the boxes in, takes key presses, and on leaving
// loads the player's details (name screen) or goes on to the level map or
// back to the main menu (preferences, display_mode 3 or 2).
// FUNCTION: 0x0008ee50
void draw_nickname(void)
{
    s32 next;

    switch (nickname.state) {
    case 0:
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        lbInkey = 0;
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost2.dat");
        calculate_grey(ppalette.data);
        nickname.value = 0;
        BufferedInkey = 0;
        keybuffer_clear();
        nickname.state = 7;
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        myplayer.name_chars[0] = 0;
        delay(800);
        init_zoom_boxes(2);
        break;
    case 7:
        next = next_map();
        if (next == 0)
            nickname.state = 2;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        break;
    case 1:
        fade_out_menu();
        nickname.state = 0;
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        mainmenu.state = MMS_ENTER;
        display_mode = DM_MAIN_MENU;
        if (lib_strlen(myplayer.name_chars) == 0)
            lib_strcpy(myplayer.name_chars, misc_text[1]);
        sprintf(statvar.save_prefix, "save/%s.h", myplayer.name_chars);
        load_player_details();
        break;
    case 2:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        display_nick_name_screen();
        break;
    case 3:
        return_from_screen(9, 0, 1);
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        load_ingame_pic(SCR_NETWORK_PREFS, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost1.dat");
        calculate_grey(ppalette.data);
        lbDisplay.left_button = lbDisplay.rleft_button = 0;
        setup_balance_files(0);
        nickname.state = 6;
        nickname.value = 0;
        BufferedInkey = 0;
        keybuffer_clear();
        delay(800);
        init_zoom_boxes(1);
        break;
    case 6:
        next = next_map();
        if (next == 0)
            nickname.state = 5;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        break;
    case 4:
    case 8:
        nickname.value = 0;
        fade_out_menu();
        if (nickname.state == 4) {
            area.state = 0;
            display_mode = DM_AREA_SELECT;
        } else {
            mainmenu.state = MMS_ENTER;
            display_mode = DM_MAIN_MENU;
        }
        nickname.state = 0;
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        free_balance_files();
        break;
    case 5:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        draw_new_prefs_screen();
        break;
    default:
        nickname.state = 0;
        break;
    }
}

// Name entry keys: letters append to myplayer.name, upper-cased (8 at most),
// Backspace deletes, Return accepts (an empty name becomes the default) and
// Escape quits the game. On the preferences screen Escape goes back to the
// main menu.
// FUNCTION: 0x0008f188
void process_keys_nickname(void)
{
    u8 i;

    switch (nickname.state) {
    case 2:
        if (BufferedInkey) {
            switch (BufferedInkey) {
            case KC_ESCAPE:
                fade_out_menu();
                quit_game = 3;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                break;
            case KC_RETURN:
            case KC_NUMPADENTER:
                nickname.state = 1;
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                if (myplayer.name[0] == 0)
                    lib_strcpy(myplayer.name_chars, misc_text[1]);
                break;
            case KC_BACKSPACE:
            case KC_DECIMAL:
                for (i = 1; i < 9; i++) {
                    if (myplayer.name[i] == 0) {
                        if (myplayer.name[0] != 0)
                            play_sound(SNDSRC_GENERAL, SFX_DELANAME, -1, -1);
                        myplayer.name[i - 1] = 0;
                        break;
                    }
                }
                break;
            default:
                if (BufferedInkey < 0x7f) {
                    for (i = 0; i < 8; i++) {
                        if (myplayer.name[i] == 0) {
                            if (__IsTable[(u8)(lbInkeyToAsciiShift[BufferedInkey] + 1)] & 0xc0) {
                                myplayer.name[i] = toupper((u8)lbInkeyToAsciiShift[BufferedInkey]);
                                myplayer.name[i + 1] = 0;
                                play_sound(SNDSRC_GENERAL, SFX_ENTANAME, -1, -1);
                            }
                            break;
                        }
                    }
                }
                break;
            }
        }
        break;
    case 5:
        if (BufferedInkey) {
            switch (BufferedInkey) {
            case KC_ESCAPE:
                nickname.state = 8;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                break;
            case KC_S:
                break;
            case KC_H:
                break;
            case KC_RETURN:
                break;
            default:
                break;
            }
        }
        break;
    }
    BufferedInkey = 0;
}

// Preferences screen clicks: presses the button under the mouse and runs its
// callback, and draws the selected tutorial and resolution options pressed.
// FUNCTION: 0x0008f448
void process_mouse_nickname(void)
{
    u8 i;
    s16 x;
    s16 first_button;
    s16 y;
    s32 mask;
    s32 bit;

    if ((CurrentDisplay & DesiredDisplay) & 1)
        reqspr_draw((u16)resvars.screen_buttons[163].x, (u16)resvars.screen_buttons[163].y,
                    &screen_sprites_tab_start[2]);
    if (nickname.state == 5) {
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            first_button = button_index(bit);
            if (!first_button)
                continue;
            for (i = 0;; i++) {
                if (current_balance_file == current_balance_file->prev) {
                    if (first_button + i == 0xa4 || first_button + i == 0xa5)
                        continue;
                }
                if (resvars.screen_buttons[first_button + i].width == 0 ||
                    resvars.screen_buttons[first_button + i].height == 0)
                    break;
                x = resvars.screen_buttons[first_button + i].x;
                y = resvars.screen_buttons[first_button + i].y;
                if (lbDisplay.rleft_button) {
                    if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, y,
                                     resvars.screen_buttons[first_button + i].width - x + 1,
                                     resvars.screen_buttons[first_button + i].height - y + 1)) {
                        lbDisplay.rleft_button = 0;
                        if (resvars.screen_buttons[first_button + i].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                              resvars.screen_buttons[first_button + i].height - y, 1);
                        if (resvars.screen_buttons[first_button + i].callback)
                            resvars.screen_buttons[first_button + i].callback(i);
                        continue;
                    }
                }
                if (lbDisplay.mleft_button && over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, y,
                                                           resvars.screen_buttons[first_button + i].width - x + 1,
                                                           resvars.screen_buttons[first_button + i].height - y + 1)) {
                    if (resvars.screen_buttons[first_button + i].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                          resvars.screen_buttons[first_button + i].height - y, 1);
                }
                if (first_button == 0xa0) {
                    switch (i) {
                    case 0:
                        if (advisor.flags & 1)
                            draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                              resvars.screen_buttons[first_button + i].height - y, 1);
                        break;
                    case 1:
                        if (!(advisor.flags & 1))
                            draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                              resvars.screen_buttons[first_button + i].height - y, 1);
                        break;
                    case 6:
                        if (resvars.game_mode == SCREEN_MODE_1)
                            draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                              resvars.screen_buttons[first_button + i].height - y, 1);
                        break;
                    case 7:
                        if (resvars.game_mode == SCREEN_MODE_13)
                            draw_button_edges(x, y, resvars.screen_buttons[first_button + i].width - x,
                                              resvars.screen_buttons[first_button + i].height - y, 1);
                        break;
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    default:
                        break;
                    }
                }
            }
        }
    }
    lbDisplay.rleft_button = 0;
}

// Draws the name prompt with the name typed so far.
// FUNCTION: 0x0008f9d4
void display_nick_name_screen(void)
{
    char buf[100];
    s32 x;
    s32 y;

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0x23);
    sprintf(buf, "%s : %s*", nickname_text[0], myplayer.name_chars);
    y = ((0x2c - character_height()) >> 1) + 0x1a6;
    x = ((0x1cb - LbTextStringWidth(nickname_text[0]) - 0x70) >> 1) + 0x5a;
    LbTextDraw(x, y, buf);
}

// Preferences screen button `index`: 0/1 tutorial on/off (advisor.flags bit 0),
// 2 accept, 3 cancel, 4/5 previous/next balance file, 6/7 320x200 or 640x480.
// FUNCTION: 0x0008fa90
extern "C" void process_nickname_button(u8 index)
{
    switch (index) {
    case 4:
        if (current_balance_file != current_balance_file->prev)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        prev_balance_file();
        break;
    case 5:
        if (current_balance_file != current_balance_file->next)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        next_balance_file();
        break;
    case 0:
        advisor.flags |= 1;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 1:
        advisor.flags &= ~1;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 2:
        nickname.state = 4;
        play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        break;
    case 3:
        nickname.state = 8;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    case 6:
        resvars.game_mode = SCREEN_MODE_1;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    case 7:
        resvars.game_mode = SCREEN_MODE_13;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    }
}

// Does nothing.
// FUNCTION: 0x0008fc30
void draw_balance_file_explanation(void)
{
}

// Draws the preferences screen's options, highlighting the current ones, and
// the selected balance file's description.
// FUNCTION: 0x0008fc48
void draw_new_prefs_screen(void)
{
    s16 x;
    s16 y;

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (current_balance_file != current_balance_file->prev) {
        draw_glass_text(SLIST(164), upstr);
        draw_glass_text(SLIST(165), downstr);
    }
    lbDisplay.draw_flags = 0;
    draw_glass_text(SLIST(162), nickname_text[0x12]);
    if (advisor.flags & 1) {
        draw_glass_text_hilight(SLIST(160), nickname_text[0x10]);
        draw_glass_text(SLIST(161), nickname_text[0x11]);
    } else {
        draw_glass_text(SLIST(160), nickname_text[0x10]);
        draw_glass_text_hilight(SLIST(161), nickname_text[0x11]);
    }
    if (resvars.game_mode == SCREEN_MODE_1) {
        draw_glass_text_hilight(SLIST(166), nickname_text[0x10]);
        draw_glass_text(SLIST(167), nickname_text[0x11]);
    } else {
        draw_glass_text(SLIST(166), nickname_text[0x10]);
        draw_glass_text_hilight(SLIST(167), nickname_text[0x11]);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x21);
    x = 0xaf;
    y = 0x12f;
    y += (0x150 - y - character_height()) >> 1;
    LbTextDraw(x, y, nickname_text[6]);
    y = 0x155;
    y += (0x176 - y - character_height()) >> 1;
    LbTextDraw(x, y, misc_text[0x1d]);
    LbTextDraw(resvars.screen_boxes_NetBoxp[122].x +
                   (resvars.screen_boxes_NetBoxp[122].w - LbTextStringWidth(misc_text[0x1e])) / 2,
               resvars.screen_boxes_NetBoxp[122].y +
                   (resvars.screen_boxes_NetBoxp[122].h - LbTextCharHeight('B') * 2) / 2,
               misc_text[0x1e]);
    LbTextDraw(resvars.screen_boxes_NetBoxp[122].x +
                   (resvars.screen_boxes_NetBoxp[122].w - LbTextStringWidth(balance_file_name)) / 2,
               resvars.screen_boxes_NetBoxp[122].y +
                   (resvars.screen_boxes_NetBoxp[122].h - LbTextCharHeight('B') * 2) / 2 + LbTextCharHeight('B'),
               balance_file_name);
    draw_balance_file_explanation();
}

// Builds the ring of balance files (levels/*00.sam, or net00.sam when
// net_game is set, from the CD for a small install), choosing "full" if
// present, and sets balance_id and the description. Returns 1 if any were
// found.
// FUNCTION: 0x0008ffb0
u8 setup_balance_files(u8 net_game)
{
    BalanceFileInfo *entry;
    BalanceFileInfo *last;
    u8 done;
    char path[144];
    s32 handle;
    char fname[16];
    u8 len;
    u8 level;
    char num[4];

    entry = 0;
    last = 0;
    if (current_balance_file)
        free_balance_files();
    done = 0;
    if (install_type < 3) {
        if (net_game) {
            sprintf(path, "%slevels/net00.sam", cd_install_path);
            check_cd_available(path);
        } else {
            sprintf(path, "%slevels/*00.sam", cd_install_path);
        }
    } else if (net_game) {
        sprintf(path, "levels/net00.sam");
    } else {
        sprintf(path, "levels/*00.sam");
    }
    handle = LbFileFindFirst(path, &balance_file, 0x21);
    if (handle == -1)
        done = 1;
    while (!done) {
        lib_strcpy(fname, balance_file.Filename);
        strtok(fname, ".");
        len = lib_strlen(fname);
        strncpy(num, &fname[len] - 2, 2);
        num[2] = 0;
        level = atoi(num);
        if (net_game == 0) {
            if (toupper((s8)fname[0]) == 'N' && toupper((s8)fname[1]) == 'E' && toupper((s8)fname[2]) == 'T')
                level = 1;
        }
        if (level == 0) {
            entry = (BalanceFileInfo *)LbMemoryAlloc(sizeof(BalanceFileInfo));
            if (entry == 0)
                break;
            if (current_balance_file == 0)
                current_balance_file = entry;
            if (last == 0)
                last = entry;
            entry->prev = last;
            entry->next = current_balance_file;
            entry->prev->next = entry;
            current_balance_file->prev = entry;
            strncpy(entry->name, fname, len - 2);
            entry->name[len - 2] = 0;
            othervar.balance_file_count++;
            last = entry;
        }
        handle = LbFileFindNext(&balance_file);
        if (handle == -1)
            done = 1;
    }
    if (othervar.balance_file_count != 0) {
        if (stricmp(current_balance_file->name, "full") != 0) {
            entry = current_balance_file;
            current_balance_file = entry->next;
            while (current_balance_file != entry) {
                if (stricmp(current_balance_file->name, "full") == 0)
                    break;
                current_balance_file = current_balance_file->next;
            }
        }
        balance_filename(current_balance_file->name);
        lib_strcpy(balance_id, current_balance_file->name);
        return 1;
    } else {
        lib_strcpy(balance_file_name, nickname_text[0x15]);
        return 0;
    }
}

// Unlinks and frees current_balance_file and clears the list; only that one
// entry is freed, since the loop never moves to the next.
// FUNCTION: 0x000902d0
void free_balance_files(void)
{
    BalanceFileInfo *next;

    next = 0;
    while (current_balance_file) {
        if (current_balance_file->prev)
            current_balance_file->prev->next = current_balance_file->next;
        if (current_balance_file->next)
            current_balance_file->next->prev = current_balance_file->prev;
        if (next == current_balance_file)
            next = 0;
        LbMemoryFree(current_balance_file);
        current_balance_file = next;
    }
    othervar.balance_file_count = 0;
}

// Selects the next balance file in the ring and loads its description.
// FUNCTION: 0x00090360
void next_balance_file(void)
{
    if (current_balance_file) {
        current_balance_file = current_balance_file->next;
        lib_strcpy(balance_id, current_balance_file->name);
        balance_filename(balance_id);
    }
}

// Selects the previous balance file in the ring and loads its description.
// FUNCTION: 0x000903a8
void prev_balance_file(void)
{
    if (current_balance_file) {
        current_balance_file = current_balance_file->prev;
        lib_strcpy(balance_id, current_balance_file->name);
        balance_filename(balance_id);
    }
}

// Loads the description of balance file `name` (levels/<name>00.sam) into
// balance_file_name: the "&<n>" line for language myplayer.language, else the
// first line, or the "not found" text.
// FUNCTION: 0x000903f0
void balance_filename(char *name)
{
    s32 i;
    s32 found;
    s32 len;
    char *buf;
    char path[144];
    char line[36];

    len = 0;
    buf = 0;
    if (install_type < 3) {
        sprintf(path, "%slevels\\%s00.sam", cd_install_path, name);
        check_cd_available(path);
    } else {
        sprintf(path, "levels\\%s00.sam", name);
    }
    len = LbFileLengthRnc(path);
    if (len == -1) {
        lib_strcpy(balance_file_name, nickname_text[0x15]);
        return;
    }
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0) {
        lib_strcpy(balance_file_name, nickname_text[0x15]);
        return;
    }
    if (LbFileLoadAt(path, buf) == -1) {
        lib_strcpy(balance_file_name, nickname_text[0x15]);
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    read_line_from_buf(0, 0, buf);
    i = found = 0;
    read_line_from_buf(line, 0x20, buf);
    if (line[0] == '&' && (s8)line[1] - '0' == myplayer.language)
        found = 1;
    else
        sprintf(balance_file_name, "%s", line);
    while (!found && i < 5) {
        read_line_from_buf(line, 0x20, buf);
        if (line[0] == '&') {
            if ((s8)line[1] - '0' == myplayer.language)
                found = 1;
        }
        i++;
    }
    if (found)
        sprintf(balance_file_name, "%s", &line[2]);
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// Does nothing.
// FUNCTION: 0x000905f4
extern "C" void fn_000905f4(void)
{
}
