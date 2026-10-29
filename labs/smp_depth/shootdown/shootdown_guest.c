/* Implement the original isolated mapping and permission experiment. */
static void stg_worker(unsigned cpu)
{
    (void)cpu;
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

void stg_bsp(void)
{
    text("ST_GUEST_CHECK:unsolved-shootdown-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
