#include "hosp.h"

extern "C" {
extern HelpHotspot *D_0015f420;
extern u16 D_0015536e;
extern u16 D_00155d75;
extern u8 D_00155d77;
extern u8 D_00155c5e;
extern u8 D_00155c5f;
extern u16 D_00155c64;
extern s16 D_00155c4e;
extern s16 D_00155c50;
extern s16 D_00155c52;
extern s16 D_00155c54;
extern s16 D_00155c60;
extern s16 D_00155c62;
extern u32 D_00155c56;
extern u32 D_00155c5a;
extern u32 D_0015544a;
extern char D_0015544e[];
extern u16 D_00188c74;
extern u8 D_00188c85;
extern u8 D_0012d808[];
extern u8 D_0015831b;
extern u8 D_00156910;
extern u8 D_00155e1d;
extern u8 D_00161134;
extern u8 D_00155e4a;
extern u8 D_00155e4b;
extern char **D_0016037c;
extern char **D_00160374;
extern char **D_00160398;
extern char **D_0016039c;
extern u16 D_0015632f[];
extern u16 D_0015636d[];
extern u16 D_001563ab[];
extern u16 D_001563bd[];
extern u16 D_00155d89;
extern u8 D_001547e5;
extern Event D_001549d0[];
extern char D_00120c50[];
extern char D_00120c53[];
extern char D_00120c55[];
extern char D_00120c58[];
extern char D_00120c5b[];
extern char D_00120c5d[];

int strcmp(const char *a, const char *b);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
void LbMouseSetWindow(s32 x, s32 y, s32 w, s32 h);
}

void set_help_text(u8 mode, s16 x, s16 y, char **strs);
void copy_and_format(char *dst, char *src);
void copy_and_format_composite(char *dst, char **strs);
void help_hotspot_detected(u16 i, u8 c, s16 x, s16 y);
void set_help_from_index(u16 i, s16 x, s16 y);

// FUNCTION: 0x00099c70
void setup_help_indices(void)
{
    u8 n;
    u8 done;
    u16 i;

    i = 0;
    n = 0;
    done = 0;
    while (!done) {
        while (D_0015f420[i].x0 >= 0)
            i++;
        if (D_0015f420[i].x0 == -2)
            done = 1;
        ++i;
        ++n;
        statvar.help_start[n] = i;
        if (i == 0x1b) {}
    }
}

// FUNCTION: 0x00099d00
void compute_help_text(s16 x, s16 y, u8 c)
{
    s16 dx;
    s16 dy;
    u16 i;

    dx = mouse_dx - x;
    dy = mouse_dy - y;
    if (D_0015536e == 0x25)
        return;
    if (D_00155d75) {
        if (c == D_00155d77)
            set_help_from_index(D_00155d75, x, y);
        return;
    }
    for (i = statvar.help_start[c]; D_0015f420[i].x0 >= 0; i++) {
        if (dx >= D_0015f420[i].x0 && dx <= D_0015f420[i].x1 && dy >= D_0015f420[i].y0
            && dy <= D_0015f420[i].y1) {
            help_hotspot_detected(i, c, x, y);
            return;
        }
    }
    if (c == D_00155c5e)
        D_00155c64 = 0;
}

