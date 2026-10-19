#include "partial_optics.h"
AdlStatus adl_get(AdlConfig s,uint32_t *o){(void)s;(void)o;return ADL_INVALID;}
AdlStatus adl_put(AdlConfig s,uint32_t v,AdlConfig *o){(void)s;(void)v;(void)o;return ADL_INVALID;}
AdlStatus adl_update(AdlConfig s,const AdlStep *f,size_t n,AdlConfig *o){(void)s;(void)f;(void)n;(void)o;return ADL_INVALID;}
AdlStatus adl_preview(AdlError s,AdlIO *o){(void)s;(void)o;return ADL_INVALID;}
AdlStatus adl_review(AdlIO s,AdlError *o){(void)s;(void)o;return ADL_INVALID;}
