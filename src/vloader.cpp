#include "hosp.h"

struct VariableTable {
    char name[0x30];
    u16 flags;
    u32 address;
    u16 stride;
    u16 max_index;
    s32 min;
    s32 max;
};

extern VariableTable balance_variables[];
extern char *file_buffer;
extern char current_line[];

extern "C" {
extern s32 D_00160f2c;
extern char D_00161034[];
extern char D_001610b4[];
extern char D_00120a34[];
extern char D_00120a68[];
extern char D_00120a9c[];
extern char D_00120ad0[];
extern char D_00120b04[];
extern char D_00120b38[];
extern char D_00120b6c[];
extern char D_00120b86[];
extern char D_00120bc2[];
extern char D_00120bf9[];
extern char D_00120c10[];
extern char D_00120c28[];
extern char D_00120c3f[];
extern char D_00120c41[];
extern char D_00120c43[];
extern char D_00120c45[];
extern char D_00120c47[];
extern char D_00120c49[];
extern char D_00120c4b[];
extern u8 __IsTable[];
#pragma aux __IsTable "*"

int strcmp(const char *a, const char *b);
char *strncpy(char *dst, const char *src, u32 n);
char *strncat(char *dst, const char *src, u32 n);
char *strstr(const char *s, const char *sub);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strtok(char *s, const char *delim);
s32 atol(const char *s);
void fn_000c93bf(char *dst, char *src);
u32 fn_000cd716(char *s);
#pragma aux fn_000cd716 "W?GetCurrentPlayersCallback$n(pn$TbNetworkCallbackData$$pnv)v"
}

s32 variable_address(char *name);
u16 array_index(char *name);
u8 number_of_members(char *name);
void strip_formatting(char *s);
char *read_line_from_buf(char *line, s32 max, char *src);

#define isspace(c) (__IsTable[(u8)((c) + 1)] & 2)

