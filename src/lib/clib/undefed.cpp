extern "C" {


void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// __wcpp_2_undefed_cdtor_: fatal "undefined constructor or destructor called!" error.
// FUNCTION: 0xccca8
void __wcpp_2_undefed_cdtor_(void)
{
    __wcpp_2_fatal_runtime_error_("undefined constructor or destructor called!", 1);
}

}
