/* Implement the saved-context adapter and the stated experiment. */
static void swg_worker(unsigned c)
{
    (void)c;
    for (;;)
        __asm__ volatile("cli;hlt");
}

void swg_bsp(void)
{
    text("SW_GUEST_CHECK:unsolved-wakeup-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
        __asm__ volatile("cli;hlt");
}
