/* Link-only stand-ins. Replace these with completed T14/T15 learner artifacts. */
#include "lookup.h"
enum cd_status cd_hash_init(struct cd_hash*h,unsigned n,struct cd_hash_allocator a,unsigned c){(void)h;(void)n;(void)a;(void)c;return CD_INVALID;}
enum cd_status cd_hash_put(struct cd_hash*h,uint32_t k,int v,unsigned*p){(void)h;(void)k;(void)v;(void)p;return CD_INVALID;}
enum cd_status cd_hash_get(const struct cd_hash*h,uint32_t k,int*v,unsigned*p){(void)h;(void)k;(void)v;(void)p;return CD_INVALID;}
enum cd_status cd_hash_delete(struct cd_hash*h,uint32_t k){(void)h;(void)k;return CD_INVALID;}
enum cd_status cd_hash_destroy(struct cd_hash*h){(void)h;return CD_INVALID;}
enum ad_status ad_match_init(struct ad_matcher*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AD_INVALID;}
enum ad_status ad_match_feed(struct ad_matcher*s,const unsigned char*p,size_t n,uint64_t*out,size_t cap,size_t*count){(void)s;(void)p;(void)n;(void)out;(void)cap;(void)count;return AD_INVALID;}
