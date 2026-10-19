#ifndef LD_BOUNDARY_H
#define LD_BOUNDARY_H
#include <stdint.h>
/* Stable explicit standard-type boundary; never pass _BitInt across it. */
int ld_bit_encode(int32_t value,unsigned char out[2]);
int ld_bit_decode(const unsigned char in[2],int32_t *out);
#endif
