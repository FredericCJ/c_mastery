#include "register_rules.h"
#include <stdio.h>

int main(void)
{
    uint32_t word = p4_tc_clear_word();
    if ((word & ~UINT32_C(0x3ff)) || (word & 64))
    {
        puts("P4_STATUS_REJECT reserved/TC bits");
        return 1;
    }
    for (uint32_t flags = 0; flags < 1024; flags++)
    {
        /* Independent write-zero-to-clear flag model; no hardware observation. */
        uint32_t after = flags & word;
        if (after != (flags & UINT32_C(0x3bf)))
        {
            puts("P4_STATUS_REJECT unrelated flags");
            return 1;
        }
    }
    puts("P4_STATUS_PASS states=1024 reserved_zero");
}
