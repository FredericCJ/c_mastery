static void sl_worker(unsigned index)
{
    (void)index;
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}

void sl_bsp(void)
{
    text("SL_GUEST_CHECK:unsolved-lock-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
