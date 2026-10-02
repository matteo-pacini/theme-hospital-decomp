#include "hosp.h"

struct MenuItem {
    u8 unknown0[8];
    u16 text;
    u8 unknown1[6];
};

extern "C" {

extern Event player_events[];
extern char **cabinet_text, **illness_text;
extern char **skill_names;

void conv_string(void *dst, char *src, int mode);
}

// Runs one frame of the casebook screen (drugs.state: 0 loads it, 2 shows
// it, 1 asks to return to the game): the selected illness's figures (price or
// price percentage, earnings, reputation, ...), the illnesses listed around it
// (clicking one selects it), and what is needed to treat it. Then handles the
// buttons and keys.
// FUNCTION: 0x00012690
void display_drugs(void)
{
    u8 unused30;
    u8 count;
    u8 unused18;
    u8 no_room;
    u8 unavailable;
    u8 weak_drug;
    u8 i;
    u8 unused28;
    u8 unused24;
    u8 unused20;
    u8 unused1c;
    u16 value;
    s16 line_height;
    u16 illness;
    Expertise *def;
    int result;
    int n;
    u8 list[18];
    char buf[100];

    n = 0;
    unused28 = 0;
    unused24 = 0;
    unused20 = 0;
    unused1c = 0;
    unused18 = 0;
    no_room = 0;
    unavailable = 0;
    weak_drug = 0;
    unused30 = 0;
    compute_help_text(0, 0, 0xe);
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    othervar.concentrate_help_on = 0;
    switch (drugs.state) {
    case 0:
        fade_out_menu();
        load_ingame_pic(SCR_DRUGS, 1, 0x1b, 0, SCREEN_MODE_13);
        drugs.state = 2;
        drugs.value = 0;
        othervar.repeat_step = 0x100;
        othervar.drug_pct_display_time = 0;
        break;
    case 1:
        player_events[player].type = EV_RETURN_FROM_SCREEN;
        player_events[player].value = 0;
        break;
    case 2:
        copy_screen_to_wscreen(bscreen);
        break;
    default:
        break;
    }
    set_font_from_cache(0xe);
    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    LbTextSetWindow(resvars.screen_boxes->drugs_text_x, resvars.screen_boxes->drugs_text_y,
                    resvars.screen_boxes->drugs_text_w, resvars.screen_boxes->drugs_text_h);
    lbDisplay.draw_flags = 0x100;
    illness = othervar.selected_illness;
    def = &expertise[illness];
    result = max_price_for_treatment(illness, player);
    for (i = 0; i + 0x11 != 0x1d; i++) {
        switch (i) {
        case 6:
            if (illness >= 0x24 || illness == 1) {
                sprintf(buf, "%d", myhosp->reputation);
            } else {
                sprintf(buf, "%d", myhosp->rooms[illness].reputation);
            }
            break;
        case 7:
            if (othervar.drug_pct_display_time) {
                othervar.drug_pct_display_time--;
                sprintf(buf, "%d%%", myhosp->rooms[illness].price_scale / 100);
            } else {
                sprintf(buf, "$%d", myhosp->rooms[illness].price);
            }
            break;
        case 8:
            sprintf(buf, "$%d", myhosp->rooms[illness].income);
            break;
        case 9:
            sprintf(buf, "%d", myhosp->rooms[illness].cured);
            break;
        case 10:
            sprintf(buf, "%d", myhosp->rooms[illness].deaths);
            break;
        case 11:
            sprintf(buf, "%d", myhosp->rooms[illness].turned_away);
            break;
        default:
            sprintf(buf, "%s", cabinet_text[i]);
            conv_string(buf, buf, 1);
            break;
        }
        LbTextDraw(resvars.screen_coords[i + 0x11].x, resvars.screen_coords[i + 0x11].y, buf);
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    for (i = 0; i < 0x12; i++) {
        list[i] = 0;
    }
    n = 0;
    list[8] = othervar.selected_illness;
    count = 0;
    if (othervar.selected_illness + 1 < 0x2e) {
        for (i = othervar.selected_illness + 1; i < 0x2e; i++) {
            if ((i < 0x24 && myhosp->rooms[i].discovered) ||
                (i >= 0x24 && allhosps[player].rooms[i].cure_available == 1) ||
                (i == 0x2c &&
                 (allhosps[player].equipment[55].available != 0 &&
                  objects[OBJ_AUTO_AUTOPSY].available_for_level != 0) &&
                 listhead[player].rooms[ROOM_RESEARCH])) {
                if (count >= 7) {
                    break;
                }
                list[count + 9] = i;
                n++;
                count++;
            }
        }
    }
    if (othervar.selected_illness - 1 > 0) {
        n = 7;
        count = 7;
        for (i = othervar.selected_illness - 1; i > 0; i--) {
            if (((i < 0x24 && myhosp->rooms[i].discovered) ||
                 (i >= 0x24 && allhosps[player].rooms[i].cure_available == 1)) &&
                i < 0x2c) {
                if (count == 0) {
                    break;
                }
                list[count] = i;
                n++;
                count--;
            }
        }
    }
    count = 0;
    n = 0;
    for (i = 0; i < 0x11; i++) {
        if (list[i]) {
            sprintf(buf, "%s", illness_text[list[i]]);
            conv_string(buf, buf, 0);
            if (i == 8) {
                set_font_from_cache(0xf);
            } else {
                set_font_from_cache(0xe);
                if (lbDisplay.left_button) {
                    if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.screen_coords[i].x,
                                     resvars.screen_coords[i].y,
                                     lbDisplay.graphics_screen_width - resvars.screen_coords[i].y, line_height)) {
                        othervar.selected_illness = list[i];
                        play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
                        lbDisplay.left_button = 0;
                    }
                }
            }
            LbTextDraw(resvars.screen_coords[i].x, resvars.screen_coords[i].y, buf);
        }
    }
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    lbDisplay.draw_flags = 0;
    set_font_from_cache(0xe);
    lbDisplay.draw_flags = 0x100;
    LbTextSetWindow(resvars.screen_coords[0x1e].x, resvars.screen_coords[0x1e].y, 0x36, 0x1a);
    LbTextDraw(0, (0x1a - (lbFontPtr ? lbFontPtr[34].height : 0)) / 2, cabinet_text[7]);
    lbDisplay.draw_flags = 0;
    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
    if (!(illness >= 0x24 || illness == 1)) {
        if (allhosps[player].rooms[illness].cure_available == 1) {
            if (def->kind == 0x27) {
                value = allhosps[player].rooms[illness].drug_effectiveness;
                reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y, screen_sprites_tab_start + 7);
                set_font_from_cache(0x2e);
                sprintf(buf, "%d", value);
                LbTextSetWindow(0x134, resvars.screen_coords[0x21].y, 0x1c, 0x32);
                lbDisplay.draw_flags = 0x100;
                LbTextDraw(0, 0, buf);
                if (value < 100) {
                    weak_drug = 1;
                }
                LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
                lbDisplay.draw_flags = 0;
            } else if (def->category == 10) {
                reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y, screen_sprites_tab_start + 8);
            } else if (def->outcome != 1 && def->outcome != 5 && def->outcome != 4) {
                reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y, screen_sprites_tab_start + 6);
            }
            reqspr_draw(resvars.screen_coords[0x20].x, resvars.screen_coords[0x20].y, screen_sprites_tab_start + 11);
        } else {
            reqspr_draw(resvars.screen_coords[0x20].x, resvars.screen_coords[0x20].y, screen_sprites_tab_start + 12);
            if (def->outcome != 1 && def->outcome != 5 && def->outcome != 4) {
                if (def->kind == 0x27) {
                    reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y,
                                screen_sprites_tab_start + 7);
                    set_font_from_cache(0x2e);
                    sprintf(buf, "?");
                    LbTextSetWindow(0x134, resvars.screen_coords[0x21].y, 0x1c, 0x32);
                    lbDisplay.draw_flags = 0x100;
                    LbTextDraw(0, 0, buf);
                    unavailable = 1;
                    LbTextSetWindow(0, 0, lbDisplay.graphics_screen_width, lbDisplay.graphics_screen_height);
                    lbDisplay.draw_flags = 0;
                } else if (allhosps[player].equipment[def->kind].available == 1 &&
                           objects[def->kind].available_for_level == 1) {
                    reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y,
                                screen_sprites_tab_start + 6);
                    if (listhead[player].rooms[def->category] == 0) {
                        no_room = 1;
                    }
                    if (def->staff_mask == 0x40) {
                        reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y,
                                    screen_sprites_tab_start + 10);
                    }
                    if (def->category == 10) {
                        reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y,
                                    screen_sprites_tab_start + 8);
                    }
                } else {
                    reqspr_draw(resvars.screen_coords[0x1f].x, resvars.screen_coords[0x1f].y,
                                screen_sprites_tab_start + 9);
                    unavailable = 1;
                }
            }
        }
    } else if (illness >= 0x24) {
        if (allhosps[player].rooms[illness].cure_available == 1) {
            reqspr_draw(resvars.screen_coords[0x20].x, resvars.screen_coords[0x20].y, screen_sprites_tab_start + 11);
        } else {
            reqspr_draw(resvars.screen_coords[0x20].x, resvars.screen_coords[0x20].y, screen_sprites_tab_start + 12);
        }
    }
    if (expertise[illness].research_type != 0) {
        if (illness == (statvar.num_players > 1 ? allhosps[player].hospital_open : gamevar.research_illness)) {
            set_font_from_cache(0xf);
        } else {
            set_font_from_cache(0xe);
        }
        conv_string(buf, cabinet_text[6], 1);
        LbTextDraw(resvars.screen_coords[0x1d].x, resvars.screen_coords[0x1d].y, buf);
    }
    process_drug_panel();
    process_drug_keys();
    lbDisplay.draw_flags = 0;
    if (exit_quick_requester()) {
        drugs.state = 1;
    }
    set_font_from_cache(0);
}

