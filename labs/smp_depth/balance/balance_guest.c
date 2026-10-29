/* Implement the affinity and saved-context adapter. */
static void sbg_worker(unsigned c)
{
    (void)c;
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

void sbg_bsp(void)
{
    text("SB_GUEST_CHECK:unsolved-balance-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
