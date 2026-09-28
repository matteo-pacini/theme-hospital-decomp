#include "hosp.h"

extern "C" {
void fn_000549bc(Thing *w);
}

// NONMATCHING: register choice
// FUNCTION: 0x00054dd4
void start_new_interaction(Thing *w, u8 who)
{
    switch (w->renderer) {
    case 2:
        w->kind = 0x22;
        break;
    case 4:
        w->kind = 0x1a;
        break;
    default:
        return;
    }
    w->unknown99 = statvar.unknowne8[who] - 1;
    w->label[1] = 0;
    fn_000549bc(w);
}
