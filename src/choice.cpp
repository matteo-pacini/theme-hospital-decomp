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
void process_mayor_start_buttons(u8 button);
void process_mayor_end_buttons(u8 button);

extern "C" {
extern Menu choice;
extern s16 choice_mouse_position;
extern u32 choice_actual_game_turn;
extern u8 fax_no[];
extern Event D_001549d0[];
}

FaxNo cheat_numbers[] = {
    {{0xc2, 0xc2, 0xc2}, 3, 1},
    {{0xc2, 0xba, 0xba}, 3, 1},
    {{0xc2, 0xba, 0xbb}, 3, 1},
    {{0xba, 0xb9, 0xb9}, 3, 1},
    {{0xba, 0xba, 0xba}, 3, 1},
    {{0xbb, 0xbb, 0xbb}, 3, 1},
    {{0xbc, 0xbc, 0xbc}, 3, 1},
    {{0xbd, 0xbd, 0xbd}, 3, 1},
    {{0xbe, 0xbe, 0xbe}, 3, 1},
    {{0xbf, 0xbf, 0xbf}, 3, 1},
    {{0xc0, 0xc0, 0xc0}, 3, 1},
    {{0xc1, 0xc1, 0xc1}, 3, 1},
    {{0xc2, 0xc2, 0xc2}, 3, 1},
    {{0xc0, 0xbb, 0xc1, 0xc0}, 4, 2},
    {{0xbb, 0xbd, 0xbc, 0xbb, 0xc1}, 5, 0},
    {{0}, 0, 0},
};

extern "C" {
int strcmp(const char *a, const char *b);
void display_cheat_mode(void);
}

// Does nothing in this build.
// FUNCTION: 0x000a3140
void display_cheat_mode(void)
{
}

// Runs one frame of the fax (choice) screen, by its state choice.state: 0x5a
// draws the message for choice with its buttons and moves to 0x5b, which
// handles the buttons and keypad and, if the fax can time out, closes it
// (0x5c) on Escape, Space, or choice_time_out turns without mouse movement.
// 0x5c restores the screen it replaced.
// FUNCTION: 0x000a3158
void display_choice(void)
{
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1e);
    lbDisplay.draw_flags = 0;
    switch (choice.state) {
    case 0x5a:
        available_buttons = 0xffff;
        load_ingame_pic(SCR_FAX, 1, 0x24, 0, (TbScreenMode)0xd);
        othervar.saved_screen_ptr = lbDisplay.wscreen;
        othervar.saved_screen_width = lbDisplay.graphics_screen_width;
        othervar.saved_screen_height = lbDisplay.graphics_screen_height;
        lbDisplay.wscreen = bscreen;
        lbDisplay.graphics_screen_width = lbDisplay.physical_screen_width;
        lbDisplay.graphics_screen_height = lbDisplay.physical_screen_height;
        LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
        LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
        calculate_buttons(choice.value);
        LbTextSetWindow(resvars.screen_boxes_ChoiceBoxp[173].x, resvars.screen_boxes_ChoiceBoxp[173].y,
                        resvars.screen_boxes_ChoiceBoxp[173].w, resvars.screen_boxes_ChoiceBoxp[173].h);
        display_choice_text(choice.value);
        display_buttons();
        if (!choice_time_out)
            reqspr_draw(resvars.screen_coords[0xbe].x, resvars.screen_coords[0xbe].y, &screen_sprites_tab_start[20]);
        lbDisplay.wscreen = othervar.saved_screen_ptr;
        lbDisplay.graphics_screen_width = othervar.saved_screen_width;
        lbDisplay.graphics_screen_height = othervar.saved_screen_height;
        LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height);
        choice.state = 0x5b;
        choice_actual_game_turn = actual_game_turn;
        NumPressed = 0;
        copy_screen_to_wscreen(bscreen);
        play_sound(SNDSRC_GENERAL, SFX_FAX_IN, -1, -1);
        break;
    case 0x5b:
        copy_screen_to_wscreen(bscreen);
        check_choice_buttons();
        check_fax_buttons();
        display_cheat_mode();
        if (choice_time_out) {
            if (lbInkey == KC_ESCAPE || lbInkey == KC_SPACE) {
                lbInkey = 0;
                play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
                choice.state = 0x5c;
            }
            if (choice_mouse_position != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
                choice_actual_game_turn = actual_game_turn;
                choice_mouse_position = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
            }
            if (actual_game_turn - choice_actual_game_turn > choice_time_out || exit_quick_requester())
                choice.state = 0x5c;
        }
        break;
    case 0x5c:
        restore_display_screen();
        break;
    }
}

