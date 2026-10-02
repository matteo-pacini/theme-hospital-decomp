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
    s16 x1;
    s16 y1;
    // 0x8: Set to 0x27 here; add_glass_box copies its low byte to the zoom
    // box, where no code reads it.
    u16 unused_copied_to_zoom;
    u8 unknownA[4];
    u8 type;
    u8 button;
    u8 steps;
    u8 unknown11;
    u8 origin;
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

/*
 * The levels of a network game read from levels/net<n>.dat (net_control).
 */
struct NetControl {
    u8 unknown0;
    u8 unknown1[3];
    // 0x4: Number of levels read, -1 if none.
    s8 level_count;
    // 0x5: The levels: number, title and name.
    NetPlayer players[6];
};

struct LoginPlayer {
    u16 len;
    char text[0x28];
};

struct LoginControl {
    u8 unknown0[0x17];
    u8 state;
    u8 unknown18[7];
    u8 player_scroll;
    // 0x20: Sent as screen_packet's shared_value in each login exchange; set
    // from the host's on the first exchange and from any player sending
    // command 4. Nothing else reads it.
    u16 shared_value;
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
    // 0x4: Copied from the host's packet on the first login exchange; no code
    // sets or reads it otherwise.
    u8 unused_copied_from_host_0;
    u8 command;
    // 0x6: Set to 1 by process_login_exchange; no code reads it.
    u8 unused_set_by_process_login_exchange;
    u8 key;
    // 0x8: login_control.shared_value, exchanged between players.
    u16 shared_value;
    // 0xa: See unused_copied_from_host_0.
    u16 unused_copied_from_host_1;
    // 0xc: See unused_copied_from_host_0.
    u16 unused_copied_from_host_2;
};

extern "C" {
extern u8 host_player;
extern Event player_events[];
extern u8 *palette;
extern u8 session_index;
extern u8 first_login_exchange;
extern char **misc_text;
extern char **network_text;
extern u32 menu_turn;
extern u8 TopBaud;
extern u8 TopPort;
extern u8 TopSession;
extern s8 BookIndex;
extern char ConnectMessage[];
void draw_modem_selection_screen(void);
void draw_serial_selection_screen(void);
void draw_available_ports(u8 modem);
void draw_available_bauds(u8 modem);
void draw_active_session(u8 com_panel);
void draw_available_irqs(void);
#pragma aux draw_available_irqs "W?draw_available_irqs$n()v"

char *strchr(char *s, int c);
int strcmp(const char *a, const char *b);
void keybuffer_clear(void);
void use_hires_layout(void);
u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"
void choose_host_player(s8 me);
}

__tibcaGUID theme_hospital_guid = {0x1e8b2240, 0x11d037fe, 0xa0002b8d, 0xc96a00c9};
s8 BookHilight = -1;
char *upstr = "{";
char *downstr = "|";
u32 Bauds[] = {4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200};
u8 TopIRQ = 1;
long hilighted_session = -1;
extern "C" char hilighted_session_name[0x20] = "";
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
s8 add_glass_box(GlassBox *box, s32 id, s8 open_delay, s8 close_delay);
void add_glass_frame(GlassFrame *frames, s32 mask);
s16 glass_index(s32 mask);
void network_read_control(u8 n);
void set_glass_map(s32 mask);
void reset_glass_map(s32 mask);
void display_button_text(s32 mask);
void calculate_grey(u8 *palette);
void init_zoom_boxes(s32 mask);
u8 setup_balance_files(u8 net_game);
s32 next_map(void);
s8 draw_zoom_boxes(void);
void free_balance_files(void);
u8 start_session(void);
void swap_level_number(void);
void init_network_players(void);
long keybuffer_get(void);
u8 what_key_is_it_really(u8 key);
void draw_button_edges(s16 x, s16 y, s16 w, s16 h, u8 style);
void draw_glass_text(SlistButton *button, char *text);
void draw_glass_text_hilight(SlistButton *button, char *text);
void draw_glass_text_hilight_no_jump(SlistButton *button, char *text);
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
s16 button_index(s32 mask);
void add_number(char c);
void add_phone_number(char *number);

#define SLIST(n) ((SlistButton *)&resvars.screen_buttons[n])

// Reads the modem command strings (init, dial, hang-up, auto-answer, in that
// order, as "key=value" lines) from modem.ini, keeping the defaults for a
// missing file or a value over 80 characters.
// FUNCTION: 0x0007f9f0
void read_modem_ini_file(void)
{
    char *value;
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
    value = strchr(line, '=');
    if (value && lib_strlen(++value) <= 0x50)
        lib_strcpy(init_str, value);
    read_line_from_buf(line, 0x7f, buf);
    value = strchr(line, '=');
    if (value && lib_strlen(++value) <= 0x50)
        lib_strcpy(dial_str, value);
    read_line_from_buf(line, 0x7f, buf);
    value = strchr(line, '=');
    if (value && lib_strlen(++value) <= 0x50)
        lib_strcpy(hangup_str, value);
    read_line_from_buf(line, 0x7f, buf);
    value = strchr(line, '=');
    if (value && lib_strlen(++value) <= 0x50)
        lib_strcpy(auto_ans_str, value);
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// Sets host_player to the first active player other than `me` (0 if none);
// with me = -1, the first active player, who runs the session.
// FUNCTION: 0x0007fbb8
void choose_host_player(s8 me)
{
    s32 i;

    host_player = 0;
    for (i = 0; i < 4; i++) {
        if (player_info[i].active != 0 && me != i) {
            host_player = i;
            break;
        }
    }
}

// Shows the network screen's glass panels whose bits are in mask (0x10 modem,
// 0x20 serial, 0x40 IPX sessions, 0x80 connection type, 0x100 balance file,
// 0x200 chat, 0x400 dial pad, 0x800 connecting, 0x1000 level choice, ...),
// zooming them in. Showing a connection panel (0x10-0x40) first hides all
// others but 0x80 and 4; showing 0x1000 hides all others and lays out one
// button per level of net_control.
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
            box.x = resvars.screen_boxes_NetBoxp[126].x;
            box.y1 = (s16)resvars.screen_boxes_NetBoxp[126].h * net_control.level_count + 0x17;
            box.y = (lbDisplay.graphics_screen_height - box.y1) / 2;
            box.y1 += box.y;
            box.x1 = box.x + resvars.screen_boxes_NetBoxp[126].w;
            box.unused_copied_to_zoom = 0x27;
            box.type = 0;
            box.button = 0;
            box.steps = 7;
            box.origin = 1;
            add_glass_box(&box, 0x1000, 0, 0);
            box.x += resvars.screen_boxes_NetBoxp[126].y;
            box.y += 0xd;
            box.y1 = box.y + resvars.screen_boxes_NetBoxp[126].h - 3;
            box.x1 = box.x + resvars.screen_boxes_NetBoxp[126].w - resvars.screen_boxes_NetBoxp[126].y * 2;
            box.type = 1;
            box.button = 1;
            box.origin = 0;
            for (i = 0; i < net_control.level_count; i++) {
                resvars.screen_buttons[i + 0x8c].x = box.x;
                resvars.screen_buttons[i + 0x8c].y = box.y;
                resvars.screen_buttons[i + 0x8c].width = box.x1;
                resvars.screen_buttons[i + 0x8c].height = box.y1;
                add_glass_box(&box, 0x1000, i * 3, i * 3);
                box.y += resvars.screen_boxes_NetBoxp[126].h;
                box.y1 += resvars.screen_boxes_NetBoxp[126].h;
            }
            resvars.screen_buttons[i + 0x8c].x = 0;
            resvars.screen_buttons[i + 0x8c].y = 0;
            resvars.screen_buttons[i + 0x8c].width = 0;
            resvars.screen_buttons[i + 0x8c].height = 0;
            DesiredDisplay |= bit;
        }
        idx = glass_index(bit);
        if (idx) {
            if (!(DesiredDisplay & bit))
                add_glass_frame(&resvars.glass_frames_GlassFramep[idx], bit);
            DesiredDisplay |= bit;
        }
    }
    network.state = 3;
}

// Hides the network screen's glass panels whose bits are in mask.
// FUNCTION: 0x0007ff24
void reset_glass_map(s32 mask)
{
    DesiredDisplay &= ~mask;
    network.state = 3;
}

// Hides the panels in mask if they are shown, else shows them.
// FUNCTION: 0x0007ff50
void toggle_glass_map(s32 mask)
{
    if (CurrentDisplay & mask)
        reset_glass_map(mask);
    else
        set_glass_map(mask);
}

