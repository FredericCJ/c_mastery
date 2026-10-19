/* Implement the saved-context adapter and the stated experiment. */
static void sqg_worker(unsigned c){(void)c;for(;;)__asm__ volatile("cli;hlt");}
void sqg_bsp(void){text("SQ_GUEST_CHECK:unsolved-queue-adapter\n");unsigned value=17;__asm__ volatile("outl %0,$0xf4"::"a"(value));for(;;)__asm__ volatile("cli;hlt");}
