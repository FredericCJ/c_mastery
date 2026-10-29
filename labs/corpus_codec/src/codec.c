#include "codec.h"
/* Intentionally unfinished learner implementation. */
int cc_lengths(const uint16_t f[16],uint8_t l[16]){(void)f;(void)l;return CC_INVALID;}
int cc_book(const uint8_t l[16],struct cc_book *b){(void)l;(void)b;return CC_INVALID;}
#define STUB(name) int name(const uint8_t *i,size_t n,uint8_t *o,size_t c,size_t *u) \
{(void)i;(void)n;(void)o;(void)c;(void)u;return CC_INVALID;}
STUB(cc_hencode)
STUB(cc_hdecode)
STUB(cc_lzencode)
STUB(cc_lzoptimal)
STUB(cc_lzdecode)
STUB(cc_compress)
STUB(cc_compress_optimal)
STUB(cc_expand)
