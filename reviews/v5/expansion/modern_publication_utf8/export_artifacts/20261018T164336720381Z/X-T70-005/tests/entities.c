#include "dod_entities.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name="entities";
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DOD_CHECK:%s:%d:%s\n",test_name,__LINE__,#x);exit(1);}}while(0)
static void invariant(const DEntityWorld *w){
 unsigned active=0;
 for(unsigned id=0;id<4;++id){unsigned mask=w->world.ecs.mask[id];CHECK(!!mask==!!w->ids.slots[id].live);if(mask){++active;CHECK(mask<=3&&w->ids.dense[w->ids.rank[id]]==id);unsigned row=w->world.ecs.row[id];const DTable *t=&w->world.ecs.tables[mask-1];CHECK(row<t->count&&t->ids[row]==id);}}
 CHECK(active==w->ids.count);
 for(unsigned mask=1;mask<=3;++mask){const DTable *t=&w->world.ecs.tables[mask-1];CHECK(t->count<=2);for(unsigned row=0;row<t->count;++row)CHECK(w->world.ecs.mask[t->ids[row]]==mask&&w->world.ecs.row[t->ids[row]]==row);}
}
static void same(const DEntityWorld *a,const DEntityWorld *b){CHECK(a->world.version==b->world.version&&a->ids.count==b->ids.count);for(unsigned id=0;id<4;++id){CHECK(a->ids.slots[id].generation==b->ids.slots[id].generation&&a->ids.slots[id].live==b->ids.slots[id].live&&a->ids.dense[id]==b->ids.dense[id]&&a->ids.rank[id]==b->ids.rank[id]);CHECK(a->world.ecs.mask[id]==b->world.ecs.mask[id]&&a->world.ecs.row[id]==b->world.ecs.row[id]);}for(unsigned i=0;i<3;++i){const DTable *x=&a->world.ecs.tables[i],*y=&b->world.ecs.tables[i];CHECK(x->count==y->count);for(unsigned j=0;j<x->count;++j)CHECK(x->ids[j]==y->ids[j]&&x->position[j]==y->position[j]&&x->velocity[j]==y->velocity[j]);}}
static void checks(void){
 DEntityWorld w;d_entity_init(&w);DHandle h[4];CHECK(d_entity_create(&w,1,10,0,&h[0])==D_OK&&d_entity_create(&w,2,0,2,&h[1])==D_OK&&d_entity_create(&w,2,0,3,&h[2])==D_OK&&d_entity_create(&w,1,40,0,&h[3])==D_OK);invariant(&w);
 DEntityQuery q={0};CHECK(d_entity_query(&w,1,2,&q)==D_OK&&q.query.count==2);DHandle saved=q.handles[0];unsigned cached=q.query.version;
 DEntityCommand commands[2]={{h[0],3,11,1},{h[3],2,0,4}};DEntityWorld before=w;CHECK(d_entity_apply(&w,commands,2)==D_FULL);same(&w,&before);CHECK(q.query.version==cached);
 commands[1].mask=3;CHECK(d_entity_apply(&w,commands,2)==D_OK);invariant(&w);CHECK(d_entity_query(&w,1,2,&q)==D_OK&&q.query.count==0&&q.query.version!=cached);
 DEntityCommand remove={h[0],0,0,0};CHECK(d_entity_apply(&w,&remove,1)==D_OK);CHECK(d_entity_create(&w,1,99,0,&h[0])==D_OK&&h[0].slot==saved.slot&&h[0].generation!=saved.generation);DEntityCommand stale={saved,3,5,5};before=w;CHECK(d_entity_apply(&w,&stale,1)==D_STALE);same(&w,&before);CHECK(d_entity_query(&w,1,2,&q)==D_OK&&q.query.count==1&&q.handles[0].generation==h[0].generation);invariant(&w);
 DEntityWorld other;d_entity_init(&other);CHECK(d_entity_apply(&other,&stale,1)==D_STALE);w.world.version=UINT_MAX;before=w;CHECK(d_entity_apply(&w,&remove,1)==D_STALE);same(&w,&before);remove.entity=h[0];CHECK(d_entity_apply(&w,&remove,1)==D_FULL);same(&w,&before);
 d_entity_init(&w);for(unsigned i=0;i<12;++i){DHandle current;CHECK(d_entity_create(&w,1,(int)i,0,&current)==D_OK&&current.slot==i/3&&current.generation==i%3+1);DEntityCommand r={current,0,0,0};CHECK(d_entity_apply(&w,&r,1)==D_OK&&d_entity_apply(&w,&r,1)==D_STALE);invariant(&w);}DHandle untouched={0};CHECK(d_entity_create(&w,1,0,0,&untouched)==D_FULL&&untouched.owner==NULL);CHECK(d_entity_apply(&w,NULL,0)==D_OK);
 /* Deletion then stale command in one batch must roll back its generation. */
 d_entity_init(&w);CHECK(d_entity_create(&w,1,1,0,&h[0])==D_OK);commands[0]=(DEntityCommand){h[0],0,0,0};commands[1]=(DEntityCommand){h[0],2,0,2};before=w;CHECK(d_entity_apply(&w,commands,2)==D_STALE);same(&w,&before);
}
static void oracle(void){
 for(unsigned script=0;script<729;++script){DEntityWorld w;d_entity_init(&w);DHandle latest={0},first={0};unsigned code=script;
  for(unsigned step=0;step<6;++step){unsigned op=code%3;code/=3;int status;if(op==0){DHandle created={0};status=d_entity_create(&w,1,(int)step,0,&created);if(status==D_OK){latest=created;if(!first.owner)first=created;}}else{DEntityCommand c={op==1?latest:first,op==1?0:2,0,(int)step};status=d_entity_apply(&w,&c,1);}
   printf("E %u %u %d %u",script,step,status,w.world.version);for(unsigned id=0;id<4;++id)printf(" %u %u %u",w.ids.slots[id].live,w.ids.slots[id].generation,w.world.ecs.mask[id]);putchar('\n');invariant(&w);
  }
 }
}
int main(int argc,char **argv){CHECK(argc==2);test_name=argv[1];if(!strcmp(argv[1],"entities"))checks();else if(!strcmp(argv[1],"oracle"))oracle();else CHECK(0);return 0;}
