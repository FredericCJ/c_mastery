#ifndef P4_REGISTER_RULES_H
#define P4_REGISTER_RULES_H
#include <stdint.h>
#ifndef P4_BAD_STATUS_MASK
#define P4_BAD_STATUS_MASK 0
#endif
/* RM0090 Rev21 p1011: bits31:10 reserved at reset0; TC6 clears on0.
   Ones preserve the other writable status flags of this transmit-only port. */
static inline uint32_t p4_tc_clear_word(void){return P4_BAD_STATUS_MASK?~UINT32_C(0x40):UINT32_C(0x3bf);}
#endif