// FUNCTION: 0x00099e4c
void set_help_text(u8 mode, s16 x, s16 y, char **strs)
{
    s32 border;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 mx;
    s32 my;

    D_00155c5f = mode;
    set_font_from_cache(display_mode == 2 ? 0x2d : 0);
    D_00188c74 = 0;
    switch (mode) {
    case 1:
        copy_and_format(D_0015544e, strs[0]);
        border = 3;
        D_00155c52 = LbTextStringWidth(D_0015544e) + border * 2;
        D_00155c54 = LbTextStringHeight(D_0015544e) + border * 2;
        break;
    case 2:
        copy_and_format_composite(D_0015544e, strs);
        border = pulldv[12].height + 0xc;
        D_00155c52 = LbTextStringWidth(D_0015544e) + border * 2;
        D_00155c54 = LbTextStringHeight(D_0015544e) + border * 2 + pulldv[19].height;
        break;
    }
    if (x == -1 && y == -1) {
        D_00155c4e = (lbDisplay.unknown18 - D_00155c52) / 2;
        D_00155c50 = (lbDisplay.unknown1c - D_00155c54) / 2;
        LbMouseSetWindow(D_00155c4e, D_00155c50, D_00155c52, D_00155c54);
        x0 = D_00155c4e;
        y0 = D_00155c50;
        x1 = x0 + D_00155c52;
        y1 = y0 + D_00155c54;
        if (!((mouse_dx >= x0 && mouse_dx < x1) && mouse_dy >= y0 && mouse_dy < y1)) {
            mx = mouse_dx;
            my = mouse_dy;
            mx < x0 ? mx = x0 : 0;
            mx > x1 ? mx = x1 : 0;
            my < y0 ? my = y0 : 0;
            my > y1 ? my = y1 : 0;
            LbMouseSetPosition(mx, my);
        }
    } else {
        D_00155c4e = x - border;
        D_00155c50 = y - border;
        D_00155c50 -= D_00155c54;
        D_00155c50 += 6;
    }
    D_00155c4e < 0 ? D_00155c4e = 0 : 0;
    D_00155c50 < 0 ? D_00155c50 = 0 : 0;
    D_00155c4e > lbDisplay.unknown18 - D_00155c52 ? D_00155c4e = lbDisplay.unknown18 - D_00155c52 : 0;
    D_00155c50 > lbDisplay.unknown18 - D_00155c54 ? D_00155c50 = lbDisplay.unknown18 - D_00155c54 : 0;
}

// FUNCTION: 0x0009a1f8
void draw_help_text(void)
{
    s32 i;
    s32 j;
    s32 step;
    s32 right;
    s32 bottom;
    s32 border;

    if (!(D_0015544e[0] && (D_0012d808[D_0015536e] || (D_0015831b & 1))))
        return;
    set_all_refresh_zones_in_window(D_00155c4e, D_00155c50, D_00155c4e + D_00155c52 + 0xc,
        D_00155c50 + D_00155c54 + 0xc, 3);
    if (D_00155c5e == 3)
        D_00156910 = 1;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown18);
    D_00188c74 = 0;
    LbDrawBox(D_00155c4e, D_00155c50, D_00155c52, D_00155c54, D_00155e1d ? color.unknown7 : 0xab);
    switch (D_00155c5f) {
    case 1:
        D_00188c74 = 0x10;
        LbDrawBox(D_00155c4e, D_00155c50, D_00155c52, D_00155c54, color.unknown8);
        border = 3;
        break;
    case 2:
        step = pulldv[12].width;
        j = D_00155c50;
        bottom = D_00155c50 + D_00155c54 - pulldv[16].height;
        for (i = D_00155c4e; D_00155c4e + D_00155c52 - step > i; i += step) {
            reqspr_draw(i, j, &pulldv[12]);
            reqspr_draw(i, bottom, &pulldv[16]);
        }
        step = pulldv[18].height;
        i = D_00155c4e;
        right = D_00155c4e + D_00155c52 - pulldv[14].width;
        for (j = D_00155c50; D_00155c50 + D_00155c54 - step > j; j += step) {
            reqspr_draw(i, j, &pulldv[18]);
            reqspr_draw(right, j, &pulldv[14]);
        }
        reqspr_draw(D_00155c4e, D_00155c50, &pulldv[11]);
        reqspr_draw(D_00155c4e + D_00155c52 - pulldv[13].width, D_00155c50, &pulldv[13]);
        reqspr_draw(D_00155c4e, D_00155c50 + D_00155c54 - pulldv[17].height, &pulldv[17]);
        reqspr_draw(D_00155c4e + D_00155c52 - pulldv[15].width, D_00155c50 + D_00155c54 - pulldv[15].height,
            &pulldv[15]);
        border = pulldv[12].height + 0xc;
        D_00155c60 = D_00155c4e + D_00155c52 - 0xc - pulldv[19].width;
        D_00155c62 = D_00155c50 + D_00155c54 - 0xc - pulldv[19].height;
        if ((D_00155c60 <= mouse_dx && D_00155c60 + pulldv[19].width > mouse_dx)
            && D_00155c62 <= mouse_dy && D_00155c62 + pulldv[19].height > mouse_dy) {
            reqspr_draw(D_00155c60, D_00155c62, &pulldv[20]);
            if (!D_00161134)
                play_sound(10, 0x44, -1, -1);
            D_00161134 = 1;
        } else {
            reqspr_draw(D_00155c60, D_00155c62, &pulldv[19]);
            D_00161134 = 0;
        }
        D_00188c74 = 4;
        LbDrawBox(D_00155c4e + D_00155c52, D_00155c50 + 0xc, 0xc, D_00155c54, color.unknown8);
        LbDrawBox(D_00155c4e + 0xc, D_00155c50 + D_00155c54, D_00155c52 - 0xc, 0xc, color.unknown8);
        D_00188c74 = 0;
        break;
    }
    D_00188c74 = 0x40;
    set_font_from_cache(display_mode == 2 ? 0x2d : 0);
    D_00188c85 = color.unknown8;
    LbTextDraw(D_00155c4e + border, D_00155c50 + border, D_0015544e);
    D_00188c74 = 0;
    if (D_0015536e != 0x25)
        D_0015544e[0] = 0;
}

