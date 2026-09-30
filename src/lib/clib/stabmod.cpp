#include "rtinit.h"

extern "C" {

struct RO_DTREG {
    unsigned kind;
};

struct RW_DTREG {
    RW_DTREG *prev;
    RO_DTREG *ro;
    unsigned state_var;
    void *extra;
};

struct RO_STATE;
struct STAB_TRAVERSE;
struct DTREG_VFPTRS;

struct THREAD_CTL {
    RW_DTREG *registered;
};

extern THREAD_CTL _wint_thread_data;
extern RW_DTREG *D_00151700;
extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];

void __wcpp_2_destruct_all_(void);
#pragma aux __wcpp_2_destruct_all_ "__wcpp_2_destruct_all__"

void __wcpp_2_module_dtor__(void);
#pragma aux __wcpp_2_module_dtor__ "*"

// FUNCTION: 0xcf1d8
void __wcpp_2_module_dtor__(void)
{
    THREAD_CTL *thr;
    RW_DTREG *rw;
    RW_DTREG local;

    thr = &_wint_thread_data;
    for (;;) {
        rw = D_00151700;
        if (rw != 0)
            D_00151700 = rw->prev;
        if (rw == 0)
            break;
        local.prev = thr->registered;
        thr->registered = &local;
        local.ro = rw->ro;
        local.state_var = rw->state_var;
        if (rw->ro->kind == 4)
            local.extra = rw->extra;
        __wcpp_2_destruct_all_();
        rw->prev = 0;
        thr->registered = thr->registered->prev;
    }
}

RO_STATE *fs_state_table(RO_DTREG *ro);
#pragma aux fs_state_table "W?fs_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xcf234
RO_STATE *fs_state_table(RO_DTREG *ro)
{
    return (RO_STATE *)(ro + 1);
}

void *fs_point_object(RW_DTREG *rw, unsigned index);
#pragma aux fs_point_object "W?fs_point_object$n(pn$rw_dtreg$$ui)pnv"

// FUNCTION: 0xcf238
void *fs_point_object(RW_DTREG *rw, unsigned index)
{
    return ((void **)rw->ro)[index * 2];
}

struct STAB_TRAVERSE {
    void *rw;
    unsigned state_var;
    unsigned char obj_type;
};

void fs_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG *rw);
#pragma aux fs_init_traverse "W?fs_init_traverse$n(pn$__1j2o3bSTAB_TRAVERSE$$pn$rw_dtreg$$)v"

// FUNCTION: 0xcf23f
void fs_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG *rw)
{
    ctl->obj_type = 0;
}

struct DTREG_VFPTRS {
    RO_STATE *(*state_table)(RO_DTREG *ro);
    void *(*point_object)(RW_DTREG *rw, unsigned index);
    void (*init_traverse)(STAB_TRAVERSE *ctl, RW_DTREG *rw);
};

static DTREG_VFPTRS fs_vftable = { fs_state_table, fs_point_object, fs_init_traverse };
#pragma aux fs_vftable "W?fs_vftable$n$__6rq0qbDTREG_VFTBL$$"

void fs_init(void);
#pragma aux fs_init "fn_000cf244_"

// FUNCTION: 0xcf244
void fs_init(void)
{
    __wcpp_2_data_dtreg_vfptrs_[3] = &fs_vftable;
}

AXI(fs_init, 10)

void __wcpp_2_mod_register__(RW_DTREG *rw);
#pragma aux __wcpp_2_mod_register__ "__wcpp_2_mod_register__"

// FUNCTION: 0xcf24f
void __wcpp_2_mod_register__(RW_DTREG *rw)
{
    rw->prev = D_00151700;
    D_00151700 = rw;
}

}
