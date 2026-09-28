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
extern s32 D_00188c38;
extern u16 D_00188c74;
extern u8 D_00188c7b;
extern u8 D_00188c7c;
extern u8 D_00188c7e;
extern u8 D_00188c81;
extern u8 D_00188c83;
extern u8 D_00188c85;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern Event D_001549d0[];
extern ManagerBox *D_0015f430;
extern Point *D_0015f434;
extern Button *D_0015f43c;
extern ManagerHelp *D_0015f420;
extern u16 D_00155c64;
extern char D_0015544e[];
extern u8 D_00155daf;
extern u8 D_00155db0;
extern u8 D_00155e04;
extern u8 D_00155e05;
extern u8 D_00155e06;
extern s8 D_0012d748[];
extern u16 D_0012ddde[];
extern char D_00120654[];
extern char D_00120657[];
extern char D_0012065a[];
extern char D_0012065d[];
extern char D_00120660[];
extern char D_00120663[];
extern char D_00120666[];
extern char D_00120669[];
extern char D_0012066e[];
extern char D_00120673[];
extern char D_00120678[];
extern char D_0012067d[];
extern char D_00120682[];
extern char D_00120687[];
extern char D_0012068c[];
extern char D_00120694[];
extern char D_0012069c[];
extern char D_0012069f[];
extern char D_001206a2[];
extern char D_001206a5[];
extern char D_001206a8[];
extern char D_001206ab[];

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
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0x1b);
    switch (manager.unknown2) {
    case 0:
        load_ingame_pic(7, 1, 0x17, 0, SCREEN_MODE_13);
        D_00155daf = fn_000c960c(current_palette, 0x15, 0x15, 0x18);
        D_00155db0 = fn_000c960c(current_palette, 0x2c, 0x2c, 0x2c);
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
    if (D_00188c81) {
        if (over_hotspot(D_00188c6c, D_00188c70, (s16)D_0015f430[31].x, (s16)D_0015f430[31].y,
                         (s16)D_0015f430[31].w, (s16)D_0015f430[31].h))
            fn_00074220(0, 0x3b);
        if (over_hotspot(D_00188c6c, D_00188c70, 0x15b, 0x198, 0x87, 0x45))
            fn_00074220(0, 0x14);
    }
    D_00188c7b = D_00188c7c = D_00188c81 = D_00188c83 = 0;
    D_00188c74 = 0;
    if (exit_quick_requester())
        manager.unknown2 = 2;
}

