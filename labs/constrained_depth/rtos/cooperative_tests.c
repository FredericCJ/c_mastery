#include "cooperative.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("CR_COOP_CHECK:%u:%s\n",(unsigned)__LINE__,#x);return 1;}}while(0)
int main(void){
 for(unsigned cut=0;cut<=101;cut++){
  CrCoopReport result={0};CHECK(cr_cooperative(cut,1000,0,&result)==CR_OK);unsigned n=cut<100?cut:100,sum=0;for(unsigned i=0;i<n;i++)sum+=i*(i%16+1);CHECK(result.jobs==n&&result.sum==sum&&result.steps>0);printf("CR_COOP_ROW:%u,%u,%u,%u,%u,%u\n",cut,result.jobs,result.sum,result.steps,result.blocked,result.wakes);
 }
 CrCoopReport result={99,99,99,99,99},before=result;CHECK(cr_cooperative(100,1,0,&result)==CR_BUSY&&!memcmp(&result,&before,sizeof result));CHECK(cr_cooperative(100,1000,1,&result)==CR_UNKNOWN&&!memcmp(&result,&before,sizeof result));CHECK(cr_cooperative(100,1000,2,&result)==CR_UNKNOWN&&!memcmp(&result,&before,sizeof result));puts("CR_COOP_PASS:drain-quiescence-and-explicit-unsupported-preemption-ISR");return 0;
}
