/* Complete target binding scaffold; implement st_main without hosted runtime. */
#include "selected_startup.h"

_Noreturn void st_main(void)
{
    for (;;)
        __asm__ volatile("wfi");
}
