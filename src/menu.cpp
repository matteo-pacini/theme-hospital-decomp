#include "hosp.h"

struct IconScreen {
    union {
        s32 v[0x156];
        struct {
            u8 unknown0[0x54c];
            s32 unknown54c;
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

extern char **D_001602f0;
extern char D_00120824[], D_00120826[];
extern u16 D_00160f18, D_00160f1a;
extern char D_00120829[];
extern Event D_001549d0[];
void FadeMusic(u32 time, u8 volume);
extern CompElement *D_00153f70;
extern char D_0012082c[], D_00120834[];
extern u8 D_001290d0, D_001290d1, D_001290d2;
extern u8 D_0015f982;
extern TbSprite *D_00153f30;
extern u8 *D_00153f28;
extern u8 D_00156910;
void fn_000c93bf(char *dst, char *src);
int fn_000bed50(void);
void fn_0008db38(TbScreenMode mode);
extern s32 D_0014b1c4[], D_0013bb0c[], D_00146c8c[];
extern IconScreen D_0014b7b4, D_0013c0fc;
extern u8 D_0014619c[], D_00145108[], D_0013fd80[], D_00145358[], D_0014bd5c[], D_0014d414[];
extern u8 D_0014a810[], D_0014adda[], D_0012cc7c[];
extern u8 D_001361ac[], D_001310e4[], D_0013c6a4[], D_00130288[], D_0013e5fc[], D_0014a620[], D_0014a9fe[], D_0012cc3c[];
extern int D_0014be34[], D_0013e6f4[], D_0013c77c[], D_0013e634[];
extern TbSprite *D_00153f44, *D_00153f48, *D_00153f24, *D_00153f88, *D_00153f94;
extern u8 *D_00153f40, *D_00153f1c, *D_00153f80, *D_00153f8c;
extern TbLoadFiles D_0013e22c[], D_0013e2b8[];
void LbSpriteReset(TbSprite *start, TbSprite *end, void *data);
int fn_000bce44(TbLoadFiles *files);

int strcmp(const char *a, const char *b);
void LbTextSetClipWindow(u32 x, u32 y, u32 w, u32 h);

}

// FUNCTION: 0x00089df0
void setup_menus(void)
{
    u8 done;
    u8 sub;
    u8 i;
    u8 j;
    char sep[2];
    char end[3];

    pmenu.unknown0 = 0;
    pmenu.font = 1;
    pmenu.font_hi = 2;
    pmenu.unknown7 = 0;
    set_font_from_cache(pmenu.font);
    pmenu.line_height = LbTextCharHeight('A') + resvars.f1c_IconScreenp->unknown54c;
    pmenu.char_width = LbTextCharWidth('A');
    set_font_from_cache(0);
    sprintf(sep, D_00120824);
    sprintf(end, D_00120826);
    for (i = 1; (pmenu.pulldowns[i].length = fn_000cd716(D_001602f0[i])), strcmp(sep, D_001602f0[i]); i++)
        ;
    pmenu.count = i;
    i++;
    for (j = 1; j < pmenu.count; j++) {
        pmenu.pulldowns[j].width = 0;
        for (sub = 0;; sub++) {
            if (strcmp(sep, D_001602f0[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                break;
            }
            if (fn_000cd716(D_001602f0[i]) > pmenu.pulldowns[j].width)
                pmenu.pulldowns[j].width = fn_000cd716(D_001602f0[i]);
            i++;
        }
    }
    for (done = 0; !done; j++) {
        pmenu.pulldowns[j].width = 0;
        for (sub = 0;; sub++) {
            if (strcmp(end, D_001602f0[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                done = 1;
                break;
            }
            if (strcmp(sep, D_001602f0[i]) == 0) {
                pmenu.pulldowns[j].count = sub;
                i++;
                break;
            }
            if (fn_000cd716(D_001602f0[i]) > pmenu.pulldowns[j].width)
                pmenu.pulldowns[j].width = fn_000cd716(D_001602f0[i]);
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

// FUNCTION: 0x0008a1f0
void draw_menus(void)
{
    u8 h1;
    u8 h2;
    u8 h3;
    u8 w;
    u8 y0;
    u16 i;
    u16 last;
    u16 x;

    y0 = 0;
    h1 = pulldv[1].height;
    h2 = pulldv[4].height;
    h3 = pulldv[7].height * 2;
    if ((y0 + h1 + h2 + h3 > lbDisplay.mmouse_y || pmenu.unknown0) && !pmenu.unknown7) {
        othervar.f8a = 1;
        lbDisplay.draw_flags = 0;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        LbTextSetClipWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        w = pulldv[1].width;
        last = lbDisplay.graphics_screen_width - w;
        reqspr_draw(0, y0, &pulldv[1]);
        reqspr_draw(0, h1, &pulldv[4]);
        reqspr_draw(0, h1 + h2, &pulldv[7]);
        for (i = w; i < lbDisplay.graphics_screen_width - w; i += w) {
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
                LbTextDraw(x, resvars.f1c_IconScreenp->unknown54c, D_001602f0[pmenu.pulldowns[i].text]);
                if (lbDisplay.mleft_button) {
                    if (pmenu.unknown0 == i)
                        draw_pulldown(x, (u16)pmenu.line_height, i);
                }
                x += pmenu.pulldowns[i].length * pmenu.char_width;
            }
        }
        if (pmenu.unknown0 >= pmenu.count) {
            if (lbDisplay.mleft_button) {
            x = 0;
            for (i = 1; i < pmenu.pulldowns[pmenu.unknown0].parent; i++)
                x += pmenu.pulldowns[i].length * pmenu.char_width;
            draw_pulldown(x, (u16)pmenu.line_height, (u16)pmenu.pulldowns[pmenu.unknown0].parent);
            x += pmenu.char_width * pmenu.pulldowns[i].width + pulldv[6].width;
            draw_pulldown(x, pmenu.pulldowns[pmenu.unknown0].y, (u16)pmenu.unknown0);
            }
        }
        set_font_from_cache(0);
    } else {
        othervar.f8a = 0;
    }
}

// FUNCTION: 0x0008a618
void draw_pulldown(u16 x, u16 y, u16 idx)
{
    u8 i;
    u16 saved;
    u16 saved2;

    draw_pmenu_box(x, y, pmenu.pulldowns[idx].width * pmenu.char_width, pmenu.pulldowns[idx].count * pmenu.line_height);
    set_all_refresh_zones_in_window(x, y, x + pmenu.pulldowns[idx].width * pmenu.char_width, y + pmenu.pulldowns[idx].count * pmenu.line_height, 3);
    for (i = 0; i < pmenu.pulldowns[idx].count; i++) {
        if (!(pmenu.pulldowns[idx].items[i].flags & 1) || statvar.unknownfd <= 1) {
            if (pmenu.pulldowns[idx].items[i].check) {
                if (*pmenu.pulldowns[idx].items[i].check)
                    reqspr_draw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, &pulldv[10]);
            }
            if (pmenu.pulldowns[idx].items[i].check2) {
                if (*pmenu.pulldowns[idx].items[i].check2)
                    reqspr_draw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, &pulldv[10]);
            }
        }
        if ((pmenu.pulldowns[idx].items[i].flags & 1) && statvar.unknownfd > 1) {
            saved = lbDisplay.draw_flags;
            lbDisplay.draw_flags = 4;
            LbTextDraw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, D_001602f0[pmenu.pulldowns[idx].items[i].text]);
            lbDisplay.draw_flags = saved;
        } else {
            LbTextDraw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, D_001602f0[pmenu.pulldowns[idx].items[i].text]);
        }
        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, i * pmenu.line_height + y, pmenu.pulldowns[idx].width * pmenu.char_width, pmenu.line_height)) {
            set_font_from_cache(pmenu.font_hi);
            if (D_00160f1a != i + 1 || D_00160f18 != idx) {
                D_00160f18 = idx;
                play_sound(10, 0x44, -1, -1);
            }
            D_00160f1a = i + 1;
            if ((pmenu.pulldowns[idx].items[i].flags & 1) && statvar.unknownfd > 1) {
                saved2 = lbDisplay.draw_flags;
                lbDisplay.draw_flags = 4;
                LbTextDraw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, D_001602f0[pmenu.pulldowns[idx].items[i].text]);
                lbDisplay.draw_flags = saved2;
            } else {
                LbTextDraw(x, y + i * pmenu.line_height + resvars.f1c_IconScreenp->unknown54c, D_001602f0[pmenu.pulldowns[idx].items[i].text]);
            }
            set_font_from_cache(pmenu.font);
        }
    }
}

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

// FUNCTION: 0x0008add8
extern "C" void fn_0008add8(void)
{
    u8 h1;
    u8 y0;
    u8 h2;
    u16 i;
    u16 j;
    u16 x2;
    u16 x;
    u16 y;
    u16 n;

    y0 = 0;
    h1 = pulldv[1].height;
    h2 = pulldv[4].height;
    if (lbDisplay.rleft_button && !pmenu.unknown7) {
        x = 0;
        if (pmenu.unknown0 >= pmenu.count)
            y = pmenu.pulldowns[pmenu.unknown0].y;
        else
            y = pmenu.line_height;
        n = 0;
        for (i = 1; i < pmenu.count + pmenu.subcount; i++) {
            if (pmenu.pulldowns[i].active) {
                if (pmenu.unknown0 == i) {
                    for (j = 0; j < pmenu.pulldowns[i].count; j++) {
                        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, x, pmenu.line_height * j + y, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.line_height)) {
                            debug.unknown8 = n;
                            othervar.f40 = n;
                            if (!(((pmenu.pulldowns[i].items[j].flags & 1) && statvar.unknownfd > 1) || (!pmenu.pulldowns[i].items[j].f0 && gamevar.unknown21))) {
                                if (gamevar.f174 && !pmenu.pulldowns[i].items[j].f1) {
                                    play_sound(10, 8, -1, -1);
                                } else {
                                    if (pmenu.pulldowns[i].items[j].check) {
                                        if (*pmenu.pulldowns[i].items[j].check)
                                            *pmenu.pulldowns[i].items[j].check = 0;
                                        else
                                            *pmenu.pulldowns[i].items[j].check = 1;
                                    }
                                    if (pmenu.pulldowns[i].items[j].key)
                                        fn_00074220(pmenu.pulldowns[i].items[j].shift, pmenu.pulldowns[i].items[j].key);
                                    if (pmenu.pulldowns[i].items[j].callback) {
                                        othervar.f8f = pmenu.pulldowns[i].items[j].arg;
                                        othervar.f90 = j;
                                        pmenu.pulldowns[i].items[j].callback();
                                    }
                                    play_sound(10, 0x43, -1, -1);
                                }
                            } else {
                                play_sound(10, 8, -1, -1);
                            }
                            break;
                        }
                        n++;
                    }
                } else {
                    n += pmenu.pulldowns[i].count;
                }
                if (pmenu.unknown0 < pmenu.count) {
                    x += pmenu.pulldowns[i].length * pmenu.char_width;
                } else {
                    x = 0;
                    for (j = 1; j < pmenu.pulldowns[pmenu.unknown0].parent; j++)
                        x += pmenu.pulldowns[j].length * pmenu.char_width;
                    x += pmenu.pulldowns[j].width * pmenu.char_width + pulldv[6].width;
                }
            }
        }
        lbDisplay.rleft_button = 0;
    }
    if (lbDisplay.mleft_button && !pmenu.unknown7) {
        if (y0 + h1 + h2 > lbDisplay.mmouse_y) {
            x2 = 0;
            x = pmenu.char_width * pmenu.pulldowns[1].length;
            for (i = 1; i < pmenu.count; i++) {
                if (pmenu.pulldowns[i].active) {
                    if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x2, 0, x, pmenu.line_height)) {
                        if (pmenu.unknown0 != i)
                            play_sound(10, 0x44, -1, -1);
                        pmenu.unknown0 = i;
                        break;
                    }
                    x2 += x;
                    x = pmenu.pulldowns[i + 1].length * pmenu.char_width;
                }
            }
        } else {
            if (pmenu.unknown0 >= pmenu.count) {
                x = 0;
                for (i = 1; i < pmenu.pulldowns[pmenu.unknown0].parent; i++) {
                    if (pmenu.pulldowns[i].active)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                }
                i = pmenu.pulldowns[pmenu.unknown0].parent;
                if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, pmenu.line_height, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.pulldowns[i].count * pmenu.line_height))
                    pmenu.unknown0 = pmenu.pulldowns[pmenu.unknown0].parent;
            }
            if (pmenu.unknown0 && pmenu.pulldowns[pmenu.unknown0].active) {
                if (pmenu.pulldowns[pmenu.unknown0].parent) {
                    x = 0;
                    for (i = 1; i < pmenu.pulldowns[pmenu.unknown0].parent; i++)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                } else {
                    x = 0;
                    for (i = 1; i < pmenu.unknown0; i++)
                        x += pmenu.pulldowns[i].length * pmenu.char_width;
                }
                i = pmenu.unknown0;
                for (j = 0; j < pmenu.pulldowns[i].count; j++) {
                    if ((pmenu.pulldowns[i].items[j].flags & 1) && statvar.unknownfd > 1)
                        continue;
                    if (pmenu.pulldowns[i].items[j].submenu) {
                        if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, x, pmenu.line_height + pmenu.line_height * j, pmenu.pulldowns[i].width * pmenu.char_width, pmenu.line_height)) {
                            pmenu.unknown0 = pmenu.pulldowns[i].items[j].submenu;
                            break;
                        }
                    }
                }
            }
        }
    } else {
        pmenu.unknown0 = 0;
    }
}

