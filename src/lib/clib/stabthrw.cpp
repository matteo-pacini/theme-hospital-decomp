extern "C" {

void *memset(void *s, int c, unsigned n);
void *memcpy(void *dst, const void *src, unsigned n);
char *strcpy(char *dst, const char *src);
unsigned strlen(const char *s);
#pragma intrinsic(memcpy, strcpy, strlen)

struct DTOR_CMD {
    unsigned short code;
};

struct RT_TYPE_SIG;

struct DTOR_CMD_TRY {
    unsigned short code;
    unsigned jmp_buf;
    unsigned offset;
    unsigned count;
    RT_TYPE_SIG *sigs[1];
};

struct DTOR_CMD_FN_EXC {
    unsigned short code;
    unsigned count;
    RT_TYPE_SIG *sigs[1];
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
};

struct STAB_TRAVERSE {
    rw_dtreg *rw;
    unsigned state_var;
    unsigned char obj_type;
    unsigned bound;
};

enum TS_TYPE {
    TS_CLASS,
    TS_POINTER_SCALAR,
    TS_POINTER,
    TS_SCALAR,
    TS_CLASS_ARRAY,
    TS_ANY,
    TS_UNUSED6,
    TS_UNUSED7,
    TS_UNUSED8
};

struct RT_TYPE_SIG {
    TS_TYPE type;
    union {
        RT_TYPE_SIG *base;
        unsigned char size;
        struct {
            unsigned size;
            void (*ctor)();
            void (*dtor)(void *, unsigned);
        } clss;
    };
};

struct THROW_CNV {
    RT_TYPE_SIG *sig;
    unsigned offset;
};

struct THROW_RO {
    unsigned count;
    THROW_CNV cnvs[1];
};

struct active_exc {
    active_exc *prev;
    THROW_RO *throw_ro;
    RT_TYPE_SIG *sig;
    void *extra_object;
    unsigned reserved;
    rw_dtreg *rw;
    DTOR_CMD *cmd;
    unsigned char state;
    unsigned char zero_thrown : 1;
    char data[1];
};

struct thread_ctl {
    rw_dtreg *registered;
    unsigned char flags;
    char pad[3];
    active_exc *excepts;
    const char *abort_msg;
};

struct dispatch_exc {
    rw_dtreg *rw;
    THROW_RO *ro;
    thread_ctl *thr;
    active_exc *exc;
    unsigned reserved;
    THROW_CNV *cnv;
    DTOR_CMD *cmd;
    unsigned state_var;
    unsigned catch_no;
    unsigned zero : 1;
    unsigned rethrow : 1;
    unsigned char pad;
    unsigned long code;
};

enum DISPATCHABLE {
    DISPATCHABLE_NONE,
    DISPATCHABLE_CATCH,
    DISPATCHABLE_FNEXC,
    DISPATCHABLE_NO_CATCH,
    DISPATCHABLE_SYS_EXC
};

extern thread_ctl _wint_thread_data;
extern const char D_00128488[];
extern const char D_001284a7[];
extern const char D_001284d6[];
extern const char D_001284ff[];
extern const char D_00128541[];
extern const char D_0012856a[];
extern const char D_00128592[];
extern const char D_001285c1[];
extern const char D_001285ec[];
extern const char D_00128616[];
extern const char D_00128641[];
extern const char D_0012866b[];
extern void (*D_00152a90)(rw_dtreg *rw);

void __wcpp_2_call_terminate_(void);
void __wcpp_2_fatal_runtime_error_(const char *msg, unsigned code);
void __wcpp_2_destruct_internal_(unsigned bound, rw_dtreg *rw);
RO_STATE *__wcpp_2_stab_entry_(RO_DTREG *ro, unsigned state_var);
void __wcpp_2_stab_trav_init_(STAB_TRAVERSE *ctl, rw_dtreg *rw);
RO_STATE *__wcpp_2_stab_trav_next_(STAB_TRAVERSE *ctl);
RO_STATE *__wcpp_2_stab_trav_move_(STAB_TRAVERSE *ctl);
active_exc *__wcpp_2_alloc_exc_(void *object, THROW_RO *ro, thread_ctl *thr);
void __wcpp_2_free_exc_(thread_ctl *thr);
active_exc *__wcpp_2_pop_exc_(thread_ctl *thr);
void __wcpp_2_push_exc_(thread_ctl *thr, active_exc *exc);
char *ltoa(long value, char *buffer, int radix);

int ts_equiv(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero);
#pragma aux ts_equiv "__wcpp_2_ts_equiv__"
void unexpected(void);
#pragma aux unexpected "__wcpp_2_call_unexpected__"
void longjmp(void *env, int val);
#pragma aux longjmp aborts

void callTerminate(thread_ctl *thr);
#pragma aux callTerminate "W?callTerminate$n(pn$thread_ctl$$)v"
unsigned typesigIndex(dispatch_exc *dispatch, unsigned count, RT_TYPE_SIG **sigs);
#pragma aux typesigIndex "W?typesigIndex$n(pn$dispatch_exc$$uipnpn$rt_type_sig$$)ui"
int dispatchableFnExc(dispatch_exc *dispatch, DTOR_CMD *cmd);
#pragma aux dispatchableFnExc "W?dispatchableFnExc$n(pn$dispatch_exc$$pn$__iaap9DTOR_CMD$$)i"
void fneDispatch(dispatch_exc *dispatch);
#pragma aux fneDispatch "W?fneDispatch$n(pn$dispatch_exc$$)v"
int dispatchableCatch(dispatch_exc *dispatch, DTOR_CMD *cmd);
#pragma aux dispatchableCatch "W?dispatchableCatch$n(pn$dispatch_exc$$pn$__iaap9DTOR_CMD$$)i"
RT_TYPE_SIG *throwCnvSig(dispatch_exc *dispatch);
#pragma aux throwCnvSig "W?throwCnvSig$n(pn$dispatch_exc$$)pn$rt_type_sig$$"
int excToBePopped(thread_ctl *thr);
#pragma aux excToBePopped "W?excToBePopped$n(pn$thread_ctl$$)i"
void catchDispatch(dispatch_exc *dispatch);
#pragma aux catchDispatch "W?catchDispatch$n(pn$dispatch_exc$$)v"
void catchDone(thread_ctl *thr, active_exc *exc);
#pragma aux catchDone "W?catchDone$n(pn$thread_ctl$$pn$active_exc$$)v"
void unwindExc(dispatch_exc *dispatch);
#pragma aux unwindExc "W?unwindExc$n(pn$dispatch_exc$$)v"
void unwindStack(dispatch_exc *dispatch);
#pragma aux unwindStack "W?unwindStack$n(pn$dispatch_exc$$)v"
DISPATCHABLE dispatchable(dispatch_exc *dispatch, rw_dtreg *rw);
#pragma aux dispatchable "W?dispatchable$n(pn$dispatch_exc$$pn$rw_dtreg$$)$$TE$$"
active_exc *getActiveExc(thread_ctl *thr);
#pragma aux getActiveExc "W?getActiveExc$n(pn$thread_ctl$$)pn$active_exc$$"
void dispatch_init(dispatch_exc *dispatch, THROW_RO *ro, int zero);
#pragma aux dispatch_init "W?dispatch_init$n(pn$dispatch_exc$$pn$throw_ro$$i)v"
void throwIt(void *object, THROW_RO *ro, int zero);
/* config/functions.txt has no name for the common throw code. */
#pragma aux throwIt "W?processThrow$n(pnvpn$throw_ro$$i)v"
void popCatches(dispatch_exc *dispatch, rw_dtreg *rw);
/* config/functions.txt has no name for this longjmp helper. */
#pragma aux popCatches "W?longjmpExcPop$n(pn$dispatch_exc$$pn$rw_dtreg$$)v"
void __wcpp_2_lj_handler_(rw_dtreg *bound);
#pragma aux __wcpp_2_lj_handler_ "W?longjmpDtoring$n(pfv)v" modify [edx]

void __wcpp_2_corrupted_stack_(void);
DISPATCHABLE __wcpp_2_dispatchable_(dispatch_exc *dispatch, rw_dtreg *rw);
void __wcpp_2_save_exc_(dispatch_exc *dispatch, void *object);
void __wcpp_2_dispatch_(dispatch_exc *dispatch, DISPATCHABLE kind);

// FUNCTION: 0xf3e1c
void callTerminate(thread_ctl *thr)
{
    active_exc *exc;

    exc = thr->excepts;
    if (exc != 0)
        exc->state = 3;
    thr->flags |= 1;
    __wcpp_2_call_terminate_();
}

// FUNCTION: 0xf3e33
// NONMATCHING: the original reaches callTerminate with a short jump within
// the same segment
void __wcpp_2_corrupted_stack_(void)
{
    _wint_thread_data.abort_msg = D_00128488;
    callTerminate(&_wint_thread_data);
}

// FUNCTION: 0xf3e44
// NONMATCHING: register allocation and stack slots
unsigned typesigIndex(dispatch_exc *dispatch, unsigned count, RT_TYPE_SIG **sigs)
{
    int zero;
    RT_TYPE_SIG *sig;
    THROW_CNV *cnv_base;
    unsigned index;
    unsigned ctr;
    THROW_CNV *cnv;
    THROW_RO *throw_ro;
    RT_TYPE_SIG **end;

    throw_ro = dispatch->ro;
    zero = dispatch->zero;
    cnv_base = throw_ro->cnvs;
    end = sigs + count;
    index = 0;
    for (; sigs < end; ++sigs) {
        sig = *sigs;
        if (sig == 0)
            return index + 1;
        for (ctr = 0; ctr < throw_ro->count; ++ctr) {
            cnv = &cnv_base[ctr];
            dispatch->cnv = cnv;
            if (ts_equiv(sig, cnv->sig, zero))
                return index + 1;
        }
        ++index;
    }
    return 0;
}

// FUNCTION: 0xf3ee0
int dispatchableFnExc(dispatch_exc *dispatch, DTOR_CMD *cmd)
{
    unsigned index;
    DTOR_CMD_FN_EXC *fn_exc;

    fn_exc = (DTOR_CMD_FN_EXC *)cmd;
    index = typesigIndex(dispatch, fn_exc->count, fn_exc->sigs);
    if (index == 0 || fn_exc->sigs[index - 1] == 0)
        return 1;
    return 0;
}

// FUNCTION: 0xf3f07
void fneDispatch(dispatch_exc *dispatch)
{
    thread_ctl *thr;

    thr = dispatch->thr;
    thr->abort_msg = D_001284a7;
    thr->excepts->state = 2;
    unexpected();
}

// FUNCTION: 0xf3f1d
int dispatchableCatch(dispatch_exc *dispatch, DTOR_CMD *cmd)
{
    unsigned index;
    DTOR_CMD_TRY *try_cmd;

    try_cmd = (DTOR_CMD_TRY *)cmd;
    index = typesigIndex(dispatch, try_cmd->count, try_cmd->sigs);
    if (index == 0)
        return 0;
    dispatch->cmd = cmd;
    dispatch->catch_no = index - 1;
    return 1;
}

// FUNCTION: 0xf3f43
RT_TYPE_SIG *throwCnvSig(dispatch_exc *dispatch)
{
    RT_TYPE_SIG *sig;

    sig = dispatch->cnv->sig;
    if (sig->type == TS_POINTER)
        sig = sig->base;
    return sig;
}

// FUNCTION: 0xf3f51
int excToBePopped(thread_ctl *thr)
{
    rw_dtreg *rw;
    active_exc *exc;
    RO_STATE *state;
    void *try_cmd;
    STAB_TRAVERSE ctl;

    rw = thr->registered;
    exc = thr->excepts;
    state = __wcpp_2_stab_entry_(rw->ro, rw->state_var);
    if (state == 0)
        return 0;
    try_cmd = (char *)state->cmd + 2;
    if (rw == exc->rw && try_cmd == exc->cmd) {
    pop:
        return 1;
    }
    for (; rw != 0; rw = rw->prev) {
        if (rw == exc->rw) {
            for (__wcpp_2_stab_trav_init_(&ctl, rw); (state = __wcpp_2_stab_trav_move_(&ctl)) != 0;
                 __wcpp_2_stab_trav_next_(&ctl)) {
                if (state->dtor == 0 && state->cmd->code == 3
                    && (char *)state->cmd + 2 == (char *)exc->cmd)
                    return 0;
            }
            goto pop;
        }
    }
    goto pop;
}

// FUNCTION: 0xf3ff4
// NONMATCHING: the original keeps rw in a stack slot and src/offset in
// edx/esi; 10.6 allocates them differently
void catchDispatch(dispatch_exc *dispatch)
{
    rw_dtreg *rw;
    active_exc *exc;
    DTOR_CMD_TRY *try_cmd;
    void *env;
    RT_TYPE_SIG *sig;
    RT_TYPE_SIG *cnv_sig;
    char *dest;
    char *src;
    unsigned offset;

    rw = dispatch->rw;
    exc = dispatch->exc;
    try_cmd = (DTOR_CMD_TRY *)dispatch->cmd;
    exc->state = 1;
    env = (char *)rw + try_cmd->jmp_buf;
    sig = try_cmd->sigs[dispatch->catch_no];
    if (sig != 0) {
        dest = (char *)rw + try_cmd->offset;
        offset = dispatch->cnv->offset;
        src = exc->data + offset;
        switch (sig->type) {
        case 1:
            if (dispatch->zero) {
                *(void **)dest = 0;
            } else {
                *(char **)dest = *(char **)exc->data + offset;
            }
            break;
        case 6:
        case 7:
        case 8:
            if (dispatch->zero) {
                memset(dest, 0, sig->size);
            } else {
                memcpy(dest, src, sig->size);
            }
            break;
        case 3:
            memcpy(dest, src, sig->size);
            break;
        case 0:
            cnv_sig = throwCnvSig(dispatch);
            ((void (*)(void *, void *))cnv_sig->clss.ctor)(dest, src);
            break;
        case 4:
            cnv_sig = throwCnvSig(dispatch);
            ((void (*)(void *, int, void *))cnv_sig->clss.ctor)(dest, 0, src);
            break;
        case 2:
            switch (sig->base->type) {
            case 1:
                if (dispatch->zero) {
                    exc->extra_object = 0;
                } else {
                    exc->extra_object = *(char **)exc->data + offset;
                }
                *(void **)dest = &exc->extra_object;
                break;
            case 6:
            case 7:
            case 8:
                if (dispatch->zero) {
                    src = (char *)&exc->extra_object;
                    exc->extra_object = 0;
                }
                *(char **)dest = src;
                break;
            case 4:
            case 0:
            case 3:
                *(char **)dest = src;
                break;
            }
            break;
        default:
            __wcpp_2_corrupted_stack_();
            break;
        }
    }
    longjmp(env, dispatch->catch_no + 1);
}

// FUNCTION: 0xf4132
void catchDone(thread_ctl *thr, active_exc *exc)
{
    RT_TYPE_SIG *sig;
    void (*dtor)(void *, unsigned);

    sig = exc->sig;
    if (sig->type == TS_POINTER)
        sig = sig->base;
    if (sig->type == TS_CLASS || sig->type == TS_CLASS_ARRAY) {
        dtor = sig->clss.dtor;
        if (dtor != 0) {
            exc->state = 6;
            dtor(exc->data, 0);
        }
    }
    __wcpp_2_free_exc_(thr);
}

// FUNCTION: 0xf416f
void __wcpp_2_catch_done_(void)
{
    if (excToBePopped(&_wint_thread_data))
        catchDone(&_wint_thread_data, _wint_thread_data.excepts);
}

// FUNCTION: 0xf4190
void unwindExc(dispatch_exc *dispatch)
{
    thread_ctl *thr;
    active_exc *bound;
    active_exc *exc;
    active_exc *prev;

    thr = dispatch->thr;
    bound = dispatch->exc;
    exc = thr->excepts;
    for (;;) {
        if (exc == bound)
            break;
        prev = exc->prev;
        catchDone(thr, exc);
        exc = prev;
    }
}

// FUNCTION: 0xf41b5
void unwindStack(dispatch_exc *dispatch)
{
    thread_ctl *thr;
    active_exc *exc;
    rw_dtreg *rw;
    unsigned char state;

    thr = dispatch->thr;
    exc = thr->excepts;
    rw = dispatch->rw;
    if (exc != 0) {
        state = exc->state;
        exc->state = 0;
    }
    while (rw != thr->registered) {
        __wcpp_2_destruct_internal_(0, thr->registered);
        thr->registered = thr->registered->prev;
    }
    if (rw != 0)
        __wcpp_2_destruct_internal_(dispatch->state_var, rw);
    if (exc != 0)
        exc->state = state;
}

// FUNCTION: 0xf420b
// NONMATCHING: the target places the command compare tree before the case
// bodies; 10.6 emits it after them
DISPATCHABLE dispatchable(dispatch_exc *dispatch, rw_dtreg *rw)
{
    int rethrow;
    active_exc *exc;
    RO_STATE *state;
    DISPATCHABLE retn;
    STAB_TRAVERSE ctl;

    if (dispatch->rethrow) {
        exc = dispatch->exc;
        if (exc == 0)
            __wcpp_2_corrupted_stack_();
        dispatch->ro = exc->throw_ro;
        dispatch->zero = exc->zero_thrown;
        rethrow = 1;
    } else {
        rethrow = 0;
    }
    for (__wcpp_2_stab_trav_init_(&ctl, rw); (state = __wcpp_2_stab_trav_move_(&ctl)) != 0;
         __wcpp_2_stab_trav_next_(&ctl)) {
        if (state->dtor == 0) {
            switch (state->cmd->code) {
            case 3:
                exc = dispatch->exc;
                if (exc == 0)
                    __wcpp_2_corrupted_stack_();
                if (rethrow) {
                    rethrow = 0;
                } else {
                    dispatch->exc = exc->prev;
                }
                break;
            case 4:
                if (dispatchableFnExc(dispatch, state->cmd)) {
                    __wcpp_2_stab_trav_next_(&ctl);
                    retn = DISPATCHABLE_FNEXC;
                    __wcpp_2_stab_trav_move_(&ctl);
                    goto found;
                }
                break;
            case 2:
                if (dispatchableCatch(dispatch, state->cmd)) {
                    retn = DISPATCHABLE_CATCH;
                    goto found;
                }
                break;
        default:
            break;
            }
        }
    }
    retn = DISPATCHABLE_NONE;
    goto done;
found:
    dispatch->state_var = ctl.state_var;
done:
    return retn;
}

// FUNCTION: 0xf430c
active_exc *getActiveExc(thread_ctl *thr)
{
    active_exc *exc;

    exc = thr->excepts;
    if (exc == 0) {
        if (thr->flags & 1)
            __wcpp_2_fatal_runtime_error_(D_001284d6, 1);
    } else {
        switch (exc->state) {
        case 0:
            thr->abort_msg = D_001284ff;
            callTerminate(thr);
        case 3:
            __wcpp_2_fatal_runtime_error_(D_00128541, 1);
        case 5:
            __wcpp_2_fatal_runtime_error_(D_0012856a, 1);
        case 6:
            __wcpp_2_fatal_runtime_error_(D_00128592, 1);
        default:
            __wcpp_2_corrupted_stack_();
            break;
        case 1:
        case 2:
            break;
        }
    }
    thr->abort_msg = 0;
    return exc;
}

// FUNCTION: 0xf4390
void dispatch_init(dispatch_exc *dispatch, THROW_RO *ro, int zero)
{
    dispatch->thr = &_wint_thread_data;
    dispatch->cmd = 0;
    dispatch->ro = ro;
    dispatch->zero = zero;
    dispatch->exc = getActiveExc(dispatch->thr);
    if (ro == 0) {
        dispatch->rethrow = 1;
    } else {
        dispatch->rethrow = 0;
    }
}

// FUNCTION: 0xf43c9
DISPATCHABLE __wcpp_2_dispatchable_(dispatch_exc *dispatch, rw_dtreg *rw)
{
    unsigned kind;

    if (rw == 0) {
        if (dispatch->rethrow) {
            __wcpp_2_fatal_runtime_error_(D_001285c1, 1);
        } else {
            dispatch->thr->abort_msg = D_001285ec;
            callTerminate(dispatch->thr);
        }
    }
    dispatch->rw = rw;
    kind = rw->ro->kind;
    switch (kind) {
    default:
        __wcpp_2_corrupted_stack_();
    case 0:
        return dispatchable(dispatch, rw);
    case 1:
    case 2:
    case 3:
    case 4:
        return DISPATCHABLE_NONE;
    }
}

// FUNCTION: 0xf4422
void __wcpp_2_save_exc_(dispatch_exc *dispatch, void *object)
{
    active_exc *exc;

    if (dispatch->rethrow) {
        exc = __wcpp_2_pop_exc_(dispatch->thr);
        if (exc != dispatch->exc)
            unwindExc(dispatch);
        __wcpp_2_push_exc_(dispatch->thr, exc);
        if (excToBePopped(dispatch->thr)) {
            exc->cmd = dispatch->cmd;
            exc->rw = dispatch->rw;
        }
    } else {
        unwindExc(dispatch);
        exc = __wcpp_2_alloc_exc_(object, dispatch->ro, dispatch->thr);
        dispatch->exc = exc;
        exc->cmd = dispatch->cmd;
        exc->rw = dispatch->rw;
    }
}

// FUNCTION: 0xf44a0
void __wcpp_2_dispatch_(dispatch_exc *dispatch, DISPATCHABLE kind)
{
    char buf[40];

    switch (kind) {
    case DISPATCHABLE_FNEXC:
        fneDispatch(dispatch);
    case DISPATCHABLE_CATCH:
        catchDispatch(dispatch);
    case DISPATCHABLE_NO_CATCH:
        if (dispatch->rethrow) {
            __wcpp_2_fatal_runtime_error_(D_00128616, 1);
        } else {
            dispatch->thr->abort_msg = D_00128641;
            callTerminate(dispatch->thr);
        }
    case DISPATCHABLE_SYS_EXC:
        strcpy(buf, D_0012866b);
        ltoa(dispatch->code, buf + strlen(buf) - 8, 16);
        __wcpp_2_fatal_runtime_error_(buf, 1);
    default:
        __wcpp_2_corrupted_stack_();
    }
}

// FUNCTION: 0xf4551
void throwIt(void *object, THROW_RO *ro, int zero)
{
    dispatch_exc dispatch;
    rw_dtreg *rw;
    DISPATCHABLE disp;

    dispatch_init(&dispatch, ro, zero);
    for (disp = DISPATCHABLE_NONE, rw = dispatch.thr->registered; disp == DISPATCHABLE_NONE; rw = rw->prev)
        disp = __wcpp_2_dispatchable_(&dispatch, rw);
    __wcpp_2_save_exc_(&dispatch, object);
    unwindStack(&dispatch);
    __wcpp_2_dispatch_(&dispatch, disp);
}

// FUNCTION: 0xf4598
void __wcpp_2_rethrow_(void)
{
    throwIt(0, 0, 0);
}

// FUNCTION: 0xf45a8
void __wcpp_2_throw_(void *object, THROW_RO *ro)
{
    throwIt(object, ro, 0);
}

// FUNCTION: 0xf45b2
// NONMATCHING: the target jumps into __wcpp_2_throw_'s call, so the module
// was built in one segment; compiling it with -zm- reproduces this but moves
// getActiveExc's jump table until dispatchable matches
void __wcpp_2_throw_zero_(void *object, THROW_RO *ro)
{
    throwIt(object, ro, 1);
}

// FUNCTION: 0xf45ba
void popCatches(dispatch_exc *dispatch, rw_dtreg *rw)
{
    STAB_TRAVERSE ctl;
    RO_STATE *state;
    active_exc *exc;

    __wcpp_2_stab_trav_init_(&ctl, rw);
    for (exc = dispatch->exc; (state = __wcpp_2_stab_trav_move_(&ctl)) != 0;
         __wcpp_2_stab_trav_next_(&ctl)) {
        if (state->dtor == 0 && state->cmd->code == 3) {
            if (exc == 0)
                __wcpp_2_corrupted_stack_();
            exc = exc->prev;
            if (exc == 0)
                break;
        }
    }
    dispatch->exc = exc;
}

// FUNCTION: 0xf4605
void __wcpp_2_lj_handler_(rw_dtreg *bound)
{
    dispatch_exc dispatch;
    rw_dtreg *rw;
    rw_dtreg *last;

    dispatch.thr = &_wint_thread_data;
    rw = _wint_thread_data.registered;
    last = rw;
    for (;; rw = rw->prev) {
        if (rw == 0) {
            if (last == 0)
                return;
            last = rw;
            dispatch.state_var = 0;
            break;
        }
        last = rw;
        if (rw >= bound) {
            dispatch.state_var = rw->state_var;
            break;
        }
    }
    dispatch.zero = 0;
    dispatch.ro = 0;
    dispatch.cmd = 0;
    dispatch.rw = last;
    dispatch.exc = getActiveExc(dispatch.thr);
    for (rw = dispatch.thr->registered; rw != dispatch.rw && dispatch.exc != 0; rw = rw->prev) {
        switch (rw->ro->kind) {
        case 1:
        case 2:
        case 3:
        case 4:
            continue;
        default:
            __wcpp_2_corrupted_stack_();
        case 0:
            break;
        }
        popCatches(&dispatch, rw);
    }
    unwindExc(&dispatch);
    unwindStack(&dispatch);
}

// FUNCTION: 0xf46a5
void __wcpp_2_lj_init_(void)
{
    D_00152a90 = (void (*)(rw_dtreg *))__wcpp_2_lj_handler_;
}

}
