#include "cmk2_abi.h"

void _start(void)
{
    static const char marker[] =
        "SP_GUEST_CHECK:unsolved-pool-target-adapter\n";
    cmk_write(marker, sizeof marker - 1);
    cmk_call(CMK_EXIT, 91, 0, 0);
    for (;;)
    {
    }
}
