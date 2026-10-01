#include "hosp.h"

extern "C" {
extern char **D_001602a4;
extern u16 D_00158190;
extern u8 D_00158192;
void fn_000c93bf(char *dst, char *src);
}


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

    othervar.f0 = old_newrand() % 100;
    if (othervar.f0 < 0 || othervar.f0 >= gbv.f145 + warmth / 2)
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
            if (expertise[other].unknown10[1] != 0 || (s8)victim->contagion != 0 || old_newrand() & 1)
                continue;
            othervar.f87++;
            if (other != illness) {
                victim->illness = illness;
                effect_illness(carrier, illness, 0);
            }
            othervar.f39 = illness;
            victim->contagion = (s8)carrier->contagion / 2;
            carrier->contagion -= victim->contagion;
            if (carrier->flags & PF_CONTAGIOUS)
                victim->flags |= PF_CONTAGIOUS;
            return;
        }
    }
}

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
    sprintf(line, strs[0], D_001602a4[myhosp->f20b]);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[1], myhosp->f203);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[2]);
    fn_000c93bf(text, line);
    fn_000c93bf(text, new_line);
    sprintf(line, strs[3]);
    fn_000c93bf(text, line);
    LbTextDraw(x, y, text);
}

// FUNCTION: 0x000a03b0
void process_epidemic_start_buttons(u8 button)
{
    choice_spare_data = 0;
    switch (button) {
    case 0:
        packet_choice_result(10);
        break;
    case 1:
        packet_choice_result(11);
        advisor.f128_u8a[1] |= 1;
        break;
    }
}

// FUNCTION: 0x000a0414
void alert_epidemic(u8 illness, u8 owner)
{
    AllHosps *hosp;

    if ((statvar.unknownfd <= 1 && display_mode == 9) && D_00158190 == 2)
        return;
    hosp = &allhosps[owner];
    if (hosp->f20b != 0 || hosp->f1e2s != 0)
        return;
    get_playercall(2, 0, owner);
    if (owner == player)
        advisor.f128_u8a[0] |= 8;
    hosp->f20b = illness;
    make_a_playercall(3, 1, 0x14, owner);
}

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
        if (!(map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].unknown7 & 0x10)) {
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

// FUNCTION: 0x000a0654
u32 epidemic_status(u8 owner)
{
    u32 over;
    AllHosps *hosp;

    over = 0;
    hosp = &allhosps[owner];
    if (hosp->f20d) {
        if (hosp->f205) {
            hosp->f207--;
            if ((s16)hosp->f207 <= 0)
                over = 1;
        } else {
            if (owner == player)
                play_sound(10, 0xce, -1, -1);
            over = 1;
        }
    }
    if (over) {
        if (owner == player)
            advisor.f2 = 9;
        hosp->f207 = 0;
        hosp->f20d = 0;
        hosp->f20e = 1;
    }
    return over;
}

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
    switch (hosp->unknown209[0]) {
    case 0:
        sprintf(buf1, strs[2], D_001602a4[illness]);
        sprintf(buf2, strs[3]);
        sprintf(buf3, strs[4], hosp->f203);
        break;
    case 1:
        sprintf(buf1, strs[0], D_001602a4[illness]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[5], hosp->f203);
        break;
    case 2:
        sprintf(buf1, strs[0], D_001602a4[illness]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[6], hosp->f203);
        break;
    case 3:
        sprintf(buf1, strs[0], D_001602a4[illness]);
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

// FUNCTION: 0x000a09f4
void process_epidemic_end_buttons(u8 button)
{
    switch (button) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000a0a30
void send_peeps_home(u8 all, u8 owner)
{
    Thing *peep;

    for (peep = thing + 1; peep < end_things; peep++) {
        if (peep->type != THING_PATIENT || peep->owner != owner)
            continue;
        if (map[(s16)peep->x.tile + ((s16)peep->y.tile << 7)].unknown7 & 0x10)
            continue;
        if (all || (peep->flags & PF_CONTAGIOUS)) {
            peep->flags &= ~PF_CONTAGIOUS;
            peep->contagion = 0;
            peep->flags |= PF_SENT_HOME_BY_INSPECTOR;
        }
    }
}

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
    if (othervar.f87 >= 1)
        othervar.f87--;
}
