#ifndef LP_FACILITIES_H
#define LP_FACILITIES_H
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT==8,"selected octet profile");
typedef enum {LF_OK,LF_INVALID} LfStatus;
typedef struct {unsigned leading,trailing,width;} LfBits;
/* Live disjoint output, unchanged on refusal; no retained borrow. */
LfStatus lf_add(int32_t a,int32_t b,int32_t *out);
LfStatus lf_mixed(int32_t a,uint32_t b,uint16_t *out);
LfStatus lf_bits(uint32_t value,LfBits *out);
LfStatus lf_div(uint64_t a,uint64_t b,uint64_t *out);
/* bits: language1,header2,link4,run8,no-implicit-helper16. strict is0/1. */
LfStatus lf_select(unsigned observations,unsigned strict,unsigned *enhanced);
#endif