// Runs one frame of the network game screen's state machine
// (network.state): sets up the screen, zooms panels in and out, draws them,
// and once a session starts (state 1) loads the level and enters the game.
// FUNCTION: 0x0007ff88
void draw_network_select(void)
{
    s32 next;

    switch (network.state) {
    case 0:
        read_modem_ini_file();
        statvar.num_players = 1;
        player = 0;
        return_from_screen(9, 0, 1);
        load_ingame_pic(SCR_NETWORK_PREFS, 1, 0x19, 0, SCREEN_MODE_13);
        LbGhostTableGenerate(ppalette.data, 0x1e, "qdata/ghost1.dat");
        calculate_grey(ppalette.data);
        keybuffer_clear();
        BufferedInkey = 0;
        login_control.state = 6;
        lbDisplay.draw_colour = color.green;
        session_index = 0;
        init_zoom_boxes(0x84);
        setup_balance_files(1);
        network.state = 3;
        if (myplayer.serial_config[0] == 0) {
            myplayer.serial_config[0] = 4;
            myplayer.serial_config[1] = 3;
            myplayer.serial_config[2] = 4;
            myplayer.serial_config[3] = 3;
            myplayer.serial_config[4] = 2;
            myplayer.serial_config[5] = 2;
        }
        break;
    case 3:
        next = next_map();
        if (next == 0) {
            network.state = 2;
            if ((CurrentDisplay | DesiredDisplay) == 0) {
                statvar.num_players = 1;
                player = 0;
                network.state = 0;
                display_mode = DM_MAIN_MENU;
                mainmenu.state = MMS_ENTER;
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
        network.state = 0;
        swap_level_number();
        LbGhostTableGenerate(palette, 0x42, "qdata/ghost66.dat");
        use_hires_layout();
        LbPackHeap();
        load_balance((u16)level_number);
        LbPackHeap();
        load_game_files();
        debug.thing_count = 0;
        setup_new_level((u16)level_number, 0);
        init_network_players();
        memset(packet, 0, 0x3c);
        memset(screen_packet, 0, 0x38);
        quit_game = 2;
        display_mode = DM_GAME;
        debug.unused_set_by_draw_network_select = 0;
        set_font_from_cache(0);
        lbDisplay.rleft_button = lbDisplay.left_button = 0;
        login_control.state = 6;
        first_login_exchange = 0;
        clear_loading_screen(0);
        othervar.pending_palette = palette;
        current_palette = palette;
        set_mouse_mode(MM_NORMAL);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        break;
    case 2:
        BufferedInkey = keybuffer_get();
        BufferedInkey = what_key_is_it_really(BufferedInkey);
        copy_screen_to_wscreen(bscreen);
        draw_zoom_boxes();
        display_button_text(CurrentDisplay);
        if (login_control.state == 5) {
            if (host_player == player && !(CurrentDisplay & 0x100))
                set_glass_map(0x100);
        }
        break;
    default:
        network.state = 0;
        break;
    }
}

// Draws the text and buttons of each shown glass panel in mask, greying the
// scroll arrows at either end and flashing the button to press next.
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
                draw_glass_text_hilight(SLIST(89), network_text[7]);
            } else {
                draw_glass_text(SLIST(89), network_text[7]);
            }
            if (DesiredDisplay & 0x20) {
                draw_button_edges(SLIST(90)->x, SLIST(90)->y, SLIST(90)->x1 - SLIST(90)->x,
                                  SLIST(90)->y1 - SLIST(90)->y, 1);
                draw_glass_text_hilight(SLIST(90), network_text[8]);
            } else {
                draw_glass_text(SLIST(90), network_text[8]);
            }
            if (DesiredDisplay & 0x10) {
                draw_button_edges(SLIST(91)->x, SLIST(91)->y, SLIST(91)->x1 - SLIST(91)->x,
                                  SLIST(91)->y1 - SLIST(91)->y, 1);
                draw_glass_text_hilight(SLIST(91), network_text[9]);
            } else {
                draw_glass_text(SLIST(91), network_text[9]);
            }
            break;
        case 0x10:
            lbDisplay.draw_flags = 0;
            if (TopBaud == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(106), upstr);
            lbDisplay.draw_flags = 0;
            if (TopBaud >= 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(107), downstr);
            if (login_control.state == 5) {
                lbDisplay.draw_flags = 0;
                draw_glass_text(SLIST(104), network_text[11]);
                if (host_player != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(105), network_text[13]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(105), network_text[3]);
                } else if ((menu_turn & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(105), network_text[3]);
                } else {
                    draw_glass_text(SLIST(105), network_text[3]);
                }
                lbDisplay.draw_flags |= 4;
                draw_glass_text(SLIST(103), network_text[12]);
            } else {
                lbDisplay.draw_flags = 0;
                if (!BookCount && (menu_turn & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(103), network_text[12]);
                else
                    draw_glass_text(SLIST(103), network_text[12]);
                if ((menu_turn & 7) > 3 && !(DesiredDisplay & 0x400))
                    draw_glass_text_hilight_no_jump(SLIST(105), network_text[13]);
                else
                    draw_glass_text(SLIST(105), network_text[13]);
                if (BookHilight == -1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(104), network_text[10]);
                } else if ((menu_turn & 7) < 4 && !(DesiredDisplay & 0x400)) {
                    draw_glass_text_hilight_no_jump(SLIST(104), network_text[10]);
                } else {
                    draw_glass_text(SLIST(104), network_text[10]);
                }
            }
            lbDisplay.draw_flags = 0;
            if (login_control.state != 5) {
                if (!BookIndex)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.player_scroll == 0)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(101), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.state != 5) {
                if (BookIndex >= BookCount - 3)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.player_scroll >= player_count - 3)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(102), downstr);
            lbDisplay.draw_flags = 0;
            draw_modem_selection_screen();
            break;
        case 0x20:
            lbDisplay.draw_flags = 0;
            if (login_control.state != 5) {
                if (TopBaud == 0)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.player_scroll == 0)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(109), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.state != 5) {
                if (TopBaud >= 5)
                    lbDisplay.draw_flags |= 4;
            } else {
                if (login_control.player_scroll >= player_count - 3)
                    lbDisplay.draw_flags |= 4;
            }
            draw_glass_text(SLIST(110), downstr);
            lbDisplay.draw_flags = 0;
            if (!TopPort)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(113), upstr);
            lbDisplay.draw_flags = 0;
            if (TopPort)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(114), downstr);
            lbDisplay.draw_flags = 0;
            switch (login_control.state) {
            case 5:
                draw_glass_text(SLIST(112), network_text[4]);
                if (host_player != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(111), network_text[14]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(111), network_text[3]);
                } else if ((menu_turn & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(111), network_text[3]);
                } else {
                    draw_glass_text(SLIST(111), network_text[3]);
                }
                break;
            case 6:
                if (!myplayer.serial_config[4] && (int)myplayer.serial_config[5] < 0)
                    lbDisplay.draw_flags |= 4;
                if ((menu_turn & 7) > 3)
                    draw_glass_text_hilight_no_jump(SLIST(111), network_text[2]);
                else
                    draw_glass_text(SLIST(111), network_text[2]);
                if ((menu_turn & 7) < 4)
                    draw_glass_text_hilight_no_jump(SLIST(112), network_text[1]);
                else
                    draw_glass_text(SLIST(112), network_text[1]);
                break;
            }
            lbDisplay.draw_flags = 0;
            draw_serial_selection_screen();
            break;
        case 0x40:
            lbDisplay.draw_flags = 0;
            if (login_control.player_scroll == 0)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(120), upstr);
            lbDisplay.draw_flags = 0;
            if (login_control.player_scroll >= player_count - 3)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(121), downstr);
            lbDisplay.draw_flags = 0;
            if (TopSession == 0 || login_control.state == 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(118), upstr);
            lbDisplay.draw_flags = 0;
            if (TopSession >= session_index - 3 || login_control.state == 5)
                lbDisplay.draw_flags |= 4;
            draw_glass_text(SLIST(119), downstr);
            lbDisplay.draw_flags = 0;
            switch (login_control.state) {
            case 5:
                draw_glass_text(SLIST(116), network_text[4]);
                break;
            case 6:
                if ((menu_turn & 7) < 4)
                    draw_glass_text_hilight_no_jump(SLIST(116), network_text[1]);
                else
                    draw_glass_text(SLIST(116), network_text[1]);
                break;
            }
            switch (login_control.state) {
            case 5:
                if (host_player != player) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), network_text[2]);
                } else if (player_count <= 1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), network_text[3]);
                } else if ((menu_turn & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), network_text[3]);
                } else {
                    draw_glass_text(SLIST(117), network_text[3]);
                }
                break;
            case 6:
                if (hilighted_session == -1) {
                    lbDisplay.draw_flags |= 4;
                    draw_glass_text(SLIST(117), network_text[2]);
                } else if ((menu_turn & 7) > 3) {
                    draw_glass_text_hilight_no_jump(SLIST(117), network_text[2]);
                } else {
                    draw_glass_text(SLIST(117), network_text[2]);
                }
                break;
            }
            lbDisplay.draw_flags = 0;
            draw_network_game_selection_screen();
            break;
        case 0x100: {
            char buf[100];

            sprintf(buf, "%s %s", misc_text[30], balance_file_name);
            lbDisplay.draw_flags = 0;
            if (current_balance_file != current_balance_file->next) {
                draw_glass_text(SLIST(94), upstr);
                draw_glass_text(SLIST(95), downstr);
            }
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            set_font_from_cache(0x21);
            LbTextDraw((resvars.screen_boxes_NetBoxp[132].w - LbTextStringWidth(buf)) / 2 +
                           resvars.screen_boxes_NetBoxp[132].x,
                       resvars.screen_boxes_NetBoxp[132].y +
                           (resvars.screen_boxes_NetBoxp[132].h - (lbFontPtr ? lbFontPtr[66].height : 0)) / 2,
                       buf);
            break;
        }
        case 0x200:
            draw_current_message(login_control.players[player].text);
            draw_message_box();
            draw_glass_text(SLIST(97), network_text[6]);
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
            if ((menu_turn & 7) > 3)
                draw_glass_text_hilight_no_jump(SLIST(138), "+");
            else
                draw_glass_text(SLIST(138), "+");
            lbDisplay.draw_flags = 0;
            LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
            break;
        case 0x1000:
            for (i = 0; i < net_control.level_count; i++)
                draw_glass_text(SLIST(i + 0x8c), net_control.players[i].name);
            draw_level_selection_screen();
            break;
        case 8:
            draw_glass_text(SLIST(147), MessageText);
            draw_report_selection_screen();
            break;
        case 0x800:
            draw_small_centre_text(0x1b, 0x24a, 0x113, ConnectMessage);
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

// Draws three COM ports from TopPort in the serial (modem 0) or modem panel;
// clicking one selects it (bit in myplayer.serial_config[4]).
// FUNCTION: 0x00081284
void draw_available_ports(u8 modem)
{
    s32 idx;
    s16 y;
    s16 i;
    char buf[52];

    if (modem)
        idx = 0x82;
    else
        idx = 0x80;
    y = resvars.screen_boxes_NetBoxp[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, network_text[15], i + 1 + TopPort);
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.screen_boxes_NetBoxp[idx].x, y,
                         (s16)resvars.screen_boxes_NetBoxp[idx].w, character_height())) {
            if (lbDisplay.mleft_button) {
                myplayer.serial_config[4] = 1 << (i + TopPort);
                lbDisplay.left_button = 0;
            }
        }
        if (((s8)myplayer.serial_config[4] >> (i + TopPort)) == 1)
            draw_small_highlight_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w,
                                             y, buf);
        else
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w, y, buf);
    }
}

// Draws three baud rates from TopBaud; clicking one selects it
// (myplayer.serial_config[5], a Bauds index).
// FUNCTION: 0x00081408
void draw_available_bauds(u8 modem)
{
    s32 idx;
    s16 y;
    s16 i;
    char buf[52];

    if (modem)
        idx = 0x83;
    else
        idx = 0x81;
    y = resvars.screen_boxes_NetBoxp[idx].y + 4;
    for (i = 0; i < 3; i++, y += character_height()) {
        sprintf(buf, "%d", Bauds[i + TopBaud]);
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.screen_boxes_NetBoxp[idx].x, y,
                         (s16)resvars.screen_boxes_NetBoxp[idx].w, character_height())) {
            if (lbDisplay.mleft_button) {
                myplayer.serial_config[5] = TopBaud + i;
                lbDisplay.left_button = 0;
            }
        }
        if (myplayer.serial_config[5] == TopBaud + i)
            draw_small_highlight_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w,
                                             y, buf);
        else
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w, y, buf);
    }
}

