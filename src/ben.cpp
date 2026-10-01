#include "hosp.h"

extern "C" {

void fn_000c93bf(char *dst, char *src);
void fn_000cd2cc(char *s, ...);
int strcmp(const char *a, const char *b);
void LbDataLoadSetModifyFilenameFunction(char *(*fn)(TbLoadFiles *));
}

extern s32 close_next_time;
#pragma aux close_next_time "W?close_next_time$:.0$:?read_line_from_buf$n(pnalpna)pnanl"
extern char *buf;
#pragma aux buf "W?buf$:.0$:?read_line_from_buf$n(pnalpna)pnanpna"
extern char filename[];
#pragma aux filename "W?filename$:.0$:?modify_data_load_filename$n(pn$TbLoadFiles$$)pnan[]a"
extern u8 install_language;

// FUNCTION: 0x0007a190
char *read_line_from_buf(char *line, s32 max, char *src)
{
    s32 count;

    count = 0;
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
    while ((s8)*buf != '\n' && count < max && (s8)*buf != -1) {
        *line++ = *buf++;
        count++;
    }
    if (count) {
        if ((s8)*buf != -1) {
            line[-1] = 0;
            buf++;
        } else if (count == max) {
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
        fn_000bbff0(filename, cd_install_path);
        fn_000c93bf(filename, (char *)file);
    } else {
        fn_000bbff0(filename, (char *)file);
    }
    check_cd_available(filename);
    fn_000cd2cc(filename);
    return filename;
}

// FUNCTION: 0x0007a318
s32 read_config_file(void)
{
    u32 pos;
    u32 key;
    u32 value;
    u32 i;
    char *data;
    s32 len;

    len = LbFileLengthRnc("hospital.cfg");
    if (len == -1)
        return -1;
    data = (char *)LbMemoryAlloc(len);
    if (data == 0)
        return -1;
    if (LbFileLoadAt("hospital.cfg", data) == -1) {
        LbMemoryFree(data);
        return -1;
    }
    pos = 0;
    for (i = 0; i < 4; i++) {
        if (len - 1 <= pos)
            break;
        key = value = pos;
        while ((s8)data[pos++] != '\n')
            ;
        data[pos - 2] = 0;
        data[pos - 1] = 0;
        while (data[value] != '=' && (s8)data[value] != 0)
            value++;
        if (data[value] != '=')
            return -1;
        data[value++] = 0;
        if (!strcmp(data + key, "INSTALL_PATH")) {
            fn_000bbff0(cd_install_path, data + value);
            LbDataLoadSetModifyFilenameFunction(modify_data_load_filename);
        } else if (!strcmp(data + key, "INSTALL_TYPE")) {
            switch (tolower((s8)data[value + 1])) {
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
        } else if (!strcmp(data + key, "LANGUAGE")) {
            install_language = 0;
            switch (tolower((s8)data[value])) {
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
                if (tolower((s8)data[value + 1]) == 'p')
                    install_language = 4;
                else
                    install_language = 5;
                break;
            }
            myplayer.f21 = install_language;
        } else if (!strcmp(data + key, "SAVE_OVERRIDE")) {
            fn_000bbff0(statvar.f575, data + value);
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
        case KC_Y:
            return 0x2c;
        case KC_Z:
            return 0x15;
        default:
            return key;
        }
    } else if (install_language == 1) {
        switch (key) {
        case KC_Q:
            return 0x1e;
        case KC_A:
            return 0x10;
        case KC_W:
            return 0x2c;
        case KC_Z:
            return 0x11;
        case KC_M:
            return 0x33;
        case KC_COMMA:
            return 0x27;
        case KC_SEMICOLON:
            return 0x32;
        default:
            return key;
        }
    }
    return key;
}
