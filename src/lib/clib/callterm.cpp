void terminate(void);

extern "C" {

// __wcpp_2_call_terminate_: C entry point for std terminate().
// FUNCTION: 0xfb830
void __wcpp_2_call_terminate_(void)
{
    terminate();
}

}
