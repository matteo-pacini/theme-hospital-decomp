extern "C" {

struct FREE_AREA {
    FREE_AREA *next;
    unsigned size;
};

struct EXC_AREA {
    unsigned size;
    FREE_AREA *freed;
};

struct active_exc {
    active_exc *prev;
    void *throw_ro;
    void *sig;
    void *extra_object;
    EXC_AREA *area;
};

struct EXC_HDR {
    unsigned size;
    active_exc exc;
};

struct thread_ctl {
    void *registered;
    unsigned char flags;
    char pad[3];
    active_exc *excepts;
};

// FUNCTION: 0xfba7b
void __wcpp_2_free_exc_(thread_ctl *thr)
{
    active_exc *exc;
    EXC_HDR *hdr;
    EXC_AREA *area;
    FREE_AREA **prev;
    FREE_AREA *cur;

    exc = thr->excepts;
    thr->excepts = exc->prev;
    hdr = (EXC_HDR *)((char *)exc - sizeof(unsigned));
    area = hdr->exc.area;
    ((FREE_AREA *)hdr)->size = hdr->size;
    for (prev = &area->freed; (cur = *prev) != 0 && cur <= (FREE_AREA *)hdr; prev = &cur->next)
        ;
    *prev = (FREE_AREA *)hdr;
    ((FREE_AREA *)hdr)->next = cur;
}

// FUNCTION: 0xfbaa8
active_exc *__wcpp_2_pop_exc_(thread_ctl *thr)
{
    active_exc *exc;

    exc = thr->excepts;
    thr->excepts = exc->prev;
    return exc;
}

// NONMATCHING: config/functions.txt extends this function over the next one
// (0xfbac4, a lone ret)
// FUNCTION: 0xfbab7
active_exc *__wcpp_2_push_exc_(thread_ctl *thr, active_exc *exc)
{
    exc->prev = thr->excepts;
    thr->excepts = exc;
    return exc;
}

}
