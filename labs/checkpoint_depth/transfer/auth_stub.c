#include "auth.h"
CPXAuthStatus cpx_auth_sign(const CPXAuthState *s,uint64_t q,const unsigned char *p,size_t n,unsigned char *o,size_t c,size_t *w,CPXMac m){(void)s;(void)q;(void)p;(void)n;(void)o;(void)c;(void)w;(void)m;return CPX_AUTH_NOT_IMPLEMENTED;}
CPXAuthStatus cpx_auth_open(CPXAuthState *s,const unsigned char *b,size_t n,unsigned char *o,size_t c,size_t *w,CPXMac m){(void)s;(void)b;(void)n;(void)o;(void)c;(void)w;(void)m;return CPX_AUTH_NOT_IMPLEMENTED;}
