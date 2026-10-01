#include "hosp.h"

extern "C" {
extern u8 D_00156910;
extern u8 D_00161134;
extern char **D_0016037c;
extern char **D_00160374;
extern char **D_00160398;
extern char **D_0016039c;
extern Event D_001549d0[];

int strcmp(const char *a, const char *b);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "fn_000cd716_"
void LbMouseSetWindow(s32 x, s32 y, s32 w, s32 h);
}

void set_help_text(u8 mode, s16 x, s16 y, char **strs);
void copy_and_format(char *dst, char *src);
void copy_and_format_composite(char *dst, char **strs);
void help_hotspot_detected(u16 hotspot, u8 help_group, s16 x, s16 y);
void set_help_from_index(u16 hotspot, s16 x, s16 y);

// FUNCTION: 0x00099c70
void setup_help_indices(void)
{
    u8 help_group;
    u8 done;
    u16 i;

    i = 0;
    help_group = 0;
    done = 0;
    while (!done) {
        while (resvars.f2c_HelpHotspotp[i].x0 >= 0)
            i++;
        if (resvars.f2c_HelpHotspotp[i].x0 == -2)
            done = 1;
        ++i;
        ++help_group;
        statvar.help_start[help_group] = i;
        if (i == 0x1b) {}
    }
}

