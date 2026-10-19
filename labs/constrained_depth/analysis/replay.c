#include "analysis.h"
#include <inttypes.h>
#include <stdio.h>
int main(void){
 CAModel m={CA_PREMISES,CA_PERIODIC};
 for(unsigned c=1;c<=3;++c)for(unsigned t=2;t<=7;++t)for(unsigned d=0;d<=3;++d)for(unsigned u=3;u<=9;++u)for(unsigned b=0;b<=2;++b){
  if(t>u)continue;
  RTATask a[2]={{c,t,t,0,0,0},{d,u,u,0,b,0}};RTAResult r={0};enum rta_status s=RTA_INVALID;
  if(ca_analyze(&m,a,2,1,128,&r,&s)!=CA_OK)return 2;
  printf("A,%u,%u,%u,%u,%u,%u,%"PRIu64",%u\n",c,t,d,u,b,(unsigned)s,r.estimate,r.iterations);
 }
 /* Explicit integer workload, priority order 0 then1, releases before dispatch. */
 unsigned left[2]={0},release[2]={0},period[2]={4,5},cost[2]={1,2};
 for(unsigned now=0;now<20;++now){
  for(unsigned i=0;i<2;++i)if(now%period[i]==0){if(left[i])return 3;left[i]=cost[i];release[i]=now;printf("R,%u,%u\n",i,now);}
  unsigned selected=left[0]?0:left[1]?1:2;
  if(selected<2&&--left[selected]==0)printf("F,%u,%u,%u\n",selected,release[selected],now+1);
 }
 return 0;
}
