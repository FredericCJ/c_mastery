static void si_worker(unsigned index)
{
    (void)index;
    for (;;)
        __asm__ volatile("cli;hlt");
}

void si_bsp(void)
{
    text("SI_GUEST_CHECK:unsolved-interrupt-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}
