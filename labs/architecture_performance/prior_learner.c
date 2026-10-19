#include "bound.h"
/* Supply the actual prior learner artifacts. These compile-only placeholders
 * intentionally provide no working lower-bound/admission implementation. */
int algo_lower(const uint32_t *a,uint32_t n,uint32_t key,uint32_t *out,algo_work *w){(void)a;(void)n;(void)key;(void)out;(void)w;return ALGO_UNIMPLEMENTED;}
int ab_checked(const uint32_t *a,uint32_t n,uint32_t cap,uint32_t key,unsigned trusted,uint32_t *out,uint32_t *checks){(void)a;(void)n;(void)cap;(void)key;(void)trusted;(void)out;(void)checks;return ALGO_UNIMPLEMENTED;}
