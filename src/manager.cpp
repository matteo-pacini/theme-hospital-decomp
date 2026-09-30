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
extern s8 D_0012d748[];

void fn_000c9be0(s32 x1, s32 y1, s32 x2, s32 y2, u8 colour);
}

extern u16 ins_control[];
extern InsuranceCo insurance_co[];
extern s8 bank_manager_face1[];
extern s8 bank_manager_face2[];

void display_manager_text(void);
void display_manager_face(void);
void process_bank_panel(void);

// FUNCTION: 0x0007e210
void display_manager(void)
{
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1b);
    switch (manager.unknown2) {
    case 0:
        load_ingame_pic(7, 1, 0x17, 0, SCREEN_MODE_13);
        othervar.f9f7 = LbPaletteFindColour(current_palette, 0x15, 0x15, 0x18);
        othervar.f9f8 = LbPaletteFindColour(current_palette, 0x2c, 0x2c, 0x2c);
        manager.unknown2 = 1;
        manager.unknown0 = 0;
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
        D_001549d0[player].type = 0x70;
        D_001549d0[player].value = 0;
        break;
    }
    if (lbDisplay.rleft_button) {
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, (s16)resvars.f3c_ManagerBoxp[31].x, (s16)resvars.f3c_ManagerBoxp[31].y,
                         (s16)resvars.f3c_ManagerBoxp[31].w, (s16)resvars.f3c_ManagerBoxp[31].h))
            fn_00074220(0, 0x3b);
        if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, 0x15b, 0x198, 0x87, 0x45))
            fn_00074220(0, 0x14);
    }
    lbDisplay.left_button = lbDisplay.right_button = lbDisplay.rleft_button = lbDisplay.rright_button = 0;
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester())
        manager.unknown2 = 2;
}

