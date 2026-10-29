#ifndef LP_FACILITIES_H
#define LP_FACILITIES_H
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "selected octet profile");

enum lf_status
{
    LF_OK,
    LF_INVALID
};

struct lf_bits
{
    unsigned leading, trailing, width;
};
/* Live disjoint output, unchanged on refusal; no retained borrow. */
enum lf_status lf_add(int32_t a, int32_t b, int32_t *out);
enum lf_status lf_mixed(int32_t a, uint32_t b, uint16_t *out);
enum lf_status lf_bits(uint32_t value, struct lf_bits *out);
enum lf_status lf_div(uint64_t a, uint64_t b, uint64_t *out);
/* bits: language1,header2,link4,run8,no-implicit-helper16. strict is0/1. */
enum lf_status lf_select(unsigned observations, unsigned strict,
                         unsigned *enhanced);
#endif