// FUNCTION: 0x0008b6a0
extern "C" void fn_0008b6a0(void)
{
    u8 n;

    n = othervar.f90 + 1;
    if (n < 10) {
        debug.unknown8 = n;
        D_001549d0[player].type = 0xd;
        D_001549d0[player].value = n;
    }
}

// FUNCTION: 0x0008b6f8
extern "C" void fn_0008b6f8(void)
{
    u8 n;

    n = othervar.f90 + 1;
    debug.unknown8 = n;
    if (othervar.fa0e[n] == 1) {
        sprintf(othervar.f935, D_00120829, yesno_text[3]);
        open_yes_no_requester(0xc, (u16)n, 0, 0x8c, 0x63, 0);
    } else {
        D_001549d0[player].type = 0xc;
        D_001549d0[player].value = n;
        if (n < 10)
            othervar.fa0e[n] = 1;
    }
}

// FUNCTION: 0x0008b7b4
void pulldown_briefing(void)
{
    if (gamevar.f12e == 0xe)
        play_sound(10, 8, -1, -1);
    else if (statvar.unknownfd <= 1)
        show_help_page(1, level_number);
    else
        show_help_page(2, current_net_criteria.unknown0);
}

// FUNCTION: 0x0008b824
extern "C" void fn_0008b824(void)
{
    if (myplayer.f1d > 0) {
        myplayer.f1d = 0;
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        othervar.f10 = 1;
    } else {
        myplayer.f1d = 1;
        add_static_shadows();
        set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
        othervar.f10 = 1;
    }
}

