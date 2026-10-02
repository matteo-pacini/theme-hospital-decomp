#include "hosp.h"

struct HighScore {
    char name[0x20];
    s32 score;
};

struct HospScores {
    HighScore fame[10];
    HighScore shame[10];
};

extern "C" {

u32 lib_strlen(char *s);
#pragma aux lib_strlen "strlen_"
int strcmp(const char *a, const char *b);
void modify_background(void);
void display_hall(void);
void insert_hall_details(void);
void draw_fame_text(void);
void add_highscore_fame(s8 slot);
void bubble_sort_fame(void);
void add_highscore_shame(s8 slot);
void bubble_sort_shame(void);
}

extern s32 TextPos[];
extern s32 DesiredTextPos[];
extern s32 TitlePos[];
extern s32 DesiredTitlePos[];
extern char *TitleText;
extern u8 TitleLen;
HospScores hosp_scores = {
    {
        {"WEBLEY", 250000},
        {"SAMUEL", 210000},
        {"JAYNE", 200000},
        {"GAZZA", 190000},
        {"CHIVERS", 175000},
        {"RAJ", 165000},
        {"WAZ", 145000},
        {"BEN", 99000},
        {"LORD RIDER", 79000},
        {"BASSEY", 40000},
    },
    {
        {"JAN SV.", 8000},
        {"COGGY", 10000},
        {"BRUTUS", 11000},
        {"NUTTERS", 12500},
        {"ROBBO", 16000},
        {"NATS", 16500},
        {"MR MAT", 17000},
        {"JON", 18000},
        {"ADIE", 19000},
        {"RUSS", 25000},
    },
};

void move_text(void);
void set_fame_pos(s32 *text, s32 *title);
void set_shame_pos(s32 *text, s32 *title);
void check_hall_buttons(void);
void save_highscores(void);
void load_highscores(void);

// Moves the hall of fame's rows and title letters an eighth of the way to
// their targets (16.16 fixed point), and switches the title between fame
// and shame (othervar.showing_fame) when the rows cross over.
// FUNCTION: 0x00094500
void move_text(void)
{
    u8 old;
    s16 i;

    for (i = 0; i < 10; i++) {
        TextPos[i] += (DesiredTextPos[i] - TextPos[i] + 0x1000) >> 3;
        if (TextPos[i] > DesiredTextPos[i]) {
            if (TextPos[i] - DesiredTextPos[i] < 0x800)
                TextPos[i] = DesiredTextPos[i];
        }
        if (TextPos[i] < DesiredTextPos[i]) {
            if (DesiredTextPos[i] - TextPos[i] < 0x800)
                TextPos[i] = DesiredTextPos[i];
        }
    }
    old = othervar.showing_fame;
    othervar.showing_fame = TextPos[0] < TextPos[5] ? (u8)1 : (u8)0;
    if (othervar.showing_fame != old) {
        if (othervar.showing_fame) {
            TitleText = hall_text[4];
            TitleLen = lib_strlen(TitleText);
        } else {
            TitleText = hall_text[5];
            TitleLen = lib_strlen(TitleText);
        }
    }
    for (i = 0; i < 20; i++) {
        TitlePos[i] += (DesiredTitlePos[i] - TitlePos[i] + 0x1000) >> 3;
        if (TitlePos[i] > DesiredTitlePos[i]) {
            if (TitlePos[i] - DesiredTitlePos[i] < 0x800)
                TitlePos[i] = DesiredTitlePos[i];
        }
        if (TitlePos[i] < DesiredTitlePos[i]) {
            if (DesiredTitlePos[i] - TitlePos[i] < 0x800)
                TitlePos[i] = DesiredTitlePos[i];
        }
    }
}

// Sets the row and title letter positions (16.16 fixed point) for showing
// the hall of fame.
// FUNCTION: 0x000947f0
void set_fame_pos(s32 *text, s32 *title)
{
    char *saved;
    s16 i;
    u8 c;

    for (i = 0; i < 10; i++)
        text[i] = resvars.screen_coords[i + 191].y << 16;
    saved = TitleText;
    TitleText = hall_text[4];
    for (i = 0; i < 20; i++) {
        c = TitleText[i];
        title[i] = (lbDisplay.graphics_screen_width - LbTextStringWidth(TitleText)) / 2;
        TitleText[i] = 0;
        title[i] += LbTextStringWidth(TitleText);
        TitleText[i] = c;
        title[i] = title[i] << 16;
    }
    TitleText = saved;
}