// Draws four IRQs from TopIRQ for the selected COM port; clicking one sets
// that port's IRQ (myplayer.serial_config[port]).
// FUNCTION: 0x00081588
void draw_available_irqs(void)
{
    s16 y;
    s16 i;
    s32 bits;
    s32 idx;
    s32 n;
    char buf[52];

    if (CurrentDisplay & 0x18000) {
        n = 0;
        bits = (s8)myplayer.serial_config[4] >> 1;
        while (bits != 0) {
            n++;
            bits >>= 1;
        }
        idx = 0x85;
        y = resvars.screen_boxes_NetBoxp[idx].y + 10;
        for (i = 0; i < 4; i++, y += character_height()) {
            sprintf(buf, "IRQ %d", i + TopIRQ);
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.screen_boxes_NetBoxp[idx].x,
                             y, (s16)resvars.screen_boxes_NetBoxp[idx].w, character_height())) {
                if (lbDisplay.mleft_button) {
                    myplayer.serial_config[n] = TopIRQ + i;
                    lbDisplay.left_button = 0;
                }
            }
            if (myplayer.serial_config[n] == TopIRQ + i)
                draw_small_highlight_centre_text(resvars.screen_boxes_NetBoxp[idx].x,
                                                 resvars.screen_boxes_NetBoxp[idx].w, y, buf);
            else
                draw_small_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w, y,
                                       buf);
        }
    }
}

// Serial panel: the port, baud and IRQ lists while choosing, the players and
// the login exchange once connected; starts the game when it begins.
// FUNCTION: 0x0008172c
void draw_serial_selection_screen(void)
{
    switch (login_control.state) {
    case 5:
        draw_active_session(1);
        process_login_exchange();
        break;
    case 6:
        draw_available_ports(0);
        draw_available_bauds(0);
        draw_available_irqs();
        break;
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.num_players <= 1)
            break;
        network.state = 1;
        break;
    case 7:
    default:
        break;
    }
}

// Modem panel: as draw_serial_selection_screen, plus the phone book.
// FUNCTION: 0x000817c4
void draw_modem_selection_screen(void)
{
    switch (login_control.state) {
    case 5:
        draw_active_session(1);
        process_login_exchange();
        break;
    case 6:
        draw_available_ports(1);
        draw_available_bauds(1);
        draw_available_irqs();
        draw_phone_numbers();
        break;
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.num_players <= 1)
            break;
        network.state = 1;
        break;
    case 7:
    default:
        break;
    }
}

// Adds `number` to the phone book (15 entries; when full, overwrites the
// first) and highlights it. Ignores an empty string.
// FUNCTION: 0x0008186c
void add_phone_number(char *number)
{
    if (*number) {
        if (BookCount < 15) {
            BookHilight = BookCount;
            lib_memcpy(PhoneBook[BookCount++], number, 12);
        } else {
            BookHilight = 0;
            lib_memcpy(PhoneBook[0], number, 12);
        }
    }
}

// Appends digit c to the number being typed on the dial pad (12 at most).
// FUNCTION: 0x000818e8
void add_number(char c)
{
    s8 len;

    len = lib_strlen(PhoneNumber);
    if (len < 12) {
        if (BufferedInkey)
            play_sound(SNDSRC_GENERAL, SFX_ENTANAME, -1, -1);
        PhoneNumber[len++] = c;
        PhoneNumber[len] = 0;
    }
}

// Closes the message box and restores the panels shown before it, unless the
// game is starting.
// FUNCTION: 0x00081958
void process_report_buttons(u8 button)
{
    if (!(login_control.state == 9 || login_control.state == 8)) {
        set_glass_map(RestoreDisplay);
        reset_glass_map(8);
        network.state = 3;
    }
}

// Shows `message` in a box over the network screen, hiding the other panels
// until it is closed (process_report_buttons).
// FUNCTION: 0x000819a8
void report_message(char *message)
{
    GlassBox box;

    MessageText = message;
    set_font_from_cache(0x22);
    box.x1 = LbTextStringWidth(message) + resvars.screen_boxes_NetBoxp[126].y * 4;
    box.x = (lbDisplay.graphics_screen_width - box.x1) / 2;
    box.y1 = resvars.screen_boxes_NetBoxp[126].h + resvars.screen_boxes_NetBoxp[126].y * 2;
    box.y = (lbDisplay.graphics_screen_height - box.y1) / 2;
    box.y1 += box.y;
    box.x1 += box.x;
    box.unused_copied_to_zoom = 0x27;
    box.type = 0;
    box.button = 0;
    box.steps = 10;
    box.origin = 1;
    add_glass_box(&box, 8, 0, 0);
    box.x += resvars.screen_boxes_NetBoxp[126].y;
    box.y += resvars.screen_boxes_NetBoxp[126].y;
    box.x1 -= resvars.screen_boxes_NetBoxp[126].y;
    box.y1 -= resvars.screen_boxes_NetBoxp[126].y;
    SLIST(147)->x = box.x;
    SLIST(147)->y = box.y;
    SLIST(147)->x1 = box.x1;
    SLIST(147)->y1 = box.y1;
    box.type = 1;
    box.button = 1;
    box.steps = 10;
    box.origin = 0;
    add_glass_box(&box, 8, 0, 0);
    RestoreDisplay = DesiredDisplay;
    DesiredDisplay = 8;
    network.state = 3;
}

