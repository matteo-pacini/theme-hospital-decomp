#include "hosp.h"

struct ManagerBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct ManagerHelp {
    u8 unknown0[8];
    u16 text;
    u16 arg;
    u8 unknownc[4];
};

extern "C" {
extern Event D_001549d0[];

void fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
}

extern u16 ins_control[];
extern InsuranceCo insurance_co[];
extern s8 bank_manager_face1[];
extern s8 bank_manager_face2[];

void display_manager_text(void);
void display_manager_face(void);
void process_bank_panel(void);

// Runs one frame of the bank manager screen (manager.state: 0 loading,
// 1 shown, 2 closing, which queues EV_RETURN_FROM_SCREEN for this player).
// FUNCTION: 0x0007e210
void display_manager(void)
{
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1b);
    switch (manager.state) {
    case 0:
        load_ingame_pic(7, 1, 0x17, 0, SCREEN_MODE_13);
        othervar.manager_line_colour = LbPaletteFindColour(current_palette, 0x15, 0x15, 0x18);
        othervar.manager_line_shadow_colour = LbPaletteFindColour(current_palette, 0x2c, 0x2c, 0x2c);
        manager.state = 1;
        manager.value = 0;
        copy_screen_to_wscreen(bscreen);
        display_manager_text();
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        display_manager_text();
        display_manager_face();
        process_bank_panel();
        break;
    case 2:
        D_001549d0[player].type = EV_RETURN_FROM_SCREEN;
        D_001549d0[player].value = 0;
        break;
    }
    if (lbDisplay.rleft_button) {
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, (s16)resvars.screen_boxes_ManagerBoxp[31].x, (s16)resvars.screen_boxes_ManagerBoxp[31].y,
                         (s16)resvars.screen_boxes_ManagerBoxp[31].w, (s16)resvars.screen_boxes_ManagerBoxp[31].h))
            process_keys(0, 0x3b);
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, 0x15b, 0x198, 0x87, 0x45))
            process_keys(0, 0x14);
    }
    lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester())
        manager.state = 2;
}

