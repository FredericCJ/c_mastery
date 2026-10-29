#include "checkpoint_numeric.h"
#include <float.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
_Static_assert(FLT_RADIX==2 && DBL_MANT_DIG==53 && DBL_MAX_EXP==1024,"Selected binary64 numerical model");
void cpn_exact_init(struct cpn_exact *s){s->previous=0;s->older=0;}
void cpn_double_init(struct cpn_double *s){s->previous=0;s->older=0;}
enum cpn_status cpn_exact(struct cpn_exact *s,const int32_t *in,size_t n,int64_t *out,size_t capacity) {
    if(!s || n>CPN_BLOCK || (n && (!in || !out)))return CPN_INVALID;
    if(capacity<n)return CPN_CAPACITY;
    for(size_t i=0;i<n;++i){int32_t x=in[i];out[i]=(int64_t)x+2*(int64_t)s->previous+(int64_t)s->older;s->older=s->previous;s->previous=x;}
    return CPN_OK;
}
static int admissible(double x){return x==x && x>=-1e100 && x<=1e100;}
enum cpn_status cpn_double(struct cpn_double *s,const double *in,size_t n,double *out,size_t capacity) {
    if(!s || n>CPN_BLOCK || (n && (!in || !out)))return CPN_INVALID;
    if(CP_MUTANT==12)s->previous=0;
    if(capacity<n)return CPN_CAPACITY;
    if(!admissible(s->previous) || !admissible(s->older))return CPN_INVALID;
    for(size_t i=0;i<n;++i)if(!admissible(in[i]))return CPN_INVALID;
    if(in!=out && CP_MUTANT!=11)
        for(size_t i=0;i<n;++i)for(size_t j=0;j<n;++j)if(in+i==out+j)return CPN_OVERLAP;
    if(CP_MUTANT==10){s->previous=0;s->older=0;}
    for(size_t i=0;i<n;++i) {
        double x=in[i];
        double first=0.25*x+(CP_MUTANT==9?0.25:0.5)*s->previous;
        out[i]=first+0.25*s->older;
        s->older=s->previous;s->previous=x;
    }
    return CPN_OK;
}
enum cpn_status cpn_transaction(struct cpn_double *s,const double *in,size_t n,double *out,size_t capacity,size_t fail_at,int atomic,size_t *committed){
    if(!s || !committed || (atomic!=0 && atomic!=1) || n>CPN_BLOCK || (n && !out))return CPN_INVALID;
    if(capacity<n)return CPN_CAPACITY;
    struct cpn_double staged=*s;double pending[CPN_BLOCK];
    enum cpn_status status=cpn_double(&staged,in,n,pending,CPN_BLOCK);
    if(status!=CPN_OK)return status;
    int failure=fail_at && fail_at<=n;
    size_t publish=failure?(atomic?0:fail_at-1):n;
    if(CP_MUTANT==17 && failure && atomic)publish=fail_at-1;
    struct cpn_double next=*s;
    for(size_t i=0;i<publish;++i){next.older=next.previous;next.previous=in[i];}
    for(size_t i=0;i<publish;++i)out[i]=pending[i];
    *s=next;*committed=CP_MUTANT==18 && failure?publish+1:publish;
    return failure?CPN_FAILED:CPN_OK;
}
enum cpn_status cpn_round_even(int64_t numerator,int32_t *out){
    if(!out)return CPN_INVALID;
    int64_t q=numerator/4,r=numerator%4;
    if(r>2 || (r==2 && q%2))++q;
    if(r< -2 || (r== -2 && q%2))--q;
    if(q<INT32_MIN || q>INT32_MAX)return CPN_CAPACITY;
    *out=(int32_t)q;return CPN_OK;
}
enum cpn_status cpn_iir_half(double *state,const double *in,size_t n,double *out,size_t capacity){
    if(!state || n>CPN_BLOCK || (n && (!in || !out)))return CPN_INVALID;
    if(capacity<n)return CPN_CAPACITY;
    if(!admissible(*state))return CPN_INVALID;
    double pending[CPN_BLOCK],next=*state;
    for(size_t i=0;i<n;++i){if(!admissible(in[i]))return CPN_INVALID;next=0.5*in[i]+0.5*next;pending[i]=next;}
    for(size_t i=0;i<n;++i)out[i]=pending[i];
    *state=next;return CPN_OK;
}
