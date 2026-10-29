#include "wrappers.h"
int aw_finite_compare(const struct aw_record*a,const struct aw_record*b,unsigned d){(void)a;(void)b;(void)d;return 0;}
int aw_callback_compare(aw_compare_fn f,void*c,const struct aw_record*a,const struct aw_record*b,int*out){(void)f;(void)c;(void)a;(void)b;(void)out;return 1;}
enum aw_status aw_init(struct aw_owner*o,unsigned k,unsigned n,unsigned l,struct aw_context*c,unsigned d,aw_compare_fn f,void*x,struct aw_backing b){(void)o;(void)k;(void)n;(void)l;(void)c;(void)d;(void)f;(void)x;(void)b;return AW_INVALID;}
enum aw_status aw_put(struct aw_owner*o,const struct aw_record*r){(void)o;(void)r;return AW_INVALID;}
enum aw_status aw_remove(struct aw_owner*o,int32_t id){(void)o;(void)id;return AW_INVALID;}
enum aw_status aw_reserve(struct aw_owner*o,unsigned n){(void)o;(void)n;return AW_INVALID;}
enum aw_status aw_set_context(struct aw_owner*o,struct aw_context*c){(void)o;(void)c;return AW_INVALID;}
enum aw_status aw_view(const struct aw_owner*o,unsigned i,struct aw_view*v){(void)o;(void)i;(void)v;return AW_INVALID;}
enum aw_status aw_read(struct aw_view v,struct aw_record*r){(void)v;(void)r;return AW_INVALID;}
enum aw_status aw_destroy(struct aw_owner*o){(void)o;return AW_INVALID;}
