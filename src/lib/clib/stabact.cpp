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

struct TYPE_SIG_CLASS {
    char type;
    void (*ctor)(void *);
    void *copier;
    void *dtor;
    unsigned size;
};

THREAD_CTL *__wcpp_2_arr_register_(void *array, RW_DTREG_ARRAY *rw, TYPE_SIG_CLASS *sig);

// FUNCTION: 0xed300
void *__wcpp_2_ctor_array_(void *array, unsigned count, TYPE_SIG_CLASS *sig)
{
    void (*ctor)(void *);
    unsigned size;
    THREAD_CTL *thr;
    RW_DTREG_ARRAY rw;

    thr = __wcpp_2_arr_register_(array, &rw, sig);
    ctor = sig->ctor;
    size = sig->size;
    for (;;) {
        if (count <= rw.count)
            break;
        ctor(array);
        ++rw.count;
        array = (char *)array + size;
    }
    thr->registered = thr->registered->prev;
    return rw.array;
}

}
