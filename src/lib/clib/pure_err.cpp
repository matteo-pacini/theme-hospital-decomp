extern "C" {

extern char D_00124de4[];
extern short D_001516fc;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// FUNCTION: 0xd12ea
void fn_000d12ea(void)
{
    if (!D_001516fc) {
        D_001516fc = 1;
        __wcpp_2_fatal_runtime_error_(D_00124de4, 1);
    }
}

}
