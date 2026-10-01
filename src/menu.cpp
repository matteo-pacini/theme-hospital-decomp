#include "hosp.h"

/*
 * The in-game position table read through resvars.coords, as words (v) or at
 * the pulldown menu's entry.
 */
struct IconScreen {
    union {
        s32 v[0x156];
        struct {
            u8 unknown0[0x54c];
            // 0x54c: Added to the pulldown line height and to each pulldown
            // text's y.
            s32 pulldown_text_dy;
        };
    };
};

#define LbTextCharWidth(c) (lbFontPtr ? lbFontPtr[(c) - 31].width : 0)
#define LbTextCharHeight(c) (lbFontPtr ? lbFontPtr[(c) - 31].height : 0)

void draw_pulldown(u16 x, u16 y, u16 idx);
void draw_pmenu_box(u16 x, u16 y, u16 w, u16 h);
void swap_game_sprites(TbScreenMode mode);
void precalculate_shadow_blocks(void);

u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"

extern "C" {

extern char **pulldown_text;
extern u16 last_p, selected_option;
extern Event D_001549d0[];
void FadeMusic(u32 rate, u8 volume);
extern CompElement *end_elements;
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 error_number;
extern TbSprite *end_pointers;
extern u8 *pointer_data;
extern u8 D_00156910;
void fn_000c93bf(char *dst, char *src);
int LbScreenLock(void);
void change_screen_mode(TbScreenMode mode);
extern s32 D_0014b1c4[], D_0013bb0c[], D_00146c8c[];
extern IconScreen D_0014b7b4, D_0013c0fc;
extern u8 ScreenBoxes[], D_00145108[], mcgabutton_list[], screenlistpos[], D_0014bd5c[], mcgapanel_popup_coord[];
extern u8 D_0012cc7c[];
extern u8 window_list[], button_list[], hiresbuttons[], panel_popup_coord[], D_0012cc3c[];
extern int D_0014be34[], hireshelp[];
extern TbSprite *tile, *end_tiles, *end_sprites, *end_game_panel, *end_pulldv;
extern u8 *tile_data, *sprite_data, *game_panel_data, *pulldv_data;
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
int LbDataLoadAll(TbLoadFiles *files);

int strcmp(const char *a, const char *b);
void LbTextSetClipWindow(u32 x, u32 y, u32 width, u32 height);

}

// Builds the pull-down menu bar from pulldown_text (titles up to ".", then
// each menu's items up to ".", the last ending with ".."): sets each menu's
// item count and width, numbers the submenus and the font metrics. Call after
// a resolution change.
// FUNCTION: 0x00089df0
void setup_menus(void)
{
    u8 done;
    u8 sub;
    u8 i;
    u8 j;
    char sep[2];
    char end[3];

    pmenu.open_pulldown = 0;
    pmenu.font = 1;
    pmenu.font_hi = 2;
    pmenu.disabled = 0;
    set_font_from_cache(pmenu.font);
    pmenu.line_height = LbTextCharHeight('A') + resvars.coords_IconScreenp->pulldown_text_dy;
    pmenu.char_width = LbTextCharWidth('A');
    set_font_from_cache(0);
    sprintf(sep, ".");
    sprintf(end, "..");
    for (i = 1; (pmenu.pulldowns[i].length = fn_000cd716(pulldown_text[i])), strcmp(sep, pulldown_text[i]); i++)
        ;
    pmenu.count = i;
    i++;
    for (j = 1; j < pmenu.count; j++) {
        pmenu.pulldowns[j].width = 0;
        for (sub = 0;; sub++) {
            if (strcmp(sep, pulldown_text[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                break;
            }
            if (fn_000cd716(pulldown_text[i]) > pmenu.pulldowns[j].width)
                pmenu.pulldowns[j].width = fn_000cd716(pulldown_text[i]);
            i++;
        }
    }
    for (done = 0; !done; j++) {
        pmenu.pulldowns[j].width = 0;
        for (sub = 0;; sub++) {
            if (strcmp(end, pulldown_text[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                done = 1;
                break;
            }
            if (strcmp(sep, pulldown_text[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                break;
            }
            if (fn_000cd716(pulldown_text[i]) > pmenu.pulldowns[j].width)
                pmenu.pulldowns[j].width = fn_000cd716(pulldown_text[i]);
            i++;
        }
    }
    sub = pmenu.count;
    for (i = 1; i < pmenu.count; i++) {
        for (j = 0; j < pmenu.pulldowns[i].count; j++) {
            if (pmenu.pulldowns[i].items[j].submenu) {
                pmenu.pulldowns[sub].parent = i;
                pmenu.pulldowns[sub].y = pmenu.line_height + j * pmenu.line_height;
                pmenu.pulldowns[i].items[j].submenu = sub++;
            }
        }
    }
    pmenu.subcount = sub;
}

// Draws the menu bar while the mouse is over the top of the screen or a menu
// is open, with the open menu (and its parent, for a submenu) while the left
// button is held. Sets othervar.menu_bar_active while the bar is shown.
// FUNCTION: 0x0008a1f0
void draw_menus(void)
{
    u8 h1;
    u8 h2;
    u8 h3;
    u8 tile_width;
    u8 y0;
    u16 i;
    u16 last;
    u16 x;

    y0 = 0;
    h1 = pulldv[1].height;
    h2 = pulldv[4].height;
    h3 = pulldv[7].height * 2;
    if ((y0 + h1 + h2 + h3 > lbDisplay.mmouse_y || pmenu.open_pulldown) && !pmenu.disabled) {
        othervar.menu_bar_active = 1;
        lbDisplay.draw_flags = 0;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetClipWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        tile_width = pulldv[1].width;
        last = lbDisplay.graphics_screen_width - tile_width;
        reqspr_draw(0, y0, &pulldv[1]);
        reqspr_draw(0, h1, &pulldv[4]);
        reqspr_draw(0, h1 + h2, &pulldv[7]);
        for (i = tile_width; i < lbDisplay.graphics_screen_width - tile_width; i += tile_width) {
            reqspr_draw(i, y0, &pulldv[2]);
            reqspr_draw(i, h1, &pulldv[5]);
            reqspr_draw(i, h1 + h2, &pulldv[8]);
        }
        reqspr_draw(last, y0, &pulldv[3]);
        reqspr_draw(last, h1, &pulldv[6]);
        reqspr_draw(last, h1 + h2, &pulldv[9]);
        set_all_refresh_zones_in_window(0, 0, lbDisplay.graphics_screen_width, pmenu.line_height, 3);
        set_font_from_cache(pmenu.font);
        x = 0;
        for (i = 1; i < pmenu.count; i++) {
            lbDisplay.draw_flags = 0;
            if (pmenu.pulldowns[i].active) {
                LbTextDraw(x, resvars.coords_IconScreenp->pulldown_text_dy, pulldown_text[pmenu.pulldowns[i].text]);
                if (lbDisplay.mleft_button) {
                    if (pmenu.open_pulldown == i)
                        draw_pulldown(x, (u16)pmenu.line_height, i);
                }
                x += pmenu.pulldowns[i].length * pmenu.char_width;
            }
        }
        if (pmenu.open_pulldown >= pmenu.count) {
            if (lbDisplay.mleft_button) {
            x = 0;
            for (i = 1; i < pmenu.pulldowns[pmenu.open_pulldown].parent; i++)
                x += pmenu.pulldowns[i].length * pmenu.char_width;
            draw_pulldown(x, (u16)pmenu.line_height, (u16)pmenu.pulldowns[pmenu.open_pulldown].parent);
            x += pmenu.char_width * pmenu.pulldowns[i].width + pulldv[6].width;
            draw_pulldown(x, pmenu.pulldowns[pmenu.open_pulldown].y, (u16)pmenu.open_pulldown);
            }
        }
        set_font_from_cache(0);
    } else {
        othervar.menu_bar_active = 0;
    }
}

// Draws pulldown idx at (x, y): its box, items (greyed when disabled in a
// network game), check marks, and the item under the mouse highlighted, which
// becomes selected_option (1-based) with last_p = idx.
// FUNCTION: 0x0008a618
void draw_pulldown(u16 x, u16 y, u16 idx)
{
    u8 i;
    u16 saved;
    u16 saved2;

    draw_pmenu_box(x, y, pmenu.pulldowns[idx].width * pmenu.char_width, pmenu.pulldowns[idx].count * pmenu.line_height);
    set_all_refresh_zones_in_window(x, y, x + pmenu.pulldowns[idx].width * pmenu.char_width, y + pmenu.pulldowns[idx].count * pmenu.line_height, 3);
    for (i = 0; i < pmenu.pulldowns[idx].count; i++) {
        if (!(pmenu.pulldowns[idx].items[i].flags & 1) || statvar.num_players <= 1) {
            if (pmenu.pulldowns[idx].items[i].check) {
                if (*pmenu.pulldowns[idx].items[i].check)
                    reqspr_draw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, &pulldv[10]);
            }
            if (pmenu.pulldowns[idx].items[i].check2) {
                if (*pmenu.pulldowns[idx].items[i].check2)
                    reqspr_draw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, &pulldv[10]);
            }
        }
        if ((pmenu.pulldowns[idx].items[i].flags & 1) && statvar.num_players > 1) {
            saved = lbDisplay.draw_flags;
            lbDisplay.draw_flags = 4;
            LbTextDraw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, pulldown_text[pmenu.pulldowns[idx].items[i].text]);
            lbDisplay.draw_flags = saved;
        } else {
            LbTextDraw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, pulldown_text[pmenu.pulldowns[idx].items[i].text]);
        }
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, i * pmenu.line_height + y, pmenu.pulldowns[idx].width * pmenu.char_width, pmenu.line_height)) {
            set_font_from_cache(pmenu.font_hi);
            if (selected_option != i + 1 || last_p != idx) {
                last_p = idx;
                play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
            }
            selected_option = i + 1;
            if ((pmenu.pulldowns[idx].items[i].flags & 1) && statvar.num_players > 1) {
                saved2 = lbDisplay.draw_flags;
                lbDisplay.draw_flags = 4;
                LbTextDraw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, pulldown_text[pmenu.pulldowns[idx].items[i].text]);
                lbDisplay.draw_flags = saved2;
            } else {
                LbTextDraw(x, y + i * pmenu.line_height + resvars.coords_IconScreenp->pulldown_text_dy, pulldown_text[pmenu.pulldowns[idx].items[i].text]);
            }
            set_font_from_cache(pmenu.font);
        }
    }
}

