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
    TS_POINTER_KIND6,
    TS_POINTER_KIND7,
    TS_SCALAR_KIND8
};

struct RT_TYPE_SIG {
    TS_TYPE type;
    union {
        RT_TYPE_SIG *base;
        unsigned char size;
        struct {
            unsigned flags;
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
    // The EXC_AREA the exception was allocated in (allocexc.cpp).
    unsigned area;
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
// The module's messages, in its CONST segment in this order. _1/_2 pairs are
// separate copies of the same text.
extern const char stabthrw_msg_stack_corrupted[];
extern const char stabthrw_msg_fn_exc_spec[];
extern const char stabthrw_msg_throw_in_terminate_1[];
extern const char stabthrw_msg_throw_in_unwind_dtor[];
extern const char stabthrw_msg_throw_in_terminate_2[];
extern const char stabthrw_msg_throw_in_ctor[];
extern const char stabthrw_msg_throw_in_handled_dtor[];
extern const char stabthrw_msg_rethrow_no_handler_1[];
extern const char stabthrw_msg_no_handler_1[];
extern const char stabthrw_msg_rethrow_no_handler_2[];
extern const char stabthrw_msg_no_handler_2[];
extern const char stabthrw_msg_system_exception[];
extern void (*__longjmp_handler)(rw_dtreg *rw);

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

int __wcpp_2_ts_equiv_(RT_TYPE_SIG *catch_sig, RT_TYPE_SIG *throw_sig, int zero);
#pragma aux __wcpp_2_ts_equiv_ "__wcpp_2_ts_equiv__"
void __wcpp_2_call_unexpected_(void);
#pragma aux __wcpp_2_call_unexpected_ "__wcpp_2_call_unexpected__"
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
void processThrow(void *object, THROW_RO *ro, int zero);
/* The library's name for the common throw code. */
#pragma aux processThrow "W?processThrow$n(pnvpn$throw_ro$$i)v"
void longjmpExcPop(dispatch_exc *dispatch, rw_dtreg *rw);
/* The library's name for this longjmp helper. */
#pragma aux longjmpExcPop "W?longjmpExcPop$n(pn$dispatch_exc$$pn$rw_dtreg$$)v"
void longjmpDtoring(rw_dtreg *bound);
#pragma aux longjmpDtoring "W?longjmpDtoring$n(pfv)v" modify [edx]

void __wcpp_2_corrupted_stack_(void);
DISPATCHABLE __wcpp_2_dispatchable_(dispatch_exc *dispatch, rw_dtreg *rw);
void __wcpp_2_save_exc_(dispatch_exc *dispatch, void *object);
void __wcpp_2_dispatch_(dispatch_exc *dispatch, DISPATCHABLE kind);

// callTerminate: mark the current exception and the thread as terminating, then call terminate().
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

// __wcpp_2_corrupted_stack_: terminate with the "corrupted stack" abort message.
// FUNCTION: 0xf3e33
// NONMATCHING: the original reaches callTerminate with a short jump within
// the same segment
void __wcpp_2_corrupted_stack_(void)
{
    _wint_thread_data.abort_msg = stabthrw_msg_stack_corrupted;
    callTerminate(&_wint_thread_data);
}

// typesigIndex: 1-based index of the first of count catch signatures sigs (NULL = catch-all) that accepts one of the thrown type's conversions (recorded in dispatch->cnv); 0 if none.
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
            if (__wcpp_2_ts_equiv_(sig, cnv->sig, zero))
                return index + 1;
        }
        ++index;
    }
    return 0;
}

// dispatchableFnExc: 1 if the thrown type violates the exception specification cmd (no listed type matches, or the match is a NULL entry), else 0.
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

// fneDispatch: exception-specification violation; mark the exception and call unexpected() (through __wcpp_2_call_unexpected__).
// FUNCTION: 0xf3f07
void fneDispatch(dispatch_exc *dispatch)
{
    thread_ctl *thr;

    thr = dispatch->thr;
    thr->abort_msg = stabthrw_msg_fn_exc_spec;
    thr->excepts->state = 2;
    __wcpp_2_call_unexpected_();
}

// dispatchableCatch: 1 if try block cmd has a catch for the thrown type, recording the try and the catch number in dispatch; else 0.
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

// throwCnvSig: type signature of the matched conversion (a pointer's base type).
// FUNCTION: 0xf3f43
RT_TYPE_SIG *throwCnvSig(dispatch_exc *dispatch)
{
    RT_TYPE_SIG *sig;

    sig = dispatch->cnv->sig;
    if (sig->type == TS_POINTER)
        sig = sig->base;
    return sig;
}

// excToBePopped: 1 if leaving the current catch block should destroy the thread's current exception (it is not still caught further out), else 0.
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

// catchDispatch: initialise the matched catch parameter from the exception object and longjmp into the catch block.
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

