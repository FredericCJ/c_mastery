#ifndef CMASTERY_DOD_ENTITIES_H
#define CMASTERY_DOD_ENTITIES_H
#include "dod_ext.h"
typedef struct {DSparse ids;DQueryWorld world;} DEntityWorld;
typedef struct {DHandle entity;unsigned mask;int position,velocity;} DEntityCommand;
typedef struct {DQuery query;DHandle handles[4];} DEntityQuery;
void d_entity_init(DEntityWorld *);
int d_entity_create(DEntityWorld *,unsigned mask,int position,int velocity,DHandle *);
int d_entity_apply(DEntityWorld *,const DEntityCommand *,size_t);
int d_entity_query(const DEntityWorld *,unsigned required,unsigned excluded,DEntityQuery *);
#endif
