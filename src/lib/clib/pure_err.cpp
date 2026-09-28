extern "C" {

extern char D_00124de4[];
extern short D_001516fc;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// FUNCTION: 0xd12e0
// NONMATCHING: config/functions.txt starts __wcpp_2_pure_error__ 10 bytes
// early, on the preceding function (the real start is 0xd12ea)
void __wcpp_2_pure_error_(void)
{
    if (!D_001516fc) {
        D_001516fc = 1;
        __wcpp_2_fatal_runtime_error_(D_00124de4, 1);
    }
}

}
