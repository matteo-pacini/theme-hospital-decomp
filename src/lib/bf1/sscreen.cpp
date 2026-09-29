#include "bflib.h"

extern u32 screen_initialised;
extern u32 _lbScreenInitialised;
extern u32 mouse_pos_change_saved;

extern "C" {

int LbMouseSuspend(void);
int LbVesaGetInfo(void);
int LbVesaIsModeAvailable(u16 mode);
void LbVesaSetPage(s16 page);

void rep_movsd(void *dst, void *src, u32 count);
#pragma aux rep_movsd = 0xf3 0xa5 parm [edi] [esi] [ecx] modify [ecx esi edi];

void *memcpy(void *dst, const void *src, u32 n);
#pragma intrinsic(memcpy)

void swap_clear(void *dst, void *src, u32 count, u32 fill);
#pragma aux swap_clear = 0xa5 0x89 0x56 0xfc 0xe2 0xfa parm [edi] [esi] [ecx] [edx] modify [ecx esi edi];

/* Adds 0x80 to each byte from p on (at least one) until p reaches end. */
void invert_row(u8 *p, u8 *end);
#pragma aux invert_row = 0x8a 0x07 0x04 0x80 0x88 0x07 0x47 0x39 0xf9 0x7f 0xf5 parm [edi] [ecx] modify [eax edi];

/* Adds 0x80 to every stride-th byte from p on (at least one) until p reaches end; returns the final p. */
u8 *invert_column(u8 *p, u8 *end, s32 stride);
#pragma aux invert_column = 0x8a 0x07 0x04 0x80 0x88 0x07 0x01 0xdf 0x39 0xf9 0x7f 0xf4 parm [edi] [ecx] [ebx] value [edi] modify [eax];

void wait_vbi(void);
#pragma aux wait_vbi = 0xba 0xda 0x03 0x00 0x00 0x29 0xc0 0xec 0xa8 0x08 0x74 0xf4 modify [eax edx];

// FUNCTION: 0x000becd0
int LbScreenClear(u8 colour, ...)
{
    LbMemorySet(lbDisplay.unknown4, colour, lbDisplay.graphics_screen_width * lbDisplay.graphics_screen_height);
    return 1;
}

// FUNCTION: 0x000becfc
int LbScreenReset(void)
{
    REGS out;
    REGS in;

    if (!screen_initialised)
        return -1;
    LbMouseSuspend();
    in.x.eax = lbDisplay.old_video_mode;
    int386(0x10, &in, &out);
    screen_initialised = 0;
    _lbScreenInitialised = 0;
    return 1;
}

// FUNCTION: 0x000bed50
int fn_000bed50(void)
{
    return 1;
}

// FUNCTION: 0x000bed5c
int LbIsActive(void)
{
    return 1;
}

u32 LbScreenSetDoubleBuffering(u32 state);
#pragma aux LbScreenSetDoubleBuffering parm caller []
u32 LbScreenSetWScreenInVideo(u32 state);
#pragma aux LbScreenSetWScreenInVideo parm caller []

// FUNCTION: 0x000bed68
u32 LbScreenSetDoubleBuffering(u32 state)
{
    static u32 cur_flag;
    u32 old = cur_flag;

    cur_flag = state;
    return old;
}

// FUNCTION: 0x000bed88
u32 LbScreenSetWScreenInVideo(u32 state)
{
    static u32 cur_flag;
    u32 old = cur_flag;

    cur_flag = state;
    return old;
}

// FUNCTION: 0x000beda8
int LbScreenFindVideoModes(void)
{
    int i;

    LbVesaGetInfo();
    for (i = 1; (u32)lbScreenModeInfo[i].width; i++) {
        if (lbScreenModeInfo[i].VideoMode >= 256)
            lbScreenModeInfo[i].Available = LbVesaIsModeAvailable(lbScreenModeInfo[i].VideoMode);
    }
    lbScreenModeInfo[1].Available = 1;
    return 1;
}

// FUNCTION: 0x000bee60
int LbScreenSwap(void)
{
    s32 remaining;
    s32 chunk;
    u8 *src;
    s16 page;

    LbMousePlace();
    if (!lbDisplay.vesa_is_set_up) {
        rep_movsd(lbDisplay.physical_screen, lbDisplay.unknown4, (lbDisplay.graphics_screen_width * lbDisplay.graphics_screen_height) >> 2);
    } else {
        remaining = lbDisplay.graphics_screen_width * lbDisplay.graphics_screen_height;
        src = lbDisplay.unknown4;
        for (page = 0; remaining != 0; src += 0x10000) {
            chunk = remaining < 0x10000 ? remaining : 0x10000;
            remaining -= chunk;
            LbVesaSetPage(page++);
            rep_movsd(lbDisplay.physical_screen, src, chunk >> 2);
        }
    }
    LbMouseRemove();
    return 1;
}

// FUNCTION: 0x000bef14
int LbScreenSwapBoxClear(u8 *source, s32 source_x, s32 source_y, s32 dest_x, s32 dest_y, u32 width, u32 height, u8 colour, ...)
{
    u8 *src;
    u32 dst;
    u32 offset;
    u32 end_offset;
    u32 last_bank;
    u32 bank;
    u32 end_bank;
    u32 first;
    u32 rest;
    u16 i;

    LbMousePlace();
    offset = dest_x + dest_y * lbDisplay.unknown18;
    end_offset = dest_x + width + dest_y * lbDisplay.unknown18;
    src = source + source_x + source_y * lbDisplay.graphics_screen_width;
    last_bank = bank = (offset & 0xffff0000) >> 16;
    if (lbDisplay.vesa_is_set_up)
        LbVesaSetPage(bank);
    for (i = 0; i < height;
         i++, offset += lbDisplay.graphics_screen_width, end_offset += lbDisplay.unknown18,
         src += lbDisplay.graphics_screen_width, dst += width) {
        bank = (offset & 0xffff0000) >> 16;
        if (bank != last_bank)
            LbVesaSetPage(bank);
        last_bank = bank;
        end_bank = (end_offset & 0xffff0000) >> 16;
        if (bank != end_bank) {
            last_bank = end_bank;
            first = 0x10000 - (offset & 0xffff);
            rest = width - first;
            dst = (offset & 0xffff) + 0xa0000;
            memcpy((void *)dst, src, first);
            /* Bullfrog passes the length as the fill value and the colour as the length. */
            memset(src, first, colour);
            dst = 0xa0000;
            LbVesaSetPage(end_bank);
            memcpy((void *)dst, src + first, rest);
            memset(src + first, rest, colour);
        } else {
            dst = (offset & 0xffff) + 0xa0000;
            memcpy((void *)dst, src, width);
            memset(src, width, colour);
        }
    }
    LbMouseRemove();
    return 1;
}

// FUNCTION: 0x000bf0e4
int LbScreenSwapClear(u8 colour, ...)
{
    s32 remaining;
    s32 chunk;
    u8 *src;
    s16 page;

    LbMousePlace();
    if (!lbDisplay.vesa_is_set_up) {
        swap_clear(lbDisplay.physical_screen, lbDisplay.unknown4, (lbDisplay.graphics_screen_width * lbDisplay.graphics_screen_height) >> 2,
            colour + (colour << 8) + (colour << 16) + (colour << 24));
    } else {
        remaining = lbDisplay.graphics_screen_width * lbDisplay.graphics_screen_height;
        src = lbDisplay.unknown4;
        for (page = 0; remaining != 0; src += 0x10000) {
            chunk = remaining < 0x10000 ? remaining : 0x10000;
            LbVesaSetPage(page++);
            swap_clear(lbDisplay.physical_screen, src, chunk >> 2, colour + (colour << 8) + (colour << 16) + (colour << 24));
            remaining -= chunk;
        }
    }
    LbMouseRemove();
    return 1;
}

// FUNCTION: 0x000bf1e8
int LbScreenSwapBox(u8 *source, s32 source_x, s32 source_y, s32 dest_x, s32 dest_y, u32 width, u32 height, ...)
{
    u8 *src;
    u32 dst;
    u32 offset;
    u32 end_offset;
    u32 last_bank;
    u32 bank;
    u32 end_bank;
    u32 first;
    u32 rest;
    u16 i;

    LbMousePlace();
    offset = dest_x + dest_y * lbDisplay.unknown18;
    end_offset = dest_x + width + dest_y * lbDisplay.unknown18;
    src = source + source_x + source_y * lbDisplay.graphics_screen_width;
    last_bank = bank = (offset & 0xffff0000) >> 16;
    if (lbDisplay.vesa_is_set_up)
        LbVesaSetPage(bank);
    for (i = 0; i < height;
         i++, offset += lbDisplay.graphics_screen_width, end_offset += lbDisplay.unknown18,
         src += lbDisplay.graphics_screen_width, dst += width) {
        bank = (offset & 0xffff0000) >> 16;
        if (bank != last_bank)
            LbVesaSetPage(bank);
        end_bank = (end_offset & 0xffff0000) >> 16;
        if (bank != end_bank) {
            last_bank = end_bank;
            first = 0x10000 - (offset & 0xffff);
            rest = width - first;
            dst = (offset & 0xffff) + 0xa0000;
            memcpy((void *)dst, src, first);
            dst = 0xa0000;
            LbVesaSetPage(end_bank);
            memcpy((void *)dst, src + first, rest);
        } else {
            last_bank = bank;
            dst = (offset & 0xffff) + 0xa0000;
            memcpy((void *)dst, src, width);
        }
    }
    LbMouseRemove();
    return 1;
}

// FUNCTION: 0x000bf374
int LbScreenDrawHVLineDirect(s32 x1, s32 y1, s32 x2, s32 y2, ...)
{
    u32 length;
    u32 start;
    u32 end;
    u32 start_page;
    u32 end_page;
    u32 page = 0;
    u8 *p;
    u8 *pstart;
    u8 *pend;
    u8 *pbank_end;

    if (x1 <= x2) {
        if (x2 < 1)
            return -1;
        if (x1 > lbDisplay.graphics_window_width - 1)
            return -1;
        if (x1 < 0)
            x1 = 0;
        if (x2 > lbDisplay.graphics_window_width)
            x2 = lbDisplay.graphics_window_width;
    } else {
        if (x1 < 0)
            return -1;
        if (x2 > lbDisplay.graphics_window_width)
            return -1;
        if (x2 < 0)
            x2 = 0;
        if (x1 > lbDisplay.graphics_window_width)
            x1 = lbDisplay.graphics_window_width;
    }
    if (y1 <= y2) {
        if (y2 < 0)
            return -1;
        if (y1 > lbDisplay.graphics_window_height)
            return -1;
        if (y1 < 0)
            y1 = 0;
        if (y2 > lbDisplay.graphics_window_height)
            y2 = lbDisplay.graphics_window_height;
    } else {
        if (y1 < 0)
            return -1;
        if (y2 > lbDisplay.graphics_window_height)
            return -1;
        if (y2 < 0)
            y2 = 0;
        if (y1 > lbDisplay.graphics_window_height)
            y1 = lbDisplay.graphics_window_height;
    }
    mouse_pos_change_saved = 1;
    if (x1 != x2) {
        if (x1 > x2) {
            length = x1 - x2;
            start = x2 + y1 * lbDisplay.graphics_window_width;
        } else {
            length = x2 - x1;
            start = x1 + y1 * lbDisplay.graphics_window_width;
        }
        end = start + length;
        start_page = start >> 16;
        end_page = end >> 16;
        if (!lbDisplay.vesa_is_set_up) {
            pstart = lbDisplay.physical_screen + start;
            pend = lbDisplay.physical_screen + end;
            invert_row(pstart, pend);
        } else if (start_page == end_page) {
            LbVesaSetPage(start_page);
            pstart = lbDisplay.physical_screen + (start - (start_page << 16));
            pend = lbDisplay.physical_screen + (end - (end_page << 16));
            p = pstart;
            invert_row(p, pend);
        } else {
            LbVesaSetPage(start_page);
            pstart = lbDisplay.physical_screen + (start - (start_page << 16));
            pbank_end = lbDisplay.physical_screen + 0x10000;
            p = pstart;
            invert_row(p, pbank_end);
            LbVesaSetPage(end_page);
            pend = lbDisplay.physical_screen + (end - (end_page << 16));
            p = lbDisplay.physical_screen;
            invert_row(p, pend);
        }
    } else {
        if (y1 > y2) {
            length = y1 - y2;
            start = x1 + y2 * lbDisplay.graphics_window_width;
        } else {
            length = y2 - y1;
            start = x1 + y1 * lbDisplay.graphics_window_width;
        }
        end = start + lbDisplay.graphics_screen_width * length;
        if (!lbDisplay.vesa_is_set_up) {
            pstart = lbDisplay.physical_screen + start;
            pend = lbDisplay.physical_screen + end;
            p = invert_column(pstart, pend, lbDisplay.graphics_screen_width);
        } else {
            start_page = start >> 16;
            end_page = end >> 16;
            pstart = lbDisplay.physical_screen + (start - (start_page << 16));
            pend = lbDisplay.physical_screen + (end - (end_page << 16));
            pbank_end = lbDisplay.physical_screen + 0x10000;
            if (start_page == end_page)
                pbank_end = pend;
            for (page = start_page; page <= end_page; page++) {
                LbVesaSetPage(page);
                if (page == start_page) {
                    p = pstart;
                    p = invert_column(p, pbank_end, lbDisplay.graphics_screen_width);
                } else if (page == end_page) {
                    p -= 0x10000;
                    p = invert_column(p, pend, lbDisplay.graphics_screen_width);
                } else {
                    p -= 0x10000;
                    p = invert_column(p, pbank_end, lbDisplay.graphics_screen_width);
                }
            }
        }
    }
    mouse_pos_change_saved = 0;
    return 1;
}

// FUNCTION: 0x000bf760
int LbScreenWaitVbi(void)
{
    wait_vbi();
    return 1;
}

}
