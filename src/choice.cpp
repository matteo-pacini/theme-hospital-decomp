#include "hosp.h"

struct ChoiceBox {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

/* A fax number: the buttons dialled, how many, and what dialling it does. */
struct FaxNo {
    u8 digits[6];
    u8 count;
    u8 action;
};

extern char **c_buttons;
extern char **c_text;
extern u8 numbuttons;
extern u8 numtext;
extern u8 NumPressed;
extern Menu objs;
extern Menu options;

void check_choice_buttons(void);
void check_fax_buttons(void);
void calculate_buttons(const u8 type);
void display_buttons(void);
void display_choice_text(const u8 type);
void save_display_screen(void);
u8 fax_no_equal(FaxNo fax, u8 *pressed);
void format_text(char *text, s16 width);
void remove_newlines(char *text);
u32 announce_emergency(u8 owner, char **strs);
void emergency_over(u8 owner, char **strs);
void declare_epidemic(char **strs);
void epidemic_over(const u8 owner, char **strs);
void mayor_requester(u8 idx, char **strs);
void report_mayor_result(u8 owner, char **strs);
void process_emergency_start_buttons(u8 button);
void process_emergency_end_buttons(u8 button);
void process_epidemic_start_buttons(u8 button);
void process_epidemic_end_buttons(u8 button);
void process_mayor_start_buttons(u8 n);
void process_mayor_end_buttons(u8 n);

extern "C" {
extern u8 D_00158192;
extern u16 D_00158190;
extern s16 D_0015f760;
extern u32 D_001611f0;
extern u8 D_001611f8[];
extern Event D_001549d0[];

}

FaxNo cheat_numbers[] = {
    { { 0xc2, 0xc2, 0xc2 }, 3, 1 },
    { { 0xc2, 0xba, 0xba }, 3, 1 },
    { { 0xc2, 0xba, 0xbb }, 3, 1 },
    { { 0xba, 0xb9, 0xb9 }, 3, 1 },
    { { 0xba, 0xba, 0xba }, 3, 1 },
    { { 0xbb, 0xbb, 0xbb }, 3, 1 },
    { { 0xbc, 0xbc, 0xbc }, 3, 1 },
    { { 0xbd, 0xbd, 0xbd }, 3, 1 },
    { { 0xbe, 0xbe, 0xbe }, 3, 1 },
    { { 0xbf, 0xbf, 0xbf }, 3, 1 },
    { { 0xc0, 0xc0, 0xc0 }, 3, 1 },
    { { 0xc1, 0xc1, 0xc1 }, 3, 1 },
    { { 0xc2, 0xc2, 0xc2 }, 3, 1 },
    { { 0xc0, 0xbb, 0xc1, 0xc0 }, 4, 2 },
    { { 0xbb, 0xbd, 0xbc, 0xbb, 0xc1 }, 5, 0 },
    { { 0 }, 0, 0 },
};

extern "C" {
int strcmp(const char *a, const char *b);
void fn_000a3140(void);
}

// FUNCTION: 0x000a3140
void fn_000a3140(void)
{
}

// FUNCTION: 0x000a3158
void display_choice(void)
{
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1e);
    lbDisplay.draw_flags = 0;
    switch (D_00158192) {
    case 0x5a:
        available_buttons = 0xffff;
        load_ingame_pic(0x13, 1, 0x24, 0, (TbScreenMode)0xd);
        othervar.f9d7 = lbDisplay.unknown4;
        othervar.f9db = lbDisplay.graphics_screen_width;
        othervar.f9df = lbDisplay.graphics_screen_height;
        lbDisplay.unknown4 = bscreen;
        lbDisplay.graphics_screen_width = lbDisplay.unknown18;
        lbDisplay.graphics_screen_height = lbDisplay.unknown1c;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        calculate_buttons(D_00158190);
        LbTextSetWindow(resvars.f3c_ChoiceBoxp[173].x, resvars.f3c_ChoiceBoxp[173].y, resvars.f3c_ChoiceBoxp[173].w, resvars.f3c_ChoiceBoxp[173].h);
        display_choice_text(D_00158190);
        display_buttons();
        if (!choice_time_out)
            reqspr_draw(resvars.f40[0xbe].x, resvars.f40[0xbe].y, &screen_sprites_tab_start[20]);
        lbDisplay.unknown4 = othervar.f9d7;
        lbDisplay.graphics_screen_width = othervar.f9db;
        lbDisplay.graphics_screen_height = othervar.f9df;
        LbScreenSetGraphicsWindow(0, 0, othervar.f9db, othervar.f9df);
        D_00158192 = 0x5b;
        D_001611f0 = actual_game_turn;
        NumPressed = 0;
        copy_screen_to_wscreen(bscreen);
        play_sound(10, 0xab, -1, -1);
        break;
    case 0x5b:
        copy_screen_to_wscreen(bscreen);
        check_choice_buttons();
        check_fax_buttons();
        fn_000a3140();
        if (choice_time_out) {
            if (lbInkey == 1 || lbInkey == 0x39) {
                lbInkey = 0;
                play_sound(10, 0x42, -1, -1);
                D_00158192 = 0x5c;
            }
            if (D_0015f760 != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
                D_001611f0 = actual_game_turn;
                D_0015f760 = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
            }
            if (actual_game_turn - D_001611f0 > choice_time_out || exit_quick_requester())
                D_00158192 = 0x5c;
        }
        break;
    case 0x5c:
        restore_display_screen();
        break;
    }
}

