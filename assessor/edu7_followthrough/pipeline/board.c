#include "pipeline.h"
/* Selected reset/quiescent board resources only; clocks and elapsed time are
   explicit caller proofs. The temporary probe has never submitted a byte.
   Its immutable callbacks have no borrowed probe context; transfer ownership
   to ew_init, which reconfigures the still-quiescent selected UART. */
int ew_f439_open(struct ew_node *s,unsigned node,unsigned profile,uint16_t incarnation,const struct pu_clock *clock,uint32_t now){
 if(!s||!clock||s->node||(node!=17&&node!=34&&node!=51))return EW_INVALID;
 struct ep_tx probe[2]={0};struct r5_io io[2];unsigned ports=node==34?2:1;
 for(unsigned i=0;i<ports;i++){if(ep_f439_open_port(&probe[i],clock,115200,R5_THVD1400,i,now))return EW_INVALID;io[i]=probe[i].io;}
 return ew_init(s,node,profile,incarnation,io,clock,now);
}
