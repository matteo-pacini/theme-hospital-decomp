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

}

void terminate(void);
void unexpected(void);
#pragma aux unexpected "W?unexpected$n()v"

// FUNCTION: 0xfc770
void unexpected(void)
{
    void (*handler)(void);

    handler = _wint_thread_data.unexpected;
    if (handler == 0) {
        terminate();
    } else {
        handler();
        __wcpp_2_fatal_runtime_error_("return from \"unexpected\" function!", 1);
    }
}
