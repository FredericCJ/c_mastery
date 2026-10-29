#include "target.h"
#include <stdint.h>

int df_target(void)
{
    return 99;
}

void df_handler(uint32_t *words, uint32_t exc_return)
{
    (void)words;
    (void)exc_return;
    for (;;)
        __asm__ volatile("wfi");
}