// FUNCTION: 0x000a3430
void check_choice_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; (u16)resvars.f48[i + 0xae].x != 0 && (u16)resvars.f48[i + 0xae].y != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0xae].x, resvars.f48[i + 0xae].y,
                             (s16)resvars.f48[i + 0xae].width, (s16)resvars.f48[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    reqspr_draw(resvars.f48[i + 0xae].unknownC, resvars.f48[i + 0xae].unknownE, &screen_sprites_tab_start[18]);
                    break;
                }
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; (u16)resvars.f48[i + 0xae].x != 0 && (u16)resvars.f48[i + 0xae].y != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0xae].x, resvars.f48[i + 0xae].y,
                             (s16)resvars.f48[i + 0xae].width, (s16)resvars.f48[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    play_sound(10, 0x41, -1, -1);
                    if (resvars.f48[i + 0xae].callback)
                        resvars.f48[i + 0xae].callback(i);
                    reqspr_draw(resvars.f48[i + 0xae].unknownC, resvars.f48[i + 0xae].unknownE, &screen_sprites_tab_start[18]);
                    lbDisplay.rleft_button = lbDisplay.left_button = 0;
                    break;
                }
            }
        }
    }
}

// FUNCTION: 0x000a374c
void check_fax_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.f48[i + 0xb8].width != 0 && resvars.f48[i + 0xb8].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.f48[i + 0xb8].x, resvars.f48[i + 0xb8].y,
                             (s16)resvars.f48[i + 0xb8].width, (s16)resvars.f48[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                reqspr_draw(resvars.f48[i + 0xb8].unknownC, resvars.f48[i + 0xb8].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + resvars.f48[i + 0xb8].sprite * 6));
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.f48[i + 0xb8].width != 0 && resvars.f48[i + 0xb8].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.f48[i + 0xb8].x, resvars.f48[i + 0xb8].y,
                             (s16)resvars.f48[i + 0xb8].width, (s16)resvars.f48[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                if (resvars.f48[i + 0xb8].callback)
                    resvars.f48[i + 0xb8].callback((u8)(i + 0xb8));
                reqspr_draw(resvars.f48[i + 0xb8].unknownC, resvars.f48[i + 0xb8].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + resvars.f48[i + 0xb8].sprite * 6));
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// FUNCTION: 0x000a3a90
void calculate_buttons(const u8 type)
{
    u8 i;
    u8 skip;
    void (*handler)(int);

    handler = 0;
    switch (type) {
    case 2:
        skip = 1;
        handler = (void (*)(int))process_emergency_start_buttons;
        break;
    case 3:
        skip = 2;
        handler = (void (*)(int))process_emergency_end_buttons;
        break;
    case 7:
        skip = 3;
        handler = (void (*)(int))process_no_treatment_buttons;
        break;
    case 4:
        skip = 4;
        handler = (void (*)(int))process_announce_new_illness_buttons;
        break;
    case 5:
        skip = 5;
        handler = (void (*)(int))process_epidemic_start_buttons;
        break;
    case 6:
        skip = 6;
        handler = (void (*)(int))process_epidemic_end_buttons;
        break;
    case 8:
        skip = 7;
        handler = (void (*)(int))process_mayor_start_buttons;
        break;
    case 9:
        skip = 8;
        handler = (void (*)(int))process_mayor_end_buttons;
        break;
    case 10:
        skip = 9;
        handler = (void (*)(int))process_no_diagnosis_buttons;
        break;
    }
    c_buttons = choice_text;
    while (--skip != (u8)0xff) {
        while (strcmp(*c_buttons, ".") != 0)
            c_buttons++;
        c_buttons++;
    }
    for (numbuttons = 0; strcmp(c_buttons[numbuttons], "!") != 0; numbuttons++)
        ;
    c_text = &c_buttons[numbuttons + 1];
    for (numtext = 0; strcmp(c_text[numtext], ".") != 0; numtext++)
        ;
    for (i = 0; i < numbuttons; i++) {
        resvars.f48[i + 0xae].x = resvars.f3c_ChoiceBoxp[173].x + resvars.f3c_ChoiceBoxp[173].w - resvars.f48[0xae].width;
        resvars.f48[i + 0xae].unknownC = resvars.f48[i + 0xae].x;
        resvars.f48[i + 0xae].y = resvars.f3c_ChoiceBoxp[173].y + resvars.f3c_ChoiceBoxp[173].h - (u16)(numbuttons - i) * (u16)(resvars.f48[0xae].height + 5);
        resvars.f48[i + 0xae].unknownE = resvars.f48[i + 0xae].y;
        resvars.f48[i + 0xae].callback = handler;
    }
    resvars.f48[numbuttons + 0xae].x = 0;
    resvars.f48[numbuttons + 0xae].y = 0;
}

