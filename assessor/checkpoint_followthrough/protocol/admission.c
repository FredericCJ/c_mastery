#include "admission.h"
CPStatus cpa_admit(CPParser *p,CPQueue *q,size_t capacity)
{
    if(!p || !q || !capacity || capacity>CP_QUEUE || q->count>capacity)return CP_INVALID;
    if(!p->ready)return CP_NEED_MORE;
    if(q->count==capacity){
#ifdef CPA_MUTANT_DROP
        p->ready=0;
#endif
        return CP_FULL;
    }
    return cp_deliver(p,q);
}