// Draws the bank manager's figures: balance, loan, interest, the three
// insurance companies' yearly totals and the rates; with manager.value
// set (1-3), the chosen company's monthly payments as a line graph.
// FUNCTION: 0x0007e42c
void display_manager_text(void)
{
    u8 i;
    s32 sum;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0x100;
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[11].x, resvars.screen_boxes_ManagerBoxp[11].y, resvars.screen_boxes_ManagerBoxp[11].w, resvars.screen_boxes_ManagerBoxp[11].h);
    sprintf(buf, "%s", bank_text[0]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[12].x, resvars.screen_boxes_ManagerBoxp[12].y, resvars.screen_boxes_ManagerBoxp[12].w, resvars.screen_boxes_ManagerBoxp[12].h);
    sprintf(buf, "%s", bank_text[1]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[13].x, resvars.screen_boxes_ManagerBoxp[13].y, resvars.screen_boxes_ManagerBoxp[13].w, resvars.screen_boxes_ManagerBoxp[13].h);
    sprintf(buf, "%s", bank_text[2]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[14].x, resvars.screen_boxes_ManagerBoxp[14].y, resvars.screen_boxes_ManagerBoxp[14].w, resvars.screen_boxes_ManagerBoxp[14].h);
    sprintf(buf, "%s", bank_text[3]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[15].x, resvars.screen_boxes_ManagerBoxp[15].y, resvars.screen_boxes_ManagerBoxp[15].w, resvars.screen_boxes_ManagerBoxp[15].h);
    sprintf(buf, "%s", bank_text[4]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[19].x, resvars.screen_boxes_ManagerBoxp[19].y, resvars.screen_boxes_ManagerBoxp[19].w, resvars.screen_boxes_ManagerBoxp[19].h);
    sprintf(buf, "%s", bank_text[5]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[20].x, resvars.screen_boxes_ManagerBoxp[20].y, resvars.screen_boxes_ManagerBoxp[20].w, resvars.screen_boxes_ManagerBoxp[20].h);
    sprintf(buf, "%s", bank_text[6]);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[21].x, resvars.screen_boxes_ManagerBoxp[21].y, resvars.screen_boxes_ManagerBoxp[21].w, resvars.screen_boxes_ManagerBoxp[21].h);
    sprintf(buf, "$ %d", myhosp->hospital_value);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[22].x, resvars.screen_boxes_ManagerBoxp[22].y, resvars.screen_boxes_ManagerBoxp[22].w, resvars.screen_boxes_ManagerBoxp[22].h);
    sprintf(buf, "$ %d", myhosp->balance);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[23].x, resvars.screen_boxes_ManagerBoxp[23].y, resvars.screen_boxes_ManagerBoxp[23].w, resvars.screen_boxes_ManagerBoxp[23].h);
    sprintf(buf, "$ %d", myhosp->loan);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[24].x, resvars.screen_boxes_ManagerBoxp[24].y, resvars.screen_boxes_ManagerBoxp[24].w, resvars.screen_boxes_ManagerBoxp[24].h);
    sprintf(buf, "$ %d", towns[myhosp->level].interest_rate * myhosp->loan / 10000 / 12);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[25].x, resvars.screen_boxes_ManagerBoxp[25].y, resvars.screen_boxes_ManagerBoxp[25].w, resvars.screen_boxes_ManagerBoxp[25].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->insurance_claims[0][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[26].x, resvars.screen_boxes_ManagerBoxp[26].y, resvars.screen_boxes_ManagerBoxp[26].w, resvars.screen_boxes_ManagerBoxp[26].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->insurance_claims[1][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[27].x, resvars.screen_boxes_ManagerBoxp[27].y, resvars.screen_boxes_ManagerBoxp[27].w, resvars.screen_boxes_ManagerBoxp[27].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->insurance_claims[2][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[28].x, resvars.screen_boxes_ManagerBoxp[28].y, resvars.screen_boxes_ManagerBoxp[28].w, resvars.screen_boxes_ManagerBoxp[28].h);
    sprintf(buf, "%d.%d%%", towns[myhosp->level].inflation_rate / 100, towns[myhosp->level].inflation_rate % 100);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[29].x, resvars.screen_boxes_ManagerBoxp[29].y, resvars.screen_boxes_ManagerBoxp[29].w, resvars.screen_boxes_ManagerBoxp[29].h);
    sprintf(buf, "%d.%d%%", towns[myhosp->level].interest_rate / 100, towns[myhosp->level].interest_rate % 100);
    LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
    if (manager.value) {
        s32 max;
        s32 end;
        s32 x;
        s32 y;
        s32 pass;
        s32 px;
        s32 py;

        max = 1;
        reqspr_draw(resvars.screen_coords[78].x, resvars.screen_coords[78].y, &screen_sprites_tab_start[1]);
        resvars.screen_buttons[5].sprite = 0;
        resvars.screen_buttons[6].sprite = 0;
        resvars.screen_buttons[7].sprite = 2;
        LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[16].x, resvars.screen_boxes_ManagerBoxp[16].y, resvars.screen_boxes_ManagerBoxp[16].w, resvars.screen_boxes_ManagerBoxp[16].h);
        sprintf(buf, "%s", insurance_names[insurance_co[gbv.f15e[manager.value]].unknown0[0]]);
        LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
        for (i = 0; i < 12; i++) {
            if (max < myhosp->insurance_claims[manager.value - 1][i])
                max = myhosp->insurance_claims[manager.value - 1][i];
        }
        set_font_from_cache(3);
        lbDisplay.draw_flags = 0x40;
        lbDisplay.draw_colour = color.black;
        lbDisplay.draw_flags |= 0x40;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (max > 1) {
            sprintf(buf, "%d", max);
            LbTextDraw(resvars.screen_coords[169].x, resvars.screen_coords[169].y, buf);
        }
        i = mycontrol->month;
        end = 12;
        x = resvars.screen_boxes_ManagerBoxp[30].x;
        y = resvars.screen_boxes_ManagerBoxp[30].y - myhosp->insurance_claims[manager.value - 1][i] * resvars.screen_boxes_ManagerBoxp[30].h / max;
        for (pass = 0; pass < 2; pass++) {
            while (i < end) {
                px = x;
                py = y;
                x += resvars.screen_boxes_ManagerBoxp[30].w / 12;
                y = resvars.screen_boxes_ManagerBoxp[30].y - myhosp->insurance_claims[manager.value - 1][i] * resvars.screen_boxes_ManagerBoxp[30].h / max;
                fn_000c9be0(px + 1, py + 1, x + 1, y + 1, othervar.manager_line_shadow_colour);
                fn_000c9be0(px, py, x, y, othervar.manager_line_colour);
                i++;
            }
            end = mycontrol->month;
            i = 0;
        }
        sprintf(buf, "%s", month_text[mycontrol->month]);
        LbTextDraw(resvars.screen_coords[170].x, resvars.screen_coords[170].y, buf);
        compute_help_text(0, 0, 0x17);
        lbDisplay.draw_flags = 0x100;
        set_font_from_cache(0x1b);
    } else {
        resvars.screen_buttons[5].sprite = 3;
        resvars.screen_buttons[6].sprite = 3;
        resvars.screen_buttons[7].sprite = 3;
        LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[16].x, resvars.screen_boxes_ManagerBoxp[16].y, resvars.screen_boxes_ManagerBoxp[16].w, resvars.screen_boxes_ManagerBoxp[16].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[0]].unknown0[0]]);
        LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
        LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[17].x, resvars.screen_boxes_ManagerBoxp[17].y, resvars.screen_boxes_ManagerBoxp[17].w, resvars.screen_boxes_ManagerBoxp[17].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[1]].unknown0[0]]);
        LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
        LbTextSetWindow(resvars.screen_boxes_ManagerBoxp[18].x, resvars.screen_boxes_ManagerBoxp[18].y, resvars.screen_boxes_ManagerBoxp[18].w, resvars.screen_boxes_ManagerBoxp[18].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[2]].unknown0[0]]);
        LbTextDraw(resvars.screen_coords[71].x, resvars.screen_coords[71].y, buf);
        compute_help_text(0, 0, 0x16);
    }
    compute_help_text(0, 0, 0x15);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
}

