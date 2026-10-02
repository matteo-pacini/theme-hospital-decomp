#ifndef RTINIT_H
#define RTINIT_H

/* An entry of the runtime's initialisation (XI) and termination (YI)
   tables: __InitRtns calls the XI routines at startup and __FiniRtns the
   YI routines at exit, each in order of priority. */
struct rt_init {
    /* 0 as defined; __InitRtns and __FiniRtns set it to 2 once the routine
       has been handled. */
    unsigned char rtn_type;
    /* Lower runs earlier at startup and later at exit. */
    unsigned char priority;
    void (*rtn)(void);
};

#define AXI(routine, priority) static struct rt_init __based(__segname("XI")) _xi_##routine = {0, priority, routine};
#define AYI(routine, priority) static struct rt_init __based(__segname("YI")) _yi_##routine = {0, priority, routine};

#endif
