#include "hosp.h"

extern "C" {
extern s16 D_001553b8;
extern u8 D_0012ddc5;
extern u16 D_0015543f;
extern u16 D_001553f1;
extern char **D_001602a4;
extern char D_0014532c[];
extern u8 D_00158324[];
extern u16 D_00158190;
extern u8 D_00158192;
extern u16 D_001581fe;
void fn_000c93bf(char *dst, char *src);
}

extern char D_0014532e[];
#pragma aux D_0014532e "W?$Wsi0dnew_line$n[]a$:.0$:?epidemic_over$n(xucpnpna)vnx[]a"

// NONMATCHING: register choice (operand load order of the threshold, t/unknown174 store)
// FUNCTION: 0x000a0040
void spread_epidemic(Thing *w, u16 n)
{
    Thing *t;
    Map *adj;
    Map *cell;
    s16 i;
    s16 id;
    u8 illness;
    u8 other;

    D_001553b8 = old_newrand() % 100;
    if (D_001553b8 < 0 || D_001553b8 >= D_0012ddc5 + n / 2)
        return;
    illness = w->style;
    cell = &map[(s16)w->x.tile + ((s16)w->y.tile << 7)];
    for (i = 0; i < 5; i++) {
        adj = &cell[around[i]];
        for (id = adj->first; id > 0; id = t->next) {
            t = &thing[id];
            if (t->renderer != 2 || t == w)
                continue;
            if (t->style == t->unknown169)
                continue;
            other = t->style;
            if (expertise[other].unknown10[1] != 0 || (s8)t->unknown174 != 0 || old_newrand() & 1)
                continue;
            D_0015543f++;
            if (other != illness) {
                t->style = illness;
                effect_illness(w, illness, 0);
            }
            D_001553f1 = illness;
            t->unknown174 = (s8)w->unknown174 / 2;
            w->unknown174 -= t->unknown174;
            if (w->flags & 0x20)
                t->flags |= 0x20;
            return;
        }
    }
}

// NONMATCHING: frame layout (the target places a and b between buf2 and buf1)
// FUNCTION: 0x000a0240
void declare_epidemic(char **strs)
{
    u16 sep;
    int v;
    char buf2[1000];
    int a;
    int b;
    char buf1[200];

    sep = *(u16 *)D_0014532c;
    v = lbFontPtr ? lbFontPtr[34].height : 0;
    buf2[0] = 0;
    a = 0;
    b = 0;
    sprintf(buf1, strs[0], D_001602a4[myhosp->f20b]);
    fn_000c93bf(buf2, buf1);
    fn_000c93bf(buf2, (char *)&sep);
    sprintf(buf1, strs[1], myhosp->f203);
    fn_000c93bf(buf2, buf1);
    fn_000c93bf(buf2, (char *)&sep);
    sprintf(buf1, strs[2]);
    fn_000c93bf(buf2, buf1);
    fn_000c93bf(buf2, (char *)&sep);
    sprintf(buf1, strs[3]);
    fn_000c93bf(buf2, buf1);
    LbTextDraw(a, b, buf2);
}

// FUNCTION: 0x000a03b0
void process_epidemic_start_buttons(u8 n)
{
    choice_spare_data = 0;
    switch (n) {
    case 0:
        packet_choice_result(10);
        break;
    case 1:
        packet_choice_result(11);
        D_00158324[1] |= 1;
        break;
    }
}

// FUNCTION: 0x000a0414
void alert_epidemic(u8 illness, u8 owner)
{
    AllHosps *h;

    if ((statvar.unknownfd <= 1 && display_mode == 9) && D_00158190 == 2)
        return;
    h = &allhosps[owner];
    if (h->f20b != 0 || h->f1e2s != 0)
        return;
    get_playercall(2, 0, owner);
    if (owner == player)
        D_00158324[0] |= 8;
    h->f20b = illness;
    make_a_playercall(3, 1, 0x14, owner);
}

