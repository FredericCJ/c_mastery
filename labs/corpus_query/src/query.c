#include "query.h"
/* Intentionally unfinished learner source, separate from private reference. */
int qi_build(const QiRow *r,size_t n,QiIndex *o){(void)r;(void)n;(void)o;return QI_INVALID;}
int qi_replace(QiIndex *x,size_t r,QiRow v,int l){(void)x;(void)r;(void)v;(void)l;return QI_INVALID;}
int qi_rebuild_zones(QiIndex *x){(void)x;return QI_INVALID;}
int qi_terms(const QiIndex *x,unsigned a,unsigned b,uint32_t *o){(void)x;(void)a;(void)b;(void)o;return QI_INVALID;}
int qi_compare(const QiIndex *x,int16_t t,QiTruth *o){(void)x;(void)t;(void)o;return QI_INVALID;}
int qi_logic(const QiIndex *x,QiTruth a,QiTruth b,unsigned p,QiTruth *o){(void)x;(void)a;(void)b;(void)p;(void)o;return QI_INVALID;}
int qi_phrase(const QiIndex *x,const uint8_t *q,size_t n,uint32_t *o){(void)x;(void)q;(void)n;(void)o;return QI_INVALID;}
int qi_near(const QiIndex *x,unsigned a,unsigned b,unsigned d,uint32_t *o){(void)x;(void)a;(void)b;(void)d;(void)o;return QI_INVALID;}
int qi_range(const QiIndex *x,int16_t a,int16_t b,uint32_t *o,QiCost *c){(void)x;(void)a;(void)b;(void)o;(void)c;return QI_INVALID;}
int qi_search(const QiIndex *x,unsigned a,unsigned b,int16_t l,int16_t h,const uint8_t *p,size_t n,uint32_t *o,QiCost *c){(void)x;(void)a;(void)b;(void)l;(void)h;(void)p;(void)n;(void)o;(void)c;return QI_INVALID;}
