#include "free.h"
/* Intentionally incomplete learner scaffold; reference implementations are
 * withheld. A semantic test failure is expected until the learner implements it. */
void *fr_copy(void *d,const void *s,size_t n){(void)s;(void)n;return d;}
void *fr_move(void *d,const void *s,size_t n){(void)s;(void)n;return d;}
void *fr_set(void *d,unsigned char x,size_t n){(void)x;(void)n;return d;}
int fr_compare(const void *a,const void *b,size_t n){(void)a;(void)b;(void)n;return 0;}
enum fr_status fr_arena_copy(unsigned char *a,size_t c,size_t d,size_t s,size_t n){(void)a;(void)c;(void)d;(void)s;(void)n;return FR_UNSUPPORTED;}
enum fr_status fr_arena_move(unsigned char *a,size_t c,size_t d,size_t s,size_t n){(void)a;(void)c;(void)d;(void)s;(void)n;return FR_UNSUPPORTED;}
enum fr_status fr_divmod128(struct fr_u128 n,struct fr_u128 d,struct fr_u128 *q,struct fr_u128 *r){(void)n;(void)d;(void)q;(void)r;return FR_UNSUPPORTED;}
enum fr_status fr_divstep128(struct fr_u128 r,struct fr_u128 d,unsigned bit,struct fr_u128 *out,unsigned *q){(void)r;(void)d;(void)bit;(void)out;(void)q;return FR_UNSUPPORTED;}
