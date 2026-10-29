#include "recovery.h"
#include <stdio.h>
static void put(unsigned char*p,uint32_t x){for(unsigned k=0;k<4;k++)p[k]=(unsigned char)(x>>(8*k));}
static int image(uint32_t version,struct du_slot*out){unsigned char payload[16]={0};put(payload,0x20010000);put(payload+4,0x40009);payload[8]=42;struct du_image i={version,16,0x40000,0x40009,0x20010000,25000000,version};return du_encode(&i,payload,sizeof payload,out);}
static void hex(const unsigned char*p,unsigned n){for(unsigned i=0;i<n;i++)printf("%02x",p[i]);}
int main(void){
 struct du_slot old,fresh;if(image(1,&old)||image(3,&fresh))return 1;
 for(unsigned unit=0;unit<=16;unit++)for(unsigned bits=0;bits<=32;bits++){
  struct du_slot pair[2]={old,old};unsigned sectors[2]={4,5},pick=99;uint32_t mask=bits==32?UINT32_MAX:(UINT32_C(1)<<bits)-1;int result=du_install(pair,0,sectors,&fresh,unit,mask);int selected=du_select(pair,2,1,3,25000000,&pick);printf("I %u %u %d %d %u ",unit,bits,result,selected,pick);hex(pair[0].bytes,64);putchar(' ');hex(pair[1].bytes,64);putchar('\n');
 }
 /* Byte subsets in each next word, distinct from low-bit prefix tears. */
 for(unsigned unit=0;unit<16;unit++)for(unsigned subset=0;subset<16;subset++){
  struct du_slot pair[2]={old,old};unsigned sectors[2]={4,5},pick=99;uint32_t mask=0;for(unsigned b=0;b<4;b++)if(subset&(1u<<b))mask|=UINT32_C(255)<<(8*b);int result=du_install(pair,0,sectors,&fresh,unit,mask);int selected=du_select(pair,2,1,3,25000000,&pick);printf("B %u %u %d %d %u ",unit,subset,result,selected,pick);hex(pair[0].bytes,64);putchar(' ');hex(pair[1].bytes,64);putchar('\n');
 }
 for(unsigned cut=0;cut<=24;cut++)for(unsigned discarded=0;discarded<2;discarded++){
  struct du_retry retry;uint32_t value=99;if(du_retry_initialize(&retry))return 1;int first=du_retry_take(&retry,2,cut,&value);unsigned issued=first==DU_OK&&!discarded;for(unsigned f=0;f<5;f++)if(du_retry_take(&retry,2,24,&value)==DU_OK)issued++;uint32_t used=99;int state=du_retry_state(&retry,2,&used);printf("R %u %u %d %u %d %u ",cut,discarded,first,issued,state,used);hex(retry.records[0],24);putchar(' ');hex(retry.records[1],24);putchar('\n');
 }
 return 0;
}
