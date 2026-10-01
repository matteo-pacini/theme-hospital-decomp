#include "hosp.h"

extern "C" {
extern char **illness_text;
extern u16 choice;
extern u8 D_00158192;
void fn_000c93bf(char *dst, char *src);
}


// With a chance that grows with warmth (gbv.contagious_spread_factor + warmth / 2 percent), passes
// carrier's contagious illness to one undiagnosed, uncontagious patient on or
// next to carrier's tile (half the time each; not to those whose illness has
// expertise not_contagious set), splitting carrier's contagion with them. Note
// that effect_illness is applied to carrier, not the new victim.
// FUNCTION: 0x000a0040
void spread_epidemic(Thing *carrier, u16 warmth)
{
    Thing *victim;
    Map *adj;
    Map *cell;
    s16 i;
    s16 id;
    u8 illness;
    u8 other;

    othervar.rand_pct = old_newrand() % 100;
    if (othervar.rand_pct < 0 || othervar.rand_pct >= gbv.contagious_spread_factor + warmth / 2)
        return;
    illness = carrier->illness;
    cell = &map[(s16)carrier->x.tile + ((s16)carrier->y.tile << 7)];
    for (i = 0; i < 5; i++) {
        adj = &cell[around[i]];
        for (id = adj->first; id > 0; id = victim->cell_next) {
            victim = &thing[id];
            if (victim->type != THING_PATIENT || victim == carrier)
                continue;
            if (victim->illness == victim->known_illness)
                continue;
            other = victim->illness;
            if (expertise[other].not_contagious != 0 || (s8)victim->contagion != 0 || old_newrand() & 1)
                continue;
            othervar.contagious_count++;
            if (other != illness) {
                victim->illness = illness;
                effect_illness(carrier, illness, 0);
            }
            othervar.last_spread_illness = illness;
            victim->contagion = (s8)carrier->contagion / 2;
            carrier->contagion -= victim->contagion;
            if (carrier->flags & PF_CONTAGIOUS)
                victim->flags |= PF_CONTAGIOUS;
            return;
        }
    }
}

// Composes the epidemic fax for the local player from strs: the illness
// (myhosp->epidemic_illness), the number of contagious patients (f203), and the choices.
// FUNCTION: 0x000a0240
void declare_epidemic(char **strs)
{
    int line_height;
    char text[1000];
    int x;
    int y;
    char line[200];
    char new_line[] = "\n";

    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    text[0] = 0;
    x = 0;
    y = 0;
    sprintf(line, strs[0], illness_text[myhosp->epidemic_illness]);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[1], myhosp->epidemic_fine);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[2]);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[3]);
    fn_000c93bf(text, line);
    LbTextDraw(x, y, text);
}

// Epidemic fax button: queues the answer, 0 to declare it and pay the fine
// (10) or 1 to try to cover it up (11, noted for the advisor).
// FUNCTION: 0x000a03b0
void process_epidemic_start_buttons(u8 button)
{
    choice_spare_data = 0;
    switch (button) {
    case 0:
        packet_choice_result(CH_EPIDEMIC_DECLARE);
        break;
    case 1:
        packet_choice_result(CH_EPIDEMIC_COVER_UP);
        advisor.event_flag_bytes[1] |= 1;
        break;
    }
}

// Starts an epidemic of illness in owner's hospital, unless one or an
// emergency is already running (or a single player is viewing an emergency
// fax): withdraws any emergency call and raises the epidemic call.
// FUNCTION: 0x000a0414
void alert_epidemic(u8 illness, u8 owner)
{
    AllHosps *hosp;

    if ((statvar.num_players <= 1 && display_mode == DM_CHOICE) && choice == 2)
        return;
    hosp = &allhosps[owner];
    if (hosp->epidemic_illness != 0 || hosp->emergency_state_s != 0)
        return;
    get_playercall(2, 0, owner);
    if (owner == player)
        advisor.event_flag_bytes[0] |= 8;
    hosp->epidemic_illness = illness;
    make_a_playercall(3, 1, 0x14, owner);
}

// Goes over owner's patients inside the building who carry illness: mode 1
// flags them contagious (curing fed-up ones of it instead) and counts them,
// mode 2 cleans them all and counts those that were flagged. Patients outside
// lose their contagion. Returns the count (0 for other modes).
// FUNCTION: 0x000a04e4
u32 count_and_flag_contagious_peeps(const u8 illness, const u8 mode, const u8 owner)
{
    u32 count;
    u32 max;
    Thing *peep;

    count = 0;
    max = 0;
    for (peep = thing + 1; peep < end_things; peep++) {
        if (peep->type != THING_PATIENT || peep->owner != owner)
            continue;
        if (!peep->contagion || peep->illness != illness)
            continue;
        if (!(map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].owner_flags & MAP_OUTDOOR)) {
            if ((s8)peep->contagion > max)
                max = (s8)peep->contagion;
            if (mode == 1) {
                if (peep->flags & PF_FED_UP) {
                    peep->flags &= ~PF_CONTAGIOUS;
                    peep->contagion = 0;
                    continue;
                }
                count++;
                set_icon_value(peep, ICON_CONTAGIOUS);
                peep->flags |= PF_CONTAGIOUS;
            } else if (mode == 2) {
                if (peep->flags & PF_CONTAGIOUS)
                    count++;
                clean_contagious_peep(peep);
            }
        } else {
            peep->flags &= ~PF_CONTAGIOUS;
            peep->contagion = 0;
        }
    }
    return count;
}

