#include "hosp.h"

struct ComputerBucket;

extern u8 *person_complex_group;
extern ComputerBucket (*comp_buckets)[12];

extern "C" {
extern u8 *palette;
extern u8 *lbVesaData;
extern u8 *sprite_data, *end_sprites, *pointer_data, *end_pointers, *tile_data;
extern u8 *tile, *end_tiles, *watch_data, *end_watch, *money_data;
extern u8 *money, *end_money, *end_frames, *end_elements, *game_panel_data;
extern u8 *end_game_panel, *pulldv_data, *end_pulldv, *staff_complex_group, *face_buffer;
extern u8 *end_allhosps, *end_comp_buckets, *end_text_buffer, *heapptr;

int __far harderr_handler(unsigned deverr, unsigned errcode, unsigned __far *devhdr);
void _harderr(int(__far *handler)(unsigned, unsigned, unsigned __far *));
char *strchr(const char *s, int c);
int LbFileFindFirst(char *path, void *ff, u32 attr);
int LbScreenLock(void);
void setup_lores_screen(void);
}

TbLoadFiles mcgaspritefiles[] = {
    {"datam/mspr-0.dat", (u8 **)&sprite_data, 0, 0, 0, 2},
    {"datam/mspr-0.tab", (u8 **)&sprite, (u8 **)&end_sprites, 0, 0, 2},
    {"datam/panel02m.dat", (u8 **)&game_panel_data, 0, 0, 0, 2},
    {"datam/panel02m.tab", (u8 **)&game_panel, (u8 **)&end_game_panel, 0, 0, 2},
    {"datam/pulldm.dat", (u8 **)&pulldv_data, 0, 0, 0, 2},
    {"datam/pulldm.tab", (u8 **)&pulldv, (u8 **)&end_pulldv, 0, 0, 2},
    {"datam/watch01m.dat", (u8 **)&watch_data, 0, 0, 0, 2},
    {"datam/watch01m.tab", (u8 **)&watch, (u8 **)&end_watch, 0, 0, 2},
    {"datam/money01m.dat", (u8 **)&money_data, 0, 0, 0, 2},
    {"datam/money01m.tab", (u8 **)&money, (u8 **)&end_money, 0, 0, 2},
    {0},
};

TbLoadFiles hiresspritefiles[] = {
    {"data/vspr-0.dat", (u8 **)&sprite_data, 0, 0, 0, 2},
    {"data/vspr-0.tab", (u8 **)&sprite, (u8 **)&end_sprites, 0, 0, 2},
    {"data/panel02v.dat", (u8 **)&game_panel_data, 0, 0, 0, 2},
    {"data/panel02v.tab", (u8 **)&game_panel, (u8 **)&end_game_panel, 0, 0, 2},
    {"data/pulldv.dat", (u8 **)&pulldv_data, 0, 0, 0, 2},
    {"data/pulldv.tab", (u8 **)&pulldv, (u8 **)&end_pulldv, 0, 0, 2},
    {"data/watch01v.dat", (u8 **)&watch_data, 0, 0, 0, 2},
    {"data/watch01v.tab", (u8 **)&watch, (u8 **)&end_watch, 0, 0, 2},
    {"data/money01v.dat", (u8 **)&money_data, 0, 0, 0, 2},
    {"data/money01v.tab", (u8 **)&money, (u8 **)&end_money, 0, 0, 2},
    {0},
};

TbLoadFiles hiresblockfiles[] = {
    {"data/vblk-0.dat", (u8 **)&tile_data, 0, 0, 0, 2},
    {"data/vblk-0.tab", (u8 **)&tile, (u8 **)&end_tiles, 0, 0, 2},
    {0},
};

TbLoadFiles loresblockfiles[] = {
    {"datam/mblk-0.dat", (u8 **)&tile_data, 0, 0, 0, 2},
    {"datam/mblk-0.tab", (u8 **)&tile, (u8 **)&end_tiles, 0, 0, 2},
    {0},
};

TbLoadFiles hiresgamepointerfiles[] = {
    {"data/mpointer.dat", (u8 **)&pointer_data, 0, 0, 0, 2},
    {"data/mpointer.tab", (u8 **)&pointer, (u8 **)&end_pointers, 0, 0, 2},
    {0},
};

char *hiresscreenpointerfiles[] = {"qdata/spointer.dat", "qdata/spointer.tab"};

