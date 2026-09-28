#include "hosp.h"

struct TbIffPalette {
    u32 unknown0;
    u8 data[0x300];
};

struct NetBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct GlassBox {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
    u16 f8;
    u8 unknownA[4];
    u8 fe;
    u8 ff;
    u8 f10;
    u8 unknown11;
    u8 f12;
    u8 unknown13;
};

struct GlassFrame {
    u8 unknown0[0x24d];
};

struct NetPlayer {
    u8 unknown0[0x21];
    char name[0x14];
};

struct NetControl {
    u8 unknown0;
    u8 unknown1[3];
    s8 f4;
    NetPlayer players[6];
};

struct LoginPlayer {
    u16 len;
    char text[0x28];
};

struct LoginControl {
    u8 unknown0[0x17];
    u8 f17;
    u8 unknown18[7];
    u8 f1f;
    u8 unknown20[2];
    LoginPlayer players[4];
};

struct SlistButton {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 unknown8[10];
};

struct BalanceFileInfo {
    u32 unknown0;
    BalanceFileInfo *next;
};

struct ScreenPacket {
    u8 unknown0[0x38];
};

extern "C" {
extern s32 D_00188c38;
extern u16 D_00188c74;
extern u8 D_00188c7b;
extern u8 D_00188c7e;
extern u8 D_00188c81;
extern u8 D_00188c85;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern Event D_001549d0[];
extern NetBox *D_0015f430;
extern Button *D_0015f43c;
extern GlassFrame *D_0015f444;
extern u8 *D_00153f34;
extern u8 *D_001553dc;
extern u8 D_0015481a[];
extern u8 D_0015f1aa;
extern u8 D_0015f1ab;
extern char D_001206b0[];
extern char D_001206ba[];
extern char D_001206c4[];
extern char D_001206d5[];
extern char D_001206e7[];
extern char D_001206ed[];
extern char D_001206ef[];
extern char D_001206f1[];
extern char D_001206f3[];
extern char D_001206f5[];
extern char D_001206f7[];
extern char D_001206f9[];
extern char D_001206fb[];
extern char D_001206fd[];
extern char D_001206ff[];
extern char D_00120701[];
extern char D_00120703[];
extern char *D_00138020;
extern char *D_00138024;
extern s32 D_0013804c;
extern char **D_001602a0;
extern char **D_00160308;
extern u32 D_001606bc;
extern u8 D_001606c8;
extern u8 D_001606c9;
extern u8 D_001606cc;
extern s8 D_001605f1;
extern char D_00160810[];
void fn_000817c4(void);
void fn_0008172c(void);
void fn_00081284(u8 flag);
void fn_00081408(u8 flag);
void fn_000841b8(u8 a);
void fn_00081588(void);
#pragma aux fn_00081588 "W?draw_available_bauds$n(uc)v"
extern char *D_00138028[];
extern char D_00120705[];
extern char D_00120708[];

char *strchr(char *s, int c);
void fn_00074cd8(void);
void fn_000110b0(void);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "W?GetCurrentPlayersCallback$n(pn$TbNetworkCallbackData$$pnv)v"
void fn_0007fbb8(s8 me);
}

extern Menu network;
extern s32 DesiredDisplay;
extern s32 CurrentDisplay;
extern s32 PendingDisplay;
extern NetControl net_control;
extern LoginControl login_control;
extern TbIffPalette ppalette;
extern u8 BufferedInkey;
extern ScreenPacket screen_packet[];
extern char init_str[];
extern char dial_str[];
extern char hangup_str[];
extern char auto_ans_str[];
extern s8 BookCount;
extern s8 BookHilight;
extern u8 TopIRQ;
extern u8 TopMessage;
extern u8 NumMessages;
extern char *MessageText;
extern s32 player_count;
extern char balance_file_name[];
extern BalanceFileInfo *current_balance_file;
extern char PhoneBook[][13];
extern char PhoneNumber[];
extern s32 RestoreDisplay;

