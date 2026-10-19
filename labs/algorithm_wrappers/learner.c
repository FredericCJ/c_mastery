#include "wrappers.h"
int aw_finite_compare(const AWRecord*a,const AWRecord*b,unsigned d){(void)a;(void)b;(void)d;return 0;}
int aw_callback_compare(AWCompare f,void*c,const AWRecord*a,const AWRecord*b,int*out){(void)f;(void)c;(void)a;(void)b;(void)out;return 1;}
enum AWStatus aw_init(AWOwner*o,unsigned k,unsigned n,unsigned l,AWContext*c,unsigned d,AWCompare f,void*x,AWBacking b){(void)o;(void)k;(void)n;(void)l;(void)c;(void)d;(void)f;(void)x;(void)b;return AW_INVALID;}
enum AWStatus aw_put(AWOwner*o,const AWRecord*r){(void)o;(void)r;return AW_INVALID;}
enum AWStatus aw_remove(AWOwner*o,int32_t id){(void)o;(void)id;return AW_INVALID;}
enum AWStatus aw_reserve(AWOwner*o,unsigned n){(void)o;(void)n;return AW_INVALID;}
enum AWStatus aw_set_context(AWOwner*o,AWContext*c){(void)o;(void)c;return AW_INVALID;}
enum AWStatus aw_view(const AWOwner*o,unsigned i,AWView*v){(void)o;(void)i;(void)v;return AW_INVALID;}
enum AWStatus aw_read(AWView v,AWRecord*r){(void)v;(void)r;return AW_INVALID;}
enum AWStatus aw_destroy(AWOwner*o){(void)o;return AW_INVALID;}