// Draws an empty pull-down box with its border, w x h pixels inside, at
// (x, y), and marks it for redraw.
// FUNCTION: 0x0008aaf8
void draw_pmenu_box(u16 x, u16 y, u16 w, u16 h)
{
    u8 sw;
    u8 sh;
    u16 dw;
    u16 saved;
    u16 dh;
    u16 cx;
    u16 cy;

    saved = lbDisplay.draw_flags;
    lbDisplay.draw_flags = 8;
    dw = 0;
    dh = 0;
    sw = pulldv[5].width;
    sh = pulldv[5].height;
    for (cy = y + sh; cy <= y + h; cy += sh) {
        for (cx = x + sw; cx <= x + w; cx += sw) {
            reqspr_draw(cx, cy, &pulldv[5]);
            dw += sw;
            dh += sh;
        }
    }
    sw = pulldv[2].width;
    sh = pulldv[2].height;
    for (cx = x + sw; cx <= x + w; cx += sw) {
        reqspr_draw(cx, y, &pulldv[2]);
        reqspr_draw(cx, y + h, &pulldv[8]);
    }
    sw = pulldv[4].width;
    sh = pulldv[4].height;
    for (cy = y + sh; cy <= y + h; cy += sh) {
        reqspr_draw(x, cy, &pulldv[4]);
        reqspr_draw(x + w, cy, &pulldv[6]);
    }
    reqspr_draw(x, y, &pulldv[1]);
    reqspr_draw(x + w, y, &pulldv[3]);
    reqspr_draw(x, y + h, &pulldv[7]);
    reqspr_draw(x + w, y + h, &pulldv[9]);
    dw = 0;
    dh = 0;
    dw += pulldv[3].width;
    dh += pulldv[3].height;
    lbDisplay.draw_flags = saved;
    set_all_refresh_zones_in_window(x, y, dw + (x + w), dh + (y + h), 3);
}

// Menu bar mouse handling: on a release over an item of the open menu,
// toggles its check flag, sends its key and calls its callback (othervar.pulldown_arg
// = the item's arg, f90 = its index); while held, opens the menu or submenu
// under the mouse. Closes the menu when the button is up.
// FUNCTION: 0x0008add8
extern "C" void process_menus(void)
{
    u8 h1;
    u8 y0;
    u8 h2;
    u16 i;
    u16 j;
    u16 x2;
    u16 x;
    u16 y;
    u16 item_no;

    y0 = 0;
    h1 = pulldv[1].height;
    h2 = pulldv[4].height;
    if (lbDisplay.rleft_button && !pmenu.disabled) {
        x = 0;
        if (pmenu.open_pulldown >= pmenu.count)
            y = pmenu.pulldowns[pmenu.open_pulldown].y;
        else
            y = pmenu.line_height;
        item_no = 0;
        for (i = 1; i < pmenu.count + pmenu.subcount; i++) {
            if (pmenu.pulldowns[i].active) {
                if (pmenu.open_pulldown == i) {
                    for (j = 0; j < pmenu.pulldowns[i].count; j++) {
                        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, pmenu.line_height * j + y, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.line_height)) {
                            debug.last_action = item_no;
                            othervar.clicked_pulldown_item = item_no;
                            if (!(((pmenu.pulldowns[i].items[j].flags & 1) && statvar.num_players > 1) || (!pmenu.pulldowns[i].items[j].usable_paused && gamevar.paused))) {
                                if (gamevar.rat_fest_level && !pmenu.pulldowns[i].items[j].usable_rat_level) {
                                    play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                                } else {
                                    if (pmenu.pulldowns[i].items[j].check) {
                                        if (*pmenu.pulldowns[i].items[j].check)
                                            *pmenu.pulldowns[i].items[j].check = 0;
                                        else
                                            *pmenu.pulldowns[i].items[j].check = 1;
                                    }
                                    if (pmenu.pulldowns[i].items[j].key)
                                        process_keys(pmenu.pulldowns[i].items[j].shift, pmenu.pulldowns[i].items[j].key);
                                    if (pmenu.pulldowns[i].items[j].callback) {
                                        othervar.pulldown_arg = pmenu.pulldowns[i].items[j].arg;
                                        othervar.pulldown_item = j;
                                        pmenu.pulldowns[i].items[j].callback();
                                    }
                                    play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
                                }
                            } else {
                                play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
                            }
                            break;
                        }
                        item_no++;
                    }
                } else {
                    item_no += pmenu.pulldowns[i].count;
                }
                if (pmenu.open_pulldown < pmenu.count) {
                    x += pmenu.pulldowns[i].length * pmenu.char_width;
                } else {
                    x = 0;
                    for (j = 1; j < pmenu.pulldowns[pmenu.open_pulldown].parent; j++)
                        x += pmenu.pulldowns[j].length * pmenu.char_width;
                    x += pmenu.pulldowns[j].width * pmenu.char_width + pulldv[6].width;
                }
            }
        }
        lbDisplay.rleft_button = 0;
    }
    if (lbDisplay.mleft_button && !pmenu.disabled) {
        if (y0 + h1 + h2 > lbDisplay.mmouse_y) {
            x2 = 0;
            x = pmenu.char_width * pmenu.pulldowns[1].length;
            for (i = 1; i < pmenu.count; i++) {
                if (pmenu.pulldowns[i].active) {
                    if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x2, 0, x, pmenu.line_height)) {
                        if (pmenu.open_pulldown != i)
                            play_sound(SNDSRC_GENERAL, SFX_HLIGHT5, -1, -1);
                        pmenu.open_pulldown = i;
                        break;
                    }
                    x2 += x;
                    x = pmenu.pulldowns[i + 1].length * pmenu.char_width;
                }
            }
        } else {
            if (pmenu.open_pulldown >= pmenu.count) {
                x = 0;
                for (i = 1; i < pmenu.pulldowns[pmenu.open_pulldown].parent; i++) {
                    if (pmenu.pulldowns[i].active)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                }
                i = pmenu.pulldowns[pmenu.open_pulldown].parent;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, pmenu.line_height, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.pulldowns[i].count * pmenu.line_height))
                    pmenu.open_pulldown = pmenu.pulldowns[pmenu.open_pulldown].parent;
            }
            if (pmenu.open_pulldown && pmenu.pulldowns[pmenu.open_pulldown].active) {
                if (pmenu.pulldowns[pmenu.open_pulldown].parent) {
                    x = 0;
                    for (i = 1; i < pmenu.pulldowns[pmenu.open_pulldown].parent; i++)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                } else {
                    x = 0;
                    for (i = 1; i < pmenu.open_pulldown; i++)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                }
                i = pmenu.open_pulldown;
                for (j = 0; j < pmenu.pulldowns[i].count; j++) {
                    if ((pmenu.pulldowns[i].items[j].flags & 1) && statvar.num_players > 1)
                        continue;
                    if (pmenu.pulldowns[i].items[j].submenu) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, pmenu.line_height + pmenu.line_height * j, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.line_height)) {
                            pmenu.open_pulldown = pmenu.pulldowns[i].items[j].submenu;
                            break;
                        }
                    }
                }
            }
        }
    } else {
        pmenu.open_pulldown = 0;
    }
}

