#include "hosp.h"

struct TbIffPalette {
    u32 unknown0;
    u8 data[0x300];
};

struct PicLengths {
    s32 palette;
    s32 pointers;
    s32 sprites;
    s32 sprite_tab;
    s32 map;
};

extern const char *hiresscreens[];
extern const char *screenpalettes[];
extern TbIffPalette ppalette;

extern "C" {
extern TbSprite *D_00153f18;
extern u8 *D_00153f10;
extern u8 *D_00153f04;
extern TbSprite *D_00153f0c;
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 *D_00153f34;
extern u8 D_00156910;
extern PicLengths D_0017526c;
extern char *D_0013e2b0;
extern char *D_0013e2b4;
extern char *D_0013f534[];
extern char *D_0013f598[];
extern char D_00121ba8[];
extern char D_00121ba9[];
extern char D_00121bbc[];
extern char D_00121bcd[];
extern char D_00121bce[];

int fn_000bed50(void);
u8 *fn_00022b60(TbCacheUsedBlock *block);
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
int strcmp(const char *a, const char *b);
}

void fade_out_menu(void);
void load_screen(const u8 idx);

// FUNCTION: 0x000b73a0
void return_from_screen(const u8 mouse_mode, const u8 in_game, const u8 sprites)
{
    u8 active;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if ((resvars.f1 == SCREEN_MODE_1 && in_game) || resvars.f0 == SCREEN_MODE_1) {
        if (active)
            LbIsActive();
        if (in_game)
            AIL_stop_sequence(GetSongHandle());
        if (!change_res(SCREEN_MODE_1)) {
            reset_host();
            exit(1);
        }
        if (in_game && othervar.fa4f)
            AIL_resume_sequence(GetSongHandle());
        while (fn_000bed50() != 1) {
        }
    }
    othervar.fa65 = 0;
    if (sprites)
        LbSpriteReset(screen_sprites_tab_start, D_00153f18, D_00153f10);
    LbMouseChangeSprite(0);
    StopAllSamples();
    if (othervar.fa06)
        cache.Free(othervar.fa06);
    othervar.f9f9 = 0;
    hosp_fade_palette(0, 5, Lb_PALETTE_FADE_CLOSED);
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    LbMouseSetup(0);
    set_mouse_mode(mouse_mode);
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    if (in_game) {
        if (resvars.f1 == SCREEN_MODE_1)
            setup_menus();
        D_001290d0 = 0xff;
        D_001290d1 = 0xff;
        D_001290d2 = 0xff;
        othervar.f1f = D_00153f34;
        display_mode = 0;
        othervar.f10 = 1;
        D_00156910 = 1;
        set_font_from_cache(1);
        close_irequesters();
        window.unknown2eb = 0;
        draw_it();
        if (crequester.unknown2 == 5) {
            switch (crequester.unknown0) {
            case 0:
                set_mouse_mode(1);
                break;
            case 2:
            case 4:
                set_mouse_mode(2);
                break;
            case 6:
            case 10: {
                s32 obj;

                obj = next_object_to_build();
                if (obj)
                    init_place_object(obj, gamevar.unknown26, gamevar.unknown2f, 0);
                break;
            }
            case 8:
                set_mouse_mode(0xb);
                break;
            case 1:
            case 3:
            case 5:
            case 7:
            case 9:
            default:
                break;
            }
        }
        if (crequester.unknown2 != 0x63)
            fn_0001bc94(crequester.unknown2, 0);
    }
}