// Draws pressed and runs the callbacks of the fax's available answer buttons
// (available_buttons bits) under the mouse.
// FUNCTION: 0x000a3430
void check_choice_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; (u16)resvars.screen_buttons[i + 0xae].x != 0 && (u16)resvars.screen_buttons[i + 0xae].y != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0xae].x,
                             resvars.screen_buttons[i + 0xae].y, (s16)resvars.screen_buttons[i + 0xae].width,
                             (s16)resvars.screen_buttons[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    reqspr_draw(resvars.screen_buttons[i + 0xae].sprite_x, resvars.screen_buttons[i + 0xae].sprite_y,
                                &screen_sprites_tab_start[18]);
                    break;
                }
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; (u16)resvars.screen_buttons[i + 0xae].x != 0 && (u16)resvars.screen_buttons[i + 0xae].y != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0xae].x,
                             resvars.screen_buttons[i + 0xae].y, (s16)resvars.screen_buttons[i + 0xae].width,
                             (s16)resvars.screen_buttons[i + 0xae].height)) {
                if (available_buttons & (1 << i)) {
                    play_sound(SNDSRC_GENERAL, SFX_YESX, -1, -1);
                    if (resvars.screen_buttons[i + 0xae].callback)
                        resvars.screen_buttons[i + 0xae].callback(i);
                    reqspr_draw(resvars.screen_buttons[i + 0xae].sprite_x, resvars.screen_buttons[i + 0xae].sprite_y,
                                &screen_sprites_tab_start[18]);
                    lbDisplay.rleft_button = lbDisplay.left_button = 0;
                    break;
                }
            }
        }
    }
}

// Draws pressed and runs the callbacks (with the button's index) of the fax's
// keypad buttons under the mouse; the close button only works if the fax can
// time out.
// FUNCTION: 0x000a374c
void check_fax_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0xb8].width != 0 && resvars.screen_buttons[i + 0xb8].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0xb8].x,
                             resvars.screen_buttons[i + 0xb8].y, (s16)resvars.screen_buttons[i + 0xb8].width,
                             (s16)resvars.screen_buttons[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                reqspr_draw(resvars.screen_buttons[i + 0xb8].sprite_x, resvars.screen_buttons[i + 0xb8].sprite_y,
                            (TbSprite *)((u32)screen_sprites_tab_start + resvars.screen_buttons[i + 0xb8].sprite * 6));
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0xb8].width != 0 && resvars.screen_buttons[i + 0xb8].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0xb8].x,
                             resvars.screen_buttons[i + 0xb8].y, (s16)resvars.screen_buttons[i + 0xb8].width,
                             (s16)resvars.screen_buttons[i + 0xb8].height)) {
                if (i + 0xb8 == 0xc7 && !choice_time_out)
                    break;
                if (resvars.screen_buttons[i + 0xb8].callback)
                    resvars.screen_buttons[i + 0xb8].callback((u8)(i + 0xb8));
                reqspr_draw(resvars.screen_buttons[i + 0xb8].sprite_x, resvars.screen_buttons[i + 0xb8].sprite_y,
                            (TbSprite *)((u32)screen_sprites_tab_start + resvars.screen_buttons[i + 0xb8].sprite * 6));
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// Finds the answer buttons and message text for choice type type in
// choice_text (sections ended by ".", buttons ended by "!"), sets c_buttons,
// numbuttons, c_text and numtext, and lays the buttons out with type's handler.
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
        resvars.screen_buttons[i + 0xae].x = resvars.screen_boxes_ChoiceBoxp[173].x +
                                             resvars.screen_boxes_ChoiceBoxp[173].w -
                                             resvars.screen_buttons[0xae].width;
        resvars.screen_buttons[i + 0xae].sprite_x = resvars.screen_buttons[i + 0xae].x;
        resvars.screen_buttons[i + 0xae].y = resvars.screen_boxes_ChoiceBoxp[173].y +
                                             resvars.screen_boxes_ChoiceBoxp[173].h -
                                             (u16)(numbuttons - i) * (u16)(resvars.screen_buttons[0xae].height + 5);
        resvars.screen_buttons[i + 0xae].sprite_y = resvars.screen_buttons[i + 0xae].y;
        resvars.screen_buttons[i + 0xae].callback = handler;
    }
    resvars.screen_buttons[numbuttons + 0xae].x = 0;
    resvars.screen_buttons[numbuttons + 0xae].y = 0;
}

