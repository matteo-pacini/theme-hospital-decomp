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
    u8 level;
    char title[0x20];
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
    u16 f20;
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
    BalanceFileInfo *prev;
    BalanceFileInfo *next;
};

struct ScreenPacket {
    u8 unknown0[4];
    u8 f4;
    u8 f5;
    u8 f6;
    u8 f7;
    u16 f8;
    u16 fa;
    u16 fc;
};

extern "C" {
extern u8 D_00160508;
extern Event D_001549d0[];
extern u8 *D_00153f34;
extern u8 D_0015f1aa;
extern u8 D_0015f1ab;
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

char *strchr(char *s, int c);
int strcmp(const char *a, const char *b);
void fn_00074cd8(void);
void fn_000110b0(void);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
void fn_0007fbb8(s8 me);
}

__tibcaGUID theme_hospital_guid = {0x1e8b2240, 0x11d037fe, 0xa0002b8d, 0xc96a00c9};
s8 BookHilight = -1;
char *upstr = "{";
char *downstr = "|";
u32 Bauds[] = {4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200};
u8 TopIRQ = 1;
long hilighted_session = -1;
extern "C" char D_00138050[0x20] = "";
char init_str[0x54] = "ATZ";
char dial_str[0x54] = "ATDT";
char hangup_str[0x54] = "ATH";
char auto_ans_str[0x54] = "ATS0=1";

extern Menu network;
extern s32 DesiredDisplay;
extern s32 CurrentDisplay;
extern s32 PendingDisplay;
extern NetControl net_control;
extern LoginControl login_control;
extern TbIffPalette ppalette;
extern u8 BufferedInkey;
extern ScreenPacket screen_packet[];
extern s8 BookCount;
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

#define SLIST(n) ((SlistButton *)&resvars.f48[n])

// FUNCTION: 0x0007f9f0
void read_modem_ini_file(void)
{
    char *p;
    char *buf;
    s32 len;
    char line[128];

    len = 0;
    buf = 0;
    len = LbFileLengthRnc("modem.ini");
    if (len == -1)
        return;
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0)
        return;
    if (LbFileLoadAt("modem.ini", buf) == -1) {
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    read_line_from_buf(0, 0, buf);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        fn_000bbff0(init_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        fn_000bbff0(dial_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        fn_000bbff0(hangup_str, p);
    read_line_from_buf(line, 0x7f, buf);
    p = strchr(line, '=');
    if (p && fn_000cd716(++p) <= 0x50)
        fn_000bbff0(auto_ans_str, p);
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// FUNCTION: 0x0007fbb8
void fn_0007fbb8(s8 me)
{
    s32 i;

    D_00160508 = 0;
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active != 0 && me != i) {
            D_00160508 = i;
            break;
        }
    }
}

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
            box.x = resvars.f3c_NetBoxp[126].x;
            box.h = (s16)resvars.f3c_NetBoxp[126].h * net_control.f4 + 0x17;
            box.y = (lbDisplay.graphics_screen_height - box.h) / 2;
            box.h += box.y;
            box.w = box.x + resvars.f3c_NetBoxp[126].w;
            box.f8 = 0x27;
            box.fe = 0;
            box.ff = 0;
            box.f10 = 7;
            box.f12 = 1;
            add_glass_box(&box, 0x1000, 0, 0);
            box.x += resvars.f3c_NetBoxp[126].y;
            box.y += 0xd;
            box.h = box.y + resvars.f3c_NetBoxp[126].h - 3;
            box.w = box.x + resvars.f3c_NetBoxp[126].w - resvars.f3c_NetBoxp[126].y * 2;
            box.fe = 1;
            box.ff = 1;
            box.f12 = 0;
            for (i = 0; i < net_control.f4; i++) {
                resvars.f48[i + 0x8c].x = box.x;
                resvars.f48[i + 0x8c].y = box.y;
                resvars.f48[i + 0x8c].width = box.w;
                resvars.f48[i + 0x8c].height = box.h;
                add_glass_box(&box, 0x1000, i * 3, i * 3);
                box.y += resvars.f3c_NetBoxp[126].h;
                box.h += resvars.f3c_NetBoxp[126].h;
            }
            resvars.f48[i + 0x8c].x = 0;
            resvars.f48[i + 0x8c].y = 0;
            resvars.f48[i + 0x8c].width = 0;
            resvars.f48[i + 0x8c].height = 0;
            DesiredDisplay |= bit;
        }
        idx = glass_index(bit);
        if (idx) {
            if (!(DesiredDisplay & bit))
                add_glass_frame(&resvars.f50_GlassFramep[idx], bit);
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
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost1.dat");
        calculate_grey(ppalette.data);
        fn_00074cd8();
        BufferedInkey = 0;
        login_control.f17 = 6;
        lbDisplay.draw_colour = color.unknownA;
        D_0015f1aa = 0;
        init_zoom_boxes(0x84);
        setup_balance_files(1);
        network.unknown2 = 3;
        if (myplayer.f56[0] == 0) {
            myplayer.f56[0] = 4;
            myplayer.f56[1] = 3;
            myplayer.f56[2] = 4;
            myplayer.f56[3] = 3;
            myplayer.f56[4] = 2;
            myplayer.f56[5] = 2;
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
        LbGhostTableGenerate(D_00153f34, 0x42, "qdata/ghost66.dat");
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
        lbDisplay.rleft_button = lbDisplay.left_button = 0;
        login_control.f17 = 6;
        D_0015f1ab = 0;
        clear_loading_screen(0);
        othervar.f24 = D_00153f34;
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
            if (D_00160508 == player && !(CurrentDisplay & 0x100))
                set_glass_map(0x100);
        }
        break;
    default:
        network.unknown2 = 0;
        break;
    }
}

