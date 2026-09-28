extern "C" {

struct DTOR_CMD {
    unsigned short code;
};

struct RO_STATE {
    void (*dtor)(void *, unsigned);
    DTOR_CMD *cmd;
};

struct STAB_TRAVERSE {
    void *rw;
    unsigned state_var;
    unsigned char obj_type;
    unsigned bound;
};

RO_STATE *__wcpp_2_stab_trav_move_(STAB_TRAVERSE *ctl);
void __wcpp_2_corrupted_stack_(void);

RO_STATE *nextDtorPosn(STAB_TRAVERSE *ctl);
#pragma aux nextDtorPosn "W?nextDtorPosn$n(pn$__1j2o3bSTAB_TRAVERSE$$)pn$__7a4u3RO_STATE$$"

// FUNCTION: 0xeb9c0
// NONMATCHING: the range check for code 5 is emitted as < 6 instead of <= 5
RO_STATE *nextDtorPosn(STAB_TRAVERSE *ctl)
{
    RO_STATE *state;

    state = __wcpp_2_stab_trav_move_(ctl);
    if (state != 0 && state->dtor == 0 && ctl->state_var != ctl->bound) {
        switch (state->cmd->code) {
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            break;
        default:
            __wcpp_2_corrupted_stack_();
            break;
        }
    }
    return state;
}

struct RO_DTREG {
    unsigned kind;
};

struct rw_dtreg {
    rw_dtreg *prev;
    RO_DTREG *ro;
    unsigned state_var;
};

struct DTREG_VFPTRS {
    void *reserved;
    void *(*get_object)(rw_dtreg *rw, unsigned state_var);
};

struct DTOR_CMD_ARRAY {
    unsigned short code;
    unsigned offset;
    unsigned count;
    void *sig;
};

struct DTOR_CMD_COMPONENT {
    unsigned short code;
    unsigned offset;
    unsigned delta;
    void (*dtor)(void *, unsigned);
};

struct DTOR_CMD_DELETE {
    unsigned short code;
    unsigned offset;
    void (*op_del)(void *, void *);
    void *extra;
};

struct COMPONENT_REG {
    char *base;
    unsigned char flags;
};

struct ARRAY_REG {
    void *array;
    void *sig;
    unsigned count;
};

extern DTREG_VFPTRS *__wcpp_2_data_dtreg_vfptrs_[];

void __wcpp_2_stab_trav_init_(STAB_TRAVERSE *ctl, rw_dtreg *rw);
void __wcpp_2_stab_trav_comp_(STAB_TRAVERSE *ctl);
void __wcpp_2_stab_trav_next_(STAB_TRAVERSE *ctl, unsigned state_var);
void __wcpp_2_dtor_array_(void *array, unsigned count, void *sig);

void dtorComponent(DTOR_CMD_COMPONENT *cmd, unsigned dtor_flag, rw_dtreg *rw, unsigned mask);
#pragma aux dtorComponent "W?dtorComponent$n(pn$__iaap9DTOR_CMD$$uipn$rw_dtreg$$ui)v"
void destruct_traverse(unsigned bound, rw_dtreg *rw, int partial);
#pragma aux destruct_traverse "W?destruct_traverse$n(uipn$rw_dtreg$$i)v"

// FUNCTION: 0xeba0a
// NONMATCHING: config/functions.txt includes destruct_traverse's jump table
// in dtorComponent
void dtorComponent(DTOR_CMD_COMPONENT *cmd, unsigned dtor_flag, rw_dtreg *rw, unsigned mask)
{
    COMPONENT_REG *comp;

    comp = (COMPONENT_REG *)((char *)rw + cmd->offset);
    if (!(comp->flags & mask))
        cmd->dtor(comp->base + cmd->delta, dtor_flag);
}

// FUNCTION: 0xeba54
// NONMATCHING: the switch's jump table is placed after the function; code
// otherwise unverified
void destruct_traverse(unsigned bound, rw_dtreg *rw, int partial)
{
    STAB_TRAVERSE ctl;
    RO_STATE *state;
    RO_STATE *prev;
    RO_DTREG *ro;
    void (*dtor)(void *, unsigned);
    void *obj;
    ARRAY_REG *arr;

    __wcpp_2_stab_trav_init_(&ctl, rw);
    ctl.bound = bound;
    if (partial)
        __wcpp_2_stab_trav_comp_(&ctl);
    ro = rw->ro;
    state = nextDtorPosn(&ctl);
    while (ctl.state_var != bound) {
        if (ctl.state_var < bound)
            __wcpp_2_corrupted_stack_();
        __wcpp_2_stab_trav_next_(&ctl, ctl.state_var);
        prev = state;
        state = nextDtorPosn(&ctl);
        rw->state_var = ctl.state_var;
        dtor = prev->dtor;
        if (dtor != 0) {
            obj = __wcpp_2_data_dtreg_vfptrs_[ro->kind]->get_object(rw, ctl.state_var);
            dtor(obj, 0);
            continue;
        }
        switch (prev->cmd->code) {
        case 0:
            obj = __wcpp_2_data_dtreg_vfptrs_[ro->kind]->get_object(rw, ctl.state_var);
            __wcpp_2_dtor_array_((char *)obj + ((DTOR_CMD_ARRAY *)prev->cmd)->offset,
                ((DTOR_CMD_ARRAY *)prev->cmd)->count, ((DTOR_CMD_ARRAY *)prev->cmd)->sig);
            break;
        case 2:
        case 3:
        case 4:
            break;
        case 6:
            dtorComponent((DTOR_CMD_COMPONENT *)prev->cmd, 1, rw, 0x11);
            break;
        case 7:
            dtorComponent((DTOR_CMD_COMPONENT *)prev->cmd, 1, rw, 0x10);
            break;
        case 8:
            dtorComponent((DTOR_CMD_COMPONENT *)prev->cmd, 0, rw, 0x10);
            break;
        case 9:
            arr = (ARRAY_REG *)((char *)rw + ((DTOR_CMD_ARRAY *)prev->cmd)->offset);
            __wcpp_2_dtor_array_(arr->array, arr->count, arr->sig);
            break;
        case 10:
            ((DTOR_CMD_DELETE *)prev->cmd)->op_del(
                *(void **)((char *)rw + ((DTOR_CMD_DELETE *)prev->cmd)->offset), 0);
            break;
        case 11:
            ((DTOR_CMD_DELETE *)prev->cmd)->op_del(
                *(void **)((char *)rw + ((DTOR_CMD_DELETE *)prev->cmd)->offset),
                ((DTOR_CMD_DELETE *)prev->cmd)->extra);
            break;
        default:
            __wcpp_2_corrupted_stack_();
            break;
        }
    }
    rw->state_var = ctl.state_var;
}

void __wcpp_2_destruct_internal_(unsigned bound, rw_dtreg *rw);

// FUNCTION: 0xebb84
void __wcpp_2_destruct_internal_(unsigned bound, rw_dtreg *rw)
{
    destruct_traverse(bound, rw, 0);
}

struct THREAD_CTL {
    rw_dtreg *registered;
};

extern THREAD_CTL _wint_thread_data;

void __wcpp_2_destruct_all_(void);
/* config/functions.txt has no name for __wcpp_2_destruct_all__. */
#pragma aux __wcpp_2_destruct_all_ "fn_000ebb8e_"

// FUNCTION: 0xebb8e
// NONMATCHING: the target clears eax before the register saves
void __wcpp_2_destruct_all_(void)
{
    destruct_traverse(0, _wint_thread_data.registered, 0);
}

void __wcpp_2_destruct_(void);
/* config/functions.txt has no name for __wcpp_2_destruct__. */
#pragma aux __wcpp_2_destruct_ "fn_000ebba2_"

// FUNCTION: 0xebba2
// NONMATCHING: the target shares the call and epilogue of
// __wcpp_2_destruct_all__ by jumping into it
void __wcpp_2_destruct_(void)
{
    destruct_traverse(0, _wint_thread_data.registered, 1);
}

}
