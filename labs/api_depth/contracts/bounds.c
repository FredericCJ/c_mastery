#include "bounds.h"
/* Intentional learner scaffold; reference implementation is assessor-only. */
enum ac_status ac_admit(struct ac_view v){(void)v;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_timeout(int m,struct ac_wait *o){(void)m;(void)o;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_deadline(unsigned n,struct ac_wait w,struct ac_deadline *o){(void)n;(void)w;(void)o;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_expired(struct ac_deadline d,unsigned n,int *o){(void)d;(void)n;(void)o;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_octets16(const uint_least16_t *p,size_t n,unsigned char *o,size_t c){(void)p;(void)n;(void)o;(void)c;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_append_slice(struct api_buffer *b,struct ac_slice s){(void)b;(void)s;return AC_NOT_IMPLEMENTED;}
enum f_status ac_append_allow(struct f_buffer *b,size_t o,size_t n){(void)b;(void)o;(void)n;return F_NOT_IMPLEMENTED;}
enum ac_status ac_lookup(unsigned k,struct ac_value *o){(void)k;(void)o;return AC_NOT_IMPLEMENTED;}
struct ac_result ac_lookup_tagged(unsigned k){(void)k;return (struct ac_result){AC_NOT_IMPLEMENTED,{0,0}};}
struct ac_progress ac_ascii_prefix(const unsigned char *p,size_t n,unsigned char *o,size_t c){(void)p;(void)n;(void)o;(void)c;return (struct ac_progress){AC_NOT_IMPLEMENTED,0,0};}
enum ac_status ac_copy_ascii(const unsigned char *p,size_t n,unsigned char *o,size_t c,size_t *w,int i){(void)p;(void)n;(void)o;(void)c;(void)w;(void)i;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_checked_add(unsigned n,unsigned c,unsigned *v,unsigned *e){(void)n;(void)c;(void)v;(void)e;return AC_NOT_IMPLEMENTED;}
enum ac_status ac_admit_wire(const unsigned char *p,size_t n,unsigned *v){(void)p;(void)n;(void)v;return AC_NOT_IMPLEMENTED;}
