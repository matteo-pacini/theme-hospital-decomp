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
extern TbSprite *screen_sprites_tab_end;
extern u8 *screen_sprites_data;
extern u8 *screen_pointers_data;
extern TbSprite *screen_pointers_tab_end;
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 *palette;
extern u8 D_00156910;
extern PicLengths D_0017526c;

int LbScreenLock(void);
u8 *fn_00022b60(TbCacheUsedBlock *block);
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
int strcmp(const char *a, const char *b);
}

void fade_out_menu(void);
void load_screen(const u8 idx);

// FUNCTION: 0x000b73a0
void return_from_screen(const u8 mouse_mode, const u8 in_game, const u8 sprites)
{
    u8 was_locked;

    was_locked = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if ((resvars.f1 == SCREEN_MODE_1 && in_game) || resvars.f0 == SCREEN_MODE_1) {
        if (was_locked)
            LbScreenUnlock();
        if (in_game)
            AIL_stop_sequence(GetSongHandle());
        if (!change_res(SCREEN_MODE_1)) {
            reset_host();
            exit(1);
        }
        if (in_game && othervar.fa4f)
            AIL_resume_sequence(GetSongHandle());
        while (LbScreenLock() != 1) {
        }
    }
    othervar.fa65 = 0;
    if (sprites)
        LbSpriteReset(screen_sprites_tab_start, screen_sprites_tab_end, screen_sprites_data);
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
        othervar.f1f = palette;
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
                set_mouse_mode(MM_BLUEPRINT);
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
            open_requester(crequester.unknown2, 0);
    }
}

// FUNCTION: 0x000b7648
void load_ingame_pic(const u8 idx, const u8 fade, const u8 mouse_mode, const u8 map, TbScreenMode mode)
{
    PicLengths len = D_0017526c;
    u8 was_locked;
    char name[144];
    u8 *dest;

    was_locked = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if (resvars.f0 != mode) {
        if (was_locked)
            LbScreenUnlock();
        AIL_stop_sequence(GetSongHandle());
        if (!change_res(mode)) {
            reset_host();
            exit(1);
        }
        if (othervar.fa4f)
            AIL_resume_sequence(GetSongHandle());
        while (LbScreenLock() != 1) {
        }
    }
    len.palette = LbFileLengthRnc(hiresscreenpointerfiles[0]);
    len.pointers = LbFileLengthRnc(hiresscreenpointerfiles[1]);
    if (strcmp(hiresscreen_sprites_data[idx], "")) {
        len.sprites = LbFileLengthRnc((char *)hiresscreen_sprites_data[idx]);
        len.sprite_tab = LbFileLengthRnc((char *)hiresscreen_sprites_tabs[idx]);
    }
    if (map) {
        if (install_type < 3) {
            sprintf(name, "%slevels/level.l%d", cd_install_path, map);
            check_cd_available(name);
        } else {
            sprintf(name, "levels/level.l%d", map);
        }
        len.map += LbFileLengthRnc(name);
    }
    clear_sound_heap();
    othervar.fa06 = cache.Allocate(len.palette + len.pointers + len.sprites + len.sprite_tab + len.map + 0x40, 0x63);
    if (othervar.fa06)
        dest = fn_00022b60(othervar.fa06);
    else
        return;
    if (fade)
        fade_out_menu();
    if (strcmp(hiresscreens[idx], "")) {
        u8 is_locked;

        is_locked = lbDisplay.unknown4 ? (u8)1 : (u8)0;
        if (!is_locked) {
            while (LbScreenLock() != 1) {
            }
        }
        load_screen(idx);
        copy_screen_to_wscreen(bscreen);
        LbScreenUnlock();
        LbScreenSwap();
        if (is_locked) {
            while (LbScreenLock() != 1) {
            }
        }
        othervar.fa65 = 1;
    }
    if (LbFileLoadAt(hiresscreenpointerfiles[0], dest) == len.palette) {
        screen_pointers_data = dest;
        dest += len.palette;
        othervar.fa65 = 1;
    } else {
        return;
    }
    if (LbFileLoadAt(hiresscreenpointerfiles[1], dest) == len.pointers) {
        screen_pointers_tab_start = (TbSprite *)dest;
        dest += len.pointers;
        screen_pointers_tab_end = (TbSprite *)dest;
        LbSpriteSetup(screen_pointers_tab_start, screen_pointers_tab_end, screen_pointers_data);
    } else {
        return;
    }
    if (strcmp(hiresscreen_sprites_data[idx], "")) {
        if (LbFileLoadAt((char *)hiresscreen_sprites_tabs[idx], dest) == len.sprite_tab) {
            screen_sprites_tab_start = (TbSprite *)dest;
            dest += len.sprite_tab;
            screen_sprites_tab_end = (TbSprite *)dest;
            if (LbFileLoadAt((char *)hiresscreen_sprites_data[idx], dest) == len.sprites) {
                screen_sprites_data = dest;
                LbSpriteSetup(screen_sprites_tab_start, screen_sprites_tab_end, screen_sprites_data);
                dest += len.sprites;
            } else {
                return;
            }
        } else {
            return;
        }
    }
    if (map) {
        level = (Level *)dest;
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
    u8 was_locked;

    was_locked = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (was_locked)
        LbScreenUnlock();
    LbMouseChangeSprite(0);
    if (resvars.f0 == SCREEN_MODE_1) {
        if (!change_res(SCREEN_MODE_13))
            return 0;
    }
    while (LbScreenLock() != 1) {
    }
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    load_screen(0x15);
    copy_screen_to_wscreen(bscreen);
    LbScreenUnlock();
    LbScreenSwap();
    hosp_fade_palette(othervar.f1f, 10, Lb_PALETTE_FADE_CLOSED);
    if (was_locked) {
        while (LbScreenLock() != 1) {
        }
    }
    return 1;
}

// FUNCTION: 0x000b7b3c
void clear_loading_screen(u8 *palette)
{
    u8 was_locked;

    was_locked = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    if (!was_locked) {
        while (LbScreenLock() != 1) {
        }
    }
    hosp_fade_palette(0, 10, Lb_PALETTE_FADE_CLOSED);
    LbScreenUnlock();
    LbScreenClear(0);
    LbScreenSwap();
    othervar.f1f = palette;
    if (was_locked) {
        while (LbScreenLock() != 1) {
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
