#ifndef CMASTERY_MACHINE_H
#define CMASTERY_MACHINE_H
#include <stddef.h>
#include <stdint.h>
uint64_t mi_mix(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
                uint64_t, uint64_t);
uint64_t mi_asm_sum(const uint32_t *, size_t);
void *mi_move(void *, const void *, size_t);
int mi_compare(const void *, const void *, size_t);
__extension__ typedef unsigned __int128 mi_u128;
mi_u128 mi_divide(mi_u128, mi_u128);
#endif