// FUNCTION: 0x0008b8a0
extern "C" void fn_0008b8a0(void)
{
    u8 sel;
    u8 i;

    sel = othervar.f90;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.fa19[i] = 0;
        othervar.fa19[sel] = 1;
        myplayer.fd = (10 - sel) * 0x7f / 10;
    }
}

// FUNCTION: 0x0008b91c
extern "C" void fn_0008b91c(void)
{
    u8 i;
    u32 sel;

    sel = othervar.f90;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.fa2f[i] = 0;
        othervar.fa2f[sel] = 1;
        myplayer.fc = (10 - sel) * 0x7f / 10;
        FadeMusic(1000, myplayer.fc);
    }
}

// FUNCTION: 0x0008b99c
extern "C" void fn_0008b99c(void)
{
    u8 sel;
    u8 i;

    sel = othervar.f90;
    if (sel < 11) {
        for (i = 0; i < 10; i++)
            othervar.fa24[i] = 0;
        othervar.fa24[sel] = 1;
        myplayer.fe = (10 - sel) * 0x7f / 10;
    }
}

// FUNCTION: 0x0008ba18
extern "C" void fn_0008ba18(void)
{
    if (statvar.unknowne5) {
        myplayer.unknown11 = !myplayer.unknown11 ? (u8)1 : (u8)0;
        if (myplayer.unknown11)
            play_midi(othervar.f9ba);
        else
            stop_midi();
    } else {
        myplayer.unknown11 = 0;
    }
}

// FUNCTION: 0x0008ba78
extern "C" void fn_0008ba78(void)
{
    if (statvar.fe4)
        myplayer.ff = !myplayer.ff ? (u8)1 : (u8)0;
    else
        myplayer.ff = 0;
    if (myplayer.ff == 0)
        StopAllSamples();
}

// FUNCTION: 0x0008bacc
extern "C" void fn_0008bacc(void)
{
    if (statvar.fe4)
        myplayer.f10 = !myplayer.f10 ? (u8)1 : (u8)0;
    else
        myplayer.f10 = 0;
}

// FUNCTION: 0x0008bb10
extern "C" void fn_0008bb10(void)
{
    u8 sel;
    u8 i;

    sel = othervar.f90;
    if (sel < 5) {
        for (i = 0; i < 5; i++)
            othervar.fa3a[i] = 0;
        othervar.fa3a[sel] = 1;
        myplayer.f20 = sel;
        D_001549d0[player].type = 0xe;
        D_001549d0[player].value = othervar.f8f;
    }
}

// FUNCTION: 0x0008bb98
extern "C" void fn_0008bb98(void)
{
    if (!((resvars.f0 == SCREEN_MODE_1 && othervar.f90 == 1) || (resvars.f0 == SCREEN_MODE_13 && othervar.f90 == 0))) {
        if (!(crequester.unknown2 == 5 || (crequester.unknown2 == 10 && crequester.unknown4 != 1))) {
            if (crequester.unknown2 != 0x63)
                close_crequester(crequester.unknown2);
            close_irequesters();
            advisor.f0 = 0;
            if (resvars.f1 == SCREEN_MODE_13) {
                if (othervar.fa4f)
                    AIL_stop_sequence(GetSongHandle());
                fn_0008db38(SCREEN_MODE_1);
                if (othervar.fa4f)
                    AIL_resume_sequence(GetSongHandle());
            } else if (resvars.f1 == SCREEN_MODE_1) {
                if (othervar.fa4f)
                    AIL_stop_sequence(GetSongHandle());
                fn_0008db38(SCREEN_MODE_13);
                if (othervar.fa4f)
                    AIL_resume_sequence(GetSongHandle());
            }
        }
    }
}

// FUNCTION: 0x0008bcb4
extern "C" void fn_0008bcb4(void)
{
    CompElement *e;

    e = element;
    while (e <= D_00153f70) {
        e->x = e->x / 2 + 0x46;
        e->y = e->y / 2 + 0x65;
        e++;
    }
}

// FUNCTION: 0x0008bd24
void check_save_games(void)
{
    char buf[100];
    s32 handle;
    u8 i;

    for (i = 0; i < 10; i++) {
        othervar.fa0e[i] = 0;
        sprintf(buf, D_0012082c, statvar.f575);
        fn_000c93bf(buf, myplayer.unknown0);
        sprintf(buf, D_00120834, buf, i);
        handle = LbFileOpen(buf, 2);
        if (handle != -1) {
            othervar.fa0e[i] = 1;
            LbFileClose(handle);
        }
    }
}

// FUNCTION: 0x0008bdd4
extern "C" void fn_0008bdd4(void)
{
    u8 i;

    switch (resvars.f0) {
    case SCREEN_MODE_1:
        othervar.fa41 = 0;
        othervar.fa40 = 1;
        break;
    case SCREEN_MODE_13:
        othervar.fa40 = 0;
        othervar.fa41 = 1;
        break;
    default:
        break;
    }
    check_save_games();
    for (i = 0; i < 10; i++) {
        othervar.fa19[i] = 0;
        othervar.fa24[i] = 0;
        othervar.fa2f[i] = 0;
    }
    othervar.fa19[9 - myplayer.fd * 10 / 128] = 1;
    othervar.fa24[9 - myplayer.fe * 10 / 128] = 1;
    othervar.fa2f[9 - myplayer.fc * 10 / 128] = 1;
    for (i = 0; i < 5; i++)
        othervar.fa3a[i] = 0;
    othervar.fa3a[myplayer.f20] = 1;
}


