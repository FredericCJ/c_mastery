#include "auth.h"
enum cpx_auth_status cpx_auth_sign(const struct cpx_auth_state *s,uint64_t q,const unsigned char *p,size_t n,unsigned char *o,size_t c,size_t *w,cpx_mac_fn m){(void)s;(void)q;(void)p;(void)n;(void)o;(void)c;(void)w;(void)m;return CPX_AUTH_NOT_IMPLEMENTED;}
enum cpx_auth_status cpx_auth_open(struct cpx_auth_state *s,const unsigned char *b,size_t n,unsigned char *o,size_t c,size_t *w,cpx_mac_fn m){(void)s;(void)b;(void)n;(void)o;(void)c;(void)w;(void)m;return CPX_AUTH_NOT_IMPLEMENTED;}