// FUNCTION: 0x0007e42c
void display_manager_text(void)
{
    u8 i;
    s32 sum;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0x100;
    LbTextSetWindow(resvars.f3c_ManagerBoxp[11].x, resvars.f3c_ManagerBoxp[11].y, resvars.f3c_ManagerBoxp[11].w, resvars.f3c_ManagerBoxp[11].h);
    sprintf(buf, "%s", bank_text[0]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[12].x, resvars.f3c_ManagerBoxp[12].y, resvars.f3c_ManagerBoxp[12].w, resvars.f3c_ManagerBoxp[12].h);
    sprintf(buf, "%s", bank_text[1]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[13].x, resvars.f3c_ManagerBoxp[13].y, resvars.f3c_ManagerBoxp[13].w, resvars.f3c_ManagerBoxp[13].h);
    sprintf(buf, "%s", bank_text[2]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[14].x, resvars.f3c_ManagerBoxp[14].y, resvars.f3c_ManagerBoxp[14].w, resvars.f3c_ManagerBoxp[14].h);
    sprintf(buf, "%s", bank_text[3]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[15].x, resvars.f3c_ManagerBoxp[15].y, resvars.f3c_ManagerBoxp[15].w, resvars.f3c_ManagerBoxp[15].h);
    sprintf(buf, "%s", bank_text[4]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[19].x, resvars.f3c_ManagerBoxp[19].y, resvars.f3c_ManagerBoxp[19].w, resvars.f3c_ManagerBoxp[19].h);
    sprintf(buf, "%s", bank_text[5]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[20].x, resvars.f3c_ManagerBoxp[20].y, resvars.f3c_ManagerBoxp[20].w, resvars.f3c_ManagerBoxp[20].h);
    sprintf(buf, "%s", bank_text[6]);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[21].x, resvars.f3c_ManagerBoxp[21].y, resvars.f3c_ManagerBoxp[21].w, resvars.f3c_ManagerBoxp[21].h);
    sprintf(buf, "$ %d", myhosp->f1bb);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[22].x, resvars.f3c_ManagerBoxp[22].y, resvars.f3c_ManagerBoxp[22].w, resvars.f3c_ManagerBoxp[22].h);
    sprintf(buf, "$ %d", myhosp->f35);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[23].x, resvars.f3c_ManagerBoxp[23].y, resvars.f3c_ManagerBoxp[23].w, resvars.f3c_ManagerBoxp[23].h);
    sprintf(buf, "$ %d", myhosp->f3d);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[24].x, resvars.f3c_ManagerBoxp[24].y, resvars.f3c_ManagerBoxp[24].w, resvars.f3c_ManagerBoxp[24].h);
    sprintf(buf, "$ %d", towns[myhosp->f29].f10 * myhosp->f3d / 10000 / 12);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[25].x, resvars.f3c_ManagerBoxp[25].y, resvars.f3c_ManagerBoxp[25].w, resvars.f3c_ManagerBoxp[25].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[0][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[26].x, resvars.f3c_ManagerBoxp[26].y, resvars.f3c_ManagerBoxp[26].w, resvars.f3c_ManagerBoxp[26].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[1][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[27].x, resvars.f3c_ManagerBoxp[27].y, resvars.f3c_ManagerBoxp[27].w, resvars.f3c_ManagerBoxp[27].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[2][i];
    sprintf(buf, "$ %d", sum);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[28].x, resvars.f3c_ManagerBoxp[28].y, resvars.f3c_ManagerBoxp[28].w, resvars.f3c_ManagerBoxp[28].h);
    sprintf(buf, "%d.%d%%", towns[myhosp->f29].fe / 100, towns[myhosp->f29].fe % 100);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    LbTextSetWindow(resvars.f3c_ManagerBoxp[29].x, resvars.f3c_ManagerBoxp[29].y, resvars.f3c_ManagerBoxp[29].w, resvars.f3c_ManagerBoxp[29].h);
    sprintf(buf, "%d.%d%%", towns[myhosp->f29].f10 / 100, towns[myhosp->f29].f10 % 100);
    LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
    if (manager.unknown0) {
        s32 max;
        s32 end;
        s32 x;
        s32 y;
        s32 pass;
        s32 px;
        s32 py;

        max = 1;
        reqspr_draw(resvars.f40[78].x, resvars.f40[78].y, &screen_sprites_tab_start[1]);
        resvars.f48[5].sprite = 0;
        resvars.f48[6].sprite = 0;
        resvars.f48[7].sprite = 2;
        LbTextSetWindow(resvars.f3c_ManagerBoxp[16].x, resvars.f3c_ManagerBoxp[16].y, resvars.f3c_ManagerBoxp[16].w, resvars.f3c_ManagerBoxp[16].h);
        sprintf(buf, "%s", insurance_names[insurance_co[gbv.f15e[manager.unknown0]].unknown0[0]]);
        LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
        for (i = 0; i < 12; i++) {
            if (max < myhosp->f45[manager.unknown0 - 1][i])
                max = myhosp->f45[manager.unknown0 - 1][i];
        }
        set_font_from_cache(3);
        lbDisplay.draw_flags = 0x40;
        lbDisplay.draw_colour = color.unknown8;
        lbDisplay.draw_flags |= 0x40;
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        if (max > 1) {
            sprintf(buf, "%d", max);
            LbTextDraw(resvars.f40[169].x, resvars.f40[169].y, buf);
        }
        i = mycontrol->unknown650;
        end = 12;
        x = resvars.f3c_ManagerBoxp[30].x;
        y = resvars.f3c_ManagerBoxp[30].y - myhosp->f45[manager.unknown0 - 1][i] * resvars.f3c_ManagerBoxp[30].h / max;
        for (pass = 0; pass < 2; pass++) {
            while (i < end) {
                px = x;
                py = y;
                x += resvars.f3c_ManagerBoxp[30].w / 12;
                y = resvars.f3c_ManagerBoxp[30].y - myhosp->f45[manager.unknown0 - 1][i] * resvars.f3c_ManagerBoxp[30].h / max;
                fn_000c9be0(px + 1, py + 1, x + 1, y + 1, othervar.f9f8);
                fn_000c9be0(px, py, x, y, othervar.f9f7);
                i++;
            }
            end = mycontrol->unknown650;
            i = 0;
        }
        sprintf(buf, "%s", month_text[mycontrol->unknown650]);
        LbTextDraw(resvars.f40[170].x, resvars.f40[170].y, buf);
        compute_help_text(0, 0, 0x17);
        lbDisplay.draw_flags = 0x100;
        set_font_from_cache(0x1b);
    } else {
        resvars.f48[5].sprite = 3;
        resvars.f48[6].sprite = 3;
        resvars.f48[7].sprite = 3;
        LbTextSetWindow(resvars.f3c_ManagerBoxp[16].x, resvars.f3c_ManagerBoxp[16].y, resvars.f3c_ManagerBoxp[16].w, resvars.f3c_ManagerBoxp[16].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[0]].unknown0[0]]);
        LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
        LbTextSetWindow(resvars.f3c_ManagerBoxp[17].x, resvars.f3c_ManagerBoxp[17].y, resvars.f3c_ManagerBoxp[17].w, resvars.f3c_ManagerBoxp[17].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[1]].unknown0[0]]);
        LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
        LbTextSetWindow(resvars.f3c_ManagerBoxp[18].x, resvars.f3c_ManagerBoxp[18].y, resvars.f3c_ManagerBoxp[18].w, resvars.f3c_ManagerBoxp[18].h);
        sprintf(buf, "%s", insurance_names[insurance_co[ins_control[2]].unknown0[0]]);
        LbTextDraw(resvars.f40[71].x, resvars.f40[71].y, buf);
        compute_help_text(0, 0, 0x16);
    }
    compute_help_text(0, 0, 0x15);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
}

