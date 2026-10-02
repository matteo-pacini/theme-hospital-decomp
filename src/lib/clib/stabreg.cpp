extern "C" {

struct RW_DTREG {
    RW_DTREG *prev;
    void *ro;
    unsigned state_var;
};

struct THREAD_CTL {
    RW_DTREG *registered;
};

extern THREAD_CTL _wint_thread_data;

// __wcpp_2_base_register_: push destruction-registration block rw (with ro and state_var) on the thread's list; returns the thread data.
// FUNCTION: 0xec324
THREAD_CTL *__wcpp_2_base_register_(RW_DTREG *rw, void *ro, unsigned state_var)
{
    rw->ro = ro;
    rw->state_var = state_var;
    rw->prev = _wint_thread_data.registered;
    _wint_thread_data.registered = rw;
    return &_wint_thread_data;
}

// __wcpp_2_base_deregister_: pop the thread's newest registration block.
// FUNCTION: 0xec33d
void __wcpp_2_base_deregister_(void)
{
    _wint_thread_data.registered = _wint_thread_data.registered->prev;
}

}
