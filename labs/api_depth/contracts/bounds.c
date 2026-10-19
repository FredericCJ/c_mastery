#include "bounds.h"
/* Intentional learner scaffold; reference implementation is assessor-only. */
AcStatus ac_admit(AcView v){(void)v;return AC_NOT_IMPLEMENTED;}
AcStatus ac_timeout(int m,AcWait *o){(void)m;(void)o;return AC_NOT_IMPLEMENTED;}
AcStatus ac_deadline(unsigned n,AcWait w,AcDeadline *o){(void)n;(void)w;(void)o;return AC_NOT_IMPLEMENTED;}
AcStatus ac_expired(AcDeadline d,unsigned n,int *o){(void)d;(void)n;(void)o;return AC_NOT_IMPLEMENTED;}
AcStatus ac_octets16(const uint_least16_t *p,size_t n,unsigned char *o,size_t c){(void)p;(void)n;(void)o;(void)c;return AC_NOT_IMPLEMENTED;}
AcStatus ac_append_slice(ApiBuffer *b,AcSlice s){(void)b;(void)s;return AC_NOT_IMPLEMENTED;}
FStatus ac_append_allow(FBuffer *b,size_t o,size_t n){(void)b;(void)o;(void)n;return F_NOT_IMPLEMENTED;}
AcStatus ac_lookup(unsigned k,AcValue *o){(void)k;(void)o;return AC_NOT_IMPLEMENTED;}
AcResult ac_lookup_tagged(unsigned k){(void)k;return (AcResult){AC_NOT_IMPLEMENTED,{0,0}};}
AcProgress ac_ascii_prefix(const unsigned char *p,size_t n,unsigned char *o,size_t c){(void)p;(void)n;(void)o;(void)c;return (AcProgress){AC_NOT_IMPLEMENTED,0,0};}
AcStatus ac_copy_ascii(const unsigned char *p,size_t n,unsigned char *o,size_t c,size_t *w,int i){(void)p;(void)n;(void)o;(void)c;(void)w;(void)i;return AC_NOT_IMPLEMENTED;}
AcStatus ac_checked_add(unsigned n,unsigned c,unsigned *v,unsigned *e){(void)n;(void)c;(void)v;(void)e;return AC_NOT_IMPLEMENTED;}
AcStatus ac_admit_wire(const unsigned char *p,size_t n,unsigned *v){(void)p;(void)n;(void)v;return AC_NOT_IMPLEMENTED;}
