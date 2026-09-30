extern "C" {

extern short D_001516fc;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// FUNCTION: 0xd12ea
void __wcpp_2_pure_error_(void)
{
    if (!D_001516fc) {
        D_001516fc = 1;
        __wcpp_2_fatal_runtime_error_("pure virtual function called!", 1);
    }
}

}