// FUNCTION: 0x000802c4
void display_button_text(s32 mask)
{
    s32 bit;

    while (mask) {
        bit = mask & -mask;
        mask ^= bit;
        lbDisplay.draw_flags = 0;
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
            lbDisplay.draw_flags = 0;
            if (D_001606c8 == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(106), upstr);
            lbDisplay.draw_flags = 0;
            if (D_001606c8 >= 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(107), downstr);
            if (login_control.f17 == 5) {
                lbDisplay.draw_flags = 0;
                draw_glass_text(SLIST(104), D_00160308[11]);
                if (D_00160508 != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(105), D_00160308[13]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(105), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(105), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(105), D_00160308[3]);
                }
                lbDisplay.draw_flags |= 4;
                draw_glass_text(SLIST(103), D_00160308[12]);
            } else {
                lbDisplay.draw_flags = 0;
                if (!BookCount && (D_001606bc & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(103), D_00160308[12]);
                else
                    draw_glass_text(SLIST(103), D_00160308[12]);
                if ((D_001606bc & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(105), D_00160308[13]);
                else
                    draw_glass_text(SLIST(105), D_00160308[13]);
                if (BookHilight == -1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(104), D_00160308[10]);
                } else if ((D_001606bc & 7) < 4 && !(DesiredDisplay & 0x400)) {
                    draw_glass_text_hilight_no_jump(SLIST(104), D_00160308[10]);
                } else {
                    draw_glass_text(SLIST(104), D_00160308[10]);
                }
            }
            lbDisplay.draw_flags = 0;
            if (login_control.f17 != 5) {
                if (!D_001605f1)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.f1f == 0)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(101), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.f17 != 5) {
                if (D_001605f1 >= BookCount - 3)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.f1f >= player_count - 3)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(102), downstr);
            lbDisplay.draw_flags = 0;
            fn_000817c4();
            break;
        case 0x20:
            lbDisplay.draw_flags = 0;
            if (login_control.f17 != 5) {
                if (D_001606c8 == 0)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.f1f == 0)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(109), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.f17 != 5) {
                if (D_001606c8 >= 5)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.f1f >= player_count - 3)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(110), downstr);
            lbDisplay.draw_flags = 0;
            if (!D_001606c9)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(113), upstr);
            lbDisplay.draw_flags = 0;
            if (D_001606c9)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(114), downstr);
            lbDisplay.draw_flags = 0;
            switch (login_control.f17) {
            case 5:
                draw_glass_text(SLIST(112), D_00160308[4]);
                if (D_00160508 != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(111), D_00160308[14]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(111), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(111), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(111), D_00160308[3]);
                }
                break;
            case 6:
                if (!myplayer.f56[4] && (int)myplayer.f56[5] < 0)
                    lbDisplay.draw_flags |= 4;
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
            lbDisplay.draw_flags = 0;
            fn_0008172c();
            break;
        case 0x40:
            lbDisplay.draw_flags = 0;
            if (login_control.f1f == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(120), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.f1f >= player_count - 3)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(121), downstr);
            lbDisplay.draw_flags = 0;
            if (D_001606cc == 0 || login_control.f17 == 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(118), upstr);
            lbDisplay.draw_flags = 0;
            if (D_001606cc >= D_0015f1aa - 3 || login_control.f17 == 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(119), downstr);
            lbDisplay.draw_flags = 0;
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
                if (D_00160508 != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), D_00160308[2]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), D_00160308[3]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), D_00160308[3]);
                } else {
                    draw_glass_text(SLIST(117), D_00160308[3]);
                }
                break;
            case 6:
                if (hilighted_session == -1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), D_00160308[2]);
                } else if ((D_001606bc & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), D_00160308[2]);
                } else {
                    draw_glass_text(SLIST(117), D_00160308[2]);
                }
                break;
            }
            lbDisplay.draw_flags = 0;
            draw_network_game_selection_screen();
            break;
        case 0x100: {
            char buf[100];

            sprintf(buf, "%s %s", D_001602a0[30], balance_file_name);
            lbDisplay.draw_flags = 0;
            if (current_balance_file != current_balance_file->next) {
                draw_glass_text(SLIST(94), upstr);
                draw_glass_text(SLIST(95), downstr);
            }
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            set_font_from_cache(0x21);
            LbTextDraw((resvars.f3c_NetBoxp[132].w - LbTextStringWidth(buf)) / 2 + resvars.f3c_NetBoxp[132].x,
                       resvars.f3c_NetBoxp[132].y + (resvars.f3c_NetBoxp[132].h - (lbFontPtr ? lbFontPtr[66].height : 0)) / 2, buf);
            break;
        }
        case 0x200:
            draw_current_message(login_control.players[player].text);
            draw_message_box();
            draw_glass_text(SLIST(97), D_00160308[6]);
            lbDisplay.draw_flags = 0;
            if (TopMessage == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(98), upstr);
            lbDisplay.draw_flags = 0;
            if (TopMessage >= NumMessages - 3)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(99), downstr);
            break;
        case 0x400:
            draw_glass_text(SLIST(128), "1");
            draw_glass_text(SLIST(129), "2");
            draw_glass_text(SLIST(130), "3");
            draw_glass_text(SLIST(131), "4");
            draw_glass_text(SLIST(132), "5");
            draw_glass_text(SLIST(133), "6");
            draw_glass_text(SLIST(134), "7");
            draw_glass_text(SLIST(135), "8");
            draw_glass_text(SLIST(136), "9");
            draw_glass_text(SLIST(137), "0");
            if ((D_001606bc & 7) > 3)
                draw_glass_text_hilight_no_jump(SLIST(138), "+");
            else
                draw_glass_text(SLIST(138), "+");
            lbDisplay.draw_flags = 0;
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
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
            lbDisplay.draw_flags = 0;
            if (TopIRQ == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(123), upstr);
            lbDisplay.draw_flags = 0;
            if (TopIRQ >= 4)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(124), downstr);
            lbDisplay.draw_flags = 0;
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
    y = resvars.f3c_NetBoxp[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, D_00160308[15], i + 1 + D_001606c9);
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.f3c_NetBoxp[idx].x, y, (s16)resvars.f3c_NetBoxp[idx].w,
                         character_height())) {
            if (lbDisplay.mleft_button) {
                myplayer.f56[4] = 1 << (i + D_001606c9);
                lbDisplay.left_button = 0;
            }
        }
        if (((s8)myplayer.f56[4] >> (i + D_001606c9)) == 1)
            draw_small_highlight_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
        else
            draw_small_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
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
    y = resvars.f3c_NetBoxp[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, "%d", Bauds[i + D_001606c8]);
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.f3c_NetBoxp[idx].x, y, (s16)resvars.f3c_NetBoxp[idx].w,
                         character_height())) {
            if (lbDisplay.mleft_button) {
                myplayer.f56[5] = D_001606c8 + i;
                lbDisplay.left_button = 0;
            }
        }
        if (myplayer.f56[5] == D_001606c8 + i)
            draw_small_highlight_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
        else
            draw_small_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
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
        bits = (s8)myplayer.f56[4] >> 1;
        while (bits != 0) {
            n++;
            bits >>= 1;
        }
        idx = 0x85;
        y = resvars.f3c_NetBoxp[idx].y + 10;
        for (i = 0; i < 4; i++, y += character_height()) {
            sprintf(buf, "IRQ %d", i + TopIRQ);
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.f3c_NetBoxp[idx].x, y, (s16)resvars.f3c_NetBoxp[idx].w,
                             character_height())) {
                if (lbDisplay.mleft_button) {
                    myplayer.f56[n] = TopIRQ + i;
                    lbDisplay.left_button = 0;
                }
            }
            if (myplayer.f56[n] == TopIRQ + i)
                draw_small_highlight_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
            else
                draw_small_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, buf);
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
            fn_000c21e6(PhoneBook[BookCount++], number, 12);
        } else {
            BookHilight = 0;
            fn_000c21e6(PhoneBook[0], number, 12);
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
    box.w = LbTextStringWidth(message) + resvars.f3c_NetBoxp[126].y * 4;
    box.x = (lbDisplay.graphics_screen_width - box.w) / 2;
    box.h = resvars.f3c_NetBoxp[126].h + resvars.f3c_NetBoxp[126].y * 2;
    box.y = (lbDisplay.graphics_screen_height - box.h) / 2;
    box.h += box.y;
    box.w += box.x;
    box.f8 = 0x27;
    box.fe = 0;
    box.ff = 0;
    box.f10 = 10;
    box.f12 = 1;
    add_glass_box(&box, 8, 0, 0);
    box.x += resvars.f3c_NetBoxp[126].y;
    box.y += resvars.f3c_NetBoxp[126].y;
    box.w -= resvars.f3c_NetBoxp[126].y;
    box.h -= resvars.f3c_NetBoxp[126].y;
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
                    fn_000c21e6(PhoneBook[i], PhoneBook[i + 1], 12);
                memset(PhoneBook[i], 0, 12);
                BookCount--;
                if (BookCount + 1 > 3 && D_001605f1 > BookCount - 3)
                    D_001605f1 = BookCount - 3;
                BookHilight = -1;
            }
            break;
        case 1:
            if (!(CurrentDisplay & 0x200) || !login_control.players[player].len) {
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
    s16 x;
    s16 base;
    s16 y;
    s32 mask;
    s32 bit;

    if ((CurrentDisplay & DesiredDisplay) & 0x80)
        reqspr_draw((u16)resvars.f48[92].x, (u16)resvars.f48[92].y, &screen_sprites_tab_start[2]);
    if (network.unknown2 == 2) {
        mask = CurrentDisplay;
        while (mask) {
            bit = mask & -mask;
            mask ^= bit;
            base = button_index(bit);
            if (!base)
                continue;
            for (i = 0;; i++) {
                if (current_balance_file == current_balance_file->next) {
                    if (base + i == 0x5e || base + i == 0x5f)
                        continue;
                }
                if (resvars.f48[base + i].width == 0 || resvars.f48[base + i].height == 0)
                    break;
                x = resvars.f48[base + i].x;
                y = resvars.f48[base + i].y;
                if (lbDisplay.rleft_button) {
                    if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, y, resvars.f48[base + i].width - x + 1,
                                     resvars.f48[base + i].height - y + 1)) {
                        lbDisplay.rleft_button = 0;
                        if (resvars.f48[base + i].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, resvars.f48[base + i].width - x, resvars.f48[base + i].height - y, 1);
                        if (resvars.f48[base + i].callback)
                            resvars.f48[base + i].callback(i);
                        break;
                    }
                }
                if (lbDisplay.mleft_button && over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, x, y, resvars.f48[base + i].width - x + 1,
                                               resvars.f48[base + i].height - y + 1)) {
                    if (resvars.f48[base + i].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, resvars.f48[base + i].width - x, resvars.f48[base + i].height - y, 1);
                }
            }
        }
    }
    lbDisplay.rleft_button = 0;
}

struct NetInitData {
    s32 port;
    u32 baud;
    s32 irq;
    u8 unknownC[4];
    char *init;
    char *dial;
    char *hangup;
    char *auto_ans;
};

/* __stdcall, but with the address-named symbol */
#pragma aux net_callback "*_" parm routine [] modify [eax ecx edx]

extern "C" {
extern NetInitData D_0016051c;
extern char D_00160594[];
extern s32 D_001605e4;
extern s32 (*D_001605e8)(void);
extern s32 (*D_001605ec)(void);
extern TbNetworkSessionNameEntry *D_00160e34[];
extern s32 D_00160eb4;
void fn_000842d4(TbNetworkCallbackData *data, void *arg);
#pragma aux (net_callback) fn_000842d4;
}

#define SESSION_NAME ((char *)D_001609c4 + 0x70)

u8 setup_network(TbNetworkService service, void *data, u8 max_players);
s32 network_button_create(void);
s32 network_button_join(void);
void network_button_message(void);
void report_message(char *message);
void prev_balance_file(void);
void next_balance_file(void);
s32 network_service_function_init(void);
s32 network_service_function_connect(void);

