#include "expression_follow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CE_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct co_state{unsigned live,calls,fail,wait,delivered;int32_t observed;};
static void*alloc(void*v,size_t n){struct co_state*s=v;if(++s->calls==s->fail)return NULL;void*p=malloc(n);if(p)s->live++;return p;}
static void drop(void*v,void*p){((struct co_state*)v)->live--;free(p);}
static int delivery(void*v,int32_t result){struct co_state*s=v;s->observed=result;if(s->wait){s->wait--;return CO_PENDING;}s->delivered++;return CO_OK;}
static int run_value(struct ce_node*n,int32_t expected){
 for(unsigned budget=1;budget<=17;budget++){
  struct co_state s={0};struct ce_frame f;C(ce_init(&f,n,7,(struct co_allocator){&s,alloc,drop},NULL,NULL)==CO_OK);int32_t out=91;struct ce_frame before=f;C(ce_step(&f,0,&out)==CO_PENDING&&!memcmp(&f,&before,sizeof f)&&out==91);
  int r=CO_PENDING;for(unsigned i=0;i<100&&r==CO_PENDING;i++)r=ce_step(&f,budget,&out);
  C(r==CO_DONE&&out==expected&&!s.live&&f.max_depth==3);out=77;C(ce_step(&f,1,&out)==CO_DONE&&out==77);
 }return 0;
}
int main(void){
 struct ce_node n[]={{CE_ADD,1,2,0},{CE_MULTIPLY,3,4,0},{CE_ADD,5,6,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
 for(int a=-4;a<=4;a++)for(int b=-4;b<=4;b++)for(int c=-2;c<=2;c++){n[3].value=a;n[4].value=b;n[5].value=c;n[6].value=3;C(run_value(n,a*b+c+3)==0);}
 n[3].value=2;n[4].value=3;n[5].value=4;n[6].value=5;
 for(unsigned cutoff=0;cutoff<20;cutoff++){
  struct co_state s={0};struct ce_frame f;C(ce_init(&f,n,7,(struct co_allocator){&s,alloc,drop},NULL,NULL)==0);int32_t out=91;
  for(unsigned i=0;i<cutoff;i++)if(ce_step(&f,1,&out)==CO_DONE)break;
  C(ce_cancel(&f)==0&&!s.live);C(ce_cancel(&f)==0&&!s.live);
 }
 for(unsigned fail=1;fail<=3;fail++){struct co_state s={.fail=fail};struct ce_frame f;C(ce_init(&f,n,7,(struct co_allocator){&s,alloc,drop},NULL,NULL)==0);int32_t out=91;C(ce_step(&f,100,&out)==CO_RANGE&&!s.live&&out==91);C(ce_step(&f,1,&out)==CO_STATE);}
 n[3].value=INT32_MAX;n[4].value=2;struct co_state s={0};struct ce_frame f;C(ce_init(&f,n,7,(struct co_allocator){&s,alloc,drop},NULL,NULL)==0);int32_t out=91;C(ce_step(&f,100,&out)==CO_RANGE&&!s.live&&out==91);
 n[3].value=2;n[4].value=3;s=(struct co_state){.wait=3};C(ce_init(&f,n,7,(struct co_allocator){&s,alloc,drop},delivery,&s)==0);
 for(unsigned i=0;i<3;i++)C(ce_step(&f,100,&out)==CO_PENDING&&out==91&&!s.delivered&&!s.live);
 C(ce_step(&f,1,&out)==CO_DONE&&out==15&&s.delivered==1&&s.observed==15);
 puts("CE_CASES:405 expressions x17 budgets;20 cancellation cuts;3 allocation failures;overflow;external suspension");return 0;
}
