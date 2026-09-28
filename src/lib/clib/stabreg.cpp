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

// FUNCTION: 0xec324
// NONMATCHING: config/functions.txt extends __wcpp_2_base_register__ over
// __wcpp_2_base_deregister__ (0xec33d)
THREAD_CTL *__wcpp_2_base_register_(RW_DTREG *rw, void *ro, unsigned state_var)
{
    rw->ro = ro;
    rw->state_var = state_var;
    rw->prev = _wint_thread_data.registered;
    _wint_thread_data.registered = rw;
    return &_wint_thread_data;
}

void __wcpp_2_base_deregister_(void)
{
    _wint_thread_data.registered = _wint_thread_data.registered->prev;
}

}
