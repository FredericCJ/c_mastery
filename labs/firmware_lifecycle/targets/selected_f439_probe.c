/* Selected single-core startup diagnostic, not a two-core bootloader. */
#include "selected_startup.h"
volatile uint32_t st_probe;

_Noreturn void st_main(void)
{
    st_probe = st_cold_f439(st_target_io()) ? 0x46343339u : 0;
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
