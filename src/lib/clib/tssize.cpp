extern "C" {

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

#pragma pack(1)
struct RT_TYPE_SIG {
    TS_TYPE type;
    union {
        RT_TYPE_SIG *base;
        unsigned char size;
        struct {
            unsigned flags;
            void (*ctor)();
            void (*dtor)(void *, unsigned);
            unsigned size;
        } clss;
    };
};
#pragma pack()

void __wcpp_2_corrupted_stack_(void);

// FUNCTION: 0xfbcf0
unsigned __wcpp_2_ts_size_(RT_TYPE_SIG *sig)
{
    unsigned size;

    if (sig->type == TS_POINTER)
        sig = sig->base;
    switch (sig->type) {
    case 3:
    case 8:
        size = sig->size;
        break;
    case 0:
    case 4:
        size = sig->clss.size;
        break;
    case 1:
    case 6:
    case 7:
        size = 4;
        break;
    default:
        __wcpp_2_corrupted_stack_();
    }
    return size;
}

}
