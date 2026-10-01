extern "C" {


void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// Reports a call to an undefined member function made through debug
// information as a fatal runtime error.
// FUNCTION: 0xc9360
void fn_000c9360(void)
{
    __wcpp_2_fatal_runtime_error_("undefined member function called through debug information!", 1);
}

}
