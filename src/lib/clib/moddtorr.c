#include "rtinit.h"

/* Destroys the C++ modules' static objects at exit. */

void __wcpp_2_module_dtor__(void);
#pragma aux __wcpp_2_module_dtor__ "*"

struct rt_init __based(__segname("YI")) __wcpp_2_data_module_dtor_ref_ = { 0, 40, __wcpp_2_module_dtor__ };
