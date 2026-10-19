#include "ring.h"
#ifndef CR_MUTANT
#define CR_MUTANT 0
#endif
static int less(CrToken a,CrToken b)
{
    if(a.position!=b.position)return a.position<b.position;
    if(a.owner!=b.owner)return CR_MUTANT==2 ? a.owner>b.owner : a.owner<b.owner;
    return a.ordinal<b.ordinal;
}
CrStatus cr_build(const CrToken *tokens,size_t n,uint32_t epoch,CrRing *out)
{
    if(!out||(!tokens&&n)||n>CR_TOKENS||!epoch)return CR_INVALID;
    CrRing tmp;tmp.count=n;tmp.epoch=epoch;
    for(size_t i=0;i<CR_TOKENS;i++)tmp.token[i]=(CrToken){0,0,0};
    for(size_t i=0;i<n;i++){
        if(tokens[i].owner==0||tokens[i].owner>CR_OWNERS)return CR_INVALID;
        for(size_t j=0;j<i;j++)if(tokens[j].owner==tokens[i].owner&&tokens[j].ordinal==tokens[i].ordinal)return CR_INVALID;
        size_t j=i;while(j&&less(tokens[i],tmp.token[j-1])){tmp.token[j]=tmp.token[j-1];j--;}
        tmp.token[j]=tokens[i];
    }
    *out=tmp;return CR_OK;
}
static CrStatus admit(const CrRing *r,uint32_t epoch)
{
    if(!r||!r->epoch||r->count>CR_TOKENS)return CR_INVALID;
    if(r->epoch!=epoch && CR_MUTANT!=4)return CR_STALE;
    return r->count?CR_OK:CR_EMPTY;
}
static size_t successor(const CrRing *r,uint16_t key,CrCost *cost)
{
    size_t lo=0,hi=r->count;
    while(lo<hi){size_t mid=lo+(hi-lo)/2;cost->comparisons++;if(r->token[mid].position<key||(CR_MUTANT==1&&r->token[mid].position==key))lo=mid+1;else hi=mid;}
    return lo==r->count?0:lo;
}
CrStatus cr_owner(const CrRing *r,uint16_t key,uint32_t epoch,unsigned *out,CrCost *cost)
{
    if(!out||!cost)return CR_INVALID;
    CrStatus s=admit(r,epoch);if(s!=CR_OK)return s;
    CrCost c={0,0};size_t i=successor(r,key,&c);c.visited=1;
    *out=r->token[i].owner;*cost=c;return CR_OK;
}
CrStatus cr_change(CrRing *r,unsigned owner,const CrToken *tokens,size_t n,size_t cut)
{
    if(!r||!r->epoch||r->count>CR_TOKENS||owner==0||owner>CR_OWNERS||n>CR_TOKENS||(!tokens&&n))return CR_INVALID;
    if(r->epoch==UINT32_MAX)return CR_FULL;
    CrToken all[CR_TOKENS];size_t count=0,old=0;
    for(size_t i=0;i<r->count;i++)if(r->token[i].owner==owner)old++;else all[count++]=r->token[i];
    if(n && old)return CR_INVALID;
    if(!n && !old)return CR_INVALID;
    if(n>CR_TOKENS-count)return CR_FULL;
    for(size_t i=0;i<n;i++){if(tokens[i].owner!=owner)return CR_INVALID;all[count++]=tokens[i];}
    CrRing staged;CrStatus s=cr_build(all,count,r->epoch+1,&staged);if(s!=CR_OK)return s;
    if(CR_MUTANT==3)r->epoch=staged.epoch;
    if(cut<=count)return CR_ABORTED;
    *r=staged;return CR_OK;
}
CrStatus cr_replicas(const CrRing *r,uint16_t key,uint32_t epoch,unsigned wanted,uint16_t down,unsigned *out,CrCost *cost)
{
    if(!out||!cost||!wanted||wanted>CR_OWNERS)return CR_INVALID;
    CrStatus s=admit(r,epoch);if(s!=CR_OK)return s;
    CrCost c={0,0};size_t first=successor(r,key,&c);unsigned list[CR_OWNERS],n=0;uint16_t seen=0;
    for(size_t i=0;i<r->count&&n<wanted;i++){
        size_t j=first+i;if(j>=r->count)j-=r->count;
        unsigned owner=r->token[j].owner;uint16_t bit=(uint16_t)(1u<<(owner-1));c.visited++;
        if((!(down&bit)||CR_MUTANT==6)&&(!(seen&bit)||CR_MUTANT==5)){list[n++]=owner;seen|=bit;}
    }
    if(n<wanted)return CR_UNAVAILABLE;
    for(unsigned i=0;i<n;i++)out[i]=list[i];
    *cost=c;return CR_OK;
}
CrStatus cr_route(const CrRing *r,uint32_t epoch,const unsigned char *bytes,size_t length,CrHash hash,void *ctx,unsigned *out,CrCost *cost)
{
    if(!hash||(!bytes&&length)||!out||!cost)return CR_INVALID;
    CrStatus s=admit(r,epoch);if(s!=CR_OK)return s;
    unsigned char digest[32]={0};
    int failed=hash(ctx,bytes,length,digest);
    if(failed && CR_MUTANT!=7)return CR_HASH_FAILED;
    uint16_t key=(uint16_t)((uint16_t)digest[0]<<8|digest[1]);
    if(CR_MUTANT==8)key=(uint16_t)((uint16_t)digest[1]<<8|digest[0]);
    return cr_owner(r,key,epoch,out,cost);
}
CrStatus cr_plan(const CrRing *a,const CrRing *b,const uint16_t *keys,size_t n,CrMove *out,size_t capacity,size_t *count)
{
    if(!a||!b||(!keys&&n)||!out||!count||n>CR_TOKENS||a->epoch==UINT32_MAX||b->epoch!=a->epoch+1)return CR_INVALID;
    CrMove staged[CR_TOKENS];size_t used=0;
    for(size_t i=0;i<n;i++){
        unsigned before,after;CrCost c;CrStatus s=cr_owner(a,keys[i],a->epoch,&before,&c);if(s!=CR_OK)return s;
        s=cr_owner(b,keys[i],b->epoch,&after,&c);if(s!=CR_OK)return s;
        if(before!=after){
            if(used==capacity)return CR_FULL;
            staged[used++]=(CrMove){keys[i],(uint8_t)before,(uint8_t)after};
            if(CR_MUTANT==9)out[used-1]=staged[used-1];
        }
    }
    for(size_t i=0;i<used;i++)out[i]=staged[i];
    *count=used;return CR_OK;
}
