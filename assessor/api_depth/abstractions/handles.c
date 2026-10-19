#include "handles.h"
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
void ah_pool_init(AhPool *p) { if(p){*p=(AhPool){0};p->epoch=1;for(unsigned i=0;i<3;++i)p->slots[i].generation=1;} }
static int valid(const AhPool *p,AhHandle h) {
 if(!p||h.index>=3||h.epoch!=p->epoch||!h.generation)return 0;
 const AhSlot *s=p->slots+h.index;return s->occupied&&h.generation==s->generation;
}
AhResult ah_allocate(AhPool *p,unsigned value,AhHandle *out) {
 if(!p||!out||!p->epoch)return AH_INVALID;
 for(unsigned i=0;i<3;++i)if(!p->slots[i].occupied&&!p->slots[i].retired){p->slots[i].occupied=1;p->slots[i].value=value;*out=(AhHandle){i,p->slots[i].generation,p->epoch};return AH_OK;}
 return AH_FULL;
}
AhResult ah_read(const AhPool *p,AhHandle h,unsigned *out) {
 if(!p||!out)return AH_INVALID;
 if(!valid(p,h))return AH_STALE;
 *out=p->slots[h.index].value;return AH_OK;
}
AhResult ah_release(AhPool *p,AhHandle h) {
 if(!p)return AH_INVALID;
 if(!valid(p,h))return AH_STALE;
 AhSlot *s=p->slots+h.index;s->occupied=0;
#ifdef AH_MUTANT_WRAP
 s->generation=s->generation==3?1:s->generation+1;
#else
 if(s->generation==3)s->retired=1;else ++s->generation;
#endif
 return AH_OK;
}
AhResult ah_enter(AhPool *p){if(!p||!p->epoch)return AH_INVALID;if(p->borrowers==3)return AH_FULL;++p->borrowers;return AH_OK;}
AhResult ah_leave(AhPool *p){if(!p||!p->borrowers)return AH_INVALID;--p->borrowers;return AH_OK;}
AhResult ah_reuse(AhPool *p) {
 if(!p||!p->epoch)return AH_INVALID;
#ifndef AH_MUTANT_QUIESCENCE
 if(p->borrowers)return AH_BUSY;
#endif
 for(unsigned i=0;i<3;++i)if(p->slots[i].occupied)return AH_BUSY;
 if(p->epoch==3)return AH_FULL;
 unsigned next=p->epoch+1;ah_pool_init(p);p->epoch=next;return AH_OK;
}
struct AhShared { pthread_mutex_t lock;unsigned strong,weak;unsigned *payload;AhStats *stats; };
static void dispose(AhShared *s){AhStats *stats=s->stats;pthread_mutex_destroy(&s->lock);free(s);atomic_fetch_add(&stats->control_frees,1);}
AhResult ah_shared_create(unsigned value,AhStats *stats,AhShared **out) {
 if(!stats||!out)return AH_INVALID;
 AhShared *s=malloc(sizeof *s);if(!s)return AH_NOMEM;
 unsigned *p=malloc(sizeof *p);if(!p){free(s);return AH_NOMEM;}
 if(pthread_mutex_init(&s->lock,0)){free(p);free(s);return AH_NOMEM;}
 *p=value;s->payload=p;s->strong=1;s->weak=1;s->stats=stats;
 atomic_fetch_add(&stats->controls,1);atomic_fetch_add(&stats->payloads,1);*out=s;return AH_OK;
}
AhResult ah_promote(AhShared *s) {
 if(!s)return AH_INVALID;
 pthread_mutex_lock(&s->lock);AhResult result=AH_OK;
#ifdef AH_MUTANT_RESURRECT
 if(s->strong==0)s->strong=1;
#else
 if(!s->strong)result=AH_DEAD;
#endif
 if(result==AH_OK){if(s->strong==3)result=AH_FULL;else ++s->strong;}
 pthread_mutex_unlock(&s->lock);return result;
}
AhResult ah_retain(AhShared *s){return ah_promote(s);}
AhResult ah_drop(AhShared *s) {
 if(!s)return AH_INVALID;
 pthread_mutex_lock(&s->lock);
 if(!s->strong){pthread_mutex_unlock(&s->lock);return AH_DEAD;}
 if(!--s->strong){free(s->payload);s->payload=0;atomic_fetch_add(&s->stats->payload_frees,1);--s->weak;}
 int last=!s->weak;pthread_mutex_unlock(&s->lock);if(last)dispose(s);return AH_OK;
}
AhResult ah_weak_retain(AhShared *s) {
 if(!s)return AH_INVALID;
 pthread_mutex_lock(&s->lock);AhResult result=AH_FULL;
 if(s->weak<3){++s->weak;result=AH_OK;}pthread_mutex_unlock(&s->lock);return result;
}
AhResult ah_weak_drop(AhShared *s) {
 if(!s)return AH_INVALID;
 pthread_mutex_lock(&s->lock);
 if(s->weak<=(s->strong?1u:0u)){pthread_mutex_unlock(&s->lock);return AH_INVALID;}
 int last=!--s->weak;pthread_mutex_unlock(&s->lock);if(last)dispose(s);return AH_OK;
}
AhResult ah_counts(AhShared *s,unsigned *strong,unsigned *weak) {
 if(!s||!strong||!weak)return AH_INVALID;
 pthread_mutex_lock(&s->lock);*strong=s->strong;*weak=s->weak;pthread_mutex_unlock(&s->lock);return AH_OK;
}
AhResult ah_add(AhShared *s,unsigned value) {
 if(!s)return AH_INVALID;
 pthread_mutex_lock(&s->lock);AhResult result=AH_OK;
 if(!s->strong)result=AH_DEAD;else if(value>UINT_MAX-*s->payload)result=AH_FULL;else *s->payload+=value;
 pthread_mutex_unlock(&s->lock);return result;
}
unsigned *ah_payload(AhShared *s){return s?s->payload:0;}
