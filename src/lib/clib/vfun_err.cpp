extern "C" {

extern char D_001257bc[];
extern short D_001516fe;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// FUNCTION: 0xd2b7b
void __wcpp_2_undef_vfun_(void)
{
    if (!D_001516fe) {
        D_001516fe = 1;
        __wcpp_2_fatal_runtime_error_(D_001257bc, 1);
    }
}

}