// FUNCTION: 0x000a3db4
void display_buttons(void)
{
    u8 i;
    u8 unused8;
    s16 y;
    s16 width;

    width = resvars.f3c_ChoiceBoxp[173].w - resvars.f48[0xae].width;
    unused8 = 0;
    LbTextSetWindow(resvars.f3c_ChoiceBoxp[173].x, 0, width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1e);
    lbDisplay.draw_flags = 0x20;
    for (i = 0; (u16)resvars.f48[i + 0xae].x != 0; i++) {
        if (available_buttons & (1 << i))
            reqspr_draw((u16)resvars.f48[i + 0xae].x, (u16)resvars.f48[i + 0xae].y, &screen_sprites_tab_start[17]);
        else
            reqspr_draw((u16)resvars.f48[i + 0xae].x, (u16)resvars.f48[i + 0xae].y, &screen_sprites_tab_start[19]);
        format_text(c_buttons[i], width);
        y = resvars.f48[i + 0xae].y;
        y += (resvars.f48[i + 0xae].width - LbTextStringHeight(c_buttons[i])) / 2;
        LbTextDraw(0, y, c_buttons[i]);
        remove_newlines(c_buttons[i]);
    }
}

// FUNCTION: 0x000a3fb0
void display_choice_text(const u8 type)
{
    lbDisplay.draw_flags = 0x100;
    set_font_from_cache(0x1e);
    switch (type) {
    case 2:
        announce_emergency(player, c_text);
        break;
    case 3:
        emergency_over(player, c_text);
        break;
    case 7:
        call_no_treatment(&thing[choice_spare_data], c_text);
        break;
    case 4:
        announce_new_illness_fax(choice_spare_data, c_text);
        break;
    case 5:
        declare_epidemic(c_text);
        break;
    case 6:
        epidemic_over(player, c_text);
        break;
    case 8:
        mayor_requester(choice_spare_data, c_text);
        break;
    case 9:
        report_mayor_result(player, c_text);
        break;
    case 10:
        call_no_diagnosis(&thing[choice_spare_data], c_text);
        break;
    }
}