char *read_line_from_buf(char *line, s32 max, char *src);
s8 add_glass_box(GlassBox *box, s32 id, s8 a, s8 b);
void add_glass_frame(GlassFrame *frame, s32 id);
s16 glass_index(s32 id);
void network_read_control(u8 n);
void set_glass_map(s32 mask);
void reset_glass_map(s32 mask);
void display_button_text(s32 mask);
void calculate_grey(u8 *palette);
void init_zoom_boxes(s32 mask);
u8 setup_balance_files(u8 a);
s32 next_map(void);
s8 draw_zoom_boxes(void);
void free_balance_files(void);
u8 start_session(void);
void swap_level_number(void);
void init_network_players(void);
long keybuffer_get(void);
u8 what_key_is_it_really(u8 key);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_glass_text(SlistButton *b, char *text);
void draw_glass_text_hilight(SlistButton *b, char *text);
void draw_glass_text_hilight_no_jump(SlistButton *b, char *text);
void draw_network_game_selection_screen(void);
void draw_current_message(char *text);
void draw_message_box(void);
void draw_level_selection_screen(void);
void draw_report_selection_screen(void);
void draw_small_centre_text(s16 x, s16 w, s16 y, char *text);
void draw_small_highlight_centre_text(s16 x, s16 w, s16 y, char *text);
s16 character_height(void);
void process_login_exchange(void);
void draw_phone_numbers(void);
void network_button_quit(void);
u8 send_message(void);
s16 button_index(s32 id);
void add_number(char c);
void add_phone_number(char *number);

#define SLIST(n) ((SlistButton *)&D_0015f43c[n])