// Load menu item: queues event EV_LOAD_GAME to load save slot othervar.pulldown_item + 1 (1-9).
// FUNCTION: 0x0008b6a0
extern "C" void pulldown_load(void)
{
    u8 slot;

    slot = othervar.pulldown_item + 1;
    if (slot < 10) {
        debug.last_action = slot;
        D_001549d0[player].type = EV_LOAD_GAME;
        D_001549d0[player].value = slot;
    }
}

// Save menu item: queues event EV_SAVE_GAME to save to slot othervar.pulldown_item + 1, asking
// first if the slot is already used.
// FUNCTION: 0x0008b6f8
extern "C" void pulldown_save(void)
{
    u8 slot;

    slot = othervar.pulldown_item + 1;
    debug.last_action = slot;
    if (othervar.save_slot_used[slot] == 1) {
        sprintf(othervar.yesno_msg, "%s", yesno_text[3]);
        open_yes_no_requester(EV_SAVE_GAME, (u16)slot, 0, EV_CLOSE_SAVE_YES_NO, 0x63, 0);
    } else {
        D_001549d0[player].type = EV_SAVE_GAME;
        D_001549d0[player].value = slot;
        if (slot < 10)
            othervar.save_slot_used[slot] = 1;
    }
}

// Shows the level's briefing (or the network game's criteria); refused in
// mouse mode 0xe.
// FUNCTION: 0x0008b7b4
void pulldown_briefing(void)
{
    if (gamevar.mouse_mode == MM_EDIT_ROOM_WAIT)
        play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
    else if (statvar.num_players <= 1)
        show_help_page(1, level_number);
    else
        show_help_page(2, current_net_criteria.criterion);
}

// Toggles the shadows option (myplayer.shadows_on) and redraws the view.
// FUNCTION: 0x0008b824
extern "C" void pulldown_toggle_shadows(void)
{
    if (myplayer.shadows_on > 0) {
        myplayer.shadows_on = 0;
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        othervar.full_redraw = 1;
    } else {
        myplayer.shadows_on = 1;
        add_static_shadows();
        set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
        othervar.full_redraw = 1;
    }
}

// Sound effects volume item: item othervar.pulldown_item (0 loudest, 9 quietest) sets
// myplayer.sfx_volume to 0-127 and moves the menu's check mark.
// FUNCTION: 0x0008b8a0
extern "C" void pulldown_svol(void)
{
    u8 sel;
    u8 i;

    sel = othervar.pulldown_item;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.sfx_volume_checked[i] = 0;
        othervar.sfx_volume_checked[sel] = 1;
        myplayer.sfx_volume = (10 - sel) * 0x7f / 10;
    }
}

// Music volume item: as pulldown_svol for myplayer.music_volume, fading the music to
// the new volume.
// FUNCTION: 0x0008b91c
extern "C" void pulldown_smusic(void)
{
    u8 i;
    u32 sel;

    sel = othervar.pulldown_item;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.music_volume_checked[i] = 0;
        othervar.music_volume_checked[sel] = 1;
        myplayer.music_volume = (10 - sel) * 0x7f / 10;
        FadeMusic(1000, myplayer.music_volume);
    }
}

// Announcer volume item: as pulldown_svol for myplayer.ambient_volume.
// FUNCTION: 0x0008b99c
extern "C" void pulldown_sannounce(void)
{
    u8 sel;
    u8 i;

    sel = othervar.pulldown_item;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.ambient_volume_checked[i] = 0;
        othervar.ambient_volume_checked[sel] = 1;
        myplayer.ambient_volume = (10 - sel) * 0x7f / 10;
    }
}

// Toggles the music (myplayer.music_on), starting or stopping the MIDI song;
// off if there is no music driver.
// FUNCTION: 0x0008ba18
extern "C" void pulldown_toggle_music(void)
{
    if (statvar.music_ok) {
        myplayer.music_on = !myplayer.music_on ? (u8)1 : (u8)0;
        if (myplayer.music_on)
            play_midi(othervar.midi_track);
        else
            stop_midi();
    } else {
        myplayer.music_on = 0;
    }
}

// Toggles the sound effects (myplayer.sfx_on), stopping all samples when off;
// off if there is no sound driver.
// FUNCTION: 0x0008ba78
extern "C" void pulldown_toggle_sound(void)
{
    if (statvar.sound_ok)
        myplayer.sfx_on = !myplayer.sfx_on ? (u8)1 : (u8)0;
    else
        myplayer.sfx_on = 0;
    if (myplayer.sfx_on == 0)
        StopAllSamples();
}

// Toggles the ambient sounds (myplayer.ambients_on); off if there is no sound driver.
// FUNCTION: 0x0008bacc
extern "C" void pulldown_toggle_ambient(void)
{
    if (statvar.sound_ok)
        myplayer.ambients_on = !myplayer.ambients_on ? (u8)1 : (u8)0;
    else
        myplayer.ambients_on = 0;
}

// Game speed item: item othervar.pulldown_item (0-4) becomes myplayer.speed_choice and queues
// EV_VARY_SPEED with the item's arg.
// FUNCTION: 0x0008bb10
extern "C" void pulldown_vary_speed(void)
{
    u8 sel;
    u8 i;

    sel = othervar.pulldown_item;
    if (sel < 5) {
        for (i = 0; i < 5; i++)
            othervar.speed_checked[i] = 0;
        othervar.speed_checked[sel] = 1;
        myplayer.speed_choice = sel;
        D_001549d0[player].type = EV_VARY_SPEED;
        D_001549d0[player].value = othervar.pulldown_arg;
    }
}

