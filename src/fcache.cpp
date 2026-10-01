#include "hosp.h"

struct FontMem {
    s32 offset;
    s32 end;
    s32 size;
    u32 used;
    u8 font;
    s8 next;
};

extern FontMem font_memory[];
extern s8 fontmem_head;
extern s8 used_head;
extern u8 cfonts[];

extern "C" {
extern s8 D_00175239;
extern u8 D_00161270[];
extern u8 D_0016126c;
extern u8 D_0016126d;
}

s8 get_fontmem(void);
void put_fontmem(s8 block);
void init_font_cache(void);
void insert_block_into_free_list(s8 block);
void insert_block_into_used_list(s8 block);
void realign_sprites(TbSprite *start, TbSprite *end, s32 delta);
void defrag_cache(void);
void throw_out(s32 size);
s8 get_block_from_free_list(s32 size);

// FUNCTION: 0x000b6800
s8 get_fontmem(void)
{
    s8 block;

    block = fontmem_head;
    if (block == -1) {
        hosp_dos_printf("FONT CACHE IS PANICKED! Increase MAX_NB_FONTS (number of FontMem structures)!\n");
        return -1;
    }
    block = fontmem_head;
    fontmem_head = font_memory[block].next;
    font_memory[block].next = -1;
    return block;
}

// FUNCTION: 0x000b6870
void put_fontmem(s8 block)
{
    font_memory[block].offset = 0;
    font_memory[block].size = 0;
    font_memory[block].next = fontmem_head;
    fontmem_head = block;
}

// FUNCTION: 0x000b68c8
void init_font_cache(void)
{
    s32 i;

    for (i = 0; i < 0x30; i++) {
        font_memory[i].offset = 0;
        font_memory[i].size = 0;
        font_memory[i].next = i + 1;
        cfonts[i] = 0;
    }
    font_memory[i - 1].next = -1;
    fontmem_head = 0;
    used_head = -1;
    D_00175239 = get_fontmem();
    if (D_00175239 != -1) {
        font_memory[D_00175239].offset = 0;
        font_memory[D_00175239].size = 81000;
    }
}

// FUNCTION: 0x000b6988
void insert_block_into_free_list(s8 block)
{
    s8 cur;

    cur = D_00175239;
    if (cur == -1) {
        font_memory[block].next = -1;
        D_00175239 = block;
        return;
    }
    while (cur != -1) {
        if (font_memory[cur].offset + font_memory[cur].size == font_memory[block].offset) {
            font_memory[cur].size += font_memory[block].size;
            put_fontmem(block);
            return;
        }
        if (font_memory[block].offset + font_memory[block].size == font_memory[cur].offset) {
            font_memory[cur].size += font_memory[block].size;
            font_memory[cur].offset -= font_memory[block].size;
            put_fontmem(block);
            return;
        }
        cur = font_memory[cur].next;
    }
    font_memory[block].next = D_00175239;
    D_00175239 = block;
}

// FUNCTION: 0x000b6ac8
void insert_block_into_used_list(s8 block)
{
    s8 cur;

    cur = used_head;
    font_memory[block].used = othervar.f8a2;
    if (cur == -1) {
        font_memory[block].next = -1;
        used_head = block;
        return;
    }
    if (font_memory[block].offset <= font_memory[cur].offset) {
        font_memory[block].next = cur;
        used_head = block;
        return;
    }
    while (font_memory[cur].next != -1) {
        if (font_memory[block].offset <= font_memory[font_memory[cur].next].offset) {
            font_memory[block].next = font_memory[cur].next;
            font_memory[cur].next = block;
            return;
        }
        cur = font_memory[cur].next;
    }
    font_memory[cur].next = block;
    font_memory[block].next = -1;
}

// FUNCTION: 0x000b6bf4
void realign_sprites(TbSprite *first, TbSprite *last, s32 delta)
{
    TbSprite *s;

    for (s = first; s < last; s++)
        s->data += delta;
}

// FUNCTION: 0x000b6c34
void defrag_cache(void)
{
    s8 i;
    s32 pos;

    pos = 0;
    i = used_head;
    while (i != -1) {
        if (font_memory[i].offset != pos) {
            memmove(&D_00161270[pos], &D_00161270[font_memory[i].offset], font_memory[i].size);
            font_memory[i].end -= font_memory[i].offset - pos;
            realign_sprites((TbSprite *)&D_00161270[pos], (TbSprite *)&D_00161270[font_memory[i].end],
                            pos - font_memory[i].offset);
            font_memory[i].offset = pos;
        }
        pos += font_memory[i].size;
        i = font_memory[i].next;
    }
    while (D_00175239 != -1) {
        i = D_00175239;
        D_00175239 = font_memory[i].next;
        put_fontmem(i);
    }
    D_00175239 = get_fontmem();
    if (D_00175239 != -1) {
        font_memory[D_00175239].offset = pos;
        font_memory[D_00175239].size = 81000 - pos;
    }
}

