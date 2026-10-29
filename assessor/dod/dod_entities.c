#include "dod_entities.h"
#include <limits.h>
#ifndef D_ENTITY_MUTANT
#define D_ENTITY_MUTANT 0
#endif
void d_entity_init(struct d_entity_world *w){*w=(struct d_entity_world){0};d_sparse_init(&w->ids);d_query_world_init(&w->world);}
int d_entity_create(struct d_entity_world *w,unsigned mask,int position,int velocity,struct d_handle *out){
 if(!w||!out||mask<1||mask>3)return D_INVALID;
 struct d_entity_world next=*w;struct d_handle h;int status=d_sparse_add(&next.ids,0,&h);if(status)return status;
 status=d_query_set(&next.world,h.slot,mask,position,velocity);if(status)return status;
 *w=next;h.owner=&w->ids;*out=h;return D_OK;
}
static int apply(struct d_entity_world *next,const struct d_entity_world *owner,struct d_entity_command command){
 if(command.entity.owner!=&owner->ids)return D_STALE;
 struct d_handle h=command.entity;h.owner=&next->ids;
#if D_ENTITY_MUTANT == 1
 if(h.slot<4)h.generation=next->ids.slots[h.slot].generation;
#endif
 int unused,status=d_sparse_get(&next->ids,h,&unused);if(status)return status;
 if(command.mask)return d_query_set(&next->world,h.slot,command.mask,command.position,command.velocity);
 if(next->world.version==UINT_MAX)return D_FULL;
 struct d_ecs *e=&next->world.ecs;unsigned mask=e->mask[h.slot],row=e->row[h.slot];struct d_table *t=&e->tables[mask-1];unsigned last=--t->count,moved=t->ids[last];
 if(row!=last){t->ids[row]=moved;t->position[row]=t->position[last];t->velocity[row]=t->velocity[last];e->row[moved]=row;}
 e->mask[h.slot]=0;e->row[h.slot]=0;++next->world.version;
 return d_sparse_remove(&next->ids,h);
}
int d_entity_apply(struct d_entity_world *w,const struct d_entity_command *commands,size_t n){
 if(!w||(!commands&&n)||n>4)return D_INVALID;
 struct d_entity_world next=*w;
 for(size_t i=0;i<n;++i){if(commands[i].mask>3)return D_INVALID;int status=apply(&next,w,commands[i]);if(status)return status;
#if D_ENTITY_MUTANT == 2
  *w=next;
#endif
 }
 *w=next;return D_OK;
}
int d_entity_query(const struct d_entity_world *w,unsigned required,unsigned excluded,struct d_entity_query *out){
 if(!w||!out)return D_INVALID;
 struct d_entity_query result=*out;int status=d_query_get(&w->world,required,excluded,&result.query);if(status)return status;
 for(unsigned i=0;i<result.query.count;++i){unsigned id=result.query.ids[i];result.handles[i]=(struct d_handle){&w->ids,id,w->ids.slots[id].generation};}
 *out=result;return D_OK;
}