// FUNCTION: 0x0008bf24
void mcga_coords(void)
{
    resvars.f1c_IconScreenp->v[0x110] = 2;
    resvars.f1c_IconScreenp->v[0x111] = 2;
    resvars.f1c_IconScreenp->v[0x98] = 0x3d;
    resvars.f1c_IconScreenp->v[0x99] = 0xc4;
    resvars.f1c_IconScreenp->v[0x96] = 0x1c;
    resvars.f1c_IconScreenp->v[0x97] = 0xc2;
    resvars.f1c_IconScreenp->v[0x132] = 6;
    resvars.f1c_IconScreenp->v[0x134] = 2;
    resvars.f1c_IconScreenp->v[0x138] = 8;
    resvars.f1c_IconScreenp->v[0x136] = 7;
    resvars.f1c_IconScreenp->v[0x13a] = 0x13;
    resvars.f1c_IconScreenp->v[0x153] = 1;
    resvars.f1c_IconScreenp->v[0x127] = 0x90;
    resvars.f1c_IconScreenp->v[0x13c] = 0x118;
    resvars.f1c_IconScreenp->v[0x13d] = 0x90;
    resvars.f1c_IconScreenp->v[0x12d] = 0x93;
    resvars.f1c_IconScreenp->v[0x12f] = 0x98;
    resvars.f1c_IconScreenp->v[0x131] = 0x9d;
    resvars.f18_s32p[0x104] = 0x27;
    resvars.f18_s32p[0x105] = 0x6e;
    resvars.f18_s32p[0x106] = 0x58;
    resvars.f18_s32p[0x107] = 0x1c;
    resvars.f1c_IconScreenp->v[0xa4] = 0x26;
    resvars.f1c_IconScreenp->v[0xa5] = 9;
    resvars.f1c_IconScreenp->v[0xa6] = 0x5d;
    resvars.f1c_IconScreenp->v[0xa7] = 0x40;
    resvars.f1c_IconScreenp->v[0x14c] = 1;
    resvars.f1c_IconScreenp->v[0x14d] = 1;
    resvars.f1c_IconScreenp->v[0x9c] = 0xe;
    resvars.f1c_IconScreenp->v[0x9d] = 0x1a;
    resvars.f1c_IconScreenp->v[0x9e] = 0xe;
    resvars.f1c_IconScreenp->v[0x9f] = 0x43;
    resvars.f1c_IconScreenp->v[0xa0] = 0xe;
    resvars.f1c_IconScreenp->v[0xa1] = 0x6c;
    resvars.f1c_IconScreenp->v[0xa2] = 0xe;
    resvars.f1c_IconScreenp->v[0xa3] = 0x95;
    resvars.f40_s32p[0x128] = 0x15;
    resvars.f1c_IconScreenp->v[0x6] = 0x3d;
    resvars.f1c_IconScreenp->v[0x7] = 0x40;
    resvars.f1c_IconScreenp->v[0x8] = 0x1c;
    resvars.f1c_IconScreenp->v[0x9] = 0x5e;
    resvars.f1c_IconScreenp->v[0x4] = 9;
    resvars.f1c_IconScreenp->v[0x5] = 9;
    resvars.f1c_IconScreenp->v[0x0] = 0x1d;
    resvars.f1c_IconScreenp->v[0x1] = 0x6a;
    resvars.f1c_IconScreenp->v[0xa8] = 0x39;
    resvars.f1c_IconScreenp->v[0xa9] = 0x5e;
    resvars.f18_s32p[0x15c] = 0xa;
    resvars.f18_s32p[0x15d] = 0x45;
    resvars.f18_s32p[0x15e] = 0x5a;
    resvars.f18_s32p[0x15f] = 0x14;
    resvars.f18_s32p[0x160] = 0x39;
    resvars.f18_s32p[0x161] = 0x6d;
    resvars.f18_s32p[0x162] = 0x26;
    resvars.f18_s32p[0x163] = 0x15;
    resvars.f1c_IconScreenp->v[0xaa] = 0x2e;
    resvars.f1c_IconScreenp->v[0xab] = 0x28;
    resvars.f1c_IconScreenp->v[0xac] = 0x39;
    resvars.f1c_IconScreenp->v[0xad] = 0x28;
    resvars.f1c_IconScreenp->v[0xae] = 0x49;
    resvars.f1c_IconScreenp->v[0xaf] = 0x28;
    resvars.f1c_IconScreenp->v[0xb0] = 0x58;
    resvars.f1c_IconScreenp->v[0xb1] = 0x49;
    resvars.f1c_IconScreenp->v[0xb2] = 0x61;
    resvars.f1c_IconScreenp->v[0xb3] = 0x49;
    resvars.f1c_IconScreenp->v[0xb4] = 0x6e;
    resvars.f1c_IconScreenp->v[0xb5] = 0x49;
    resvars.f1c_IconScreenp->v[0x2c] = 0x59;
    resvars.f1c_IconScreenp->v[0x2d] = 0xa;
    resvars.f1c_IconScreenp->v[0x4c] = 0xa;
    resvars.f1c_IconScreenp->v[0x4d] = 0xa;
    resvars.f1c_IconScreenp->v[0x4e] = 0xa;
    resvars.f1c_IconScreenp->v[0x4f] = 0x1c;
    resvars.f1c_IconScreenp->v[0x50] = 0xa;
    resvars.f1c_IconScreenp->v[0x51] = 0x2e;
    resvars.f1c_IconScreenp->v[0x52] = 0xa;
    resvars.f1c_IconScreenp->v[0x53] = 0x40;
    resvars.f1c_IconScreenp->v[0x2e] = 0x59;
    resvars.f1c_IconScreenp->v[0x2f] = 0x7a;
    resvars.f1c_IconScreenp->v[0x54] = 0x59;
    resvars.f1c_IconScreenp->v[0x55] = 0x16;
    resvars.f1c_IconScreenp->v[0x56] = 0x59;
    resvars.f1c_IconScreenp->v[0x57] = 0x21;
    resvars.f1c_IconScreenp->v[0x58] = 0x59;
    resvars.f1c_IconScreenp->v[0x59] = 0x2c;
    resvars.f1c_IconScreenp->v[0x5a] = 0x59;
    resvars.f1c_IconScreenp->v[0x5b] = 0x37;
    resvars.f1c_IconScreenp->v[0x5c] = 0x59;
    resvars.f1c_IconScreenp->v[0x5d] = 0x42;
    resvars.f1c_IconScreenp->v[0x5e] = 0x59;
    resvars.f1c_IconScreenp->v[0x5f] = 0x4d;
    resvars.f1c_IconScreenp->v[0x60] = 0x59;
    resvars.f1c_IconScreenp->v[0x61] = 0x58;
    resvars.f1c_IconScreenp->v[0x62] = 0x59;
    resvars.f1c_IconScreenp->v[0x63] = 0x63;
    resvars.f1c_IconScreenp->v[0x64] = 0x59;
    resvars.f1c_IconScreenp->v[0x65] = 0x6e;
    resvars.f1c_IconScreenp->v[0x66] = 0x59;
    resvars.f1c_IconScreenp->v[0x67] = 0x79;
    resvars.f18_s32p[0x4] = 0x59;
    resvars.f18_s32p[0x5] = 0x16;
    resvars.f18_s32p[0x6] = 0x62;
    resvars.f18_s32p[0x7] = 7;
    resvars.f18_s32p[0x8] = 0x59;
    resvars.f18_s32p[0x9] = 0x21;
    resvars.f18_s32p[0xa] = 0x62;
    resvars.f18_s32p[0xb] = 7;
    resvars.f18_s32p[0xc] = 0x59;
    resvars.f18_s32p[0xd] = 0x2c;
    resvars.f18_s32p[0xe] = 0x62;
    resvars.f18_s32p[0xf] = 7;
    resvars.f18_s32p[0x10] = 0x59;
    resvars.f18_s32p[0x11] = 0x37;
    resvars.f18_s32p[0x12] = 0x62;
    resvars.f18_s32p[0x13] = 7;
    resvars.f18_s32p[0x14] = 0x59;
    resvars.f18_s32p[0x15] = 0x42;
    resvars.f18_s32p[0x16] = 0x62;
    resvars.f18_s32p[0x17] = 7;
    resvars.f18_s32p[0x18] = 0x59;
    resvars.f18_s32p[0x19] = 0x4d;
    resvars.f18_s32p[0x1a] = 0x62;
    resvars.f18_s32p[0x1b] = 7;
    resvars.f18_s32p[0x1c] = 0x59;
    resvars.f18_s32p[0x1d] = 0x58;
    resvars.f18_s32p[0x1e] = 0x62;
    resvars.f18_s32p[0x1f] = 7;
    resvars.f18_s32p[0x20] = 0x59;
    resvars.f18_s32p[0x21] = 0x63;
    resvars.f18_s32p[0x22] = 0x62;
    resvars.f18_s32p[0x23] = 7;
    resvars.f18_s32p[0x24] = 0x59;
    resvars.f18_s32p[0x25] = 0x6e;
    resvars.f18_s32p[0x26] = 0x62;
    resvars.f18_s32p[0x27] = 7;
    resvars.f18_s32p[0x28] = 0x59;
    resvars.f18_s32p[0x29] = 0x79;
    resvars.f18_s32p[0x2a] = 0x62;
    resvars.f18_s32p[0x2b] = 7;
    resvars.f1c_IconScreenp->v[0xc2] = 0xb;
    resvars.f1c_IconScreenp->v[0xc3] = 0xb;
    resvars.f1c_IconScreenp->v[0xc4] = 0x21;
    resvars.f1c_IconScreenp->v[0xc5] = 0x1e;
    resvars.f18_s32p[0x14c] = 0x23;
    resvars.f18_s32p[0x14d] = 0x35;
    resvars.f18_s32p[0x14e] = 0x15;
    resvars.f1c_IconScreenp->v[0xd6] = 0x3d;
    resvars.f1c_IconScreenp->v[0xd7] = 0x74;
    resvars.f1c_IconScreenp->v[0xca] = 0x13;
    resvars.f1c_IconScreenp->v[0xcb] = 0x42;
    resvars.f1c_IconScreenp->v[0xce] = 0x4b;
    resvars.f1c_IconScreenp->v[0xcf] = 0x1f;
    resvars.f1c_IconScreenp->v[0xd2] = 0x15;
    resvars.f1c_IconScreenp->v[0xd0] = 0x4b;
    resvars.f1c_IconScreenp->v[0xd1] = 0x2d;
    resvars.f1c_IconScreenp->v[0xd4] = 0x4b;
    resvars.f1c_IconScreenp->v[0xd5] = 0x3b;
    resvars.f1c_IconScreenp->v[0xe4] = 0x17;
    resvars.f1c_IconScreenp->v[0xe5] = 0x68;
    resvars.f1c_IconScreenp->v[0xe6] = 0x38;
    resvars.f1c_IconScreenp->v[0xe7] = 0x68;
    resvars.f1c_IconScreenp->v[0xe8] = 0x59;
    resvars.f1c_IconScreenp->v[0xe9] = 0x68;
    resvars.f1c_IconScreenp->v[0xcc] = 0x3d;
    resvars.f1c_IconScreenp->v[0xcd] = 0x63;
    resvars.f18_s32p[0x68] = 0x13;
    resvars.f18_s32p[0x69] = 0x57;
    resvars.f18_s32p[0x6c] = 0x1e;
    resvars.f18_s32p[0x6d] = 0x57;
    resvars.f18_s32p[0x70] = 0x2d;
    resvars.f18_s32p[0x71] = 0x57;
    resvars.f18_s32p[0x74] = 0x4c;
    resvars.f18_s32p[0x75] = 0x4e;
    resvars.f18_s32p[0x78] = 0x58;
    resvars.f18_s32p[0x79] = 0x4e;
    resvars.f18_s32p[0x7c] = 0x64;
    resvars.f18_s32p[0x7d] = 0x4e;
    resvars.f18_s32p[0x100] = 0x12;
    resvars.f18_s32p[0x101] = 0x47;
    resvars.f18_s32p[0x102] = 0x5f;
    resvars.f18_s32p[0x103] = 0x15;
    resvars.f1c_IconScreenp->v[0xc8] = 0x13;
    resvars.f1c_IconScreenp->v[0xc9] = 0xc;
    resvars.f18_s32p[0x138] = 0xa;
    resvars.f18_s32p[0x139] = 0x6f;
    resvars.f18_s32p[0x13a] = 0x25;
    resvars.f18_s32p[0x13b] = 0x25;
    resvars.f18_s32p[0x154] = 0x22;
    resvars.f18_s32p[0x155] = 0x50;
    resvars.f18_s32p[0x156] = 0x15;
    resvars.f1c_IconScreenp->v[0xd8] = 0x22;
    resvars.f1c_IconScreenp->v[0xd9] = 0x5e;
    resvars.f1c_IconScreenp->v[0xda] = 0x22;
    resvars.f1c_IconScreenp->v[0xdb] = 0x6c;
    resvars.f18_s32p[0x13c] = 0xa;
    resvars.f18_s32p[0x13d] = 0x7a;
    resvars.f18_s32p[0x13e] = 0x25;
    resvars.f18_s32p[0x13f] = 0x25;
    resvars.f18_s32p[0x148] = 0x46;
    resvars.f18_s32p[0x149] = 0x79;
    resvars.f18_s32p[0x14a] = 0x20;
    resvars.f18_s32p[0x14b] = 0x21;
    resvars.f18_s32p[0xf8] = 0x48;
    resvars.f18_s32p[0xf9] = 0x63;
    resvars.f18_s32p[0xfa] = 0x1e;
    resvars.f18_s32p[0xfb] = 0x16;
    resvars.f18_s32p[0xfc] = 0x58;
    resvars.f18_s32p[0xfd] = 0x36;
    resvars.f18_s32p[0xfe] = 0xf;
    resvars.f18_s32p[0xff] = 0x11;
    resvars.f18_s32p[0x144] = 0x48;
    resvars.f18_s32p[0x145] = 0x4b;
    resvars.f18_s32p[0x146] = 0x1e;
    resvars.f18_s32p[0x147] = 0x16;
    resvars.f1c_IconScreenp->v[0x154] = 0x14;
    resvars.f1c_IconScreenp->v[0x155] = 0;
    resvars.f1c_IconScreenp->v[0x32] = 0x59;
    resvars.f1c_IconScreenp->v[0x33] = 9;
    resvars.f1c_IconScreenp->v[0x34] = 0xa;
    resvars.f1c_IconScreenp->v[0x35] = 0x5e;
    resvars.f1c_IconScreenp->v[0x36] = 0xa;
    resvars.f1c_IconScreenp->v[0x37] = 0x75;
    resvars.f1c_IconScreenp->v[0x68] = 0x59;
    resvars.f1c_IconScreenp->v[0x69] = 0x15;
    resvars.f1c_IconScreenp->v[0x6a] = 0x59;
    resvars.f1c_IconScreenp->v[0x6b] = 0x20;
    resvars.f1c_IconScreenp->v[0x6c] = 0x59;
    resvars.f1c_IconScreenp->v[0x6d] = 0x2b;
    resvars.f1c_IconScreenp->v[0x6e] = 0x59;
    resvars.f1c_IconScreenp->v[0x6f] = 0x36;
    resvars.f1c_IconScreenp->v[0x70] = 0x59;
    resvars.f1c_IconScreenp->v[0x71] = 0x41;
    resvars.f1c_IconScreenp->v[0x72] = 0x59;
    resvars.f1c_IconScreenp->v[0x73] = 0x4c;
    resvars.f1c_IconScreenp->v[0x74] = 0x59;
    resvars.f1c_IconScreenp->v[0x75] = 0x57;
    resvars.f1c_IconScreenp->v[0x76] = 0x59;
    resvars.f1c_IconScreenp->v[0x77] = 0x62;
    resvars.f1c_IconScreenp->v[0x78] = 0x59;
    resvars.f1c_IconScreenp->v[0x79] = 0x6d;
    resvars.f1c_IconScreenp->v[0x7a] = 0x59;
    resvars.f1c_IconScreenp->v[0x7b] = 0x78;
    resvars.f1c_IconScreenp->v[0x7c] = 0xbc;
    resvars.f1c_IconScreenp->v[0x7d] = 0x16;
    resvars.f1c_IconScreenp->v[0x7e] = 0xbc;
    resvars.f1c_IconScreenp->v[0x7f] = 0x21;
    resvars.f1c_IconScreenp->v[0x80] = 0xbc;
    resvars.f1c_IconScreenp->v[0x81] = 0x2c;
    resvars.f1c_IconScreenp->v[0x82] = 0xbc;
    resvars.f1c_IconScreenp->v[0x83] = 0x37;
    resvars.f1c_IconScreenp->v[0x84] = 0xbc;
    resvars.f1c_IconScreenp->v[0x85] = 0x42;
    resvars.f1c_IconScreenp->v[0x86] = 0xbc;
    resvars.f1c_IconScreenp->v[0x87] = 0x4d;
    resvars.f1c_IconScreenp->v[0x88] = 0xbc;
    resvars.f1c_IconScreenp->v[0x89] = 0x58;
    resvars.f1c_IconScreenp->v[0x8a] = 0xbc;
    resvars.f1c_IconScreenp->v[0x8b] = 0x63;
    resvars.f1c_IconScreenp->v[0x8c] = 0xbc;
    resvars.f1c_IconScreenp->v[0x8d] = 0x6e;
    resvars.f1c_IconScreenp->v[0x8e] = 0xbc;
    resvars.f1c_IconScreenp->v[0x8f] = 0x79;
    resvars.f18_s32p[0x2c] = 0x57;
    resvars.f18_s32p[0x2d] = 0x14;
    resvars.f18_s32p[0x2e] = 0x7a;
    resvars.f18_s32p[0x2f] = 0xa;
    resvars.f18_s32p[0x30] = 0x57;
    resvars.f18_s32p[0x31] = 0x1f;
    resvars.f18_s32p[0x32] = 0x7a;
    resvars.f18_s32p[0x33] = 0xa;
    resvars.f18_s32p[0x34] = 0x57;
    resvars.f18_s32p[0x35] = 0x2a;
    resvars.f18_s32p[0x36] = 0x7a;
    resvars.f18_s32p[0x37] = 0xa;
    resvars.f18_s32p[0x38] = 0x57;
    resvars.f18_s32p[0x39] = 0x35;
    resvars.f18_s32p[0x3a] = 0x7a;
    resvars.f18_s32p[0x3b] = 0xa;
    resvars.f18_s32p[0x3c] = 0x57;
    resvars.f18_s32p[0x3d] = 0x40;
    resvars.f18_s32p[0x3e] = 0x7a;
    resvars.f18_s32p[0x3f] = 0xa;
    resvars.f18_s32p[0x40] = 0x57;
    resvars.f18_s32p[0x41] = 0x4b;
    resvars.f18_s32p[0x42] = 0x7a;
    resvars.f18_s32p[0x43] = 0xa;
    resvars.f18_s32p[0x44] = 0x57;
    resvars.f18_s32p[0x45] = 0x56;
    resvars.f18_s32p[0x46] = 0x7a;
    resvars.f18_s32p[0x47] = 0xa;
    resvars.f18_s32p[0x48] = 0x57;
    resvars.f18_s32p[0x49] = 0x61;
    resvars.f18_s32p[0x4a] = 0x7a;
    resvars.f18_s32p[0x4b] = 0xa;
    resvars.f18_s32p[0x4c] = 0x57;
    resvars.f18_s32p[0x4d] = 0x6c;
    resvars.f18_s32p[0x4e] = 0x7a;
    resvars.f18_s32p[0x4f] = 0xa;
    resvars.f18_s32p[0x50] = 0x57;
    resvars.f18_s32p[0x51] = 0x77;
    resvars.f18_s32p[0x52] = 0x7a;
    resvars.f18_s32p[0x53] = 0xa;
    resvars.f1c_IconScreenp->v[0xb6] = 0x3a;
    resvars.f1c_IconScreenp->v[0xb7] = 0x7d;
    resvars.f1c_IconScreenp->v[0xb8] = 0x3a;
    resvars.f1c_IconScreenp->v[0xb9] = 0x7d;
    resvars.f1c_IconScreenp->v[0xba] = 0x3a;
    resvars.f1c_IconScreenp->v[0xbb] = 0x7d;
    resvars.f1c_IconScreenp->v[0xbc] = 0x20;
    resvars.f1c_IconScreenp->v[0xbd] = 0x49;
    resvars.f1c_IconScreenp->v[0xbe] = 0x34;
    resvars.f1c_IconScreenp->v[0xbf] = 0x49;
    resvars.f1c_IconScreenp->v[0xc0] = 0x48;
    resvars.f1c_IconScreenp->v[0xc1] = 0x49;
    resvars.f18_s32p[0x5c] = 0xd;
    resvars.f18_s32p[0x5d] = 0x18;
    resvars.f18_s32p[0x5e] = 0x29;
    resvars.f18_s32p[0x5f] = 0x29;
    resvars.f1c_IconScreenp->v[0x0] = 0x1d;
    resvars.f1c_IconScreenp->v[0x1] = 0x7e;
    resvars.f18_s32p[0x8c] = 0x21;
    resvars.f18_s32p[0x8d] = 0x3c;
    resvars.f18_s32p[0x8e] = 0x31;
    resvars.f18_s32p[0x8f] = 0x31;
    resvars.f18_s32p[0x88] = 0xa;
    resvars.f18_s32p[0x89] = 0x7a;
    resvars.f18_s32p[0x8a] = 0x2f;
    resvars.f18_s32p[0x8b] = 0x9f;
    resvars.f18_s32p[0x84] = 0xa;
    resvars.f18_s32p[0x85] = 0x80;
    resvars.f18_s32p[0x86] = 0x2f;
    resvars.f18_s32p[0x87] = 0xa5;
    resvars.f18_s32p[0x80] = 0xa;
    resvars.f18_s32p[0x81] = 0x6f;
    resvars.f18_s32p[0x82] = 0x2f;
    resvars.f18_s32p[0x83] = 0x94;
    resvars.f1c_IconScreenp->v[0xee] = 0xb;
    resvars.f1c_IconScreenp->v[0xef] = 0xb;
    resvars.f1c_IconScreenp->v[0xf0] = 0xb;
    resvars.f1c_IconScreenp->v[0xf1] = 0x18;
    resvars.f1c_IconScreenp->v[0xf2] = 0xb;
    resvars.f1c_IconScreenp->v[0xf3] = 0x26;
    resvars.f1c_IconScreenp->v[0xf4] = 0xb;
    resvars.f1c_IconScreenp->v[0xf5] = 0x32;
    resvars.f1c_IconScreenp->v[0xf6] = 0x44;
    resvars.f1c_IconScreenp->v[0xf7] = 0xb;
    resvars.f1c_IconScreenp->v[0xf8] = 0x44;
    resvars.f1c_IconScreenp->v[0xf9] = 0x17;
    resvars.f1c_IconScreenp->v[0xfa] = 0x44;
    resvars.f1c_IconScreenp->v[0xfb] = 0x24;
    resvars.f1c_IconScreenp->v[0xfc] = 0x37;
    resvars.f1c_IconScreenp->v[0xfd] = 0x31;
    resvars.f18_s32p[0x94] = 0xb;
    resvars.f18_s32p[0x95] = 0xb;
    resvars.f18_s32p[0x96] = 0x4a;
    resvars.f18_s32p[0x97] = 0x2b;
    resvars.f1c_IconScreenp->v[0xdc] = 1;
    resvars.f1c_IconScreenp->v[0xdd] = 0x54;
    resvars.f1c_IconScreenp->v[0xdf] = 0xe;
    resvars.f18_s32p[0xa0] = 0xb;
    resvars.f18_s32p[0xa1] = 6;
    resvars.f18_s32p[0xa2] = 0x3d;
    resvars.f18_s32p[0xa4] = 0x4c;
    resvars.f18_s32p[0xa5] = 6;
    resvars.f18_s32p[0xa6] = 8;
    resvars.f1c_IconScreenp->v[0xe1] = 0x48;
    resvars.f1c_IconScreenp->v[0xdd] = 0x4c;
    resvars.f1c_IconScreenp->v[0xe3] = 0xfffffffc;
    resvars.f18_s32p[0x9c] = 0xc;
    resvars.f18_s32p[0x9d] = 6;
    resvars.f18_s32p[0x9e] = 0x3d;
    resvars.f18_s32p[0x9f] = 8;
    resvars.f1c_IconScreenp->v[0x30] = 0x25;
    resvars.f1c_IconScreenp->v[0x31] = 0x7c;
    resvars.f1c_IconScreenp->v[0x38] = 0x25;
    resvars.f1c_IconScreenp->v[0x39] = 0x2b;
    resvars.f1c_IconScreenp->v[0x10e] = 0;
    resvars.f1c_IconScreenp->v[0x10f] = 0;
    resvars.f1c_IconScreenp->v[0x10c] = 7;
    resvars.f1c_IconScreenp->v[0x10d] = 0x6f;
    resvars.f1c_IconScreenp->v[0x108] = 5;
    resvars.f1c_IconScreenp->v[0x109] = 0x7a;
    resvars.f18_s32p[0x174] = 0x14;
    resvars.f18_s32p[0x175] = 0x1c;
    resvars.f18_s32p[0x176] = 0x20;
    resvars.f18_s32p[0x177] = 0x25;
    resvars.f18_s32p[0x164] = 0x14;
    resvars.f18_s32p[0x165] = 0x1c;
    resvars.f18_s32p[0x166] = 0x20;
    resvars.f18_s32p[0x167] = 0x25;
    resvars.f18_s32p[0x168] = 0x14;
    resvars.f18_s32p[0x169] = 0x1c;
    resvars.f18_s32p[0x16a] = 0x20;
    resvars.f18_s32p[0x16b] = 0x25;
    resvars.f18_s32p[0x16c] = 0x54;
    resvars.f18_s32p[0x16d] = 0x11;
    resvars.f18_s32p[0x16e] = 0x18;
    resvars.f18_s32p[0x16f] = 0x29;
    resvars.f18_s32p[0x170] = 0x115;
    resvars.f18_s32p[0x171] = 0x19;
    resvars.f18_s32p[0x172] = 0x18;
    resvars.f18_s32p[0x173] = 0x23;
    resvars.f1c_IconScreenp->v[0x12] = 0x7d;
    resvars.f1c_IconScreenp->v[0x13] = 0x2f;
    resvars.f18_s32p[0x119] = 0x42;
    resvars.f18_s32p[0x127] = 0x1a;
    resvars.f18_s32p[0x12b] = 0x1a;
    resvars.f18_s32p[0x124] = 0;
    resvars.f18_s32p[0x125] = 0;
    resvars.f18_s32p[0x128] = 0x31;
    resvars.f18_s32p[0x129] = 0;
    resvars.f18_s32p[0x164] = 0x29;
    resvars.f18_s32p[0x165] = 6;
    resvars.f18_s32p[0x166] = 0x32;
    resvars.f18_s32p[0x167] = 0xb;
    resvars.f18_s32p[0x11d] = 0x21;
    resvars.f18_s32p[0x12c] = 0;
    resvars.f18_s32p[0x12d] = 0x17;
    resvars.f18_s32p[0x12e] = 0x63;
    resvars.f18_s32p[0x12f] = 0x14;
    resvars.f18_s32p[0x114] = 0x11d;
    resvars.f18_s32p[0x115] = 0xc;
}

