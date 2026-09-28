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

struct Settings {
    u8 f0;
    u8 f1;
    u16 f2;
};

#define LbTextCharHeight(c) (lbFontPtr ? lbFontPtr[(c) - 31].height : 0)
#define SLIST(n) ((SlistButton *)&D_0015f43c[n])

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
u8 setup_balance_files(u8 a);
void free_balance_files(void);
void next_balance_file(void);
void prev_balance_file(void);
void display_nick_name_screen(void);
void draw_new_prefs_screen(void);
s16 character_height(void);
s16 button_index(s32 id);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_balance_file_explanation(void);
void balance_filename(char *name);
void draw_glass_text(SlistButton *b, char *text);
void draw_glass_text_hilight(SlistButton *b, char *text);
char *read_line_from_buf(char *line, s32 max, char *src);
void init_mcga_coords(void);
void init_midi(void);
void init_andys_variables(void);
void setup_req4_references(void);
void setup_help_indices(void);
void setup_aniseq_indices(void);
void setup_indices_for_composite_help(char **strs, u16 *idx, u32 n);
void init_wibble(void *xoff, void *yoff, s32 w, s32 h, s32 width, s32 freq, s32 amp);

extern TbFileFind balance_file;
extern char balance_file_name[];

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

extern "C" {

extern u8 D_00188c7b, D_00188c81, D_00188c7e;
extern u16 D_00188c74;
extern s32 D_00188c38;
extern volatile s32 D_00188c6c, D_00188c70;
extern u8 *D_00153f34;
extern char **D_001602a0;
extern char D_00120898[], D_001208a9[], D_001208bb[], D_001208c5[], D_001208d6[], D_001208e8[];
extern Button *D_0015f43c;
extern u8 D_0015831b;
extern TbScreenMode D_0015f3f5;
extern char *D_00138020, *D_00138024;
extern NetBox *D_0015f430;
extern u8 D_00155446;
extern u8 D_001547e5;
extern char D_001208f1[], D_00120904[], D_00120915[], D_00120926[], D_00120935[], D_00120937[], D_0012093c[];
extern char D_00120941[], D_00120953[], D_00120963[], D_00120966[];
extern u8 D_001581a6;
extern Settings D_001547e0;
extern char **D_0016037c, **D_00160374, **D_00160320, **D_00160398, **D_0016039c;
extern u16 D_0015632f[], D_0015636d[], D_00156393[], D_001563ab[], D_001563bf[];
extern u8 D_00156c04[], D_00157624[];
extern u8 *D_001553dc;
extern u8 D_0015f99f;
extern Event D_001549d0[];
extern s8 __IsTable[];
#pragma aux __IsTable "*"

void fn_00074cd8(void);
void delay(int ms);
int LbFileFindFirst(char *spec, void *find, u32 attributes);
int LbFileFindNext(void *find);
char *strtok(char *s, char *delim);
char *strncpy(char *dst, char *src, u32 n);
int stricmp(char *a, char *b);
void fn_00054d60(void);

}

// FUNCTION: 0x0008ee50
void draw_nickname(void)
{
    s32 next;

    switch (nickname.unknown2) {
    case 0:
        D_00188c7b = D_00188c81 = 0;
        lbInkey = 0;
        LbGhostTableGenerate(ppalette.data, 0x1e, D_00120898);
        calculate_grey(ppalette.data);
        nickname.unknown0 = 0;
        BufferedInkey = 0;
        fn_00074cd8();
        nickname.unknown2 = 7;
        D_00188c7b = D_00188c81 = 0;
        myplayer.unknown0[0] = 0;
        delay(800);
        init_zoom_boxes(2);
        break;
    case 7:
        next = next_map();
        if (next == 0)
            nickname.unknown2 = 2;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        break;
    case 1:
        fade_out_menu();
        nickname.unknown2 = 0;
        LbGhostTableGenerate(D_00153f34, 0x42, D_001208a9);
        mainmenu.unknown2 = 0;
        display_mode = 2;
        if (fn_000cd716(myplayer.unknown0) == 0)
            fn_000bbff0(myplayer.unknown0, D_001602a0[1]);
        sprintf(statvar.f6c, D_001208bb, myplayer.unknown0);
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
        D_00188c7b = D_00188c81 = 0;
        load_ingame_pic(0xc, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, D_001208c5);
        calculate_grey(ppalette.data);
        D_00188c7b = D_00188c81 = 0;
        setup_balance_files(0);
        nickname.unknown2 = 6;
        nickname.unknown0 = 0;
        BufferedInkey = 0;
        fn_00074cd8();
        delay(800);
        init_zoom_boxes(1);
        break;
    case 6:
        next = next_map();
        if (next == 0)
            nickname.unknown2 = 5;
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        break;
    case 4:
    case 8:
        nickname.unknown0 = 0;
        fade_out_menu();
        if (nickname.unknown2 == 4) {
            area.unknown2 = 0;
            display_mode = 3;
        } else {
            mainmenu.unknown2 = 0;
            display_mode = 2;
        }
        nickname.unknown2 = 0;
        LbGhostTableGenerate(D_00153f34, 0x42, D_001208d6);
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
        nickname.unknown2 = 0;
        break;
    }
}