// Screen mode item: switches between 320x200 (SCREEN_MODE_1) and 640x480
// (SCREEN_MODE_13), closing open requesters, unless the item is the current
// mode or a requester that must stay open is shown.
// FUNCTION: 0x0008bb98
extern "C" void pulldown_toggle_screen_mode(void)
{
    if (!((resvars.screen_mode == SCREEN_MODE_1 && othervar.pulldown_item == 1) || (resvars.screen_mode == SCREEN_MODE_13 && othervar.pulldown_item == 0))) {
        if (!(crequester.id == REQ_EDIT_ROOM || (crequester.id == REQ_BUY_OBJECTS && crequester.arg != 1))) {
            if (crequester.id != REQ_NONE)
                close_crequester(crequester.id);
            close_irequesters();
            advisor.advice_id = 0;
            if (resvars.game_mode == SCREEN_MODE_13) {
                if (othervar.music_playing)
                    AIL_stop_sequence(GetSongHandle());
                change_screen_mode(SCREEN_MODE_1);
                if (othervar.music_playing)
                    AIL_resume_sequence(GetSongHandle());
            } else if (resvars.game_mode == SCREEN_MODE_1) {
                if (othervar.music_playing)
                    AIL_stop_sequence(GetSongHandle());
                change_screen_mode(SCREEN_MODE_13);
                if (othervar.music_playing)
                    AIL_resume_sequence(GetSongHandle());
            }
        }
    }
}

// Scales the CompElement positions of element[] to the 320x200 layout.
// FUNCTION: 0x0008bcb4
extern "C" void setup_mcga_elements(void)
{
    CompElement *elem;

    elem = element;
    while (elem <= end_elements) {
        elem->x = elem->x / 2 + 0x46;
        elem->y = elem->y / 2 + 0x65;
        elem++;
    }
}

// Marks which of the player's ten save slots exist (othervar.save_slot_used).
// FUNCTION: 0x0008bd24
void check_save_games(void)
{
    char buf[100];
    s32 handle;
    u8 i;

    for (i = 0; i < 10; i++) {
        othervar.save_slot_used[i] = 0;
        sprintf(buf, "%ssave\\", statvar.save_root);
        fn_000c93bf(buf, myplayer.name_chars);
        sprintf(buf, "%s.h%d", buf, i);
        handle = LbFileOpen(buf, 2);
        if (handle != -1) {
            othervar.save_slot_used[i] = 1;
            LbFileClose(handle);
        }
    }
}

// Sets the menus' check marks (screen mode, volume levels, game speed) from
// the current settings and finds the used save slots.
// FUNCTION: 0x0008bdd4
extern "C" void setup_pulldown_menu_options(void)
{
    u8 i;

    switch (resvars.screen_mode) {
    case SCREEN_MODE_1:
        othervar.hires_checked = 0;
        othervar.lores_checked = 1;
        break;
    case SCREEN_MODE_13:
        othervar.lores_checked = 0;
        othervar.hires_checked = 1;
        break;
    default:
        break;
    }
    check_save_games();
    for (i = 0; i < 10; i++) {
        othervar.sfx_volume_checked[i] = 0;
        othervar.ambient_volume_checked[i] = 0;
        othervar.music_volume_checked[i] = 0;
    }
    othervar.sfx_volume_checked[9 - myplayer.sfx_volume * 10 / 128] = 1;
    othervar.ambient_volume_checked[9 - myplayer.ambient_volume * 10 / 128] = 1;
    othervar.music_volume_checked[9 - myplayer.music_volume * 10 / 128] = 1;
    for (i = 0; i < 5; i++)
        othervar.speed_checked[i] = 0;
    othervar.speed_checked[myplayer.speed_choice] = 1;
}