// FUNCTION: 0x000822c0
void process_network_buttons(u8 n)
{
    TbNetworkService service;

    switch (n) {
    case 0:
        play_sound(10, 0x43, -1, -1);
        if (CurrentDisplay & 0x40) {
            reset_glass_map(0x40);
        } else {
            service = NS_IPX;
            if (setup_network(service, 0, 4)) {
                if (!(CurrentDisplay & 0x40))
                    set_glass_map(0x40);
            } else {
                report_message(D_00160308[16]);
            }
        }
        break;
    case 1:
        play_sound(10, 0x43, -1, -1);
        toggle_glass_map(0x20);
        if (DesiredDisplay & 0x20)
            set_glass_map(0x8000);
        else
            reset_glass_map(0x8000);
        break;
    case 2:
        play_sound(10, 0x43, -1, -1);
        toggle_glass_map(0x10);
        if (DesiredDisplay & 0x10)
            set_glass_map(0x10000);
        else
            reset_glass_map(0x10000);
        break;
    case 3:
        play_sound(10, 0x42, -1, -1);
        network_button_quit();
        break;
    }
}

// FUNCTION: 0x0008242c
void process_level_buttons(u8 n)
{
    play_sound(10, 0x43, -1, -1);
    level_number = net_control.players[n].level;
    screen_packet[player].f5 = 2;
}

// FUNCTION: 0x00082480
void process_ipx_buttons(u8 n)
{
    switch (n) {
    case 2:
        if (D_001606cc) {
            play_sound(10, 0x43, -1, -1);
            D_001606cc--;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 3:
        if (D_001606cc < D_0015f1aa - 3) {
            play_sound(10, 0x43, -1, -1);
            D_001606cc++;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 0:
        switch (login_control.f17) {
        case 5:
            play_sound(10, 0x42, -1, -1);
            network_button_quit();
            break;
        case 6:
            play_sound(10, 0x43, -1, -1);
            if (network_button_create() == -1)
                report_message(D_00160308[27]);
            break;
        }
        break;
    case 4:
        if (login_control.f1f) {
            play_sound(10, 0x43, -1, -1);
            login_control.f1f--;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 5:
        if (login_control.f1f < player_count - 3) {
            play_sound(10, 0x43, -1, -1);
            login_control.f1f++;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 1:
        switch (login_control.f17) {
        case 5:
            play_sound(10, 0x43, -1, -1);
            if (D_00160508 == player && player_count > 1)
                screen_packet[player].f5 = 9;
            break;
        case 6:
            play_sound(10, 0x43, -1, -1);
            if (hilighted_session != -1) {
                if (network_button_join() == -1)
                    report_message(D_00160308[28]);
            }
            break;
        }
        break;
    }
}

// FUNCTION: 0x00082740
void process_message_buttons(u8 n)
{
    switch (n) {
    case 0:
        network_button_message();
        break;
    case 1:
        if (TopMessage) {
            play_sound(10, 0x43, -1, -1);
            TopMessage--;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 2:
        if (TopMessage < NumMessages - 3) {
            play_sound(10, 0x43, -1, -1);
            TopMessage++;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    }
}

// FUNCTION: 0x00082838
void process_balance_buttons(u8 n)
{
    switch (n) {
    case 0:
        if (current_balance_file != current_balance_file->next)
            play_sound(10, 0x43, -1, -1);
        prev_balance_file();
        break;
    case 1:
        if (current_balance_file != current_balance_file->prev)
            play_sound(10, 0x43, -1, -1);
        next_balance_file();
        break;
    }
}

// FUNCTION: 0x000828d4
void process_irq_buttons(u8 n)
{
    switch (n) {
    case 0:
        if (TopIRQ) {
            play_sound(10, 0x43, -1, -1);
            TopIRQ--;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 1:
        if (TopIRQ < 4) {
            play_sound(10, 0x43, -1, -1);
            TopIRQ++;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    }
}

// FUNCTION: 0x000829a4
void process_serial_buttons(u8 n)
{
    TbNetworkService service;
    s8 bits;

    switch (n) {
    case 0:
        if (login_control.f17 != 5) {
            if (D_001606c8) {
                play_sound(10, 0x43, -1, -1);
                D_001606c8--;
            } else {
                play_sound(10, 8, -1, -1);
            }
        } else {
            if (login_control.f1f) {
                play_sound(10, 0x43, -1, -1);
                login_control.f1f--;
            } else {
                play_sound(10, 8, -1, -1);
            }
        }
        break;
    case 1:
        if (login_control.f17 != 5) {
            if (D_001606c8 < 5) {
                play_sound(10, 0x43, -1, -1);
                D_001606c8++;
            } else {
                play_sound(10, 8, -1, -1);
            }
        } else {
            if (login_control.f1f < player_count - 3) {
                play_sound(10, 0x43, -1, -1);
                login_control.f1f++;
            } else {
                play_sound(10, 8, -1, -1);
            }
        }
        break;
    case 4:
        if (D_001606c9) {
            play_sound(10, 0x43, -1, -1);
            D_001606c9 = 0;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 5:
        if (!D_001606c9) {
            play_sound(10, 0x43, -1, -1);
            D_001606c9 = 1;
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 2:
    case 3:
        if (login_control.f17 != 5) {
            play_sound(10, 0x43, -1, -1);
            if (myplayer.f56[4] && (int)myplayer.f56[5] >= 0) {
                service = NS_SERIAL;
                bits = (s8)myplayer.f56[4] >> 1;
                D_0016051c.port = 0;
                while (bits) {
                    bits >>= 1;
                    D_0016051c.port++;
                }
                D_0016051c.baud = Bauds[myplayer.f56[5]];
                D_0016051c.irq = myplayer.f56[D_0016051c.port];
                D_0016051c.init = init_str;
                D_0016051c.dial = dial_str;
                D_0016051c.hangup = hangup_str;
                D_0016051c.auto_ans = auto_ans_str;
                if (setup_network(service, &D_0016051c, 2)) {
                    if (n == 3) {
                        if (network_button_create() == -1) {
                            LbNetwork_Stop();
                            report_message(D_00160308[29]);
                        }
                    } else {
                        D_00160eb4 = 0;
                        if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)fn_000842d4, 0) == Lb_OK) {
                            hilighted_session = 0;
                            fn_000bbff0(D_00138050, SESSION_NAME);
                            if (network_button_join() == -1) {
                                LbNetwork_Stop();
                                report_message(D_00160308[31]);
                            }
                        }
                    }
                } else {
                    report_message(D_00160308[17]);
                }
            }
        } else {
            if (n == 3) {
                play_sound(10, 0x42, -1, -1);
                network_button_quit();
            } else {
                play_sound(10, 0x43, -1, -1);
                if (D_00160508 == player && player_count > 1)
                    screen_packet[player].f5 = 9;
            }
        }
        break;
    }
}

// FUNCTION: 0x00082da0
void process_modem_buttons(u8 n)
{
    if (!(CurrentDisplay & 0x400)) {
        s8 bits;
        TbNetworkService service;
        s32 res;

        switch (n) {
        case 0:
            if (login_control.f17 != 5) {
                if (D_001605f1) {
                    play_sound(10, 0x43, -1, -1);
                    D_001605f1--;
                } else {
                    play_sound(10, 8, -1, -1);
                }
            } else {
                if (login_control.f1f) {
                    play_sound(10, 0x43, -1, -1);
                    login_control.f1f--;
                } else {
                    play_sound(10, 8, -1, -1);
                }
            }
            break;
        case 1:
            if (login_control.f17 != 5) {
                if (D_001605f1 < BookCount - 3) {
                    play_sound(10, 0x43, -1, -1);
                    D_001605f1++;
                } else {
                    play_sound(10, 8, -1, -1);
                }
            } else {
                if (login_control.f1f < player_count - 3) {
                    play_sound(10, 0x43, -1, -1);
                    login_control.f1f++;
                } else {
                    play_sound(10, 8, -1, -1);
                }
            }
            break;
        case 5:
            if (D_001606c8) {
                play_sound(10, 0x43, -1, -1);
                D_001606c8--;
            } else {
                play_sound(10, 8, -1, -1);
            }
            if (D_001606c9)
                D_001606c9 = 0;
            break;
        case 6:
            if (D_001606c8 < 5) {
                play_sound(10, 0x43, -1, -1);
                D_001606c8++;
            } else {
                play_sound(10, 8, -1, -1);
            }
            if (!D_001606c9)
                D_001606c9 = 1;
            break;
        case 2:
            play_sound(10, 0x43, -1, -1);
            if (login_control.f17 != 5) {
                if (!(CurrentDisplay & 0x400)) {
                    set_glass_map(0x400);
                    PhoneNumber[0] = 0;
                }
            }
            break;
        case 3:
        case 4:
            if (login_control.f17 != 5) {
                play_sound(10, 0x43, -1, -1);
                if (n == 3 && BookHilight == -1)
                    break;
                reset_glass_map(0x400);
                if (myplayer.f56[4] && (int)myplayer.f56[5] >= 0) {
                    service = NS_MODEM;
                    bits = (s8)myplayer.f56[4] >> 1;
                    D_0016051c.port = 0;
                    while (bits) {
                        bits >>= 1;
                        D_0016051c.port++;
                    }
                    D_0016051c.baud = Bauds[myplayer.f56[5]];
                    D_0016051c.irq = myplayer.f56[D_0016051c.port];
                    D_0016051c.init = init_str;
                    D_0016051c.dial = dial_str;
                    D_0016051c.hangup = hangup_str;
                    D_0016051c.auto_ans = auto_ans_str;
                    if (setup_network(service, &D_0016051c, 2)) {
                        fn_000bbff0(D_00160594, PhoneBook[BookHilight]);
                        if (n == 4) {
                            D_001605e8 = network_service_function_init;
                            RestoreDisplay = DesiredDisplay;
                            DesiredDisplay = 4;
                            set_glass_map(0x800);
                            D_00160810[0] = 0;
                            res = network_button_create();
                            reset_glass_map(0x800);
                            if (res == -1) {
                                switch (D_001605e4) {
                                case 4:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[21]);
                                    break;
                                case 5:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[22]);
                                    break;
                                case 6:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[23]);
                                    break;
                                case 7:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[24]);
                                    break;
                                case -1:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[25]);
                                    break;
                                case -2:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(D_00160308[26]);
                                    break;
                                case -7:
                                    set_glass_map(RestoreDisplay);
                                    break;
                                case -6:
                                case 0:
                                case -4:
                                case 2:
                                case -5:
                                case 1:
                                case -3:
                                case 3:
                                default:
                                    set_glass_map(RestoreDisplay);
                                    break;
                                }
                                LbNetwork_Stop();
                            } else {
                                set_glass_map((RestoreDisplay | DesiredDisplay) & ~0x10080);
                            }
                        } else {
                            D_00160eb4 = 0;
                            if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)fn_000842d4, 0) == Lb_OK) {
                                hilighted_session = 0;
                                fn_000bbff0(D_00138050, SESSION_NAME);
                                D_001605e8 = network_service_function_init;
                                D_001605ec = network_service_function_connect;
                                RestoreDisplay = DesiredDisplay;
                                DesiredDisplay = 4;
                                set_glass_map(0x800);
                                D_00160810[0] = 0;
                                res = network_button_join();
                                reset_glass_map(0x800);
                                if (res == -1) {
                                    switch (D_001605e4) {
                                    case 4:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[21]);
                                        break;
                                    case 5:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[22]);
                                        break;
                                    case 6:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[23]);
                                        break;
                                    case 7:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[24]);
                                        break;
                                    case -1:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[25]);
                                        break;
                                    case -2:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(D_00160308[26]);
                                        break;
                                    case -7:
                                        set_glass_map(RestoreDisplay);
                                        break;
                                    case -6:
                                    case 0:
                                    case -4:
                                    case 2:
                                    case -5:
                                    case 1:
                                    case -3:
                                    case 3:
                                    default:
                                        set_glass_map(RestoreDisplay);
                                        break;
                                    }
                                    LbNetwork_Stop();
                                } else {
                                    set_glass_map((RestoreDisplay | DesiredDisplay) & ~0x10080);
                                }
                            }
                        }
                    } else {
                        report_message(D_00160308[18]);
                    }
                }
            } else {
                if (n == 3) {
                    play_sound(10, 0x42, -1, -1);
                    network_button_quit();
                } else {
                    play_sound(10, 0x43, -1, -1);
                    if (D_00160508 == player && player_count > 1)
                        screen_packet[player].f5 = 9;
                }
            }
            break;
        }
    }
}

