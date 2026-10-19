#define _POSIX_C_SOURCE 200809L
#include "numeric_transfer.h"
#include "numeric_extension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#define C(x) do{if(!(x)){fprintf(stderr,"NT_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static double now(void){struct timespec t;C(!clock_gettime(CLOCK_MONOTONIC,&t));return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){C(sizeof(double)==8&&DBL_MANT_DIG==53);int16_t a[4104],b[4104];for(size_t i=0;i<4104;i++){a[i]=i%3?INT16_MAX:INT16_MIN;b[i]=i%2?INT16_MIN:INT16_MAX;}for(unsigned offset=0;offset<8;offset++){int64_t exact=0;for(size_t n=0;n<=4096;n++){if(n)exact+=(int32_t)a[n-1+offset]*b[n-1+offset];int64_t scalar=91,vec=91;C(nt_dot(a+offset,b+offset,n,&scalar,0)&&scalar==exact);C(nt_dot(a+offset,b+offset,n,&vec,1)&&vec==exact);}}int64_t fail=91;C(!nt_dot(a,b,4097,&fail,1)&&fail==91);double v[257];for(unsigned i=0;i<257;i++)v[i]=i%4==0?1e16:i%4==1?1:i%4==2?-1e16:1;for(size_t n=0;n<=257;n++){double serial,p0,p1;C(nd_pairwise(v,n,&serial));C(nt_parallel(v,n,&p0,0)&&nt_parallel(v,n,&p1,1));C(!memcmp(&serial,&p0,8)&&!memcmp(&serial,&p1,8));}double out=91;v[0]=NAN;C(!nt_parallel(v,257,&out,0)&&out==91);v[0]=1e16;
 uint64_t ulp=91;C(nt_ulp(-0.0,0.0,&ulp)&&ulp==0);C(signbit(-0.0)&&!signbit(0.0));C(nt_ulp(0,DBL_TRUE_MIN,&ulp)&&ulp==1);C(nt_ulp(-DBL_TRUE_MIN,DBL_TRUE_MIN,&ulp)&&ulp==2);C(nt_ulp(1,nextafter(1,2),&ulp)&&ulp==1);C(!nt_ulp(INFINITY,1,&ulp));C(!nt_close(0,DBL_TRUE_MIN,0,0.5));C(nt_close(0,DBL_TRUE_MIN,DBL_TRUE_MIN,0));C(!nt_close(0,2,0.1,0.1));double a3[]={2,4,8},b3[]={4,8,16},r,s;C(nd_pairwise(a3,3,&r)&&nd_pairwise(b3,3,&s)&&s==2*r&&r==14);
 puts("schedule,sample,seconds,result");for(unsigned schedule=0;schedule<3;schedule++)for(unsigned sample=0;sample<9;sample++){double begin=now(),value=0;for(unsigned repeat=0;repeat<16;repeat++)C(schedule?nt_parallel(v,257,&value,schedule-1):nd_pairwise(v,257,&value));printf("%u,%u,%.9g,%a\n",schedule,sample,now()-begin,value);}puts("NT_PASS exactSSE2 fixedtree2workers ulp policies");return 0;}
