#include "register_rules.h"
#include <stdio.h>

/*
 * USART2 status register, modelled conservatively: bits 9:0 are all treated
 * as write-zero-to-clear flags, and the word written for "clear TC" must
 * clear only TC (bit 6) and leave bits 0-5 and 7-9 alone. On the real USART_SR
 * only TC, RXNE, LBD and CTS are rc_w0; PE, FE, NF, ORE, IDLE and TXE are
 * read-only, so writing zero to them has no effect. Bits 31:10 are reserved
 * and must be written as zero. Exit status: 0 pass, 1 rejected (with a
 * diagnostic line).
 * With -DP4_BAD_STATUS_MASK=1 the rule header returns the known-bad word.
 */
enum
{
    STATUS_BITS = 10,
    STATUS_STATES = 1 << STATUS_BITS, /* every combination of bits 9:0 */
    TC_BIT = 64,
    WRITABLE_MASK = 0x3ff, /* bits 9:0, all modelled as write-zero-to-clear */
    EXPECTED_AFTER_CLEAR = 0x3bf /* all modelled bits except TC */
};

int main(void)
{
    uint32_t word = p4_tc_clear_word();
    if ((word & ~(uint32_t)WRITABLE_MASK) || (word & TC_BIT))
    {
        puts("P4_STATUS_REJECT reserved/TC bits");
        return 1;
    }
    for (uint32_t flags = 0; flags < STATUS_STATES; flags++)
    {
        /* Independent write-zero-to-clear flag model; no hardware observation. */
        uint32_t after = flags & word;
        if (after != (flags & (uint32_t)EXPECTED_AFTER_CLEAR))
        {
            puts("P4_STATUS_REJECT unrelated flags");
            return 1;
        }
    }
    puts("P4_STATUS_PASS states=1024 reserved_zero");
    return 0;
}
