void __do_exit_with_msg_(const char *msg, unsigned retcode);
int __EnterWVIDEO(const char __far *msg);

// __exit_with_msg: print msg and exit with retcode (__do_exit_with_msg_).
// FUNCTION: 0xf2e91
void __exit_with_msg(const char *msg, unsigned retcode)
{
    __do_exit_with_msg_(msg, retcode);
}

// __fatal_runtime_error: report msg to the debugger if present, else print it and exit with retcode.
// FUNCTION: 0xf2e96
void __fatal_runtime_error(const char *msg, unsigned retcode)
{
    if (__EnterWVIDEO(msg) == 0)
        __do_exit_with_msg_(msg, retcode);
}
