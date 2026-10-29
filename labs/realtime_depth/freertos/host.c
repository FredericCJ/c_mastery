#include "checks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void fc_fail(unsigned line){fprintf(stderr,"FC_CHECK:%u\n",line);exit(1);}
void fc_done(const struct fc_report *r){unsigned check=(unsigned)fc_checks(r);if(check)fc_fail(check);
 printf("FC_PASS count=%u sum=%u full=%u empty=%u boosted=%u restored=%u closed=%u trace=%u,%u,%u ticks=%u elapsed=%u isr=%u nested=%u\n",r->count,r->sum,r->full,r->empty,r->boosted,r->restored,r->closed,r->trace[0],r->trace[1],r->trace[2],r->ticks,r->elapsed,r->isr,r->nested_retained);}
int main(int argc,char **argv){
 if(argc==2&&!strcmp(argv[1],"mapping")){unsigned ms,rate,urgency;while(scanf("%u %u %u",&ms,&rate,&urgency)==3){unsigned out=55;int valid=fc_ticks(ms,rate,&out);printf("M %u %u %u %d %u %u\n",ms,rate,urgency,valid,out,fc_priority(urgency));}return 0;}
 if(argc!=1)return 2;
 struct fc_report r={0};fc_run(&r);fc_done(&r);return 0;}
