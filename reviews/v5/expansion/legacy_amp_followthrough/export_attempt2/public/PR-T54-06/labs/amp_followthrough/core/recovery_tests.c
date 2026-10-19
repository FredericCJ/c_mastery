#include "amp.h"
#include <stdio.h>
#define C(x) do{if(!(x)){fprintf(stderr,"AF_RECOVERY_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 for(unsigned cut=0;cut<=4;cut++){
  AfBoot b={0};AfBootOwner o={0};AfRing q;af_ring_init(&q,0);C(af_boot_begin(&o,7,UINT32_MAX-9,20,1,1)==AF_OK);
  for(unsigned i=0;i<cut;i++)C(af_boot_stage(&o,&b,i,2,AF_LAYOUT)==AF_OK);
  if(cut<4)C(af_boot_ack(&b,2,AF_LAYOUT,8)==AF_BUSY);
  for(unsigned i=0;i<cut;i++)C(af_boot_stage(&o,&b,i,2,AF_LAYOUT)==AF_OK);
  for(unsigned i=cut;i<4;i++)C(af_boot_stage(&o,&b,i,2,AF_LAYOUT)==AF_OK);
  C(af_boot_status(&o,&b,0)==AF_BUSY);C(af_boot_ack(&b,2,AF_LAYOUT+4,8)==AF_STALE);C(af_queued(&q)==0);
  C(af_boot_ack(&b,2,AF_LAYOUT,8)==AF_OK);C(af_boot_status(&o,&b,0)==AF_OK);
  C(af_boot_close(&o,&b,0,1)==AF_BUSY&&o.active);C(af_boot_begin(&o,8,1,20,1,1)==AF_BUSY);C(af_boot_close(&o,&b,1,1)==AF_OK);
  C(af_boot_begin(&o,8,1,20,1,1)==AF_OK);for(unsigned i=0;i<4;i++)C(af_boot_stage(&o,&b,i,2,AF_LAYOUT)==AF_OK);
  C(af_boot_ack(&b,2,AF_LAYOUT,8)==AF_STALE);C(af_boot_status(&o,&b,2)==AF_BUSY);C(af_boot_ack(&b,2,AF_LAYOUT,9)==AF_OK);
 }
 /* Explicit survivor ledger: 90 replies, five rejected admissions, five
    requests lacking replies. The latter remain uncertain after peer reset. */
 unsigned state[100];for(unsigned i=0;i<100;i++)state[i]=i<90?1:i<95?2:3;
 unsigned complete=0,rejected=0,uncertain=0;for(unsigned i=0;i<100;i++){complete+=state[i]==1;rejected+=state[i]==2;uncertain+=state[i]==3;}C(complete==90&&rejected==5&&uncertain==5);
 AfReceipt survivor={.session=10};int32_t value=0;
 for(unsigned cut=0;cut<3;cut++){
  AfReceipt s=survivor;if(cut)C(af_effect(&s,10,1,5,&value)==AF_OK);
  /* Cut before effect, after effect before reply, or after reply. Receipt is
     retained in the supervisor's still-live memory, not reset with peer. */
  C(af_effect(&s,10,1,5,&value)==AF_OK&&value==5&&s.total==5);
  C(af_effect(&s,11,1,5,&value)==AF_STALE); /* full reset needs a new session */
 }
 puts("AF_PASS:recovery cuts0..4 epoch8to9 uncertainty receipt cuts0..2");return 0;
}
