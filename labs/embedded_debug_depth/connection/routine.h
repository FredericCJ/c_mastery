#ifndef CMASTERY_DEBUG_ROUTINE_H
#define CMASTERY_DEBUG_ROUTINE_H
#include <stdint.h>
extern volatile uint32_t dc_input, dc_output, dc_counter, dc_done;
extern unsigned char dc_bytes[4];
uint32_t dc_checksum(const unsigned char *, unsigned);
uint32_t dc_tail(uint32_t);
uint32_t dc_leaf(uint32_t);
void dc_checkpoint(void);
#endif