// FUNCTION: 0x000a410c
void display_choice_screen(const u8 owner, const u8 type, const s16 data, const s16 timeout)
{
    if (owner == player) {
        save_display_screen();
        display_mode = 9;
        D_00158192 = 0x5a;
        D_00158190 = type;
        choice_spare_data = data;
        choice_time_out = timeout;
    }
}

// FUNCTION: 0x000a4168
void packet_choice_result(u8 choice)
{
    D_001549d0[player].type = 0x7d;
    D_001549d0[player].value = choice;
    D_001549d0[player].arg = choice_spare_data;
}

// FUNCTION: 0x000a41b8
void save_display_screen(void)
{
    memmove(choice_backup.unknown2, &D_00158190, 3);
    choice_backup.mode = display_mode;
    choice_backup.f6 = choice_spare_data;
    choice_backup.f8 = choice_time_out;
}

// FUNCTION: 0x000a4208
void restore_display_screen(void)
{
    memmove(&D_00158190, choice_backup.unknown2, 3);
    display_mode = choice_backup.mode;
    choice_spare_data = choice_backup.f6;
    choice_time_out = choice_backup.f8;
    if (display_mode == 9)
        D_00158192 = 0x5a;
    choice_backup.mode = 0;
    switch (display_mode) {
    case 0:
    case 0xb:
        return_from_screen(0, 1, 1);
        break;
    case 1:
        bank.unknown2 = 0;
        drugs.unknown2 = 0;
        pol.unknown2 = 0;
        research.unknown2 = 0;
        graphs.unknown2 = 0;
        slist.unknown2 = 0;
        manager.unknown2 = 0;
        reputation.unknown2 = 0;
        variables.unknown2 = 0;
        objs.unknown2 = 1;
        options.unknown2 = 1;
        que.unknown2 = 0;
        break;
    }
}

// FUNCTION: 0x000a431c
u8 fax_no_equal(FaxNo fax, u8 *pressed)
{
    u8 i;

    if (NumPressed != fax.count)
        return 0;
    for (i = 0; i < NumPressed; i++) {
        if (fax.digits[i] != pressed[i])
            return 0;
    }
    return 1;
}

// FUNCTION: 0x000a4384
void process_fax_buttons(u8 button)
{
    u8 i;
    u8 found;

    switch (button) {
    case 0xb9:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xbe, -1, -1);
        break;
    case 0xba:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xbf, -1, -1);
        break;
    case 0xbb:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc0, -1, -1);
        break;
    case 0xbc:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc1, -1, -1);
        break;
    case 0xbd:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc2, -1, -1);
        break;
    case 0xbe:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc3, -1, -1);
        break;
    case 0xbf:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc4, -1, -1);
        break;
    case 0xc0:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc5, -1, -1);
        break;
    case 0xc1:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc6, -1, -1);
        break;
    case 0xc2:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = button;
        play_sound(10, 0xc7, -1, -1);
        break;
    case 0xc3:
    case 0xc4:
        play_sound(10, 0xbe, -1, -1);
        break;
    case 0xb8:
    case 0xc5:
    case 0xc6:
        i = found = 0;
        while (cheat_numbers[i].digits[0]) {
            if (fax_no_equal(cheat_numbers[i], D_001611f8)) {
                found = 1;
                switch (cheat_numbers[i].action) {
                case 1:
                    play_sound(10, 0xac, -1, -1);
                    break;
                case 0:
                    othervar.fa8e = !othervar.fa8e ? (u8)1 : (u8)0;
                    play_sound(10, 0xac, -1, -1);
                    break;
                case 2:
                    debug.f219 = 1;
                    play_sound(10, 0xac, -1, -1);
                    break;
                default:
                    found = 0;
                    break;
                }
            }
            i++;
        }
        if (!found)
            play_sound(10, 0xad, -1, -1);
        NumPressed = 0;
        break;
    case 0xc7:
        if (choice_time_out) {
            D_00158192 = 0x5c;
            play_sound(10, 0x42, -1, -1);
        }
        break;
    }
}