// Bank screen clicks: runs the callback of the resvars.screen_buttons button released
// over and draws the button pressed while the left button is held.
// FUNCTION: 0x0007f258
void process_bank_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 3].width != 0 && resvars.screen_buttons[i + 3].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 3].x, resvars.screen_buttons[i + 3].y,
                             (s16)resvars.screen_buttons[i + 3].width, (s16)resvars.screen_buttons[i + 3].height)) {
                resvars.screen_buttons[i + 3].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 3].sprite_x, resvars.screen_buttons[i + 3].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 3].sprite]);
            }
        }
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 3].width != 0 && resvars.screen_buttons[i + 3].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 3].x, resvars.screen_buttons[i + 3].y,
                             (s16)resvars.screen_buttons[i + 3].width, (s16)resvars.screen_buttons[i + 3].height)) {
                reqspr_draw(resvars.screen_buttons[i + 3].sprite_x, resvars.screen_buttons[i + 3].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 3].sprite]);
                break;
            }
        }
    }
}

// Carries out bank screen button `button`: 0 repays and 1 borrows 5000 (as
// event 0x67 / 0x66, if the balance or the loan limit allows), 2-4 show an
// insurance company's graph (4 again goes back), 5 closes the screen.
// FUNCTION: 0x0007f50c
void process_bank_button(u8 button)
{
    switch (button) {
    case 0:
        if (myhosp->balance >= 5000 && myhosp->loan > 0) {
            D_001549d0[player].type = 0x67;
            play_sound(10, SFX_SELECTX, -1, -1);
        } else {
            play_sound(10, SFX_WRONG2, -1, -1);
        }
        break;
    case 1:
        if (myhosp->loan <= myhosp->max_loan - 5000) {
            D_001549d0[player].type = 0x66;
            play_sound(10, SFX_SELECTX, -1, -1);
        } else {
            play_sound(10, SFX_WRONG2, -1, -1);
        }
        break;
    case 2:
        if (!manager.value) {
            manager.value = 1;
            play_sound(10, SFX_SELECTX, -1, -1);
        }
        break;
    case 3:
        if (!manager.value) {
            manager.value = 2;
            play_sound(10, SFX_SELECTX, -1, -1);
        }
        break;
    case 4:
        if (manager.value)
            manager.value = 0;
        else
            manager.value = 3;
        play_sound(10, SFX_SELECTX, -1, -1);
        break;
    case 5:
        manager.state = 2;
        play_sound(10, SFX_NO4, -1, -1);
        break;
    }
}

