#include "ownership.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("CR_CHECK:%u:%s\n",(unsigned)__LINE__,#x);return 1;}}while(0)
int main(void){
 CrPool p;CrToken a,b,f,sentinel={99,99,99};unsigned char input[16],out[16];size_t length=99;
 for(unsigned i=0;i<16;i++)input[i]=(unsigned char)i;
 CHECK(cr_pool_init(&p,1)==CR_OK&&cr_allocate(&p,0,input,3,&a)==CR_OK&&cr_allocate(&p,0,input,16,&b)==CR_OK);
 CHECK(cr_allocate(&p,0,input,1,&sentinel)==CR_FULL&&sentinel.world==99&&sentinel.slot==99);
 CHECK(cr_allocate(&p,1,input,4,&f)==CR_OK&&f.slot==2);memset(input,99,sizeof input);
 CHECK(cr_offer(&p,a)==CR_OK&&cr_release(&p,a)==CR_BUSY);CHECK(cr_claim(&p,a)==CR_OK&&cr_abort_offer(&p,a)==CR_BUSY);
 CHECK(cr_read(&p,a,out,2,&length)==CR_FULL&&length==99);CHECK(cr_read(&p,a,out,16,&length)==CR_OK&&length==3&&out[2]==2);CHECK(cr_release(&p,a)==CR_OK&&cr_release(&p,a)==CR_STALE);
 CHECK(cr_offer(&p,b)==CR_OK&&cr_abort_offer(&p,b)==CR_OK&&cr_release(&p,b)==CR_OK);
 CHECK(cr_close(&p)==CR_OK&&cr_drained(&p)==CR_BUSY&&cr_allocate(&p,0,input,1,&sentinel)==CR_BUSY);CHECK(cr_release(&p,f)==CR_OK&&cr_drained(&p)==CR_OK);
 CHECK(cr_pool_init(&p,2)==CR_OK);p.slots[0].generation=UINT64_MAX-1;CHECK(cr_allocate(&p,0,input,1,&a)==CR_OK&&a.generation==UINT64_MAX);CHECK(cr_close(&p)==CR_OK&&cr_release(&p,a)==CR_OK&&p.slots[0].state==CR_RETIRED&&cr_drained(&p)==CR_OK);
 CHECK(cr_pool_init(&p,3)==CR_OK&&cr_release(&p,a)==CR_STALE&&cr_drained(&p)==CR_BUSY);
 CrPool before=p;CHECK(cr_allocate(&p,0,input,17,&sentinel)==CR_INVALID&&!memcmp(&p,&before,sizeof p));CHECK(cr_allocate(&p,0,NULL,1,&sentinel)==CR_INVALID&&!memcmp(&p,&before,sizeof p));
 CHECK(cr_allocate(&p,0,NULL,0,&a)==CR_OK&&cr_offer(&p,a)==CR_OK&&cr_claim(&p,a)==CR_OK);memset(out,71,sizeof out);length=99;CHECK(cr_read(&p,a,out,0,&length)==CR_OK&&length==0&&out[0]==71);CHECK(cr_release(&p,a)==CR_OK);CrToken old=a;CHECK(cr_allocate(&p,0,input,1,&a)==CR_OK&&a.generation==old.generation+1&&cr_offer(&p,old)==CR_STALE);CHECK(cr_release(&p,a)==CR_OK);
 CHECK(cr_pool_init(&p,4)==CR_OK);p.slots[0].generation=UINT64_MAX-1;CHECK(cr_allocate(&p,0,input,1,&a)==CR_OK&&cr_release(&p,a)==CR_OK&&p.slots[0].state==CR_RETIRED);CHECK(cr_allocate(&p,0,input,1,&b)==CR_OK&&b.slot==1);CHECK(cr_close(&p)==CR_OK&&cr_release(&p,b)==CR_OK&&cr_drained(&p)==CR_OK);
 uint64_t parts[]={256,256,8*12,64,128},total=99;CHECK(cr_budget(parts,5,1,800,&total)==CR_OK&&total==800);CHECK(cr_budget(parts,5,1,799,&total)==CR_FULL&&total==800);CHECK(cr_budget(parts,5,0,1000,&total)==CR_UNKNOWN&&total==800);
 uint64_t huge[]={UINT64_MAX,1};CHECK(cr_budget(huge,2,1,UINT64_MAX,&total)==CR_RANGE&&total==800);
 printf("CR_LAYOUT:pool=%zu token=%zu payload=%zu fixture=800 task-FP-frame=204 integer-frame=68\n",sizeof(CrPool),sizeof(CrToken),sizeof(CrPayload));puts("CR_PASS:reserved-fault-ownership-and-budget");return 0;
}
