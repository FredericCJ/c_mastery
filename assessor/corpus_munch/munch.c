#include "munch.h"
#ifndef MM_MUTANT
#define MM_MUTANT 0
#endif
static int valid(const MmDfa *d) {
    if(!d || !d->count || d->count>16 || d->start>=d->count || d->accept[d->start])return 0;
    for(unsigned i=0;i<256;++i)if(d->map[i]>4)return 0;
    for(unsigned q=0;q<d->count;++q)for(unsigned c=0;c<4;++c)
        if(d->next[q][c]!=MM_DEAD && d->next[q][c]>=d->count)return 0;
    return 1;
}
static unsigned rule(unsigned mask) {
    unsigned r=0;
    if(MM_MUTANT==2){while(mask>>1){mask>>=1;++r;}return r;}
    while(!(mask&1u)){mask>>=1;++r;}return r;
}
int mm_from_rx(const RxDfa *source,MmDfa *out) {
    MmDfa d={0};
    if(!source || !out || !source->count || source->count>16 || source->accept[0])return PD_INVALID;
    d.count=source->count;
    for(unsigned i=0;i<256;++i)d.map[i]=4;
    d.map['a']=0;d.map['b']=1;
    for(unsigned q=0;q<d.count;++q) {
        if(source->accept[q]>1)return PD_INVALID;
        d.accept[q]=(unsigned char)source->accept[q];
        for(unsigned c=0;c<4;++c) {
            if(c<2 && source->next[q][c]>=source->count)return PD_INVALID;
            d.next[q][c]=c<2?(unsigned char)source->next[q][c]:MM_DEAD;
        }
    }
    *out=d;return PD_OK;
}
int mm_scan(const MmDfa *d,const unsigned char *input,unsigned length,
            unsigned mode,unsigned capacity,MmWorkspace *w,
            MmTokens *out,MmCost *cost,unsigned *error_offset) {
    MmTokens result={0};MmCost counts={0};unsigned begin=0;
    if(!valid(d) || (!input && length) || !out || !cost || length>256 || mode>1 || capacity>256 || (mode && !w))return PD_INVALID;
    if(mode && MM_MUTANT!=5)for(unsigned i=0;i<=length;++i)for(unsigned q=0;q<d->count;++q)w->failed[i][q]=0;
    while(begin<length) {
        unsigned q=d->start,pos=begin,last=begin,mask=0,top=0;
        for(;;) {
            if(d->accept[q]) {
                last=pos;mask=d->accept[q];top=0;
                if(MM_MUTANT==1)break;
            }
            if(pos==length)break;
            if(mode && w->failed[pos][q] && MM_MUTANT!=6){++counts.memo_stops;break;}
            {
                unsigned cls=d->map[input[pos]],next=cls<4?d->next[q][cls]:MM_DEAD;
                if(next==MM_DEAD)break;
                if(mode){w->stack_state[top]=(unsigned char)q;w->stack_position[top++]=(unsigned short)pos;}
                q=next;++pos;++counts.transitions;
            }
        }
        counts.rollback_bytes+=pos-last;
        if(mode && !d->accept[q]) {
            for(;;) {
                if(!w->failed[pos][q]){w->failed[pos][q]=1;++counts.marked_pairs;}
                if(!top)break;
                --top;q=w->stack_state[top];pos=w->stack_position[top];
                if(d->accept[q])break;
            }
        }
        if(!mask || last==begin){if(error_offset)*error_offset=begin;return PD_NO_MATCH;}
        if(result.count==capacity){if(error_offset)*error_offset=begin;return PD_RANGE;}
        result.token[result.count++]=(MmToken){begin,last,rule(mask)};
        if(MM_MUTANT==3)*out=result;
        begin=MM_MUTANT==4?length:last;
    }
    *out=result;*cost=counts;return PD_OK;
}
