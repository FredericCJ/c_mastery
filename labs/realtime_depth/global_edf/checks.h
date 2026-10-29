#ifndef GE_CHECKS_H
#define GE_CHECKS_H
#include "global_edf.h"
static struct ge_result ge_result,ge_saved;
static struct ge_job gj(unsigned task,unsigned generation,unsigned release,unsigned cost,unsigned deadline,unsigned affinity){
 struct ge_job j={.task=task,.generation=generation,.affinity=affinity,.strands=1,.nominal=release,.release=release,.deadline=deadline,.cost={cost,0,0,0}};return j;
}
static int gs(const void *a,const void *b,unsigned n){const unsigned char *x=a,*y=b;for(unsigned i=0;i<n;i++)if(x[i]!=y[i])return 0;return 1;}
#define GC(x) do{if(!(x))return __LINE__;}while(0)
static int conserved(const struct ge_job *jobs,unsigned n,const struct ge_config *c,const struct ge_result *r){
 unsigned served[GE_JOBS][GE_STRANDS]={{0}},work=0,queue=0,wait=0,migrate=0,idle=0;
 for(unsigned t=0;t<c->horizon;t++){
  unsigned queues=0;
  for(unsigned cpu=0;cpu<c->cpus;cpu++){
   const struct ge_tick *v=&r->ticks[t][cpu];
   if(v->kind==GE_IDLE){idle++;continue;}if(v->kind==GE_QUEUE_WAIT){wait++;continue;}
   GC(v->job<n&&v->strand<jobs[v->job].strands&&v->generation==jobs[v->job].generation);
   GC((jobs[v->job].affinity&(1u<<cpu))&&(!c->partitioned||c->home[jobs[v->job].task]==cpu));
   if(v->kind==GE_QUEUE){queue++;queues++;continue;}if(v->kind==GE_MIGRATION){migrate++;continue;}
   GC(v->kind==GE_SERVICE);
   for(unsigned other=0;other<cpu;other++){const struct ge_tick *q=&r->ticks[t][other];GC(q->kind!=GE_SERVICE||q->job!=v->job||q->strand!=v->strand);}
   for(unsigned before=0;before<jobs[v->job].strands;before++)if(jobs[v->job].predecessors[v->strand]&(1u<<before))GC(served[v->job][before]==jobs[v->job].cost[before]);
  }
  GC(queues<=1);
  for(unsigned cpu=0;cpu<c->cpus;cpu++){
   const struct ge_tick *v=&r->ticks[t][cpu];if(v->kind==GE_SERVICE){served[v->job][v->strand]++;work++;GC(served[v->job][v->strand]<=jobs[v->job].cost[v->strand]);}
  }
 }
 GC(work==r->work&&queue==r->queue&&wait==r->queue_wait&&migrate==r->migration_overhead&&idle==r->idle);
 GC(work+queue+wait+migrate+idle==c->horizon*c->cpus);
 for(unsigned i=0;i<n;i++)for(unsigned v=0;v<jobs[i].strands;v++){
  GC(served[i][v]==r->jobs[i].executed[v]);
  if(r->jobs[i].state==GE_ACTIVE||r->jobs[i].state==GE_COMPLETE)GC(served[i][v]+r->jobs[i].remaining[v]==jobs[i].cost[v]);
  else GC(served[i][v]==0);
 }
 return 0;
}
static int dhall(void){
 struct ge_job jobs[3]={gj(0,1,0,10,10,3),gj(1,1,0,1,9,3),gj(2,1,0,1,9,3)};struct ge_config c={.cpus=2,.horizon=12};
 GC(ge_simulate(jobs,3,&c,&ge_result)==GE_OK&&conserved(jobs,3,&c,&ge_result)==0);
 GC(ge_result.jobs[0].start==1&&ge_result.jobs[0].finish==11&&ge_result.jobs[0].missed&&ge_result.misses==1&&ge_result.completed==3&&ge_result.work==12);
 const unsigned scale[]={10,20,50,100};
 for(unsigned i=0;i<4;i++){
  unsigned n=scale[i];jobs[0]=gj(0,1,0,n,n,3);jobs[1]=gj(1,1,0,1,n-1,3);jobs[2]=gj(2,1,0,1,n-1,3);c.horizon=n+2;
  GC(ge_simulate(jobs,3,&c,&ge_result)==GE_OK&&ge_result.jobs[0].finish==n+1&&ge_result.misses==1&&conserved(jobs,3,&c,&ge_result)==0);
 }
 return 0;
}
static int dag(void){
 struct ge_job j={.task=0,.generation=3,.affinity=3,.strands=4,.deadline=8,.cost={1,2,2,1},.predecessors={0,1,1,6}};struct ge_config c={.cpus=2,.horizon=8};
 GC(ge_simulate(&j,1,&c,&ge_result)==GE_OK&&conserved(&j,1,&c,&ge_result)==0);
 GC(ge_result.jobs[0].finish==4&&ge_result.work==6&&ge_result.misses==0);
 ge_saved=ge_result;j.predecessors[0]=8;
 GC(ge_simulate(&j,1,&c,&ge_result)==GE_INVALID&&gs(&ge_result,&ge_saved,sizeof(ge_result)));
 return 0;
}
static int overlap(void){
 struct ge_job j[2]={gj(0,1,0,4,3,3),gj(0,2,2,1,5,3)};struct ge_config c={.cpus=2,.horizon=6};
 GC(ge_simulate(j,2,&c,&ge_result)==GE_OK&&conserved(j,2,&c,&ge_result)==0);
 GC(ge_result.jobs[0].finish==4&&ge_result.jobs[0].missed&&ge_result.jobs[1].state==GE_REJECTED&&ge_result.rejected==1&&ge_result.work==4);
 j[1].nominal=j[1].release=4;
 GC(ge_simulate(j,2,&c,&ge_result)==GE_OK&&ge_result.completed==2&&ge_result.rejected==0);
 j[1].generation=1;ge_saved=ge_result;GC(ge_simulate(j,2,&c,&ge_result)==GE_INVALID&&gs(&ge_result,&ge_saved,sizeof(ge_result)));
 return 0;
}
static int costs(void){
 struct ge_job j[3]={gj(0,1,0,3,9,3),gj(1,1,0,3,10,3),gj(2,1,1,1,5,3)};struct ge_config c={.cpus=2,.horizon=12,.migration_cost=1};
 GC(ge_simulate(j,3,&c,&ge_result)==GE_OK&&conserved(j,3,&c,&ge_result)==0);
 GC(ge_result.migrations>=1&&ge_result.migration_overhead==ge_result.migrations&&ge_result.work==7);
 struct ge_job two[2]={gj(0,1,0,2,10,1),gj(1,1,0,2,10,2)};c.queue_cost=1;c.migration_cost=0;
 GC(ge_simulate(two,2,&c,&ge_result)==GE_OK&&conserved(two,2,&c,&ge_result)==0);
 GC(ge_result.queue==2&&ge_result.queue_wait==1&&ge_result.jobs[0].finish==3&&ge_result.jobs[1].finish==4);
 two[0]=gj(0,1,2,2,3,3);two[0].nominal=0;two[0].jitter=2;c.queue_cost=0;
 GC(ge_simulate(two,2,&c,&ge_result)==GE_OK&&ge_result.jobs[0].missed&&ge_result.jobs[0].finish==4);
 return 0;
}
static int affinity(void){
 struct ge_job j[2]={gj(0,1,0,3,4,2),gj(1,1,0,3,4,2)};struct ge_config c={.cpus=2,.horizon=8};
 GC(ge_simulate(j,2,&c,&ge_result)==GE_OK&&conserved(j,2,&c,&ge_result)==0);
 GC(ge_result.work==6&&ge_result.misses==1&&ge_result.jobs[1].finish==6);
 j[0].affinity=1;
 GC(ge_simulate(j,2,&c,&ge_result)==GE_OK&&ge_result.misses==0&&ge_result.jobs[0].finish==3&&ge_result.jobs[1].finish==3);
 return 0;
}
static int comparison(void){
 struct ge_job j[3]={gj(0,1,0,2,10,3),gj(1,1,0,2,10,3),gj(2,1,0,2,10,3)};struct ge_config c={.cpus=2,.horizon=10};
 cmx_job producer[3]={{.cost=2,.deadline=10,.period=10},{.cost=2,.deadline=10,.period=10},{.cost=2,.deadline=10,.period=10}};
 cmx_sim_result result;GC(cmx_simulate(producer,3,2,0,10,&result)==CMX_OK);
 GC(ge_simulate(j,3,&c,&ge_result)==GE_OK&&ge_result.work==result.work&&ge_result.completed==result.completed&&ge_result.migrations==result.migrations&&ge_result.misses==result.misses);
 c.partitioned=1;c.home[0]=0;c.home[1]=1;c.home[2]=0;unsigned homes[3]={0,1,0};
 GC(cmx_simulate(producer,3,2,homes,10,&result)==CMX_OK&&ge_simulate(j,3,&c,&ge_result)==GE_OK&&ge_result.migrations==0&&ge_result.work==result.work);
 unsigned sufficient=9;GC(ge_sufficient(producer,3,2,10,&sufficient)==GE_OK&&sufficient==1);
 producer[0].cost=10;producer[1].cost=producer[2].cost=1;
 GC(ge_sufficient(producer,3,2,10,&sufficient)==GE_OK&&sufficient==0);
 producer[0].deadline=9;sufficient=9;GC(ge_sufficient(producer,3,2,10,&sufficient)==GE_INVALID&&sufficient==9);
 producer[0]=(cmx_job){.cost=9,.period=10,.deadline=10};producer[1]=(cmx_job){.cost=3,.period=10,.deadline=10};
 GC(ge_sufficient(producer,2,2,10,&sufficient)==GE_OK&&sufficient==0);
 j[0]=gj(0,1,0,9,10,3);j[1]=gj(1,1,0,3,10,3);c.partitioned=0;
 GC(ge_simulate(j,2,&c,&ge_result)==GE_OK&&ge_result.misses==0&&ge_result.completed==2);
 ge_saved=ge_result;c.horizon=257;
 GC(ge_simulate(j,2,&c,&ge_result)==GE_INVALID&&gs(&ge_result,&ge_saved,sizeof(ge_result)));
 return 0;
}
#endif