// Handles clicks on the casebook's buttons (resvars.screen_buttons from 0x12); holding
// the button repeats the price buttons. A click restarts the price change
// acceleration.
// FUNCTION: 0x00013540
void process_drug_panel(void)
{
    u8 i;

    if (lbDisplay.left_button) {
        drugs.value = 0;
        for (i = 0; resvars.screen_buttons[i + 0x12].width != 0 && resvars.screen_buttons[i + 0x12].height != 0; i++) {
            if (over_hotspot(lbDisplay.mouse_x, lbDisplay.mouse_y, resvars.screen_buttons[i + 0x12].x,
                             resvars.screen_buttons[i + 0x12].y, (s16)resvars.screen_buttons[i + 0x12].width,
                             (s16)resvars.screen_buttons[i + 0x12].height)) {
                resvars.screen_buttons[i + 0x12].callback(i);
                if (resvars.screen_buttons[i + 0x12].sprite) {
                    reqspr_draw(
                        resvars.screen_buttons[i + 0x12].sprite_x, resvars.screen_buttons[i + 0x12].sprite_y,
                        (TbSprite *)((u32)screen_sprites_tab_start + resvars.screen_buttons[i + 0x12].sprite * 6));
                }
                break;
            }
        }
        othervar.repeat_step = 0x100;
        othervar.repeat_time = othervar.clock_ms;
    }
    if (lbDisplay.mleft_button) {
        for (i = 0; resvars.screen_buttons[i + 0x12].width != 0 && resvars.screen_buttons[i + 0x12].height != 0; i++) {
            if (over_hotspot((s16)lbDisplay.mmouse_x, (s16)lbDisplay.mmouse_y, resvars.screen_buttons[i + 0x12].x,
                             resvars.screen_buttons[i + 0x12].y, (s16)resvars.screen_buttons[i + 0x12].width,
                             (s16)resvars.screen_buttons[i + 0x12].height)) {
                if (i == 0 || i == 1) {
                    resvars.screen_buttons[i + 0x12].callback(i);
                }
                if (resvars.screen_buttons[i + 0x12].sprite) {
                    reqspr_draw(
                        resvars.screen_buttons[i + 0x12].sprite_x, resvars.screen_buttons[i + 0x12].sprite_y,
                        (TbSprite *)((u32)screen_sprites_tab_start + resvars.screen_buttons[i + 0x12].sprite * 6));
                }
                break;
            }
        }
    }
    lbDisplay.left_button = 0;
}

