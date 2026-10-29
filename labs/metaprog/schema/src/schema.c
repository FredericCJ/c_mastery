#include "schema.h"
void MPS_LINK_GUARD(void){}
size_t mps_fields(const struct mps_field **p){(void)p;return 0;}
int mps_validate(const struct mps_record *p){(void)p;return MPS_INVALID;}
int mps_equal(const struct mps_record *a,const struct mps_record *b,unsigned *o){(void)a;(void)b;(void)o;return MPS_INVALID;}
int mps_encode(unsigned v,const struct mps_record *r,unsigned char *p,size_t n,size_t *o){(void)v;(void)r;(void)p;(void)n;(void)o;return MPS_INVALID;}
int mps_decode(const unsigned char *p,size_t n,struct mps_record *o){(void)p;(void)n;(void)o;return MPS_INVALID;}
int mps_visit(const struct mps_record *r,mps_visitor_fn v,void *c,size_t *n){(void)r;(void)v;(void)c;(void)n;return MPS_INVALID;}
