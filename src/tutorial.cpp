#include "hosp.h"

extern "C" {
extern u8 D_0015832d;
extern char D_0015820c[];
extern u8 D_00158320;
extern u8 D_0015831b;
extern u16 D_0015831c;
extern u8 D_0015831e;
extern u8 D_0015831f;
extern TbScreenMode D_0015f3f5;
extern s16 D_001581fe;
extern u8 *D_0015f402;
extern u8 D_001553d0[];
extern Event D_001549d0[];
extern Event D_00154a0c[];
extern char D_0015544e[];
extern u8 D_00161200;
extern u8 D_00161201;
extern u8 D_00161202;
extern u8 D_00161203;
extern u8 D_00161204;
extern u8 D_00161205;
extern u8 D_00161206;
extern u8 D_00161207;
extern u8 D_00161208;
extern u8 D_00161209;
extern u8 D_0016120a;
extern u8 D_0016120b;
extern u8 D_0016120c;
extern u8 D_0016120d;
extern u8 D_0016120e;
extern u8 D_0016120f;
extern u8 D_00161210;
extern u8 D_00161211;
extern u8 D_00161212;
extern u8 D_00161213;
extern u8 D_00161214;
extern u8 D_00161215;
extern u8 D_00161216;
extern u8 D_00161217;
extern u8 D_00161218;
extern u8 D_00161219;
extern u8 D_0016121a;
extern u8 D_0016121b;
extern u8 D_0016121c;
extern u8 D_0016121d;
}

void set_force_help(u8 kind, u16 text);
void clear_force_help(void);