// FUNCTION: 0x0007f9f0
void read_modem_ini_file(void)
{
    char *p;
    char *buf;
    s32 len;
    char line[128];

    len = 0;
    buf = 0;
    len = LbFileLengthRnc(D_001206b0);
    if (len == -1)
        return;
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0)
        return;
    if (LbFileLoadAt(D_001206ba, buf) == -1) {
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    read_line_from_buf(0, 0, buf);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        LbDataLoadSetModifyFilenameFunction(init_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        LbDataLoadSetModifyFilenameFunction(dial_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        LbDataLoadSetModifyFilenameFunction(hangup_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        LbDataLoadSetModifyFilenameFunction(auto_ans_str, p);
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// FUNCTION: 0x0007fbb8
void fn_0007fbb8(s8 me)
{
    s32 i;

    modem_dev.unknown1c = 0;
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active != 0 && me != i) {
            modem_dev.unknown1c = i;
            break;
        }
    }
}

// NONMATCHING: 16-bit multiply, register choice
// FUNCTION: 0x0007fc14
void set_glass_map(s32 mask)
{
    s16 idx;
    s16 i;
    s32 bit;
    GlassBox box;

    while (mask) {
        bit = mask & -mask;
        mask ^= bit;
        switch (bit) {
        case 0x10:
        case 0x20:
        case 0x40:
            DesiredDisplay &= 0x84;
        }
        if (bit == 0x1000) {
            DesiredDisplay &= 0x1000;
            network_read_control(network_num_players());
            set_font_from_cache(0x22);
            box.x = D_0015f430[126].x;
            box.h = (s16)D_0015f430[126].h * net_control.f4 + 0x17;
            box.y = (D_00188c38 - box.h) / 2;
            box.h += box.y;
            box.w = box.x + D_0015f430[126].w;
            box.f8 = 0x27;
            box.fe = 0;
            box.ff = 0;
            box.f10 = 7;
            box.f12 = 1;
            add_glass_box(&box, 0x1000, 0, 0);
            box.x += D_0015f430[126].y;
            box.y += 0xd;
            box.h = box.y + D_0015f430[126].h - 3;
            box.w = box.x + D_0015f430[126].w - D_0015f430[126].y * 2;
            box.fe = 1;
            box.ff = 1;
            box.f12 = 0;
            for (i = 0; i < net_control.f4; i++) {
                D_0015f43c[i + 0x8c].x = box.x;
                D_0015f43c[i + 0x8c].y = box.y;
                D_0015f43c[i + 0x8c].width = box.w;
                D_0015f43c[i + 0x8c].height = box.h;
                add_glass_box(&box, 0x1000, i * 3, i * 3);
                box.y += D_0015f430[126].h;
                box.h += D_0015f430[126].h;
            }
            D_0015f43c[i + 0x8c].x = 0;
            D_0015f43c[i + 0x8c].y = 0;
            D_0015f43c[i + 0x8c].width = 0;
            D_0015f43c[i + 0x8c].height = 0;
            DesiredDisplay |= bit;
        }
        idx = glass_index(bit);
        if (idx) {
            if (!(DesiredDisplay & bit))
                add_glass_frame(&D_0015f444[idx], bit);
            DesiredDisplay |= bit;
        }
    }
    network.unknown2 = 3;
}

// FUNCTION: 0x0007ff24
void reset_glass_map(s32 mask)
{
    DesiredDisplay &= ~mask;
    network.unknown2 = 3;
}

// FUNCTION: 0x0007ff50
void toggle_glass_map(s32 mask)
{
    if (CurrentDisplay & mask)
        reset_glass_map(mask);
    else
        set_glass_map(mask);
}

// FUNCTION: 0x0007ff88
void draw_network_select(void)
{
    s32 next;

    switch (network.unknown2) {
    case 0:
        read_modem_ini_file();
        statvar.unknownfd = 1;
        player = 0;
        return_from_screen(9, 0, 1);
        load_ingame_pic(0xc, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, D_001206c4);
        calculate_grey(ppalette.data);
        fn_00074cd8();
        BufferedInkey = 0;
        login_control.f17 = 6;
        D_00188c85 = color.unknownA;
        D_0015f1aa = 0;
        init_zoom_boxes(0x84);
        setup_balance_files(1);
        network.unknown2 = 3;
        if (D_0015481a[0] == 0) {
            D_0015481a[0] = 4;
            D_0015481a[1] = 3;
            D_0015481a[2] = 4;
            D_0015481a[3] = 3;
            D_0015481a[4] = 2;
            D_0015481a[5] = 2;
        }
        break;
    case 3:
        next = next_map();
        if (next == 0) {
            network.unknown2 = 2;
            if ((CurrentDisplay | DesiredDisplay) == 0) {
                statvar.unknownfd = 1;
                player = 0;
                network.unknown2 = 0;
                display_mode = 2;
                mainmenu.unknown2 = 0;
            }
            break;
        }
        PendingDisplay = next;
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        display_button_text(~next & CurrentDisplay);
        break;
    case 1:
        LbScreenClear(0);
        display_loading_screen();
        free_balance_files();
        start_session();
        network.unknown2 = 0;
        swap_level_number();
        LbGhostTableGenerate(D_00153f34, 0x42, D_001206d5);
        fn_000110b0();
        LbPackHeap();
        load_balance((u16)level_number);
        LbPackHeap();
        load_game_files();
        debug.thing_count = 0;
        fn_00090804((u16)level_number, 0);
        init_network_players();
        memset(packet, 0, 0x3c);
        memset(screen_packet, 0, 0x38);
        quit_game = 2;
        display_mode = 0;
        debug.f0 = 0;
        set_font_from_cache(0);
        D_00188c81 = D_00188c7b = 0;
        login_control.f17 = 6;
        D_0015f1ab = 0;
        clear_loading_screen(0);
        D_001553dc = D_00153f34;
        current_palette = D_00153f34;
        set_mouse_mode(0);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        break;
    case 2:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        display_button_text(CurrentDisplay);
        if (login_control.f17 == 5) {
            if (modem_dev.unknown1c == player && !(CurrentDisplay & 0x100))
                set_glass_map(0x100);
        }
        break;
    default:
        network.unknown2 = 0;
        break;
    }
}

// NONMATCHING: register choice, buffer frame slot
// FUNCTION: 0x000802c4
void display_button_text(s32 mask)
{
    s32 bit;

    while (mask) {
        bit = mask & -mask;
        mask ^= bit;
        D_00188c74 = 0;
        switch (bit) {
            s16 i;

        case 0x80:
            if (DesiredDisplay & 0x40) {
                draw_button_edges(SLIST(89)->x, SLIST(89)->y, SLIST(89)->x1 - SLIST(89)->x,
                                  SLIST(89)->y1 - SLIST(89)->y, 1);
                draw_glass_text_hilight(SLIST(89), D_00160308[7]);
            } else {
                draw_glass_text(SLIST(89), D_00160308[7]);
            }
            if (DesiredDisplay & 0x20) {
                draw_button_edges(SLIST(90)->x, SLIST(90)->y, SLIST(90)->x1 - SLIST(90)->x,
                                  SLIST(90)->y1 - SLIST(90)->y, 1);
                draw_glass_text_hilight(SLIST(90), D_00160308[8]);
            } else {
                draw_glass_text(SLIST(90), D_00160308[8]);
            }
            if (DesiredDisplay & 0x10) {
                draw_button_edges(SLIST(91)->x, SLIST(91)->y, SLIST(91)->x1 - SLIST(91)->x,
                                  SLIST(91)->y1 - SLIST(91)->y, 1);
                draw_glass_text_hilight(SLIST(91), D_00160308[9]);
            } else {
                draw_glass_text(SLIST(91), D_00160308[9]);
            }
            break;
        case 0x10:
            D_00188c74 = 0;
            if (D_001606c8 == 0)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(106), D_00138020);
            D_00188c74 = 0;
            if (D_001606c8 >= 5)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(107), D_00138024);
            if (login_control.f17 == 5) {
                D_00188c74 = 0;
                draw_glass_text(SLIST(104), D_00160308[11]);
                if (modem_dev.unknown1c != player) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(105), D_00160308[13]);
                } else if (player_count <= 1) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(105), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(105), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(105), D_00160308[3]);
                }
                D_00188c74 |= 4;
                draw_glass_text(SLIST(103), D_00160308[12]);
            } else {
                D_00188c74 = 0;
                if (!BookCount && (D_001606bc & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(103), D_00160308[12]);
                else
                    draw_glass_text(SLIST(103), D_00160308[12]);
                if ((D_001606bc & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(105), D_00160308[13]);
                else
                    draw_glass_text(SLIST(105), D_00160308[13]);
                if (BookHilight == -1) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(104), D_00160308[10]);
                } else if ((D_001606bc & 7) < 4 && !(DesiredDisplay & 0x400)) {
                    draw_glass_text_hilight_no_jump(SLIST(104), D_00160308[10]);
                } else {
                    draw_glass_text(SLIST(104), D_00160308[10]);
                }
            }
            D_00188c74 = 0;
            if (login_control.f17 != 5) {
                if (!D_001605f1)
                    D_00188c74 |= 4;
            } else {
                if (login_control.f1f == 0)
                    D_00188c74 |= 4;
            }
            draw_glass_text(SLIST(101), D_00138020);
            D_00188c74 = 0;
            if (login_control.f17 != 5) {
                if (D_001605f1 >= BookCount - 3)
                    D_00188c74 |= 4;
            } else {
                if (login_control.f1f >= player_count - 3)
                    D_00188c74 |= 4;
            }
            draw_glass_text(SLIST(102), D_00138024);
            D_00188c74 = 0;
            fn_000817c4();
            break;
        case 0x20:
            D_00188c74 = 0;
            if (login_control.f17 != 5) {
                if (D_001606c8 == 0)
                    D_00188c74 |= 4;
            } else {
                if (login_control.f1f == 0)
                    D_00188c74 |= 4;
            }
            draw_glass_text(SLIST(109), D_00138020);
            D_00188c74 = 0;
            if (login_control.f17 != 5) {
                if (D_001606c8 >= 5)
                    D_00188c74 |= 4;
            } else {
                if (login_control.f1f >= player_count - 3)
                    D_00188c74 |= 4;
            }
            draw_glass_text(SLIST(110), D_00138024);
            D_00188c74 = 0;
            if (!D_001606c9)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(113), D_00138020);
            D_00188c74 = 0;
            if (D_001606c9)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(114), D_00138024);
            D_00188c74 = 0;
            switch (login_control.f17) {
            case 5:
                draw_glass_text(SLIST(112), D_00160308[4]);
                if (modem_dev.unknown1c != player) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(111), D_00160308[14]);
                } else if (player_count <= 1) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(111), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(111), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(111), D_00160308[3]);
                }
                break;
            case 6:
                if (!D_0015481a[4] && (int)D_0015481a[5] < 0)
                    D_00188c74 |= 4;
                if ((D_001606bc & 7) > 3)
                    draw_glass_text_hilight_no_jump(SLIST(111), D_00160308[2]);
                else
                    draw_glass_text(SLIST(111), D_00160308[2]);
                if ((D_001606bc & 7) < 4)
                    draw_glass_text_hilight_no_jump(SLIST(112), D_00160308[1]);
                else
                    draw_glass_text(SLIST(112), D_00160308[1]);
                break;
            }
            D_00188c74 = 0;
            fn_0008172c();
            break;
        case 0x40:
            D_00188c74 = 0;
            if (login_control.f1f == 0)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(120), D_00138020);
            D_00188c74 = 0;
            if (login_control.f1f >= player_count - 3)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(121), D_00138024);
            D_00188c74 = 0;
            if (D_001606cc == 0 || login_control.f17 == 5)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(118), D_00138020);
            D_00188c74 = 0;
            if (D_001606cc >= D_0015f1aa - 3 || login_control.f17 == 5)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(119), D_00138024);
            D_00188c74 = 0;
            switch (login_control.f17) {
            case 5:
                draw_glass_text(SLIST(116), D_00160308[4]);
                break;
            case 6:
                if ((D_001606bc & 7) < 4)
                    draw_glass_text_hilight_no_jump(SLIST(116), D_00160308[1]);
                else
                    draw_glass_text(SLIST(116), D_00160308[1]);
                break;
            }
            switch (login_control.f17) {
            case 5:
                if (modem_dev.unknown1c != player) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(117), D_00160308[2]);
                } else if (player_count <= 1) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(117), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(117), D_00160308[3]);
                }
                break;
            case 6:
                if (D_0013804c == -1) {
                    D_00188c74 |= 4;
                    draw_glass_text(SLIST(117), D_00160308[2]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), D_00160308[2]);
                } else {
                    draw_glass_text(SLIST(117), D_00160308[2]);
                }
                break;
            }
            D_00188c74 = 0;
            draw_network_game_selection_screen();
            break;
        case 0x100: {
            char buf[128];

            sprintf(buf, D_001206e7, D_001602a0[30], balance_file_name);
            D_00188c74 = 0;
            if (current_balance_file != current_balance_file->next) {
                draw_glass_text(SLIST(94), D_00138020);
                draw_glass_text(SLIST(95), D_00138024);
            }
            LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
            set_font_from_cache(0x21);
            LbTextDraw((D_0015f430[132].w - LbTextStringWidth(buf)) / 2 + D_0015f430[132].x,
                       D_0015f430[132].y + (D_0015f430[132].h - (lbFontPtr ? lbFontPtr[66].height : 0)) / 2, buf);
            break;
        }
        case 0x200:
            draw_current_message(login_control.players[player].text);
            draw_message_box();
            draw_glass_text(SLIST(97), D_00160308[6]);
            D_00188c74 = 0;
            if (TopMessage == 0)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(98), D_00138020);
            D_00188c74 = 0;
            if (TopMessage >= NumMessages - 3)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(99), D_00138024);
            break;
        case 0x400:
            draw_glass_text(SLIST(128), D_001206ed);
            draw_glass_text(SLIST(129), D_001206ef);
            draw_glass_text(SLIST(130), D_001206f1);
            draw_glass_text(SLIST(131), D_001206f3);
            draw_glass_text(SLIST(132), D_001206f5);
            draw_glass_text(SLIST(133), D_001206f7);
            draw_glass_text(SLIST(134), D_001206f9);
            draw_glass_text(SLIST(135), D_001206fb);
            draw_glass_text(SLIST(136), D_001206fd);
            draw_glass_text(SLIST(137), D_001206ff);
            if ((D_001606bc & 7) > 3)
                draw_glass_text_hilight_no_jump(SLIST(138), D_00120701);
            else
                draw_glass_text(SLIST(138), D_00120703);
            D_00188c74 = 0;
            LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
            break;
        case 0x1000:
            for (i = 0; i < net_control.f4; i++)
                draw_glass_text(SLIST(i + 0x8c), net_control.players[i].name);
            draw_level_selection_screen();
            break;
        case 8:
            draw_glass_text(SLIST(147), MessageText);
            draw_report_selection_screen();
            break;
        case 0x800:
            draw_small_centre_text(0x1b, 0x24a, 0x113, D_00160810);
            draw_glass_text(SLIST(126), nickname_text[19]);
            break;
        case 0x8000:
        case 0x10000:
            D_00188c74 = 0;
            if (TopIRQ == 0)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(123), D_00138020);
            D_00188c74 = 0;
            if (TopIRQ >= 4)
                D_00188c74 |= 4;
            draw_glass_text(SLIST(124), D_00138024);
            D_00188c74 = 0;
            break;
        }
    }
}