// FUNCTION: 0x00083554
void process_number_buttons(u8 n)
{
    play_sound(10, 0x43, -1, -1);
    switch (n) {
    case 0:
        add_number('1');
        break;
    case 1:
        add_number('2');
        break;
    case 2:
        add_number('3');
        break;
    case 3:
        add_number('4');
        break;
    case 4:
        add_number('5');
        break;
    case 5:
        add_number('6');
        break;
    case 6:
        add_number('7');
        break;
    case 7:
        add_number('8');
        break;
    case 8:
        add_number('9');
        break;
    case 10:
        reset_glass_map(0x400);
        add_phone_number(PhoneNumber);
        break;
    case 9:
        add_number('0');
        break;
    }
}

extern "C" {
}

// FUNCTION: 0x00083684
void draw_phone_numbers(void)
{
    u8 start;
    s16 y;
    s16 i;
    char buf[16];

    if ((DesiredDisplay & 0x400) && BookCount >= 3)
        start = 1;
    else
        start = 0;
    y = resvars.f3c_NetBoxp[127].y + 4;
    for (i = start; i < 3 && i + D_001605f1 < BookCount; i++, y += character_height()) {
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.f3c_NetBoxp[127].x, y, (s16)resvars.f3c_NetBoxp[127].w,
                         character_height())) {
            if (lbDisplay.mleft_button) {
                BookHilight = i + D_001605f1;
                lbDisplay.left_button = 0;
            }
        }
        if (i + D_001605f1 == BookHilight)
            draw_small_highlight_centre_text(resvars.f3c_NetBoxp[127].x, resvars.f3c_NetBoxp[127].w, y, PhoneBook[i + D_001605f1]);
        else
            draw_small_centre_text(resvars.f3c_NetBoxp[127].x, resvars.f3c_NetBoxp[127].w, y, PhoneBook[i + D_001605f1]);
    }
    if (DesiredDisplay & 0x400) {
        sprintf(buf, "%s*", PhoneNumber);
        draw_small_centre_text(resvars.f3c_NetBoxp[127].x, resvars.f3c_NetBoxp[127].w, y, buf);
    }
}

struct SessionInfo;

extern SessionInfo t_session[];
extern char NetMessageText[][40];

extern "C" {
void fn_00084324(SessionInfo *sessions);
}

// FUNCTION: 0x0008385c
void draw_network_game_selection_screen(void)
{
    switch (login_control.f17) {
    case 5:
        fn_000841b8(0);
        process_login_exchange();
        break;
    case 6:
        fn_00084324(t_session);
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

// FUNCTION: 0x000838e8
void draw_report_selection_screen(void)
{
    switch (login_control.f17) {
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.unknownfd <= 1)
            break;
        network.unknown2 = 1;
        break;
    }
}

// FUNCTION: 0x00083944
void draw_level_selection_screen(void)
{
    switch (login_control.f17) {
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.unknownfd <= 1)
            break;
        network.unknown2 = 1;
        break;
    }
}

// FUNCTION: 0x000839a0
void draw_message_box(void)
{
    s16 y;
    s16 i;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1f);
    lbDisplay.draw_flags = 0;
    y = resvars.f3c_NetBoxp[121].y;
    for (i = TopMessage; i - TopMessage < 3 && NetMessageText[i][0]; i++, y += character_height())
        draw_small_centre_text(resvars.f3c_NetBoxp[121].x, resvars.f3c_NetBoxp[121].w, y, NetMessageText[i]);
}

void update_message_box(u8 c, u8 flag, u8 from);

// FUNCTION: 0x00083a78
void process_login_exchange(void)
{
    TbNetworkService service;
    s16 i;
    s16 j;

    fn_0007fbb8(-1);
    screen_packet[player].f6 = 1;
    screen_packet[player].f8 = login_control.f20;
    LbNetwork_ChangeExchangeBuffer(screen_packet, 0xe);
    LbNetwork_Exchange(&screen_packet[player]);
    if (!D_0015f1ab) {
        screen_packet[player].f4 = screen_packet[D_00160508].f4;
        screen_packet[player].fa = screen_packet[D_00160508].fa;
        screen_packet[player].fc = screen_packet[D_00160508].fc;
        login_control.f20 = screen_packet[player].f8 = screen_packet[D_00160508].f8;
        D_0015f1ab = 1;
    }
    for (i = 0; i < 4; i++) {
        switch (screen_packet[i].f5) {
        case 1:
            break;
        case 5:
            break;
        case 4:
            login_control.f20 = screen_packet[i].f8;
            break;
        case 6:
            break;
        case 7:
            update_message_box(screen_packet[i].f7 & 0x7f, screen_packet[i].f7 & 0x80, i);
            break;
        case 2:
            login_control.f17 = 8;
            statvar.unknownfd = network_num_players();
            LbNetwork_ChangeExchangeTimeout(0x1e);
            break;
        case 8:
            if (i == player || (!(DesiredDisplay & 0x40) && i == D_00160508)) {
                LbNetwork_Stop();
                login_control.f17 = 6;
                set_glass_map(0x80);
                reset_glass_map(0x300);
                for (j = 0; j < 4; j++)
                    screen_packet[j].f5 = 0;
                if (DesiredDisplay & 0x40) {
                    service = NS_IPX;
                    if (!setup_network(service, 0, 4)) {
                        reset_glass_map(0x40);
                        report_message(D_00160308[16]);
                    }
                }
                if (DesiredDisplay & 0x20)
                    set_glass_map(0x8000);
                if (DesiredDisplay & 0x10)
                    set_glass_map(0x10000);
            }
            fn_0007fbb8(i);
            break;
        case 9:
            login_control.f17 = 9;
            if (D_00160508 == player) {
                LbNetwork_EnableNewPlayers(0);
                set_glass_map(0x1000);
            } else {
                report_message(D_00160308[19]);
            }
            break;
        }
        screen_packet[i].f5 = 0;
    }
}

