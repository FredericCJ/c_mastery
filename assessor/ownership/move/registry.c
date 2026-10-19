#include "move.h"
#ifndef OMM_MUTATION
#define OMM_MUTATION 0
#endif
typedef struct {OmmValue value;uint32_t generation;unsigned active,retired;} Slot;
struct OmmRegistry {Slot slot[2];OmmMemory memory;V4Allocator cells;};
static int empty(OmmToken t){return !t.owner&&!t.slot&&!t.generation;}
static int valid(OmmRegistry *r,OmmToken t){return r&&t.owner==r&&t.slot<2&&r->slot[t.slot].active&&(OMM_MUTATION==4||r->slot[t.slot].generation==t.generation);}
OwStatus omm_registry_create(OmmMemory m,V4Allocator a,uint32_t seed,OmmRegistry **out){
 if(!out||*out||!m.take||!m.release||!a.take||!a.release)return OW_INVALID;
 OmmRegistry *r=m.take(m.context,sizeof(*r));if(!r)return OW_NOMEM;
 *r=(OmmRegistry){0};r->memory=m;r->cells=a;
 for(unsigned i=0;i<2;++i){r->slot[i].generation=seed;r->slot[i].retired=seed==UINT32_MAX;}
 *out=r;return OW_OK;
}
OwStatus omm_registry_destroy(OmmRegistry **owner){
 if(!owner)return OW_INVALID;
 OmmRegistry *r=*owner;if(!r)return OW_OK;
 for(unsigned i=0;i<2;++i)if(r->slot[i].active)return OW_BUSY;
 OmmMemory m=r->memory;*owner=NULL;m.release(m.context,r);return OW_OK;
}
OwStatus omm_token_open(OmmRegistry *r,const int32_t *p,size_t n,OmmToken *out){
 if(!r||!out||!empty(*out)||!p||!n||n>8)return OW_INVALID;
 unsigned at=2;for(unsigned i=0;i<2;++i)if(!r->slot[i].active&&!r->slot[i].retired){at=i;break;}
 if(at==2)return OW_FULL;
 Slot *s=&r->slot[at];OwStatus code=omm_init(&s->value,r->cells,p,n);if(code!=OW_OK)return code;
 ++s->generation;s->active=1;*out=(OmmToken){r,at,s->generation};return OW_OK;
}
OwStatus omm_token_move(OmmRegistry *r,OmmToken *dst,OmmToken *src){
 if(!dst||!src||dst==src||!empty(*dst))return OW_INVALID;
 if(!valid(r,*src))return OW_STALE;
 Slot *s=&r->slot[src->slot];if(s->generation==UINT32_MAX)return OW_RETIRED;
 if(OMM_MUTATION!=3)++s->generation;
 *dst=(OmmToken){r,src->slot,s->generation};*src=(OmmToken){0};return OW_OK;
}
OwStatus omm_token_read(OmmRegistry *r,OmmToken t,size_t i,int32_t *out){if(!valid(r,t))return OW_STALE;return omm_read(&r->slot[t.slot].value,i,out);}
OwStatus omm_token_consume(OmmRegistry *r,OmmToken *t){
 if(!t)return OW_INVALID;
 if(!valid(r,*t))return OW_STALE;
 Slot *s=&r->slot[t->slot];OwStatus status=omm_consume(&s->value);if(status!=OW_OK)return status;
 s->active=0;if(s->generation==UINT32_MAX)s->retired=1;*t=(OmmToken){0};return OW_OK;
}
