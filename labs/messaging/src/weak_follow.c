#include "weak.h"
void mfw_deque_init(MfwDeque *q) {
    atomic_init(&q->top,1); atomic_init(&q->bottom,1);
    for(size_t i=0;i<8;i++)atomic_init(&q->slots[i],0);
}
MfwStatus mfw_push(MfwDeque*q,unsigned v){(void)q;(void)v;return MFW_INVALID;}
MfwStatus mfw_pop(MfwDeque*q,unsigned*out){(void)q;(void)out;return MFW_INVALID;}
MfwStatus mfw_steal(MfwDeque*q,unsigned*out){(void)q;(void)out;return MFW_INVALID;}
void mfw_sequence_init(MfwSequence*s){atomic_init(&s->sequence,0);atomic_init(&s->first,0);atomic_init(&s->second,0);}
MfwStatus mfw_write(MfwSequence*s,unsigned a,unsigned b){(void)s;(void)a;(void)b;return MFW_INVALID;}
MfwStatus mfw_read(MfwSequence*s,size_t n,unsigned out[2]){(void)s;(void)n;(void)out;return MFW_INVALID;}
