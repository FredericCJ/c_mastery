#include "fractional.h"
int rzf_bucket_init(RzfBucket *b,uint64_t c,uint64_t p,uint64_t q,uint64_t n){(void)b;(void)c;(void)p;(void)q;(void)n;return RZ_INVALID;}
int rzf_bucket_consume(RzfBucket *b,uint64_t n,uint64_t c){(void)b;(void)n;(void)c;return RZ_INVALID;}
int rzf_bucket_pair(RzfBucket *a,RzfBucket *p,uint64_t n,uint64_t c){(void)a;(void)p;(void)n;(void)c;return RZ_INVALID;}
int rzf_credit_export(const RzfBucket *b,uint64_t n,RzfCredit *o){(void)b;(void)n;(void)o;return RZ_INVALID;}
int rzf_credit_import(const RzfCredit *c,uint64_t n,uint64_t d,RzfDowntime p,RzfBucket *o){(void)c;(void)n;(void)d;(void)p;(void)o;return RZ_INVALID;}
