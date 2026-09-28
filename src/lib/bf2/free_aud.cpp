#include "snd.h"

extern "C" {

extern u8 D_00176b70;

void fn_000bbb68(void);
void fn_000bbb98(void);
void fn_000dc820(void);
void fn_000dc8d0(void);

// FUNCTION: 0xc20f0
void fn_000c20f0(void)
{
    if (D_00176b70)
        fn_000bbb68();
    fn_000bbb98();
    fn_000dc820();
    fn_000dc8d0();
}

}