// Animates the bank manager's face: randomly starts and advances the three
// face animations (othervar.manager_face1_frame-fa4e, bank_manager_face1-3).
// FUNCTION: 0x0007f6e4
void display_manager_face(void)
{
    if (saferand() % 45 == 1) {
        if (othervar.manager_face1_frame == 0)
            othervar.manager_face1_frame = 1;
    }
    if (saferand() % 300 == 1) {
        if (othervar.manager_face2_frame == 0)
            othervar.manager_face2_frame = 1;
    }
    if (saferand() % 300 == 1) {
        if (othervar.manager_face3_frame == 0)
            othervar.manager_face3_frame = 1;
    }
    if (othervar.manager_face1_frame) {
        reqspr_draw(resvars.screen_coords[79].x, resvars.screen_coords[79].y, &screen_sprites_tab_start[bank_manager_face1[othervar.manager_face1_frame]]);
        othervar.manager_face1_frame++;
        if (bank_manager_face1[othervar.manager_face1_frame] == -1) {
            othervar.manager_face1_frame = 0;
            if (saferand() % 5 == 1)
                othervar.manager_face1_frame = 1;
        }
    }
    if (othervar.manager_face2_frame) {
        reqspr_draw(resvars.screen_coords[80].x, resvars.screen_coords[80].y, &screen_sprites_tab_start[bank_manager_face2[othervar.manager_face2_frame]]);
        othervar.manager_face2_frame++;
        if (bank_manager_face2[othervar.manager_face2_frame] == -1)
            othervar.manager_face2_frame = 0;
    }
    if (othervar.manager_face3_frame) {
        reqspr_draw(resvars.screen_coords[81].x, resvars.screen_coords[81].y, &screen_sprites_tab_start[bank_manager_face3[othervar.manager_face3_frame]]);
        othervar.manager_face3_frame++;
        if (bank_manager_face3[othervar.manager_face3_frame] == -1)
            othervar.manager_face3_frame = 0;
    }
}

// Help text for an insurance company row of the list: formats the hovered
// help entry with that company's name into othervar.help_msg.
// FUNCTION: 0x0007f8cc
void manager_help_pointed_ins(void)
{
    u32 text;
    u32 idx;

    text = resvars.help_hotspots_ManagerHelpp[othervar.help_hotspot].text;
    idx = resvars.help_hotspots_ManagerHelpp[othervar.help_hotspot].arg;
    sprintf(othervar.help_msg, help_text[text], insurance_names[insurance_co[ins_control[idx]].unknown0[0]]);
}

// Help text for the insurance graph: formats the hovered help entry with the
// shown company's name into othervar.help_msg.
// FUNCTION: 0x0007f968
void manager_help_displayed_ins(void)
{
    u32 text;

    text = resvars.help_hotspots_ManagerHelpp[othervar.help_hotspot].text;
    sprintf(othervar.help_msg, help_text[text], insurance_names[insurance_co[gbv.f15e[manager.value]].unknown0[0]]);
}
