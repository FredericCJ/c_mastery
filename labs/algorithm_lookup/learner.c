#include "lookup.h"
enum al_status al_lookup(const struct cd_hash*h,uint32_t k,int*v,struct al_probe*p){(void)h;(void)k;(void)v;(void)p;return AL_INVALID;}
enum al_status al_quadratic(unsigned c,unsigned s,struct al_probe*p){(void)c;(void)s;(void)p;return AL_INVALID;}
enum al_status al_match(const unsigned char*t,size_t n,const unsigned char*p,size_t m,uint64_t*out,size_t cap,size_t*count,struct al_match_work*w){(void)t;(void)n;(void)p;(void)m;(void)out;(void)cap;(void)count;(void)w;return AL_INVALID;}
enum al_status al_publish(const unsigned char*t,size_t n,const unsigned char*p,size_t m,al_publish_fn f,void*c,struct al_publication*out){(void)t;(void)n;(void)p;(void)m;(void)f;(void)c;(void)out;return AL_INVALID;}
enum al_status al_stream_init(struct al_stream*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AL_INVALID;}
enum al_status al_stream_feed(struct al_stream*s,const unsigned char*p,size_t n,uint64_t*out,size_t cap,size_t*count){(void)s;(void)p;(void)n;(void)out;(void)cap;(void)count;return AL_INVALID;}
