/* Link-only stand-ins. Replace these with completed T14/T15 learner artifacts. */
#include "lookup.h"
enum CDStatus cd_hash_init(CDHash*h,unsigned n,CDHashAllocator a,unsigned c){(void)h;(void)n;(void)a;(void)c;return CD_INVALID;}
enum CDStatus cd_hash_put(CDHash*h,uint32_t k,int v,unsigned*p){(void)h;(void)k;(void)v;(void)p;return CD_INVALID;}
enum CDStatus cd_hash_get(const CDHash*h,uint32_t k,int*v,unsigned*p){(void)h;(void)k;(void)v;(void)p;return CD_INVALID;}
enum CDStatus cd_hash_delete(CDHash*h,uint32_t k){(void)h;(void)k;return CD_INVALID;}
enum CDStatus cd_hash_destroy(CDHash*h){(void)h;return CD_INVALID;}
enum ADStatus ad_match_init(ADMatcher*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AD_INVALID;}
enum ADStatus ad_match_feed(ADMatcher*s,const unsigned char*p,size_t n,uint64_t*out,size_t cap,size_t*count){(void)s;(void)p;(void)n;(void)out;(void)cap;(void)count;return AD_INVALID;}