// Queues raising the selected illness's price (event EV_PRICE_UP), with a step that
// grows while the button is held (othervar.repeat_step, 8.8 fixed point, up to 10),
// and plays the refusal sound once at the 200% limit.
// FUNCTION: 0x0001387c
void price_up(void)
{
    if (othervar.clock_ms - othervar.repeat_time > 0xfa || othervar.repeat_step == 0x100) {
        player_events[player].type = EV_PRICE_UP;
        player_events[player].value = othervar.repeat_step >> 8;
        player_events[player].arg = othervar.selected_illness;
        if (othervar.repeat_step < 0xa00) {
            othervar.repeat_step += 0x10;
        } else {
            othervar.repeat_step = 0xa00;
        }
        if (myhosp->rooms[othervar.selected_illness].price_scale < 20000) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        } else if (!drugs.value) {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            drugs.value = 1;
        }
    }
}

// Queues lowering the selected illness's price (event EV_PRICE_DOWN), as price_up; the
// limit is 50%.
// FUNCTION: 0x0001397c
void price_down(void)
{
    if (othervar.clock_ms - othervar.repeat_time > 0xfa || othervar.repeat_step == 0x100) {
        player_events[player].type = EV_PRICE_DOWN;
        player_events[player].value = othervar.repeat_step >> 8;
        player_events[player].arg = othervar.selected_illness;
        if (othervar.repeat_step < 0xa00) {
            othervar.repeat_step += 0x10;
        } else {
            othervar.repeat_step = 0xa00;
        }
        if (myhosp->rooms[othervar.selected_illness].price_scale > 5000) {
            play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        } else if (!drugs.value) {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
            drugs.value = 1;
        }
    }
}

