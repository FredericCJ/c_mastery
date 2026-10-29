#include "dag.h"
#include <stdio.h>
#define CHECK(e) do{if(!(e)){printf("DG_CHECK:%u:%s\n",(unsigned)__LINE__,#e);return 1;}}while(0)
static struct dg_spec diamond(void){struct dg_spec s={0};s.count=4;s.edge_count=4;s.costs[0]=2;s.costs[1]=3;s.costs[2]=5;s.costs[3]=1;for(unsigned i=0;i<4;i++)s.payloads[i]=i+1;s.edges[0]=(struct dg_edge){0,1};s.edges[1]=(struct dg_edge){0,2};s.edges[2]=(struct dg_edge){1,3};s.edges[3]=(struct dg_edge){2,3};return s;}
static int ordinary(void){
 struct sa_dg g;struct dg_spec s=diamond();struct dg_snapshot snapshot;struct dg_claim a,b,c,d;CHECK(dg_init(&g,1,&s)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.work==11&&snapshot.span==8&&snapshot.ready==1);
 CHECK(dg_claim(&g,0,&a)==DG_OK&&a.node==0&&a.input==1&&dg_claim(&g,1,&b)==DG_EMPTY);
 CHECK(dg_finish(&g,a,DG_DONE,a.input)==DG_OK&&dg_finish(&g,a,DG_DONE,9)==DG_STALE);
 CHECK(dg_claim(&g,0,&b)==DG_OK&&b.node==1&&b.input==3&&dg_claim(&g,1,&c)==DG_OK&&c.node==2&&c.input==4);
 CHECK(dg_finish(&g,b,DG_DONE,b.input)==DG_OK&&dg_claim(&g,0,&d)==DG_EMPTY);
 CHECK(dg_finish(&g,c,DG_DONE,c.input)==DG_OK&&dg_claim(&g,0,&d)==DG_OK&&d.node==3&&d.input==11);
 CHECK(dg_finish(&g,d,DG_DONE,d.input)==DG_OK&&dg_drained(&g)==DG_BUSY&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 CHECK(dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.terminal==4&&snapshot.results[3]==11);
 for(unsigned i=0;i<4;i++)CHECK(snapshot.ready_events[i]==1);
 s.edges[s.edge_count++]=(struct dg_edge){1,2};CHECK(dg_init(&g,2,&s)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.work==11&&snapshot.span==11);
 return 0;
}
static int validation(void){
 struct sa_dg g;struct dg_spec empty={0},s=diamond();struct dg_snapshot snapshot;CHECK(dg_init(&g,99,&empty)==DG_OK);
 s.edges[s.edge_count++]=(struct dg_edge){0,1};CHECK(dg_init(&g,1,&s)==DG_DUPLICATE&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.world==99);
 s=diamond();s.edges[s.edge_count++]=(struct dg_edge){3,0};CHECK(dg_init(&g,1,&s)==DG_CYCLE&&g.world==99);
 s=diamond();s.edges[0].to=4;CHECK(dg_init(&g,1,&s)==DG_INVALID&&g.world==99);
 s=diamond();s.costs[0]=UINT64_MAX;CHECK(dg_init(&g,1,&s)==DG_RANGE&&g.world==99);
 CHECK(dg_init(&g,1,&empty)==DG_OK&&dg_drained(&g)==DG_BUSY&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 s=(struct dg_spec){0};s.count=1;s.costs[0]=UINT64_MAX;s.payloads[0]=UINT64_MAX;struct dg_claim claim;CHECK(dg_init(&g,UINT64_MAX,&s)==DG_OK&&dg_claim(&g,0,&claim)==DG_OK&&dg_finish(&g,claim,DG_DONE,claim.input)==DG_OK&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 return 0;
}
static int failures(void){
 struct sa_dg g;struct dg_spec s=diamond();struct dg_claim a,b,c,d;struct dg_snapshot snapshot;
 CHECK(dg_init(&g,3,&s)==DG_OK&&dg_claim(&g,0,&a)==DG_OK&&dg_finish(&g,a,DG_DONE,1)==DG_OK&&dg_claim(&g,0,&b)==DG_OK&&dg_claim(&g,1,&c)==DG_OK);
 CHECK(dg_finish(&g,b,DG_FAILED,777)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[3]==DG_SKIPPED&&snapshot.results[1]==0);
 CHECK(dg_finish(&g,c,DG_DONE,c.input)==DG_OK&&dg_claim(&g,0,&d)==DG_EMPTY&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 CHECK(dg_init(&g,4,&s)==DG_OK&&dg_claim(&g,0,&a)==DG_OK&&dg_cancel(&g,0)==DG_REQUESTED&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_BUSY);
 CHECK(dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[0]==DG_RUNNING&&snapshot.terminal==0);
 CHECK(dg_finish(&g,a,DG_DONE,999)==DG_OK&&dg_drained(&g)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[0]==DG_CANCELLED&&snapshot.states[1]==DG_SKIPPED&&snapshot.states[2]==DG_SKIPPED&&snapshot.states[3]==DG_SKIPPED);
 CHECK(dg_init(&g,5,&s)==DG_OK&&dg_cancel(&g,2)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[3]==DG_SKIPPED);
 CHECK(dg_claim(&g,0,&a)==DG_OK&&dg_finish(&g,a,DG_DONE,UINT64_MAX)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[1]==DG_FAILED&&snapshot.results[1]==0);
 CHECK(dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 return 0;
}
static int runtime(void){
 struct sa_dg g;struct dg_spec empty={0};unsigned id=99,parent=0;struct dg_claim claim,child;struct dg_snapshot snapshot;
 CHECK(dg_init(&g,6,&empty)==DG_OK&&dg_append(&g,1,2,NULL,0,&id)==DG_OK&&id==0&&dg_claim(&g,0,&claim)==DG_OK);
 CHECK(dg_append(&g,2,3,&parent,1,&id)==DG_OK&&id==1&&dg_claim(&g,1,&child)==DG_EMPTY);
 CHECK(dg_finish(&g,claim,DG_DONE,7)==DG_OK&&dg_claim(&g,1,&child)==DG_OK&&child.input==9);
 CHECK(dg_finish(&g,child,DG_DONE,9)==DG_OK);
 parent=1;CHECK(dg_append(&g,4,1,&parent,1,&id)==DG_OK&&id==2&&dg_claim(&g,0,&child)==DG_OK&&child.input==13&&dg_finish(&g,child,DG_DONE,13)==DG_OK);
 unsigned invalid[2]={0,0};id=99;CHECK(dg_append(&g,1,1,invalid,2,&id)==DG_DUPLICATE&&id==99);
 invalid[0]=3;CHECK(dg_append(&g,1,1,invalid,1,&id)==DG_INVALID&&id==99);
 CHECK(dg_append(&g,UINT64_MAX,1,&parent,1,&id)==DG_RANGE&&id==99);
 CHECK(dg_append(&g,1,UINT64_MAX,NULL,0,&id)==DG_RANGE&&id==99);
 CHECK(dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.count==3&&snapshot.work==6&&snapshot.span==6);
 CHECK(dg_seal(&g)==DG_OK&&dg_append(&g,1,1,NULL,0,&id)==DG_CLOSED&&id==99&&dg_drained(&g)==DG_OK);
 CHECK(dg_init(&g,7,&empty)==DG_OK&&dg_append(&g,1,0,NULL,0,&id)==DG_OK&&dg_cancel(&g,id)==DG_OK);parent=0;
 CHECK(dg_append(&g,1,0,&parent,1,&id)==DG_OK&&dg_snapshot(&g,&snapshot)==DG_OK&&snapshot.states[1]==DG_SKIPPED&&snapshot.ready_events[1]==0&&dg_seal(&g)==DG_OK&&dg_drained(&g)==DG_OK);
 CHECK(dg_init(&g,8,&empty)==DG_OK);for(unsigned i=0;i<DG_LIMIT;i++)CHECK(dg_append(&g,i,0,NULL,0,&id)==DG_OK&&id==i);
 id=99;CHECK(dg_append(&g,1,0,NULL,0,&id)==DG_CAPACITY&&id==99);
 return 0;
}
int main(void){if(ordinary()||validation()||failures()||runtime())return 1;puts("DG_PASS:arbitrary-validation-runtime-admission-last-dependency-and-failure-ownership");return 0;}
