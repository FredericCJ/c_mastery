#ifndef B02_FOLLOW_PARSE_H
#define B02_FOLLOW_PARSE_H
#include "depth.h"
enum { BF_CANCELLED = 7, BF_CLOSED = 8, BF_UNIMPLEMENTED = 9 };
typedef struct {
  unsigned base, phase, negative, seen, zero, invalid, range;
  uint32_t magnitude;
} BfParse;
/* Fresh or exclusively owned parser; reset discards tentative state. No borrows
 * survive feed. Control, input and output storage are disjoint live objects. */
int bf_parse_reset(BfParse *, unsigned base);
int bf_parse_feed(BfParse *, const unsigned char *, size_t);
int bf_parse_finish(BfParse *, int32_t *);
int bf_parse_cancel(BfParse *);
int bf_parse_radix(const unsigned char *, size_t, unsigned, int32_t *);
/* Hosted verification only: strict grammar followed by actual strtol. */
int bf_parse_libc(const unsigned char *, size_t, unsigned, int32_t *);
#endif
