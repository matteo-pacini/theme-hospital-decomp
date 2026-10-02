#include "hosp.h"

extern "C" {
extern u8 D_001567c0[][16];
void LbVesaSetPage(s16 page);
}

void refresh_to_VRAM(void);

void copy_dwords(u32 dst, u8 *src, u32 count);
#pragma aux copy_dwords = "rep movsd" parm [edi] [esi] [ecx] modify [edi esi ecx];

// Copies the dirty parts of the screen to video memory with the mouse
// pointer drawn in.
// FUNCTION: 0x000ab120
void special_refresh(void)
{
    LbMousePlace();
    refresh_to_VRAM();
    LbMouseRemove();
}

// Copies each 32x32 zone of the back buffer marked in D_001567c0 to the
// 640-wide banked VESA screen, switching banks as needed, and decrements
// the zones' counts.
// FUNCTION: 0x000ab144
void refresh_to_VRAM(void)
{
    static s8 vesaPageNum[] = {0, 0, 0, -1, 1, 1, -2, 2, 2, -3, 3, 3, -4, 4, 4, 0};
    s8 page;
    u32 dst;
    s32 col;
    s32 row;
    s32 line;
    s32 end;
    s32 width;
    u8 *src_next;
    u32 dst_next;
    u32 offset;
    u8 *src;

    page = -1;
    for (row = 0; row < lbDisplay.physical_screen_height / 32; row++) {
        for (col = 0; col < lbDisplay.physical_screen_width / 32; col++) {
            if (D_001567c0[col][row] > 0) {
                end = col;
                do {
                    D_001567c0[end][row]--;
                    end++;
                } while (D_001567c0[end][row] > 0 && end < lbDisplay.physical_screen_width / 32);
                width = end - col;
                offset = lbDisplay.physical_screen_width * (row << 5) + (col << 5);
                src = lbDisplay.wscreen + offset;
                dst = (offset & 0xffff) + 0xa0000;
                dst_next = dst + 0x280;
                src_next = src + 0x280;
                if (vesaPageNum[row] > -1) {
                    if (vesaPageNum[row] != page) {
                        page = vesaPageNum[row];
                        LbVesaSetPage(page);
                    }
                    for (line = 0; line < 32;
                         line++, src = src_next, dst = dst_next, src_next += 0x280, dst_next += 0x280)
                        copy_dwords(dst, src, width << 3);
                } else {
                    u32 bytes;
                    u8 done;
                    u32 bank;
                    u32 bank2;
                    u32 dwords;

                    bytes = width << 5;
                    done = 0;
                    bank = (offset & 0xffff0000) >> 16;
                    if (page != bank) {
                        page = bank;
                        LbVesaSetPage(page);
                    }
                    for (line = 0; !done; line++, src = src_next, dst = dst_next, src_next += 0x280, dst_next += 0x280,
                        offset += lbDisplay.physical_screen_width, bank = (offset & 0xffff0000) >> 16) {
                        if (page != bank) {
                            page = bank;
                            LbVesaSetPage(page);
                            dst = (offset & 0xffff) + 0xa0000;
                            copy_dwords(dst, src, width << 3);
                            dst_next = dst + 0x280;
                            done = 1;
                        } else {
                            bank2 = ((offset + bytes) & 0xffff0000) >> 16;
                            if (bank != bank2) {
                                dwords = (0x10000 - (offset & 0xffff)) >> 2;
                                copy_dwords(dst, src, dwords);
                                page = bank2;
                                LbVesaSetPage(page);
                                copy_dwords(0xa0000, (dwords << 2) + src, (width << 3) - dwords);
                                dst_next = ((offset + lbDisplay.physical_screen_width) & 0xffff) + 0xa0000;
                                done = 1;
                            } else {
                                copy_dwords(dst, src, width << 3);
                            }
                        }
                    }
                    for (; line < 32; line++, src = src_next, dst = dst_next, src_next += 0x280, dst_next += 0x280)
                        copy_dwords(dst, src, width << 3);
                }
                col = end - 1;
            }
        }
    }
}

// Marks the 32x32 zones covering the box (x1, y1)-(x2, y2), clipped to the
// screen, to be copied to video memory for the next refresh_count frames.
// FUNCTION: 0x000ab49c
void set_all_refresh_zones_in_window(const s32 x1, const s32 y1, const s32 x2, const s32 y2, const u8 refresh_count)
{
    s32 zy2;
    s32 zx2;
    s32 zy1;
    s32 zx1;
    s32 cy2;
    s32 cy1;
    s32 cx2;
    s32 cx1;
    u8 x;
    u8 y;

    if (x1 < x2 && y1 < y2) {
        cx1 = x1;
        cx2 = x2;
        cy1 = y1;
        cy2 = y2;
        if (cx1 < 0)
            cx1 = 0;
        else if (cx1 > lbDisplay.physical_screen_width)
            cx1 = lbDisplay.physical_screen_width;
        if (cx2 < 0)
            cx2 = 0;
        else if (cx2 > lbDisplay.physical_screen_width)
            cx2 = lbDisplay.physical_screen_width;
        if (cy1 < 0)
            cy1 = 0;
        else if (cy1 > lbDisplay.physical_screen_height)
            cy1 = lbDisplay.physical_screen_height;
        if (cy2 < 0)
            cy2 = 0;
        else if (cy2 > lbDisplay.physical_screen_height)
            cy2 = lbDisplay.physical_screen_height;
        zx1 = cx1 / 32;
        zy1 = cy1 / 32;
        zx2 = cx2 / 32;
        zy2 = cy2 / 32;
        for (y = zy1; y <= (u8)zy2; y++)
            for (x = zx1; x <= (u8)zx2; x++)
                D_001567c0[x][y] = refresh_count;
    }
}
