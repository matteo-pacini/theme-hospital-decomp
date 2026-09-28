#include "hosp.h"

extern "C" {
extern char D_00120608[];
extern char D_00120615[];
extern char D_00120622[];
extern char D_0012062f[];
extern char D_0012063c[];
extern char D_00120645[];
extern u8 D_001547e5;
extern char D_001563c5[];

void fn_000c93bf(char *dst, char *src);
void fn_000cd2cc(char *s, ...);
int strcmp(const char *a, const char *b);
void ___GETDSEnd(char *(*fn)(TbLoadFiles *));
}

extern s32 close_next_time;
#pragma aux close_next_time "W?close_next_time$:.0$:?read_line_from_buf$n(pnalpna)pnanl"
extern char *buf;
#pragma aux buf "W?buf$:.0$:?read_line_from_buf$n(pnalpna)pnanpna"
extern char filename[];
#pragma aux filename "W?filename$:.0$:?modify_data_load_filename$n(pn$TbLoadFiles$$)pnan[]a"
extern u8 install_language;

// NONMATCHING: register choice
// FUNCTION: 0x0007a190
char *read_line_from_buf(char *line, s32 max, char *src)
{
    s32 n;

    n = 0;
    if (close_next_time) {
        close_next_time = 0;
        if (line)
            return 0;
    }
    if (!line) {
        if (max)
            close_next_time = 1;
        else
            buf = src;
        return 0;
    }
    *line = 0;
    while ((s8)*buf != '\n' && n < max && (s8)*buf != -1) {
        *line++ = *buf++;
        n++;
    }
    if (n) {
        if ((s8)*buf != -1) {
            line[-1] = 0;
            buf++;
        } else if (n == max) {
            *line = 0;
        } else {
            line[-1] = 0;
        }
        return buf;
    }
    return 0;
}

// FUNCTION: 0x0007a29c
char *modify_data_load_filename(TbLoadFiles *file)
{
    if (*(u16 *)&file->unknown[0x2a] > install_type) {
        LbDataLoadSetModifyFilenameFunction(filename, cd_install_path);
        fn_000c93bf(filename, (char *)file);
    } else {
        LbDataLoadSetModifyFilenameFunction(filename, (char *)file);
    }
    check_cd_available(filename);
    fn_000cd2cc(filename);
    return filename;
}

// NONMATCHING: register choice
// FUNCTION: 0x0007a318
s32 read_config_file(void)
{
    u32 p;
    u32 key;
    u32 j;
    u32 i;
    char *data;
    s32 len;

    len = LbFileLengthRnc(D_00120608);
    if (len == -1)
        return -1;
    data = (char *)LbMemoryAlloc(len);
    if (data == 0)
        return -1;
    if (LbFileLoadAt(D_00120615, data) == -1) {
        LbMemoryFree(data);
        return -1;
    }
    p = 0;
    for (i = 0; i < 4; i++) {
        if (len - 1 <= p)
            break;
        key = j = p;
        while ((s8)data[p++] != '\n')
            ;
        data[p - 2] = 0;
        data[p - 1] = 0;
        while (data[j] != '=' && (s8)data[j] != 0)
            j++;
        if (data[j] != '=')
            return -1;
        data[j++] = 0;
        if (!strcmp(data + key, D_00120622)) {
            LbDataLoadSetModifyFilenameFunction(cd_install_path, data + j);
            ___GETDSEnd(modify_data_load_filename);
        } else if (!strcmp(data + key, D_0012062f)) {
            switch (tolower((s8)data[j + 1])) {
            case 'i':
                install_type = 1;
                break;
            case 'e':
                install_type = 2;
                break;
            case 'a':
                install_type = 3;
                break;
            }
        } else if (!strcmp(data + key, D_0012063c)) {
            install_language = 0;
            switch (tolower((s8)data[j])) {
            case 'e':
                install_language = 0;
                break;
            case 'f':
                install_language = 1;
                break;
            case 'g':
                install_language = 2;
                break;
            case 'i':
                install_language = 3;
                break;
            case 's':
                if (tolower((s8)data[j + 1]) == 'p')
                    install_language = 4;
                else
                    install_language = 5;
                break;
            }
            D_001547e5 = install_language;
        } else if (!strcmp(data + key, D_00120645)) {
            LbDataLoadSetModifyFilenameFunction(D_001563c5, data + j);
        } else {
            LbMemoryFree(data);
            return -1;
        }
    }
    LbMemoryFree(data);
    return 1;
}

// FUNCTION: 0x0007a5e8
u8 what_key_is_it_really(u8 key)
{
    if (install_language == 2) {
        switch (key) {
        case 0x15:
            return 0x2c;
        case 0x2c:
            return 0x15;
        default:
            return key;
        }
    } else if (install_language == 1) {
        switch (key) {
        case 0x10:
            return 0x1e;
        case 0x1e:
            return 0x10;
        case 0x11:
            return 0x2c;
        case 0x2c:
            return 0x11;
        case 0x32:
            return 0x33;
        case 0x33:
            return 0x27;
        case 0x27:
            return 0x32;
        default:
            return key;
        }
    }
    return key;
}