// FUNCTION: 0x0008d77c
void swap_game_sprites(TbScreenMode mode)
{
    u8 active;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    if (resvars.f5 == 2) {
        LbSpriteReset(pointer, D_00153f30, D_00153f28);
        LbSpriteReset(D_00153f44, D_00153f48, D_00153f40);
        LbSpriteReset(sprite, D_00153f24, D_00153f1c);
        LbSpriteReset(button_sprite, end_button_sprites, button_sprite_data);
        LbSpriteReset(game_panel, D_00153f88, D_00153f80);
        LbSpriteReset(pulldv, D_00153f94, D_00153f8c);
        unload_game_files();
    }
    switch (mode) {
    case SCREEN_MODE_1:
        statvar.unknown4 = lbScreenModeInfo[SCREEN_MODE_1].height - 0x19;
        statvar.unknowne6 = 0x17;
        statvar.unknowne7 = 0x1a;
        LbMouseChangeSprite(0);
        LbDataFreeAll(D_0013e22c);
        fn_000bce44(D_0013e2b8);
        LbPackHeap();
        resvars.f1 = SCREEN_MODE_1;
        load_game_files();
        resvars.f18_s32p = D_0014b1c4;
        resvars.f3c_u8p = D_0014619c;
        resvars.f1c_IconScreenp = &D_0014b7b4;
        resvars.f40_s32p = D_00146c8c;
        resvars.f3 = 0x10;
        resvars.f4 = 0x20;
        resvars.f20_u8p = D_00145108;
        resvars.f24 = D_0013fd80;
        resvars.f44 = D_00145358;
        resvars.f28 = D_0014bd5c;
        resvars.f2c_intp = D_0014be34;
        resvars.f34 = D_0014d414;
        resvars.fe = D_0014a810;
        resvars.f12_u8p = D_0014adda;
        resvars.fa = D_0013e6f4;
        resvars.f16 = 4;
        resvars.f17 = 0x21;
        resvars.f38 = D_0012cc7c;
        mcga_coords();
        othervar.fa40 = 1;
        othervar.fa41 = 0;
        break;
    case SCREEN_MODE_13:
        statvar.unknown4 = lbScreenModeInfo[SCREEN_MODE_13].height - 0x30;
        statvar.unknowne6 = 0x1c;
        statvar.unknowne7 = 0x1f;
        LbMouseChangeSprite(0);
        LbDataFreeAll(D_0013e2b8);
        fn_000bce44(D_0013e22c);
        LbPackHeap();
        resvars.f1 = SCREEN_MODE_13;
        load_game_files();
        resvars.f18_s32p = D_0013bb0c;
        resvars.f3c_u8p = D_0014619c;
        resvars.f1c_IconScreenp = &D_0013c0fc;
        resvars.f40_s32p = D_00146c8c;
        resvars.f3 = 0x20;
        resvars.f4 = 0x40;
        resvars.f20_u8p = D_001361ac;
        resvars.f24 = D_001310e4;
        resvars.f44 = D_00145358;
        resvars.f28 = D_0013c6a4;
        resvars.f2c_intp = D_0013c77c;
        resvars.f34 = D_00130288;
        resvars.f6 = D_0013e5fc;
        resvars.fe = D_0014a620;
        resvars.f12_u8p = D_0014a9fe;
        resvars.fa = D_0013e634;
        resvars.f16 = 3;
        resvars.f17 = 0x20;
        resvars.f38 = D_0012cc3c;
        othervar.fa41 = 1;
        othervar.fa40 = 0;
        break;
    default:
        break;
    }
    resvars.f2 = mode;
    window.unknown30 = 0;
    window.unknown48 = lbScreenModeInfo[mode].width;
    window.unknown3c = 0;
    window.unknown54 = lbScreenModeInfo[mode].height;
    window.unknown2c0 = 0;
    window.unknown90 = (window.unknown30 + window.unknown48) / 2;
    window.unknown9c = (window.unknown3c + window.unknown54) / 2;
    window.unknown2eb = 0;
    setup_menus();
    LbSpriteSetup(pointer, D_00153f30, D_00153f28);
}

