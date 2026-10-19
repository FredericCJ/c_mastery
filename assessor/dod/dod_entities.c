#include "dod_entities.h"
#include <limits.h>
#ifndef D_ENTITY_MUTANT
#define D_ENTITY_MUTANT 0
#endif
void d_entity_init(DEntityWorld *w){*w=(DEntityWorld){0};d_sparse_init(&w->ids);d_query_world_init(&w->world);}
int d_entity_create(DEntityWorld *w,unsigned mask,int position,int velocity,DHandle *out){
 if(!w||!out||mask<1||mask>3)return D_INVALID;
 DEntityWorld next=*w;DHandle h;int status=d_sparse_add(&next.ids,0,&h);if(status)return status;
 status=d_query_set(&next.world,h.slot,mask,position,velocity);if(status)return status;
 *w=next;h.owner=&w->ids;*out=h;return D_OK;
}
static int apply(DEntityWorld *next,const DEntityWorld *owner,DEntityCommand command){
 if(command.entity.owner!=&owner->ids)return D_STALE;
 DHandle h=command.entity;h.owner=&next->ids;
#if D_ENTITY_MUTANT == 1
 if(h.slot<4)h.generation=next->ids.slots[h.slot].generation;
#endif
 int unused,status=d_sparse_get(&next->ids,h,&unused);if(status)return status;
 if(command.mask)return d_query_set(&next->world,h.slot,command.mask,command.position,command.velocity);
 if(next->world.version==UINT_MAX)return D_FULL;
 DEcs *e=&next->world.ecs;unsigned mask=e->mask[h.slot],row=e->row[h.slot];DTable *t=&e->tables[mask-1];unsigned last=--t->count,moved=t->ids[last];
 if(row!=last){t->ids[row]=moved;t->position[row]=t->position[last];t->velocity[row]=t->velocity[last];e->row[moved]=row;}
 e->mask[h.slot]=0;e->row[h.slot]=0;++next->world.version;
 return d_sparse_remove(&next->ids,h);
}
int d_entity_apply(DEntityWorld *w,const DEntityCommand *commands,size_t n){
 if(!w||(!commands&&n)||n>4)return D_INVALID;
 DEntityWorld next=*w;
 for(size_t i=0;i<n;++i){if(commands[i].mask>3)return D_INVALID;int status=apply(&next,w,commands[i]);if(status)return status;
#if D_ENTITY_MUTANT == 2
  *w=next;
#endif
 }
 *w=next;return D_OK;
}
int d_entity_query(const DEntityWorld *w,unsigned required,unsigned excluded,DEntityQuery *out){
 if(!w||!out)return D_INVALID;
 DEntityQuery result=*out;int status=d_query_get(&w->world,required,excluded,&result.query);if(status)return status;
 for(unsigned i=0;i<result.query.count;++i){unsigned id=result.query.ids[i];result.handles[i]=(DHandle){&w->ids,id,w->ids.slots[id].generation};}
 *out=result;return D_OK;
}
