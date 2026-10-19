#include "global_edf.h"
#ifndef GE_MUTANT
#define GE_MUTANT 0
#endif
static int valid(const GEJob *jobs,unsigned n,const GEConfig *c){
 if(!jobs||!c||!n||n>GE_JOBS||!c->cpus||c->cpus>2||!c->horizon||c->horizon>GE_HORIZON||c->partitioned>1||c->queue_cost>32||c->migration_cost>32)return 0;
 for(unsigned i=0;i<n;i++){
  const GEJob *j=&jobs[i];
  if(j->task>=GE_TASKS||!j->generation||!j->affinity||(j->affinity>>c->cpus)||!j->strands||j->strands>GE_STRANDS||
     j->release<j->nominal||j->release-j->nominal>j->jitter||j->nominal>=j->deadline||j->deadline>GE_HORIZON||j->release>GE_HORIZON||
     (c->partitioned&&(c->home[j->task]>=c->cpus||!(j->affinity&(1u<<c->home[j->task])))))return 0;
  for(unsigned k=0;k<i;k++)if(jobs[k].task==j->task&&jobs[k].generation==j->generation)return 0;
  unsigned done=0;
  for(unsigned v=0;v<j->strands;v++)if(!j->cost[v]||j->cost[v]>GE_HORIZON||(j->predecessors[v]>>j->strands)||(j->predecessors[v]&(1u<<v)))return 0;
  for(unsigned pass=0;pass<j->strands;pass++)for(unsigned v=0;v<j->strands;v++)if((j->predecessors[v]&done)==j->predecessors[v])done|=1u<<v;
  if(done!=(1u<<j->strands)-1u)return 0;
 }
 return 1;
}
typedef struct {unsigned node,stage,left,last;} CPU;
static unsigned best(const GEJob *jobs,unsigned n,const GEConfig *cfg,uint64_t ready,uint64_t excluded,unsigned cpu){
 unsigned answer=GE_NONE;
 for(unsigned i=0;i<n;i++)if((GE_MUTANT==5||(jobs[i].affinity&(1u<<cpu)))&&(!cfg->partitioned||cfg->home[jobs[i].task]==cpu)){
  for(unsigned v=0;v<jobs[i].strands;v++){
   unsigned node=i*GE_STRANDS+v;uint64_t bit=UINT64_C(1)<<node;
   if(!(ready&bit)||(GE_MUTANT!=1&&(excluded&bit)))continue;
   if(answer==GE_NONE||jobs[i].deadline<jobs[answer/GE_STRANDS].deadline)answer=node;
  }
 }
 return answer;
}
enum ge_status ge_simulate(const GEJob *jobs,unsigned n,const GEConfig *cfg,GEResult *out){
 if(!out||!valid(jobs,n,cfg))return GE_INVALID;
 GEResult r={0};CPU cpu[2]={{GE_NONE,0,0,GE_NONE},{GE_NONE,0,0,GE_NONE}};
 unsigned last_cpu[GE_JOBS*GE_STRANDS],lock_owner=2;
 for(unsigned i=0;i<GE_JOBS*GE_STRANDS;i++)last_cpu[i]=2;
 for(unsigned i=0;i<n;i++){r.jobs[i].start=GE_NONE;r.jobs[i].finish=GE_NONE;}
 for(unsigned tick=0;tick<=cfg->horizon;tick++){
  if(tick<cfg->horizon)for(unsigned i=0;i<n;i++)if(jobs[i].release==tick){
   unsigned active=0;
   for(unsigned k=0;k<n;k++)if(k!=i&&jobs[k].task==jobs[i].task&&r.jobs[k].state==GE_ACTIVE){
    active=1;if(GE_MUTANT==4)r.jobs[k].state=GE_REJECTED;
   }
   if(active&&GE_MUTANT!=4){r.jobs[i].state=GE_REJECTED;r.rejected++;}
   else {r.jobs[i].state=GE_ACTIVE;for(unsigned v=0;v<jobs[i].strands;v++)r.jobs[i].remaining[v]=jobs[i].cost[v];}
  }
  for(unsigned i=0;i<n;i++)if(r.jobs[i].state==GE_ACTIVE&&!r.jobs[i].missed&&jobs[i].deadline<=tick){r.jobs[i].missed=1;r.misses++;}
  if(tick==cfg->horizon)break;
  uint64_t ready=0,reserved=0,chosen=0;unsigned release_lock=0;
  for(unsigned i=0;i<n;i++)if(r.jobs[i].state==GE_ACTIVE)for(unsigned v=0;v<jobs[i].strands;v++)
   if(r.jobs[i].remaining[v]&&(GE_MUTANT==3||(jobs[i].predecessors[v]&r.jobs[i].complete_mask)==jobs[i].predecessors[v]))ready|=UINT64_C(1)<<(i*GE_STRANDS+v);
  for(unsigned c=0;c<cfg->cpus;c++)if(cpu[c].stage)reserved|=UINT64_C(1)<<cpu[c].node;
  for(unsigned c=0;c<cfg->cpus;c++){
   CPU *p=&cpu[c];GETick *trace=&r.ticks[tick][c];*trace=(GETick){.kind=GE_IDLE,.job=GE_NONE,.strand=GE_NONE};
   if(!p->stage){
    unsigned node=best(jobs,n,cfg,ready,reserved|chosen,c);
    if(node==GE_NONE){r.idle++;p->last=GE_NONE;continue;}
    if(node!=p->last||last_cpu[node]!=c){
     unsigned q=GE_MUTANT==6?0:cfg->queue_cost;
     if(q&&lock_owner!=2){trace->kind=GE_QUEUE_WAIT;r.queue_wait++;p->last=GE_NONE;continue;}
     p->node=node;p->stage=q?1:2;p->left=q;
     if(q)lock_owner=c;
     reserved|=UINT64_C(1)<<node;
    }else{p->node=node;p->stage=3;reserved|=UINT64_C(1)<<node;}
   }
   unsigned node=p->node,i=node/GE_STRANDS,v=node%GE_STRANDS;
   trace->job=i;trace->strand=v;trace->generation=jobs[i].generation;trace->remaining=r.jobs[i].remaining[v];
   if(p->stage==1){
    trace->kind=GE_QUEUE;r.queue++;p->left--;
    if(!p->left){release_lock=1;p->stage=2;}
    continue;
   }
   if(p->stage==2){
    p->left=(GE_MUTANT!=2&&last_cpu[node]!=2&&last_cpu[node]!=c)?cfg->migration_cost:0;
    p->stage=p->left?4:3;
   }
   if(p->stage==4){trace->kind=GE_MIGRATION;r.migration_overhead++;p->left--;if(!p->left)p->stage=3;continue;}
   /* The paid dispatch executes one service unit before the next EDF choice.
    * A release during bounded overhead does not repay or preempt that charge. */
   trace->kind=GE_SERVICE;chosen|=UINT64_C(1)<<node;
   if(last_cpu[node]!=2&&last_cpu[node]!=c)r.migrations++;
   last_cpu[node]=c;
   if(r.jobs[i].start==GE_NONE)r.jobs[i].start=tick;
   if(r.jobs[i].remaining[v])r.jobs[i].remaining[v]--;
   r.jobs[i].executed[v]++;r.work++;trace->remaining=r.jobs[i].remaining[v];
   if(!r.jobs[i].remaining[v])r.jobs[i].complete_mask|=1u<<v;
   if(r.jobs[i].complete_mask==(1u<<jobs[i].strands)-1u){r.jobs[i].state=GE_COMPLETE;r.jobs[i].finish=tick+1;r.completed++;}
   reserved&=~(UINT64_C(1)<<node);p->stage=0;p->last=node;
  }
  if(release_lock)lock_owner=2;
 }
 *out=r;return GE_OK;
}
enum ge_status ge_sufficient(const cmx_job *jobs,unsigned n,unsigned cpus,uint32_t h,unsigned *out){
 if(!jobs||!out||!n||n>GE_TASKS||!cpus||cpus>2||!h||h>GE_HORIZON)return GE_INVALID;
 uint32_t total=0,max=0;
 for(unsigned i=0;i<n;i++){
  if(!jobs[i].period||h%jobs[i].period||jobs[i].cost>jobs[i].period||jobs[i].deadline!=jobs[i].period||jobs[i].jitter||jobs[i].blocking)return GE_INVALID;
  uint32_t w=(h/jobs[i].period)*jobs[i].cost;total+=w;if(w>max)max=w;
 }
 *out=total<=cpus*h-(cpus-1)*max;return GE_OK;
}