// NONMATCHING: register choice
// FUNCTION: 0x00099020
s8 assign_value(char *name, u32 value, u16 index)
{
    s32 var;
    u32 addr;
    char buf[0x80];

    var = 0;
    var = variable_address(name);
    if (var >= 0 && var < 0xcb) {
        if (index <= balance_variables[var].max_index) {
            addr = balance_variables[var].address;
            addr += balance_variables[var].stride * index;
            if ((balance_variables[var].flags & 1) && (balance_variables[var].flags & 0x10)) {
                if ((s32)value >= balance_variables[var].min && value <= balance_variables[var].max) {
                    *(u8 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120a34, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            if ((balance_variables[var].flags & 1) && (balance_variables[var].flags & 8)) {
                if ((s32)value >= balance_variables[var].min && (s32)value <= balance_variables[var].max) {
                    *(u8 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120a68, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            if ((balance_variables[var].flags & 2) && (balance_variables[var].flags & 0x10)) {
                if ((s32)value >= balance_variables[var].min && value <= balance_variables[var].max) {
                    *(u16 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120a9c, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            if ((balance_variables[var].flags & 2) && (balance_variables[var].flags & 8)) {
                s32 v = value;
                if ((s32)value >= balance_variables[var].min && (s32)value <= balance_variables[var].max) {
                    *(u16 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120ad0, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            if ((balance_variables[var].flags & 4) && (balance_variables[var].flags & 0x10)) {
                if ((s32)value >= balance_variables[var].min && value <= balance_variables[var].max) {
                    *(u32 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120b04, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            if ((balance_variables[var].flags & 4) && (balance_variables[var].flags & 8)) {
                s32 v = value;
                if ((s32)value >= balance_variables[var].min && (s32)value <= balance_variables[var].max) {
                    *(u32 *)addr = value;
                    return 1;
                }
                sprintf(buf, D_00120b38, value, balance_variables[var].name,
                    balance_variables[var].min, balance_variables[var].max);
                return -1;
            }
            sprintf(buf, D_00120b6c, name);
            return -1;
        } else {
            sprintf(buf, D_00120b86, balance_variables[var].max_index, balance_variables[var].name);
            return -1;
        }
    } else {
        sprintf(buf, D_00120bc2, name);
        return -1;
    }
}

// FUNCTION: 0x00099494
s32 variable_address(char *name)
{
    u32 i;

    i = 0;
    while (i < 0xcb) {
        if (strcmp(balance_variables[i].name, name) == 0)
            return i;
        i++;
    }
    return -1;
}

// NONMATCHING: register choice
// FUNCTION: 0x000994f0
void read_variable_file(char *name)
{
    u8 len;
    u8 members;
    u16 idx;
    s32 value;
    char *start;
    char *q;
    char *p;
    char var[0x80];
    char num[0x10];
    char full[0x80];

    len = 0;
    value = 0;
    D_00160f2c = LbFileLengthRnc(name);
    if (D_00160f2c == -1) {
        hosp_dos_printf(D_00120bf9, name);
        return;
    }
    file_buffer = (char *)LbMemoryAlloc(D_00160f2c + 1);
    if (file_buffer == 0) {
        hosp_dos_printf(D_00120c10, D_00160f2c + 1);
        return;
    }
    if (LbFileLoadAt(name, file_buffer) == -1) {
        hosp_dos_printf(D_00120c28, name);
        LbMemoryFree(file_buffer);
        return;
    }
    *(file_buffer + D_00160f2c) = 0xff;
    LbMemorySet(current_line, 0, 0x100);
    read_line_from_buf(0, 0, file_buffer);
    while (read_line_from_buf(current_line, 0x100, file_buffer)) {
        p = strstr(current_line, D_00120c3f);
        if (p) {
            LbMemorySet(var, 0, 0x80);
            LbMemorySet(num, 0, 0x10);
            LbMemorySet(D_00161034, 0, 0x80);
            p++;
            start = p;
            len = 0;
            while (!isspace(*p) && *p) {
                len++;
                p++;
            }
            strncpy(var, start, len);
            idx = array_index(var);
            q = p;
            while (isspace(*q))
                q++;
            strncpy(D_00161034, var, fn_000cd716(var) + 1);
            if (strchr(var, '.')) {
                LbMemorySet(full, 0, 0x80);
                LbMemorySet(D_001610b4, 0, 0x80);
                strtok(D_00161034, D_00120c41);
                p = strchr(var, '.');
                members = number_of_members(var);
                while (members-- > 0) {
                    start = p++;
                    len = 1;
                    while (!isspace(*p) && *p && *p != '.') {
                        len++;
                        p++;
                    }
                    strncpy(D_001610b4, start, len);
                    strncpy(full, D_00161034, fn_000cd716(D_00161034) + 1);
                    fn_000c93bf(full, D_001610b4);
                    start = q;
                    len = 0;
                    while (!isspace(*q) && *q) {
                        len++;
                        q++;
                    }
                    strncpy(num, start, len);
                    num[len] = 0;
                    value = atol(num);
                    if (assign_value(full, value, idx) == -1)
                        read_line_from_buf(0, 1, 0);
                    while (isspace(*q))
                        q++;
                    LbMemorySet(D_001610b4, 0, 0x80);
                    LbMemorySet(full, 0, 0x80);
                    LbMemorySet(num, 0, 0x10);
                }
                LbMemorySet(D_00161034, 0, 0x80);
            } else {
                start = q;
                len = 0;
                while (!isspace(*q) && *q) {
                    len++;
                    q++;
                }
                strncpy(num, start, len);
                num[len] = 0;
                value = atol(num);
                strip_formatting(var);
                if (assign_value(var, value, idx) == -1)
                    read_line_from_buf(0, 1, 0);
            }
        }
        LbMemorySet(current_line, 0, 0x100);
        LbMemorySet(var, 0, 0x80);
        LbMemorySet(num, 0, 0x10);
        LbMemorySet(D_00161034, 0, 0x80);
        LbMemorySet(D_001610b4, 0, 0x80);
        LbMemorySet(full, 0, 0x80);
    }
    LbMemoryFree(file_buffer);
    file_buffer = 0;
}

// FUNCTION: 0x00099a00
u16 array_index(char *name)
{
    u8 len;
    char *start;
    s32 idx;
    char *p;
    char buf[0x80];
    char buf2[0x80];
    char buf3[0x80];
    char num[8];

    len = 0;
    LbMemorySet(buf, 0, 0x80);
    LbMemorySet(buf2, 0, 0x80);
    LbMemorySet(buf3, 0, 0x80);
    LbMemorySet(num, 0, 8);
    strncpy(buf, name, fn_000cd716(name) + 1);
    p = strstr(name, D_00120c43);
    if (p) {
        strtok(buf, D_00120c45);
        strncpy(buf2, name, fn_000cd716(name) + 1);
        p = strstr(buf2, D_00120c47);
        ++p;
        start = p;
        while (*p != ']') {
            p++;
            len++;
        }
        strncpy(num, start, len);
        idx = atoi(num);
        len = 0;
        ++p;
        start = p;
        while (*p) {
            len++;
            p++;
        }
        strncpy(buf3, start, len);
        strncat(buf, buf3, fn_000cd716(buf3) + 1);
        LbDataLoadSetModifyFilenameFunction(name, buf);
        return idx;
    }
    return 0;
}

// FUNCTION: 0x00099b8c
u8 number_of_members(char *name)
{
    u8 n;
    char *p;
    char *last;

    n = 1;
    p = strchr(name, '.');
    last = strrchr(name, '.');
    while (p != last) {
        if (*p == '.')
            n++;
        p++;
    }
    return n;
}

// FUNCTION: 0x00099bf4
void strip_formatting(char *s)
{
    u8 n;
    char *p;

    n = 0;
    p = strstr(current_line, D_00120c49);
    if (p != 0 && (p = strstr(current_line, D_00120c4b)) != 0) {
        for (p = s; *p != '\t' && *p != ' ';) {
            p++;
            n++;
        }
        *p = 0;
    }
}