// Overwrites the 320x200 layout tables with that mode's coordinates.
// FUNCTION: 0x0008bf24
void mcga_coords(void)
{
    resvars.coords_IconScreenp->v[0x110] = 2;
    resvars.coords_IconScreenp->v[0x111] = 2;
    resvars.coords_IconScreenp->v[0x98] = 0x3d;
    resvars.coords_IconScreenp->v[0x99] = 0xc4;
    resvars.coords_IconScreenp->v[0x96] = 0x1c;
    resvars.coords_IconScreenp->v[0x97] = 0xc2;
    resvars.coords_IconScreenp->v[0x132] = 6;
    resvars.coords_IconScreenp->v[0x134] = 2;
    resvars.coords_IconScreenp->v[0x138] = 8;
    resvars.coords_IconScreenp->v[0x136] = 7;
    resvars.coords_IconScreenp->v[0x13a] = 0x13;
    resvars.coords_IconScreenp->v[0x153] = 1;
    resvars.coords_IconScreenp->v[0x127] = 0x90;
    resvars.coords_IconScreenp->v[0x13c] = 0x118;
    resvars.coords_IconScreenp->v[0x13d] = 0x90;
    resvars.coords_IconScreenp->v[0x12d] = 0x93;
    resvars.coords_IconScreenp->v[0x12f] = 0x98;
    resvars.coords_IconScreenp->v[0x131] = 0x9d;
    resvars.boxes_s32p[0x104] = 0x27;
    resvars.boxes_s32p[0x105] = 0x6e;
    resvars.boxes_s32p[0x106] = 0x58;
    resvars.boxes_s32p[0x107] = 0x1c;
    resvars.coords_IconScreenp->v[0xa4] = 0x26;
    resvars.coords_IconScreenp->v[0xa5] = 9;
    resvars.coords_IconScreenp->v[0xa6] = 0x5d;
    resvars.coords_IconScreenp->v[0xa7] = 0x40;
    resvars.coords_IconScreenp->v[0x14c] = 1;
    resvars.coords_IconScreenp->v[0x14d] = 1;
    resvars.coords_IconScreenp->v[0x9c] = 0xe;
    resvars.coords_IconScreenp->v[0x9d] = 0x1a;
    resvars.coords_IconScreenp->v[0x9e] = 0xe;
    resvars.coords_IconScreenp->v[0x9f] = 0x43;
    resvars.coords_IconScreenp->v[0xa0] = 0xe;
    resvars.coords_IconScreenp->v[0xa1] = 0x6c;
    resvars.coords_IconScreenp->v[0xa2] = 0xe;
    resvars.coords_IconScreenp->v[0xa3] = 0x95;
    resvars.screen_coords_s32p[0x128] = 0x15;
    resvars.coords_IconScreenp->v[0x6] = 0x3d;
    resvars.coords_IconScreenp->v[0x7] = 0x40;
    resvars.coords_IconScreenp->v[0x8] = 0x1c;
    resvars.coords_IconScreenp->v[0x9] = 0x5e;
    resvars.coords_IconScreenp->v[0x4] = 9;
    resvars.coords_IconScreenp->v[0x5] = 9;
    resvars.coords_IconScreenp->v[0x0] = 0x1d;
    resvars.coords_IconScreenp->v[0x1] = 0x6a;
    resvars.coords_IconScreenp->v[0xa8] = 0x39;
    resvars.coords_IconScreenp->v[0xa9] = 0x5e;
    resvars.boxes_s32p[0x15c] = 0xa;
    resvars.boxes_s32p[0x15d] = 0x45;
    resvars.boxes_s32p[0x15e] = 0x5a;
    resvars.boxes_s32p[0x15f] = 0x14;
    resvars.boxes_s32p[0x160] = 0x39;
    resvars.boxes_s32p[0x161] = 0x6d;
    resvars.boxes_s32p[0x162] = 0x26;
    resvars.boxes_s32p[0x163] = 0x15;
    resvars.coords_IconScreenp->v[0xaa] = 0x2e;
    resvars.coords_IconScreenp->v[0xab] = 0x28;
    resvars.coords_IconScreenp->v[0xac] = 0x39;
    resvars.coords_IconScreenp->v[0xad] = 0x28;
    resvars.coords_IconScreenp->v[0xae] = 0x49;
    resvars.coords_IconScreenp->v[0xaf] = 0x28;
    resvars.coords_IconScreenp->v[0xb0] = 0x58;
    resvars.coords_IconScreenp->v[0xb1] = 0x49;
    resvars.coords_IconScreenp->v[0xb2] = 0x61;
    resvars.coords_IconScreenp->v[0xb3] = 0x49;
    resvars.coords_IconScreenp->v[0xb4] = 0x6e;
    resvars.coords_IconScreenp->v[0xb5] = 0x49;
    resvars.coords_IconScreenp->v[0x2c] = 0x59;
    resvars.coords_IconScreenp->v[0x2d] = 0xa;
    resvars.coords_IconScreenp->v[0x4c] = 0xa;
    resvars.coords_IconScreenp->v[0x4d] = 0xa;
    resvars.coords_IconScreenp->v[0x4e] = 0xa;
    resvars.coords_IconScreenp->v[0x4f] = 0x1c;
    resvars.coords_IconScreenp->v[0x50] = 0xa;
    resvars.coords_IconScreenp->v[0x51] = 0x2e;
    resvars.coords_IconScreenp->v[0x52] = 0xa;
    resvars.coords_IconScreenp->v[0x53] = 0x40;
    resvars.coords_IconScreenp->v[0x2e] = 0x59;
    resvars.coords_IconScreenp->v[0x2f] = 0x7a;
    resvars.coords_IconScreenp->v[0x54] = 0x59;
    resvars.coords_IconScreenp->v[0x55] = 0x16;
    resvars.coords_IconScreenp->v[0x56] = 0x59;
    resvars.coords_IconScreenp->v[0x57] = 0x21;
    resvars.coords_IconScreenp->v[0x58] = 0x59;
    resvars.coords_IconScreenp->v[0x59] = 0x2c;
    resvars.coords_IconScreenp->v[0x5a] = 0x59;
    resvars.coords_IconScreenp->v[0x5b] = 0x37;
    resvars.coords_IconScreenp->v[0x5c] = 0x59;
    resvars.coords_IconScreenp->v[0x5d] = 0x42;
    resvars.coords_IconScreenp->v[0x5e] = 0x59;
    resvars.coords_IconScreenp->v[0x5f] = 0x4d;
    resvars.coords_IconScreenp->v[0x60] = 0x59;
    resvars.coords_IconScreenp->v[0x61] = 0x58;
    resvars.coords_IconScreenp->v[0x62] = 0x59;
    resvars.coords_IconScreenp->v[0x63] = 0x63;
    resvars.coords_IconScreenp->v[0x64] = 0x59;
    resvars.coords_IconScreenp->v[0x65] = 0x6e;
    resvars.coords_IconScreenp->v[0x66] = 0x59;
    resvars.coords_IconScreenp->v[0x67] = 0x79;
    resvars.boxes_s32p[0x4] = 0x59;
    resvars.boxes_s32p[0x5] = 0x16;
    resvars.boxes_s32p[0x6] = 0x62;
    resvars.boxes_s32p[0x7] = 7;
    resvars.boxes_s32p[0x8] = 0x59;
    resvars.boxes_s32p[0x9] = 0x21;
    resvars.boxes_s32p[0xa] = 0x62;
    resvars.boxes_s32p[0xb] = 7;
    resvars.boxes_s32p[0xc] = 0x59;
    resvars.boxes_s32p[0xd] = 0x2c;
    resvars.boxes_s32p[0xe] = 0x62;
    resvars.boxes_s32p[0xf] = 7;
    resvars.boxes_s32p[0x10] = 0x59;
    resvars.boxes_s32p[0x11] = 0x37;
    resvars.boxes_s32p[0x12] = 0x62;
    resvars.boxes_s32p[0x13] = 7;
    resvars.boxes_s32p[0x14] = 0x59;
    resvars.boxes_s32p[0x15] = 0x42;
    resvars.boxes_s32p[0x16] = 0x62;
    resvars.boxes_s32p[0x17] = 7;
    resvars.boxes_s32p[0x18] = 0x59;
    resvars.boxes_s32p[0x19] = 0x4d;
    resvars.boxes_s32p[0x1a] = 0x62;
    resvars.boxes_s32p[0x1b] = 7;
    resvars.boxes_s32p[0x1c] = 0x59;
    resvars.boxes_s32p[0x1d] = 0x58;
    resvars.boxes_s32p[0x1e] = 0x62;
    resvars.boxes_s32p[0x1f] = 7;
    resvars.boxes_s32p[0x20] = 0x59;
    resvars.boxes_s32p[0x21] = 0x63;
    resvars.boxes_s32p[0x22] = 0x62;
    resvars.boxes_s32p[0x23] = 7;
    resvars.boxes_s32p[0x24] = 0x59;
    resvars.boxes_s32p[0x25] = 0x6e;
    resvars.boxes_s32p[0x26] = 0x62;
    resvars.boxes_s32p[0x27] = 7;
    resvars.boxes_s32p[0x28] = 0x59;
    resvars.boxes_s32p[0x29] = 0x79;
    resvars.boxes_s32p[0x2a] = 0x62;
    resvars.boxes_s32p[0x2b] = 7;
    resvars.coords_IconScreenp->v[0xc2] = 0xb;
    resvars.coords_IconScreenp->v[0xc3] = 0xb;
    resvars.coords_IconScreenp->v[0xc4] = 0x21;
    resvars.coords_IconScreenp->v[0xc5] = 0x1e;
    resvars.boxes_s32p[0x14c] = 0x23;
    resvars.boxes_s32p[0x14d] = 0x35;
    resvars.boxes_s32p[0x14e] = 0x15;
    resvars.coords_IconScreenp->v[0xd6] = 0x3d;
    resvars.coords_IconScreenp->v[0xd7] = 0x74;
    resvars.coords_IconScreenp->v[0xca] = 0x13;
    resvars.coords_IconScreenp->v[0xcb] = 0x42;
    resvars.coords_IconScreenp->v[0xce] = 0x4b;
    resvars.coords_IconScreenp->v[0xcf] = 0x1f;
    resvars.coords_IconScreenp->v[0xd2] = 0x15;
    resvars.coords_IconScreenp->v[0xd0] = 0x4b;
    resvars.coords_IconScreenp->v[0xd1] = 0x2d;
    resvars.coords_IconScreenp->v[0xd4] = 0x4b;
    resvars.coords_IconScreenp->v[0xd5] = 0x3b;
    resvars.coords_IconScreenp->v[0xe4] = 0x17;
    resvars.coords_IconScreenp->v[0xe5] = 0x68;
    resvars.coords_IconScreenp->v[0xe6] = 0x38;
    resvars.coords_IconScreenp->v[0xe7] = 0x68;
    resvars.coords_IconScreenp->v[0xe8] = 0x59;
    resvars.coords_IconScreenp->v[0xe9] = 0x68;
    resvars.coords_IconScreenp->v[0xcc] = 0x3d;
    resvars.coords_IconScreenp->v[0xcd] = 0x63;
    resvars.boxes_s32p[0x68] = 0x13;
    resvars.boxes_s32p[0x69] = 0x57;
    resvars.boxes_s32p[0x6c] = 0x1e;
    resvars.boxes_s32p[0x6d] = 0x57;
    resvars.boxes_s32p[0x70] = 0x2d;
    resvars.boxes_s32p[0x71] = 0x57;
    resvars.boxes_s32p[0x74] = 0x4c;
    resvars.boxes_s32p[0x75] = 0x4e;
    resvars.boxes_s32p[0x78] = 0x58;
    resvars.boxes_s32p[0x79] = 0x4e;
    resvars.boxes_s32p[0x7c] = 0x64;
    resvars.boxes_s32p[0x7d] = 0x4e;
    resvars.boxes_s32p[0x100] = 0x12;
    resvars.boxes_s32p[0x101] = 0x47;
    resvars.boxes_s32p[0x102] = 0x5f;
    resvars.boxes_s32p[0x103] = 0x15;
    resvars.coords_IconScreenp->v[0xc8] = 0x13;
    resvars.coords_IconScreenp->v[0xc9] = 0xc;
    resvars.boxes_s32p[0x138] = 0xa;
    resvars.boxes_s32p[0x139] = 0x6f;
    resvars.boxes_s32p[0x13a] = 0x25;
    resvars.boxes_s32p[0x13b] = 0x25;
    resvars.boxes_s32p[0x154] = 0x22;
    resvars.boxes_s32p[0x155] = 0x50;
    resvars.boxes_s32p[0x156] = 0x15;
    resvars.coords_IconScreenp->v[0xd8] = 0x22;
    resvars.coords_IconScreenp->v[0xd9] = 0x5e;
    resvars.coords_IconScreenp->v[0xda] = 0x22;
    resvars.coords_IconScreenp->v[0xdb] = 0x6c;
    resvars.boxes_s32p[0x13c] = 0xa;
    resvars.boxes_s32p[0x13d] = 0x7a;
    resvars.boxes_s32p[0x13e] = 0x25;
    resvars.boxes_s32p[0x13f] = 0x25;
    resvars.boxes_s32p[0x148] = 0x46;
    resvars.boxes_s32p[0x149] = 0x79;
    resvars.boxes_s32p[0x14a] = 0x20;
    resvars.boxes_s32p[0x14b] = 0x21;
    resvars.boxes_s32p[0xf8] = 0x48;
    resvars.boxes_s32p[0xf9] = 0x63;
    resvars.boxes_s32p[0xfa] = 0x1e;
    resvars.boxes_s32p[0xfb] = 0x16;
    resvars.boxes_s32p[0xfc] = 0x58;
    resvars.boxes_s32p[0xfd] = 0x36;
    resvars.boxes_s32p[0xfe] = 0xf;
    resvars.boxes_s32p[0xff] = 0x11;
    resvars.boxes_s32p[0x144] = 0x48;
    resvars.boxes_s32p[0x145] = 0x4b;
    resvars.boxes_s32p[0x146] = 0x1e;
    resvars.boxes_s32p[0x147] = 0x16;
    resvars.coords_IconScreenp->v[0x154] = 0x14;
    resvars.coords_IconScreenp->v[0x155] = 0;
    resvars.coords_IconScreenp->v[0x32] = 0x59;
    resvars.coords_IconScreenp->v[0x33] = 9;
    resvars.coords_IconScreenp->v[0x34] = 0xa;
    resvars.coords_IconScreenp->v[0x35] = 0x5e;
    resvars.coords_IconScreenp->v[0x36] = 0xa;
    resvars.coords_IconScreenp->v[0x37] = 0x75;
    resvars.coords_IconScreenp->v[0x68] = 0x59;
    resvars.coords_IconScreenp->v[0x69] = 0x15;
    resvars.coords_IconScreenp->v[0x6a] = 0x59;
    resvars.coords_IconScreenp->v[0x6b] = 0x20;
    resvars.coords_IconScreenp->v[0x6c] = 0x59;
    resvars.coords_IconScreenp->v[0x6d] = 0x2b;
    resvars.coords_IconScreenp->v[0x6e] = 0x59;
    resvars.coords_IconScreenp->v[0x6f] = 0x36;
    resvars.coords_IconScreenp->v[0x70] = 0x59;
    resvars.coords_IconScreenp->v[0x71] = 0x41;
    resvars.coords_IconScreenp->v[0x72] = 0x59;
    resvars.coords_IconScreenp->v[0x73] = 0x4c;
    resvars.coords_IconScreenp->v[0x74] = 0x59;
    resvars.coords_IconScreenp->v[0x75] = 0x57;
    resvars.coords_IconScreenp->v[0x76] = 0x59;
    resvars.coords_IconScreenp->v[0x77] = 0x62;
    resvars.coords_IconScreenp->v[0x78] = 0x59;
    resvars.coords_IconScreenp->v[0x79] = 0x6d;
    resvars.coords_IconScreenp->v[0x7a] = 0x59;
    resvars.coords_IconScreenp->v[0x7b] = 0x78;
    resvars.coords_IconScreenp->v[0x7c] = 0xbc;
    resvars.coords_IconScreenp->v[0x7d] = 0x16;
    resvars.coords_IconScreenp->v[0x7e] = 0xbc;
    resvars.coords_IconScreenp->v[0x7f] = 0x21;
    resvars.coords_IconScreenp->v[0x80] = 0xbc;
    resvars.coords_IconScreenp->v[0x81] = 0x2c;
    resvars.coords_IconScreenp->v[0x82] = 0xbc;
    resvars.coords_IconScreenp->v[0x83] = 0x37;
    resvars.coords_IconScreenp->v[0x84] = 0xbc;
    resvars.coords_IconScreenp->v[0x85] = 0x42;
    resvars.coords_IconScreenp->v[0x86] = 0xbc;
    resvars.coords_IconScreenp->v[0x87] = 0x4d;
    resvars.coords_IconScreenp->v[0x88] = 0xbc;
    resvars.coords_IconScreenp->v[0x89] = 0x58;
    resvars.coords_IconScreenp->v[0x8a] = 0xbc;
    resvars.coords_IconScreenp->v[0x8b] = 0x63;
    resvars.coords_IconScreenp->v[0x8c] = 0xbc;
    resvars.coords_IconScreenp->v[0x8d] = 0x6e;
    resvars.coords_IconScreenp->v[0x8e] = 0xbc;
    resvars.coords_IconScreenp->v[0x8f] = 0x79;
    resvars.boxes_s32p[0x2c] = 0x57;
    resvars.boxes_s32p[0x2d] = 0x14;
    resvars.boxes_s32p[0x2e] = 0x7a;
    resvars.boxes_s32p[0x2f] = 0xa;
    resvars.boxes_s32p[0x30] = 0x57;
    resvars.boxes_s32p[0x31] = 0x1f;
    resvars.boxes_s32p[0x32] = 0x7a;
    resvars.boxes_s32p[0x33] = 0xa;
    resvars.boxes_s32p[0x34] = 0x57;
    resvars.boxes_s32p[0x35] = 0x2a;
    resvars.boxes_s32p[0x36] = 0x7a;
    resvars.boxes_s32p[0x37] = 0xa;
    resvars.boxes_s32p[0x38] = 0x57;
    resvars.boxes_s32p[0x39] = 0x35;
    resvars.boxes_s32p[0x3a] = 0x7a;
    resvars.boxes_s32p[0x3b] = 0xa;
    resvars.boxes_s32p[0x3c] = 0x57;
    resvars.boxes_s32p[0x3d] = 0x40;
    resvars.boxes_s32p[0x3e] = 0x7a;
    resvars.boxes_s32p[0x3f] = 0xa;
    resvars.boxes_s32p[0x40] = 0x57;
    resvars.boxes_s32p[0x41] = 0x4b;
    resvars.boxes_s32p[0x42] = 0x7a;
    resvars.boxes_s32p[0x43] = 0xa;
    resvars.boxes_s32p[0x44] = 0x57;
    resvars.boxes_s32p[0x45] = 0x56;
    resvars.boxes_s32p[0x46] = 0x7a;
    resvars.boxes_s32p[0x47] = 0xa;
    resvars.boxes_s32p[0x48] = 0x57;
    resvars.boxes_s32p[0x49] = 0x61;
    resvars.boxes_s32p[0x4a] = 0x7a;
    resvars.boxes_s32p[0x4b] = 0xa;
    resvars.boxes_s32p[0x4c] = 0x57;
    resvars.boxes_s32p[0x4d] = 0x6c;
    resvars.boxes_s32p[0x4e] = 0x7a;
    resvars.boxes_s32p[0x4f] = 0xa;
    resvars.boxes_s32p[0x50] = 0x57;
    resvars.boxes_s32p[0x51] = 0x77;
    resvars.boxes_s32p[0x52] = 0x7a;
    resvars.boxes_s32p[0x53] = 0xa;
    resvars.coords_IconScreenp->v[0xb6] = 0x3a;
    resvars.coords_IconScreenp->v[0xb7] = 0x7d;
    resvars.coords_IconScreenp->v[0xb8] = 0x3a;
    resvars.coords_IconScreenp->v[0xb9] = 0x7d;
    resvars.coords_IconScreenp->v[0xba] = 0x3a;
    resvars.coords_IconScreenp->v[0xbb] = 0x7d;
    resvars.coords_IconScreenp->v[0xbc] = 0x20;
    resvars.coords_IconScreenp->v[0xbd] = 0x49;
    resvars.coords_IconScreenp->v[0xbe] = 0x34;
    resvars.coords_IconScreenp->v[0xbf] = 0x49;
    resvars.coords_IconScreenp->v[0xc0] = 0x48;
    resvars.coords_IconScreenp->v[0xc1] = 0x49;
    resvars.boxes_s32p[0x5c] = 0xd;
    resvars.boxes_s32p[0x5d] = 0x18;
    resvars.boxes_s32p[0x5e] = 0x29;
    resvars.boxes_s32p[0x5f] = 0x29;
    resvars.coords_IconScreenp->v[0x0] = 0x1d;
    resvars.coords_IconScreenp->v[0x1] = 0x7e;
    resvars.boxes_s32p[0x8c] = 0x21;
    resvars.boxes_s32p[0x8d] = 0x3c;
    resvars.boxes_s32p[0x8e] = 0x31;
    resvars.boxes_s32p[0x8f] = 0x31;
    resvars.boxes_s32p[0x88] = 0xa;
    resvars.boxes_s32p[0x89] = 0x7a;
    resvars.boxes_s32p[0x8a] = 0x2f;
    resvars.boxes_s32p[0x8b] = 0x9f;
    resvars.boxes_s32p[0x84] = 0xa;
    resvars.boxes_s32p[0x85] = 0x80;
    resvars.boxes_s32p[0x86] = 0x2f;
    resvars.boxes_s32p[0x87] = 0xa5;
    resvars.boxes_s32p[0x80] = 0xa;
    resvars.boxes_s32p[0x81] = 0x6f;
    resvars.boxes_s32p[0x82] = 0x2f;
    resvars.boxes_s32p[0x83] = 0x94;
    resvars.coords_IconScreenp->v[0xee] = 0xb;
    resvars.coords_IconScreenp->v[0xef] = 0xb;
    resvars.coords_IconScreenp->v[0xf0] = 0xb;
    resvars.coords_IconScreenp->v[0xf1] = 0x18;
    resvars.coords_IconScreenp->v[0xf2] = 0xb;
    resvars.coords_IconScreenp->v[0xf3] = 0x26;
    resvars.coords_IconScreenp->v[0xf4] = 0xb;
    resvars.coords_IconScreenp->v[0xf5] = 0x32;
    resvars.coords_IconScreenp->v[0xf6] = 0x44;
    resvars.coords_IconScreenp->v[0xf7] = 0xb;
    resvars.coords_IconScreenp->v[0xf8] = 0x44;
    resvars.coords_IconScreenp->v[0xf9] = 0x17;
    resvars.coords_IconScreenp->v[0xfa] = 0x44;
    resvars.coords_IconScreenp->v[0xfb] = 0x24;
    resvars.coords_IconScreenp->v[0xfc] = 0x37;
    resvars.coords_IconScreenp->v[0xfd] = 0x31;
    resvars.boxes_s32p[0x94] = 0xb;
    resvars.boxes_s32p[0x95] = 0xb;
    resvars.boxes_s32p[0x96] = 0x4a;
    resvars.boxes_s32p[0x97] = 0x2b;
    resvars.coords_IconScreenp->v[0xdc] = 1;
    resvars.coords_IconScreenp->v[0xdd] = 0x54;
    resvars.coords_IconScreenp->v[0xdf] = 0xe;
    resvars.boxes_s32p[0xa0] = 0xb;
    resvars.boxes_s32p[0xa1] = 6;
    resvars.boxes_s32p[0xa2] = 0x3d;
    resvars.boxes_s32p[0xa4] = 0x4c;
    resvars.boxes_s32p[0xa5] = 6;
    resvars.boxes_s32p[0xa6] = 8;
    resvars.coords_IconScreenp->v[0xe1] = 0x48;
    resvars.coords_IconScreenp->v[0xdd] = 0x4c;
    resvars.coords_IconScreenp->v[0xe3] = 0xfffffffc;
    resvars.boxes_s32p[0x9c] = 0xc;
    resvars.boxes_s32p[0x9d] = 6;
    resvars.boxes_s32p[0x9e] = 0x3d;
    resvars.boxes_s32p[0x9f] = 8;
    resvars.coords_IconScreenp->v[0x30] = 0x25;
    resvars.coords_IconScreenp->v[0x31] = 0x7c;
    resvars.coords_IconScreenp->v[0x38] = 0x25;
    resvars.coords_IconScreenp->v[0x39] = 0x2b;
    resvars.coords_IconScreenp->v[0x10e] = 0;
    resvars.coords_IconScreenp->v[0x10f] = 0;
    resvars.coords_IconScreenp->v[0x10c] = 7;
    resvars.coords_IconScreenp->v[0x10d] = 0x6f;
    resvars.coords_IconScreenp->v[0x108] = 5;
    resvars.coords_IconScreenp->v[0x109] = 0x7a;
    resvars.boxes_s32p[0x174] = 0x14;
    resvars.boxes_s32p[0x175] = 0x1c;
    resvars.boxes_s32p[0x176] = 0x20;
    resvars.boxes_s32p[0x177] = 0x25;
    resvars.boxes_s32p[0x164] = 0x14;
    resvars.boxes_s32p[0x165] = 0x1c;
    resvars.boxes_s32p[0x166] = 0x20;
    resvars.boxes_s32p[0x167] = 0x25;
    resvars.boxes_s32p[0x168] = 0x14;
    resvars.boxes_s32p[0x169] = 0x1c;
    resvars.boxes_s32p[0x16a] = 0x20;
    resvars.boxes_s32p[0x16b] = 0x25;
    resvars.boxes_s32p[0x16c] = 0x54;
    resvars.boxes_s32p[0x16d] = 0x11;
    resvars.boxes_s32p[0x16e] = 0x18;
    resvars.boxes_s32p[0x16f] = 0x29;
    resvars.boxes_s32p[0x170] = 0x115;
    resvars.boxes_s32p[0x171] = 0x19;
    resvars.boxes_s32p[0x172] = 0x18;
    resvars.boxes_s32p[0x173] = 0x23;
    resvars.coords_IconScreenp->v[0x12] = 0x7d;
    resvars.coords_IconScreenp->v[0x13] = 0x2f;
    resvars.boxes_s32p[0x119] = 0x42;
    resvars.boxes_s32p[0x127] = 0x1a;
    resvars.boxes_s32p[0x12b] = 0x1a;
    resvars.boxes_s32p[0x124] = 0;
    resvars.boxes_s32p[0x125] = 0;
    resvars.boxes_s32p[0x128] = 0x31;
    resvars.boxes_s32p[0x129] = 0;
    resvars.boxes_s32p[0x164] = 0x29;
    resvars.boxes_s32p[0x165] = 6;
    resvars.boxes_s32p[0x166] = 0x32;
    resvars.boxes_s32p[0x167] = 0xb;
    resvars.boxes_s32p[0x11d] = 0x21;
    resvars.boxes_s32p[0x12c] = 0;
    resvars.boxes_s32p[0x12d] = 0x17;
    resvars.boxes_s32p[0x12e] = 0x63;
    resvars.boxes_s32p[0x12f] = 0x14;
    resvars.boxes_s32p[0x114] = 0x11d;
    resvars.boxes_s32p[0x115] = 0xc;
}

