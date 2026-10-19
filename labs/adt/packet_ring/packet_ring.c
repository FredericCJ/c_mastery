#include "packet_ring.h"
AdpStatus adp_init(AdpRing*r,size_t a,uint32_t b){(void)r;(void)a;(void)b;return ADP_INVALID;}
AdpStatus adp_feed(AdpRing*r,const unsigned char*p,size_t n){(void)r;(void)p;(void)n;return ADP_INVALID;}
AdpStatus adp_discard(AdpRing*r,size_t n){(void)r;(void)n;return ADP_INVALID;}
AdpStatus adp_acquire(AdpRing*r,size_t n,AdpLease*out){(void)r;(void)n;(void)out;return ADP_INVALID;}
AdpStatus adp_validate(const AdpRing*r,AdpToken t){(void)r;(void)t;return ADP_INVALID;}
AdpStatus adp_cancel(AdpRing*r,AdpToken t){(void)r;(void)t;return ADP_INVALID;}
AdpStatus adp_release(AdpRing*r,AdpToken t){(void)r;(void)t;return ADP_INVALID;}
AdpStatus adp_close(AdpRing*r){(void)r;return ADP_INVALID;}
AdpStatus adp_copy(const AdpRing*r,AdpLease v,unsigned char*p,size_t n){(void)r;(void)v;(void)p;(void)n;return ADP_INVALID;}
AdpStatus adp_record(const AdpRing*r,AdpLease v,unsigned ver,ArRecord*out){(void)r;(void)v;(void)ver;(void)out;return ADP_INVALID;}
AdpStatus adp_parse(const unsigned char*p,size_t n,ADPacket*out){(void)p;(void)n;(void)out;return ADP_INVALID;}
AdpStatus adp_build(unsigned kind,const unsigned char*p,size_t n,unsigned char*out,size_t cap,size_t*used){(void)kind;(void)p;(void)n;(void)out;(void)cap;(void)used;return ADP_INVALID;}
AdpStatus adp_read_pipe(int fd,unsigned char*p,size_t n,ADPacket*out){(void)fd;(void)p;(void)n;(void)out;return ADP_INVALID;}
