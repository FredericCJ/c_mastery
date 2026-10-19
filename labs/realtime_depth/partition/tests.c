#include "checks.h"
#include <stdio.h>
#include <string.h>
static int stream(void){
 unsigned n,cpus,h,demand_flag;
 while(scanf("%u%u%u%u",&n,&cpus,&h,&demand_flag)==4){
  if(!n||n>RP_TASKS||!cpus||cpus>RP_CPUS)return 2;
  RPTask t[RP_TASKS];for(unsigned i=0;i<n;i++){
   unsigned period,deadline,affinity,cost[RP_CPUS]={0};
   if(scanf("%u%u%u",&period,&deadline,&affinity)!=3)return 2;
   for(unsigned c=0;c<cpus;c++)if(scanf("%u",&cost[c])!=1)return 2;
   t[i]=job(cost[0],period,deadline,affinity);for(unsigned c=0;c<cpus;c++)t[i].cost[c]=cost[c];
  }
  RPPlan p={0},e={0};enum rp_status a=rp_fit(t,n,cpus,h,demand_flag,&p),b=rp_exact(t,n,cpus,h,demand_flag,&e);
  printf("P %u %u",a,b);
  if(a==RP_OK){for(unsigned i=0;i<n;i++)printf(" %u",p.cpu[i]);for(unsigned c=0;c<cpus;c++)printf(" %u",p.used[c]);
   cmx_sim_result r;enum rp_status s=rp_simulate(t,n,&p,&r);printf(" S %u",s);if(s==RP_OK)printf(" %u %u %u %u",r.work,r.completed,r.misses,r.migrations);
  }
  if(b==RP_OK){printf(" E");for(unsigned i=0;i<n;i++)printf(" %u",e.cpu[i]);}
  puts("");
 }
 return ferror(stdin)?2:0;
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"stream"))return stream();
 int result;
 if(!strcmp(argv[1],"packing"))result=packing();else if(!strcmp(argv[1],"transaction"))result=transaction();
 else if(!strcmp(argv[1],"demand"))result=demand();else if(!strcmp(argv[1],"affinity"))result=affinity();
 else if(!strcmp(argv[1],"handoff"))result=handoff();else return 2;
 if(result){printf("RP_CHECK:%d\n",result);return 1;}printf("RP_PASS:%s\n",argv[1]);return 0;
}
