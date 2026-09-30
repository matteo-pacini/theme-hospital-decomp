#include "rtinit.h"

extern "C" {

struct RT_TYPE_SIG;

#pragma pack(1)
struct RT_ARRAY_SIG {
    unsigned char type;
    RT_TYPE_SIG *base;
    unsigned size;
    unsigned count;
};
#pragma pack()

struct RO_DTREG_ARRAY {
    unsigned kind;
    RT_ARRAY_SIG *sig;
    unsigned count;
    unsigned state;
};

struct RW_DTREG_ARRAY {
    RW_DTREG_ARRAY *prev;
    RO_DTREG_ARRAY *ro;
    unsigned state_var;
    void *array;
    RO_DTREG_ARRAY ro_arr;
};

struct STAB_TRAVERSE {
    void *rw;
    unsigned state_var;
    unsigned char obj_type;
    unsigned bound;
};

struct DTREG_VFPTRS;

extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];

void *__wcpp_2_base_register_(RW_DTREG_ARRAY *rw, RO_DTREG_ARRAY *ro, unsigned state_var);

struct RO_STATE;

unsigned __wcpp_2_ts_size_(RT_TYPE_SIG *sig);

RO_STATE *arr_state_table(RO_DTREG_ARRAY *ro);
#pragma aux arr_state_table "W?arr_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xf5190
RO_STATE *arr_state_table(RO_DTREG_ARRAY *ro)
{
    return (RO_STATE *)&ro->count;
}

void *arr_get_object(RW_DTREG_ARRAY *rw, unsigned state_var);
/* config/functions.txt has no name for arr_get_object. */
#pragma aux arr_get_object "fn_000f5194_"

// FUNCTION: 0xf5194
void *arr_get_object(RW_DTREG_ARRAY *rw, unsigned state_var)
{
    return (char *)rw->array + (state_var - 1) * __wcpp_2_ts_size_((RT_TYPE_SIG *)rw->ro->sig);
}

void arr_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG_ARRAY *rw);
#pragma aux arr_init_traverse "W?arr_init_traverse$n(pn$__1j2o3bSTAB_TRAVERSE$$pn$rw_dtreg$$)v"

// FUNCTION: 0xf51ad
void arr_init_traverse(STAB_TRAVERSE *ctl, RW_DTREG_ARRAY *rw)
{
    ctl->obj_type = 0;
}

struct DTREG_VFPTRS {
    RO_STATE *(*state_table)(RO_DTREG_ARRAY *ro);
    void *(*get_object)(RW_DTREG_ARRAY *rw, unsigned state_var);
    void (*init_traverse)(STAB_TRAVERSE *ctl, RW_DTREG_ARRAY *rw);
};

static DTREG_VFPTRS arr_vftable = { arr_state_table, arr_get_object, arr_init_traverse };
#pragma aux arr_vftable "W?arr_vftable$n$__6rq0qbDTREG_VFTBL$$"

// FUNCTION: 0xf51b2
void fn_000f51b2(void)
{
    __wcpp_2_data_dtreg_vfptrs_[2] = &arr_vftable;
}

AXI(fn_000f51b2, 10)

// FUNCTION: 0xf51bd
void __wcpp_2_arr_register_(void *array, RW_DTREG_ARRAY *rw, RT_ARRAY_SIG *sig)
{
    rw->ro_arr.kind = 2;
    rw->ro_arr.sig = sig;
    rw->ro_arr.count = sig->count;
    rw->ro_arr.state = 0;
    rw->array = array;
    rw->ro = &rw->ro_arr;
    __wcpp_2_base_register_(rw, &rw->ro_arr, 0);
}

}