// catchDone: destroy exception exc's object (class types), then free the thread's newest exception.
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

// __wcpp_2_catch_done_: called at the end of a catch block; finish the current exception if it is to be popped.
// FUNCTION: 0xf416f
void __wcpp_2_catch_done_(void)
{
    if (excToBePopped(&_wint_thread_data))
        catchDone(&_wint_thread_data, _wint_thread_data.excepts);
}

// unwindExc: finish every active exception newer than dispatch->exc.
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

// unwindStack: destroy the objects of every registration above the handler's, then those of the handler's block down to its state.
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

// dispatchable: search registration rw's state table for a matching catch or a violated exception specification (re-throws use the active exception's type); records the handler's state.
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

// getActiveExc: the thread's current exception, after checking its state (fatal error or terminate if a throw is not allowed now).
// FUNCTION: 0xf430c
active_exc *getActiveExc(thread_ctl *thr)
{
    active_exc *exc;

    exc = thr->excepts;
    if (exc == 0) {
        if (thr->flags & 1)
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_throw_in_terminate_1, 1);
    } else {
        switch (exc->state) {
        case 0:
            thr->abort_msg = stabthrw_msg_throw_in_unwind_dtor;
            callTerminate(thr);
        case 3:
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_throw_in_terminate_2, 1);
        case 5:
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_throw_in_ctor, 1);
        case 6:
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_throw_in_handled_dtor, 1);
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

// dispatch_init: start dispatching a throw of type ro (NULL = re-throw), zero set for a thrown 0 constant.
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

// __wcpp_2_dispatchable_: check function registration rw for a handler (other kinds have none); terminates if the registrations are exhausted.
// FUNCTION: 0xf43c9
DISPATCHABLE __wcpp_2_dispatchable_(dispatch_exc *dispatch, rw_dtreg *rw)
{
    unsigned kind;

    if (rw == 0) {
        if (dispatch->rethrow) {
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_rethrow_no_handler_1, 1);
        } else {
            dispatch->thr->abort_msg = stabthrw_msg_no_handler_1;
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

// __wcpp_2_save_exc_: make the thrown object (or the re-thrown exception) the current exception, finishing newer ones, and record the handler.
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

// __wcpp_2_dispatch_: transfer control to the handler found (kind), or terminate / report an unhandled exception; does not return.
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
            __wcpp_2_fatal_runtime_error_(stabthrw_msg_rethrow_no_handler_2, 1);
        } else {
            dispatch->thr->abort_msg = stabthrw_msg_no_handler_2;
            callTerminate(dispatch->thr);
        }
    case DISPATCHABLE_SYS_EXC:
        strcpy(buf, stabthrw_msg_system_exception);
        ltoa(dispatch->code, buf + strlen(buf) - 8, 16);
        __wcpp_2_fatal_runtime_error_(buf, 1);
    default:
        __wcpp_2_corrupted_stack_();
    }
}

// processThrow: throw object of type ro: find a handler, save the exception, unwind the stack and dispatch.
// FUNCTION: 0xf4551
void processThrow(void *object, THROW_RO *ro, int zero)
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

// __wcpp_2_rethrow_: "throw;" — re-throw the current exception.
// FUNCTION: 0xf4598
void __wcpp_2_rethrow_(void)
{
    processThrow(0, 0, 0);
}

// __wcpp_2_throw_: throw object of type ro.
// FUNCTION: 0xf45a8
void __wcpp_2_throw_(void *object, THROW_RO *ro)
{
    processThrow(object, ro, 0);
}

// __wcpp_2_throw_zero_: throw object of type ro, a 0 constant (convertible to any pointer).
// FUNCTION: 0xf45b2
// NONMATCHING: the target jumps into __wcpp_2_throw_'s call, so the module
// was built in one segment; compiling it with -zm- reproduces this but moves
// getActiveExc's jump table until dispatchable matches
void __wcpp_2_throw_zero_(void *object, THROW_RO *ro)
{
    processThrow(object, ro, 1);
}

// longjmpExcPop: step dispatch->exc past the exceptions caught by the active catch blocks of registration rw.
// FUNCTION: 0xf45ba
void longjmpExcPop(dispatch_exc *dispatch, rw_dtreg *rw)
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

// longjmpDtoring: longjmp hook; destroy registered objects and finish exceptions above bound before the jump.
// FUNCTION: 0xf4605
void longjmpDtoring(rw_dtreg *bound)
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
        longjmpExcPop(&dispatch, rw);
    }
    unwindExc(&dispatch);
    unwindStack(&dispatch);
}

// __wcpp_2_lj_init_: install longjmpDtoring as the longjmp handler.
// FUNCTION: 0xf46a5
void __wcpp_2_lj_init_(void)
{
    __longjmp_handler = (void (*)(rw_dtreg *))longjmpDtoring;
}

}
