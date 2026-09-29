extern "C" {

struct RW_DTREG {
    RW_DTREG *prev;
    void *ro;
};

struct RW_DTREG_ARRAY {
    RW_DTREG base;
    unsigned count;
    void *array;
    char reserved[16];
};

struct THREAD_CTL {
    RW_DTREG *registered;
};

struct RT_TYPE_SIG;

THREAD_CTL *__wcpp_2_arr_register_(void *array, RW_DTREG_ARRAY *rw, RT_TYPE_SIG *sig);
void __wcpp_2_destruct_internal_(unsigned bound, RW_DTREG_ARRAY *rw);

// FUNCTION: 0xf46b0
void *__wcpp_2_dtor_array_(void *array, unsigned count, RT_TYPE_SIG *sig)
{
    THREAD_CTL *thr;
    RW_DTREG_ARRAY rw;

    thr = __wcpp_2_arr_register_(array, &rw, sig);
    rw.count = count;
    __wcpp_2_destruct_internal_(0, &rw);
    thr->registered = thr->registered->prev;
    return array;
}

}