// Loads the pointer sprites and points resvars at the layout tables for
// screen mode `mode` (SCREEN_MODE_1 or SCREEN_MODE_13), reloading the game
// sprites, then resets the view window and rebuilds the menus.
// FUNCTION: 0x0008d77c
void swap_game_sprites(TbScreenMode mode)
{
    u8 active;

    active = lbDisplay.wscreen ? (u8)1 : (u8)0;
    if (resvars.game_files_state == 2) {
        LbSpriteReset(pointer, end_pointers, pointer_data);
        LbSpriteReset(tile, end_tiles, tile_data);
        LbSpriteReset(sprite, end_sprites, sprite_data);
        LbSpriteReset(button_sprite, end_button_sprites, button_sprite_data);
        LbSpriteReset(game_panel, end_game_panel, game_panel_data);
        LbSpriteReset(pulldv, end_pulldv, pulldv_data);
        unload_game_files();
    }
    switch (mode) {
    case SCREEN_MODE_1:
        statvar.view_height = lbScreenModeInfo[SCREEN_MODE_1].height - 0x19;
        statvar.unknowne6 = 0x17;
        statvar.unknowne7 = 0x1a;
        LbMouseChangeSprite(0);
        LbDataFreeAll(hiresgamepointerfiles);
        LbDataLoadAll(lorespointerfiles);
        LbPackHeap();
        resvars.game_mode = SCREEN_MODE_1;
        load_game_files();
        resvars.boxes_s32p = D_0014b1c4;
        resvars.screen_boxes_u8p = ScreenBoxes;
        resvars.coords_IconScreenp = &D_0014b7b4;
        resvars.screen_coords_s32p = D_00146c8c;
        resvars.tile_height = 0x10;
        resvars.tile_width = 0x20;
        resvars.window_list_u8p = D_00145108;
        resvars.button_list = mcgabutton_list;
        resvars.screen_list_pos = screenlistpos;
        resvars.buttons = D_0014bd5c;
        resvars.help_hotspots_intp = D_0014be34;
        resvars.panel_popup_coords = mcgapanel_popup_coord;
        resvars.requester_buttons = lores_requester_buttons;
        resvars.requester_sprites = lores_requester_sprites;
        resvars.font_filenames_charpp = lores_font_filenames;
        resvars.advisor_min_segments = 4;
        resvars.advisor_max_segments = 0x21;
        resvars.litter_pos = D_0012cc7c;
        mcga_coords();
        othervar.lores_checked = 1;
        othervar.hires_checked = 0;
        break;
    case SCREEN_MODE_13:
        statvar.view_height = lbScreenModeInfo[SCREEN_MODE_13].height - 0x30;
        statvar.unknowne6 = 0x1c;
        statvar.unknowne7 = 0x1f;
        LbMouseChangeSprite(0);
        LbDataFreeAll(lorespointerfiles);
        LbDataLoadAll(hiresgamepointerfiles);
        LbPackHeap();
        resvars.game_mode = SCREEN_MODE_13;
        load_game_files();
        resvars.boxes_s32p = D_0013bb0c;
        resvars.screen_boxes_u8p = ScreenBoxes;
        resvars.coords_IconScreenp = &D_0013c0fc;
        resvars.screen_coords_s32p = D_00146c8c;
        resvars.tile_height = 0x20;
        resvars.tile_width = 0x40;
        resvars.window_list_u8p = window_list;
        resvars.button_list = button_list;
        resvars.screen_list_pos = screenlistpos;
        resvars.buttons = hiresbuttons;
        resvars.help_hotspots_intp = hireshelp;
        resvars.panel_popup_coords = panel_popup_coord;
        resvars.req_filenames_names = hires_req_filenames;
        resvars.requester_buttons = hires_requester_buttons;
        resvars.requester_sprites = hires_requester_sprites;
        resvars.font_filenames_charpp = hires_font_filenames;
        resvars.advisor_min_segments = 3;
        resvars.advisor_max_segments = 0x20;
        resvars.litter_pos = D_0012cc3c;
        othervar.hires_checked = 1;
        othervar.lores_checked = 0;
        break;
    default:
        break;
    }
    resvars.loaded_mode = mode;
    window.zone0_x = 0;
    window.zone0_x1 = lbScreenModeInfo[mode].width;
    window.zone0_y = 0;
    window.zone0_y1 = lbScreenModeInfo[mode].height;
    window.anim_mode0 = 0;
    window.center0_x = (window.zone0_x + window.zone0_x1) / 2;
    window.center0_y = (window.zone0_y + window.zone0_y1) / 2;
    window.current_window = 0;
    setup_menus();
    LbSpriteSetup(pointer, end_pointers, pointer_data);
}

