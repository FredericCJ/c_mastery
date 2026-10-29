#ifndef CMASTERY_DOD_ENTITIES_H
#define CMASTERY_DOD_ENTITIES_H
#include "dod_ext.h"
struct d_entity_world {struct d_sparse ids;struct d_query_world world;};
struct d_entity_command {struct d_handle entity;unsigned mask;int position,velocity;};
struct d_entity_query {struct d_query query;struct d_handle handles[4];};
void d_entity_init(struct d_entity_world *);
int d_entity_create(struct d_entity_world *,unsigned mask,int position,int velocity,struct d_handle *);
int d_entity_apply(struct d_entity_world *,const struct d_entity_command *,size_t);
int d_entity_query(const struct d_entity_world *,unsigned required,unsigned excluded,struct d_entity_query *);
#endif
