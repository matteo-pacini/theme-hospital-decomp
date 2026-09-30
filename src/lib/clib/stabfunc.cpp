#include "rtinit.h"

extern "C" {

struct RO_DTREG {
    unsigned kind;
};

struct RW_DTREG {
    RW_DTREG *prev;
    RO_DTREG *ro;
    unsigned state_var;
};

struct RO_STATE;
struct DTREG_VFPTRS;

struct STAB_TRAVERSE {
    void *rw;
    unsigned state_var;
    unsigned char obj_type;
};

extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];
extern DTREG_VFPTRS fun_vftable;
#pragma aux fun_vftable "W?fun_vftable$n$__6rq0qbDTREG_VFTBL$$"

void *__wcpp_2_base_register_(RW_DTREG *rw, void *ro, unsigned state_var);

RO_STATE *fun_state_table(RO_DTREG *ro);
#pragma aux fun_state_table "W?fun_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xcf3e4
RO_STATE *fun_state_table(RO_DTREG *ro)
{
    return (RO_STATE *)(ro + 1);
}

void *fun_point_object(RW_DTREG *rw, unsigned index);
#pragma aux fun_point_object "W?fun_point_object$n(pn$rw_dtreg$$ui)pnv"

// FUNCTION: 0xcf3e8
void *fun_point_object(RW_DTREG *rw, unsigned index)
{
    int *ro = (int *)rw->ro;

    return (char *)rw + ro[index * 2];
}

void fun_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG *rw);
#pragma aux fun_init_traverse "W?fun_init_traverse$n(pn$__1j2o3bSTAB_TRAVERSE$$pn$rw_dtreg$$)v"

// FUNCTION: 0xcf3f5
void fun_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG *rw)
{
    ctl->obj_type = 0;
}

void init(void);
#pragma aux init "W?init$n()v"

// FUNCTION: 0xcf3fa
void init(void)
{
    __wcpp_2_data_dtreg_vfptrs_[0] = &fun_vftable;
}

AXI(init, 10)

// FUNCTION: 0xcf405
void __wcpp_2_fun_register_(RW_DTREG *rw, void *ro)
{
    __wcpp_2_base_register_(rw, ro, 0);
}

}
