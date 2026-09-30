#ifndef COORDS_H
#define COORDS_H

#include "hosp.h"

/*
 * Element types of the screen layout tables (coords.cpp, scoords.cpp,
 * lcoords.cpp), named as in the demo's debug info. The code reads these
 * tables through D_XXXXXXXX declarations of its own types; some sources
 * (glass.cpp, loadscr.cpp, network.cpp) still define their own, different,
 * SlistButton and GlassFrame.
 */

struct BoxCoord {
    s32 x;
    s32 y;
    s32 w;
    s32 h;
};

struct SlistButton {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    void (*callback)(u8 button);
    u8 unknownC[6];
};

struct HelpControl {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    s32 text;
    void (*callback)(void);
};

struct GlassFrame {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    u8 unknown8[11];
};

#endif
