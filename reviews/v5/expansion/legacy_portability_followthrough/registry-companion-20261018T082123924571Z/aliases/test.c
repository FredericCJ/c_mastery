#include "registry.h"
#include <stdio.h>
#define C(x) do{if(!(x)){puts("LR_COMPANION_CHECK");return 1;}}while(0)
static int eq(const char*a,const char*b){while(*a && *a==*b){++a;++b;}return *a==*b;}
int main(void){int out=77;uint32_t id=88;const char *name="sentinel";
C(lr_at(0,&id,&name)==0 && id==1u);C(lr_dispatch(id,10,&out)==0 && out==11);C(lr_probes(id)<=6);
C(lr_id("start",&id)==0 && id==1u);
C(lr_name(id,&name)==0 && eq(name,"start"));
C(lr_at(1,&id,&name)==0 && id==4u);C(lr_dispatch(id,10,&out)==0 && out==14);C(lr_probes(id)<=6);
C(lr_id("stop",&id)==0 && id==4u);
C(lr_name(id,&name)==0 && eq(name,"stop"));
C(lr_at(2,&id,&name)==0 && id==7u);C(lr_dispatch(id,10,&out)==0 && out==17);C(lr_probes(id)<=6);
C(lr_id("status",&id)==0 && id==7u);
C(lr_id("query",&id)==0 && id==7u);
C(lr_id("state",&id)==0 && id==7u);
C(lr_name(id,&name)==0 && eq(name,"status"));
out=77;C(lr_dispatch(4294967295u,10,&out)==-1 && out==77);C(lr_count()==3);return 0;}