// FUNCTION: 0x00083d98
u8 start_session(void)
{
    screen_packet[player].f5 = 2;
    return 0;
}

extern "C" {
}

// FUNCTION: 0x00083dc4
void update_message_box(u8 key, u8 shift, u8 from)
{
    s32 i;
    s32 j;
    s8 c;
    char buf[64];

    switch (key) {
    case 1:
        login_control.players[from].len = 0;
        login_control.players[from].text[0] = 0;
        if (from == player)
            play_sound(10, 0x42, -1, -1);
        break;
    case 0x1c:
        if (login_control.players[from].len) {
            for (i = 0; i < 9; i++) {
                if (!NetMessageText[i][0])
                    break;
            }
            if (i == 9) {
                for (j = 0; j < 8; j++)
                    fn_000c21e6(NetMessageText[j], NetMessageText[j + 1], 40);
                i--;
            }
            sprintf(NetMessageText[i], "%s:%s", &player_info[from], login_control.players[from].text);
            login_control.players[from].len = 0;
            login_control.players[from].text[0] = 0;
            if (from == player)
                play_sound(10, 0x41, -1, -1);
            if (NumMessages < 9)
                NumMessages++;
            if (NumMessages > TopMessage + 3)
                TopMessage = NumMessages - 3;
        } else {
            if (from == player)
                play_sound(10, 0x41, -1, -1);
        }
        break;
    case 0xe:
    case 0x53:
        if (login_control.players[from].len > 0) {
            if (from == player)
                play_sound(10, 0x10, -1, -1);
            login_control.players[from].text[--login_control.players[from].len] = 0;
        }
        break;
    default:
        if (shift)
            c = lbInkeyToAsciiShift[key];
        else
            c = lbInkeyToAscii[key];
        if (c >= (s8)'a' && c <= (s8)'z')
            c += -32;
        if (c < 0x7f && c > 0x1f) {
            set_font_from_cache(0x21);
            if (from == player)
                play_sound(10, 0xc, -1, -1);
            if (login_control.players[from].len < 0x1e &&
                LbTextStringWidth(login_control.players[from].text) < resvars.f3c_NetBoxp[120].w - 0x28) {
                sprintf(buf, "%s:%s", &player_info[from], login_control.players[from].text);
                if (LbTextStringWidth(buf) < resvars.f3c_NetBoxp[121].w - 0x14) {
                    login_control.players[from].text[login_control.players[from].len++] = c;
                    login_control.players[from].text[login_control.players[from].len] = 0;
                }
            }
        }
        break;
    }
}

#define SESSION_NAMES ((char (*)[32])SESSION_NAME)

struct LevelPacket {
    u8 unknown0[4];
    u8 level;
    char balance[10];
};

struct CriteriaPacket {
    u8 unknown0[4];
    u8 result;
    u8 unknown5;
};

extern char player_names[][10];
extern CriteriaPacket criteria_packet[];

extern "C" {
extern LevelPacket D_00160eb8[];
}

void draw_players_on_session(u32 session);
void balance_filename(char *name);

// FUNCTION: 0x000841b8
void fn_000841b8(u8 flag)
{
    s32 idx;
    s16 y;
    s16 i;
    s16 j;

    if (flag)
        idx = 0x7d;
    else
        idx = 0x7b;
    y = resvars.f3c_NetBoxp[idx].y + 4;
    player_count = 0;
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active)
            player_count++;
    }
    if (player_count <= 3)
        login_control.f1f = 0;
    for (i = 0, j = login_control.f1f; i < 3 && j < 4; j++) {
        if (D_001609c4[j].active) {
            draw_small_centre_text(resvars.f3c_NetBoxp[idx].x, resvars.f3c_NetBoxp[idx].w, y, player_info[j].name);
            y += character_height();
            i++;
        }
    }
}

// FUNCTION: 0x000842d4
void fn_000842d4(TbNetworkCallbackData *data, void *arg)
{
    TbNetworkSessionNameEntry *session;

    session = (TbNetworkSessionNameEntry *)data;
    fn_000bbff0(SESSION_NAMES[D_00160eb4], session->name);
    D_00160e34[D_00160eb4] = session;
    D_00160eb4++;
}

// FUNCTION: 0x00084324
void fn_00084324(SessionInfo *sessions)
{
    u8 swapped;
    u8 found;
    s16 y;
    TbNetworkSessionNameEntry *t;
    s32 i;
    char name[32];

    found = 0;
    D_00160eb4 = 0;
    if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)fn_000842d4, 0) != Lb_OK)
        return;
    swapped = 1;
    while (swapped) {
        swapped = 0;
        for (i = 0; D_00160eb4 - 1 > i; i++) {
            if (strcmp(SESSION_NAMES[i], SESSION_NAMES[i + 1]) > 0) {
                fn_000bbff0(name, SESSION_NAMES[i]);
                fn_000bbff0(SESSION_NAMES[i], SESSION_NAMES[i + 1]);
                fn_000bbff0(SESSION_NAMES[i + 1], name);
                t = D_00160e34[i];
                D_00160e34[i] = D_00160e34[i + 1];
                D_00160e34[i + 1] = t;
                swapped = 1;
                break;
            }
        }
    }
    if (D_00160eb4 == 0) {
        hilighted_session = -1;
        D_00138050[0] = 0;
    } else if (hilighted_session == -1) {
        hilighted_session = 0;
        fn_000bbff0(D_00138050, SESSION_NAMES[0]);
    }
    if (hilighted_session >= 0) {
        if (strcmp(D_00138050, SESSION_NAMES[hilighted_session])) {
            for (i = 0; i < D_00160eb4; i++) {
            if (!strcmp(SESSION_NAMES[i], D_00138050)) {
                if (i < 0) {
                    hilighted_session = i;
                    D_00138050[0] = 0;
                } else {
                    hilighted_session = i;
                    fn_000bbff0(D_00138050, SESSION_NAMES[i]);
                }
                break;
            }
        }
        if (i == D_00160eb4) {
            hilighted_session = -1;
            D_00138050[0] = 0;
        }
        }
    }
    y = resvars.f3c_NetBoxp[125].y + 4;
    for (i = D_001606cc; D_001606cc + D_00160eb4 > i && D_001606cc + 3 > i; i++, y += character_height()) {
        if (i == hilighted_session && !found) {
            draw_small_highlight_centre_text(resvars.f3c_NetBoxp[125].x, resvars.f3c_NetBoxp[125].w, y, SESSION_NAMES[i]);
            draw_players_on_session(i);
            found = 1;
        } else {
            draw_small_centre_text(resvars.f3c_NetBoxp[125].x, resvars.f3c_NetBoxp[125].w, y, SESSION_NAMES[i]);
        }
    }
    y = resvars.f3c_NetBoxp[125].y + 4;
    for (i = D_001606cc; D_001606cc + D_00160eb4 > i && D_001606cc + 3 > i; i++, y += character_height()) {
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.f3c_NetBoxp[125].x, y, (s16)resvars.f3c_NetBoxp[125].w,
                         character_height())) {
            if (lbDisplay.mleft_button) {
                if (hilighted_session != i) {
                    memset(player_info, 0, 0x90);
                    memset(player_names, 0, 0x28);
                }
                if (i < 0) {
                    hilighted_session = i;
                    D_00138050[0] = 0;
                } else {
                    hilighted_session = i;
                    fn_000bbff0(D_00138050, SESSION_NAMES[i]);
                }
                lbDisplay.mleft_button = lbDisplay.left_button = 0;
            }
            break;
        }
    }
}

// FUNCTION: 0x00084748
void __stdcall players_callback(TbNetworkCallbackData *data, void *arg)
{
    TbNetworkPlayerNameEntry *entry;

    entry = (TbNetworkPlayerNameEntry *)data;
    fn_000bbff0(player_names[player_count], entry->name);
    player_count++;
}

// FUNCTION: 0x00084784
void draw_players_on_session(u32 session)
{
    s16 y;
    u32 n;
    u32 i;

    player_count = 0;
    if (LbNetwork_EnumeratePlayers(D_00160e34[session], players_callback, 0) != Lb_OK)
        return;
    if (player_count <= 3)
        login_control.f1f = 0;
    y = resvars.f3c_NetBoxp[123].y + 4;
    for (n = 0, i = login_control.f1f; n < 3 && i < 4; i++) {
        if (player_names[i][0]) {
            draw_small_centre_text(resvars.f3c_NetBoxp[123].x, resvars.f3c_NetBoxp[123].w, y, player_names[i]);
            y += character_height();
            n++;
        }
    }
}

