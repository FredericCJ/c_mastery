#ifndef LP_DECLARATIONS_H
#define LP_DECLARATIONS_H
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT==8,"selected wire uses octets");
enum ld_status {LD_OK,LD_INVALID};
/* Live, disjoint, writable output. Refusal preserves it; inputs are copied.
   Two octets, high three bits zero, signed 13-bit two's-complement wire value.
   Neither a C object representation nor a bit-precise ABI is exposed. */
enum ld_status ld_encode(int32_t value,unsigned char out[2]);
enum ld_status ld_decode(const unsigned char in[2],int32_t *out);
/* Exact n*(n+1)/2 for n<=65535, no allocator or implicit wide arithmetic. */
enum ld_status ld_triangle(uint32_t n,uint32_t *out);
#endif