// FUNCTION: 0x000a4830
s8 tutorial(void)
{
    switch (D_0015832d) {
    case 0x1d:
        if (D_00161200 != 2) {
            if (D_00154a0c[player].type != 0x62) {
                if (D_00161200 == 0) {
                    sprintf(D_0015820c, advisor_text[29]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0x13;
                    D_00161200 = 1;
                    return 1;
                }
            } else {
                D_00161200 = 2;
                D_0015832d = 0x28;
                D_0015831c = 0;
                clear_force_help();
            }
        }
        break;
    case 0x28:
        if (D_0016120b != 2) {
            if (D_001549d0[player].type == 0x64) {
                D_00161200 = 0;
                D_0016120b = 0;
                D_0015832d = 0x1d;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2b) {
                D_00161200 = 0;
                D_0016120b = 0;
                D_0015832d = 0x1d;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x25 || D_00154a0c[player].value != 0xb) {
                if (D_0016120b == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[40]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0xa;
                    set_force_help(5, 0x10b);
                    D_0016120b = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120b = 2;
                D_0015832d = 0x2a;
                clear_force_help();
                D_0015831c = 0;
            }
        }
        break;
    case 0x2a:
        if (D_0016120d != 2) {
            if (D_001549d0[player].type == 0x64) {
                D_00161200 = 0;
                D_0016120b = 0;
                D_0016120d = 0;
                D_0015832d = 0x1d;
                clear_force_help();
                D_0015831c = 0;
            } else if ((D_00154a0c[player].type == 0x63 && D_00154a0c[player].value == 0xb) && gamevar.unknown33.unknown0[0xb] == 0) {
                D_0016120d = 0;
                D_0016120b = 0;
                D_0015832d = 0x28;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x2b) {
                if (D_0016120d == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[42]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 9;
                    set_force_help(5, 0x27);
                    D_0016120d = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120d = 2;
                D_0015832d = 0x29;
                D_0015831c = 0;
                clear_force_help();
            }
        }
        break;
    case 0x29:
        if (D_0016120c != 2) {
            if (D_001549d0[player].type == 0x38) {
                D_0016120c = 0;
                D_0016120d = 0;
                D_0016120b = 0;
                D_00161200 = 0;
                D_0015832d = 0x1d;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_0015831b & 2) {
                sprintf(D_0015820c, advisor_text[46]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_0016120c = 0;
                return 1;
            } else if (!listhead[player].rooms[4]) {
                if (D_0016120c == 0) {
                    sprintf(D_0015820c, advisor_text[41]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0016120c = 1;
                    return 1;
                }
            } else {
                D_0016120c = 2;
                D_0015832d = 0x1e;
                D_0015831c = 0;
            }
        }
        break;
    case 0x1e:
        if (D_00161201 != 2 && !listhead[player].staff[0]) {
            if (D_00154a0c[player].type != 0x2e) {
                if (D_00161201 == 0) {
                    sprintf(D_0015820c, advisor_text[30]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0x15;
                    D_00161201 = 1;
                    return 1;
                }
            } else {
                D_00161201 = 2;
                D_0015832d = 0x21;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_00161201 = 2;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x21:
        if (D_00161204 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value != 3)) {
                D_00161201 = 0;
                D_00161204 = 0;
                D_0015832d = 0x1e;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                if (D_0015f3f5 == SCREEN_MODE_13) {
                    sprintf(D_0015820c, advisor_text[33]);
                    D_00158320 = 1;
                }
                D_0015831e = 0;
                D_0015831f = 0;
                D_0015831c = 3;
                set_force_help(4, 0x19);
                return 1;
            } else if (D_00154a0c[player].type != 0x2f || D_00154a0c[player].value != 3) {
                if (D_00161204 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[33]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 3;
                    set_force_help(4, 0x19);
                    D_00161204 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161204 = 2;
                D_0015832d = 0x25;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_0015832d = 0x25;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x25:
        if (D_00161208 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value != 3)) {
                D_00161208 = 2;
                D_00161209 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                D_0015832d = 0x1e;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value == 3) {
                D_00161208 = 2;
                D_00161209 = 2;
                D_0015832d = 0x24;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161208 = 2;
                D_00161209 = 2;
                D_00161204 = 0;
                D_0015832d = 0x21;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x32 || D_00154a0c[player].value != 1) {
                if (D_00161208 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[37]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 7;
                    set_force_help(4, 0x1b);
                    D_00161208 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161208 = 2;
                D_0015832d = 0x26;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_0015832d = 0x26;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x26:
        if (D_00161209 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value != 3)) {
                D_00161209 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                D_0015832d = 0x1e;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value == 3) {
                D_00161209 = 2;
                D_0015832d = 0x24;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161209 = 2;
                D_00161204 = 0;
                D_0015832d = 0x21;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x32 || D_00154a0c[player].value != 0) {
                if (D_00161209 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[38]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 6;
                    set_force_help(4, 0x1a);
                    D_00161209 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161209 = 2;
                D_0015832d = 0x22;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_0015832d = 0x22;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x22:
        if (D_00161205 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161205 = 0;
                D_00161201 = 0;
                D_00161204 = 0;
                D_0015832d = 0x1e;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 3) {
                D_00161204 = 0;
                D_0015832d = 0x21;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x30) {
                if (D_00161205 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[34]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 5;
                    set_force_help(4, 0x1c);
                    D_00161205 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161205 = 2;
                D_0015832d = 0x24;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_0015832d = 0x24;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x24:
        if (D_00161207 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161207 = 2;
                D_00161201 = 0;
                D_00161204 = 0;
                D_0015832d = 0x1e;
                clear_force_help();
                D_0015831c = 0;
            } else if (!gamevar.f72) {
                sprintf(D_0015820c, advisor_text[60]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_00161207 = 0;
                return 1;
            } else if (!listhead[player].staff[0]) {
                if (D_00161207 == 0) {
                    sprintf(D_0015820c, advisor_text[36]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_00161207 = 1;
                    D_0015831c = 0;
                    return 1;
                }
            } else {
                D_00161207 = 2;
                D_0015832d = 0x20;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x20:
        if (D_00161203 != 2 && !listhead[player].rooms[7]) {
            if (D_00154a0c[player].type != 0x37) {
                if (D_00161203 == 0) {
                    sprintf(D_0015820c, advisor_text[32]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0x12;
                    set_force_help(3, 0xb);
                    D_00161203 = 1;
                    return 1;
                }
            } else {
                D_00161203 = 2;
                D_0015832d = 0x2b;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_00161203 = 2;
            D_0015832d = 0x1f;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x2b:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_0016120e != 2) {
            if (D_00154a0c[player].type != 0x20 || D_00154a0c[player].value != 3) {
                if (D_0016120e == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[43]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 8;
                    set_force_help(2, 1);
                    D_0016120e = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120e = 2;
                D_0015832d = 0x2c;
                clear_force_help();
                D_0015831c = 0;
            }
        }
        break;
    case 0x2c:
        if (D_00154a0c[player].type == 0x26) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_0016120f != 2) {
            if (D_00154a0c[player].type == 0x35 && D_00154a0c[player].value != 7) {
                D_00161203 = 0;
                D_0016120e = 0;
                D_0016120f = 0;
                D_0015832d = 0x20;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x35) {
                if (D_0016120f == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[44]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0xb;
                    LbDataLoadSetModifyFilenameFunction(D_0015544e, room_help[7]);
                    set_force_help(2, 0xed);
                    D_0016120e = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_0016120f = 2;
                D_0015832d = 0x39;
                clear_force_help();
                D_0015831c = 0;
            }
        }
        break;
    case 0x39:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_00154a0c[player].type != 2) {
            sprintf(D_0015820c, advisor_text[47]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0016121c = 0;
            return 1;
        } else {
            D_0016121c = 2;
            D_0015832d = 0x2f;
            D_0015831c = 0;
        }
        break;
    case 0x2f:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        } else if ((D_0015831b & 4) && (D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[61]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0016121b = 0;
            D_0015831b &= ~4;
            D_0015831b &= ~2;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else if (!(D_0015831b & 4) && !(D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[52]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0015831b &= ~4;
            D_001581fe = 0;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else if (D_001581fe == 4 || (D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[51]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_00161212 = 0;
            D_0015831b &= ~2;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else {
            D_00161212 = 2;
            D_0015832d = 0x38;
            D_0015831c = 0;
            D_0015831b &= ~2;
            D_0015831b &= ~4;
            clear_force_help();
        }
        break;
    case 0x38:
        if (D_00154a0c[player].type == 0x38) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            D_0016121d = 0;
            D_0015832d = 0x20;
            clear_force_help();
            D_0015831c = 0;
        } else if ((D_0015831b & 4) && (D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[61]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0016121b = 0;
            D_0015831b &= ~4;
            D_0015831b &= ~2;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else if ((D_0015831b & 4) && !(D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[52]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0016121b = 0;
            D_0015831b &= ~4;
            D_0015831b &= ~2;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else if (D_001581fe == 4 || (D_0015831b & 2)) {
            sprintf(D_0015820c, advisor_text[51]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0016121b = 0;
            D_0015831b &= ~2;
            D_001581fe = 0;
            D_0015831c = 0;
            clear_force_help();
            return 1;
        } else if (D_00154a0c[player].type != 0x12) {
            if (D_0016121b == 0) {
                sprintf(D_0015820c, advisor_text[56]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_0015831c = D_0015f402[0x52];
                set_force_help(2, 0x86);
                D_0016121b = 1;
                D_0015831b &= ~2;
                return 1;
            }
        } else {
            D_0016121b = 2;
            D_0015832d = 0x30;
            D_0015831c = 0;
            clear_force_help();
        }
        break;
    case 0x30:
        if (D_00154a0c[player].type == 0x45) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_0015832d = 0x38;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_00154a0c[player].type == 3 && D_001553d0[player] == 1) {
            sprintf(D_0015820c, advisor_text[53]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_00161218 = 1;
            return 1;
        } else if (D_00154a0c[player].type != 3) {
            if (D_00161213 == 0) {
                sprintf(D_0015820c, advisor_text[48]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_00161213 = 1;
                return 1;
            }
        } else {
            D_00161213 = 2;
            D_0015832d = 0x31;
            D_0015831c = 0;
            clear_force_help();
        }
        break;
    case 0x31:
        if (D_00154a0c[player].type == 0x45) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_0015832d = 0x38;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_00154a0c[player].type == 3 && D_001553d0[player] == 1) {
            sprintf(D_0015820c, advisor_text[54]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_00161219 = 1;
            return 1;
        } else if (D_00154a0c[player].type == 3 && D_001553d0[player] == 0) {
            sprintf(D_0015820c, advisor_text[49]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_00161214 = 1;
            return 1;
        } else if (D_00154a0c[player].type != 4) {
            if (D_00161214 == 0) {
                sprintf(D_0015820c, advisor_text[49]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_0015831c = D_0015f402[0x52];
                set_force_help(0xd, 0x86);
                D_00161214 = 1;
                return 1;
            }
        } else {
            D_00161213 = 2;
            D_0015832d = 0x32;
            D_0015831c = 0;
            clear_force_help();
            D_0015831b &= ~2;
        }
        break;
    case 0x32:
        if (D_00154a0c[player].type == 0x45) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            D_0015832d = 0x38;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_0015831b & 2) {
            sprintf(D_0015820c, advisor_text[55]);
            D_00158320 = 1;
            D_0015831e = 0;
            D_0015831f = 0;
            D_0015831b &= ~2;
            D_00161215 = 0;
            return 1;
        } else if (!(D_0015831b & 0x10)) {
            if (D_00161215 == 0) {
                sprintf(D_0015820c, advisor_text[50]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_00161215 = 1;
                return 1;
            }
        } else {
            D_0016121d = 0;
            D_0015832d = 0x3a;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x3a:
        if (D_00154a0c[player].type == 0x45) {
            D_00161203 = 0;
            D_0016120e = 0;
            D_0016120f = 0;
            D_0016121c = 0;
            D_00161212 = 0;
            D_0016121b = 0;
            D_00161213 = 0;
            D_00161214 = 0;
            D_00161215 = 0;
            D_0016121d = 0;
            D_0015832d = 0x38;
            clear_force_help();
            D_0015831c = 0;
        } else if (D_00154a0c[player].type != 1) {
            if (D_0016121d == 0) {
                sprintf(D_0015820c, advisor_text[58]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_0015831c = D_0015f402[0x52];
                set_force_help(0xd, 0x86);
                D_0016121d = 1;
                return 1;
            }
        } else {
            D_0016121d = 2;
            D_0015832d = 0x1f;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x1f:
        if (D_00161202 != 2 && !listhead[player].staff[4]) {
            if (D_00154a0c[player].type != 0x2e) {
                if (D_00161202 == 0) {
                    sprintf(D_0015820c, advisor_text[31]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 0x15;
                    D_00161202 = 1;
                    return 1;
                }
            } else {
                D_00161202 = 2;
                D_0015832d = 0x23;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_00161202 = 2;
            clear_force_help();
            D_0015831c = 0;
            D_0015831b &= ~1;
        }
        break;
    case 0x23:
        if (D_00161206 != 2) {
            if (D_00154a0c[player].type == 0x38 || (D_00154a0c[player].type == 0x30 && D_00154a0c[player].value != 1)) {
                D_00161202 = 0;
                D_00161206 = 0;
                D_0015832d = 0x1f;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x2f || D_00154a0c[player].value != 1) {
                if (D_00161206 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[35]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 4;
                    D_00161206 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161206 = 2;
                D_0015832d = 0x2d;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_00161206 = 2;
            D_0015832d = 0x2d;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x2d:
        if (D_00161210 != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_00161202 = 0;
                D_00161206 = 0;
                D_00161210 = 0;
                D_0015832d = 0x1f;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type == 0x2f && D_00154a0c[player].value != 1) {
                D_00161206 = 0;
                D_0015832d = 0x23;
                clear_force_help();
                D_0015831c = 0;
            } else if (D_00154a0c[player].type != 0x30) {
                if (D_00161210 == 0) {
                    if (D_0015f3f5 == SCREEN_MODE_13) {
                        sprintf(D_0015820c, advisor_text[45]);
                        D_00158320 = 1;
                    }
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0015831c = 5;
                    D_00161210 = 1;
                    if (D_0015f3f5 == SCREEN_MODE_13)
                        return 1;
                    LbMemorySet(D_0015820c, 0, 0x10e);
                    return 0;
                }
            } else {
                D_00161210 = 2;
                D_0015832d = 0x27;
                clear_force_help();
                D_0015831c = 0;
            }
        } else {
            D_00161210 = 2;
            D_0015832d = 0x27;
            clear_force_help();
            D_0015831c = 0;
        }
        break;
    case 0x27:
        if (D_0016120a != 2) {
            if (D_00154a0c[player].type == 0x38) {
                D_0016120a = 0;
                D_00161202 = 0;
                D_00161206 = 0;
                D_00161210 = 0;
                D_0015832d = 0x1f;
                clear_force_help();
                D_0015831c = 0;
            } else if (!gamevar.f72) {
                sprintf(D_0015820c, advisor_text[59]);
                D_00158320 = 1;
                D_0015831e = 0;
                D_0015831f = 0;
                D_0016120a = 0;
                return 1;
            } else if (!listhead[player].staff[4]) {
                if (D_0016120a == 0) {
                    sprintf(D_0015820c, advisor_text[39]);
                    D_00158320 = 1;
                    D_0015831e = 0;
                    D_0015831f = 0;
                    D_0016120a = 1;
                    return 1;
                }
            } else {
                D_0016120a = 2;
                D_0015831c = 0;
                D_0015831b &= ~1;
                show_help_page(1, 0xf);
            }
        } else {
            D_0016120a = 2;
            D_0015831c = 0;
            D_0015831b &= ~1;
        }
        break;
    case 0x2e:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    default:
        break;
    }
    return -1;
}