// FUNCTION: 0x00084864
void swap_level_number(void)
{
    if (statvar.unknownfd > 1) {
        D_00160eb8[player].level = level_number;
        fn_000bbff0(D_00160eb8[player].balance, balance_id);
        LbNetwork_ChangeExchangeBuffer(D_00160eb8, 0xf);
        LbNetwork_Exchange(&D_00160eb8[player]);
        level_number = D_00160eb8[D_00160508].level;
        fn_000bbff0(balance_id, D_00160eb8[D_00160508].balance);
        if ((s8)balance_id[0] != 0)
            balance_filename(balance_id);
    }
}

// FUNCTION: 0x00084924
void swap_month_end_packets(void)
{
}

// FUNCTION: 0x0008493c
void fn_0008493c(u8 fail, u8 active)
{
    u8 losers;
    u8 winners;
    u8 i;

    losers = 0;
    winners = 0;
    if (statvar.unknownfd > 1) {
        criteria_packet[player].result = fail;
        LbNetwork_ChangeExchangeBuffer(criteria_packet, 6);
        LbNetwork_Exchange(&criteria_packet[player]);
        for (i = 0; i < 4; i++) {
            othervar.fa6c[i] = 0;
            if (D_001609c4[i].active) {
                if (criteria_packet[i].result == 0) {
                    losers++;
                } else if (criteria_packet[i].result == 1) {
                    winners++;
                    othervar.fa6c[i] = 1;
                }
            }
        }
        if (losers != 0) {
            if (winners)
                advisor.f2 = 2;
        }
        if (active) {
            if (losers == 0)
                othervar.fa5e = 1;
            else if (fail)
                othervar.fa5e = 2;
            else
                othervar.fa5e = 3;
        }
    }
}

struct YearEndPacket {
    s32 f0;
    s32 f4;
    s32 f8;
    u16 fc;
    u16 fe;
    u16 f10;
    u16 f12;
    s32 f14;
    s16 f18;
    u16 f1a;
    u16 f1c;
    u8 f1e;
    u8 f1f;
    u16 f20;
    u16 f22;
    u16 f24;
    s16 f26[2];
    u8 unknown2a[6];
    u16 f30;
    u16 f32;
    u8 f34;
    s32 f35;
    s32 f39;
    s32 f3d;
    u8 unknown41[4];
};

extern "C" {
extern Event D_00154a0c[];
extern YearEndPacket D_00154a48[];
}

/* The exchanged packets start with a 4-byte header, which overlaps D_00154a0c. */
#define YEAR_END_PACKETS ((u8 *)D_00154a0c + 0x38)

// FUNCTION: 0x00084a68
void swap_year_end_packets(void)
{
    u8 i;

    if (statvar.unknownfd <= 1)
        return;
    D_00154a48[player].f0 = myhosp->f2d;
    D_00154a48[player].f4 = myhosp->f35;
    D_00154a48[player].f8 = myhosp->f3d;
    D_00154a48[player].fc = myhosp->f1e9;
    D_00154a48[player].fe = myhosp->f1eb;
    D_00154a48[player].f10 = myhosp->f1ed;
    D_00154a48[player].f12 = myhosp->f1f1;
    D_00154a48[player].f14 = myhosp->unknownc7e;
    D_00154a48[player].f18 = myhosp->fc8a;
    D_00154a48[player].f1a = myhosp->fc90;
    D_00154a48[player].f1c = myhosp->fc92;
    D_00154a48[player].f1e = myhosp->fc96;
    D_00154a48[player].f1f = myhosp->fc97;
    D_00154a48[player].f20 = myhosp->fc98;
    D_00154a48[player].f22 = myhosp->fc9a;
    D_00154a48[player].f24 = myhosp->fc9c;
    D_00154a48[player].f26[1] = myhosp->fc9e[1];
    D_00154a48[player].f26[0] = myhosp->fc9e[0];
    D_00154a48[player].f30 = myhosp->fca8;
    D_00154a48[player].f32 = myhosp->fcaa;
    D_00154a48[player].f34 = myhosp->fcae;
    D_00154a48[player].f35 = myhosp->f1bb;
    D_00154a48[player].f39 = myhosp->fc82;
    D_00154a48[player].f3d = myhosp->fc86;
    LbNetwork_ChangeExchangeBuffer(YEAR_END_PACKETS, 0x45);
    LbNetwork_Exchange(YEAR_END_PACKETS + player * 0x45);
    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active) {
            allhosps[i].f2d = D_00154a48[i].f0;
            allhosps[i].fc8a = D_00154a48[i].f18;
            allhosps[i].f3d = D_00154a48[i].f8;
            allhosps[i].f35 = D_00154a48[i].f4;
            allhosps[i].f1e9 = D_00154a48[i].fc;
            allhosps[i].f1eb = D_00154a48[i].fe;
            allhosps[i].f1ed = D_00154a48[i].f10;
            allhosps[i].f1f1 = D_00154a48[i].f12;
            allhosps[i].unknownc7e = D_00154a48[i].f14;
            allhosps[i].fc90 = D_00154a48[i].f1a;
            allhosps[i].fc92 = D_00154a48[i].f1c;
            allhosps[i].fc96 = D_00154a48[i].f1e;
            allhosps[i].fc97 = D_00154a48[i].f1f;
            allhosps[i].fc98 = D_00154a48[i].f20;
            allhosps[i].fc9a = D_00154a48[i].f22;
            allhosps[i].fc9c = D_00154a48[i].f24;
            allhosps[i].fc9e[1] = D_00154a48[i].f26[1];
            allhosps[i].fc9e[0] = D_00154a48[i].f26[0];
            allhosps[i].fca8 = D_00154a48[i].f30;
            allhosps[i].fcaa = D_00154a48[i].f32;
            allhosps[i].fcae = D_00154a48[i].f34;
            allhosps[i].f1bb = D_00154a48[i].f35;
            allhosps[i].fc82 = D_00154a48[i].f39;
            allhosps[i].fc86 = D_00154a48[i].f3d;
        }
    }
}

extern "C" {
extern u8 D_0016053c[];
}

// FUNCTION: 0x000850f4
u8 setup_network(TbNetworkService service, void *data, u8 max_players)
{
    s32 res;

    login_control.f17 = 6;
    memset(D_0016053c, 0, 0xb4);
    memset(player_info, 0, 0x90);
    res = LbNetwork_Init(service, theme_hospital_guid, max_players, 0, 0, player_info, data);
    if (res == Lb_OK) {
        memset(login_control.players, 0, 0x540);
        return 1;
    }
    return 0;
}

void reset_message_text(void);

// FUNCTION: 0x00085198
s32 create_session(void)
{
    s32 len;
    s32 res;
    char name[28];

    reset_message_text();
    if (strcmp(myplayer.unknown0, D_001602a0[1]) == 0) {
        len = fn_000cd716(myplayer.unknown0);
        myplayer.unknown0[len] = 'A';
        myplayer.unknown0[len + 1] = 0;
    }
    sprintf(name, "%s", myplayer.unknown0);
    res = LbNetwork_Create(name, myplayer.unknown0, &player, D_0016053c);
    if (res == Lb_OK) {
        login_control.f17 = 5;
        memset(login_control.players, 0, 0x540);
    }
    return res;
}

// FUNCTION: 0x00085248
s32 join_session(void)
{
    s32 len;
    s32 res;
    s32 unused;

    unused = 0;
    reset_message_text();
    res = 0;
    if (hilighted_session >= 0 && hilighted_session < D_00160eb4) {
        if (strcmp(myplayer.unknown0, D_001602a0[1]) == 0) {
            res = fn_000cd716(myplayer.unknown0);
            myplayer.unknown0[res] = player_count + 'A';
            myplayer.unknown0[res + 1] = 0;
        }
        res = LbNetwork_Join(D_00160e34[hilighted_session], myplayer.unknown0, &player, D_0016053c);
        if (res == Lb_OK) {
            login_control.f17 = 5;
            reset_glass_map(0x80);
            reset_glass_map(0x8000);
            reset_glass_map(0x10000);
            set_glass_map(0x200);
        }
    }
    return res;
}

// FUNCTION: 0x00085334
void draw_small_centre_text(s16 x, s16 w, s16 y, char *text)
{
    LbTextSetWindow(x, 0, w, lbDisplay.unknown1c);
    set_font_from_cache(0x21);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, y, text);
}

// FUNCTION: 0x00085388
void draw_small_highlight_centre_text(s16 x, s16 w, s16 y, char *text)
{
    LbTextSetWindow(x, 0, w, lbDisplay.unknown1c);
    set_font_from_cache(0x22);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, y, text);
}

// FUNCTION: 0x000853dc
s16 character_height(void)
{
    return lbFontPtr ? lbFontPtr[34].height : 0;
}

