/* Complete the private-table/context/fault worker using the explicit prerequisite. */
static void sp_worker(unsigned c)
{
    (void)c;
}

void sp_bsp(void)
{
    text("SP_GUEST_CHECK:unsolved-percpu-adapter\n");
    unsigned v = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(v));
    for (;;)
        __asm__ volatile("cli;hlt");
}
