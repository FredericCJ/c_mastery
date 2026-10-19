/* Complete the original static range/ownership guest adapter. */
static void srg_worker(unsigned c){(void)c;for(;;)__asm__ volatile("cli;hlt");}
void srg_bsp(void){text("SR_GUEST_CHECK:unsolved-reduction-adapter\n");unsigned code=17;__asm__ volatile("outl %0,$0xf4"::"a"(code));for(;;)__asm__ volatile("cli;hlt");}
