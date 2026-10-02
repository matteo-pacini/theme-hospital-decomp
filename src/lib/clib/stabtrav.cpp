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
    void (*trav_init)(STAB_TRAVERSE *ctl, rw_dtreg *rw);
};

extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];

void __wcpp_2_corrupted_stack_(void);

}

char testFlag(unsigned char *flags, unsigned index);
#pragma aux testFlag "__wcpp_2_bit_test__"

extern "C" {

// __wcpp_2_stab_entry_: state-table entry of registration ro for state_var (NULL for state 0; array registrations have one entry).
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

// __wcpp_2_stab_trav_init_: start a traversal ctl of rw's state table at its current state.
// FUNCTION: 0xf3cde
void __wcpp_2_stab_trav_init_(STAB_TRAVERSE *ctl, rw_dtreg *rw)
{
    ctl->rw = rw;
    ctl->state_var = rw->state_var;
    ctl->bound = 0;
    __wcpp_2_data_dtreg_vfptrs_[rw->ro->kind]->trav_init(ctl, rw);
}

// __wcpp_2_stab_trav_comp_: step ctl's state back by 2 (past the object being constructed).
// FUNCTION: 0xf3cff
void __wcpp_2_stab_trav_comp_(STAB_TRAVERSE *ctl)
{
    ctl->state_var -= 2;
}

// __wcpp_2_stab_trav_next_: step ctl to the previous state, following set-state and test-flag commands; returns that entry (NULL at state 0).
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

// __wcpp_2_stab_trav_move_: from ctl's state, follow set-state and test-flag commands to an entry with a destructor or other command, storing the state reached; returns the entry.
// FUNCTION: 0xf3d89
RO_STATE *__wcpp_2_stab_trav_move_(STAB_TRAVERSE *ctl)
{
    RO_DTREG *ro;
    unsigned state_var;
    unsigned char *flags;
    DTOR_CMD *cmd;
    RO_STATE *state;

    ro = ctl->rw->ro;
    flags = ctl->rw->flags;
    state_var = ctl->state_var;
    for (;;) {
        state = __wcpp_2_stab_entry_(ro, state_var);
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
            if (testFlag(flags, ((DTOR_CMD_TEST_FLAG *)cmd)->index)) {
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
