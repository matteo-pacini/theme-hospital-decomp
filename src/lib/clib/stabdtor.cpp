extern "C" {

struct DTOR_CMD {
    unsigned short code;
};

struct RO_STATE {
    void (*dtor)(void *, unsigned);
    DTOR_CMD *cmd;
    void *obj;
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

// nextDtorPosn: advance ctl to its next state-table entry, checking that a command entry (one without a destructor) is a known kind; returns the entry.
// FUNCTION: 0xeb9c0
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
        case 0:
        case 1:
        case 5:
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
void __wcpp_2_stab_trav_next_(STAB_TRAVERSE *ctl);
void __wcpp_2_dtor_array_(void *array, unsigned count, void *sig);

void dtorComponent(DTOR_CMD_COMPONENT *cmd, unsigned dtor_flag, rw_dtreg *rw, unsigned mask);
#pragma aux dtorComponent "W?dtorComponent$n(pn$__iaap9DTOR_CMD$$uipn$rw_dtreg$$ui)v"
void destruct_traverse(unsigned bound, rw_dtreg *rw, int partial);
#pragma aux destruct_traverse "W?destruct_traverse$n(uipn$rw_dtreg$$i)v"

// dtorComponent: call a component's destructor (with dtor_flag) unless its registration flags intersect mask.
// FUNCTION: 0xeba0a
void dtorComponent(DTOR_CMD_COMPONENT *cmd, unsigned dtor_flag, rw_dtreg *rw, unsigned mask)
{
    COMPONENT_REG *comp;

    comp = (COMPONENT_REG *)((char *)rw + cmd->offset);
    if (!(comp->flags & mask))
        cmd->dtor(comp->base + cmd->delta, dtor_flag);
}

// destruct_traverse: destroy rw's registered objects from its current state down to bound (partial: start past the object under construction), executing the state table's destructors and commands.
// FUNCTION: 0xeba54
void destruct_traverse(unsigned bound, rw_dtreg *rw, int partial)
{
    RO_STATE *prev;
    RO_STATE *state;
    unsigned state_var;
    DTOR_CMD *cmd;
    ARRAY_REG *arr;
    void (*dtor)(void *, unsigned);
    void *obj;
    RO_DTREG *ro;
    STAB_TRAVERSE ctl;

    __wcpp_2_stab_trav_init_(&ctl, rw);
    ctl.bound = bound;
    if (partial)
        __wcpp_2_stab_trav_comp_(&ctl);
    ro = rw->ro;
    state = nextDtorPosn(&ctl);
    while (ctl.state_var != bound) {
        if (ctl.state_var < bound)
            __wcpp_2_corrupted_stack_();
        state_var = ctl.state_var;
        __wcpp_2_stab_trav_next_(&ctl);
        prev = state;
        state = nextDtorPosn(&ctl);
        rw->state_var = ctl.state_var;
        dtor = prev->dtor;
        if (dtor == 0) {
        cmd = prev->cmd;
        switch (cmd->code) {
        default:
            __wcpp_2_corrupted_stack_();
        case 0:
            obj = __wcpp_2_data_dtreg_vfptrs_[ro->kind]->get_object(rw, state_var);
            __wcpp_2_dtor_array_((char *)obj + ((DTOR_CMD_ARRAY *)cmd)->offset,
                ((DTOR_CMD_ARRAY *)cmd)->count, ((DTOR_CMD_ARRAY *)cmd)->sig);
            break;
        case 2:
        case 3:
        case 4:
            break;
        case 6:
            dtorComponent((DTOR_CMD_COMPONENT *)cmd, 1, rw, 0x11);
            break;
        case 7:
            dtorComponent((DTOR_CMD_COMPONENT *)cmd, 1, rw, 0x10);
            break;
        case 8:
            dtorComponent((DTOR_CMD_COMPONENT *)cmd, 0, rw, 0x10);
            break;
        case 9:
            arr = (ARRAY_REG *)((char *)rw + ((DTOR_CMD_ARRAY *)cmd)->offset);
            __wcpp_2_dtor_array_(arr->array, arr->count, arr->sig);
            break;
        case 10:
            ((void (*)(void *))((DTOR_CMD_DELETE *)cmd)->op_del)(
                *(void **)((char *)rw + ((DTOR_CMD_DELETE *)cmd)->offset));
            break;
        case 11:
            ((DTOR_CMD_DELETE *)cmd)->op_del(
                *(void **)((char *)rw + ((DTOR_CMD_DELETE *)cmd)->offset),
                ((DTOR_CMD_DELETE *)cmd)->extra);
            break;
        }
        } else {
            obj = __wcpp_2_data_dtreg_vfptrs_[ro->kind]->get_object(rw, state_var);
            dtor(obj, 0);
        }
    }
    rw->state_var = ctl.state_var;
}


void __wcpp_2_destruct_internal_(unsigned bound, rw_dtreg *rw);

// __wcpp_2_destruct_internal_: destroy rw's objects down to state bound.
// FUNCTION: 0xebb84
void __wcpp_2_destruct_internal_(unsigned bound, rw_dtreg *rw)
{
    destruct_traverse(bound, rw, 0);
}

struct THREAD_CTL {
    rw_dtreg *registered;
};

extern THREAD_CTL _wint_thread_data;

void __wcpp_2_destruct_(unsigned bound);
void __wcpp_2_destruct_after_(unsigned bound);

// __wcpp_2_destruct_all_: destroy every object of the newest registration.
// FUNCTION: 0xebb8e
void __wcpp_2_destruct_all_(void)
{
    __wcpp_2_destruct_(0);
}

// __wcpp_2_destruct_: destroy the newest registration's objects down to state bound.
// FUNCTION: 0xebb90
void __wcpp_2_destruct_(unsigned bound)
{
    destruct_traverse(bound, _wint_thread_data.registered, 0);
}

// __wcpp_2_destruct_after_all_: __wcpp_2_destruct_after_(0).
// FUNCTION: 0xebba2
void __wcpp_2_destruct_after_all_(void)
{
    __wcpp_2_destruct_after_(0);
}

// __wcpp_2_destruct_after_: destroy the newest registration's objects down to bound, skipping the one under construction.
// FUNCTION: 0xebba4
void __wcpp_2_destruct_after_(unsigned bound)
{
    destruct_traverse(bound, _wint_thread_data.registered, 1);
}

}
