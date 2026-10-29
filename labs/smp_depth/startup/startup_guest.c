/* Appended to the supplied completed CMK source in a private build copy. */
void cmk2_smp_run(unsigned info)
{
    (void)info;
    text("SS_GUEST_CHECK:unsolved-startup-adapter\n");
    unsigned value = 17;
    __asm__ volatile("outl %0,$0xf4" ::"a"(value));
    for (;;)
    {
        __asm__ volatile("cli;hlt");
    }
}