// Network screen keys: digits, Return, Escape and Backspace on the dial pad;
// Backspace deletes the highlighted phone book entry; Escape leaves; other
// keys go to the chat line.
// FUNCTION: 0x00081b38
void process_keys_network_select(void)
{
    u32 i;

    if (network.state == 2) {
        if (CurrentDisplay & 0x400) {
            switch (BufferedInkey) {
            case KC_1:
                add_number('1');
                BufferedInkey = 0;
                break;
            case KC_2:
                add_number('2');
                BufferedInkey = 0;
                break;
            case KC_3:
                add_number('3');
                BufferedInkey = 0;
                break;
            case KC_4:
                add_number('4');
                BufferedInkey = 0;
                break;
            case KC_5:
                add_number('5');
                BufferedInkey = 0;
                break;
            case KC_6:
                add_number('6');
                BufferedInkey = 0;
                break;
            case KC_7:
                add_number('7');
                BufferedInkey = 0;
                break;
            case KC_8:
                add_number('8');
                BufferedInkey = 0;
                break;
            case KC_9:
                add_number('9');
                BufferedInkey = 0;
                break;
            case KC_0:
                add_number('0');
                BufferedInkey = 0;
                break;
            case KC_RETURN:
            case KC_NUMPADENTER:
                BufferedInkey = 0;
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                reset_glass_map(0x400);
                add_phone_number(PhoneNumber);
                break;
            case KC_ESCAPE:
                BufferedInkey = 0;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                reset_glass_map(0x400);
                break;
            case KC_BACKSPACE:
            case KC_DECIMAL:
                BufferedInkey = 0;
                if (PhoneNumber[0]) {
                    play_sound(SNDSRC_GENERAL, SFX_DELANAME, -1, -1);
                    PhoneNumber[lib_strlen(PhoneNumber) - 1] = 0;
                }
                break;
            }
        }
        switch (BufferedInkey) {
        case KC_BACKSPACE:
        case KC_DECIMAL:
            if (BookHilight != -1) {
                play_sound(SNDSRC_GENERAL, SFX_DELANAME, -1, -1);
                for (i = BookHilight; BookCount - 1 > i; i++)
                    lib_memcpy(PhoneBook[i], PhoneBook[i + 1], 12);
                memset(PhoneBook[i], 0, 12);
                BookCount--;
                if (BookCount + 1 > 3 && BookIndex > BookCount - 3)
                    BookIndex = BookCount - 3;
                BookHilight = -1;
            }
            break;
        case KC_ESCAPE:
            if (!(CurrentDisplay & 0x200) || !login_control.players[player].len) {
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
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

// Network screen clicks: presses the button under the mouse in each shown
// panel and runs its callback.
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
        reqspr_draw((u16)resvars.screen_buttons[92].x, (u16)resvars.screen_buttons[92].y, &screen_sprites_tab_start[2]);
    if (network.state == 2) {
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
                if (resvars.screen_buttons[base + i].width == 0 || resvars.screen_buttons[base + i].height == 0)
                    break;
                x = resvars.screen_buttons[base + i].x;
                y = resvars.screen_buttons[base + i].y;
                if (lbDisplay.rleft_button) {
                    if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, y,
                                     resvars.screen_buttons[base + i].width - x + 1,
                                     resvars.screen_buttons[base + i].height - y + 1)) {
                        lbDisplay.rleft_button = 0;
                        if (resvars.screen_buttons[base + i].sprite)
                            reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                        else
                            draw_button_edges(x, y, resvars.screen_buttons[base + i].width - x,
                                              resvars.screen_buttons[base + i].height - y, 1);
                        if (resvars.screen_buttons[base + i].callback)
                            resvars.screen_buttons[base + i].callback(i);
                        break;
                    }
                }
                if (lbDisplay.mleft_button && over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, x, y,
                                                           resvars.screen_buttons[base + i].width - x + 1,
                                                           resvars.screen_buttons[base + i].height - y + 1)) {
                    if (resvars.screen_buttons[base + i].sprite)
                        reqspr_draw(x, y, &screen_sprites_tab_start[1]);
                    else
                        draw_button_edges(x, y, resvars.screen_buttons[base + i].width - x,
                                          resvars.screen_buttons[base + i].height - y, 1);
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
extern NetInitData serial_init_data;
// Modem session settings and results, passed to LbNetwork_Create and
// LbNetwork_Join (modemsp.cpp's ModemStartData).
struct TbModemDev {
    s32 timeout;
    s32 dial_timeout;
    char response[0x50];
    // 0x58: Number dialled.
    char number[0x50];
    // 0xa8: Outcome code, reported to the player when the connection fails.
    s32 result;
    s32 (*init_func)(void);
    s32 (*dial_func)(void);
};

extern TbModemDev modem_dev;
extern TbNetworkSessionNameEntry *session_pointers[];
extern s32 session_count;
void sessions_callback(TbNetworkCallbackData *data, void *arg);
#pragma aux (net_callback) sessions_callback;
}

extern char session_names[][32];

u8 setup_network(TbNetworkService service, void *data, u8 max_players);
s32 network_button_create(void);
s32 network_button_join(void);
void network_button_message(void);
void report_message(char *message);
void prev_balance_file(void);
void next_balance_file(void);
s32 network_service_function_init(void);
s32 network_service_function_connect(void);

// Connection type buttons: 0 IPX (starts the IPX service and lists the
// sessions), 1 serial, 2 modem, 3 back to the main menu.
// FUNCTION: 0x000822c0
void process_network_buttons(u8 button)
{
    TbNetworkService service;

    switch (button) {
    case 0:
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        if (CurrentDisplay & 0x40) {
            reset_glass_map(0x40);
        } else {
            service = NS_IPX;
            if (setup_network(service, 0, 4)) {
                if (!(CurrentDisplay & 0x40))
                    set_glass_map(0x40);
            } else {
                report_message(network_text[16]);
            }
        }
        break;
    case 1:
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        toggle_glass_map(0x20);
        if (DesiredDisplay & 0x20)
            set_glass_map(0x8000);
        else
            reset_glass_map(0x8000);
        break;
    case 2:
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        toggle_glass_map(0x10);
        if (DesiredDisplay & 0x10)
            set_glass_map(0x10000);
        else
            reset_glass_map(0x10000);
        break;
    case 3:
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        network_button_quit();
        break;
    }
}

// Level choice button: chooses net_control.players[button]'s level and
// starts the game.
// FUNCTION: 0x0008242c
void process_level_buttons(u8 button)
{
    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
    level_number = net_control.players[button].level;
    screen_packet[player].command = 2;
}

// IPX session panel buttons: 0 create a session (or leave one), 1 join the
// highlighted session (or, for the session's first player, go on to the
// level choice), 2-5 scroll the session and player lists.
// FUNCTION: 0x00082480
void process_ipx_buttons(u8 button)
{
    switch (button) {
    case 2:
        if (TopSession) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopSession--;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 3:
        if (TopSession < session_index - 3) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopSession++;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 0:
        switch (login_control.state) {
        case 5:
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
            network_button_quit();
            break;
        case 6:
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (network_button_create() == -1)
                report_message(network_text[27]);
            break;
        }
        break;
    case 4:
        if (login_control.player_scroll) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            login_control.player_scroll--;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 5:
        if (login_control.player_scroll < player_count - 3) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            login_control.player_scroll++;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 1:
        switch (login_control.state) {
        case 5:
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (host_player == player && player_count > 1)
                screen_packet[player].command = 9;
            break;
        case 6:
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (hilighted_session != -1) {
                if (network_button_join() == -1)
                    report_message(network_text[28]);
            }
            break;
        }
        break;
    }
}

// Chat buttons: 0 sends the line typed, 1/2 scroll the messages.
// FUNCTION: 0x00082740
void process_message_buttons(u8 button)
{
    switch (button) {
    case 0:
        network_button_message();
        break;
    case 1:
        if (TopMessage) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopMessage--;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 2:
        if (TopMessage < NumMessages - 3) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopMessage++;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    }
}

// Balance file buttons: 0 previous, 1 next.
// FUNCTION: 0x00082838
void process_balance_buttons(u8 button)
{
    switch (button) {
    case 0:
        if (current_balance_file != current_balance_file->next)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        prev_balance_file();
        break;
    case 1:
        if (current_balance_file != current_balance_file->prev)
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        next_balance_file();
        break;
    }
}

// IRQ list buttons: 0 scrolls up, 1 down (TopIRQ 0-4).
// FUNCTION: 0x000828d4
void process_irq_buttons(u8 button)
{
    switch (button) {
    case 0:
        if (TopIRQ) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopIRQ--;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 1:
        if (TopIRQ < 4) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopIRQ++;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    }
}

// Serial panel buttons: 0/1 scroll the baud or player list, 4/5 the port
// list, 2 joins and 3 creates a serial session on the chosen port, baud and
// IRQ. Once connected, 2 goes on to the level choice (first player only)
// and 3 leaves.
// FUNCTION: 0x000829a4
void process_serial_buttons(u8 button)
{
    TbNetworkService service;
    s8 bits;

    switch (button) {
    case 0:
        if (login_control.state != 5) {
            if (TopBaud) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                TopBaud--;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
        } else {
            if (login_control.player_scroll) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                login_control.player_scroll--;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
        }
        break;
    case 1:
        if (login_control.state != 5) {
            if (TopBaud < 5) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                TopBaud++;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
        } else {
            if (login_control.player_scroll < player_count - 3) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                login_control.player_scroll++;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
        }
        break;
    case 4:
        if (TopPort) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopPort = 0;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 5:
        if (!TopPort) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            TopPort = 1;
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        break;
    case 2:
    case 3:
        if (login_control.state != 5) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (myplayer.serial_config[4] && (int)myplayer.serial_config[5] >= 0) {
                service = NS_SERIAL;
                bits = (s8)myplayer.serial_config[4] >> 1;
                serial_init_data.port = 0;
                while (bits) {
                    bits >>= 1;
                    serial_init_data.port++;
                }
                serial_init_data.baud = Bauds[myplayer.serial_config[5]];
                serial_init_data.irq = myplayer.serial_config[serial_init_data.port];
                serial_init_data.init = init_str;
                serial_init_data.dial = dial_str;
                serial_init_data.hangup = hangup_str;
                serial_init_data.auto_ans = auto_ans_str;
                if (setup_network(service, &serial_init_data, 2)) {
                    if (button == 3) {
                        if (network_button_create() == -1) {
                            LbNetwork_Stop();
                            report_message(network_text[29]);
                        }
                    } else {
                        session_count = 0;
                        if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)sessions_callback, 0) == Lb_OK) {
                            hilighted_session = 0;
                            lib_strcpy(hilighted_session_name, session_names[0]);
                            if (network_button_join() == -1) {
                                LbNetwork_Stop();
                                report_message(network_text[31]);
                            }
                        }
                    }
                } else {
                    report_message(network_text[17]);
                }
            }
        } else {
            if (button == 3) {
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                network_button_quit();
            } else {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                if (host_player == player && player_count > 1)
                    screen_packet[player].command = 9;
            }
        }
        break;
    }
}

// Modem panel buttons: 0/1 scroll the phone book or player list, 5/6 the
// baud list, 2 opens the dial pad, 3 dials the highlighted number and 4
// waits for a call. Once connected, 4 goes on to the level choice (first
// player only) and 3 leaves. Ignored while the dial pad is open.
// FUNCTION: 0x00082da0
void process_modem_buttons(u8 button)
{
    if (!(CurrentDisplay & 0x400)) {
        s8 bits;
        TbNetworkService service;
        s32 res;

        switch (button) {
        case 0:
            if (login_control.state != 5) {
                if (BookIndex) {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    BookIndex--;
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            } else {
                if (login_control.player_scroll) {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    login_control.player_scroll--;
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            }
            break;
        case 1:
            if (login_control.state != 5) {
                if (BookIndex < BookCount - 3) {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    BookIndex++;
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            } else {
                if (login_control.player_scroll < player_count - 3) {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    login_control.player_scroll++;
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                }
            }
            break;
        case 5:
            if (TopBaud) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                TopBaud--;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            if (TopPort)
                TopPort = 0;
            break;
        case 6:
            if (TopBaud < 5) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                TopBaud++;
            } else {
                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            }
            if (!TopPort)
                TopPort = 1;
            break;
        case 2:
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
            if (login_control.state != 5) {
                if (!(CurrentDisplay & 0x400)) {
                    set_glass_map(0x400);
                    PhoneNumber[0] = 0;
                }
            }
            break;
        case 3:
        case 4:
            if (login_control.state != 5) {
                play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                if (button == 3 && BookHilight == -1)
                    break;
                reset_glass_map(0x400);
                if (myplayer.serial_config[4] && (int)myplayer.serial_config[5] >= 0) {
                    service = NS_MODEM;
                    bits = (s8)myplayer.serial_config[4] >> 1;
                    serial_init_data.port = 0;
                    while (bits) {
                        bits >>= 1;
                        serial_init_data.port++;
                    }
                    serial_init_data.baud = Bauds[myplayer.serial_config[5]];
                    serial_init_data.irq = myplayer.serial_config[serial_init_data.port];
                    serial_init_data.init = init_str;
                    serial_init_data.dial = dial_str;
                    serial_init_data.hangup = hangup_str;
                    serial_init_data.auto_ans = auto_ans_str;
                    if (setup_network(service, &serial_init_data, 2)) {
                        lib_strcpy(modem_dev.number, PhoneBook[BookHilight]);
                        if (button == 4) {
                            modem_dev.init_func = network_service_function_init;
                            RestoreDisplay = DesiredDisplay;
                            DesiredDisplay = 4;
                            set_glass_map(0x800);
                            ConnectMessage[0] = 0;
                            res = network_button_create();
                            reset_glass_map(0x800);
                            if (res == -1) {
                                switch (modem_dev.result) {
                                case 4:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[21]);
                                    break;
                                case 5:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[22]);
                                    break;
                                case 6:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[23]);
                                    break;
                                case 7:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[24]);
                                    break;
                                case -1:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[25]);
                                    break;
                                case -2:
                                    DesiredDisplay = RestoreDisplay;
                                    report_message(network_text[26]);
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
                            session_count = 0;
                            if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)sessions_callback, 0) == Lb_OK) {
                                hilighted_session = 0;
                                lib_strcpy(hilighted_session_name, session_names[0]);
                                modem_dev.init_func = network_service_function_init;
                                modem_dev.dial_func = network_service_function_connect;
                                RestoreDisplay = DesiredDisplay;
                                DesiredDisplay = 4;
                                set_glass_map(0x800);
                                ConnectMessage[0] = 0;
                                res = network_button_join();
                                reset_glass_map(0x800);
                                if (res == -1) {
                                    switch (modem_dev.result) {
                                    case 4:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[21]);
                                        break;
                                    case 5:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[22]);
                                        break;
                                    case 6:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[23]);
                                        break;
                                    case 7:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[24]);
                                        break;
                                    case -1:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[25]);
                                        break;
                                    case -2:
                                        DesiredDisplay = RestoreDisplay;
                                        report_message(network_text[26]);
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
                        report_message(network_text[18]);
                    }
                }
            } else {
                if (button == 3) {
                    play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                    network_button_quit();
                } else {
                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                    if (host_player == player && player_count > 1)
                        screen_packet[player].command = 9;
                }
            }
            break;
        }
    }
}

