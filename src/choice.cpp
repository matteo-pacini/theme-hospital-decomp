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
void process_emergency_start_buttons(u8 n);
void process_emergency_end_buttons(u8 n);
void process_epidemic_start_buttons(u8 n);
void process_epidemic_end_buttons(u8 n);
void process_mayor_start_buttons(u8 n);
void process_mayor_end_buttons(u8 n);

extern "C" {
extern s32 D_00188c38;
extern u16 D_00188c74;
extern u8 D_00188c7b;
extern u8 D_00188c7e;
extern u8 D_00188c81;
extern volatile s16 D_00188c6c;
extern volatile s16 D_00188c70;
extern u8 D_00158192;
extern u16 D_00158190;
extern u8 *D_00155d8f;
extern s32 D_00155d93;
extern s32 D_00155d97;
extern ChoiceBox *D_0015f430;
extern Point *D_0015f434;
extern Button *D_0015f43c;
extern s16 D_0015f760;
extern u32 D_001611f0;
extern u8 D_001611f8[];
extern Event D_001549d0[];
extern FaxNo D_00149598[];
extern u8 D_00155e46;
extern char D_00121af4[];
extern char D_00121af6[];
extern char D_00121af8[];
int strcmp(const char *a, const char *b);
void fn_000a3140(void);
}

// NONMATCHING: register choice
// FUNCTION: 0x000a3158
void display_choice(void)
{
    LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
    LbScreenSetGraphicsWindow(0, 0, D_00188c34, D_00188c38);
    set_font_from_cache(0x1e);
    D_00188c74 = 0;
    switch (D_00158192) {
    case 0x5a:
        available_buttons = 0xffff;
        load_ingame_pic(0x13, 1, 0x24, 0, (TbScreenMode)0xd);
        D_00155d8f = lbDisplay.unknown4;
        D_00155d93 = D_00188c34;
        D_00155d97 = D_00188c38;
        lbDisplay.unknown4 = bscreen;
        D_00188c34 = lbDisplay.unknown18;
        D_00188c38 = lbDisplay.unknown1c;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
        LbTextSetWindow(0, 0, D_00188c34, D_00188c38);
        calculate_buttons(D_00158190);
        LbTextSetWindow(D_0015f430[173].x, D_0015f430[173].y, D_0015f430[173].w, D_0015f430[173].h);
        display_choice_text(D_00158190);
        display_buttons();
        if (!choice_time_out)
            reqspr_draw(D_0015f434[0xbe].x, D_0015f434[0xbe].y, &screen_sprites_tab_start[20]);
        lbDisplay.unknown4 = D_00155d8f;
        D_00188c34 = D_00155d93;
        D_00188c38 = D_00155d97;
        LbScreenSetGraphicsWindow(0, 0, D_00155d93, D_00155d97);
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
            if (D_0015f760 != mouse_dx + mouse_dy) {
                D_001611f0 = actual_game_turn;
                D_0015f760 = mouse_dx + mouse_dy;
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

// NONMATCHING: register choice
// FUNCTION: 0x000a3430
void check_choice_buttons(void)
{
    u8 i;

    if (D_00188c7e) {
        for (i = 0; (u16)D_0015f43c[i + 0xae].x != 0 && (u16)D_0015f43c[i + 0xae].y != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0xae].x, D_0015f43c[i + 0xae].y,
                             (s16)D_0015f43c[i + 0xae].width, (s16)D_0015f43c[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    reqspr_draw(D_0015f43c[i + 0xae].unknownC, D_0015f43c[i + 0xae].unknownE, &screen_sprites_tab_start[18]);
                    break;
                }
            }
        }
    }
    if (D_00188c81) {
        for (i = 0; (u16)D_0015f43c[i + 0xae].x != 0 && (u16)D_0015f43c[i + 0xae].y != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0xae].x, D_0015f43c[i + 0xae].y,
                             (s16)D_0015f43c[i + 0xae].width, (s16)D_0015f43c[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    play_sound(10, 0x41, -1, -1);
                    if (D_0015f43c[i + 0xae].callback)
                        D_0015f43c[i + 0xae].callback(i);
                    reqspr_draw(D_0015f43c[i + 0xae].unknownC, D_0015f43c[i + 0xae].unknownE, &screen_sprites_tab_start[18]);
                    D_00188c81 = D_00188c7b = 0;
                    break;
                }
            }
        }
    }
}

