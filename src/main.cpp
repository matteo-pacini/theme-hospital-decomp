#include "hosp.h"

extern "C" {

extern u8 new_game;
extern u8 record_replay, play_replay, replay_mode, replay_number, replay_loop;
extern View ScreenBoxes;
extern Point ScreenCoords[];
extern Button screenbuttons[];
extern u8 HiresBoxes[], HiresCoords[], window_list[], button_list[], screenlistpos[];
extern u8 hiresbuttons[], panel_popup_coord[], litter_pos[];
extern u8 hiresglass[];
extern int hireshelp[];
extern MemRegion save[];
extern TbSprite *tile, *end_tiles;
extern u8 *tile_data;
extern TbSprite *end_game_panel, *end_pulldv, *end_sprites, *end_watch;
extern TbSprite *money, *end_money;
extern u8 *game_panel_data, *pulldv_data, *sprite_data, *watch_data, *money_data;

void conv_string(void *dst, char *src, int mode);
void display_copyright(void);

// 0xdc zero bytes at the start of this module's _DATA. No code refers to them,
// so their type and use are unknown; they are in _DATA, where a string
// initialiser puts them (a zero initialiser puts them in _BSS).
char unreferenced_zero_buf[0xdc] = "";
}

u8 no_cd = 1;

// Program entry: sets up memory, defaults and HOSPITAL.CFG, applies the
// command-line switches (-L<n> starts level n as a new game, -N<name> sets the
// player name, among others), shows the copyright screen unless -Q, then
// runs the game. Returns the DOS exit code: 1 if the memory setup failed.
// FUNCTION: 0x00010010
int main(u16 argc, char **argv)
{
    u16 i;
    char *arg;

    if (LbMemorySetup() == -1) {
        hosp_dos_printf("Could not allocate %d Bytes\n", 4900000);
        return 1;
    }
    init_game_vars();
    read_config_file();
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (*arg == '-' || *arg == '/') {
            s16 c;

            arg++;
            while (c = (s8)*arg++) {
                switch (c) {
                case 'B':
                case 'b':
                    lib_strcpy(balance_id, arg);
                    goto next_arg;
                case 'C':
                case 'c':
                    myplayer.language = atoi(arg);
                    goto next_arg;
                case 'E':
                case 'e':
                    statvar.unused_option_e = 1;
                    break;
                case 'G':
                case 'g':
                    debug.save_slot = atoi(arg);
                    goto next_arg;
                case 'S':
                case 's':
                    debug.direct_start = 1;
                    level_number = 1;
                    goto next_arg;
                case 'L':
                case 'l':
                    level_number = atoi(arg);
                    new_game = 1;
                    debug.direct_start = 1;
                    goto next_arg;
                case 'N':
                case 'n':
                    conv_string(&myplayer, arg, 0);
                    goto next_arg;
                case 'P':
                case 'p':
                    play_replay = 1;
                    replay_number = atoi(arg);
                    goto next_arg;
                case 'Q':
                case 'q':
                    statvar.quiet = 1;
                    goto next_arg;
                case 'R':
                case 'r':
                    record_replay = 1;
                    replay_number = atoi(arg);
                    goto next_arg;
                default:
                    break;
                }
            }
        }
    next_arg:;
    }
    if (statvar.quiet == 0) {
        display_copyright();
    }
    if (setup_host() == 0) {
        game();
    }
    reset_host();
    flushall();
    LbPackHeap();
    LbMemoryReset();
    return 0;
}

