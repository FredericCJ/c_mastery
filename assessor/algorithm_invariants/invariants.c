#include "invariants.h"
enum AIStatus ai_step(AIState state,unsigned subtract,AIState *out) {
    if(!out || subtract>1 || state.a>4095 || state.b>4095 || (!state.a && !state.b))return AI_INVALID;
#ifdef AI_MUTANT_ZERO
    if(!state.a || !state.b)return AI_INVALID;
#endif
    if(!state.b || (subtract && (!state.a || state.a==state.b)))return AI_DONE;
    AIState next;
    if(subtract)next=state.a>state.b?(AIState){state.a-state.b,state.b}:(AIState){state.a,state.b-state.a};
    else next=(AIState){state.b,state.a%state.b};
    *out=next;return AI_OK;
}
static int equal(AIRecord a,AIRecord b){return a.key==b.key && a.tag==b.tag;}
enum AIStatus ai_permutation(const AIRecord *a,const AIRecord *b,size_t n,unsigned *out) {
    if(!out || n>16 || ((!a || !b) && n))return AI_INVALID;
    unsigned matched[16]={0},yes=1;
    for(size_t i=0;i<n;++i){size_t j=0;
#ifdef AI_MUTANT_SET
        for(;j<n && !equal(a[i],b[j]);++j){}
#else
        for(;j<n && (matched[j] || !equal(a[i],b[j]));++j){}
#endif
        if(j==n){yes=0;break;}matched[j]=1;
    }
#ifdef AI_MUTANT_SET
    (void)matched;
#endif
    *out=yes;return AI_OK;
}