// FUNCTION: 0x0007f258
void process_bank_panel(void)
{
    u8 i;

    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 3].width != 0 && resvars.f48[i + 3].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 3].x, resvars.f48[i + 3].y,
                             (s16)resvars.f48[i + 3].width, (s16)resvars.f48[i + 3].height)) {
                resvars.f48[i + 3].callback(i);
                reqspr_draw(resvars.f48[i + 3].unknownC, resvars.f48[i + 3].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 3].sprite]);
            }
        }
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 3].width != 0 && resvars.f48[i + 3].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 3].x, resvars.f48[i + 3].y,
                             (s16)resvars.f48[i + 3].width, (s16)resvars.f48[i + 3].height)) {
                reqspr_draw(resvars.f48[i + 3].unknownC, resvars.f48[i + 3].unknownE,
                            &screen_sprites_tab_start[resvars.f48[i + 3].sprite]);
                break;
            }
        }
    }
}

// FUNCTION: 0x0007f50c
void process_bank_button(u8 n)
{
    switch (n) {
    case 0:
        if (myhosp->f35 >= 5000 && myhosp->f3d > 0) {
            D_001549d0[player].type = 0x67;
            play_sound(10, 0x43, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 1:
        if (myhosp->f3d <= myhosp->f41 - 5000) {
            D_001549d0[player].type = 0x66;
            play_sound(10, 0x43, -1, -1);
        } else {
            play_sound(10, 8, -1, -1);
        }
        break;
    case 2:
        if (!manager.unknown0) {
            manager.unknown0 = 1;
            play_sound(10, 0x43, -1, -1);
        }
        break;
    case 3:
        if (!manager.unknown0) {
            manager.unknown0 = 2;
            play_sound(10, 0x43, -1, -1);
        }
        break;
    case 4:
        if (manager.unknown0)
            manager.unknown0 = 0;
        else
            manager.unknown0 = 3;
        play_sound(10, 0x43, -1, -1);
        break;
    case 5:
        manager.unknown2 = 2;
        play_sound(10, 0x42, -1, -1);
        break;
    }
}

// FUNCTION: 0x0007f6e4
void display_manager_face(void)
{
    if (saferand() % 45 == 1) {
        if (othervar.fa4c == 0)
            othervar.fa4c = 1;
    }
    if (saferand() % 300 == 1) {
        if (othervar.fa4d == 0)
            othervar.fa4d = 1;
    }
    if (saferand() % 300 == 1) {
        if (othervar.fa4e == 0)
            othervar.fa4e = 1;
    }
    if (othervar.fa4c) {
        reqspr_draw(resvars.f40[79].x, resvars.f40[79].y, &screen_sprites_tab_start[bank_manager_face1[othervar.fa4c]]);
        othervar.fa4c++;
        if (bank_manager_face1[othervar.fa4c] == -1) {
            othervar.fa4c = 0;
            if (saferand() % 5 == 1)
                othervar.fa4c = 1;
        }
    }
    if (othervar.fa4d) {
        reqspr_draw(resvars.f40[80].x, resvars.f40[80].y, &screen_sprites_tab_start[bank_manager_face2[othervar.fa4d]]);
        othervar.fa4d++;
        if (bank_manager_face2[othervar.fa4d] == -1)
            othervar.fa4d = 0;
    }
    if (othervar.fa4e) {
        reqspr_draw(resvars.f40[81].x, resvars.f40[81].y, &screen_sprites_tab_start[D_0012d748[othervar.fa4e]]);
        othervar.fa4e++;
        if (D_0012d748[othervar.fa4e] == -1)
            othervar.fa4e = 0;
    }
}

// FUNCTION: 0x0007f8cc
void manager_help_pointed_ins(void)
{
    u32 text;
    u32 idx;

    text = resvars.f2c_ManagerHelpp[othervar.f8ac].text;
    idx = resvars.f2c_ManagerHelpp[othervar.f8ac].arg;
    sprintf(othervar.f96, help_text[text], insurance_names[insurance_co[ins_control[idx]].unknown0[0]]);
}

// FUNCTION: 0x0007f968
void manager_help_displayed_ins(void)
{
    u32 text;

    text = resvars.f2c_ManagerHelpp[othervar.f8ac].text;
    sprintf(othervar.f96, help_text[text], insurance_names[insurance_co[gbv.f15e[manager.unknown0]].unknown0[0]]);
}
