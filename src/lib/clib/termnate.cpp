extern "C" {

struct thread_ctl {
    void *registered;
    unsigned char flags;
    char pad[3];
    void *excepts;
    char *abort_msg;
    void (*unexpected)(void);
    void (*terminate)(void);
};

extern thread_ctl _wint_thread_data;

void __wcpp_2_fatal_runtime_error_(char *msg, int code);
void __exit(int status);
#pragma aux __exit aborts;

}

// terminate: call the installed terminate handler (fatal error if it returns); without one, exit with the pending abort message, or with status 1.
// FUNCTION: 0xfc724
void terminate(void)
{
    void (*handler)(void);
    thread_ctl *thr;

    handler = _wint_thread_data.terminate;
    if (handler == 0) {
        thr = &_wint_thread_data;
        if (thr->abort_msg == 0)
            __exit(1);
        __wcpp_2_fatal_runtime_error_(thr->abort_msg, 1);
    } else {
        handler();
        __wcpp_2_fatal_runtime_error_("return from \"terminate\" function!", 1);
    }
}
