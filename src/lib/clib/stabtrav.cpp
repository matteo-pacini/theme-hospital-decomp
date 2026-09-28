extern "C" {

struct DTOR_CMD {
    unsigned short code;
};

struct DTOR_CMD_SET_SV {
    unsigned short code;
    unsigned state_var;
};

struct DTOR_CMD_TEST_FLAG {
    unsigned short code;
    unsigned index;
    unsigned state_var_init;
    unsigned state_var_noinit;
};

struct RO_STATE {
    void (*dtor)(void *, unsigned);
    DTOR_CMD *cmd;
};

struct RO_DTREG {
    unsigned kind;
};

struct rw_dtreg {
    rw_dtreg *prev;
    RO_DTREG *ro;
    unsigned state_var;
    unsigned char flags[1];
};

struct STAB_TRAVERSE {
    rw_dtreg *rw;
    unsigned state_var;
    unsigned char obj_type;
    unsigned bound;
};

struct DTREG_VFPTRS {
    RO_STATE *(*state_table)(RO_DTREG *ro);
    void *(*get_object)(rw_dtreg *rw, unsigned state_var);
    void (*trav_init)(STAB_TRAVERSE *ctl);
};

extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];

void __wcpp_2_corrupted_stack_(void);

}

char testFlag(unsigned char *flags, unsigned index);
/* config/functions.txt names the flag test after an unrelated demo function. */
#pragma aux testFlag "W?arr_state_table$n(pn$ro_dtreg$$)pn$__7a4u3RO_STATE$$"

extern "C" {

// FUNCTION: 0xf3cb0
RO_STATE *__wcpp_2_stab_entry_(RO_DTREG *ro, unsigned state_var)
{
    RO_STATE *state;

    if (state_var == 0) {
        state = 0;
    } else {
        state = __wcpp_2_data_dtreg_vfptrs_[ro->kind]->state_table(ro);
        if (ro->kind != 2)
            state = &state[state_var - 1];
    }
    return state;
}

// FUNCTION: 0xf3cde
// NONMATCHING: register allocation (the target walks rw->ro in ebx)
void __wcpp_2_stab_trav_init_(STAB_TRAVERSE *ctl, rw_dtreg *rw)
{
    ctl->rw = rw;
    ctl->state_var = rw->state_var;
    ctl->bound = 0;
    __wcpp_2_data_dtreg_vfptrs_[rw->ro->kind]->trav_init(ctl);
}

// FUNCTION: 0xf3cff
void __wcpp_2_stab_trav_comp_(STAB_TRAVERSE *ctl)
{
    ctl->state_var -= 2;
}

// FUNCTION: 0xf3d04
RO_STATE *__wcpp_2_stab_trav_next_(STAB_TRAVERSE *ctl)
{
    rw_dtreg *rw;
    RO_STATE *state;
    DTOR_CMD *cmd;
    unsigned state_var;

    rw = ctl->rw;
    if (rw == 0) {
        state = 0;
    } else {
        state_var = ctl->state_var;
        if (state_var > 0)
            --state_var;
        for (;;) {
            if (state_var == 0) {
                state = 0;
                break;
            }
            if (state_var == ctl->bound)
                break;
            state = __wcpp_2_stab_entry_(rw->ro, state_var);
            if (state->dtor != 0)
                break;
            cmd = state->cmd;
            switch (cmd->code) {
            case 1:
                state_var = ((DTOR_CMD_SET_SV *)cmd)->state_var;
                continue;
            case 5:
                if (testFlag(rw->flags, ((DTOR_CMD_TEST_FLAG *)cmd)->index)) {
                    state_var = ((DTOR_CMD_TEST_FLAG *)cmd)->state_var_init;
                } else {
                    state_var = ((DTOR_CMD_TEST_FLAG *)cmd)->state_var_noinit;
                }
                continue;
            }
            break;
        }
        ctl->state_var = state_var;
    }
    return state;
}

// FUNCTION: 0xf3d89
// NONMATCHING: the original shares __wcpp_2_stab_trav_next__'s epilogue and
// addresses rw->ro through rw->flags
RO_STATE *__wcpp_2_stab_trav_move_(STAB_TRAVERSE *ctl)
{
    rw_dtreg *rw;
    RO_STATE *state;
    DTOR_CMD *cmd;
    unsigned state_var;

    rw = ctl->rw;
    state_var = ctl->state_var;
    for (;;) {
        state = __wcpp_2_stab_entry_(rw->ro, state_var);
        if (state == 0)
            break;
        if (ctl->state_var == ctl->bound)
            break;
        if (state->dtor != 0)
            break;
        cmd = state->cmd;
        switch (cmd->code) {
        case 2:
        case 3:
        case 4:
            break;
        case 5:
            if (testFlag(rw->flags, ((DTOR_CMD_TEST_FLAG *)cmd)->index)) {
                state_var = ((DTOR_CMD_TEST_FLAG *)cmd)->state_var_init;
            } else {
                state_var = ((DTOR_CMD_TEST_FLAG *)cmd)->state_var_noinit;
            }
            continue;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            break;
        case 1:
            state_var = ((DTOR_CMD_SET_SV *)cmd)->state_var;
            continue;
        default:
            __wcpp_2_corrupted_stack_();
            break;
        }
        break;
    }
    ctl->state_var = state_var;
    return state;
}

}
