extern "C" {

extern short _wint_pure_error_flag;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);

// __wcpp_2_pure_error_: called for a pure virtual call; fatal "pure virtual function called!" error (once).
// FUNCTION: 0xd12ea
void __wcpp_2_pure_error_(void)
{
    if (!_wint_pure_error_flag) {
        _wint_pure_error_flag = 1;
        __wcpp_2_fatal_runtime_error_("pure virtual function called!", 1);
    }
}

}