// FUNCTION: 0x000b7648
void load_ingame_pic(const u8 idx, const u8 fade, const u8 mouse_mode, const u8 map, TbScreenMode mode)
{
    PicLengths len = D_0017526c;
    u8 active;
    char name[144];
    u8 *p;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if (resvars.f0 != mode) {
        if (active)
            LbIsActive();
        AIL_stop_sequence(GetSongHandle());
        if (!change_res(mode)) {
            reset_host();
            exit(1);
        }
        if (othervar.fa4f)
            AIL_resume_sequence(GetSongHandle());
        while (fn_000bed50() != 1) {
        }
    }
    len.palette = LbFileLengthRnc(D_0013e2b0);
    len.pointers = LbFileLengthRnc(D_0013e2b4);
    if (strcmp(D_0013f534[idx], D_00121ba8)) {
        len.sprites = LbFileLengthRnc(D_0013f534[idx]);
        len.sprite_tab = LbFileLengthRnc(D_0013f598[idx]);
    }
    if (map) {
        if (install_type < 3) {
            sprintf(name, D_00121ba9, cd_install_path, map);
            check_cd_available(name);
        } else {
            sprintf(name, D_00121bbc, map);
        }
        len.map += LbFileLengthRnc(name);
    }
    clear_sound_heap();
    othervar.fa06 = cache.Allocate(len.palette + len.pointers + len.sprites + len.sprite_tab + len.map + 0x40, 0x63);
    if (othervar.fa06)
        p = fn_00022b60(othervar.fa06);
    else
        return;
    if (fade)
        fade_out_menu();
    if (strcmp(hiresscreens[idx], D_00121bcd)) {
        u8 active2;

        active2 = lbDisplay.unknown4 ? (u8)1 : (u8)0;
        if (!active2) {
            while (fn_000bed50() != 1) {
            }
        }
        load_screen(idx);
        copy_screen_to_wscreen(bscreen);
        LbIsActive();
        LbScreenSwap();
        if (active2) {
            while (fn_000bed50() != 1) {
            }
        }
        othervar.fa65 = 1;
    }
    if (LbFileLoadAt(D_0013e2b0, p) == len.palette) {
        D_00153f04 = p;
        p += len.palette;
        othervar.fa65 = 1;
    } else {
        return;
    }
    if (LbFileLoadAt(D_0013e2b4, p) == len.pointers) {
        screen_pointers_tab_start = (TbSprite *)p;
        p += len.pointers;
        D_00153f0c = (TbSprite *)p;
        LbSpriteSetup(screen_pointers_tab_start, D_00153f0c, D_00153f04);
    } else {
        return;
    }
    if (strcmp(D_0013f534[idx], D_00121bce)) {
        if (LbFileLoadAt(D_0013f598[idx], p) == len.sprite_tab) {
            screen_sprites_tab_start = (TbSprite *)p;
            p += len.sprite_tab;
            D_00153f18 = (TbSprite *)p;
            if (LbFileLoadAt(D_0013f534[idx], p) == len.sprites) {
                D_00153f10 = p;
                LbSpriteSetup(screen_sprites_tab_start, D_00153f18, D_00153f10);
                p += len.sprites;
            } else {
                return;
            }
        } else {
            return;
        }
    }
    if (map) {
        level = (Level *)p;
        if (LbFileLoadAt(name, level) == len.map) {
            othervar.f9f9 = map;
            recreate_map(1);
        } else {
            return;
        }
    }
    set_mouse_mode(mouse_mode);
}

// FUNCTION: 0x000b7a1c
void fade_out_menu(void)
{
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
}

// FUNCTION: 0x000b7a44
u8 display_loading_screen(void)
{
    u8 active;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (active)
        LbIsActive();
    LbMouseChangeSprite(0);
    if (resvars.f0 == SCREEN_MODE_1) {
        if (!change_res(SCREEN_MODE_13))
            return 0;
    }
    while (fn_000bed50() != 1) {
    }
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    load_screen(0x15);
    copy_screen_to_wscreen(bscreen);
    LbIsActive();
    LbScreenSwap();
    hosp_fade_palette(othervar.f1f, 10, Lb_PALETTE_FADE_CLOSED);
    if (active) {
        while (fn_000bed50() != 1) {
        }
    }
    return 1;
}

// FUNCTION: 0x000b7b3c
void clear_loading_screen(u8 *palette)
{
    u8 active;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (!active) {
        while (fn_000bed50() != 1) {
        }
    }
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    LbIsActive();
    LbScreenClear(0);
    LbScreenSwap();
    othervar.f1f = palette;
    if (active) {
        while (fn_000bed50() != 1) {
        }
    }
}

// FUNCTION: 0x000b7bf0
void load_screen(const u8 idx)
{
    if (LbFileLoadAt((char *)hiresscreens[idx], bscreen) == LbFileLengthRnc((char *)hiresscreens[idx])) {
        if (LbFileLoadAt((char *)screenpalettes[idx], ppalette.data) == LbFileLengthRnc((char *)screenpalettes[idx])) {
            othervar.f24 = 0;
            othervar.f1f = ppalette.data;
        }
    }
}
