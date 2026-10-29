#include "packet_ring.h"
enum adp_status adp_init(struct adp_ring*r,size_t a,uint32_t b){(void)r;(void)a;(void)b;return ADP_INVALID;}
enum adp_status adp_feed(struct adp_ring*r,const unsigned char*p,size_t n){(void)r;(void)p;(void)n;return ADP_INVALID;}
enum adp_status adp_discard(struct adp_ring*r,size_t n){(void)r;(void)n;return ADP_INVALID;}
enum adp_status adp_acquire(struct adp_ring*r,size_t n,struct adp_lease*out){(void)r;(void)n;(void)out;return ADP_INVALID;}
enum adp_status adp_validate(const struct adp_ring*r,struct adp_token t){(void)r;(void)t;return ADP_INVALID;}
enum adp_status adp_cancel(struct adp_ring*r,struct adp_token t){(void)r;(void)t;return ADP_INVALID;}
enum adp_status adp_release(struct adp_ring*r,struct adp_token t){(void)r;(void)t;return ADP_INVALID;}
enum adp_status adp_close(struct adp_ring*r){(void)r;return ADP_INVALID;}
enum adp_status adp_copy(const struct adp_ring*r,struct adp_lease v,unsigned char*p,size_t n){(void)r;(void)v;(void)p;(void)n;return ADP_INVALID;}
enum adp_status adp_record(const struct adp_ring*r,struct adp_lease v,unsigned ver,struct ar_record*out){(void)r;(void)v;(void)ver;(void)out;return ADP_INVALID;}
enum adp_status adp_parse(const unsigned char*p,size_t n,struct ad_packet*out){(void)p;(void)n;(void)out;return ADP_INVALID;}
enum adp_status adp_build(unsigned kind,const unsigned char*p,size_t n,unsigned char*out,size_t cap,size_t*used){(void)kind;(void)p;(void)n;(void)out;(void)cap;(void)used;return ADP_INVALID;}
enum adp_status adp_read_pipe(int fd,unsigned char*p,size_t n,struct ad_packet*out){(void)fd;(void)p;(void)n;(void)out;return ADP_INVALID;}