// NONMATCHING: register choice
// FUNCTION: 0x000a374c
void check_fax_buttons(void)
{
    u8 i;

    if (D_00188c7e) {
        for (i = 0; D_0015f43c[i + 0xb8].width != 0 && D_0015f43c[i + 0xb8].height != 0; i++) {
            if (over_hotspot((s16)mouse_dx, (s16)mouse_dy, D_0015f43c[i + 0xb8].x, D_0015f43c[i + 0xb8].y,
                             (s16)D_0015f43c[i + 0xb8].width, (s16)D_0015f43c[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                reqspr_draw(D_0015f43c[i + 0xb8].unknownC, D_0015f43c[i + 0xb8].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0xb8].sprite * 6));
                break;
            }
        }
    }
    if (D_00188c81) {
        for (i = 0; D_0015f43c[i + 0xb8].width != 0 && D_0015f43c[i + 0xb8].height != 0; i++) {
            if (over_hotspot(D_00188c6c, D_00188c70, D_0015f43c[i + 0xb8].x, D_0015f43c[i + 0xb8].y,
                             (s16)D_0015f43c[i + 0xb8].width, (s16)D_0015f43c[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                if (D_0015f43c[i + 0xb8].callback)
                    D_0015f43c[i + 0xb8].callback((u8)(i + 0xb8));
                reqspr_draw(D_0015f43c[i + 0xb8].unknownC, D_0015f43c[i + 0xb8].unknownE,
                            (TbSprite *)((u32)screen_sprites_tab_start + D_0015f43c[i + 0xb8].sprite * 6));
                break;
            }
        }
        D_00188c81 = 0;
    }
}

// NONMATCHING: register choice
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
        while (strcmp(*c_buttons, D_00121af4) != 0)
            c_buttons++;
        c_buttons++;
    }
    for (numbuttons = 0; strcmp(c_buttons[numbuttons], D_00121af6) != 0; numbuttons++)
        ;
    c_text = &c_buttons[numbuttons + 1];
    for (numtext = 0; strcmp(c_text[numtext], D_00121af8) != 0; numtext++)
        ;
    for (i = 0; i < numbuttons; i++) {
        D_0015f43c[i + 0xae].x = D_0015f430[173].x + D_0015f430[173].w - D_0015f43c[0xae].width;
        D_0015f43c[i + 0xae].unknownC = D_0015f43c[i + 0xae].x;
        D_0015f43c[i + 0xae].y = D_0015f430[173].y + D_0015f430[173].h - (D_0015f43c[0xae].height + 5) * (numbuttons - i);
        D_0015f43c[i + 0xae].unknownE = D_0015f43c[i + 0xae].y;
        D_0015f43c[i + 0xae].callback = handler;
    }
    D_0015f43c[numbuttons + 0xae].x = 0;
    D_0015f43c[numbuttons + 0xae].y = 0;
}

// NONMATCHING: register choice
// FUNCTION: 0x000a3db4
void display_buttons(void)
{
    u8 i;
    u8 unused8;
    s16 y;
    s16 width;

    width = D_0015f430[173].w - D_0015f43c[0xae].width;
    unused8 = 0;
    LbTextSetWindow(D_0015f430[173].x, 0, width, D_00188c38);
    set_font_from_cache(0x1e);
    D_00188c74 = 0x20;
    for (i = 0; (u16)D_0015f43c[i + 0xae].x != 0; i++) {
        if (available_buttons & (1 << i))
            reqspr_draw((u16)D_0015f43c[i + 0xae].x, (u16)D_0015f43c[i + 0xae].y, &screen_sprites_tab_start[17]);
        else
            reqspr_draw((u16)D_0015f43c[i + 0xae].x, (u16)D_0015f43c[i + 0xae].y, &screen_sprites_tab_start[19]);
        format_text(c_buttons[i], width);
        y = D_0015f43c[i + 0xae].y;
        y += (D_0015f43c[i + 0xae].width - LbTextStringHeight(c_buttons[i])) / 2;
        LbTextDraw(0, y, c_buttons[i]);
        remove_newlines(c_buttons[i]);
    }
}

// FUNCTION: 0x000a3fb0
void display_choice_text(const u8 type)
{
    D_00188c74 = 0x100;
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

// NONMATCHING: register choice
// FUNCTION: 0x000a4168
void packet_choice_result(u8 n)
{
    D_001549d0[player].type = 0x7d;
    D_001549d0[player].value = n;
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

// NONMATCHING: register choice
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
void process_fax_buttons(u8 n)
{
    u8 i;
    u8 found;

    switch (n) {
    case 0xb9:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xbe, -1, -1);
        break;
    case 0xba:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xbf, -1, -1);
        break;
    case 0xbb:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc0, -1, -1);
        break;
    case 0xbc:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc1, -1, -1);
        break;
    case 0xbd:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc2, -1, -1);
        break;
    case 0xbe:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc3, -1, -1);
        break;
    case 0xbf:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc4, -1, -1);
        break;
    case 0xc0:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc5, -1, -1);
        break;
    case 0xc1:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
        play_sound(10, 0xc6, -1, -1);
        break;
    case 0xc2:
        if (NumPressed < 6)
            D_001611f8[NumPressed++] = n;
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
        while (D_00149598[i].digits[0]) {
            if (fax_no_equal(D_00149598[i], D_001611f8)) {
                found = 1;
                switch (D_00149598[i].action) {
                case 1:
                    play_sound(10, 0xac, -1, -1);
                    break;
                case 0:
                    D_00155e46 = !D_00155e46 ? (u8)1 : (u8)0;
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