// FUNCTION: 0x0009a7c8
void copy_and_format(char *dst, char *src)
{
    s32 len;
    s32 k;
    s32 width;
    s32 col;

    len = fn_000cd716(src);
    col = 0;
    width = 0xc;
    for (k = 1; width * k < len; width += 4, k++)
        ;
    while (*src) {
        if (*src == ' ' && col >= width) {
            *dst++ = '\n';
            src++;
            col = 0;
        } else {
            *dst++ = *src++;
            col++;
        }
    }
    *dst = 0;
}

// FUNCTION: 0x0009a86c
void copy_and_format_composite(char *dst, char **strs)
{
    s32 most;
    s32 count;
    s32 k;
    s32 limit;
    s32 cur;
    char **pp;
    char *p;
    char *q;
    char *start;
    char last;

    most = 0;
    count = 0;
    for (pp = strs; **pp != '.'; pp++) {
        for (p = *pp; *p; p++) {
            if (strcmp(p, D_00120c50) == 0) {
                most < count ? most = count : 0;
                count = 0;
                p++;
            } else {
                count++;
            }
        }
    }
    most < count ? most = count : 0;
    limit = 0xc;
    for (k = 1; limit * k < most; limit += 4, k++)
        ;
    set_font_from_cache(display_mode == 2 ? 0x2d : 0);
    limit = limit * (lbFontPtr ? lbFontPtr[46].width : 0);
    limit > lbDisplay.unknown18 * 3 / 4 ? limit = lbDisplay.unknown18 * 3 / 4 : 0;
    start = dst;
    cur = 0;
    for (; strcmp(*strs, D_00120c53) != 0; strs++) {
        for (p = *strs; *p; p++) {
            if (cur > limit) {
                q = dst;
                while (--q > start) {
                    if (*q == '\n')
                        break;
                    if (*q == ' ') {
                        *q = '\n';
                        *dst = 0;
                        q++;
                        cur = LbTextStringWidth(q);
                        break;
                    }
                }
            }
            if (strcmp(p, D_00120c55) == 0) {
                *dst++ = '\n';
                *dst++ = '\n';
                p++;
                cur = 0;
            } else {
                *dst++ = *p;
                cur += lbFontPtr ? lbFontPtr[*p - 31].width : 0;
            }
        }
        last = p[-1];
        if (last == '.' || last == '!' || last == '?' || last == ',') {
            *dst++ = ' ';
            cur += lbFontPtr ? lbFontPtr[1].width : 0;
        }
    }
    *dst = 0;
}

// FUNCTION: 0x0009ab54
void cancel_helpmsg(void)
{
    D_00155c64 = 0;
    D_00155c56 = 0;
    D_0015544e[0] = 0;
}

// FUNCTION: 0x0009ab84
void help_hotspot_detected(u16 i, u8 c, s16 x, s16 y)
{
    s32 d;

    if (statvar.unknownfd > 1) {
        d = i - statvar.help_start[c];
        if (c == 0x18) {
            if (d < 4 && network_num_players() <= d)
                return;
        }
    }
    if (i == D_00155c64) {
        if (D_0015544a + 0x2ee < D_00155c5a)
            D_00155c56 = D_00155c5a;
    } else {
        D_00155c5e = c;
        D_00155c64 = i;
        D_0015544a = D_00155c5a;
    }
    if (D_00155c56 + 0x1f4 > D_00155c5a) {
        D_00155c56 = D_00155c5a;
        D_00155c5e = c;
        set_help_from_index(i, x, y);
    }
}

