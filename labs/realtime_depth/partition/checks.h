#ifndef RP_CHECKS_H
#define RP_CHECKS_H
#include "partition.h"
static RPTask job(uint32_t cost,uint32_t period,uint32_t deadline,unsigned affinity){
 RPTask t={.job={.cost=cost,.period=period,.deadline=deadline},.affinity=affinity};
 for(unsigned c=0;c<RP_CPUS;c++)t.cost[c]=cost;
 return t;
}
static int same(const void *a,const void *b,unsigned n){const unsigned char *x=a,*y=b;for(unsigned i=0;i<n;i++)if(x[i]!=y[i])return 0;return 1;}
#define RC(x) do{if(!(x))return __LINE__;}while(0)
static int packing(void){
 RPTask t[4]={job(4,10,10,3),job(4,10,10,3),job(6,10,10,3),job(6,10,10,3)};RPPlan p;
 RC(rp_fit(t,4,2,10,0,&p)==RP_OK);
 RC(p.cpu[0]==0&&p.cpu[1]==1&&p.cpu[2]==0&&p.cpu[3]==1&&p.used[0]==10&&p.used[1]==10);
 cmx_job raw[4];for(unsigned i=0;i<4;i++)raw[i]=t[i].job;
 unsigned partial[4]={9,9,9,9};RC(cmx_partition(raw,4,2,10,partial)==CMX_FULL&&partial[0]==0&&partial[1]==0&&partial[2]==1&&partial[3]==9);
 cmx_sim_result result;RC(rp_simulate(t,4,&p,&result)==RP_OK&&result.work==20&&result.migrations==0&&result.misses==0&&result.completed==4);
 return 0;
}
static int transaction(void){
 RPTask t[3]={job(6,10,10,3),job(6,10,10,3),job(6,10,10,3)};
 RPPlan p={.n=77},saved=p;RC(rp_fit(t,3,2,10,0,&p)==RP_FULL&&same(&p,&saved,sizeof(p)));
 RC(rp_exact(t,3,2,10,0,&p)==RP_IMPOSSIBLE&&same(&p,&saved,sizeof(p)));
 t[0].job.period=0;RC(rp_fit(t,3,2,10,0,&p)==RP_INVALID&&same(&p,&saved,sizeof(p)));
 t[0]=job(1,3,3,3);RC(rp_fit(t,3,2,10,0,&p)==RP_INVALID&&same(&p,&saved,sizeof(p)));
 RC(rp_fit(t,3,2,0,0,&p)==RP_INVALID&&rp_fit(t,3,5,30,0,&p)==RP_INVALID);
 RC(rp_exact(t,0,2,30,0,&p)==RP_INVALID&&rp_exact(t,3,2,257,0,&p)==RP_INVALID);
 return 0;
}
static int demand(void){
 RPTask t[2]={job(2,5,2,1),job(2,5,2,1)};RPPlan p={0},saved=p;
 RC(rp_fit(t,2,1,5,1,&p)==RP_FULL&&same(&p,&saved,sizeof(p)));
 RC(rp_exact(t,2,1,5,1,&p)==RP_IMPOSSIBLE);
 t[0]=job(5,10,5,1);t[1]=job(1,10,10,1);
 RC(rp_fit(t,2,1,10,1,&p)==RP_OK&&p.used[0]==6&&rp_verify(t,2,&p,1)==RP_OK);
 cmx_sim_result result;RC(rp_simulate(t,2,&p,&result)==RP_OK&&result.completed==2&&result.work==6);
 p.used[0]++;RC(rp_verify(t,2,&p,1)==RP_INVALID);
 return 0;
}
static int affinity(void){
 RPTask t[2]={job(6,10,10,3),job(5,10,10,1)};RPPlan p={0},saved=p;
 RC(rp_fit(t,2,2,10,0,&p)==RP_FULL&&same(&p,&saved,sizeof(p)));
 RC(rp_exact(t,2,2,10,0,&p)==RP_OK&&p.cpu[0]==1&&p.cpu[1]==0);
 RC(rp_verify(t,2,&p,0)==RP_OK);
 t[0].cost[0]=8;t[0].cost[1]=3;t[1].cost[0]=4;t[1].cost[1]=8;
 RC(rp_exact(t,2,2,10,0,&p)==RP_OK&&p.cpu[0]==1&&p.cpu[1]==0&&p.used[0]==4&&p.used[1]==3);
 t[1].affinity=0;RC(rp_fit(t,2,2,10,0,&p)==RP_INVALID);
 return 0;
}
static int handoff(void){
 RPLease p;RC(rp_lease_init(&p,0)==RP_OK&&rp_begin(&p,0,1)==RP_OK);
 RPLease saved=p;RC(rp_prepare(&p,0,1,1)==RP_BUSY&&same(&p,&saved,sizeof(p)));
 RC(rp_finish(&p,0,1,7)==RP_OK&&rp_prepare(&p,0,1,1)==RP_OK);
 RC(rp_begin(&p,0,1)==RP_BUSY&&rp_accept(&p,0,1)==RP_STALE);
 RC(rp_abort(&p,0,1)==RP_OK&&p.owner==0&&p.epoch==1);
 RC(rp_prepare(&p,0,1,1)==RP_OK&&rp_accept(&p,1,1)==RP_OK&&p.owner==1&&p.epoch==2);
 RC(rp_begin(&p,0,1)==RP_STALE&&rp_begin(&p,1,2)==RP_OK&&rp_finish(&p,1,2,5)==RP_OK&&p.work==12);
 p.epoch=UINT64_MAX;RC(rp_prepare(&p,1,UINT64_MAX,0)==RP_FULL);
 RC(rp_begin(&p,1,UINT64_MAX)==RP_OK);p.work=UINT64_MAX;
 RC(rp_finish(&p,1,UINT64_MAX,1)==RP_FULL&&rp_finish(&p,1,UINT64_MAX,0)==RP_OK&&!p.busy);
 return 0;
}
#endif