// FUNCTION: 0x00081284
void fn_00081284(u8 flag)
{
    s32 idx;
    s16 y;
    s16 i;
    char buf[52];

    if (flag)
        idx = 0x82;
    else
        idx = 0x80;
    y = D_0015f430[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, D_00160308[15], i + 1 + D_001606c9);
        if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, (s16)D_0015f430[idx].x, y, (s16)D_0015f430[idx].w,
                         character_height())) {
            if (D_00188c7e) {
                D_0015481a[4] = 1 << (i + D_001606c9);
                D_00188c7b = 0;
            }
        }
        if (((s8)D_0015481a[4] >> (i + D_001606c9)) == 1)
            draw_small_highlight_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
        else
            draw_small_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
    }
}

// FUNCTION: 0x00081408
void fn_00081408(u8 flag)
{
    s32 idx;
    s16 y;
    s16 i;
    char buf[52];

    if (flag)
        idx = 0x83;
    else
        idx = 0x81;
    y = D_0015f430[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, D_00120705, D_00138028[i + D_001606c8]);
        if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, (s16)D_0015f430[idx].x, y, (s16)D_0015f430[idx].w,
                         character_height())) {
            if (D_00188c7e) {
                D_0015481a[5] = D_001606c8 + i;
                D_00188c7b = 0;
            }
        }
        if (D_0015481a[5] == D_001606c8 + i)
            draw_small_highlight_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
        else
            draw_small_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
    }
}

