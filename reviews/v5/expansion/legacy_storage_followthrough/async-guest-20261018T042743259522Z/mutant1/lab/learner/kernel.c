/* PR-T53 and PR-T56: implement the kernel in independently testable milestones.
 * This intentionally contains no functioning kernel or solved mechanisms. */
void kernel_main(unsigned magic, unsigned boot_info) {
  (void)magic;
  (void)boot_info;
  for (;;)
    __asm__ volatile("hlt");
}