// Draws the fax's answer buttons with their text, greyed if not in
// available_buttons. The text is centred using the button's width as its
// height.
// FUNCTION: 0x000a3db4
void display_buttons(void)
{
    u8 i;
    u8 unused8;
    s16 y;
    s16 width;

    width = resvars.screen_boxes_ChoiceBoxp[173].w - resvars.screen_buttons[0xae].width;
    unused8 = 0;
    LbTextSetWindow(resvars.screen_boxes_ChoiceBoxp[173].x, 0, width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x1e);
    lbDisplay.draw_flags = 0x20;
    for (i = 0; (u16)resvars.screen_buttons[i + 0xae].x != 0; i++) {
        if (available_buttons & (1 << i))
            reqspr_draw((u16)resvars.screen_buttons[i + 0xae].x, (u16)resvars.screen_buttons[i + 0xae].y,
                        &screen_sprites_tab_start[17]);
        else
            reqspr_draw((u16)resvars.screen_buttons[i + 0xae].x, (u16)resvars.screen_buttons[i + 0xae].y,
                        &screen_sprites_tab_start[19]);
        format_text(c_buttons[i], width);
        y = resvars.screen_buttons[i + 0xae].y;
        y += (resvars.screen_buttons[i + 0xae].width - LbTextStringHeight(c_buttons[i])) / 2;
        LbTextDraw(0, y, c_buttons[i]);
        remove_newlines(c_buttons[i]);
    }
}

// Draws the fax message of choice type type, for the local player, by calling
// the routine that composes it from c_text.
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

// Opens the fax screen for type type (2 emergency, 3 emergency over, 4 new
// illness, 5 epidemic, 6 epidemic over, 7 no treatment, 8 mayor, 9 mayor's
// result, 10 no diagnosis) if owner is the local player. data is the type's
// argument (a thing or illness); timeout, in turns without mouse movement,
// lets the fax close itself (0: it must be answered).
// FUNCTION: 0x000a410c
void display_choice_screen(const u8 owner, const u8 type, const s16 data, const s16 timeout)
{
    if (owner == player) {
        save_display_screen();
        display_mode = DM_CHOICE;
        choice.state = 0x5a;
        choice.value = type;
        choice_spare_data = data;
        choice_time_out = timeout;
    }
}

// Queues the local player's answer button to the fax as a turn event
// (EV_CHOICE_RESULT) with choice_spare_data.
// FUNCTION: 0x000a4168
void packet_choice_result(u8 button)
{
    D_001549d0[player].type = EV_CHOICE_RESULT;
    D_001549d0[player].value = button;
    D_001549d0[player].arg = choice_spare_data;
}