TbLoadFiles lorespointerfiles[] = {
    {"datam/mpointer.dat", (u8 **)&pointer_data, 0, 0, 0, 2},
    {"datam/mpointer.tab", (u8 **)&pointer, (u8 **)&end_pointers, 0, 0, 2},
    {0},
};

TbLoadFiles hiresanifiles[] = {
    {"data/vfra-1.ani", (u8 **)&frame, (u8 **)&end_frames, 0, 0, 2},
    {"data/vstart-1.ani", (u8 **)&start, 0, 0, 0, 2},
    {"data/vele-1.ani", (u8 **)&element, (u8 **)&end_elements, 0, 0, 2},
    {"data/vlist-1.ani", (u8 **)&elelist, 0, 0, 0, 2},
    {0},
};

TbLoadFiles loresanifiles[] = {
    {"datam/mfra-1.ani", (u8 **)&frame, (u8 **)&end_frames, 0, 0, 2},
    {"datam/mstart-1.ani", (u8 **)&start, 0, 0, 0, 2},
    {"datam/mele-1.ani", (u8 **)&element, (u8 **)&end_elements, 0, 0, 2},
    {"datam/mlist-1.ani", (u8 **)&elelist, 0, 0, 0, 2},
    {0},
};

TbLoadFiles reqfiles[] = {
    {"qdata/req11v.dat", (u8 **)&button_sprite_data, (u8 **)&end_button_sprite_data, 0, 0, 1},
    {"qdata/req11v.tab", (u8 **)&button_sprite, (u8 **)&end_button_sprites, 0, 0, 1},
    {0},
};

TbLoadFiles mcgareqfiles[] = {
    {"datam/mutton01.dat", (u8 **)&button_sprite_data, (u8 **)&end_button_sprite_data, 0, 0, 2},
    {"datam/mutton01.tab", (u8 **)&button_sprite, (u8 **)&end_button_sprites, 0, 0, 2},
    {0},
};

const char *const hires_req_filenames[] = {
    "qdata/req00v",
    "qdata/req01v",
    "qdata/req02v",
    "qdata/req03v",
    "qdata/req04v",
    "qdata/req05v",
    "qdata/req06v",
    "",
    "",
    "qdata/req09v",
    "qdata/req10v",
    "qdata/req11v",
    "qdata/req12v",
    "qdata/req13v",
};

const char *const hires_faces = "qdata/face01v.dat";

const char *const lores_faces = "qdatam/face01m.dat";

char *hires_font_filenames[] = {
    "qdata/font00v",  "qdata/font01v",  "qdata/font02v",  "qdata/font74v",   "qdata/font04v",  "qdata/font05v",
    "qdata/font16v",  "qdata/font105v", "qdata/font106v", "qdata/font43v",   "qdata/font09v",  "qdata/font44v",
    "qdata/font60v",  "qdata/font24v",  "qdata/font25v",  "qdata/font26v",   "qdata/font45v",  "qdata/font46v",
    "qdata/font47v",  "qdata/font39v",  "qdata/font40v",  "qdata/font500v",  "qdata/font501v", "qdata/font35v",
    "qdata/font31v",  "qdata/font34v",  "qdata/font38v",  "qdata/font36v",   "qdata/font37v",  "qdata/font50v",
    "qdata/font51v",  "qdata/font19v",  "qdata/font18v",  "qdata/font112v",  "qdata/font113v", "qdata/font115v",
    "qdata/font100v", "qdata/font101v", "qdata/font102v", "qdata/font120v",  "qdata/font111v", "qdata/font110v",
    "qdata/font121v", "qdata/font122v", "qdata/font124v", "qdatam/font130m", "qdata/font502v", "qdata/font250v",
};

char *lores_font_filenames[] = {
    "qdatam/font130m", "qdatam/font132m", "qdatam/font131m", "qdatam/font130m", "qdatam/font130m", "qdatam/font100m",
    "qdatam/font101m", "qdata/font105v",  "qdata/font106v",  "qdata/font43v",   "qdata/font09v",   "qdata/font44v",
    "qdata/font60v",   "qdata/font24v",   "qdata/font25v",   "qdata/font26v",   "qdata/font45v",   "qdata/font46v",
    "qdata/font47v",   "qdata/font39v",   "qdata/font40v",   "qdata/font500v",  "qdata/font501v",  "qdata/font35v",
    "qdata/font31v",   "qdata/font34v",   "qdata/font38v",   "qdata/font36v",   "qdata/font37v",   "qdatam/font130m",
    "qdata/font51v",   "qdata/font19v",   "qdata/font18v",   "qdata/font112v",  "qdata/font113v",  "qdata/font115v",
    "qdata/font100v",  "qdata/font101v",  "qdata/font102v",  "qdata/font120v",  "qdata/font111v",  "qdata/font110v",
    "qdata/font121v",  "qdata/font122v",  "qdatam/font132m", "qdatam/font130m", "qdata/font502v",  "qdata/font250v",
};