// FUNCTION: 0x00099d00
void compute_help_text(s16 x, s16 y, u8 help_group)
{
    s16 dx;
    s16 dy;
    u16 i;

    dx = lbDisplay.mmouse_x - x;
    dy = lbDisplay.mmouse_y - y;
    if (gamevar.f12e == 0x25)
        return;
    if (othervar.f9bd) {
        if (help_group == othervar.f9bf)
            set_help_from_index(othervar.f9bd, x, y);
        return;
    }
    for (i = statvar.help_start[help_group]; resvars.f2c_HelpHotspotp[i].x0 >= 0; i++) {
        if (dx >= resvars.f2c_HelpHotspotp[i].x0 && dx <= resvars.f2c_HelpHotspotp[i].x1 && dy >= resvars.f2c_HelpHotspotp[i].y0
            && dy <= resvars.f2c_HelpHotspotp[i].y1) {
            help_hotspot_detected(i, help_group, x, y);
            return;
        }
    }
    if (help_group == othervar.f8a6)
        othervar.f8ac = 0;
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

    othervar.f8a7 = mode;
    set_font_from_cache(display_mode == 2 ? 0x2d : 0);
    lbDisplay.draw_flags = 0;
    switch (mode) {
    case 1:
        copy_and_format(othervar.f96, strs[0]);
        border = 3;
        othervar.f89a = LbTextStringWidth(othervar.f96) + border * 2;
        othervar.f89c = LbTextStringHeight(othervar.f96) + border * 2;
        break;
    case 2:
        copy_and_format_composite(othervar.f96, strs);
        border = pulldv[12].height + 0xc;
        othervar.f89a = LbTextStringWidth(othervar.f96) + border * 2;
        othervar.f89c = LbTextStringHeight(othervar.f96) + border * 2 + pulldv[19].height;
        break;
    }
    if (x == -1 && y == -1) {
        othervar.f896 = (lbDisplay.unknown18 - othervar.f89a) / 2;
        othervar.f898 = (lbDisplay.unknown1c - othervar.f89c) / 2;
        LbMouseSetWindow(othervar.f896, othervar.f898, othervar.f89a, othervar.f89c);
        x0 = othervar.f896;
        y0 = othervar.f898;
        x1 = x0 + othervar.f89a;
        y1 = y0 + othervar.f89c;
        if (!((lbDisplay.mmouse_x >= x0 && lbDisplay.mmouse_x < x1) && lbDisplay.mmouse_y >= y0 && lbDisplay.mmouse_y < y1)) {
            mx = lbDisplay.mmouse_x;
            my = lbDisplay.mmouse_y;
            mx < x0 ? mx = x0 : 0;
            mx > x1 ? mx = x1 : 0;
            my < y0 ? my = y0 : 0;
            my > y1 ? my = y1 : 0;
            LbMouseSetPosition(mx, my);
        }
    } else {
        othervar.f896 = x - border;
        othervar.f898 = y - border;
        othervar.f898 -= othervar.f89c;
        othervar.f898 += 6;
    }
    othervar.f896 < 0 ? othervar.f896 = 0 : 0;
    othervar.f898 < 0 ? othervar.f898 = 0 : 0;
    othervar.f896 > lbDisplay.unknown18 - othervar.f89a ? othervar.f896 = lbDisplay.unknown18 - othervar.f89a : 0;
    othervar.f898 > lbDisplay.unknown18 - othervar.f89c ? othervar.f898 = lbDisplay.unknown18 - othervar.f89c : 0;
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

    if (!(othervar.f96[0] && (mouse_mode_help_allowed[gamevar.f12e] || (advisor.f11f & 1))))
        return;
    set_all_refresh_zones_in_window(othervar.f896, othervar.f898, othervar.f896 + othervar.f89a + 0xc,
        othervar.f898 + othervar.f89c + 0xc, 3);
    if (othervar.f8a6 == 3)
        D_00156910 = 1;
    LbScreenSetGraphicsWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    LbTextSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown18);
    lbDisplay.draw_flags = 0;
    LbDrawBox(othervar.f896, othervar.f898, othervar.f89a, othervar.f89c, othervar.fa65 ? color.unknown7 : 0xab);
    switch (othervar.f8a7) {
    case 1:
        lbDisplay.draw_flags = 0x10;
        LbDrawBox(othervar.f896, othervar.f898, othervar.f89a, othervar.f89c, color.unknown8);
        border = 3;
        break;
    case 2:
        step = pulldv[12].width;
        j = othervar.f898;
        bottom = othervar.f898 + othervar.f89c - pulldv[16].height;
        for (i = othervar.f896; othervar.f896 + othervar.f89a - step > i; i += step) {
            reqspr_draw(i, j, &pulldv[12]);
            reqspr_draw(i, bottom, &pulldv[16]);
        }
        step = pulldv[18].height;
        i = othervar.f896;
        right = othervar.f896 + othervar.f89a - pulldv[14].width;
        for (j = othervar.f898; othervar.f898 + othervar.f89c - step > j; j += step) {
            reqspr_draw(i, j, &pulldv[18]);
            reqspr_draw(right, j, &pulldv[14]);
        }
        reqspr_draw(othervar.f896, othervar.f898, &pulldv[11]);
        reqspr_draw(othervar.f896 + othervar.f89a - pulldv[13].width, othervar.f898, &pulldv[13]);
        reqspr_draw(othervar.f896, othervar.f898 + othervar.f89c - pulldv[17].height, &pulldv[17]);
        reqspr_draw(othervar.f896 + othervar.f89a - pulldv[15].width, othervar.f898 + othervar.f89c - pulldv[15].height,
            &pulldv[15]);
        border = pulldv[12].height + 0xc;
        othervar.f8a8 = othervar.f896 + othervar.f89a - 0xc - pulldv[19].width;
        othervar.f8aa = othervar.f898 + othervar.f89c - 0xc - pulldv[19].height;
        if ((othervar.f8a8 <= lbDisplay.mmouse_x && othervar.f8a8 + pulldv[19].width > lbDisplay.mmouse_x)
            && othervar.f8aa <= lbDisplay.mmouse_y && othervar.f8aa + pulldv[19].height > lbDisplay.mmouse_y) {
            reqspr_draw(othervar.f8a8, othervar.f8aa, &pulldv[20]);
            if (!D_00161134)
                play_sound(10, 0x44, -1, -1);
            D_00161134 = 1;
        } else {
            reqspr_draw(othervar.f8a8, othervar.f8aa, &pulldv[19]);
            D_00161134 = 0;
        }
        lbDisplay.draw_flags = 4;
        LbDrawBox(othervar.f896 + othervar.f89a, othervar.f898 + 0xc, 0xc, othervar.f89c, color.unknown8);
        LbDrawBox(othervar.f896 + 0xc, othervar.f898 + othervar.f89c, othervar.f89a - 0xc, 0xc, color.unknown8);
        lbDisplay.draw_flags = 0;
        break;
    }
    lbDisplay.draw_flags = 0x40;
    set_font_from_cache(display_mode == 2 ? 0x2d : 0);
    lbDisplay.draw_colour = color.unknown8;
    LbTextDraw(othervar.f896 + border, othervar.f898 + border, othervar.f96);
    lbDisplay.draw_flags = 0;
    if (gamevar.f12e != 0x25)
        othervar.f96[0] = 0;
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
            if (strcmp(p, "//") == 0) {
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
    for (; strcmp(*strs, ".") != 0; strs++) {
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
            if (strcmp(p, "//") == 0) {
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
    othervar.f8ac = 0;
    othervar.f89e = 0;
    othervar.f96[0] = 0;
}

// FUNCTION: 0x0009ab84
void help_hotspot_detected(u16 hotspot, u8 help_group, s16 x, s16 y)
{
    s32 slot;

    if (statvar.unknownfd > 1) {
        slot = hotspot - statvar.help_start[help_group];
        if (help_group == 0x18) {
            if (slot < 4 && network_num_players() <= slot)
                return;
        }
    }
    if (hotspot == othervar.f8ac) {
        if (othervar.f92 + 0x2ee < othervar.f8a2)
            othervar.f89e = othervar.f8a2;
    } else {
        othervar.f8a6 = help_group;
        othervar.f8ac = hotspot;
        othervar.f92 = othervar.f8a2;
    }
    if (othervar.f89e + 0x1f4 > othervar.f8a2) {
        othervar.f89e = othervar.f8a2;
        othervar.f8a6 = help_group;
        set_help_from_index(hotspot, x, y);
    }
}

// FUNCTION: 0x0009ac70
void set_help_from_index(u16 hotspot, s16 x, s16 y)
{
    s16 cy;
    s16 cx;
    char *text;

    cx = x + (resvars.f2c_HelpHotspotp[hotspot].x0 + resvars.f2c_HelpHotspotp[hotspot].x1) / 2;
    cy = y + resvars.f2c_HelpHotspotp[hotspot].y0;
    if (resvars.f2c_HelpHotspotp[hotspot].func) {
        text = othervar.f96;
        resvars.f2c_HelpHotspotp[hotspot].func();
        set_help_text(1, cx, cy, &text);
    } else {
        set_help_text(1, cx, cy, &help_text[resvars.f2c_HelpHotspotp[hotspot].text]);
    }
}

// FUNCTION: 0x0009ad68
void set_force_help(u8 help_group, u16 id)
{
    u16 i;
    u8 found;

    i = 0;
    found = 0;
    othervar.f9bf = help_group;
    for (i = 0; resvars.f2c_HelpHotspotp[i].x0 != -2; i++) {
        if (id == resvars.f2c_HelpHotspotp[i].text) {
            othervar.f9bd = i;
            break;
        }
    }
}

// FUNCTION: 0x0009ade8
void clear_force_help(void)
{
    othervar.f9bd = 0;
}

// FUNCTION: 0x0009ae08
void show_help_page(u32 page, u32 sub)
{
    char *saved;

    set_mouse_mode(0x25);
    switch (page) {
    case 0:
        set_help_text(2, -1, -1, &D_0016037c[statvar.f4df[sub]]);
        break;
    case 1:
        set_help_text(2, -1, -1, &D_00160374[statvar.f51d[sub]]);
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
            sprintf(buf, D_00160398[statvar.f55b[sub]], current_net_criteria.start - othervar.f9d1);
            saved = D_00160398[statvar.f55b[sub]];
            D_00160398[statvar.f55b[sub]] = buf;
            break;
        case 4:
        case 6:
        case 5:
        case 7:
            if (current_net_criteria.unknown0 == 7 && myplayer.f21 == 2)
                sprintf(buf, D_00160398[statvar.f55b[sub]], current_net_criteria.start - othervar.f9d1,
                    current_net_criteria.value);
            else
                sprintf(buf, D_00160398[statvar.f55b[sub]], current_net_criteria.value,
                    current_net_criteria.start - othervar.f9d1);
            saved = D_00160398[statvar.f55b[sub]];
            D_00160398[statvar.f55b[sub]] = buf;
            break;
        }
        set_help_text(2, -1, -1, &D_00160398[statvar.f55b[sub]]);
        D_00160398[statvar.f55b[sub]] = saved;
        break;
    }
    case 3:
        set_help_text(2, -1, -1, &D_0016039c[statvar.f56d[sub]]);
        break;
    }
    othervar.fa92 = page;
    othervar.fa93 = sub;
}