// FUNCTION: 0x00081588
void fn_00081588(void)
{
    s16 y;
    s16 i;
    s32 bits;
    s32 idx;
    s32 n;
    char buf[52];

    if (CurrentDisplay & 0x18000) {
        n = 0;
        bits = (s8)D_0015481a[4] >> 1;
        while (bits != 0) {
            n++;
            bits >>= 1;
        }
        idx = 0x85;
        y = D_0015f430[idx].y + 10;
        for (i = 0; i < 4; i++, y += character_height()) {
            sprintf(buf, D_00120708, i + TopIRQ);
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, (s16)D_0015f430[idx].x, y, (s16)D_0015f430[idx].w,
                             character_height())) {
                if (D_00188c7e) {
                    D_0015481a[n] = TopIRQ + i;
                    D_00188c7b = 0;
                }
            }
            if (TopIRQ + i == D_0015481a[n])
                draw_small_highlight_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
            else
                draw_small_centre_text(D_0015f430[idx].x, D_0015f430[idx].w, y, buf);
        }
    }
}

// FUNCTION: 0x0008172c
void fn_0008172c(void)
{
    switch (login_control.f17) {
    case 5:
        fn_000841b8(1);
        process_login_exchange();
        break;
    case 6:
        fn_00081284(0);
        fn_00081408(0);
        fn_00081588();
        break;
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.unknownfd <= 1)
            break;
        network.unknown2 = 1;
        break;
    case 7:
    default:
        break;
    }
}

