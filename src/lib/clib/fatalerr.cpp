extern "C" {

int __EnterWVIDEO(char __far *msg);
void __exit_with_msg(char *msg, unsigned retcode);

// __wcpp_2_fatal_runtime_error_: report msg to the debugger if present, else print it and exit with code.
// FUNCTION: 0xe78f0
void __wcpp_2_fatal_runtime_error_(char *msg, int code)
{
    if (!__EnterWVIDEO(msg))
        __exit_with_msg(msg, code);
}

}
