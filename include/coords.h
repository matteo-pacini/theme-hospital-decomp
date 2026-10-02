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

/*
 * Element of the screen button table (screenbuttons), which the code reads as
 * Button.
 */
struct SlistButton {
    // 0x0: Screen x.
    s16 x;
    // 0x2: Screen y.
    s16 y;
    // 0x4: Width.
    s16 width;
    // 0x6: Height.
    s16 height;
    // 0x8: Called with the button's index when clicked.
    void (*callback)(u8 button);
    u8 unknownC[6];
};

struct HelpControl {
    s16 x;
    s16 y;
    s16 x1;
    s16 y1;
    // Help text index in the low word, requester slot (HelpHotspot.req_slot)
    // in the high word.
    s32 text_and_slot;
    void (*callback)(void);
};

/*
 * Element of the zoom-box frame tables (hiresglass, the demo's hiresglass);
 * add_glass_frame reads a list, ended by x1 0, as GlassBox.
 */
struct GlassFrame {
    // 0x0: Left.
    s16 x;
    // 0x2: Top.
    s16 y;
    // 0x4: Right; 0 ends a list.
    s16 x1;
    // 0x6: Bottom.
    s16 y1;
    u8 unknown8[11];
};

#endif
