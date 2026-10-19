/* Deliberately unfinished learner implementation; failure is not an author defect. */
#include "api.h"
uint64_t ad_sum64(const uint32_t *p,size_t n){(void)p;(void)n;return 0;}
uint32_t ad_sum32(const uint32_t *p,size_t n){(void)p;(void)n;return 0;}
struct ad_pair ad_pair(uint64_t a,uint64_t b){(void)a;(void)b;return (struct ad_pair){0};}
struct ad_triple ad_triple(uint64_t a,uint64_t b,uint64_t c){(void)a;(void)b;(void)c;return (struct ad_triple){0};}
struct ad_mixed ad_mixed(double r,uint64_t i){(void)r;(void)i;return (struct ad_mixed){0};}
uint64_t ad_nine(uint64_t a,uint64_t b,uint64_t c,uint64_t d,uint64_t e,uint64_t f,uint64_t g,uint64_t h,uint64_t i,double r){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;(void)i;(void)r;return 0;}
uint32_t ad_inline_add(uint32_t a,uint32_t b){(void)a;(void)b;return 0;}
uint32_t ad_named_memory(uint32_t *p){(void)p;return 0;}
uint32_t ad_opaque_memory(uint32_t *p){(void)p;return 0;}