// FUNCTION: 0x0009b144
void setup_indices_for_composite_help(char **strs, u16 *idx, u32 n)
{
    u32 count;
    u32 i;

    count = 0;
    i = 0;
    if (strs) {}
    for (; strcmp(*strs, "..") != 0 && count < n; count++) {
        *idx++ = i;
        for (; strcmp(*strs, ".") != 0; i++)
            strs++;
        strs++;
        i++;
    }
    if (count == n && strcmp(*strs, "..") != 0) {}
}

// FUNCTION: 0x0009b1f0
void finish_composite_help(void)
{
    LbMouseSetWindow(0, 0, lbDisplay.unknown18, lbDisplay.unknown1c);
    D_00161134 = 0;
    D_00156910 = 1;
    othervar.f96[0] = 0;
    if (gamevar.f12e == 0x25) {
        set_mouse_mode(0);
        play_sound(10, 0x41, -1, -1);
        switch (othervar.fa92) {
        case 1:
            switch (othervar.fa93) {
            case 0xf:
            case 0x10:
                show_help_page(1, othervar.fa93 + 1);
                break;
            }
            break;
        case 3:
            if (othervar.fa93 == 2) {
                D_001549d0[player].type = 0x7e;
                D_001549d0[player].value = 1;
            }
            break;
        }
    }
}
