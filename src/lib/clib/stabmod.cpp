extern "C" {

struct RW_DTREG {
    RW_DTREG *prev;
};

extern RW_DTREG *D_00151700;

/* config/functions.txt has __wcpp_2_mod_register__ under a wrong demo name. */
void __wcpp_2_mod_register__(RW_DTREG *rw);
#pragma aux __wcpp_2_mod_register__ "W?fs_init_traverse$n(pn$__1j2o3bSTAB_TRAVERSE$$pn$rw_dtreg$$)v"

// FUNCTION: 0xcf24f
void __wcpp_2_mod_register__(RW_DTREG *rw)
{
    rw->prev = D_00151700;
    D_00151700 = rw;
}

}
