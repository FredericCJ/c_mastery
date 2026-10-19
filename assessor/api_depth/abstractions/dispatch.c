#include "dispatch.h"
int ad_direct_init(AdDirect *d,unsigned kind){if(!d||kind>=3)return SF_INVALID;*d=(AdDirect){.kind=kind};return SF_OK;}
int ad_erased_init(AdErased *e,const AdOps *ops){if(!e||!ops||ops->version!=1||ops->kind>=3||!ops->write)return SF_INVALID;*e=(AdErased){.ops=ops};return SF_OK;}
ptrdiff_t ad_direct_write(void *ctx,const unsigned char *p,size_t n){AdDirect *d=ctx;if(!d)return -1;switch(d->kind){case 0:return ad_one(&d->sink,p,n);case 1:return ad_four(&d->sink,p,n);case 2:return ad_all(&d->sink,p,n);default:return -1;}}
ptrdiff_t ad_erased_write(void *ctx,const unsigned char *p,size_t n){AdErased *e=ctx;if(!e||!e->ops||e->ops->version!=1||e->ops->kind>=3||!e->ops->write)return -1;
#ifdef AD_MUTANT_DISPATCH
 return ad_one(&e->sink,p,n);
#else
 return e->ops->write(&e->sink,p,n);
#endif
}
ptrdiff_t ad_fixed_write(void *ctx,const unsigned char *p,size_t n){return ad_one(ctx,p,n);}
