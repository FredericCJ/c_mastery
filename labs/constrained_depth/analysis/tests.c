#include "analysis.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("CA_CHECK:%u:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 struct rta_task t[2]={{1,4,4,0,0,0},{2,5,5,0,0,0}};struct ca_model m={CA_PREMISES,CA_PERIODIC};struct rta_result r={0};enum rta_status status=RTA_INVALID;
 CHECK(ca_analyze(&m,t,2,0,128,&r,&status)==CA_OK&&status==RTA_CONVERGED&&r.estimate==1);
 CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_OK&&status==RTA_CONVERGED&&r.estimate==3);
 t[1].blocking=1;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_OK&&r.estimate==4);t[1].blocking=0;
 CHECK(ca_analyze(&m,t,2,1,1,&r,&status)==CA_OK&&status==RTA_LIMIT);
 m.arrivals=CA_SPORADIC_MINIMUM;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_OK&&r.estimate==3);
 for(unsigned bit=1;bit<=128;bit*=2){m.premises=CA_PREMISES^bit;struct rta_result before=r;enum rta_status old=status;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_UNSUPPORTED&&memcmp(&r,&before,sizeof r)==0&&status==old);}
 m.premises=CA_PREMISES;t[0].jitter=1;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_UNSUPPORTED);t[0].jitter=0;t[1].deadline=6;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_UNSUPPORTED);t[1].deadline=5;
 struct rta_result before=r;t[0].period=0;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_INVALID&&memcmp(&r,&before,sizeof r)==0);t[0].period=4;
 t[1].cost=UINT64_MAX;t[1].blocking=1;CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_INVALID&&memcmp(&r,&before,sizeof r)==0);
 t[1]=(struct rta_task){2,5,2,0,0,0};CHECK(ca_analyze(&m,t,2,1,128,&r,&status)==CA_OK&&status==RTA_EXCEEDED);
 struct ca_config c={{1},25000000,1,0,0,40},changed=c;uint64_t n=99,d=98;
 struct ca_evidence e={CA_OBSERVED_MAX,9,1,c};CHECK(ca_evidence(&c,&e,CA_OBSERVED_MAX,&n,&d)==CA_OK&&n==9&&d==1);
 n=99;d=98;CHECK(ca_evidence(&c,&e,CA_MODEL_BOUND,&n,&d)==CA_INVALID&&n==99&&d==98);
 for(unsigned k=0;k<6;++k){changed=c;if(k==0)++changed.image_sha256[31];if(k==1)++changed.clock_hz;if(k==2)++changed.interrupt_policy;if(k==3)++changed.cache_policy;if(k==4)++changed.probe_policy;if(k==5)++changed.resolution_ns;CHECK(ca_compatible(&c,&changed)==CA_DIFFERENT);}
 e.denominator=0;CHECK(ca_evidence(&c,&e,CA_OBSERVED_MAX,&n,&d)==CA_INVALID&&n==99&&d==98);
 puts("CA_PASS:periodic=1,3 blocking=4 U=13/20 sporadic=3 unresolved=8 kinds=4 changed-config=6");return 0;
}