// FUNCTION: 0x0007e42c
void display_manager_text(void)
{
    u8 i;
    s32 sum;
    char buf[100];

    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0x100;
    LbTextSetWindow(D_0015f430[11].x, D_0015f430[11].y, D_0015f430[11].w, D_0015f430[11].h);
    sprintf(buf, D_00120654, bank_text[0]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[12].x, D_0015f430[12].y, D_0015f430[12].w, D_0015f430[12].h);
    sprintf(buf, D_00120657, bank_text[1]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[13].x, D_0015f430[13].y, D_0015f430[13].w, D_0015f430[13].h);
    sprintf(buf, D_0012065a, bank_text[2]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[14].x, D_0015f430[14].y, D_0015f430[14].w, D_0015f430[14].h);
    sprintf(buf, D_0012065d, bank_text[3]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[15].x, D_0015f430[15].y, D_0015f430[15].w, D_0015f430[15].h);
    sprintf(buf, D_00120660, bank_text[4]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[19].x, D_0015f430[19].y, D_0015f430[19].w, D_0015f430[19].h);
    sprintf(buf, D_00120663, bank_text[5]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[20].x, D_0015f430[20].y, D_0015f430[20].w, D_0015f430[20].h);
    sprintf(buf, D_00120666, bank_text[6]);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[21].x, D_0015f430[21].y, D_0015f430[21].w, D_0015f430[21].h);
    sprintf(buf, D_00120669, myhosp->f1bb);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[22].x, D_0015f430[22].y, D_0015f430[22].w, D_0015f430[22].h);
    sprintf(buf, D_0012066e, myhosp->f35);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[23].x, D_0015f430[23].y, D_0015f430[23].w, D_0015f430[23].h);
    sprintf(buf, D_00120673, myhosp->f3d);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[24].x, D_0015f430[24].y, D_0015f430[24].w, D_0015f430[24].h);
    sprintf(buf, D_00120678, towns[myhosp->f29].f10 * myhosp->f3d / 10000 / 12);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[25].x, D_0015f430[25].y, D_0015f430[25].w, D_0015f430[25].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[0][i];
    sprintf(buf, D_0012067d, sum);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[26].x, D_0015f430[26].y, D_0015f430[26].w, D_0015f430[26].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[1][i];
    sprintf(buf, D_00120682, sum);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[27].x, D_0015f430[27].y, D_0015f430[27].w, D_0015f430[27].h);
    for (i = 0, sum = 0; i < 12; i++)
        sum += myhosp->f45[2][i];
    sprintf(buf, D_00120687, sum);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[28].x, D_0015f430[28].y, D_0015f430[28].w, D_0015f430[28].h);
    sprintf(buf, D_0012068c, towns[myhosp->f29].fe / 100, towns[myhosp->f29].fe % 100);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    LbTextSetWindow(D_0015f430[29].x, D_0015f430[29].y, D_0015f430[29].w, D_0015f430[29].h);
    sprintf(buf, D_00120694, towns[myhosp->f29].f10 / 100, towns[myhosp->f29].f10 % 100);
    LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
    if (manager.unknown0) {
        s32 max;
        s32 end;
        s32 x;
        s32 y;
        s32 pass;
        s32 px;
        s32 py;

        max = 1;
        reqspr_draw(D_0015f434[78].x, D_0015f434[78].y, &screen_sprites_tab_start[1]);
        D_0015f43c[5].sprite = 0;
        D_0015f43c[6].sprite = 0;
        D_0015f43c[7].sprite = 2;
        LbTextSetWindow(D_0015f430[16].x, D_0015f430[16].y, D_0015f430[16].w, D_0015f430[16].h);
        sprintf(buf, D_0012069c, insurance_names[insurance_co[D_0012ddde[manager.unknown0]].unknown0[0]]);
        LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
        for (i = 0; i < 12; i++) {
            if (max < myhosp->f45[manager.unknown0 - 1][i])
                max = myhosp->f45[manager.unknown0 - 1][i];
        }
        set_font_from_cache(3);
        D_00188c74 = 0x40;
        D_00188c85 = color.unknown8;
        D_00188c74 |= 0x40;
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        if (max > 1) {
            sprintf(buf, D_0012069f, max);
            LbTextDraw(D_0015f434[169].x, D_0015f434[169].y, buf);
        }
        i = mycontrol->unknown650;
        end = 12;
        x = D_0015f430[30].x;
        y = D_0015f430[30].y - myhosp->f45[manager.unknown0 - 1][i] * D_0015f430[30].h / max;
        for (pass = 0; pass < 2; pass++) {
            for (; i < end; i++) {
                px = x;
                py = y;
                x += D_0015f430[30].w / 12;
                y = D_0015f430[30].y - myhosp->f45[manager.unknown0 - 1][i] * D_0015f430[30].h / max;
                fn_000c9be0(px + 1, py + 1, x + 1, y + 1, D_00155db0);
                fn_000c9be0(px, py, x, y, D_00155daf);
            }
            end = mycontrol->unknown650;
            i = 0;
        }
        sprintf(buf, D_001206a2, month_text[mycontrol->unknown650]);
        LbTextDraw(D_0015f434[170].x, D_0015f434[170].y, buf);
        compute_help_text(0, 0, 0x17);
        D_00188c74 = 0x100;
        set_font_from_cache(0x1b);
    } else {
        D_0015f43c[5].sprite = 3;
        D_0015f43c[6].sprite = 3;
        D_0015f43c[7].sprite = 3;
        LbTextSetWindow(D_0015f430[16].x, D_0015f430[16].y, D_0015f430[16].w, D_0015f430[16].h);
        sprintf(buf, D_001206a5, insurance_names[insurance_co[ins_control[0]].unknown0[0]]);
        LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
        LbTextSetWindow(D_0015f430[17].x, D_0015f430[17].y, D_0015f430[17].w, D_0015f430[17].h);
        sprintf(buf, D_001206a8, insurance_names[insurance_co[ins_control[1]].unknown0[0]]);
        LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
        LbTextSetWindow(D_0015f430[18].x, D_0015f430[18].y, D_0015f430[18].w, D_0015f430[18].h);
        sprintf(buf, D_001206ab, insurance_names[insurance_co[ins_control[2]].unknown0[0]]);
        LbTextDraw(D_0015f434[71].x, D_0015f434[71].y, buf);
        compute_help_text(0, 0, 0x16);
    }
    compute_help_text(0, 0, 0x15);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    D_00188c74 = 0;
}