// FUNCTION: 0x0008db38
extern "C" void fn_0008db38(TbScreenMode mode)
{
    u8 active;
    s32 cell;

    active = lbDisplay.unknown4 ? (u8)1 : (u8)0;
    find_cell_at_point(window.unknown2eb, window.f90[window.unknown2eb], window.f9c[window.unknown2eb]);
    cell = info.unknown64;
    D_001290d0 = 0xff;
    D_001290d1 = 0xff;
    D_001290d2 = 0xff;
    if (active)
        LbIsActive();
    if (!change_res(mode)) {
        D_0015f982 = 10;
        sub_error_number = resvars.f0;
        reset_host();
        exit(1);
    }
    swap_game_sprites(mode);
    LbSpriteSetup(pointer, D_00153f30, D_00153f28);
    LbMouseSetup(&pointer[othervar.f30]);
    D_00156910 = 1;
    othervar.f10 = 1;
    set_all_refresh_zones_in_window(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c, 3);
    if (active) {
        while (fn_000bed50() != 1)
            ;
    }
    LbScreenClear(0);
    precalculate_shadow_blocks();
    goto_cell_in_window(window.unknown2eb, cell);
}

// FUNCTION: 0x0008dc7c
extern "C" void fn_0008dc7c(void)
{
    if (statvar.unknownfd <= 1) {
        fn_000bbff0(othervar.f935, yesno_text[6]);
        open_yes_no_requester(0x91, 0, 0, 0x78, 1, 0);
    }
}
