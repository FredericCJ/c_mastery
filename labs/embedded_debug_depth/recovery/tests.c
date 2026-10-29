#include "recovery.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){printf("DU_CHECK:%u:%s\n",__LINE__,#x);return 1;}}while(0)
static void put(unsigned char*p,uint32_t x){for(unsigned k=0;k<4;k++)p[k]=(unsigned char)(x>>(8*k));}
static int image(uint32_t version,struct du_slot*out){unsigned char payload[16]={0};put(payload,0x20010000);put(payload+4,0x40009);payload[8]=42;struct du_image i={version,16,0x40000,0x40009,0x20010000,25000000,version};return du_encode(&i,payload,sizeof payload,out);}
struct du_context {struct du_recovery*r;unsigned calls;int result;};
static int nested(void*v,uint32_t reason){struct du_context*c=v;c->calls++;if(reason!=7)return 0;(void)du_fault(c->r,8,11,nested,c);return 0;}
static int safe(void*v){struct du_context*c=v;c->calls++;return c->result;}
int main(void){
 CHECK(du_crc((const unsigned char*)"123456789",9)==0xcbf43926u);
 struct du_slot slots[3],next;CHECK(image(1,&slots[0])==DU_OK&&image(2,&slots[1])==DU_OK&&image(3,&slots[2])==DU_OK);slots[1].bytes[63]=255;unsigned selected=99;CHECK(du_select(slots,3,1,3,25000000,&selected)==DU_OK&&selected==2);CHECK(du_select(slots,3,1,2,25000000,&selected)==DU_OK&&selected==0);CHECK(du_select(slots,3,1,3,16000000,&selected)==DU_ABSENT&&selected==0);
 CHECK(image(3,&next)==DU_OK);struct du_image decoded;CHECK(du_decode(&next,&decoded)==DU_OK);struct du_machine machine={25000000,1,0,0,0};struct du_plan plan={9,9,9};CHECK(du_prepare(&decoded,&machine,&plan)==DU_OK&&plan.vector==0x40000);machine.clock=16000000;CHECK(du_prepare(&decoded,&machine,&plan)==DU_INVALID&&plan.vector==0x40000);machine.clock=25000000;machine.dma_owned=1;CHECK(du_prepare(&decoded,&machine,&plan)==DU_INVALID);
 unsigned sectors[2]={4,5};struct du_slot pair[2],before;CHECK(image(1,&pair[0])==DU_OK);before=pair[0];memset(&pair[1],255,sizeof pair[1]);CHECK(du_install(pair,0,sectors,&next,15,UINT32_MAX)==DU_INTERRUPTED);CHECK(du_select(pair,2,1,3,25000000,&selected)==DU_OK&&selected==1&&!memcmp(&pair[0],&before,sizeof before));sectors[1]=4;struct du_slot saved[2];memcpy(saved,pair,sizeof saved);CHECK(du_install(pair,0,sectors,&next,0,0)==DU_INVALID&&!memcmp(pair,saved,sizeof pair));
 /* Harmless checksum-only authorization violation; synthetic payload42->99. */
 next.bytes[36]=99;put(next.bytes+56,du_crc(next.bytes,56));CHECK(du_decode(&next,&decoded)==DU_OK);
 struct du_retry retry;CHECK(du_retry_initialize(&retry)==DU_OK);uint32_t attempt=77;
 CHECK(du_retry_take(&retry,2,23,&attempt)==DU_INTERRUPTED&&attempt==77);CHECK(du_retry_state(&retry,2,&attempt)==DU_OK&&attempt==0);unsigned retries=0;for(unsigned f=0;f<5;f++)if(du_retry_take(&retry,2,24,&attempt)==DU_OK)retries++;CHECK(retries==2&&du_retry_state(&retry,2,&attempt)==DU_STOP&&attempt==2);memset(&retry,0,sizeof retry);attempt=77;CHECK(du_retry_state(&retry,2,&attempt)==DU_STOP&&attempt==77);
 struct du_recovery recovery={0};CHECK(du_effect(&recovery,1)==DU_OK);struct du_context c={&recovery,0,0};CHECK(du_fault(&recovery,7,10,nested,&c)==DU_OK&&c.calls==1&&recovery.first==7&&recovery.secondary==8&&recovery.record.nested&&recovery.record.words[0]==7);CHECK(du_effect(&recovery,1)==DU_STOP);CHECK(du_contain(&recovery,12,safe,&c)==DU_STOP&&recovery.output==1&&!recovery.contained);c.result=1;CHECK(du_contain(&recovery,13,safe,&c)==DU_OK&&recovery.output==0&&recovery.emitted==1&&recovery.containment_step-recovery.detection_step==3);unsigned calls=c.calls;CHECK(du_contain(&recovery,14,safe,&c)==DU_OK&&c.calls==calls);
 struct du_recovery input={0};CHECK(du_recovery_input(&input,NULL,0,SIZE_MAX,0,1)==DU_INVALID&&input.first==11);input=(struct du_recovery){0};CHECK(du_recovery_input(&input,NULL,0,0,1,1)==DU_STOP&&input.first==12);input=(struct du_recovery){0};CHECK(du_recovery_input(&input,NULL,0,0,2,1)==DU_STOP&&input.first==13);input=(struct du_recovery){0};unsigned char bytes[3]={9,42,43};CHECK(du_recovery_input(&input,bytes,3,3,0,1)==DU_INVALID&&input.first==14);
 puts("DU_PASS:core");return 0;
}