// Sets the row and title letter positions (16.16 fixed point) for showing
// the hall of shame.
// FUNCTION: 0x00094914
void set_shame_pos(s32 *text, s32 *title)
{
    char *saved;
    s16 i;

    for (i = 0; i < 10; i++)
        text[i] = resvars.screen_coords[200 - i].y << 16;
    saved = TitleText;
    TitleText = hall_text[5];
    for (i = 0; i < 20; i++) {
        title[i] = (lbDisplay.graphics_screen_width + LbTextStringWidth(TitleText)) / 2;
        title[i] -= LbTextStringWidth(&TitleText[lib_strlen(TitleText) - 1 - i]);
        title[i] = title[i] << 16;
    }
    TitleText = saved;
}

// Draws the hall of fame's fixed labels into the background screen.
// FUNCTION: 0x00094a18
void modify_background(void)
{
    s16 i;

    set_font_from_cache(0x10);
    othervar.saved_screen_ptr = lbDisplay.wscreen;
    othervar.saved_screen_width = lbDisplay.graphics_screen_width;
    othervar.saved_screen_height_u = lbDisplay.graphics_screen_height;
    lbDisplay.wscreen = bscreen;
    lbDisplay.graphics_screen_width = lbDisplay.physical_screen_width;
    lbDisplay.graphics_screen_height = lbDisplay.physical_screen_height;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.physical_screen_width, lbDisplay.physical_screen_height);
    for (i = 1; resvars.screen_coords[i + 200].y != 0; i++)
        LbTextDraw(resvars.screen_coords[i + 200].x, resvars.screen_coords[i + 200].y, hall_text[i]);
    lbDisplay.wscreen = othervar.saved_screen_ptr;
    lbDisplay.graphics_screen_width = othervar.saved_screen_width;
    lbDisplay.graphics_screen_height = othervar.saved_screen_height_u;
    LbScreenSetGraphicsWindow(0, 0, othervar.saved_screen_width, othervar.saved_screen_height_u);
}

// Runs one frame of the hall of fame screen (hall.state: 0 opening, 1
// shown, 2 done, which moves on to the next year-end screen); closes by
// itself after 1200 frames without mouse movement.
// FUNCTION: 0x00094b28
void display_hall(void)
{
    s8 i;

    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    set_font_from_cache(0x10);
    lbDisplay.draw_flags = 0;
    switch (hall.state) {
    case 0:
        load_ingame_pic(SCR_HALL_OF_FAME, 1, 0x16, 0, SCREEN_MODE_13);
        load_highscores();
        modify_background();
        hall.state = 1;
        othervar.idle_countdown = 1200;
        for (i = 0; i < 20; i++)
            TitlePos[i] = DesiredTitlePos[i] = lbDisplay.graphics_screen_width / 2;
        if (othervar.showing_fame) {
            set_fame_pos(DesiredTextPos, DesiredTitlePos);
            set_fame_pos(TextPos, TitlePos);
            TitleText = hall_text[4];
        } else {
            set_shame_pos(DesiredTextPos, DesiredTitlePos);
            set_shame_pos(TextPos, TitlePos);
            TitleText = hall_text[5];
        }
        TitleLen = lib_strlen(TitleText);
        move_text();
        draw_fame_text();
        break;
    case 1:
        copy_screen_to_wscreen(bscreen);
        move_text();
        draw_fame_text();
        break;
    case 2:
        set_font_from_cache(1);
        othervar.full_redraw = 1;
        year_end_type++;
        break;
    default:
        break;
    }
    check_hall_buttons();
    if (othervar.last_mouse_sum != lbDisplay.mmouse_x + lbDisplay.mmouse_y) {
        othervar.idle_countdown = 1200;
        othervar.last_mouse_sum = lbDisplay.mmouse_x + lbDisplay.mmouse_y;
    }
    if (!othervar.idle_countdown--)
        hall.state = 2;
}

