#include "lifetime.h"
#include <limits.h>
#ifndef MFL_MUTANT
#define MFL_MUTANT 0
#endif
#ifndef MFL_REFERENCE_MAX
#define MFL_REFERENCE_MAX UINT_MAX
#endif
_Static_assert(MFL_REFERENCE_MAX>=2 && MFL_REFERENCE_MAX<=UINT_MAX,"reference maximum");
struct MflShared {
    atomic_uint references;
    MfoValue value;
    MflAllocator allocator;
};
struct MfgArray { size_t capacity;unsigned references;MflShared *slots[]; };
static bool allocator_valid(MflAllocator a){return a.allocate && a.deallocate;}
static bool value_valid(const MfoValue*v){return v && (!v->data || v->drop);}
MsgStatus mfl_make(MflAllocator a,MfoValue *value,MflShared **out){
    if(!allocator_valid(a)||!value_valid(value)||!out)return MSG_INVALID;
    MflShared*p=a.allocate(a.context,sizeof(*p));if(!p)return MSG_FAILED;
    atomic_init(&p->references,1);p->value=*value;p->allocator=a;
    *value=(MfoValue){0};*out=p;return MSG_OK;
}
MsgStatus mfl_retain(MflShared*p){
    if(!p)return MSG_INVALID;
    unsigned n=atomic_load(&p->references);
    do{if(n==MFL_REFERENCE_MAX && MFL_MUTANT!=55)return MSG_OVERFLOW;}while(!atomic_compare_exchange_weak(&p->references,&n,n+1));
    return MSG_OK;
}
void mfl_drop(MflShared**owner){
    MflShared*p=*owner;*owner=NULL;if(!p)return;
    if(atomic_fetch_sub(&p->references,MFL_MUTANT==56?2:1)==1){MflAllocator a=p->allocator;mfo_drop(&p->value);a.deallocate(a.context,p);}
}
const MfoValue*mfl_borrow(const MflShared*p){return &p->value;}
static MfgArray *array_make(MflAllocator a,size_t capacity){
    MfgArray*p=a.allocate(a.context,sizeof(*p)+capacity*sizeof(*p->slots));if(!p)return NULL;
    p->capacity=capacity;p->references=1;for(size_t i=0;i<capacity;i++)p->slots[i]=NULL;return p;
}
static void array_drop(MflAllocator a,MfgArray*p){if(!--p->references)a.deallocate(a.context,p);}
MsgStatus mfg_init(MfgDeque*q,MflAllocator a,size_t maximum){
    if(!q || !allocator_valid(a) || maximum<2 || maximum>64 || (maximum&(maximum-1)))return MSG_INVALID;
    if(pthread_mutex_init(&q->mutex,NULL))return MSG_FAILED;
    MfgArray*array=array_make(a,2);if(!array){pthread_mutex_destroy(&q->mutex);return MSG_FAILED;}
    q->allocator=a;q->array=array;q->top=q->bottom=1;q->capacity=2;q->maximum=maximum;q->active=0;q->live=true;return MSG_OK;
}
MsgStatus mfg_push(MfgDeque*q,MfoValue*value){
    if(!q || !value_valid(value))return MSG_INVALID;
    pthread_mutex_lock(&q->mutex);MsgStatus status=MSG_OK;MfgArray*next=NULL;
    if(!q->live){status=MSG_CLOSED;goto out;}
    if(q->bottom>=SIZE_MAX-1){status=MSG_OVERFLOW;goto out;}
    if(q->bottom-q->top==q->capacity){
        if(q->capacity==q->maximum){status=MSG_FULL;goto out;}
        next=array_make(q->allocator,2*q->capacity);if(!next){status=MSG_FAILED;goto out;}
        for(size_t i=q->top;i<q->bottom;i++)next->slots[i%next->capacity]=q->array->slots[i%q->capacity];
    }
    MflShared*node=NULL;status=mfl_make(q->allocator,value,&node);
    if(status!=MSG_OK){if(next)array_drop(q->allocator,next);goto out;}
    if(next){MfgArray*old=q->array;q->array=next;q->capacity=next->capacity;array_drop(q->allocator,old);}
    q->array->slots[q->bottom%q->capacity]=node;++q->bottom;
out:pthread_mutex_unlock(&q->mutex);return status;
}
MsgStatus mfg_pop(MfgDeque*q,MflShared**out){
    if(!q||!out)return MSG_INVALID;
    pthread_mutex_lock(&q->mutex);MsgStatus status=MSG_EMPTY;
    if(!q->live)status=MSG_CLOSED;
    else if(q->top<q->bottom){
        size_t index=--q->bottom;*out=q->array->slots[index%q->capacity];q->array->slots[index%q->capacity]=NULL;
        if(q->top==index){++q->top;q->bottom=q->top;}status=MSG_OK;
    }
    pthread_mutex_unlock(&q->mutex);return status;
}
MsgStatus mfg_steal(MfgDeque*q,MflShared**out,MfgPause pause,void*context){
    if(!q||!out)return MSG_INVALID;
    pthread_mutex_lock(&q->mutex);MsgStatus status=MSG_EMPTY;
    if(!q->live)status=MSG_CLOSED;
    else if(q->active==UINT_MAX || q->array->references==UINT_MAX)status=MSG_OVERFLOW;
    else if(q->top<q->bottom){
        size_t index=q->top;MfgArray*array=q->array;MflShared*node=array->slots[index%array->capacity];
        status=MFL_MUTANT==51?MSG_OK:mfl_retain(node);
        if(status!=MSG_OK){pthread_mutex_unlock(&q->mutex);return status;}
        if(MFL_MUTANT!=52)++array->references;
        ++q->active;pthread_mutex_unlock(&q->mutex);if(pause)pause(context);
        pthread_mutex_lock(&q->mutex);
        /* The retired array remains allocated even after growth and owner release. */
        size_t old_slot=index%array->capacity;
        if(q->top==index && q->top<q->bottom && array->slots[old_slot]==node){
            *out=node;q->array->slots[index%q->capacity]=NULL;++q->top;status=MSG_OK;
        }else status=MSG_CONFLICT;
        --q->active;if(MFL_MUTANT!=52)array_drop(q->allocator,array);
        pthread_mutex_unlock(&q->mutex);if(MFL_MUTANT!=51)mfl_drop(&node);return status;
    }
    pthread_mutex_unlock(&q->mutex);return status;
}
MsgStatus mfg_destroy(MfgDeque*q){
    if(!q)return MSG_INVALID;
    MflShared*nodes[64];size_t count=0;
    pthread_mutex_lock(&q->mutex);
    if(q->active){pthread_mutex_unlock(&q->mutex);return MSG_BUSY;}
    if(!q->live){pthread_mutex_unlock(&q->mutex);return MSG_CLOSED;}
    for(size_t i=q->top;i<q->bottom;i++)nodes[count++]=q->array->slots[i%q->capacity];
    MfgArray*array=q->array;MflAllocator a=q->allocator;q->live=false;q->array=NULL;
    pthread_mutex_unlock(&q->mutex);pthread_mutex_destroy(&q->mutex);array_drop(a,array);
    for(size_t i=0;i<count;i++)mfl_drop(&nodes[i]);
    return MSG_OK;
}
MsgStatus mfp_init(MfpPointer*p,MflShared**owner){
    if(!p || !owner || !*owner)return MSG_INVALID;
    if(pthread_mutex_init(&p->mutex,NULL))return MSG_FAILED;
    p->current=*owner;*owner=NULL;p->sequence=0;p->live=true;return MSG_OK;
}
MsgStatus mfp_snapshot(MfpPointer*p,MflShared**out){
    if(!p||!out)return MSG_INVALID;
    pthread_mutex_lock(&p->mutex);
    MsgStatus status=p->live?(MFL_MUTANT==53?MSG_OK:mfl_retain(p->current)):MSG_CLOSED;
    if(status==MSG_OK)*out=p->current;
    pthread_mutex_unlock(&p->mutex);return status;
}
MsgStatus mfp_replace(MfpPointer*p,MflShared**owner){
    if(!p||!owner||!*owner)return MSG_INVALID;
    pthread_mutex_lock(&p->mutex);
    if(!p->live){pthread_mutex_unlock(&p->mutex);return MSG_CLOSED;}
    if(p->sequence>UINT64_MAX-2 && MFL_MUTANT!=54){pthread_mutex_unlock(&p->mutex);return MSG_OVERFLOW;}
    MflShared*old=p->current;p->current=*owner;*owner=NULL;p->sequence+=2;
    pthread_mutex_unlock(&p->mutex);mfl_drop(&old);return MSG_OK;
}
MsgStatus mfp_destroy(MfpPointer*p){
    if(!p)return MSG_INVALID;
    pthread_mutex_lock(&p->mutex);MflShared*old=p->current;p->current=NULL;p->live=false;
    pthread_mutex_unlock(&p->mutex);pthread_mutex_destroy(&p->mutex);mfl_drop(&old);return MSG_OK;
}
