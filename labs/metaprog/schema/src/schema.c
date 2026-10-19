#include "schema.h"
void MPS_LINK_GUARD(void){}
size_t mps_fields(const MpsField **p){(void)p;return 0;}
int mps_validate(const MpsRecord *p){(void)p;return MPS_INVALID;}
int mps_equal(const MpsRecord *a,const MpsRecord *b,unsigned *o){(void)a;(void)b;(void)o;return MPS_INVALID;}
int mps_encode(unsigned v,const MpsRecord *r,unsigned char *p,size_t n,size_t *o){(void)v;(void)r;(void)p;(void)n;(void)o;return MPS_INVALID;}
int mps_decode(const unsigned char *p,size_t n,MpsRecord *o){(void)p;(void)n;(void)o;return MPS_INVALID;}
int mps_visit(const MpsRecord *r,MpsVisitor v,void *c,size_t *n){(void)r;(void)v;(void)c;(void)n;return MPS_INVALID;}