// FUNCTION: 0x000817c4
void fn_000817c4(void)
{
    switch (login_control.f17) {
    case 5:
        fn_000841b8(1);
        process_login_exchange();
        break;
    case 6:
        fn_00081284(1);
        fn_00081408(1);
        fn_00081588();
        draw_phone_numbers();
        break;
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.unknownfd <= 1)
            break;
        network.unknown2 = 1;
        break;
    case 7:
    default:
        break;
    }
}

// FUNCTION: 0x0008186c
void add_phone_number(char *number)
{
    if (*number) {
        if (BookCount < 15) {
            BookHilight = BookCount;
            LbDrawBoxCoords(PhoneBook[BookCount++], number, 12);
        } else {
            BookHilight = 0;
            LbDrawBoxCoords(PhoneBook[0], number, 12);
        }
    }
}

// FUNCTION: 0x000818e8
void add_number(char c)
{
    s8 len;

    len = fn_000cd716(PhoneNumber);
    if (len < 12) {
        if (BufferedInkey)
            play_sound(10, 0xc, -1, -1);
        PhoneNumber[len++] = c;
        PhoneNumber[len] = 0;
    }
}

// FUNCTION: 0x00081958
void process_report_buttons(u8 n)
{
    if (!(login_control.f17 == 9 || login_control.f17 == 8)) {
        set_glass_map(RestoreDisplay);
        reset_glass_map(8);
        network.unknown2 = 3;
    }
}