// NONMATCHING: the first D_0015f420[i] address builds i * 16 in edx; here in eax, then moved
// FUNCTION: 0x0009ac70
void set_help_from_index(u16 i, s16 x, s16 y)
{
    s16 cy;
    s16 cx;
    char *text;

    cx = (D_0015f420[i].x0 + D_0015f420[i].x1) / 2 + x;
    cy = y + D_0015f420[i].y0;
    if (D_0015f420[i].func) {
        text = D_0015544e;
        D_0015f420[i].func();
        set_help_text(1, cx, cy, &text);
    } else {
        set_help_text(1, cx, cy, &help_text[D_0015f420[i].text]);
    }
}

// FUNCTION: 0x0009ad68
void set_force_help(u8 c, u16 id)
{
    u16 i;
    u8 found;

    i = 0;
    found = 0;
    D_00155d77 = c;
    for (i = 0; D_0015f420[i].x0 != -2; i++) {
        if (id == D_0015f420[i].text) {
            D_00155d75 = i;
            break;
        }
    }
}

// FUNCTION: 0x0009ade8
void clear_force_help(void)
{
    D_00155d75 = 0;
}

// FUNCTION: 0x0009ae08
void show_help_page(u32 page, u32 sub)
{
    char *saved;

    set_mouse_mode(0x25);
    switch (page) {
    case 0:
        set_help_text(2, -1, -1, &D_0016037c[D_0015632f[sub]]);
        break;
    case 1:
        set_help_text(2, -1, -1, &D_00160374[D_0015636d[sub]]);
        break;
    case 2: {
        char buf[500];

        switch (current_net_criteria.unknown0) {
        case 0:
            set_mouse_mode(0);
            return;
        case 1:
        case 9:
        case 2:
        case 8:
        case 3:
            sprintf(buf, D_00160398[D_001563ab[sub]], current_net_criteria.start - D_00155d89);
            saved = D_00160398[D_001563ab[sub]];
            D_00160398[D_001563ab[sub]] = buf;
            break;
        case 4:
        case 6:
        case 5:
        case 7:
            if (current_net_criteria.unknown0 == 7 && D_001547e5 == 2)
                sprintf(buf, D_00160398[D_001563ab[sub]], current_net_criteria.start - D_00155d89,
                    current_net_criteria.value);
            else
                sprintf(buf, D_00160398[D_001563ab[sub]], current_net_criteria.value,
                    current_net_criteria.start - D_00155d89);
            saved = D_00160398[D_001563ab[sub]];
            D_00160398[D_001563ab[sub]] = buf;
            break;
        }
        set_help_text(2, -1, -1, &D_00160398[D_001563ab[sub]]);
        D_00160398[D_001563ab[sub]] = saved;
        break;
    }
    case 3:
        set_help_text(2, -1, -1, &D_0016039c[D_001563bd[sub]]);
        break;
    }
    D_00155e4a = page;
    D_00155e4b = sub;
}

// FUNCTION: 0x0009b144
void setup_indices_for_composite_help(char **strs, u16 *idx, u32 n)
{
    u32 count;
    u32 i;

    count = 0;
    i = 0;
    if (strs) {}
    for (; strcmp(*strs, D_00120c58) != 0 && count < n; count++) {
        *idx++ = i;
        for (; strcmp(*strs, D_00120c5b) != 0; i++)
            strs++;
        strs++;
        i++;
    }
    if (count == n && strcmp(*strs, D_00120c5d) != 0) {}
}

// FUNCTION: 0x0009b1f0
void finish_composite_help(void)
{
    LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    D_00161134 = 0;
    D_00156910 = 1;
    D_0015544e[0] = 0;
    if (D_0015536e == 0x25) {
        set_mouse_mode(0);
        play_sound(10, 0x41, -1, -1);
        switch (D_00155e4a) {
        case 1:
            switch (D_00155e4b) {
            case 0xf:
            case 0x10:
                show_help_page(1, D_00155e4b + 1);
                break;
            }
            break;
        case 3:
            if (D_00155e4b == 2) {
                D_001549d0[player].type = 0x7e;
                D_001549d0[player].value = 1;
            }
            break;
        }
    }
}
