void __do_exit_with_msg_(const char *msg, unsigned retcode);
int __EnterWVIDEO(const char __far *msg);

// FUNCTION: 0xf2e91
void __exit_with_msg(const char *msg, unsigned retcode)
{
    __do_exit_with_msg_(msg, retcode);
}

// FUNCTION: 0xf2e96
void __fatal_runtime_error(const char *msg, unsigned retcode)
{
    if (__EnterWVIDEO(msg) == 0)
        __do_exit_with_msg_(msg, retcode);
}