// Resets the game, debug, player and award globals and points resvars at the
// high-resolution layout tables. Called once at startup, before
// read_config_file.
// FUNCTION: 0x00010330
void init_game_vars(void)
{
    int i;

    memset(&gamevar, 0, 0x178);
    memset(&othervar, 0, sizeof(othervar));
    memset(&debug, 0, 0x21c);
    memset(packet, 0, 0x3c);
    memset(&myplayer, 0, 0x5c);
    memset(all_awards, 0, 0x4e);
    memset(trophy, 0, 0x2d);
    LbMemorySet(&award_counts, 0, sizeof(award_counts));
    resvars.loaded_mode = SCREEN_MODE_13;
    resvars.tile_height = 0x20;
    resvars.tile_width = 0x40;
    resvars.boxes_u8p = HiresBoxes;
    resvars.screen_boxes = &ScreenBoxes;
    resvars.coords_u8p = HiresCoords;
    resvars.screen_coords = ScreenCoords;
    resvars.window_list_u8p = window_list;
    resvars.button_list = button_list;
    resvars.screen_list_pos = screenlistpos;
    resvars.buttons = hiresbuttons;
    resvars.screen_buttons = screenbuttons;
    resvars.help_hotspots_intp = hireshelp;
    resvars.glass_frames = hiresglass;
    resvars.req_filenames_names = hires_req_filenames;
    resvars.requester_buttons = hires_requester_buttons;
    resvars.requester_sprites = hires_requester_sprites;
    resvars.font_filenames_charpp = hires_font_filenames;
    resvars.panel_popup_coords = panel_popup_coord;
    resvars.advisor_min_segments = 3;
    resvars.advisor_max_segments = 0x20;
    resvars.litter_pos = litter_pos;
    resvars.game_files_state = 0;
    advisor.flags &= 0xfe;
    NRAdcounts.thirsty_plants = 0;
    NRAdcounts.plants = 0;
    NRAdcounts.bench_shortage = 0;
    othervar.last_built_room = 0;
    othervar.editing_room = 0;
    play_replay = record_replay = replay_mode = replay_number = 0;
    replay_loop = 0;
    statvar.num_players = 1;
    player = 0;
    statvar.quiet = 0;
    for (i = 0; save[i].size != 0; i++) {
        statvar.save_game_size += save[i].size;
    }
    othervar.unused_set_by_init_game_vars = 1;
    statvar.unused_set_by_init_game_vars = 0x40;
    advisor.forced_advice_id = 0;
    advisor.advice_id_u = 0;
    advisor.anim_state = 0;
    statvar.view_height = 0x1b0;
    initialise_shadows();
    set_default_player_details();
}

// Measures the CD's read speed by reading animation_table[ANIM_INTRO_SM2]'s file from
// cd_install_path, storing the result in othervar.cd_speed (2 if no buffer could
// be allocated).
// FUNCTION: 0x00010584
void get_cd_speed(void)
{
    void *buf;
    int count;
    char path[144];

    buf = LbMemoryAlloc(0x10000);
    if (buf != 0) {
        sprintf(path, "%s%s", cd_install_path, animation_table[ANIM_INTRO_SM2].name);
        count = LbFileLengthRnc(path) / 12;
        othervar.cd_speed = GetCDSpeed(path, buf, 0x10000, count);
        LbMemoryFree(buf);
    } else {
        othervar.cd_speed = 2;
    }
}

// Rebuilds blocks[] from blocks_backup and turns each entry's tile number into
// a pointer into tile[] (entries from 0x50 on skip 0x51 sprites), then sets up
// the tile sprites.
// FUNCTION: 0x00010618
void setup_blocks(void)
{
    u32 i = 0;

    reset_blocks();
    for (; blocks[i].height >= 0; i++) {
        if (i >= 0x50) {
            blocks[i].data = (u32)&tile[(u8)blocks[i].data + 0x51];
        } else {
            blocks[i].data = (u32)&tile[(u8)blocks[i].data];
        }
    }
    LbSpriteSetup(tile, end_tiles, tile_data);
}

// Sets up the map tiles and the panel, button, pull-down, game, watch and
// money sprite tables after their files are loaded.
// FUNCTION: 0x000106c0
void setup_game_sprites(void)
{
    setup_blocks();
    LbSpriteSetup(game_panel, end_game_panel, game_panel_data);
    LbSpriteSetup(button_sprite, end_button_sprites, button_sprite_data);
    LbSpriteSetup(pulldv, end_pulldv, pulldv_data);
    LbSpriteSetup(sprite, end_sprites, sprite_data);
    LbSpriteSetup(watch, end_watch, watch_data);
    LbSpriteSetup(money, end_money, money_data);
}

// Restores every blocks[] entry's data from blocks_backup (tile numbers, not
// pointers).
// FUNCTION: 0x00010760
void reset_blocks(void)
{
    int i;

    for (i = 0; blocks[i].height >= 0; i++) {
        blocks[i].data = blocks_backup[i];
    }
}