// FUNCTION: 0x0007f258
void process_bank_panel(void)
{
    u8 i;

    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 3].width != 0 && D_0015f43c[i + 3].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 3].x, D_0015f43c[i + 3].y,
                             (s16)D_0015f43c[i + 3].width, (s16)D_0015f43c[i + 3].height)) {
                D_0015f43c[i + 3].callback(i);
                reqspr_draw(D_0015f43c[i + 3].unknownC, D_0015f43c[i + 3].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 3].sprite]);
            }
        }
    }
    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 3].width != 0 && D_0015f43c[i + 3].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 3].x, D_0015f43c[i + 3].y,
                             (s16)D_0015f43c[i + 3].width, (s16)D_0015f43c[i + 3].height)) {
                reqspr_draw(D_0015f43c[i + 3].unknownC, D_0015f43c[i + 3].unknownE,
                            &screen_sprites_tab_start[D_0015f43c[i + 3].sprite]);
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
        if (myhosp->f41 - 5000 >= myhosp->f3d) {
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
        if (D_00155e04 == 0)
            D_00155e04 = 1;
    }
    if (saferand() % 300 == 1) {
        if (D_00155e05 == 0)
            D_00155e05 = 1;
    }
    if (saferand() % 300 == 1) {
        if (D_00155e06 == 0)
            D_00155e06 = 1;
    }
    if (D_00155e04) {
        reqspr_draw(D_0015f434[79].x, D_0015f434[79].y, &screen_sprites_tab_start[bank_manager_face1[D_00155e04]]);
        D_00155e04++;
        if (bank_manager_face1[D_00155e04] == -1) {
            D_00155e04 = 0;
            if (saferand() % 5 == 1)
                D_00155e04 = 1;
        }
    }
    if (D_00155e05) {
        reqspr_draw(D_0015f434[80].x, D_0015f434[80].y, &screen_sprites_tab_start[bank_manager_face2[D_00155e05]]);
        D_00155e05++;
        if (bank_manager_face2[D_00155e05] == -1)
            D_00155e05 = 0;
    }
    if (D_00155e06) {
        reqspr_draw(D_0015f434[81].x, D_0015f434[81].y, &screen_sprites_tab_start[D_0012d748[D_00155e06]]);
        D_00155e06++;
        if (D_0012d748[D_00155e06] == -1)
            D_00155e06 = 0;
    }
}

// FUNCTION: 0x0007f8cc
void manager_help_pointed_ins(void)
{
    u32 text;
    u32 idx;

    text = D_0015f420[D_00155c64].text;
    idx = D_0015f420[D_00155c64].arg;
    sprintf(D_0015544e, help_text[text], insurance_names[insurance_co[ins_control[idx]].unknown0[0]]);
}

// FUNCTION: 0x0007f968
void manager_help_displayed_ins(void)
{
    u32 text;

    text = D_0015f420[D_00155c64].text;
    sprintf(D_0015544e, help_text[text], insurance_names[insurance_co[D_0012ddde[manager.unknown0]].unknown0[0]]);
}
