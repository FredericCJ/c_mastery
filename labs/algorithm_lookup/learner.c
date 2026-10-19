#include "lookup.h"
enum ALStatus al_lookup(const CDHash*h,uint32_t k,int*v,ALProbe*p){(void)h;(void)k;(void)v;(void)p;return AL_INVALID;}
enum ALStatus al_quadratic(unsigned c,unsigned s,ALProbe*p){(void)c;(void)s;(void)p;return AL_INVALID;}
enum ALStatus al_match(const unsigned char*t,size_t n,const unsigned char*p,size_t m,uint64_t*out,size_t cap,size_t*count,ALMatchWork*w){(void)t;(void)n;(void)p;(void)m;(void)out;(void)cap;(void)count;(void)w;return AL_INVALID;}
enum ALStatus al_publish(const unsigned char*t,size_t n,const unsigned char*p,size_t m,ALPublish f,void*c,ALPublication*out){(void)t;(void)n;(void)p;(void)m;(void)f;(void)c;(void)out;return AL_INVALID;}
enum ALStatus al_stream_init(ALStream*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return AL_INVALID;}
enum ALStatus al_stream_feed(ALStream*s,const unsigned char*p,size_t n,uint64_t*out,size_t cap,size_t*count){(void)s;(void)p;(void)n;(void)out;(void)cap;(void)count;return AL_INVALID;}
