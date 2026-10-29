#include "partial_optics.h"
enum adl_status adl_get(struct adl_config s,uint32_t *o){(void)s;(void)o;return ADL_INVALID;}
enum adl_status adl_put(struct adl_config s,uint32_t v,struct adl_config *o){(void)s;(void)v;(void)o;return ADL_INVALID;}
enum adl_status adl_update(struct adl_config s,const struct adl_step *f,size_t n,struct adl_config *o){(void)s;(void)f;(void)n;(void)o;return ADL_INVALID;}
enum adl_status adl_preview(struct adl_error s,struct adl_io *o){(void)s;(void)o;return ADL_INVALID;}
enum adl_status adl_review(struct adl_io s,struct adl_error *o){(void)s;(void)o;return ADL_INVALID;}