// Dial pad buttons: 0-8 type 1-9, 9 types 0, 10 adds the number to the
// phone book.
// FUNCTION: 0x00083554
void process_number_buttons(u8 button)
{
    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
    switch (button) {
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

// Draws three phone book entries from BookIndex (and the number being
// typed); clicking one highlights it (BookHilight).
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
    y = resvars.screen_boxes_NetBoxp[127].y + 4;
    for (i = start; i < 3 && i + BookIndex < BookCount; i++, y += character_height()) {
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.screen_boxes_NetBoxp[127].x, y,
                         (s16)resvars.screen_boxes_NetBoxp[127].w, character_height())) {
            if (lbDisplay.mleft_button) {
                BookHilight = i + BookIndex;
                lbDisplay.left_button = 0;
            }
        }
        if (i + BookIndex == BookHilight)
            draw_small_highlight_centre_text(resvars.screen_boxes_NetBoxp[127].x, resvars.screen_boxes_NetBoxp[127].w,
                                             y, PhoneBook[i + BookIndex]);
        else
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[127].x, resvars.screen_boxes_NetBoxp[127].w, y,
                                   PhoneBook[i + BookIndex]);
    }
    if (DesiredDisplay & 0x400) {
        sprintf(buf, "%s*", PhoneNumber);
        draw_small_centre_text(resvars.screen_boxes_NetBoxp[127].x, resvars.screen_boxes_NetBoxp[127].w, y, buf);
    }
}

struct SessionInfo;

extern SessionInfo t_session[];
extern char NetMessageText[][40];

extern "C" {
void draw_available_sessions(SessionInfo *sessions);
}

// IPX session panel: the players once connected, else the session list.
// Starts the game when it begins.
// FUNCTION: 0x0008385c
void draw_network_game_selection_screen(void)
{
    switch (login_control.state) {
    case 5:
        draw_active_session(0);
        process_login_exchange();
        break;
    case 6:
        draw_available_sessions(t_session);
        break;
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.num_players <= 1)
            break;
        network.state = 1;
        break;
    case 7:
    default:
        break;
    }
}

// Message box: runs the login exchange or starts the game, as the other
// panels.
// FUNCTION: 0x000838e8
void draw_report_selection_screen(void)
{
    switch (login_control.state) {
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.num_players <= 1)
            break;
        network.state = 1;
        break;
    }
}

// Level choice panel: runs the login exchange or starts the game, as the
// other panels.
// FUNCTION: 0x00083944
void draw_level_selection_screen(void)
{
    switch (login_control.state) {
    case 9:
        process_login_exchange();
        break;
    case 8:
        if (statvar.num_players <= 1)
            break;
        network.state = 1;
        break;
    }
}

// Draws three chat messages from TopMessage.
// FUNCTION: 0x000839a0
void draw_message_box(void)
{
    s16 y;
    s16 i;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1f);
    lbDisplay.draw_flags = 0;
    y = resvars.screen_boxes_NetBoxp[121].y;
    for (i = TopMessage; i - TopMessage < 3 && NetMessageText[i][0]; i++, y += character_height())
        draw_small_centre_text(resvars.screen_boxes_NetBoxp[121].x, resvars.screen_boxes_NetBoxp[121].w, y,
                               NetMessageText[i]);
}

void update_message_box(u8 key, u8 shift, u8 from);

// Exchanges each player's screen_packet and acts on their requests: chat
// keys, starting the game (all players), leaving the session, and opening the
// level choice (the first player) when the game is to start.
// FUNCTION: 0x00083a78
void process_login_exchange(void)
{
    TbNetworkService service;
    s16 i;
    s16 j;

    choose_host_player(-1);
    screen_packet[player].unused_set_by_process_login_exchange = 1;
    screen_packet[player].shared_value = login_control.shared_value;
    LbNetwork_ChangeExchangeBuffer(screen_packet, 0xe);
    LbNetwork_Exchange(&screen_packet[player]);
    if (!first_login_exchange) {
        screen_packet[player].unused_copied_from_host_0 = screen_packet[host_player].unused_copied_from_host_0;
        screen_packet[player].unused_copied_from_host_1 = screen_packet[host_player].unused_copied_from_host_1;
        screen_packet[player].unused_copied_from_host_2 = screen_packet[host_player].unused_copied_from_host_2;
        login_control.shared_value = screen_packet[player].shared_value = screen_packet[host_player].shared_value;
        first_login_exchange = 1;
    }
    for (i = 0; i < 4; i++) {
        switch (screen_packet[i].command) {
        case 1:
            break;
        case 5:
            break;
        case 4:
            login_control.shared_value = screen_packet[i].shared_value;
            break;
        case 6:
            break;
        case 7:
            update_message_box(screen_packet[i].key & 0x7f, screen_packet[i].key & 0x80, i);
            break;
        case 2:
            login_control.state = 8;
            statvar.num_players = network_num_players();
            LbNetwork_ChangeExchangeTimeout(0x1e);
            break;
        case 8:
            if (i == player || (!(DesiredDisplay & 0x40) && i == host_player)) {
                LbNetwork_Stop();
                login_control.state = 6;
                set_glass_map(0x80);
                reset_glass_map(0x300);
                for (j = 0; j < 4; j++)
                    screen_packet[j].command = 0;
                if (DesiredDisplay & 0x40) {
                    service = NS_IPX;
                    if (!setup_network(service, 0, 4)) {
                        reset_glass_map(0x40);
                        report_message(network_text[16]);
                    }
                }
                if (DesiredDisplay & 0x20)
                    set_glass_map(0x8000);
                if (DesiredDisplay & 0x10)
                    set_glass_map(0x10000);
            }
            choose_host_player(i);
            break;
        case 9:
            login_control.state = 9;
            if (host_player == player) {
                LbNetwork_EnableNewPlayers(0);
                set_glass_map(0x1000);
            } else {
                report_message(network_text[19]);
            }
            break;
        }
        screen_packet[i].command = 0;
    }
}

// Requests the start of the network game. Returns 0.
// FUNCTION: 0x00083d98
u8 start_session(void)
{
    screen_packet[player].command = 2;
    return 0;
}

extern "C" {
}

// Applies chat key `key` (scan code; shift is nonzero for upper case) from
// player `from` to their chat line: Escape clears, Return posts the line to
// the message list (9 kept), Backspace deletes, other keys append.
// FUNCTION: 0x00083dc4
void update_message_box(u8 key, u8 shift, u8 from)
{
    s32 i;
    s32 j;
    s8 c;
    char buf[64];

    switch (key) {
    case KC_ESCAPE:
        login_control.players[from].len = 0;
        login_control.players[from].text[0] = 0;
        if (from == player)
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    case KC_RETURN:
        if (login_control.players[from].len) {
            for (i = 0; i < 9; i++) {
                if (!NetMessageText[i][0])
                    break;
            }
            if (i == 9) {
                for (j = 0; j < 8; j++)
                    lib_memcpy(NetMessageText[j], NetMessageText[j + 1], 40);
                i--;
            }
            sprintf(NetMessageText[i], "%s:%s", &player_info[from], login_control.players[from].text);
            login_control.players[from].len = 0;
            login_control.players[from].text[0] = 0;
            if (from == player)
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
            if (NumMessages < 9)
                NumMessages++;
            if (NumMessages > TopMessage + 3)
                TopMessage = NumMessages - 3;
        } else {
            if (from == player)
                play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
        }
        break;
    case KC_BACKSPACE:
    case KC_DECIMAL:
        if (login_control.players[from].len > 0) {
            if (from == player)
                play_sound(SNDSRC_GENERAL, SFX_DELANAME, -1, -1);
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
                play_sound(SNDSRC_GENERAL, SFX_ENTANAME, -1, -1);
            if (login_control.players[from].len < 0x1e &&
                LbTextStringWidth(login_control.players[from].text) < resvars.screen_boxes_NetBoxp[120].w - 0x28) {
                sprintf(buf, "%s:%s", &player_info[from], login_control.players[from].text);
                if (LbTextStringWidth(buf) < resvars.screen_boxes_NetBoxp[121].w - 0x14) {
                    login_control.players[from].text[login_control.players[from].len++] = c;
                    login_control.players[from].text[login_control.players[from].len] = 0;
                }
            }
        }
        break;
    }
}

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
extern LevelPacket swap_level_packet[];
}

void draw_players_on_session(u32 session);
void balance_filename(char *name);

// Draws the names of the session's players from login_control.player_scroll in the
// modem/serial panel (com_panel set) or the IPX panel, and counts them in
// player_count.
// FUNCTION: 0x000841b8
void draw_active_session(u8 com_panel)
{
    s32 idx;
    s16 y;
    s16 i;
    s16 j;

    if (com_panel)
        idx = 0x7d;
    else
        idx = 0x7b;
    y = resvars.screen_boxes_NetBoxp[idx].y + 4;
    player_count = 0;
    for (i = 0; i < 4; i++) {
        if (player_info[i].active)
            player_count++;
    }
    if (player_count <= 3)
        login_control.player_scroll = 0;
    for (i = 0, j = login_control.player_scroll; i < 3 && j < 4; j++) {
        if (player_info[j].active) {
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[idx].x, resvars.screen_boxes_NetBoxp[idx].w, y,
                                   player_info[j].name);
            y += character_height();
            i++;
        }
    }
}

// LbNetwork_EnumerateSessions callback: appends the session's name and entry
// to the session list (session_count).
// FUNCTION: 0x000842d4
void sessions_callback(TbNetworkCallbackData *data, void *arg)
{
    TbNetworkSessionNameEntry *session;

    session = (TbNetworkSessionNameEntry *)data;
    lib_strcpy(session_names[session_count], session->name);
    session_pointers[session_count] = session;
    session_count++;
}