// Counts down owner's epidemic cover-up (allhosps.epidemic_active): it ends when no
// contagious patient is left or time (f207) runs out. On ending, clears the
// cover-up and sets f20e. Returns 1 if it ended this call.
// FUNCTION: 0x000a0654
u32 epidemic_status(u8 owner)
{
    u32 over;
    AllHosps *hosp;

    over = 0;
    hosp = &allhosps[owner];
    if (hosp->epidemic_active) {
        if (hosp->contagious_count) {
            hosp->epidemic_timer--;
            if ((s16)hosp->epidemic_timer <= 0)
                over = 1;
        } else {
            if (owner == player)
                play_sound(SNDSRC_GENERAL, SFX_EPIDEMIC_END, -1, -1);
            over = 1;
        }
    }
    if (over) {
        if (owner == player)
            advisor.forced_advice_id = 9;
        hosp->epidemic_timer = 0;
        hosp->epidemic_active = 0;
        hosp->epidemic_over = 1;
    }
    return over;
}

// Composes the epidemic result fax for owner from strs, by its outcome
// (allhosps.inspection[0]: 0 to 3) and the illness in choice_spare_data.
// FUNCTION: 0x000a073c
void epidemic_over(const u8 owner, char **strs)
{
    int illness;
    AllHosps *hosp;
    int line_height;
    char new_line[] = "\n";
    char buf1[200];
    char buf2[200];
    char buf3[200];
    char text[600];
    u16 x;
    u16 y;

    line_height = lbFontPtr ? lbFontPtr[34].height : 0;
    hosp = &allhosps[owner];
    illness = choice_spare_data;
    switch (hosp->inspection[0]) {
    case 0:
        sprintf(buf1, strs[2], illness_text[illness]);
        sprintf(buf2, strs[3]);
        sprintf(buf3, strs[4], hosp->epidemic_fine);
        break;
    case 1:
        sprintf(buf1, strs[0], illness_text[illness]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[5], hosp->epidemic_fine);
        break;
    case 2:
        sprintf(buf1, strs[0], illness_text[illness]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[6], hosp->epidemic_fine);
        break;
    case 3:
        sprintf(buf1, strs[0], illness_text[illness]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[7]);
        break;
    }
    text[0] = 0;
    x = 0;
    y = 0;
    fn_000c93bf(text, buf1);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf2);
    fn_000c93bf(text, new_line);
    fn_000c93bf(text, buf3);
    LbTextDraw(x, y, text);
}

// Epidemic result fax button 0 closes the fax.
// FUNCTION: 0x000a09f4
void process_epidemic_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// Sends owner's patients inside the building home (PF_SENT_HOME_BY_INSPECTOR),
// all of them or (all zero) only the contagious ones, clearing their
// contagion.
// FUNCTION: 0x000a0a30
void send_peeps_home(u8 all, u8 owner)
{
    Thing *peep;

    for (peep = thing + 1; peep < end_things; peep++) {
        if (peep->type != THING_PATIENT || peep->owner != owner)
            continue;
        if (map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].owner_flags & MAP_OUTDOOR)
            continue;
        if (all || (peep->flags & PF_CONTAGIOUS)) {
            peep->flags &= ~PF_CONTAGIOUS;
            peep->contagion = 0;
            peep->flags |= PF_SENT_HOME_BY_INSPECTOR;
        }
    }
}

// Cures peep of contagion: clears the flag, vaccination icons and queue place,
// and the count of spread infections.
// FUNCTION: 0x000a0ae8
void clean_contagious_peep(Thing *peep)
{
    peep->flags &= ~PF_CONTAGIOUS;
    peep->contagion = 0;
    reset_icon_value(peep, ICON_VACCINE_REQUESTED);
    reset_icon_value(peep, ICON_VACCINE_NURSE_SENT);
    reset_icon_value(peep, ICON_VACCINATED);
    reset_icon_value(peep, ICON_CONTAGIOUS);
    remove_person_from_vaccination_queue(peep);
    if (othervar.contagious_count >= 1)
        othervar.contagious_count--;
}
