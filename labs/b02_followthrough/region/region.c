#include "region.h"
void bf_cohort_init(BfCohort*c){(void)c;}
int bf_cohort_batch(BfCohort*c,const unsigned char*w,size_t n,unsigned a,unsigned b,unsigned*out){(void)c;(void)w;(void)n;(void)a;(void)b;(void)out;return MB_INVALID;}
int bf_cohort_reset(BfCohort*c){(void)c;return MB_INVALID;}
void bf_tree_init(BfRegionTree*t){(void)t;}
int bf_tree_child_reset(BfRegionTree*t,unsigned i){(void)t;(void)i;return MB_INVALID;}
int bf_tree_reset(BfRegionTree*t){(void)t;return MB_INVALID;}
int bf_slice_init(BfSlice*s,unsigned char*b,size_t n,size_t o,size_t c){(void)s;(void)b;(void)n;(void)o;(void)c;return MB_INVALID;}
int bf_slice_alloc(BfSlice*s,size_t n,size_t a,BfSliceSpan*out){(void)s;(void)n;(void)a;(void)out;return MB_INVALID;}
int bf_slice_read(const BfSlice*s,BfSliceSpan h,size_t i,unsigned char*out){(void)s;(void)h;(void)i;(void)out;return MB_INVALID;}
int bf_slice_reset(BfSlice*s){(void)s;return MB_INVALID;}
int bf_growth_init(BfGrowth*g,BfChunk*c,BfAcquireChunk a,BfReleaseChunk r,void*x){(void)g;(void)c;(void)a;(void)r;(void)x;return MB_INVALID;}
int bf_growth_alloc(BfGrowth*g,size_t n,size_t a,BfGrowthSpan*out){(void)g;(void)n;(void)a;(void)out;return MB_INVALID;}
int bf_growth_read(const BfGrowth*g,BfGrowthSpan s,size_t i,unsigned char*out){(void)g;(void)s;(void)i;(void)out;return MB_INVALID;}
int bf_growth_reset(BfGrowth*g){(void)g;return MB_INVALID;}
void bf_retry_init(BfRetryRegion*r){(void)r;}
int bf_retry_reserve(BfRetryRegion*r,size_t n,size_t a,BfBorrow*out){(void)r;(void)n;(void)a;(void)out;return B2_INVALID;}
int bf_retry_register(BfRetryRegion*r,BfClose f,void*c){(void)r;(void)f;(void)c;return B2_INVALID;}
int bf_retry_reset(BfRetryRegion*r){(void)r;return B2_INVALID;}
