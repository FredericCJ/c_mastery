#include "checks.h"
#include <stdio.h>
#include <string.h>
static int stream(void){
 char kind;while(scanf(" %c",&kind)==1){
  struct od_set set={0};unsigned n,h;
  if(kind=='A'){
   if(scanf("%u%u",&n,&h)!=2||n>OD_TASKS)return 2;
   enum od_decision d=OD_INCONCLUSIVE;
   for(unsigned i=0;i<n;i++){struct od_task t;if(scanf("%u%u%u",&t.cost,&t.period,&t.critical)!=3)return 2;
    enum od_status s=od_admit(&set,t,h,&d);printf("A %u %u %u\n",s,d,set.count);}
  }else if(kind=='R'){
   unsigned cap,policy,supervisor;uint32_t sw,re;
   if(scanf("%u%u%u%u%u%u%u%u",&set.count,&n,&cap,&h,&policy,&supervisor,&sw,&re)!=8||set.count>OD_TASKS||n>OD_REQUESTS)return 2;
   for(unsigned i=0;i<set.count;i++)if(scanf("%u%u%u",&set.tasks[i].cost,&set.tasks[i].period,&set.tasks[i].critical)!=3)return 2;
   struct od_request r[OD_REQUESTS];struct od_result o;
   for(unsigned i=0;i<n;i++)if(scanf("%u%u%u%u%u%u",&r[i].id,&r[i].stream,&r[i].release,&r[i].cost,&r[i].deadline,&r[i].io_until)!=6)return 2;
   enum od_status s=od_run(&set,r,n,cap,h,(enum od_policy)policy,supervisor,sw,re,&o);printf("R %u\n",s);if(s!=OD_OK)continue;
   printf("S %u %u %u %u %u %u %u\n",o.peak,o.heartbeats,o.work,o.idle,o.mode_switches,o.recovered,o.recovery_refused);
   for(unsigned i=0;i<n;i++)printf("I %u %u %u\n",o.items[i].state,o.items[i].executed,o.items[i].finish);
   for(unsigned t=0;t<h;t++)printf("T %u %u %u %u\n",o.ticks[t].selected,o.ticks[t].pending,o.ticks[t].mode,o.ticks[t].heartbeat);
  }else return 2;
 }return 0;
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"stream"))return stream();
 const char *names[]={"admission","queue","policy","overrun","mode","io","backpressure","guards"};
 int (*checks[])(void)={od_check_admission,od_check_queue,od_check_policy,od_check_overrun,od_check_mode,od_check_io,od_check_backpressure,od_check_guards};
 for(unsigned i=0;i<8;i++){if(!strcmp(argv[1],names[i])){int line=checks[i]();if(line){fprintf(stderr,"OD_CHECK:%s:%d\n",names[i],line);return 1;}printf("OD_PASS:%s\n",names[i]);return 0;}}return 2;
}