// Saves the display mode and fax state for restore_display_screen.
// FUNCTION: 0x000a41b8
void save_display_screen(void)
{
    memmove(choice_backup.choice_state, &choice, 3);
    choice_backup.mode = display_mode;
    choice_backup.spare_data = choice_spare_data;
    choice_backup.time_out = choice_time_out;
}

// Restores the display mode and fax state saved by save_display_screen,
// reopening a saved fax and resetting the menus of the screen returned to.
// FUNCTION: 0x000a4208
void restore_display_screen(void)
{
    memmove(&choice, choice_backup.choice_state, 3);
    display_mode = choice_backup.mode;
    choice_spare_data = choice_backup.spare_data;
    choice_time_out = choice_backup.time_out;
    if (display_mode == DM_CHOICE)
        choice.state = 0x5a;
    choice_backup.mode = DM_GAME;
    switch (display_mode) {
    case DM_GAME:
    case DM_MODAL:
        return_from_screen(0, 1, 1);
        break;
    case DM_CHARTS:
        bank.state = 0;
        drugs.state = 0;
        pol.state = 0;
        research.state = 0;
        graphs.state = 0;
        slist.state = 0;
        manager.state = 0;
        reputation.state = 0;
        variables.state = 0;
        objs.state = 1;
        options.state = 1;
        que.state = 0;
        break;
    }
}

// Returns 1 if the NumPressed digits in pressed match fax number fax.
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

// Handles fax keypad button button (0xb9-0xc2 dial 0-9, 0xb8/0xc5/0xc6 send
// the number, 0xc7 closes a fax that can time out). A number in cheat_numbers
// plays the success sound (action 0 toggles othervar.cheats_enabled, 2 sets
// debug.force_bonus_level); others play the fail sound.
// FUNCTION: 0x000a4384
void process_fax_buttons(u8 button)
{
    u8 i;
    u8 found;

    switch (button) {
    case 0xb9:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_0, -1, -1);
        break;
    case 0xba:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_1, -1, -1);
        break;
    case 0xbb:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_2, -1, -1);
        break;
    case 0xbc:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_3, -1, -1);
        break;
    case 0xbd:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_4, -1, -1);
        break;
    case 0xbe:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_5, -1, -1);
        break;
    case 0xbf:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_6, -1, -1);
        break;
    case 0xc0:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_7, -1, -1);
        break;
    case 0xc1:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_8, -1, -1);
        break;
    case 0xc2:
        if (NumPressed < 6)
            fax_no[NumPressed++] = button;
        play_sound(SNDSRC_GENERAL, SFX_FAX_9, -1, -1);
        break;
    case 0xc3:
    case 0xc4:
        play_sound(SNDSRC_GENERAL, SFX_FAX_0, -1, -1);
        break;
    case 0xb8:
    case 0xc5:
    case 0xc6:
        i = found = 0;
        while (cheat_numbers[i].digits[0]) {
            if (fax_no_equal(cheat_numbers[i], fax_no)) {
                found = 1;
                switch (cheat_numbers[i].action) {
                case 1:
                    play_sound(SNDSRC_GENERAL, SFX_FAX_YES, -1, -1);
                    break;
                case 0:
                    othervar.cheats_enabled = !othervar.cheats_enabled ? (u8)1 : (u8)0;
                    play_sound(SNDSRC_GENERAL, SFX_FAX_YES, -1, -1);
                    break;
                case 2:
                    debug.force_bonus_level = 1;
                    play_sound(SNDSRC_GENERAL, SFX_FAX_YES, -1, -1);
                    break;
                default:
                    found = 0;
                    break;
                }
            }
            i++;
        }
        if (!found)
            play_sound(SNDSRC_GENERAL, SFX_FAX_NO, -1, -1);
        NumPressed = 0;
        break;
    case 0xc7:
        if (choice_time_out) {
            choice.state = 0x5c;
            play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        }
        break;
    }
}
