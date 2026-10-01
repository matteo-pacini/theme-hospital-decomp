#include "hosp.h"

struct ChoiceBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

extern "C" {
extern Event D_001549d0[];
extern char **research_names;
extern char **misc_text;
void StopSample(u32 a, u32 b);
}

extern Icons panel_icons_over[];

void process_res_panel(void);
void draw_research_text(void);
s32 find_new_cure_cost(u8 illness, u8 owner);
u8 choose_next_research_illness(u8 owner, s32 (*cost)(u8, u8));

// Runs one frame of the research screen (research.unknown2: 0 opening, 1
// shown, 2 closing, which queues EV_RETURN_FROM_SCREEN): animates it, takes
// clicks and draws the allocations.
// FUNCTION: 0x000b26d0
void display_research(void)
{
    u8 frame_no;

    frame_no = (game_turn & 0xf) / 2 + 1;
    compute_help_text(0, 0, 0x10);
    switch (research.unknown2) {
    case 0:
        load_ingame_pic(6, 1, 0x1e, 0, SCREEN_MODE_13);
        research.unknown2 = 1;
        research.unknown0 = 0;
        play_sound(0xc, SFX_RESEARCH, -1, -1);
        othervar.fa84 = 0;
        panel = panel_icons_over;
        ((PanelIcon *)panel)[9].sprite = 0x15;
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        break;
    case 2:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        StopSample(0xc, 0x75);
        break;
    default:
        break;
    }
    reqspr_draw(resvars.f40[85].x, resvars.f40[85].y, &screen_sprites_tab_start[frame_no + 12]);
    reqspr_draw(resvars.f40[86].x, resvars.f40[86].y, &screen_sprites_tab_start[frame_no + 4]);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_colour = color.unknown8;
    *(volatile u16 *)&lbDisplay.draw_flags |= 0x40;
    set_font_from_cache(1);
    process_res_panel();
    draw_research_text();
    lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester())
        research.unknown2 = 2;
}

// Draws each research category's allocation (or "closed") and progress bar,
// and the total allocated. Every bar shows the progress of the next cure.
// FUNCTION: 0x000b28b4
void draw_research_text(void)
{
    u8 total;
    u16 i;
    u16 height;
    u16 x;
    u16 illness;
    s16 len;
    char buf[100];

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(9);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    for (i = 0; i < 5; i++) {
        LbTextDraw(resvars.f40[i + 0x57].x, resvars.f40[i + 0x57].y, research_names[i]);
        if (myhosp->research_alloc[i] >= 0) {
            sprintf(buf, "%d", myhosp->research_alloc[i]);
            LbTextDraw(resvars.f40[i + 0x61].x, resvars.f40[i + 0x61].y, buf);
            if (myhosp->research_cost[i] > 0) {
                illness = choose_next_research_illness(player, find_new_cure_cost);
                if (illness) {
                    len = myhosp->illness_research[illness] * 0x122 / myhosp->research_cost[i];
                    if (len < 0)
                        len = 0;
                    else if (len > 0x122)
                        len = 0x122;
                    for (x = 0; x < len; x += 5)
                        reqspr_draw(resvars.f40[i + 0x5c].x + x, resvars.f40[i + 0x5c].y, &screen_sprites_tab_start[3]);
                }
            }
        } else {
            LbDrawBox(resvars.f3c_ChoiceBoxp[i + 0x29].x, resvars.f3c_ChoiceBoxp[i + 0x29].y, resvars.f3c_ChoiceBoxp[i + 0x29].w, resvars.f3c_ChoiceBoxp[i + 0x29].h,
                      0xb);
            LbTextDraw(resvars.f40[i + 0x61].x, resvars.f40[i + 0x61].y, misc_text[0x14]);
        }
    }
    total = 0;
    for (i = 0; i < 5; i++) {
        if (myhosp->research_alloc[i] > 0)
            total += myhosp->research_alloc[i];
    }
    sprintf(buf, research_names[6]);
    LbTextDraw(resvars.f40[0x66].x, resvars.f40[0x66].y, buf);
    sprintf(buf, "%d", total);
    LbTextDraw(resvars.f40[0x67].x, resvars.f40[0x67].y, buf);
}