// Casebook keys: up and down select the previous or next listed illness, left
// and right lower or raise its price.
// FUNCTION: 0x00013a7c
void process_drug_keys(void)
{
    u8 illness;

    if (lbKeyOn[KC_NUMPAD8] || lbKeyOn[KC_UP]) {
        illness = get_next_illness(0, othervar.selected_illness);
        if (illness != 0xfa) {
            othervar.selected_illness = illness;
            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
    }
    if (lbKeyOn[KC_NUMPAD2] || lbKeyOn[KC_DOWN]) {
        illness = get_next_illness(1, othervar.selected_illness);
        if (illness != 0xfa) {
            othervar.selected_illness = illness;
            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
    }
    if (lbKeyOn[KC_NUMPAD6] || lbKeyOn[KC_RIGHT]) {
        price_up();
    } else if (lbKeyOn[KC_NUMPAD4] || lbKeyOn[KC_LEFT]) {
        price_down();
    } else if (!lbDisplay.mleft_button) {
        othervar.repeat_step = 0x100;
        othervar.repeat_time = othervar.clock_ms;
    }
}

// Casebook button action: 0 price up, 1 price down, 2 previous illness, 3
// next illness, 4 close, 5 make the illness a research concentration
// (EV_CONCENTRATE_RESEARCH).
// FUNCTION: 0x00013bc4
void process_drug_button(u8 action)
{
    u8 illness;

    debug.last_action = action;
    switch (action) {
    case 0:
        price_up();
        othervar.drug_pct_display_time = 0x32;
        break;
    case 1:
        price_down();
        othervar.drug_pct_display_time = 0x32;
        break;
    case 2:
        illness = get_next_illness(0, othervar.selected_illness);
        if (illness != 0xfa) {
            othervar.selected_illness = illness;
            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        othervar.drug_pct_display_time = 0x32;
        break;
    case 3:
        illness = get_next_illness(1, othervar.selected_illness);
        if (illness != 0xfa) {
            othervar.selected_illness = illness;
            play_sound(SNDSRC_GENERAL, SFX_PAGETUR2, -1, -1);
        } else {
            play_sound(SNDSRC_GENERAL, SFX_WRONG2, -1, -1);
        }
        othervar.drug_pct_display_time = 0x32;
        break;
    case 4:
        drugs.state = 1;
        play_sound(SNDSRC_GENERAL, SFX_NO4, -1, -1);
        break;
    case 5:
        player_events[player].type = EV_CONCENTRATE_RESEARCH;
        player_events[player].value = othervar.selected_illness;
        play_sound(SNDSRC_GENERAL, SFX_SELECTX, -1, -1);
        break;
    }
}

// Returns the illness before (forward 0) or after from in the casebook: one
// the hospital has seen, a diagnosis it can do, or autopsy once available.
// Returns 0xfa if there is none.
// FUNCTION: 0x00013d8c
u8 get_next_illness(const u8 forward, const u8 from)
{
    u8 i;

    switch (forward) {
    case 0:
        if (from - 1 > 0) {
            for (i = from - 1; i > 0; i--) {
                if ((i < 0x24 && myhosp->rooms[i].discovered) ||
                    (i >= 0x24 && allhosps[player].rooms[i].cure_available == 1) ||
                    (i == 0x2c &&
                     (allhosps[player].equipment[55].available != 0 &&
                      objects[OBJ_AUTO_AUTOPSY].available_for_level != 0) &&
                     listhead[player].rooms[ROOM_RESEARCH])) {
                    return i;
                }
            }
        }
        break;
    default:
        if (from + 1 < 0x2e) {
            for (i = from + 1; i < 0x2e; i++) {
                if ((i < 0x24 && myhosp->rooms[i].discovered) ||
                    (i >= 0x24 && allhosps[player].rooms[i].cure_available == 1) ||
                    (i == 0x2c &&
                     (allhosps[player].equipment[55].available != 0 &&
                      objects[OBJ_AUTO_AUTOPSY].available_for_level != 0) &&
                     listhead[player].rooms[ROOM_RESEARCH])) {
                    return i;
                }
            }
        }
        break;
    }
    return 0xfa;
}

// Gives hospital's drug for illness (if illness is cured by drugs) the level's
// starting effectiveness and cost.
// FUNCTION: 0x00013fb0
void setup_drug(u16 hospital, u16 illness)
{
    if (expertise[illness].kind == 0x27) {
        allhosps[hospital].rooms[illness].drug_effectiveness = gbv.start_rating;
        allhosps[hospital].rooms[illness].drug_cost = gbv.start_cost;
    }
}

// Help text callback: the hovered casebook item's help, but only while
// othervar.concentrate_help_on is set, which nothing does (the casebook clears
// it each frame).
// FUNCTION: 0x0001403c
void drugs_help_concentrate(void)
{
    if (othervar.concentrate_help_on) {
        lib_strcpy(othervar.help_msg, help_text[resvars.help_hotspots[othervar.help_hotspot].text]);
    }
}

// Help text callback: how the selected illness is cured (drug and its
// effectiveness, psychiatry, surgery, or a machine).
// FUNCTION: 0x00014090
void drugs_help_cure_type(void)
{
    if (othervar.selected_illness >= 0x24 || othervar.selected_illness == 1) {
        return;
    }
    if (expertise[othervar.selected_illness].kind == 0x27) {
        if (allhosps[player].rooms[othervar.selected_illness].cure_available == 1) {
            sprintf(othervar.help_msg, help_text[148],
                    allhosps[player].rooms[othervar.selected_illness].drug_effectiveness);
        } else {
            lib_strcpy(othervar.help_msg, help_text[147]);
        }
    } else if (expertise[othervar.selected_illness].staff_mask == 0x40) {
        lib_strcpy(othervar.help_msg, help_text[149]);
    } else if (expertise[othervar.selected_illness].category == 10) {
        lib_strcpy(othervar.help_msg, help_text[150]);
    } else {
        lib_strcpy(othervar.help_msg, help_text[151]);
    }
}

// Help text callback: whether the hospital can treat the selected illness, or
// the first thing it lacks (see what_is_required).
// FUNCTION: 0x000141f0
void drugs_help_handled(void)
{
    if (allhosps[player].rooms[othervar.selected_illness].cure_available == 1) {
        lib_strcpy(othervar.help_msg, help_text[152]);
        return;
    }
    what_is_required(othervar.selected_illness, player);
    if (required.room_unavailable) {
        lib_strcpy(othervar.help_msg, help_text[153]);
    } else if (required.missing_room) {
        lib_strcpy(othervar.help_msg, help_text[154]);
    } else if (required.surgeons_needed == 2) {
        lib_strcpy(othervar.help_msg, help_text[155]);
    } else if (required.surgeons_needed == 1) {
        lib_strcpy(othervar.help_msg, help_text[156]);
    } else if (required.missing_skill) {
        sprintf(othervar.help_msg, help_text[157], skill_names[required.missing_skill]);
    } else if (required.needs_ward) {
        lib_strcpy(othervar.help_msg, help_text[158]);
    } else if (required.needs_nurse) {
        lib_strcpy(othervar.help_msg, help_text[159]);
    } else {
        lib_strcpy(othervar.help_msg, help_text[160]);
    }
}

// Updates every hospital's price for every illness (update_single_cure_cost).
// FUNCTION: 0x0001438c
void update_cure_costs(void)
{
    u32 hospital;
    u32 illness;

    for (hospital = 0; hospital < 4; hospital++) {
        for (illness = 0; illness < 0x2f; illness++) {
            update_single_cure_cost(illness, hospital);
        }
    }
}

// Sets hospital's price for illness to its percentage (rooms[].price_scale, in
// hundredths of a percent) of the maximum price, if it moved by at least 1%.
// FUNCTION: 0x000143e0
void update_single_cure_cost(u8 illness, u8 hospital)
{
    s32 max_price;
    s32 target;
    s32 diff;

    max_price = max_price_for_treatment(illness, hospital);
    target = max_price * allhosps[hospital].rooms[illness].price_scale / 10000;
    if (target - allhosps[hospital].rooms[illness].price < 0) {
        diff = -(target - allhosps[hospital].rooms[illness].price);
    } else {
        diff = target - allhosps[hospital].rooms[illness].price;
    }
    if (target / 100 <= diff) {
        allhosps[hospital].rooms[illness].price = target;
    }
}
