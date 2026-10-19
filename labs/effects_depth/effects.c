#include "effects.h"
enum EFStatus ef_checksum(const unsigned char*p,size_t n,uint32_t*out){(void)p;(void)n;(void)out;return EF_INVALID;}
enum EFStatus ef_cached(EFCache*c,const unsigned char*p,size_t n,uint32_t*out){(void)c;(void)p;(void)n;(void)out;return EF_INVALID;}
enum EFStatus ef_cache_reset(EFCache*c){(void)c;return EF_INVALID;}
enum EFStatus ef_cache_borrow(EFCache*c,const unsigned char**out){(void)c;(void)out;return EF_INVALID;}
enum EFStatus ef_cache_return(EFCache*c){(void)c;return EF_INVALID;}
enum EFStatus ef_pool_take(EFPool*p,unsigned char**out){(void)p;(void)out;return EF_INVALID;}
enum EFStatus ef_pool_return(EFPool*p,unsigned index){(void)p;(void)index;return EF_INVALID;}
enum EFStatus ef_parse(EFParser*p,const unsigned char*d,size_t n,EFView*out){(void)p;(void)d;(void)n;(void)out;return EF_INVALID;}
enum EFStatus ef_view_release(EFView*v){(void)v;return EF_INVALID;}
enum EFStatus ef_parser_reset(EFParser*p,uint32_t seed){(void)p;(void)seed;return EF_INVALID;}
uint32_t ef_random(EFParser*p){(void)p;return 0;}
enum EFStatus ef_attempt(EFCallback f,void*ctx,unsigned nonblocking,unsigned attempts,unsigned*used){(void)f;(void)ctx;(void)nonblocking;(void)attempts;(void)used;return EF_INVALID;}
enum EFStatus ef_sort(int*a,size_t n){(void)a;(void)n;return EF_INVALID;}
enum EFStatus ef_sort_records(EFRecord*a,size_t n){(void)a;(void)n;return EF_INVALID;}