// Lists the sessions on the network sorted by name, keeps the highlight on
// the same session by name, draws three of them from TopSession with the
// highlighted session's players, and lets a click highlight another.
// FUNCTION: 0x00084324
void draw_available_sessions(SessionInfo *sessions)
{
    u8 swapped;
    u8 found;
    s16 y;
    TbNetworkSessionNameEntry *entry;
    s32 i;
    char name[32];

    found = 0;
    session_count = 0;
    if (LbNetwork_EnumerateSessions((TbNetworkCallbackFunc)sessions_callback, 0) != Lb_OK)
        return;
    swapped = 1;
    while (swapped) {
        swapped = 0;
        for (i = 0; session_count - 1 > i; i++) {
            if (strcmp(session_names[i], session_names[i + 1]) > 0) {
                lib_strcpy(name, session_names[i]);
                lib_strcpy(session_names[i], session_names[i + 1]);
                lib_strcpy(session_names[i + 1], name);
                entry = session_pointers[i];
                session_pointers[i] = session_pointers[i + 1];
                session_pointers[i + 1] = entry;
                swapped = 1;
                break;
            }
        }
    }
    if (session_count == 0) {
        hilighted_session = -1;
        hilighted_session_name[0] = 0;
    } else if (hilighted_session == -1) {
        hilighted_session = 0;
        lib_strcpy(hilighted_session_name, session_names[0]);
    }
    if (hilighted_session >= 0) {
        if (strcmp(hilighted_session_name, session_names[hilighted_session])) {
            for (i = 0; i < session_count; i++) {
                if (!strcmp(session_names[i], hilighted_session_name)) {
                    if (i < 0) {
                        hilighted_session = i;
                        hilighted_session_name[0] = 0;
                    } else {
                        hilighted_session = i;
                        lib_strcpy(hilighted_session_name, session_names[i]);
                    }
                    break;
                }
            }
            if (i == session_count) {
                hilighted_session = -1;
                hilighted_session_name[0] = 0;
            }
        }
    }
    y = resvars.screen_boxes_NetBoxp[125].y + 4;
    for (i = TopSession; TopSession + session_count > i && TopSession + 3 > i; i++, y += character_height()) {
        if (i == hilighted_session && !found) {
            draw_small_highlight_centre_text(resvars.screen_boxes_NetBoxp[125].x, resvars.screen_boxes_NetBoxp[125].w,
                                             y, session_names[i]);
            draw_players_on_session(i);
            found = 1;
        } else {
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[125].x, resvars.screen_boxes_NetBoxp[125].w, y,
                                   session_names[i]);
        }
    }
    y = resvars.screen_boxes_NetBoxp[125].y + 4;
    for (i = TopSession; TopSession + session_count > i && TopSession + 3 > i; i++, y += character_height()) {
        if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, (s16)resvars.screen_boxes_NetBoxp[125].x, y,
                         (s16)resvars.screen_boxes_NetBoxp[125].w, character_height())) {
            if (lbDisplay.mleft_button) {
                if (hilighted_session != i) {
                    memset(player_info, 0, 0x90);
                    memset(player_names, 0, 0x28);
                }
                if (i < 0) {
                    hilighted_session = i;
                    hilighted_session_name[0] = 0;
                } else {
                    hilighted_session = i;
                    lib_strcpy(hilighted_session_name, session_names[i]);
                }
                lbDisplay.mleft_button = lbDisplay.left_button = 0;
            }
            break;
        }
    }
}

// LbNetwork_EnumeratePlayers callback: appends the player's name to
// player_names (player_count).
// FUNCTION: 0x00084748
void __stdcall players_callback(TbNetworkCallbackData *data, void *arg)
{
    TbNetworkPlayerNameEntry *entry;

    entry = (TbNetworkPlayerNameEntry *)data;
    lib_strcpy(player_names[player_count], entry->name);
    player_count++;
}

// Draws the names of the players in session `session` (a session_pointers
// index), from login_control.player_scroll.
// FUNCTION: 0x00084784
void draw_players_on_session(u32 session)
{
    s16 y;
    u32 count;
    u32 i;

    player_count = 0;
    if (LbNetwork_EnumeratePlayers(session_pointers[session], players_callback, 0) != Lb_OK)
        return;
    if (player_count <= 3)
        login_control.player_scroll = 0;
    y = resvars.screen_boxes_NetBoxp[123].y + 4;
    for (count = 0, i = login_control.player_scroll; count < 3 && i < 4; i++) {
        if (player_names[i][0]) {
            draw_small_centre_text(resvars.screen_boxes_NetBoxp[123].x, resvars.screen_boxes_NetBoxp[123].w, y,
                                   player_names[i]);
            y += character_height();
            count++;
        }
    }
}

// Network game: makes every player use the session runner's level_number and
// balance file.
// FUNCTION: 0x00084864
void swap_level_number(void)
{
    if (statvar.num_players > 1) {
        swap_level_packet[player].level = level_number;
        lib_strcpy(swap_level_packet[player].balance, balance_id);
        LbNetwork_ChangeExchangeBuffer(swap_level_packet, 0xf);
        LbNetwork_Exchange(&swap_level_packet[player]);
        level_number = swap_level_packet[host_player].level;
        lib_strcpy(balance_id, swap_level_packet[host_player].balance);
        if ((s8)balance_id[0] != 0)
            balance_filename(balance_id);
    }
}

// Does nothing.
// FUNCTION: 0x00084924
void swap_month_end_packets(void)
{
}

// Network game: shares each player's result for the current criterion (fail:
// 1 failed, 0 passed) and sets othervar.net_losers[i] for each player who failed.
// If some passed and some failed, forces advice 2. When `active`, sets othervar.net_result_page to the message to show: 1 nobody passed,
// 2 this player failed, 3 this player passed.
// FUNCTION: 0x0008493c
void swap_criteria_packets(u8 fail, u8 active)
{
    u8 passed;
    u8 failed;
    u8 i;

    passed = 0;
    failed = 0;
    if (statvar.num_players > 1) {
        criteria_packet[player].result = fail;
        LbNetwork_ChangeExchangeBuffer(criteria_packet, 6);
        LbNetwork_Exchange(&criteria_packet[player]);
        for (i = 0; i < 4; i++) {
            othervar.net_losers[i] = 0;
            if (player_info[i].active) {
                if (criteria_packet[i].result == 0) {
                    passed++;
                } else if (criteria_packet[i].result == 1) {
                    failed++;
                    othervar.net_losers[i] = 1;
                }
            }
        }
        if (passed != 0) {
            if (failed)
                advisor.forced_advice_id = 2;
        }
        if (active) {
            if (passed == 0)
                othervar.net_result_page = 1;
            else if (fail)
                othervar.net_result_page = 2;
            else
                othervar.net_result_page = 3;
        }
    }
}

/*
 * A player's year-end figures exchanged in a network game
 * (year_packet[player]); swap_year_end_packets copies each field from and to
 * the same allhosps field.
 */
struct YearEndPacket {
    // 0x0: allhosps.score.
    s32 score;
    // 0x4: allhosps.balance.
    s32 balance;
    // 0x8: allhosps.loan.
    s32 loan;
    // 0xc: allhosps.cures.
    u16 cures;
    // 0xe: allhosps.deaths.
    u16 deaths;
    // 0x10: allhosps.visitors.
    u16 visitors;
    // 0x12: allhosps.area.
    u16 area;
    // 0x14: allhosps.reputation.
    s32 reputation;
    // 0x18: allhosps.avg_patient_happiness.
    s16 avg_patient_happiness;
    // 0x1a: allhosps.extinguisher_count.
    u16 extinguisher_count;
    // 0x1c: allhosps.radiator_count.
    u16 radiator_count;
    // 0x1e: allhosps.object_count.
    u8 object_count;
    // 0x1f: allhosps.machine_count.
    u8 machine_count;
    // 0x20: allhosps.mishaps.
    u16 mishaps;
    // 0x22: allhosps.amenity_count.
    u16 amenity_count;
    // 0x24: allhosps.staff_count.
    u16 staff_count;
    // 0x26: allhosps.staff_by_kind[0] and [1] (nurses, doctors).
    s16 staff_by_kind[2];
    u8 unknown2a[6];
    // 0x30: allhosps.leisure_count.
    u16 leisure_count;
    // 0x32: allhosps.litter_count.
    u16 litter_count;
    // 0x34: allhosps.rathole_count.
    u8 rathole_count;
    // 0x35: allhosps.hospital_value.
    s32 hospital_value;
    // 0x39: allhosps.rep_points.
    s32 rep_points;
    // 0x3d: allhosps.rep_events.
    s32 rep_events;
    u8 unknown41[4];
};

extern "C" {
extern Event old_packet[];
extern YearEndPacket year_packet[];
}

/* The exchanged packets start with a 4-byte header, which overlaps old_packet. */
#define YEAR_END_PACKETS ((u8 *)old_packet + 0x38)

// Network game: shares each hospital's year-end figures (balance, loan,
// value, statistics) so every machine has all players' allhosps entries.
// FUNCTION: 0x00084a68
void swap_year_end_packets(void)
{
    u8 i;

    if (statvar.num_players <= 1)
        return;
    year_packet[player].score = myhosp->score;
    year_packet[player].balance = myhosp->balance;
    year_packet[player].loan = myhosp->loan;
    year_packet[player].cures = myhosp->cures;
    year_packet[player].deaths = myhosp->deaths;
    year_packet[player].visitors = myhosp->visitors;
    year_packet[player].area = myhosp->area;
    year_packet[player].reputation = myhosp->reputation;
    year_packet[player].avg_patient_happiness = myhosp->avg_patient_happiness;
    year_packet[player].extinguisher_count = myhosp->extinguisher_count;
    year_packet[player].radiator_count = myhosp->radiator_count;
    year_packet[player].object_count = myhosp->object_count;
    year_packet[player].machine_count = myhosp->machine_count;
    year_packet[player].mishaps = myhosp->mishaps;
    year_packet[player].amenity_count = myhosp->amenity_count;
    year_packet[player].staff_count = myhosp->staff_count;
    year_packet[player].staff_by_kind[1] = myhosp->staff_by_kind[1];
    year_packet[player].staff_by_kind[0] = myhosp->staff_by_kind[0];
    year_packet[player].leisure_count = myhosp->leisure_count;
    year_packet[player].litter_count = myhosp->litter_count;
    year_packet[player].rathole_count = myhosp->rathole_count;
    year_packet[player].hospital_value = myhosp->hospital_value;
    year_packet[player].rep_points = myhosp->rep_points;
    year_packet[player].rep_events = myhosp->rep_events;
    LbNetwork_ChangeExchangeBuffer(YEAR_END_PACKETS, 0x45);
    LbNetwork_Exchange(YEAR_END_PACKETS + player * 0x45);
    for (i = 0; i < 4; i++) {
        if (player_info[i].active) {
            allhosps[i].score = year_packet[i].score;
            allhosps[i].avg_patient_happiness = year_packet[i].avg_patient_happiness;
            allhosps[i].loan = year_packet[i].loan;
            allhosps[i].balance = year_packet[i].balance;
            allhosps[i].cures = year_packet[i].cures;
            allhosps[i].deaths = year_packet[i].deaths;
            allhosps[i].visitors = year_packet[i].visitors;
            allhosps[i].area = year_packet[i].area;
            allhosps[i].reputation = year_packet[i].reputation;
            allhosps[i].extinguisher_count = year_packet[i].extinguisher_count;
            allhosps[i].radiator_count = year_packet[i].radiator_count;
            allhosps[i].object_count = year_packet[i].object_count;
            allhosps[i].machine_count = year_packet[i].machine_count;
            allhosps[i].mishaps = year_packet[i].mishaps;
            allhosps[i].amenity_count = year_packet[i].amenity_count;
            allhosps[i].staff_count = year_packet[i].staff_count;
            allhosps[i].staff_by_kind[1] = year_packet[i].staff_by_kind[1];
            allhosps[i].staff_by_kind[0] = year_packet[i].staff_by_kind[0];
            allhosps[i].leisure_count = year_packet[i].leisure_count;
            allhosps[i].litter_count = year_packet[i].litter_count;
            allhosps[i].rathole_count = year_packet[i].rathole_count;
            allhosps[i].hospital_value = year_packet[i].hospital_value;
            allhosps[i].rep_points = year_packet[i].rep_points;
            allhosps[i].rep_events = year_packet[i].rep_events;
        }
    }
}

