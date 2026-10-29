#include "sketch.h"
#ifndef SK_MUTANT
#define SK_MUTANT 0
#endif
static int valid(const struct sk_cm_config *c) {
    unsigned r;
    if(c->p<1 || c->p>5 || c->depth<1 || c->depth>4) return 0;
    for(r=0;r<c->depth;r++) if(c->bias[r]>=(1u<<c->p)) return 0;
    return 1;
}
static unsigned index_of(const struct sk_cm_config *c,unsigned r,uint32_t key) {
    unsigned b,result=c->bias[r];
    for(b=0;b<c->p;b++) {
        uint32_t bits=key&c->mask[r][b]; unsigned parity=0;
        while(bits) { parity^=(unsigned)(bits&1u); bits>>=1; }
        result^=parity<<b;
    }
    return result;
}
static int same(const struct sk_cm_config *a,const struct sk_cm_config *b) {
    unsigned r,k;
    if(a->p!=b->p || a->depth!=b->depth) return 0;
    for(r=0;r<a->depth;r++) {
        if(a->bias[r]!=b->bias[r]) return 0;
        for(k=0;k<a->p;k++) if(a->mask[r][k]!=b->mask[r][k]) return 0;
    }
    return 1;
}
int sk_cm_init(struct sk_cm *s,const struct sk_cm_config *c) {
    struct sk_cm next={0};
    if(!valid(c)) return SK_INVALID;
    next.config=*c; *s=next; return SK_OK;
}
int sk_cm_add(struct sk_cm *s,uint32_t key,uint64_t amount) {
    unsigned r,indices[4];
    if(!valid(&s->config)) return SK_INVALID;
    if(amount>UINT64_MAX-s->total) return SK_OVERFLOW;
    for(r=0;r<s->config.depth;r++) {
        indices[r]=index_of(&s->config,r,key);
        if(amount>UINT64_MAX-s->cell[r][indices[r]]) return SK_OVERFLOW;
    }
    for(r=0;r<s->config.depth;r++) s->cell[r][indices[r]]+=amount;
    s->total+=amount;
    return SK_OK;
}
int sk_cm_query(const struct sk_cm *s,uint32_t key,uint64_t *out) {
    unsigned r; uint64_t value=UINT64_MAX;
    if(!valid(&s->config)) return SK_INVALID;
    for(r=0;r<s->config.depth;r++) {
        uint64_t v=s->cell[r][index_of(&s->config,r,key)];
        if(v<value) value=v;
    }
    if(SK_MUTANT==1 && value) --value;
    *out=value; return SK_OK;
}
int sk_cm_merge(struct sk_cm *dst,const struct sk_cm *src) {
    unsigned r,k;
    if(!valid(&dst->config)||!valid(&src->config)||!same(&dst->config,&src->config)) return SK_INVALID;
    if(src->total>UINT64_MAX-dst->total) return SK_OVERFLOW;
    for(r=0;r<dst->config.depth;r++) for(k=0;k<(1u<<dst->config.p);k++)
        if(src->cell[r][k]>UINT64_MAX-dst->cell[r][k]) return SK_OVERFLOW;
    for(r=0;r<dst->config.depth;r++) for(k=0;k<(1u<<dst->config.p);k++)
        dst->cell[r][k]+=SK_MUTANT==2?0:src->cell[r][k];
    dst->total+=src->total; return SK_OK;
}
