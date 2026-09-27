#include "hosp.h"

extern "C" {

extern char D_00120004[];
extern char D_00120021[];
extern char D_0013f486[];
extern u8 D_001547e5;
extern u8 D_0015642e;
extern u8 D_0015f99f;
extern u8 D_001581ec, D_001581ed, D_001581ee, D_001581ef, D_001581f0;
extern u8 D_001553b8[];
extern u8 D_0015f79c[];
extern TbScreenMode D_0015f3f6;
extern u8 D_0015f3f7, D_0015f3f8, D_0015f3f9, D_0015f40a, D_0015f40b;
extern u8 *D_0015f40c, *D_0015f410, *D_0015f414, *D_0015f418, *D_0015f438;
extern u8 *D_0015f41c, *D_0015f428, *D_0015f3fa, *D_0015f402, *D_0015f406, *D_0015f42c;
extern u8 *D_0015f444;
extern int *D_0015f420;
extern int *D_0015f3fe;
extern View *D_0015f430;
extern View D_0014619c;
extern Point *D_0015f434;
extern Point D_00146c8c[];
extern Button *D_0015f43c;
extern Button D_00145378[];
extern u8 D_0013bb0c[], D_0013c0fc[], D_001361ac[], D_001310e4[], D_00145358[];
extern u8 D_0013c6a4[], D_00130288[], D_0013e5fc[], D_0014a620[], D_0014a9fe[], D_0012cc3c[];
extern u8 D_00147314[];
extern int D_0013c77c[];
extern int D_0013e634[];
extern u8 D_0015831b, D_0015831a;
extern u8 D_00155e03, D_00155d7d, D_001553ce, D_0015642c;
extern u16 D_001581fe, D_001581fc;
extern MemRegion D_001290d4[];
extern u8 D_00155e3f;
extern TbSprite *D_00153f44, *D_00153f48;
extern u8 *D_00153f40;
extern TbSprite *D_00153f88, *D_00153f94, *D_00153f24, *D_00153f54;
extern TbSprite *D_00153f5c, *D_00153f60;
extern u8 *D_00153f80, *D_00153f8c, *D_00153f1c, *D_00153f4c, *D_00153f58;

void fn_0001b7e0(void *dst, char *text, int flags);
void fn_0001bdf8(void);

}