// Starts network service `service` (data: the serial or modem settings, or 0)
// for up to max_players players. Returns 1 on success.
// FUNCTION: 0x000850f4
u8 setup_network(TbNetworkService service, void *data, u8 max_players)
{
    s32 res;

    login_control.state = 6;
    memset(&modem_dev, 0, 0xb4);
    memset(player_info, 0, 0x90);
    res = LbNetwork_Init(service, theme_hospital_guid, max_players, 0, 0, player_info, data);
    if (res == Lb_OK) {
        memset(login_control.players, 0, 0x540);
        return 1;
    }
    return 0;
}

void reset_message_text(void);

// Creates a session named after the player (a default name gets an "A"
// added). Returns the LbNetwork_Create result, 0 (Lb_OK) on success.
// FUNCTION: 0x00085198
s32 create_session(void)
{
    s32 len;
    s32 res;
    char name[28];

    reset_message_text();
    if (strcmp(myplayer.name_chars, misc_text[1]) == 0) {
        len = lib_strlen(myplayer.name_chars);
        myplayer.name_chars[len] = 'A';
        myplayer.name_chars[len + 1] = 0;
    }
    sprintf(name, "%s", myplayer.name_chars);
    res = LbNetwork_Create(name, myplayer.name_chars, &player, &modem_dev);
    if (res == Lb_OK) {
        login_control.state = 5;
        memset(login_control.players, 0, 0x540);
    }
    return res;
}

// Joins the highlighted session (a default name gets a letter added) and
// shows the chat panel. Returns the LbNetwork_Join result, 0 (Lb_OK) on
// success or if no session is highlighted.
// FUNCTION: 0x00085248
s32 join_session(void)
{
    s32 len;
    s32 res;
    s32 unused;

    unused = 0;
    reset_message_text();
    res = 0;
    if (hilighted_session >= 0 && hilighted_session < session_count) {
        if (strcmp(myplayer.name_chars, misc_text[1]) == 0) {
            res = lib_strlen(myplayer.name_chars);
            myplayer.name_chars[res] = player_count + 'A';
            myplayer.name_chars[res + 1] = 0;
        }
        res = LbNetwork_Join(session_pointers[hilighted_session], myplayer.name_chars, &player, &modem_dev);
        if (res == Lb_OK) {
            login_control.state = 5;
            reset_glass_map(0x80);
            reset_glass_map(0x8000);
            reset_glass_map(0x10000);
            set_glass_map(0x200);
        }
    }
    return res;
}

// Draws text centred in the column from x, w wide, at height y, in the small
// font.
// FUNCTION: 0x00085334
void draw_small_centre_text(s16 x, s16 w, s16 y, char *text)
{
    LbTextSetWindow(x, 0, w, lbDisplay.physical_screen_height);
    set_font_from_cache(0x21);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, y, text);
}

// As draw_small_centre_text, in the highlighted font.
// FUNCTION: 0x00085388
void draw_small_highlight_centre_text(s16 x, s16 w, s16 y, char *text)
{
    LbTextSetWindow(x, 0, w, lbDisplay.physical_screen_height);
    set_font_from_cache(0x22);
    lbDisplay.draw_flags = 0x100;
    LbTextDraw(0, y, text);
}

// Returns the current font's line height (the height of its 'A').
// FUNCTION: 0x000853dc
s16 character_height(void)
{
    return lbFontPtr ? lbFontPtr[34].height : 0;
}

// Sends the key in BufferedInkey (keypad keys mapped to the main keys, 0x80
// added for Shift) as this player's chat key, and clears it. Returns 0.
// FUNCTION: 0x00085420
u8 send_message(void)
{
    if (BufferedInkey >= 0x4f && BufferedInkey <= 0x51)
        BufferedInkey -= 0x4d;
    else if (BufferedInkey >= 0x4b && BufferedInkey <= 0x4d)
        BufferedInkey -= 0x46;
    else if (BufferedInkey >= 0x47 && BufferedInkey <= 0x49)
        BufferedInkey -= 0x3f;
    else if (BufferedInkey == KC_NUMPAD0)
        BufferedInkey = 0xb;
    else if (BufferedInkey == KC_DIVIDE)
        BufferedInkey = 0x35;
    else if (BufferedInkey == KC_NUMPADENTER)
        BufferedInkey = 0x1c;
    else if (HospShift & MOD_SHIFT)
        BufferedInkey |= 0x80;
    screen_packet[player].key = BufferedInkey;
    BufferedInkey = 0;
    if (screen_packet[player].key)
        screen_packet[player].command = 7;
    return 0;
}

extern s32 LastClock;
extern s32 ClockTick;

extern "C" {
}

// Draws the chat line being typed with a blinking cursor.
// FUNCTION: 0x00085538
void draw_current_message(char *text)
{
    s16 x;
    s16 y;
    s16 h;

    x = resvars.screen_boxes_NetBoxp[120].x + 10;
    y = (resvars.screen_boxes_NetBoxp[120].y + resvars.screen_boxes_NetBoxp[120].h -
         (lbFontPtr ? lbFontPtr[66].height : 0)) /
        2;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x21);
    h = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextDraw(x, y, text);
    ClockTick -= LastClock;
    LastClock = othervar.clock_ms_s;
    ClockTick += LastClock;
    if (BufferedInkey)
        ClockTick = 0;
    if (!(ClockTick & 0x40)) {
        x += LbTextStringWidth(text);
        LbTextDraw(x, y, "*");
    }
}

extern "C" {
s32 nth_other_active_player(s32 nth);
s32 nth_active_player(s32 nth);
}

// Returns the nth (1-based) active player other than this one, or -1.
// FUNCTION: 0x00085660
s32 nth_other_active_player(s32 nth)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        if (i != player) {
            if (player_info[i].active) {
                nth--;
                if (nth == 0)
                    return i;
            }
        }
    }
    return -1;
}

// Returns the nth (1-based) active player, or -1.
// FUNCTION: 0x000856c8
s32 nth_active_player(s32 nth)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        if (player_info[i].active) {
            nth--;
            if (nth == 0)
                return i;
        }
    }
    return -1;
}

#define NET_TEXT(i) ((char *)&net_text[i])

extern "C" {
void strcat(char *dst, char *src);
}

u8 cheat_mode(char *text);

// In-game message keys: F1-F3 address another player, F4 everyone, Return
// sends the message (or, in a single-player game, tries it as a cheat
// code), Escape cancels; other keys are typed via events.
// FUNCTION: 0x00085724
void process_nettext_keys(u8 shift, u8 key)
{
    switch (key) {
    case KC_ESCAPE:
        othervar.net_text_entering = 0;
        break;
    case KC_F1:
        if (statvar.num_players >= 2)
            othervar.net_text_to = 1 << nth_other_active_player(1);
        break;
    case KC_F2:
        if (statvar.num_players >= 3)
            othervar.net_text_to = 1 << nth_other_active_player(2);
        break;
    case KC_F3:
        if (statvar.num_players >= 4)
            othervar.net_text_to = 1 << nth_other_active_player(3);
        break;
    case KC_F4:
        if (statvar.num_players == 1) {
            othervar.net_text_to = 0xff;
        } else {
            othervar.net_text_to = 1 << player;
            othervar.net_text_to = ~othervar.net_text_to;
        }
        break;
    case KC_RETURN:
    case KC_NUMPADENTER:
        if (statvar.num_players <= 1) {
            if (!cheat_mode(NET_TEXT(othervar.cur_player))) {
                othervar.net_text_entering = 0;
                player_events[player].type = EV_NET_TEXT_DISPLAY;
                player_events[player].value = (u16)othervar.net_text_to;
            } else {
                othervar.net_text_entering = 0;
            }
        } else {
            othervar.net_text_entering = 0;
            player_events[player].type = EV_NET_TEXT_DISPLAY;
            player_events[player].value = (u16)othervar.net_text_to;
        }
        break;
    case KC_BACKSPACE:
    case KC_DECIMAL:
        player_events[player].type = EV_NET_TEXT_DELETE;
        break;
    case KC_NUMPAD4:
    case KC_NUMPAD6:
        break;
    default:
        if (lib_strlen(NET_TEXT(player)) < 0x32) {
            if (key < 0x7f) {
                player_events[player].type = EV_NET_TEXT_KEYPRESS;
                player_events[player].value = (u16)(key & 0x7f);
                if (shift & MOD_SHIFT)
                    player_events[player].value |= 0x80;
            }
        }
        break;
    }
    BufferedInkey = 0;
}

extern u32 TimeStamp;
extern u32 TimeLapse;

