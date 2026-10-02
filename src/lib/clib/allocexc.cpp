extern "C" {

void *memcpy(void *dst, const void *src, unsigned n);
#pragma intrinsic(memcpy)

struct FREE_AREA {
    FREE_AREA *next;
    unsigned size;
};

struct EXC_AREA {
    unsigned size;
    FREE_AREA *freed;
    FREE_AREA first;
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
    EXC_AREA *area;
    void *rw;
    void *cmd;
    unsigned char state;
    unsigned char zero_thrown : 1;
    char data[1];
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

extern EXC_AREA __EXC_AREA;
extern char allocexc_msg_no_memory[];

unsigned __wcpp_2_ts_size_(RT_TYPE_SIG *sig);
void __wcpp_2_fatal_runtime_error_(char *msg, int code);

#define EXCSTATE_CTOR 5

// NONMATCHING: size and the hoisted size + 8 swap edi/ebp, the found block
// stays in ebx instead of moving to ebp, and the rounding mask is applied
// as and di,0xfffc in the original
// __wcpp_2_alloc_exc_: allocate an active exception in the exception area, push it on thr's list and copy-construct the thrown object into it; fatal error if the area is full.
// FUNCTION: 0xfb940
active_exc *__wcpp_2_alloc_exc_(void *object, THROW_RO *ro, thread_ctl *thr)
{
    RT_TYPE_SIG *sig;
    unsigned size;
    FREE_AREA **prev;
    FREE_AREA *cur;
    FREE_AREA *next;
    EXC_HDR *hdr;
    active_exc *exc;

    if (__EXC_AREA.freed == 0) {
        __EXC_AREA.first.next = 0;
        __EXC_AREA.freed = &__EXC_AREA.first;
        __EXC_AREA.first.size = __EXC_AREA.size - sizeof(FREE_AREA);
    }
    sig = ro->cnvs[0].sig;
    if (sig->type == TS_POINTER)
        sig = sig->base;
    size = __wcpp_2_ts_size_(sig) + (sizeof(EXC_HDR) - 1);
    if (size < sizeof(FREE_AREA))
        size = sizeof(FREE_AREA);
    size = (size + 3) & ~3;
    for (prev = &__EXC_AREA.freed;; prev = &cur->next) {
        cur = *prev;
        if (cur == 0)
            __wcpp_2_fatal_runtime_error_(allocexc_msg_no_memory, 1);
        for (;;) {
            next = cur->next;
            if ((FREE_AREA *)((char *)cur + cur->size) != next)
                break;
            cur->size += next->size;
            cur->next = next->next;
        }
        if (size == cur->size) {
            hdr = (EXC_HDR *)cur;
            *prev = next;
            break;
        }
        if (size + sizeof(FREE_AREA) < cur->size) {
            cur->size -= size;
            hdr = (EXC_HDR *)((char *)cur + cur->size);
            break;
        }
    }
    hdr->size = size;
    hdr->exc.area = &__EXC_AREA;
    exc = &hdr->exc;
    exc->prev = thr->excepts;
    thr->excepts = exc;
    exc->sig = ro->cnvs[0].sig;
    exc->state = EXCSTATE_CTOR;
    exc->zero_thrown = 0;
    exc->throw_ro = ro;
    switch (sig->type) {
    case TS_SCALAR:
    case TS_SCALAR_KIND8:
        memcpy(exc->data, object, sig->size);
        break;
    case TS_CLASS:
        ((void (*)(void *, void *))sig->clss.ctor)(exc->data, object);
        break;
    case TS_CLASS_ARRAY:
        ((void (*)(void *, int, void *))sig->clss.ctor)(exc->data, 0, object);
        break;
    case TS_POINTER_SCALAR:
    case TS_POINTER_KIND6:
    case TS_POINTER_KIND7:
        memcpy(exc->data, object, sizeof(void *));
        break;
    }
    return exc;
}

// __wcpp_2_free_exc_: pop thr's current exception and return its storage to the area's address-ordered free list.
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

// __wcpp_2_pop_exc_: unlink and return thr's current exception.
// FUNCTION: 0xfbaa8
active_exc *__wcpp_2_pop_exc_(thread_ctl *thr)
{
    active_exc *exc;

    exc = thr->excepts;
    thr->excepts = exc->prev;
    return exc;
}

// __wcpp_2_push_exc_: make exc thr's current exception; returns exc.
// FUNCTION: 0xfbab7
active_exc *__wcpp_2_push_exc_(thread_ctl *thr, active_exc *exc)
{
    exc->prev = thr->excepts;
    thr->excepts = exc;
    return exc;
}

}
