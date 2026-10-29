#include <stdint.h>

/* Deliberately incomplete executable kernel boundary. Supply your own kernel
 * and trap dispatcher to build_guest.py; the supplied tests are not a kernel. */
void *kf_dispatch(void *frame)
{
    return frame;
}

void kernel_main(unsigned magic, unsigned info)
{
    (void)magic;
    (void)info;
    for (;;)
        __asm__ volatile("cli; hlt");
}