// FUNCTION: 0x000819a8
void report_message(char *message)
{
    GlassBox box;

    MessageText = message;
    set_font_from_cache(0x22);
    box.w = LbTextStringWidth(message) + D_0015f430[126].y * 4;
    box.x = (D_00188c34 - box.w) / 2;
    box.h = D_0015f430[126].h + D_0015f430[126].y * 2;
    box.y = (D_00188c38 - box.h) / 2;
    box.h += box.y;
    box.w += box.x;
    box.f8 = 0x27;
    box.fe = 0;
    box.ff = 0;
    box.f10 = 10;
    box.f12 = 1;
    add_glass_box(&box, 8, 0, 0);
    box.x += D_0015f430[126].y;
    box.y += D_0015f430[126].y;
    box.w -= D_0015f430[126].y;
    box.h -= D_0015f430[126].y;
    SLIST(147)->x = box.x;
    SLIST(147)->y = box.y;
    SLIST(147)->x1 = box.w;
    SLIST(147)->y1 = box.h;
    box.fe = 1;
    box.ff = 1;
    box.f10 = 10;
    box.f12 = 0;
    add_glass_box(&box, 8, 0, 0);
    RestoreDisplay = DesiredDisplay;
    DesiredDisplay = 8;
    network.unknown2 = 3;
}

