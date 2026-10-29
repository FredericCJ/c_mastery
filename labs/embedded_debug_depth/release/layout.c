#include <stdint.h>
volatile uint32_t release_initialized = 0x12345678u;
volatile uint32_t release_zero;

void release_entry(void)
{
    release_zero = release_initialized;
    for (;;)
    {
        __asm__ volatile("nop");
    }
}