// NONMATCHING: register choice; target names myplayer-1 as mycontrol+3
// FUNCTION: 0x0008f188
void process_keys_nickname(void)
{
    u8 i;

    switch (nickname.unknown2) {
    case 2:
        if (BufferedInkey) {
            switch (BufferedInkey) {
            case 1:
                fade_out_menu();
                quit_game = 3;
                play_sound(10, 0x42, -1, -1);
                break;
            case 0x1c:
            case 0x9c:
                nickname.unknown2 = 1;
                play_sound(10, 0x41, -1, -1);
                if (myplayer.name[0] == 0)
                    fn_000bbff0(myplayer.unknown0, D_001602a0[1]);
                break;
            case 0xe:
            case 0x53:
                for (i = 1; i < 9; i++) {
                    if (myplayer.name[i] == 0) {
                        if (myplayer.name[0] != 0)
                            play_sound(10, 0x10, -1, -1);
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
                                play_sound(10, 0xc, -1, -1);
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
            case 1:
                nickname.unknown2 = 8;
                play_sound(10, 0x42, -1, -1);
                break;
            case 0x1f:
                break;
            case 0x23:
                break;
            case 0x1c:
                break;
            default:
                break;
            }
        }
        break;
    }
    BufferedInkey = 0;
}

// NONMATCHING: register choice; first reqspr_draw arguments read D_0015f43c[163] as u16
// FUNCTION: 0x0008f448
void process_mouse_nickname(void)
{
    u8 i;
    s16 x;
    s16 base;
    s16 y;
    s32 mask;
    s32 bit;

    if ((CurrentDisplay & DesiredDisplay) & 1)
        reqspr_draw((u16)D_0015f43c[163].x, (u16)D_0015f43c[163].y, &screen_sprites_tab_start[2]);
    if (nickname.unknown2 == 5) {
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            base = button_index(bit);
            if (!base)
                continue;
            for (i = 0;; i++) {
                if (current_balance_file == current_balance_file->prev && (base + i == 0xa4 || base + i == 0xa5))
                    continue;
                if (D_0015f43c[i + base].width == 0 || D_0015f43c[i + base].height == 0)
                    break;
                x = D_0015f43c[i + base].x;
                y = D_0015f43c[i + base].y;
                if (D_00188c81) {
                    if (over_hotspot(D_00188c6c, D_00188c70, x, y, D_0015f43c[i + base].width - x + 1,
                                     D_0015f43c[i + base].height - y + 1)) {
                        D_00188c81 = 0;
                        if (D_0015f43c[i + base].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                        if (D_0015f43c[i + base].callback)
                            D_0015f43c[i + base].callback(i);
                        continue;
                    }
                }
                if (D_00188c7e && over_hotspot(mouse_dx, mouse_dy, x, y, D_0015f43c[i + base].width - x + 1,
                                               D_0015f43c[i + base].height - y + 1)) {
                    if (D_0015f43c[i + base].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                }
                if (base == 0xa0) {
                    switch (i) {
                    case 0:
                        if (D_0015831b & 1)
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                        break;
                    case 1:
                        if (!(D_0015831b & 1))
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                        break;
                    case 6:
                        if (D_0015f3f5 == SCREEN_MODE_1)
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                        break;
                    case 7:
                        if (D_0015f3f5 == SCREEN_MODE_13)
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
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
    D_00188c81 = 0;
}

// FUNCTION: 0x0008f9d4
void display_nick_name_screen(void)
{
    char buf[100];
    s32 x;
    s32 y;

    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    set_font_from_cache(0x23);
    sprintf(buf, D_001208e8, nickname_text[0], myplayer.unknown0);
    y = ((0x2c - character_height()) >> 1) + 0x1a6;
    x = ((0x1cb - LbTextStringWidth(nickname_text[0]) - 0x70) >> 1) + 0x5a;
    LbTextDraw(x, y, buf);
}

// FUNCTION: 0x0008fa90
extern "C" void fn_0008fa90(u8 index)
{
    switch (index) {
    case 4:
        if (current_balance_file != current_balance_file->prev)
            play_sound(10, 0x43, -1, -1);
        prev_balance_file();
        break;
    case 5:
        if (current_balance_file != current_balance_file->next)
            play_sound(10, 0x43, -1, -1);
        next_balance_file();
        break;
    case 0:
        D_0015831b |= 1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 1:
        D_0015831b &= ~1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 2:
        nickname.unknown2 = 4;
        play_sound(10, 0x41, -1, -1);
        break;
    case 3:
        nickname.unknown2 = 8;
        play_sound(10, 0x42, -1, -1);
        break;
    case 6:
        D_0015f3f5 = SCREEN_MODE_1;
        play_sound(10, 0x43, -1, -1);
        break;
    case 7:
        D_0015f3f5 = SCREEN_MODE_13;
        play_sound(10, 0x43, -1, -1);
        break;
    }
}

// FUNCTION: 0x0008fc30
void draw_balance_file_explanation(void)
{
}

// FUNCTION: 0x0008fc48
void draw_new_prefs_screen(void)
{
    s16 x;
    s16 y;

    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
    if (current_balance_file != current_balance_file->prev) {
        draw_glass_text(SLIST(164), D_00138020);
        draw_glass_text(SLIST(165), D_00138024);
    }
    D_00188c74 = 0;
    draw_glass_text(SLIST(162), nickname_text[0x12]);
    if (D_0015831b & 1) {
        draw_glass_text_hilight(SLIST(160), nickname_text[0x10]);
        draw_glass_text(SLIST(161), nickname_text[0x11]);
    } else {
        draw_glass_text(SLIST(160), nickname_text[0x10]);
        draw_glass_text_hilight(SLIST(161), nickname_text[0x11]);
    }
    if (D_0015f3f5 == SCREEN_MODE_1) {
        draw_glass_text_hilight(SLIST(166), nickname_text[0x10]);
        draw_glass_text(SLIST(167), nickname_text[0x11]);
    } else {
        draw_glass_text(SLIST(166), nickname_text[0x10]);
        draw_glass_text_hilight(SLIST(167), nickname_text[0x11]);
    }
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0x21);
    x = 0xaf;
    y = 0x12f;
    y += (0x150 - y - character_height()) >> 1;
    LbTextDraw(x, y, nickname_text[6]);
    y = 0x155;
    y += (0x176 - y - character_height()) >> 1;
    LbTextDraw(x, y, D_001602a0[0x1d]);
    LbTextDraw(D_0015f430[122].x + (D_0015f430[122].w - LbTextStringWidth(D_001602a0[0x1e])) / 2,
               D_0015f430[122].y + (D_0015f430[122].h - LbTextCharHeight('B') * 2) / 2, D_001602a0[0x1e]);
    LbTextDraw(D_0015f430[122].x + (D_0015f430[122].w - LbTextStringWidth(balance_file_name)) / 2,
               D_0015f430[122].y + (D_0015f430[122].h - LbTextCharHeight('B') * 2) / 2 + LbTextCharHeight('B'),
               balance_file_name);
    draw_balance_file_explanation();
}

// NONMATCHING: frame layout (target slots follow first use, arrays interleaved with scalars)
// FUNCTION: 0x0008ffb0
u8 setup_balance_files(u8 cd)
{
    BalanceFileInfo *b;
    BalanceFileInfo *last;
    u8 done;
    char path[144];
    s32 handle;
    char fname[16];
    u8 len;
    char num[4];
    u8 value;

    b = 0;
    last = 0;
    if (current_balance_file)
        free_balance_files();
    done = 0;
    if (install_type < 3) {
        if (cd) {
            sprintf(path, D_001208f1, cd_install_path);
            check_cd_available(path);
        } else {
            sprintf(path, D_00120904, cd_install_path);
        }
    } else if (cd) {
        sprintf(path, D_00120915);
    } else {
        sprintf(path, D_00120926);
    }
    handle = LbFileFindFirst(path, &balance_file, 0x21);
    if (handle == -1)
        done = 1;
    while (!done) {
        fn_000bbff0(fname, balance_file.Filename);
        strtok(fname, D_00120935);
        len = fn_000cd716(fname);
        strncpy(num, &fname[len] - 2, 2);
        num[2] = 0;
        value = atoi(num);
        if (cd == 0) {
            if (toupper((s8)fname[0]) == 'N' && toupper((s8)fname[1]) == 'E' && toupper((s8)fname[2]) == 'T')
                value = 1;
        }
        if (value == 0) {
            b = (BalanceFileInfo *)LbMemoryAlloc(sizeof(BalanceFileInfo));
            if (b == 0)
                break;
            if (current_balance_file == 0)
                current_balance_file = b;
            if (last == 0)
                last = b;
            b->prev = last;
            b->next = current_balance_file;
            b->prev->next = b;
            current_balance_file->prev = b;
            strncpy(b->name, fname, len - 2);
            b->name[len - 2] = 0;
            D_00155446++;
            last = b;
        }
        handle = LbFileFindNext(&balance_file);
        if (handle == -1)
            done = 1;
    }
    if (D_00155446 != 0) {
        if (stricmp(current_balance_file->name, D_00120937) != 0) {
            b = current_balance_file;
            current_balance_file = b->next;
            while (current_balance_file != b && stricmp(current_balance_file->name, D_0012093c) != 0)
                current_balance_file = current_balance_file->next;
        }
        balance_filename(current_balance_file->name);
        fn_000bbff0(balance_id, current_balance_file->name);
        return 1;
    } else {
        fn_000bbff0(balance_file_name, nickname_text[0x15]);
        return 0;
    }
}

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
    D_00155446 = 0;
}

// FUNCTION: 0x00090360
void next_balance_file(void)
{
    if (current_balance_file) {
        current_balance_file = current_balance_file->next;
        fn_000bbff0(balance_id, current_balance_file->name);
        balance_filename(balance_id);
    }
}

// FUNCTION: 0x000903a8
void prev_balance_file(void)
{
    if (current_balance_file) {
        current_balance_file = current_balance_file->prev;
        fn_000bbff0(balance_id, current_balance_file->name);
        balance_filename(balance_id);
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000903f0
void balance_filename(char *name)
{
    s32 found;
    s32 i;
    char *buf;
    s32 len;
    char path[144];
    char line[36];

    len = 0;
    buf = 0;
    if (install_type < 3) {
        sprintf(path, D_00120941, cd_install_path, name);
        check_cd_available(path);
    } else {
        sprintf(path, D_00120953, name);
    }
    len = LbFileLengthRnc(path);
    if (len == -1) {
        fn_000bbff0(balance_file_name, nickname_text[0x15]);
        return;
    }
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0) {
        fn_000bbff0(balance_file_name, nickname_text[0x15]);
        return;
    }
    if (LbFileLoadAt(path, buf) == -1) {
        fn_000bbff0(balance_file_name, nickname_text[0x15]);
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    read_line_from_buf(0, 0, buf);
    i = found = 0;
    read_line_from_buf(line, 0x20, buf);
    if (line[0] == '&' && (s8)line[1] - '0' == D_001547e5)
        found = 1;
    else
        sprintf(balance_file_name, D_00120963, line);
    for (; !found && i < 5; i++) {
        read_line_from_buf(line, 0x20, buf);
        if (line[0] == '&') {
            if ((s8)line[1] - '0' == D_001547e5)
                found = 1;
        }
    }
    if (found)
        sprintf(balance_file_name, D_00120966, &line[2]);
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// FUNCTION: 0x000905f4
extern "C" void fn_000905f4(void)
{
}

// FUNCTION: 0x00090610
extern "C" void fn_00090610(void)
{
    init_mcga_coords();
    set_font_from_cache(0);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    init_andys_variables();
    setup_req4_references();
    crequester.unknown2 = D_001581a6 = 0x63;
    D_001547e0.f1 = 1;
    setup_menus();
    fn_00054d60();
    setup_help_indices();
    setup_aniseq_indices();
    setup_indices_for_composite_help(D_0016037c, D_0015632f, 0x1f);
    setup_indices_for_composite_help(D_00160374, D_0015636d, 0x13);
    setup_indices_for_composite_help(D_00160320, D_00156393, 0xd);
    setup_indices_for_composite_help(D_00160398, D_001563ab, 0xb);
    setup_indices_for_composite_help(D_0016039c, D_001563bf, 4);
    InitPixBuffer(0, 0, 0xb4, 0xb4, 0xfe, 1);
    init_midi();
    init_ambients();
    gamecounts.unknown14 = LbTimerClock();
    D_001547e0.f2 = 0x19;
    init_wibble(D_00156c04, D_00157624, pixBuf.width + 0x20, pixBuf.width + 0x20, pixBuf.width, 0x11, 2);
    measure_objects();
    choice_backup.mode = 0;
    current_text.text[0] = 0;
    if (debug.unknown1fa) {
        if (fn_000cd716(myplayer.unknown0) == 0)
            fn_000bbff0(myplayer.unknown0, D_001602a0[1]);
        fn_00090804((u16)level_number, 1);
        load_player_details();
        D_001553dc = D_00153f34;
        if (!D_0015f99f) {
            D_001549d0[player].type = 0x1e;
            D_001549d0[player].value = debug.unknown10a;
        } else {
            D_001549d0[player].type = 0x1d;
        }
    } else {
        D_001549d0[player].type = 0x1d;
    }
}