// Draws the sliding title and the ten rows of the hall of fame or shame.
// FUNCTION: 0x00094d34
void draw_fame_text(void)
{
    u8 i;
    u8 c;
    u16 height;
    char *letter;
    s32 x;
    char buf[100];

    set_font_from_cache(0x10);
    height = lbFontPtr ? lbFontPtr[34].height : 0;
    if (othervar.showing_fame)
        letter = TitleText;
    else
        letter = &TitleText[TitleLen - 1];
    for (i = 0; i < TitleLen; i++) {
        x = (TitlePos[i] + 0x8000) >> 16;
        c = letter[1];
        letter[1] = 0;
        LbTextDraw(x, resvars.screen_coords[205].y, letter);
        x += LbTextStringWidth(letter);
        letter[1] = c;
        if (othervar.showing_fame)
            letter++;
        else
            letter--;
    }
    for (i = 0; i < 10; i++) {
        x = (TextPos[i] + 0x8000) >> 16;
        sprintf(buf, "%2d", i + 1);
        LbTextDraw(resvars.screen_coords[201].x, x, buf);
        if (othervar.showing_fame) {
            LbTextDraw(resvars.screen_coords[202].x, x, hosp_scores.fame[i].name);
            sprintf(buf, "%6d", hosp_scores.fame[i].score);
            LbTextDraw(resvars.screen_coords[203].x, x, buf);
        } else {
            LbTextDraw(resvars.screen_coords[202].x, x, hosp_scores.shame[9 - i].name);
            sprintf(buf, "%6d", hosp_scores.shame[9 - i].score);
            LbTextDraw(resvars.screen_coords[203].x, x, buf);
        }
    }
}

// Writes this player's hospital name and score (allhosps.score) into hall of
// fame slot `slot`.
// FUNCTION: 0x00094f5c
void add_highscore_fame(s8 slot)
{
    sprintf(hosp_scores.fame[slot].name, "%s", &allhosps[player]);
    hosp_scores.fame[slot].score = allhosps[player].score;
}