// FUNCTION: 0x00085420
u8 send_message(void)
{
    if (BufferedInkey >= 0x4f && BufferedInkey <= 0x51)
        BufferedInkey -= 0x4d;
    else if (BufferedInkey >= 0x4b && BufferedInkey <= 0x4d)
        BufferedInkey -= 0x46;
    else if (BufferedInkey >= 0x47 && BufferedInkey <= 0x49)
        BufferedInkey -= 0x3f;
    else if (BufferedInkey == 0x52)
        BufferedInkey = 0xb;
    else if (BufferedInkey == 0xb5)
        BufferedInkey = 0x35;
    else if (BufferedInkey == 0x9c)
        BufferedInkey = 0x1c;
    else if (HospShift & 1)
        BufferedInkey |= 0x80;
    screen_packet[player].f7 = BufferedInkey;
    BufferedInkey = 0;
    if (screen_packet[player].f7)
        screen_packet[player].f5 = 7;
    return 0;
}

extern s32 LastClock;
extern s32 ClockTick;

extern "C" {
}

// FUNCTION: 0x00085538
void draw_current_message(char *text)
{
    s16 x;
    s16 y;
    s16 h;

    x = resvars.f3c_NetBoxp[120].x + 10;
    y = (resvars.f3c_NetBoxp[120].y + resvars.f3c_NetBoxp[120].h - (lbFontPtr ? lbFontPtr[66].height : 0)) / 2;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x21);
    h = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextDraw(x, y, text);
    ClockTick -= LastClock;
    LastClock = othervar.f8a2_s32;
    ClockTick += LastClock;
    if (BufferedInkey)
        ClockTick = 0;
    if (!(ClockTick & 0x40)) {
        x += LbTextStringWidth(text);
        LbTextDraw(x, y, "*");
    }
}

extern "C" {
s32 fn_00085660(s32 n);
s32 fn_000856c8(s32 n);
}

// FUNCTION: 0x00085660
s32 fn_00085660(s32 n)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        if (i != player) {
            if (D_001609c4[i].active) {
                n--;
                if (n == 0)
                    return i;
            }
        }
    }
    return -1;
}

// FUNCTION: 0x000856c8
s32 fn_000856c8(s32 n)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active) {
            n--;
            if (n == 0)
                return i;
        }
    }
    return -1;
}

#define NET_TEXT(i) ((char *)&net_text[i])

extern "C" {
void fn_000c93bf(char *dst, char *src);
}

u8 cheat_mode(char *text);

// FUNCTION: 0x00085724
void process_nettext_keys(u8 shift, u8 key)
{
    switch (key) {
    case 1:
        othervar.f4a = 0;
        break;
    case 0x3b:
        if (statvar.unknownfd >= 2)
            othervar.f4c = 1 << fn_00085660(1);
        break;
    case 0x3c:
        if (statvar.unknownfd >= 3)
            othervar.f4c = 1 << fn_00085660(2);
        break;
    case 0x3d:
        if (statvar.unknownfd >= 4)
            othervar.f4c = 1 << fn_00085660(3);
        break;
    case 0x3e:
        if (statvar.unknownfd == 1) {
            othervar.f4c = 0xff;
        } else {
            othervar.f4c = 1 << player;
            othervar.f4c = ~othervar.f4c;
        }
        break;
    case 0x1c:
    case 0x9c:
        if (statvar.unknownfd <= 1) {
            if (!cheat_mode(NET_TEXT(othervar.ff))) {
                othervar.f4a = 0;
                D_001549d0[player].type = 0x4b;
                D_001549d0[player].value = (u16)othervar.f4c;
            } else {
                othervar.f4a = 0;
            }
        } else {
            othervar.f4a = 0;
            D_001549d0[player].type = 0x4b;
            D_001549d0[player].value = (u16)othervar.f4c;
        }
        break;
    case 0x0e:
    case 0x53:
        D_001549d0[player].type = 0x4c;
        break;
    case 0x4b:
    case 0x4d:
        break;
    default:
        if (fn_000cd716(NET_TEXT(player)) < 0x32) {
            if (key < 0x7f) {
                D_001549d0[player].type = 0x4d;
                D_001549d0[player].value = (u16)(key & 0x7f);
                if (shift & 1)
                    D_001549d0[player].value |= 0x80;
            }
        }
        break;
    }
    BufferedInkey = 0;
}

extern u32 TimeStamp;
extern u32 TimeLapse;

// FUNCTION: 0x000859b4
void do_net_text(void)
{
    u8 h;
    s16 len;
    u16 saved;
    s32 k;
    Coord pager_text_recieved = {504, 70};
    Coord pager_text_entering = {40, 40};
    char buf[100];

    TimeLapse = othervar.f8a2_s32 - TimeStamp;
    TimeStamp = othervar.f8a2_s32;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    saved = lbDisplay.draw_flags;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    h = lbFontPtr ? lbFontPtr[34].height : 0;
    if (current_text.text[0]) {
        len = fn_000cd716(current_text.text);
        k = (lbFontPtr ? lbFontPtr[current_text.text[0] - 0x1f].width : 0) << 16;
        if (-othervar.f4d > k) {
            memmove(current_text.text, current_text.text + 1, len);
            othervar.f4d += k;
            len--;
        }
        k = 0;
        while (current_text.text[k] == ' ')
            k++;
        memmove(current_text.text, current_text.text + k, len - k + 1);
        othervar.f4d += k * (lbFontPtr ? lbFontPtr[1].width : 0) << 16;
        set_all_refresh_zones_in_window(0, pager_text_recieved.y, lbDisplay.graphics_screen_width, h + pager_text_recieved.y - 1, 3);
        LbTextDraw(othervar.f4f, pager_text_recieved.y, current_text.text);
        if (TimeLapse)
            othervar.f4d -= (TimeLapse * 0x23 << 16) / 1000;
    }
    if (othervar.f4a) {
        othervar.f4c &= (1 << statvar.unknownfd) - 1;
        if ((-othervar.f4c & othervar.f4c) == othervar.f4c) {
            k = 0;
            while (othervar.f4c >> (k + 1))
                k++;
            sprintf(buf, "%s:", &allhosps[k]);
        } else {
            sprintf(buf, "ALL:");
        }
        fn_000c93bf(buf, NET_TEXT(player));
        fn_000c93bf(buf, "  ");
        lbDisplay.draw_flags = 4;
        LbDrawBox(0, pager_text_entering.y - 1, lbDisplay.graphics_screen_width, h, color.unknown8);
        lbDisplay.draw_flags = 0;
        set_all_refresh_zones_in_window(0, pager_text_entering.y - 1, lbDisplay.graphics_screen_width, h + pager_text_entering.y + 1, 3);
        LbTextDraw(pager_text_entering.x, pager_text_entering.y, buf);
    }
    lbDisplay.draw_flags = saved;
}

// FUNCTION: 0x00085ce8
void net_text_display(u8 from)
{
    s16 len;
    s16 n;
    char buf[400];

    n = 0;
    len = fn_000cd716(current_text.text);
    set_font_from_cache(1);
    if (current_text.text[0]) {
        n = (lbDisplay.graphics_screen_width - othervar.f4f - LbTextStringWidth(current_text.text)) /
                (lbFontPtr ? lbFontPtr[1].width : 0) + 1;
        if (n < 4)
            n = 4;
        LbMemorySet(buf, ' ', n);
        buf[n] = 0;
        if (n + len < sizeof(current_text.text))
            fn_000c93bf(current_text.text, buf);
    } else {
        othervar.f4d = lbDisplay.graphics_screen_width << 16;
    }
    sprintf(buf, "%s:%s", &allhosps[from], NET_TEXT(from));
    if (fn_000cd716(buf) + len < sizeof(current_text.text))
        fn_000c93bf(current_text.text, buf);
    net_text[from].unknown0 = 0;
    othervar.f4b = 0xc8;
}

// FUNCTION: 0x00085e50
void net_text_keypress(u8 key, u8 who)
{
    u8 i;
    u8 shift;

    if (key > 0) {
        shift = key & 0x80;
        key &= 0x7f;
        for (i = 0; i < 0x31; i++) {
            if ((s8)NET_TEXT(who)[i] == 0) {
                if (shift || (key >= 'a' && key <= 'z'))
                    NET_TEXT(who)[i] = lbInkeyToAsciiShift[key];
                else
                    NET_TEXT(who)[i] = lbInkeyToAscii[key];
                NET_TEXT(who)[i + 1] = 0;
                break;
            }
        }
    }
}

// FUNCTION: 0x00085f3c
void net_text_delete(u8 who)
{
    u8 i;

    if (NET_TEXT(who)[0x30]) {
        NET_TEXT(who)[0x30] = 0;
    } else {
        for (i = 1; i < 0x31; i++) {
            if ((s8)NET_TEXT(who)[i] == 0) {
                NET_TEXT(who)[i - 1] = 0;
                break;
            }
        }
    }
}

