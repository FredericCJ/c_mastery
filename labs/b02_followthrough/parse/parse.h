#ifndef B02_FOLLOW_PARSE_H
#define B02_FOLLOW_PARSE_H
#include "depth.h"

enum
{
    BF_CANCELLED = 7,
    BF_CLOSED = 8,
    BF_UNIMPLEMENTED = 9
};

struct bf_parse
{
    unsigned base, phase, negative, seen, zero, invalid, range;
    uint32_t magnitude;
};

/* Fresh or exclusively owned parser; reset discards tentative state. No borrows
 * survive feed. Control, input and output storage are disjoint live objects. */
int bf_parse_reset(struct bf_parse *, unsigned base);
int bf_parse_feed(struct bf_parse *, const unsigned char *, size_t);
int bf_parse_finish(struct bf_parse *, int32_t *);
int bf_parse_cancel(struct bf_parse *);
int bf_parse_radix(const unsigned char *, size_t, unsigned, int32_t *);
/* Hosted verification only: strict grammar followed by actual strtol. */
int bf_parse_libc(const unsigned char *, size_t, unsigned, int32_t *);
#endif
