#include "channel_follow.h"
#ifndef CC_MUTANT
#define CC_MUTANT 0
#endif
int ca_channel_deliver(struct ca_ticket*t,struct msg_hub*h,unsigned wait){
 if(!t||!t->owner||!h||wait>1)return CO_INVALID;unsigned which=99,value=0;enum msg_status r=msg_select(h,1,wait!=0,&which,&value);
 if(r==MSG_EMPTY){if(CC_MUTANT==2)return ca_notify(t,NULL,0);return CO_PENDING;}
 if(r==MSG_CLOSED||r==MSG_CANCELLED){(void)ca_cancel(t->owner);return ca_notify(t,NULL,0);}
 if(r!=MSG_OK)return CO_IO;
 if(which!=0||value>255){unsigned char invalid[9]={0};return ca_notify(t,invalid,sizeof invalid);}
 unsigned char byte=CC_MUTANT==1?0:(unsigned char)value;return ca_notify(t,&byte,1);
}