// Switches the game to screen mode `mode`, keeping the view centred on the
// same cell. Exits the program if the mode cannot be set.
// FUNCTION: 0x0008db38
extern "C" void change_screen_mode(TbScreenMode mode)
{
    u8 active;
    s32 cell;

    active = lbDisplay.wscreen ? (u8)1 : (u8)0;
    find_cell_at_point(window.current_window, window.center_x[window.current_window], window.center_y[window.current_window]);
    cell = info.cell;
    D_001290d0 = 0xff;
    D_001290d1 = 0xff;
    D_001290d2 = 0xff;
    if (active)
        LbScreenUnlock();
    if (!change_res(mode)) {
        error_number = 10;
        sub_error_number = resvars.screen_mode;
        reset_host();
        exit(1);
    }
    swap_game_sprites(mode);
    LbSpriteSetup(pointer, end_pointers, pointer_data);
    LbMouseSetup(&pointer[othervar.pointer_sprite]);
    D_00156910 = 1;
    othervar.full_redraw = 1;
    set_all_refresh_zones_in_window(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height, 3);
    if (active) {
        while (LbScreenLock() != 1)
            ;
    }
    LbScreenClear(0);
    precalculate_shadow_blocks();
    goto_cell_in_window(window.current_window, cell);
}

// Asks the player to confirm restarting the level (single player only).
// FUNCTION: 0x0008dc7c
extern "C" void ask_restart_level(void)
{
    if (statvar.num_players <= 1) {
        fn_000bbff0(othervar.yesno_msg, yesno_text[6]);
        open_yes_no_requester(EV_RESTART_LEVEL, 0, 0, EV_CLOSE_YES_NO, 1, 0);
    }
}