// FUNCTION: 0x00085fc4
void network_button_quit(void)
{
    switch (login_control.f17) {
    case 5:
        screen_packet[player].f5 = 8;
        break;
    case 6:
        statvar.unknownfd = 1;
        player = 0;
        network.unknown2 = 0;
        display_mode = 2;
        mainmenu.unknown2 = 0;
        break;
    }
}

// FUNCTION: 0x00086040
void reset_message_text(void)
{
    s32 i;

    TopMessage = 0;
    NumMessages = 0;
    memset(login_control.players, 0, 0x540);
    for (i = 0; i < 9; i++)
        memset(NetMessageText[i], 0, 40);
}

s32 create_session(void);
s32 join_session(void);

// FUNCTION: 0x000860a4
s32 network_button_create(void)
{
    s32 res;

    res = 0;
    switch (login_control.f17) {
    case 5:
        break;
    case 6:
        res = create_session();
        if (res == 0) {
            set_glass_map(0x300);
            reset_glass_map(0x80);
            reset_glass_map(0x8000);
            reset_glass_map(0x10000);
        }
        break;
    }
    return res;
}

// FUNCTION: 0x00086130
void network_button_message(void)
{
    screen_packet[player].f7 = 0x1c;
    screen_packet[player].f5 = 7;
}

// FUNCTION: 0x00086164
s32 network_button_join(void)
{
    s32 res;

    res = 0;
    switch (login_control.f17) {
    case 5:
        break;
    case 6:
        res = join_session();
        break;
    }
    return res;
}

// FUNCTION: 0x000861c0
void init_network_players(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (D_001609c4[i].active)
            fn_000bbff0((char *)allhosps[i].unknown0, player_info[i].name);
        allhosps[i].f2a = 1;
    }
}

extern char current_line[];

extern "C" {
char *strstr(char *s, char *sub);
}

// FUNCTION: 0x00086240
void network_read_control(u8 n)
{
    s8 count;
    s32 len;
    char *buf;
    char path[144];
    char *p;
    char num[4];
    char title[32];
    char name[32];

    len = 0;
    buf = 0;
    if (install_type < 3) {
        sprintf(path, "%slevels/net%d.dat", cd_install_path, n);
        check_cd_available(path);
    } else {
        sprintf(path, "levels/net%d.dat", n);
    }
    len = LbFileLengthRnc(path);
    if (len == -1)
        return;
    buf = (char *)LbMemoryAlloc(len + 1);
    if (buf == 0)
        return;
    if (LbFileLoadAt(path, buf) == -1) {
        LbMemoryFree(buf);
        return;
    }
    buf[len] = 0xff;
    LbMemorySet(current_line, 0, 0x100);
    read_line_from_buf(0, 0, buf);
    count = 0;
    while (read_line_from_buf(current_line, 0x100, buf)) {
        p = strstr(current_line, "#");
        if (p) {
            LbMemorySet(num, 0, 3);
            LbMemorySet(title, 0, 0x20);
            LbMemorySet(name, 0, 0x20);
            fn_000c21e6(num, current_line + 1, 2);
            read_line_from_buf(current_line, 0x100, buf);
            sprintf(title, "%s", current_line);
            current_line[0] = '&';
            while (current_line[0] == '&') {
                read_line_from_buf(current_line, 0x100, buf);
                if (current_line[0] == '&') {
                    if ((s8)current_line[1] - '0' == myplayer.f21) {
                        sprintf(name, "%s", current_line + 2);
                        break;
                    }
                }
            }
            net_control.players[count].level = atoi(num);
            sprintf(net_control.players[count].title, "%s", title);
            sprintf(net_control.players[count].name, "%s", name);
            count++;
            if (count == 6)
                break;
        }
    }
    if (count == 0)
        net_control.f4 = -1;
    else
        net_control.f4 = count;
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// FUNCTION: 0x000864f4
u8 network_num_players(void)
{
    u8 n;

    n = 0;
    {
        u32 i;

        for (i = 0; i < 4; i++) {
            if (D_001609c4[i].active)
                n++;
        }
    }
    return n;
}

extern "C" {
}

// FUNCTION: 0x00086544
u8 cheat_mode(char *text)
{
    u8 i;
    u8 found;

    found = 0;
    for (i = 0; strcmp(cheat_text[i], "..") != 0; i++) {
        if (strcmp(cheat_text[i], text) == 0) {
            debug.unknown4 = debug.unknown8 = i;
            found = 1;
            switch (i) {
            case 0:
                cheat(0x20);
                break;
            case 1:
                cheat(0x40);
                break;
            case 2:
                cheat(0x80);
                break;
            case 3:
                cheat(0x100);
                break;
            case 4:
                cheat(0x200);
                break;
            case 5:
                break;
            case 8:
                cheat(0x400);
                break;
            case 9:
                cheat(0x800);
                break;
            case 10:
                cheat(0x1000);
                break;
            case 11:
                cheat(0x2000);
                break;
            case 12:
                cheat(0x4000);
                break;
            }
        }
    }
    return found;
}

extern u8 aborted_net;

extern "C" {
int LbScreenLock(void);
}

// FUNCTION: 0x000866cc
s32 network_service_function(void)
{
    u8 first;

    first = 1;
    while (network.unknown2 != 2 || first) {
        process_sound_heap();
        LbScreenUnlock();
        LbScreenSwap();
        while (LbScreenLock() != 1)
            ;
        get_input();
        draw_network_select();
        if (BufferedInkey == 1) {
            BufferedInkey = 0;
            play_sound(10, 0x42, -1, -1);
            return -7;
        }
        process_mouse_network_select();
        if (aborted_net) {
            aborted_net = 0;
            return -7;
        }
        first = 0;
    }
    return 1;
}

// FUNCTION: 0x00086788
void process_connecting_buttons(u8 n)
{
    aborted_net = 1;
}

// FUNCTION: 0x000867ac
s32 network_service_function_init(void)
{
    if (D_00160810[0] != 'I')
        sprintf(D_00160810, D_00160308[32]);
    return network_service_function();
}

// FUNCTION: 0x000867f0
s32 network_service_function_connect(void)
{
    if (D_00160810[0] != 'D')
        sprintf(D_00160810, D_00160308[33]);
    return network_service_function();
}

extern "C" {
}

// FUNCTION: 0x00086834
void network_lost_focus(s32 who)
{
    othervar.fa7b[who] = 2;
    if (display_mode != 0xd) {
        if (display_mode == 0)
            set_mouse_sprite(9);
        othervar.fa7f = display_mode;
        display_mode = 0xd;
    }
}

// FUNCTION: 0x00086890
void network_gained_focus(s32 who)
{
    s32 unused;

    unused = 0;
    othervar.fa7b[who] = 0;
    if (!has_any_player_lost_focus()) {
        display_mode = othervar.fa7f;
        set_mouse_sprite(othervar.f31);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    }
}

// FUNCTION: 0x000868f8
void draw_lost_focus_screen(void)
{
    s32 i;
    s32 x;
    s32 y;
    s32 count;
    s32 w;
    s32 h;
    s32 border;
    s32 gap;

    count = 0;
    w = 0;
    border = 0x28;
    gap = 10;
    set_font_from_cache(0x1d);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 4; i++) {
        if (othervar.fa7b[i]) {
            count++;
            LbTextStringWidth((char *)allhosps[i].unknown0) > w ? w = LbTextStringWidth((char *)allhosps[i].unknown0) : 0;
        }
    }
    x = (lbDisplay.unknown18 - w) / 2;
    h = count * (lbFontPtr ? lbFontPtr[34].height : 0) + (count - 1) * gap;
    y = (lbDisplay.unknown1c - h) / 2;
    LbDrawBox(x - border, y - border, border * 2 + w, border * 2 + h, color.unknown7);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x - border, y - border, border * 2 + w, border * 2 + h, color.unknown8);
    for (i = 0; i < 4; i++) {
        if (othervar.fa7b[i]) {
            LbTextDraw(x, y, (char *)allhosps[i].unknown0);
            y += gap + (lbFontPtr ? lbFontPtr[34].height : 0);
        }
    }
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
}

// FUNCTION: 0x00086b10
u8 has_any_player_lost_focus(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (othervar.fa7b[i] != 0)
            return 1;
    }
    return 0;
}

extern "C" {
u8 fn_00086b60(void);
}

// FUNCTION: 0x00086b60
u8 fn_00086b60(void)
{
    if (network_num_players() == 1) {
        close_irequesters();
        if (crequester.unknown2 != 0x63)
            close_crequester(crequester.unknown2);
        allhosps[player].f2b = 0;
        LbNetwork_Stop();
        quit_game = 1;
        level_number = 0xc;
        othervar.f72 = 6;
        winlose.unknown2 = othervar.f72;
        play_winlose();
        statvar.unknownfd = 1;
        player = 0;
        return 1;
    }
    return 0;
}
