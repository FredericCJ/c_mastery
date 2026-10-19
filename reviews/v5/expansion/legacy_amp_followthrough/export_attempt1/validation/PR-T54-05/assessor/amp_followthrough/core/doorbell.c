#include "doorbell.h"
#ifndef AF_DOOR_MUTANT
#define AF_DOOR_MUTANT 0
#endif
static int valid(AfRegisters r,AfDoor d){return r.read&&r.write&&r.fence&&(d.core==7||d.core==4)&&d.outgoing<32&&d.incoming<32&&d.outgoing!=d.incoming;}
static uint32_t common(AfDoor d){return UINT32_C(0x58026500)+(d.core==4&&AF_DOOR_MUTANT!=2?16u:0u);}
int af_ring_door(AfRegisters r,AfDoor d){
 if(!valid(r,d))return AF_INVALID;
 uint32_t id=d.core==7?3u:1u;r.fence(r.context);
 uint32_t got=r.read(r.context,UINT32_C(0x58026480)+4*d.outgoing);
 if(got!=(UINT32_C(0x80000000)|(id<<8))&&AF_DOOR_MUTANT!=1)return AF_BUSY;
 r.write(r.context,UINT32_C(0x58026400)+4*d.outgoing,id<<8);r.fence(r.context);return AF_OK;
}
int af_mask_door(AfRegisters r,AfDoor d,unsigned enable){
 if(!valid(r,d)||enable>1)return AF_INVALID;
 uint32_t a=common(d),bit=UINT32_C(1)<<d.incoming,v=r.read(r.context,a);
 r.write(r.context,a,enable?v|bit:v&~bit);r.fence(r.context);return AF_OK;
}
int af_clear_door(AfRegisters r,AfDoor d){
 if(!valid(r,d))return AF_INVALID;
 r.write(r.context,common(d)+4,UINT32_C(1)<<d.incoming);r.fence(r.context);return AF_OK;
}
int af_service_door(AfRegisters r,AfDoor d,AfRing *q,uint32_t epoch,unsigned budget,unsigned *done,unsigned *runnable){
 if(!valid(r,d)||!q||!epoch||!budget||budget>AF_CAPACITY||!done||!runnable)return AF_INVALID;
 (void)af_mask_door(r,d,0);(void)af_clear_door(r,d);
 unsigned n=0;unsigned char bytes[AF_PAYLOAD];size_t len;uint32_t seq;int status=AF_OK;
 for(;n<budget;n++){status=af_receive(q,epoch,bytes,sizeof bytes,&seq,&len);if(status!=AF_OK)break;}
 (void)af_mask_door(r,d,1);
 /* Event before this read is visible in queue; event after it asserts IRQ.
  * Budget exhaustion must remain runnable without a second doorbell. */
 *done=n;*runnable=AF_DOOR_MUTANT==3?0u:af_queued(q)!=0;
 return status==AF_EMPTY?AF_OK:status;
}