// NONMATCHING: stack slots of i and the return value are swapped
// FUNCTION: 0x00010010
int main(u16 argc, char **argv)
{
    int i;
    char *arg;
    s16 c;

    if (LbMemorySetup() == -1) {
        hosp_dos_printf(D_00120004, 4900000);
        return 1;
    }
    init_game_vars();
    read_config_file();
    for (i = 1; (u16)i < argc; i++) {
        arg = argv[(u16)i];
        if (*arg == '-' || *arg == '/') {
            arg++;
            while (c = (s8)*arg++) {
                switch (c) {
                case 'B':
                case 'b':
                    LbDataLoadSetModifyFilenameFunction(balance_id, arg);
                    goto next_arg;
                case 'C':
                case 'c':
                    D_001547e5 = atoi(arg);
                    goto next_arg;
                case 'E':
                case 'e':
                    D_0015642e = 1;
                    break;
                case 'G':
                case 'g':
                    debug.unknown10a = atoi(arg);
                    goto next_arg;
                case 'S':
                case 's':
                    debug.unknown1fa = 1;
                    level_number = 1;
                    goto next_arg;
                case 'L':
                case 'l':
                    level_number = atoi(arg);
                    D_0015f99f = 1;
                    debug.unknown1fa = 1;
                    goto next_arg;
                case 'N':
                case 'n':
                    fn_0001b7e0(&myplayer, arg, 0);
                    goto next_arg;
                case 'P':
                case 'p':
                    D_001581ed = 1;
                    D_001581ef = atoi(arg);
                    goto next_arg;
                case 'Q':
                case 'q':
                    statvar.unknownfc = 1;
                    goto next_arg;
                case 'R':
                case 'r':
                    D_001581ec = 1;
                    D_001581ef = atoi(arg);
                    goto next_arg;
                default:
                    break;
                }
            }
        }
    next_arg:;
    }
    if (statvar.unknownfc == 0) {
        fn_0001bdf8();
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

// FUNCTION: 0x00010330
void init_game_vars(void)
{
    int i;

    memset(&gamevar, 0, 0x178);
    memset(D_001553b8, 0, 0xa96);
    memset(&debug, 0, 0x21c);
    memset(packet, 0, 0x3c);
    memset(&myplayer, 0, 0x5c);
    memset(all_awards, 0, 0x4e);
    memset(trophy, 0, 0x2d);
    LbMemorySet(D_0015f79c, 0, 0x155);
    D_0015f3f6 = SCREEN_MODE_13;
    D_0015f3f7 = 0x20;
    D_0015f3f8 = 0x40;
    D_0015f40c = D_0013bb0c;
    D_0015f430 = &D_0014619c;
    D_0015f410 = D_0013c0fc;
    D_0015f434 = D_00146c8c;
    D_0015f414 = D_001361ac;
    D_0015f418 = D_001310e4;
    D_0015f438 = D_00145358;
    D_0015f41c = D_0013c6a4;
    D_0015f43c = D_00145378;
    D_0015f420 = D_0013c77c;
    D_0015f444 = D_00147314;
    D_0015f3fa = D_0013e5fc;
    D_0015f402 = D_0014a620;
    D_0015f406 = D_0014a9fe;
    D_0015f3fe = D_0013e634;
    D_0015f428 = D_00130288;
    D_0015f40a = 3;
    D_0015f40b = 0x20;
    D_0015f42c = D_0012cc3c;
    D_0015f3f9 = 0;
    D_0015831b &= 0xfe;
    NRAdcounts.unknown2 = 0;
    NRAdcounts.unknown0 = 0;
    NRAdcounts.unknown6 = 0;
    D_00155e03 = 0;
    D_00155d7d = 0;
    D_001581ed = D_001581ec = D_001581ee = D_001581ef = 0;
    D_001581f0 = 0;
    statvar.unknownfd = 1;
    player = 0;
    statvar.unknownfc = 0;
    for (i = 0; D_001290d4[i].size != 0; i++) {
        statvar.unknown0 += D_001290d4[i].size;
    }
    D_001553ce = 1;
    D_0015642c = 0x40;
    D_001581fe = 0;
    D_001581fc = 0;
    D_0015831a = 0;
    statvar.unknown4 = 0x1b0;
    initialise_shadows();
    set_default_player_details();
}

// FUNCTION: 0x00010584
void get_cd_speed(void)
{
    void *buf;
    int count;
    char path[144];

    buf = LbMemoryAlloc(0x10000);
    if (buf != 0) {
        sprintf(path, D_00120021, cd_install_path, D_0013f486);
        count = LbFileLengthRnc(path) / 12;
        D_00155e3f = GetCDSpeed(path, buf, 0x10000, count);
        LbMemoryFree(buf);
    } else {
        D_00155e3f = 2;
    }
}

// FUNCTION: 0x00010618
void setup_blocks(void)
{
    u32 i = 0;

    reset_blocks();
    for (; blocks[i].height >= 0; i++) {
        if (i >= 0x50) {
            blocks[i].data = ((u8)blocks[i].data + 0x51) * 6 + (u32)D_00153f44;
        } else {
            blocks[i].data = (u8)blocks[i].data * 6 + (u32)D_00153f44;
        }
    }
    LbSpriteSetup(D_00153f44, D_00153f48, D_00153f40);
}

// FUNCTION: 0x000106c0
void setup_game_sprites(void)
{
    setup_blocks();
    LbSpriteSetup(game_panel, D_00153f88, D_00153f80);
    LbSpriteSetup(button_sprite, end_button_sprites, button_sprite_data);
    LbSpriteSetup(pulldv, D_00153f94, D_00153f8c);
    LbSpriteSetup(sprite, D_00153f24, D_00153f1c);
    LbSpriteSetup(watch, D_00153f54, D_00153f4c);
    LbSpriteSetup(D_00153f5c, D_00153f60, D_00153f58);
}

// FUNCTION: 0x00010760
void reset_blocks(void)
{
    int i;

    for (i = 0; blocks[i].height >= 0; i++) {
        blocks[i].data = blocks_backup[i];
    }
}