// FUNCTION: 0x000b6da0
void throw_out(s32 size)
{
    s32 oldest;
    s32 freed;
    s8 best;
    s8 i;
    s8 prev;

    freed = 0;
    while (freed < size) {
        best = -1;
        prev = -1;
        oldest = -1;
        i = used_head;
        while (i != -1) {
            if (font_memory[i].used <= (u32)oldest) {
                oldest = font_memory[i].used;
                best = prev;
            }
            prev = i;
            i = font_memory[i].next;
        }
        if (best == -1) {
            best = used_head;
            used_head = font_memory[best].next;
            freed += font_memory[best].size;
            cfonts[font_memory[best].font] = 0;
            insert_block_into_free_list(best);
        } else {
            prev = best;
            best = font_memory[best].next;
            freed += font_memory[best].size;
            font_memory[prev].next = font_memory[best].next;
            cfonts[font_memory[best].font] = 0;
            insert_block_into_free_list(best);
        }
    }
}

// FUNCTION: 0x000b6eec
s8 get_block_from_free_list(s32 size)
{
    s32 left;
    s32 total;
    s8 rest;
    s8 block;
    s8 cur;

    total = 0;
    if (size > 81000)
        return -1;
    cur = D_00175239;
    if (cur == -1)
        return -1;
    left = font_memory[cur].size - size;
    total += font_memory[cur].size;
    if (left >= 0) {
        D_00175239 = font_memory[cur].next;
        font_memory[cur].next = -1;
        if (left > 0) {
            rest = get_fontmem();
            if (rest != -1) {
                font_memory[cur].size = size;
                font_memory[rest].offset = font_memory[cur].offset + size;
                font_memory[rest].size = left;
                insert_block_into_free_list(rest);
            }
        }
        return cur;
    }
    while (font_memory[cur].next != -1) {
        total += font_memory[font_memory[cur].next].size;
        left = font_memory[font_memory[cur].next].size - size;
        if (left >= 0) {
            block = font_memory[cur].next;
            font_memory[cur].next = font_memory[font_memory[cur].next].next;
            font_memory[block].next = -1;
            if (left > 0) {
                rest = get_fontmem();
                if (rest != -1) {
                    font_memory[block].size = size;
                    font_memory[rest].offset = font_memory[block].offset + size;
                    font_memory[rest].size = left;
                    insert_block_into_free_list(rest);
                }
            }
            return block;
        }
        cur = font_memory[cur].next;
    }
    if (!D_0016126c) {
        throw_out(size - total);
        defrag_cache();
        D_0016126c = 1;
        block = get_block_from_free_list(size);
        D_0016126c = 0;
        return block;
    }
    return -1;
}

// FUNCTION: 0x000b714c
void set_font_from_cache(const s32 font)
{
    char name[32];
    s32 tab_len;
    s32 dat_len;
    s32 total;
    s32 loaded;
    char **names;
    s8 block;

    if (resvars.f0 != D_0016126d)
        init_font_cache();
    D_0016126d = resvars.f0;
    if (cfonts[font]) {
        lbFontPtr = (TbSprite *)&D_00161270[font_memory[cfonts[font] - 1].offset];
        font_memory[font].used = othervar.f8a2;
        return;
    }
    names = resvars.fa_charpp;
    sprintf(name, "%s.tab", names[font]);
    tab_len = LbFileLengthRnc(name);
    if (tab_len == -1)
        return;
    sprintf(name, "%s.dat", names[font]);
    dat_len = LbFileLengthRnc(name);
    if (dat_len == -1)
        return;
    total = tab_len + dat_len;
    block = get_block_from_free_list(total);
    if (block == -1)
        return;
    sprintf(name, "%s.tab", names[font]);
    loaded = LbFileLoadAt(name, &D_00161270[font_memory[block].offset]);
    if (loaded != -1) {
        font_memory[block].end = font_memory[block].offset + loaded;
        sprintf(name, "%s.dat", names[font]);
        loaded = LbFileLoadAt(name, &D_00161270[font_memory[block].end]);
        if (loaded != -1) {
            realign_sprites((TbSprite *)&D_00161270[font_memory[block].offset],
                            (TbSprite *)&D_00161270[font_memory[block].end],
                            (s32)&D_00161270[font_memory[block].end]);
            lbFontPtr = (TbSprite *)&D_00161270[font_memory[block].offset];
            cfonts[font] = block + 1;
            font_memory[block].font = font;
            insert_block_into_used_list(block);
        } else {
            insert_block_into_free_list(block);
        }
    } else {
        insert_block_into_free_list(block);
    }
}