TbLoadFiles palettefile[] = {
    {"qdata/load01v.pal", (u8 **)&palette, 0, 0, 0, 1},
    {0},
};

extern "C" {
char *palette_filename = "data/mpalette.dat";
}

TbLoadFiles langfile[] = {
    {"data/lang-0.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {"data/lang-1.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {"data/lang-2.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {"data/lang-3.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {"data/lang-4.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {"data/lang-5.dat", (u8 **)&text_buffer, (u8 **)&end_text_buffer, 0, 0, 2},
    {0},
};

TbLoadFiles complexfiles[] = {
    {"data/s_comp.lex", (u8 **)&staff_complex_group, 0, 0, 0, 2},
    {"data/p_comp.lex", (u8 **)&person_complex_group, 0, 0, 0, 2},
    {0},
};

TbLoadFiles screenfiles[] = {
    {"*W_SCREEN", (u8 **)&lbDisplay.wscreen, 0, 0x4b000},
    {"*B_SCREEN", (u8 **)&bscreen, 0, 0x4b000},
    {"*VESA", (u8 **)&lbVesaData, 0, 0x200, 1},
    {0},
};

TbLoadFiles memfiles[] = {
    {"*MAP", (u8 **)&map, (u8 **)&end_map, 0x20000},
    {"*ROOMS", (u8 **)&room_info, (u8 **)&end_room_info, 0xdac},
    {"*PAGERS", (u8 **)&pagers, (u8 **)&end_pagers, 0x14a},
    {"*HOSPITAL", (u8 **)&allhosps, (u8 **)&end_allhosps, 0x4964},
    {"*CONTROL", (u8 **)&mycontrol, 0, 0x651},
    {"*PIXBUFF", (u8 **)((u8 *)&pixBuf + 0x28), 0, 0x7e90},
    {"*LISTHEAD", (u8 **)&listhead, 0, 0x3a8},
    {"*PLAYERCALL", (u8 **)&playercall, (u8 **)&end_playercall, 0x258},
    {"*COMPUTERBUCKETS", (u8 **)&comp_buckets, (u8 **)&end_comp_buckets, 0x4b00},
    {"*FACEBUFFER", (u8 **)&face_buffer, 0, 0x130b},
    {0},
};

TbLoadFiles thingfiles[] = {
    {"*THING", (u8 **)&thing, (u8 **)&end_things, 0x40164},
    {0},
};

TbLoadFiles soundfiles[] = {
    {"*HEAPBUFFER1", (u8 **)&heapptr, 0, 0x70800},
    {"*HEAPBUFFER2", (u8 **)&heapptr, 0, 0x96000},
    {"*HEAPBUFFER3", (u8 **)&heapptr, 0, 0xc8000},
    {0},
};

TbLoadFiles nosoundfiles[] = {
    {"*HEAPBUFFER", (u8 **)&heapptr, 0, 0x32000},
    {0},
};

TbLoadFiles midifiles[] = {
    {"*MIDIBUFFER", (u8 **)&midi_buffer, 0, 0xc000},
    {0},
};

const char *hiresscreens[] = {
    "qdata/area01v.dat",  "qdata/town01v.dat",  "qdata/drugn01v.dat", "qdata/stat01v.dat",  "qdata/graph01v.dat",
    "qdata/pol01v.dat",   "qdata/res01v.dat",   "qdata/bank01v.dat",  "qdata/staff01v.dat", "qdata/score01v.dat",
    "qdata/bid01v.dat",   "qdata/pref01v.dat",  "qdata/pref01v.dat",  "qdata/main01m.dat",  "qdata/fame01v.dat",
    "qdata/award01v.dat", "qdata/award02v.dat", "qdata/pref01v.dat",  "qdata/title01v.dat", "qdata/fax01v.dat",
    "qdata/rep01v.dat",   "qdata/load01v.dat",  "qdata/brief01v.dat", "qdata/cred01v.dat",  "",
};

const char *screenpalettes[] = {
    "qdata/area01v.pal",  "qdata/town01v.pal",  "qdata/drugn01v.pal", "qdata/stat01v.pal",  "qdata/graph01v.pal",
    "qdata/pol01v.pal",   "qdata/res01v.pal",   "qdata/bank01v.pal",  "qdata/staff01v.pal", "qdata/score01v.pal",
    "qdata/bid01v.pal",   "qdata/pref01v.pal",  "qdata/pref01v.pal",  "qdata/main01m.pal",  "qdata/fame01v.pal",
    "qdata/award01v.pal", "qdata/award02v.pal", "qdata/pref01v.pal",  "qdata/title01v.pal", "qdata/fax01v.pal",
    "qdata/rep01v.pal",   "qdata/load01v.pal",  "qdata/brief01v.pal", "qdata/cred01v.pal",  "",
};

AnimTable animation_table[] = {
    {"anims/lose1.smk", 3, 0x140, 0xf0, 0x34, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/lose2.smk", 3, 0x140, 0xf0, 0x3b, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/lose3.smk", 3, 0x140, 0xf0, 0x5a, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/lose4.smk", 3, 0x140, 0xf0, 0x4f, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/lose5.smk", 3, 0x140, 0xf0, 0x89, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/lose6.smk", 3, 0x140, 0xf0, 0x94, 0xc, SMK_NO_SKIP | SMK_INTERLACE | SMK_DOUBLE_WIDTH},
    {"anims/winlevel.smk", 3, 0x140, 0xf0, 0x1e, 0xe, SMK_NO_SKIP | SMK_DOUBLE_HEIGHT | SMK_DOUBLE_WIDTH},
    {"anims/area01v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area02v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area02v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area03v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area04v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area05v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area06v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area07v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area08v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area09v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area10v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area11v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area12v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area13v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/area14v.smk", 3, 0x280, 0x1e0, 0x0, 0xe, SMK_NO_SOUND},
    {"anims/mmenu1-0.smk", 3, 0x140, 0xc8, 0x0, 0xe, 0x0},
    {"anims/mmenu2-0.smk", 3, 0x140, 0xc8, 0x0, 0xe, 0x0},
    {"anims/wingame.smk", 3, 0x140, 0xf0, 0x0, 0xe, SMK_NO_SKIP | SMK_DOUBLE_HEIGHT | SMK_DOUBLE_WIDTH},
    {"intro/intro.sm2", 4, 0x140, 0xc8, 0x0, 0xe, 0x0},
    {"intro/intro.sm4", 4, 0x140, 0xc8, 0x0, 0xe, 0x0},
    {"intro/attract.smk", 4, 0x280, 0x12c, 0x0, 0xe, 0x0},
};

const char *const hiresscreen_sprites_data[] = {
    "",
    "qdata/town02v.dat",
    "qdata/drugn02v.dat",
    "qdata/stat02v.dat",
    "qdata/graph02v.dat",
    "qdata/pol02v.dat",
    "qdata/res02v.dat",
    "qdata/bank02v.dat",
    "qdata/staff02v.dat",
    "",
    "qdata/bid02v.dat",
    "qdata/pref02v.dat",
    "qdata/pref02v.dat",
    "qdata/main02m.dat",
    "qdata/fame02v.dat",
    "qdata/award03v.dat",
    "qdata/award03v.dat",
    "qdata/pref02v.dat",
    "",
    "qdata/fax02v.dat",
    "qdata/rep02v.dat",
    "qdata/load02v.dat",
    "",
    "",
    "qdata/lettr02v.dat",
};

const char *const hiresscreen_sprites_tabs[] = {
    "",
    "qdata/town02v.tab",
    "qdata/drugn02v.tab",
    "qdata/stat02v.tab",
    "qdata/graph02v.tab",
    "qdata/pol02v.tab",
    "qdata/res02v.tab",
    "qdata/bank02v.tab",
    "qdata/staff02v.tab",
    "",
    "qdata/bid02v.tab",
    "qdata/pref02v.tab",
    "qdata/pref02v.tab",
    "qdata/main02m.tab",
    "qdata/fame02v.tab",
    "qdata/award03v.tab",
    "qdata/award03v.tab",
    "qdata/pref02v.tab",
    "",
    "qdata/fax02v.tab",
    "qdata/rep02v.tab",
    "qdata/load02v.tab",
    "",
    "",
    "qdata/lettr02v.tab",
};

TbLoadFiles mcgaratfestpanelfiles[] = {
    {"datam/panel04m.dat", (u8 **)&game_panel_data, 0, 0, 0, 2},
    {"datam/panel04m.tab", (u8 **)&game_panel, (u8 **)&end_game_panel, 0, 0, 2},
    {0},
};

TbLoadFiles hiresratfestpanelfiles[] = {
    {"data/panel04v.dat", (u8 **)&game_panel_data, 0, 0, 0, 2},
    {"data/panel04v.tab", (u8 **)&game_panel, (u8 **)&end_game_panel, 0, 0, 2},
    {0},
};

extern "C" {
char *cd_required_text[] = {
    "Theme Hospital requires the CDROM to play!",
    "Un CD-ROM requis pour jouer \205 Theme Hospital",
    "Sie ben\232tigen die CD-ROM, um Theme Hospital zu spielen!",
    "Theme Hospital necessita del CD-ROM per funzionare!",
    "\255Theme Hospital requiere el CDROM para funcionar!",
    "Theme Hospital kr\204ver cd-rom-skivan f\224r att fungera!",
};
}

// DOS critical error handler: always answers 3 (fail the call), so a missing
// CD reports an error instead of prompting.
// FUNCTION: 0x0009b590
extern "C" int __far harderr_handler(unsigned deverr, unsigned errcode, unsigned __far *devhdr)
{
    return 3;
}

// Checks that file path (or, with wildcards, a match for it) can be read; if
// not, shows the "insert the CD" screen in the language set and waits until it
// appears (Escape quits the game), then restores the screen mode and palette.
// Paths starting with '*' are not checked.
// FUNCTION: 0x0009b5b8
void check_cd_available(char *path)
{
    s32 probe;
    s32 handle;
    char *text;
    s32 unused;
    u8 had_screen;
    TbScreenMode mode;
    u8 was_lores;
    u8 find[0xe4];
    char fname[0x80];

    had_screen = lbDisplay.wscreen ? (u8)1 : (u8)0;
    mode = resvars.screen_mode;
    was_lores = othervar.use_lores_assets;
    unused = 0;
    if (*path == '*')
        return;
    _harderr(harderr_handler);
    handle = LbFileOpen(path, 2);
    if (handle != -1) {
        if (LbFileRead(handle, &probe, 4) != -1) {
            LbFileClose(handle);
            return;
        }
    }
    if (strchr(path, '*')) {
        if (LbFileFindFirst(path, find, 0x21) != -1)
            return;
    }
    if (had_screen)
        LbScreenUnlock();
    if (resvars.screen_mode != SCREEN_MODE_13 || othervar.use_lores_assets)
        setup_any_screen(palette, SCREEN_MODE_13);
    LbFileLoadAt((char *)screenpalettes[SCR_LOADING], palette);
    while (LbScreenLock() != 1)
        ;
    LbFileLoadAt((char *)hiresscreens[SCR_LOADING], lbDisplay.wscreen);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0x2f);
    text = cd_required_text[myplayer.language];
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    LbTextDraw((lbDisplay.physical_screen_width - LbTextStringWidth(text)) / 2, 0x1a4, text);
    if (lbKeyOn[KC_F] && lbKeyOn[KC_LALT])
        LbTextDraw(0, 0x1c2, path);
    LbScreenUnlock();
    LbMouseChangeSprite(0);
    LbScreenSwap();
    LbPaletteSet(palette);
    while (!LbFileExists(path)) {
        if (lbKeyOn[KC_ESCAPE]) {
            reset_host();
            exit(1);
        }
    }
    if (resvars.screen_mode != mode)
        setup_any_screen(palette, mode);
    if (was_lores)
        setup_lores_screen();
    if (had_screen) {
        while (LbScreenLock() != 1)
            ;
    }
    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
    othervar.full_redraw = 1;
    if (install_type < 2) {
        sprintf(fname, "%s%s", cd_install_path, palette_filename);
        check_cd_available(fname);
        LbFileLoadAt(fname, palette);
    } else {
        LbFileLoadAt(palette_filename, palette);
    }
}