// Sorts the hall of fame by descending score.
// FUNCTION: 0x00094fc8
void bubble_sort_fame(void)
{
    s8 i;
    s8 j;
    u8 swapped;
    HighScore tmp;

    for (i = 0; i < 9; i++) {
        swapped = 0;
        for (j = 9; j > i; j--) {
            if (hosp_scores.fame[j].score > hosp_scores.fame[j - 1].score) {
                tmp = hosp_scores.fame[j];
                hosp_scores.fame[j] = hosp_scores.fame[j - 1];
                hosp_scores.fame[j - 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

// Writes this player's hospital name and score into hall of shame slot
// `slot`.
// FUNCTION: 0x00095094
void add_highscore_shame(s8 slot)
{
    sprintf(hosp_scores.shame[slot].name, "%s", &allhosps[player]);
    hosp_scores.shame[slot].score = allhosps[player].score;
}

// Sorts the hall of shame by ascending score.
// FUNCTION: 0x00095100
void bubble_sort_shame(void)
{
    s8 i;
    s8 j;
    u8 swapped;
    HighScore tmp;

    for (i = 9; i > 0; i--) {
        swapped = 0;
        for (j = 0; j < i; j++) {
            if (hosp_scores.shame[j].score > hosp_scores.shame[j + 1].score) {
                tmp = hosp_scores.shame[j];
                hosp_scores.shame[j] = hosp_scores.shame[j + 1];
                hosp_scores.shame[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

// Enters this player's score in the hall of fame if it beats the lowest
// entry, else in the hall of shame if it beats its lowest, replacing their
// own entry if they have one with a lower score, and saves the table. Sets
// which hall is shown (othervar.showing_fame).
// FUNCTION: 0x000951c8
void insert_hall_details(void)
{
    s8 i;

    load_highscores();
    if (allhosps[player].score > hosp_scores.fame[9].score) {
        for (i = 0; i < 10; i++) {
            if (strcmp((char *)allhosps[player].name, hosp_scores.fame[i].name) == 0)
                break;
        }
        if (i == 10) {
            i--;
            add_highscore_fame(i);
            bubble_sort_fame();
            save_highscores();
        } else if (allhosps[player].score > hosp_scores.fame[i].score) {
            add_highscore_fame(i);
            bubble_sort_fame();
            save_highscores();
        }
        othervar.showing_fame = 1;
    } else {
        if (allhosps[player].score > hosp_scores.shame[0].score) {
            for (i = 0; i < 10; i++) {
                if (strcmp((char *)allhosps[player].name, hosp_scores.shame[i].name) == 0)
                    break;
            }
            if (i == 10) {
                i--;
                add_highscore_shame(i);
                bubble_sort_shame();
                save_highscores();
            } else if (allhosps[player].score > hosp_scores.shame[i].score) {
                add_highscore_shame(i);
                bubble_sort_shame();
                save_highscores();
            }
        }
        othervar.showing_fame = 0;
    }
}

// Hall of fame buttons: 0 and 1 switch between fame and shame, 2 closes.
// FUNCTION: 0x0009536c
void process_hall_buttons(u8 button)
{
    switch (button) {
    case 0:
    case 1:
        if (othervar.showing_fame) {
            play_sound(SNDSRC_GENERAL, SFX_UPDOWN, -1, -1);
            set_shame_pos(DesiredTextPos, DesiredTitlePos);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_UPDOWN, -1, -1);
            set_fame_pos(DesiredTextPos, DesiredTitlePos);
        }
        break;
    case 2:
        hall.state = 2;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    default:
        break;
    }
}

// Hall of fame clicks: presses the button under the mouse and runs its
// callback on release.
// FUNCTION: 0x0009542c
void check_hall_buttons(void)
{
    u8 i;

    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0xa9].width != 0 && resvars.screen_buttons[i + 0xa9].height != 0; i++) {
            if (over_hotspot(lbDisplay.mmouse_x, lbDisplay.mmouse_y, resvars.screen_buttons[i + 0xa9].x,
                             resvars.screen_buttons[i + 0xa9].y, resvars.screen_buttons[i + 0xa9].width,
                             resvars.screen_buttons[i + 0xa9].height)) {
                reqspr_draw(resvars.screen_buttons[i + 0xa9].sprite_x, resvars.screen_buttons[i + 0xa9].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0xa9].sprite]);
                break;
            }
        }
    }
    if (lbDisplay.rleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0xa9].width != 0 && resvars.screen_buttons[i + 0xa9].height != 0; i++) {
            if (over_hotspot(lbDisplay.rmouse_x, lbDisplay.rmouse_y, resvars.screen_buttons[i + 0xa9].x,
                             resvars.screen_buttons[i + 0xa9].y, resvars.screen_buttons[i + 0xa9].width,
                             resvars.screen_buttons[i + 0xa9].height)) {
                resvars.screen_buttons[i + 0xa9].callback(i);
                reqspr_draw(resvars.screen_buttons[i + 0xa9].sprite_x, resvars.screen_buttons[i + 0xa9].sprite_y,
                            &screen_sprites_tab_start[resvars.screen_buttons[i + 0xa9].sprite]);
                break;
            }
        }
        lbDisplay.rleft_button = 0;
    }
}

// Writes the high score table to save/hiscore.dat (in a network game only on
// player 0's machine).
// FUNCTION: 0x00095710
void save_highscores(void)
{
    int fh;
    char path[100];

    if (statvar.num_players <= 1 || (statvar.num_players > 1 && player == 0)) {
        sprintf(path, "%ssave/hiscore.dat", statvar.save_root);
        fh = LbFileOpen(path, 0);
        if (fh != -1) {
            LbFileWrite(fh, &hosp_scores, 0x2d0);
            LbFileClose(fh);
        }
    }
}

// Reads the high score table from save/hiscore.dat, if present.
// FUNCTION: 0x00095790
void load_highscores(void)
{
    int fh;
    char path[100];

    sprintf(path, "%ssave/hiscore.dat", statvar.save_root);
    fh = LbFileOpen(path, 2);
    if (fh != -1) {
        LbFileRead(fh, &hosp_scores, 0x2d0);
        LbFileClose(fh);
    }
}