// Draws the scrolling message ticker and, while a message is being typed,
// the line being typed with its addressee.
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

    TimeLapse = othervar.clock_ms_s - TimeStamp;
    TimeStamp = othervar.clock_ms_s;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    saved = lbDisplay.draw_flags;
    lbDisplay.draw_flags = 0;
    set_font_from_cache(1);
    h = lbFontPtr ? lbFontPtr[34].height : 0;
    if (current_text.text[0]) {
        len = lib_strlen(current_text.text);
        k = (lbFontPtr ? lbFontPtr[current_text.text[0] - 0x1f].width : 0) << 16;
        if (-othervar.net_text_scroll > k) {
            memmove(current_text.text, current_text.text + 1, len);
            othervar.net_text_scroll += k;
            len--;
        }
        k = 0;
        while (current_text.text[k] == ' ')
            k++;
        memmove(current_text.text, current_text.text + k, len - k + 1);
        othervar.net_text_scroll += k * (lbFontPtr ? lbFontPtr[1].width : 0) << 16;
        set_all_refresh_zones_in_window(0, pager_text_recieved.y, lbDisplay.graphics_screen_width,
                                        h + pager_text_recieved.y - 1, 3);
        LbTextDraw(othervar.net_text_x, pager_text_recieved.y, current_text.text);
        if (TimeLapse)
            othervar.net_text_scroll -= (TimeLapse * 0x23 << 16) / 1000;
    }
    if (othervar.net_text_entering) {
        othervar.net_text_to &= (1 << statvar.num_players) - 1;
        if ((-othervar.net_text_to & othervar.net_text_to) == othervar.net_text_to) {
            k = 0;
            while (othervar.net_text_to >> (k + 1))
                k++;
            sprintf(buf, "%s:", &allhosps[k]);
        } else {
            sprintf(buf, "ALL:");
        }
        strcat(buf, NET_TEXT(player));
        strcat(buf, "  ");
        lbDisplay.draw_flags = 4;
        LbDrawBox(0, pager_text_entering.y - 1, lbDisplay.graphics_screen_width, h, color.black);
        lbDisplay.draw_flags = 0;
        set_all_refresh_zones_in_window(0, pager_text_entering.y - 1, lbDisplay.graphics_screen_width,
                                        h + pager_text_entering.y + 1, 3);
        LbTextDraw(pager_text_entering.x, pager_text_entering.y, buf);
    }
    lbDisplay.draw_flags = saved;
}

// Appends player `from`'s finished message to the scrolling ticker and
// clears it.
// FUNCTION: 0x00085ce8
void net_text_display(u8 from)
{
    s16 len;
    s16 n;
    char buf[400];

    n = 0;
    len = lib_strlen(current_text.text);
    set_font_from_cache(1);
    if (current_text.text[0]) {
        n = (lbDisplay.graphics_screen_width - othervar.net_text_x - LbTextStringWidth(current_text.text)) /
                (lbFontPtr ? lbFontPtr[1].width : 0) +
            1;
        if (n < 4)
            n = 4;
        LbMemorySet(buf, ' ', n);
        buf[n] = 0;
        if (n + len < sizeof(current_text.text))
            strcat(current_text.text, buf);
    } else {
        othervar.net_text_scroll = lbDisplay.graphics_screen_width << 16;
    }
    sprintf(buf, "%s:%s", &allhosps[from], NET_TEXT(from));
    if (lib_strlen(buf) + len < sizeof(current_text.text))
        strcat(current_text.text, buf);
    net_text[from].text = 0;
    othervar.unused_set_by_net_text_display = 0xc8;
}

// Appends key `key` (scan code, 0x80 set for Shift) to player `who`'s message
// being typed (49 characters at most).
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

// Deletes the last character of player `who`'s message being typed.
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

// Leaves the session, or the network screen when not in one.
// FUNCTION: 0x00085fc4
void network_button_quit(void)
{
    switch (login_control.state) {
    case 5:
        screen_packet[player].command = 8;
        break;
    case 6:
        statvar.num_players = 1;
        player = 0;
        network.state = 0;
        display_mode = DM_MAIN_MENU;
        mainmenu.state = MMS_ENTER;
        break;
    }
}

// Clears the chat messages and lines.
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

// Creates a session if not in one and shows its panels. Returns
// create_session's result (0 if already in a session).
// FUNCTION: 0x000860a4
s32 network_button_create(void)
{
    s32 res;

    res = 0;
    switch (login_control.state) {
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

// Sends Return as this player's chat key, posting the line typed.
// FUNCTION: 0x00086130
void network_button_message(void)
{
    screen_packet[player].key = 0x1c;
    screen_packet[player].command = 7;
}

// Joins the highlighted session if not in one. Returns join_session's result
// (0 if already in a session).
// FUNCTION: 0x00086164
s32 network_button_join(void)
{
    s32 res;

    res = 0;
    switch (login_control.state) {
    case 5:
        break;
    case 6:
        res = join_session();
        break;
    }
    return res;
}

// Names each active player's hospital after the player, and sets every
// hospital's player_type to 1.
// FUNCTION: 0x000861c0
void init_network_players(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (player_info[i].active)
            lib_strcpy((char *)allhosps[i].name, player_info[i].name);
        allhosps[i].player_type = 1;
    }
}

extern char current_line[];

extern "C" {
char *strstr(char *s1, char *s2);
}

// Reads levels/net<n>.dat, the levels for an n-player game, into net_control
// (up to six: level number, title and the name for language myplayer.language);
// net_control.level_count is their count, -1 if none.
// FUNCTION: 0x00086240
void network_read_control(u8 n)
{
    s8 count;
    s32 len;
    char *buf;
    char path[144];
    char *cursor;
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
        cursor = strstr(current_line, "#");
        if (cursor) {
            LbMemorySet(num, 0, 3);
            LbMemorySet(title, 0, 0x20);
            LbMemorySet(name, 0, 0x20);
            lib_memcpy(num, current_line + 1, 2);
            read_line_from_buf(current_line, 0x100, buf);
            sprintf(title, "%s", current_line);
            current_line[0] = '&';
            while (current_line[0] == '&') {
                read_line_from_buf(current_line, 0x100, buf);
                if (current_line[0] == '&') {
                    if ((s8)current_line[1] - '0' == myplayer.language) {
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
        net_control.level_count = -1;
    else
        net_control.level_count = count;
    read_line_from_buf(0, 1, 0);
    LbMemoryFree(buf);
}

// Returns the number of active players in the session.
// FUNCTION: 0x000864f4
u8 network_num_players(void)
{
    u8 count;

    count = 0;
    {
        u32 i;

        for (i = 0; i < 4; i++) {
            if (player_info[i].active)
                count++;
        }
    }
    return count;
}

extern "C" {
}

// Applies the cheat whose code is `text` (cheat_text). Returns 1 if text is
// a cheat code.
// FUNCTION: 0x00086544
u8 cheat_mode(char *text)
{
    u8 i;
    u8 found;

    found = 0;
    for (i = 0; strcmp(cheat_text[i], "..") != 0; i++) {
        if (strcmp(cheat_text[i], text) == 0) {
            debug.last_button = debug.last_action = i;
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

// Keeps the network screen running while the network layer waits for a
// connection. Returns -7 if the player cancels, 1 when the screen is ready.
// FUNCTION: 0x000866cc
s32 network_service_function(void)
{
    u8 first;

    first = 1;
    while (network.state != 2 || first) {
        process_sound_heap();
        LbScreenUnlock();
        LbScreenSwap();
        while (LbScreenLock() != 1)
            ;
        get_input();
        draw_network_select();
        if (BufferedInkey == KC_ESCAPE) {
            BufferedInkey = 0;
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
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

// Cancel button of the connecting box: aborts the connection attempt.
// FUNCTION: 0x00086788
void process_connecting_buttons(u8 button)
{
    aborted_net = 1;
}

// Waiting callback while the modem initialises: shows "initialising" and
// runs network_service_function.
// FUNCTION: 0x000867ac
s32 network_service_function_init(void)
{
    if (ConnectMessage[0] != 'I')
        sprintf(ConnectMessage, network_text[32]);
    return network_service_function();
}

// Waiting callback while the modem connects: shows "dialling" and runs
// network_service_function.
// FUNCTION: 0x000867f0
s32 network_service_function_connect(void)
{
    if (ConnectMessage[0] != 'D')
        sprintf(ConnectMessage, network_text[33]);
    return network_service_function();
}

extern "C" {
}

// Player `who` has switched away from the game: pauses with the lost focus
// screen (DM_LOST_FOCUS), remembering the previous display mode.
// FUNCTION: 0x00086834
void network_lost_focus(s32 who)
{
    othervar.lost_focus[who] = 2;
    if (display_mode != DM_LOST_FOCUS) {
        if (display_mode == DM_GAME)
            set_mouse_sprite(PTR_SLEEP);
        othervar.lost_focus_display_mode = display_mode;
        display_mode = DM_LOST_FOCUS;
    }
}

// Player `who` is back: resumes the previous display mode once no player is
// away.
// FUNCTION: 0x00086890
void network_gained_focus(s32 who)
{
    s32 unused;

    unused = 0;
    othervar.lost_focus[who] = 0;
    if (!has_any_player_lost_focus()) {
        display_mode = othervar.lost_focus_display_mode;
        set_mouse_sprite(othervar.prev_pointer_sprite);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
    }
}

// Draws the names of the players who are away in a box in the middle of the
// screen.
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
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 4; i++) {
        if (othervar.lost_focus[i]) {
            count++;
            LbTextStringWidth((char *)allhosps[i].name) > w ? w = LbTextStringWidth((char *)allhosps[i].name) : 0;
        }
    }
    x = (lbDisplay.physical_screen_width - w) / 2;
    h = count * (lbFontPtr ? lbFontPtr[34].height : 0) + (count - 1) * gap;
    y = (lbDisplay.physical_screen_height - h) / 2;
    LbDrawBox(x - border, y - border, border * 2 + w, border * 2 + h, color.white);
    lbDisplay.draw_flags = 0x10;
    LbDrawBox(x - border, y - border, border * 2 + w, border * 2 + h, color.black);
    for (i = 0; i < 4; i++) {
        if (othervar.lost_focus[i]) {
            LbTextDraw(x, y, (char *)allhosps[i].name);
            y += gap + (lbFontPtr ? lbFontPtr[34].height : 0);
        }
    }
    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
}

// Returns 1 if any player has switched away from the game.
// FUNCTION: 0x00086b10
u8 has_any_player_lost_focus(void)
{
    u8 i;

    for (i = 0; i < 4; i++) {
        if (othervar.lost_focus[i] != 0)
            return 1;
    }
    return 0;
}

extern "C" {
u8 end_game_if_alone(void);
}

// If this is the only player left in a network game, stops the network and
// ends the game (winlose state 6). Returns 1 if it did.
// FUNCTION: 0x00086b60
u8 end_game_if_alone(void)
{
    if (network_num_players() == 1) {
        close_irequesters();
        if (crequester.id != REQ_NONE)
            close_crequester(crequester.id);
        allhosps[player].playing = 0;
        LbNetwork_Stop();
        quit_game = 1;
        level_number = 0xc;
        othervar.winlose_result = 6;
        winlose.state = othervar.winlose_result;
        play_winlose();
        statvar.num_players = 1;
        player = 0;
        return 1;
    }
    return 0;
}
