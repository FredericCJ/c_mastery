/* Original compiler-ABI sensitivity target: change the declared global guard,
 * not a return address or any out-of-bounds object. Hosted observation only. */
#include "target.h"
#include <stdio.h>
#include <stdlib.h>
uintptr_t __stack_chk_guard = UINT32_C(0x63c291a7);

void sh_target_touch(volatile unsigned char *p)
{
    (void)*p;
    __stack_chk_guard ^= UINT32_C(1);
}

_Noreturn void __stack_chk_fail(void)
{
    if (puts("OBS actual_guard_mismatch_handler") < 0)
        _Exit(43);
    if (fflush(stdout))
        _Exit(43);
    _Exit(42);
}

int main(void)
{
    (void)sh_target_probe(7);
    return 3;
}