// FUNCTION: 0x000a04e4
u32 count_and_flag_contagious_peeps(const u8 illness, const u8 mode, const u8 owner)
{
    u32 count;
    u32 max;
    Thing *t;

    count = 0;
    max = 0;
    for (t = thing + 1; t < end_things; t++) {
        if (t->renderer != 2 || t->owner != owner)
            continue;
        if (!t->unknown174 || t->style != illness)
            continue;
        if (!(map[(s16)t->x.tile + ((s16)t->y.tile << 7)].unknown7 & 0x10)) {
            if ((s8)t->unknown174 > max)
                max = (s8)t->unknown174;
            if (mode == 1) {
                if (t->flags & 0x4000) {
                    t->flags &= ~0x20;
                    t->unknown174 = 0;
                    continue;
                }
                count++;
                set_icon_value(t, 0xd);
                t->flags |= 0x20;
            } else if (mode == 2) {
                if (t->flags & 0x20)
                    count++;
                clean_contagious_peep(t);
            }
        } else {
            t->flags &= ~0x20;
            t->unknown174 = 0;
        }
    }
    return count;
}

// FUNCTION: 0x000a0654
u32 epidemic_status(u8 owner)
{
    u32 over;
    AllHosps *h;

    over = 0;
    h = &allhosps[owner];
    if (h->f20d) {
        if (h->f205) {
            h->f207--;
            if ((s16)h->f207 <= 0)
                over = 1;
        } else {
            if (owner == player)
                play_sound(10, 0xce, -1, -1);
            over = 1;
        }
    }
    if (over) {
        if (owner == player)
            D_001581fe = 9;
        h->f207 = 0;
        h->f20d = 0;
        h->f20e = 1;
    }
    return over;
}

// FUNCTION: 0x000a073c
void epidemic_over(const u8 owner, char **strs)
{
    int kind;
    AllHosps *h;
    int v;
    u16 sep;
    char buf1[200];
    char buf2[200];
    char buf3[200];
    char text[600];
    u16 a;
    u16 b;

    sep = *(u16 *)D_0014532e;
    v = lbFontPtr ? lbFontPtr[34].height : 0;
    h = &allhosps[owner];
    kind = choice_spare_data;
    switch (h->unknown209[0]) {
    case 0:
        sprintf(buf1, strs[2], D_001602a4[kind]);
        sprintf(buf2, strs[3]);
        sprintf(buf3, strs[4], h->f203);
        break;
    case 1:
        sprintf(buf1, strs[0], D_001602a4[kind]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[5], h->f203);
        break;
    case 2:
        sprintf(buf1, strs[0], D_001602a4[kind]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[6], h->f203);
        break;
    case 3:
        sprintf(buf1, strs[0], D_001602a4[kind]);
        sprintf(buf2, strs[1]);
        sprintf(buf3, strs[7]);
        break;
    }
    text[0] = 0;
    a = 0;
    b = 0;
    fn_000c93bf(text, buf1);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf2);
    fn_000c93bf(text, (char *)&sep);
    fn_000c93bf(text, buf3);
    LbTextDraw(a, b, text);
}

// FUNCTION: 0x000a09f4
void process_epidemic_end_buttons(u8 n)
{
    switch (n) {
    case 0:
        D_00158192 = 0x5c;
        break;
    }
}

// FUNCTION: 0x000a0a30
void send_peeps_home(u8 all, u8 owner)
{
    Thing *t;

    for (t = thing + 1; t < end_things; t++) {
        if (t->renderer != 2 || t->owner != owner)
            continue;
        if (map[(s16)t->x.tile + ((s16)t->y.tile << 7)].unknown7 & 0x10)
            continue;
        if (all || (t->flags & 0x20)) {
            t->flags &= ~0x20;
            t->unknown174 = 0;
            t->flags |= 0x100;
        }
    }
}

// FUNCTION: 0x000a0ae8
void clean_contagious_peep(Thing *w)
{
    w->flags &= ~0x20;
    w->unknown174 = 0;
    reset_icon_value(w, 0xe);
    reset_icon_value(w, 0xf);
    reset_icon_value(w, 0x10);
    reset_icon_value(w, 0xd);
    remove_person_from_vaccination_queue(w);
    if (D_0015543f >= 1)
        D_0015543f--;
}