// FUNCTION: 0x00081b38
void process_keys_network_select(void)
{
    u32 i;

    if (network.unknown2 == 2) {
        if (CurrentDisplay & 0x400) {
            switch (BufferedInkey) {
            case 2:
                add_number('1');
                BufferedInkey = 0;
                break;
            case 3:
                add_number('2');
                BufferedInkey = 0;
                break;
            case 4:
                add_number('3');
                BufferedInkey = 0;
                break;
            case 5:
                add_number('4');
                BufferedInkey = 0;
                break;
            case 6:
                add_number('5');
                BufferedInkey = 0;
                break;
            case 7:
                add_number('6');
                BufferedInkey = 0;
                break;
            case 8:
                add_number('7');
                BufferedInkey = 0;
                break;
            case 9:
                add_number('8');
                BufferedInkey = 0;
                break;
            case 0xa:
                add_number('9');
                BufferedInkey = 0;
                break;
            case 0xb:
                add_number('0');
                BufferedInkey = 0;
                break;
            case 0x1c:
            case 0x9c:
                BufferedInkey = 0;
                play_sound(10, 0x41, -1, -1);
                reset_glass_map(0x400);
                add_phone_number(PhoneNumber);
                break;
            case 1:
                BufferedInkey = 0;
                play_sound(10, 0x42, -1, -1);
                reset_glass_map(0x400);
                break;
            case 0xe:
            case 0x53:
                BufferedInkey = 0;
                if (PhoneNumber[0]) {
                    play_sound(10, 0x10, -1, -1);
                    PhoneNumber[fn_000cd716(PhoneNumber) - 1] = 0;
                }
                break;
            }
        }
        switch (BufferedInkey) {
        case 0xe:
        case 0x53:
            if (BookHilight != -1) {
                play_sound(10, 0x10, -1, -1);
                for (i = BookHilight; BookCount - 1 > i; i++)
                    LbDrawBoxCoords(PhoneBook[i], PhoneBook[i + 1], 12);
                memset(PhoneBook[i], 0, 12);
                BookCount--;
                if (BookCount + 1 > 3 && D_001605f1 > BookCount - 3)
                    D_001605f1 = BookCount - 3;
                BookHilight = -1;
            }
            break;
        case 1:
            if (!(CurrentDisplay & 0x200) || login_control.players[player].len == 0) {
                play_sound(10, 0x42, -1, -1);
                network_button_quit();
                BufferedInkey = 0;
            }
            break;
        }
        if (CurrentDisplay & 0x200)
            send_message();
    }
    BufferedInkey = 0;
}

// FUNCTION: 0x00081f14
void process_mouse_network_select(void)
{
    u8 i;
    s16 base;
    s16 x;
    s16 y;
    s32 bit;
    s32 mask;

    if ((CurrentDisplay & DesiredDisplay) & 0x80)
        reqspr_draw(D_0015f43c[92].unknownC, D_0015f43c[92].unknownE, &screen_sprites_tab_start[2]);
    if (network.unknown2 == 2) {
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            base = button_index(bit);
            if (base == 0)
                continue;
            for (i = 0;; i++) {
                if (current_balance_file == current_balance_file->next && (i + base == 0x5e || i + base == 0x5f))
                    continue;
                if (D_0015f43c[i + base].width == 0 || D_0015f43c[i + base].height == 0)
                    break;
                x = D_0015f43c[i + base].x;
                y = D_0015f43c[i + base].y;
                if (D_00188c81) {
                    if (over_hotspot(D_00188c6c, D_00188c70, x, y, D_0015f43c[base + i].width - x + 1,
                                     D_0015f43c[i + base].height - y + 1)) {
                        D_00188c81 = 0;
                        if (D_0015f43c[i + base].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                        if (D_0015f43c[i + base].callback)
                            D_0015f43c[base + i].callback(i);
                        break;
                    }
                }
                if (D_00188c7e && over_hotspot((s16)mouse_dx, (s16)mouse_dy, x, y, D_0015f43c[base + i].width - x + 1,
                                               D_0015f43c[i + base].height - y + 1)) {
                    if (D_0015f43c[i + base].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, D_0015f43c[i + base].width - x, D_0015f43c[i + base].height - y, 1);
                }
            }
        }
    }
    D_00188c81 = 0;
}
