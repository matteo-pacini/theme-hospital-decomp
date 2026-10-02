extern "C" {

extern short _wint_undef_vfun_flag;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// __wcpp_2_undef_vfun_: fatal "eliminated virtual function call" error (once).
// FUNCTION: 0xd2b7b
void __wcpp_2_undef_vfun_(void)
{
    if (!_wint_undef_vfun_flag) {
        _wint_undef_vfun_flag = 1;
        __wcpp_2_fatal_runtime_error_("compiler error: eliminated virtual function call!", 1);
    }
}

}
