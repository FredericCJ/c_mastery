#include "owning_option.h"
int eop_scalar_make(uint32_t v,unsigned p,EopScalar *o){(void)v;(void)p;(void)o;return EOP_INVALID;}
int eop_scalar_get(const EopScalar *o,uint32_t *v){(void)o;(void)v;return EOP_INVALID;}
int eop_result_get(er_result r,uint32_t *v,size_t *e){(void)r;(void)v;(void)e;return EOP_INVALID;}
int eop_create(EopOwner *o,V4Allocator a,uint32_t i,int32_t v){(void)o;(void)a;(void)i;(void)v;return EOP_INVALID;}
int eop_clone(const EopOwner *s,EopOwner *d){(void)s;(void)d;return EOP_INVALID;}
int eop_move(EopOwner *d,EopOwner *s){(void)d;(void)s;return EOP_INVALID;}
int eop_read(const EopOwner *s,uint32_t *i,int32_t *v){(void)s;(void)i;(void)v;return EOP_INVALID;}
int eop_drop(EopOwner *o){(void)o;return EOP_INVALID;}
int eop_unit(EopUnit u){(void)u;return EOP_INVALID;}