// Sets player `owner`'s research allocation for category cat to value
// (percent), keeping the old value if the total would pass 100. A closed
// category (-1) is not protected: the old value is read into a u8.
// FUNCTION: 0x000b2c74
void alter_research(u8 cat, u8 value, u8 owner)
{
    u8 old;
    AllHosps *hosp;
    u8 total;
    u8 i;

    hosp = &allhosps[owner];
    old = hosp->research_alloc[cat];
    if (old >= 0) {
        hosp->research_alloc[cat] = value;
        total = 0;
        for (i = 0; i < 5; i++) {
            if (hosp->research_alloc[i] > 0)
                total += hosp->research_alloc[i];
        }
        if (total > 100)
            hosp->research_alloc[cat] = old;
    }
}

// Research screen clicks: the close button on release, the allocation
// buttons repeatedly while held, with an accelerating step (othervar.f9f1).
// FUNCTION: 0x000b2d44
void process_res_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0x19].width != 0 && resvars.f48[i + 0x19].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0x19].x, resvars.f48[i + 0x19].y,
                             resvars.f48[i + 0x19].width, resvars.f48[i + 0x19].height)) {
                if (i == 0)
                    resvars.f48[i + 0x19].callback(i);
                reqspr_draw(resvars.f48[i + 0x19].unknownC, resvars.f48[i + 0x19].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x19].sprite]);
            }
        }
        othervar.f9f1 = 0x100;
        othervar.f9f3 = othervar.f8a2;
        research.unknown0 = 0;
        lbDisplay.rleft_button = 0;
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0x19].width != 0 && resvars.f48[i + 0x19].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0x19].x, resvars.f48[i + 0x19].y,
                             resvars.f48[i + 0x19].width, resvars.f48[i + 0x19].height)) {
                if (i != 0)
                    resvars.f48[i + 0x19].callback(i);
                reqspr_draw(resvars.f48[i + 0x19].unknownC, resvars.f48[i + 0x19].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 0x19].sprite]);
                break;
            }
        }
    } else {
        othervar.f9f1 = 0x100;
        othervar.f9f3 = othervar.f8a2;
    }
}

// Research screen button `button`: 0 closes, 1-5 lower and 6-10 raise
// category button - 1 or button - 6 (EV_ALTER_RESEARCH), by an amount that
// grows while the button is held, keeping the total at most 100.
// FUNCTION: 0x000b3060
void process_res_button(u8 button)
{
    u16 i;
    u32 total;

    switch (button) {
    case 0:
        research.unknown2 = 2;
        play_sound(10, SFX_NO4, -1, -1);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        if (myhosp->research_alloc[button - 1] >= 0) {
            i = myhosp->research_alloc[button - 1];
            if (i > 0) {
                if (othervar.f8a2 - othervar.f9f3 > 0xfa || othervar.f9f1 == 0x100) {
                    D_001549d0[player].type = EV_ALTER_RESEARCH;
                    D_001549d0[player].value = button - 1;
                    D_001549d0[player].arg = myhosp->research_alloc[button - 1];
                    if (othervar.f9f1 < 0xa00)
                        othervar.f9f1 += 0x10;
                    else
                        othervar.f9f1 = 0xa00;
                    D_001549d0[player].arg -= othervar.f9f1 >> 8;
                    if ((s16)D_001549d0[player].arg < 0)
                        D_001549d0[player].arg = 0;
                    play_sound(10, SFX_SELECTX, -1, -1);
                }
            } else if (!research.unknown0) {
                play_sound(10, SFX_WRONG2, -1, -1);
                research.unknown0 = 1;
            }
        }
        break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        if (myhosp->research_alloc[button - 6] >= 0) {
            total = 0;
            for (i = 0; i < 5; i++) {
                if (i != button - 6) {
                    if (myhosp->research_alloc[i] >= 0)
                        total += myhosp->research_alloc[i];
                }
            }
            i = myhosp->research_alloc[button - 6];
            if (i + total < 100) {
                if (othervar.f8a2 - othervar.f9f3 > 0xfa || othervar.f9f1 == 0x100) {
                    D_001549d0[player].type = EV_ALTER_RESEARCH;
                    D_001549d0[player].value = button - 6;
                    D_001549d0[player].arg = myhosp->research_alloc[button - 6];
                    if (othervar.f9f1 < 0xa00)
                        othervar.f9f1 += 0x10;
                    else
                        othervar.f9f1 = 0xa00;
                    D_001549d0[player].arg += othervar.f9f1 >> 8;
                    if (D_001549d0[player].arg + total > 100)
                        D_001549d0[player].arg = 100 - total;
                    play_sound(10, SFX_SELECTX, -1, -1);
                }
            } else if (!research.unknown0) {
                play_sound(10, SFX_WRONG2, -1, -1);
                research.unknown0 = 1;
            }
        }
        break;
    default:
        break;
    }
}
