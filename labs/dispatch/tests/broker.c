#include "dispatch_broker.h"
#include <stdio.h>
#include <string.h>
static const char *test;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DS_CHECK:%s:%d:%s\n",test,__LINE__,#x);return 1;}}while(0)
int ds_broker_process_experiment(void);
static int checks(void) {
 struct ds_broker b={0};ds_broker_init(&b);CHECK(b.epoch[0]==1 && b.counters[1]==100);
 unsigned char wire[16]={0};const unsigned char encoded[16]={1,2,0,0,1,2,3,4,9,10,11,0,5,6,7,8};
 ds_broker_request(wire,DS_B_ADD,0,0,0x01020304u,0x090au,11,0x05060708u);CHECK(memcmp(wire,encoded,16)==0);
 const unsigned char maximum[4]={255,255,255,255};CHECK(ds_broker_u32(maximum)==UINT32_MAX);
 CHECK(ds_broker_process_experiment()==0);
 unsigned char p[16],reply[16];ds_broker_request(p,DS_B_READ,1,0,3,1,1,0);CHECK(ds_broker_apply(&b,1,p,reply)==DS_OK && reply[1]==DS_OK && ds_broker_u32(reply+12)==100);
 ds_broker_request(p,DS_B_DERIVE,0,1,1,1,1,0);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_OK && ds_broker_u32(reply+4)==4);
 ds_broker_request(p,DS_B_DERIVE,0,2,4,2,1,0);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_DENIED);
 ds_broker_request(p,DS_B_DERIVE,0,1,1,3,1,0);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_FULL);
 ds_broker_request(p,DS_B_ADD,0,0,1,4,1,1000);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_OK && b.counters[0]==1000);
 ds_broker_request(p,DS_B_ADD,0,0,1,5,1,1);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_RANGE && b.counters[0]==1000);
 for(unsigned epoch=2;epoch<=4;++epoch){CHECK(ds_broker_revoke(&b,0)==DS_OK && b.epoch[0]==epoch);ds_broker_request(p,DS_B_READ,0,0,1,b.next_sequence[0],1,0);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_STALE && ds_broker_u32(reply+12)==0);}
 CHECK(ds_broker_revoke(&b,0)==DS_FULL && b.epoch[0]==4);
 ds_broker_init(&b);b.next_sequence[0]=65535;ds_broker_request(p,DS_B_ADD,0,0,1,65535,1,1);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_OK && b.next_sequence[0]==65536);
 CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_STALE && b.counters[0]==1);ds_broker_request(p,DS_B_ADD,0,0,1,0,1,1);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_STALE && b.counters[0]==1);
 ds_broker_init(&b);for(unsigned byte=0;byte<16;++byte)reply[byte]=91;CHECK(ds_broker_apply(NULL,0,p,reply)==DS_INVALID);for(unsigned byte=0;byte<16;++byte)CHECK(reply[byte]==91);
 /* Reply may replace request after all fields have been decoded. */
 ds_broker_request(p,DS_B_ADD,0,0,1,1,1,7);CHECK(ds_broker_apply(&b,0,p,p)==DS_OK && p[1]==DS_OK && ds_broker_u32(p+12)==7);
 const unsigned char exact_reply[16]={1,0,0,3,0,0,0,1,0,1,1,0,0,0,0,7};CHECK(memcmp(p,exact_reply,16)==0);
 ds_broker_request(p,DS_B_READ,0,0,1,2,1,0);p[11]=1;CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK && reply[1]==DS_INVALID && b.next_sequence[0]==2);
 return 0;
}
static int oracle(void) {
 const uint32_t values[3]={0,7,1001};
 for(unsigned op=1;op<=4;++op)for(unsigned target=0;target<2;++target)for(unsigned id=0;id<5;++id)for(unsigned rights=0;rights<4;++rights)for(unsigned epoch=0;epoch<5;++epoch)for(unsigned v=0;v<3;++v){struct ds_broker b={0};unsigned char p[16],reply[16];ds_broker_init(&b);ds_broker_request(p,op,target,rights,id,1,epoch,values[v]);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK);printf("B %u %u %u %u %u %u %u %u %u %u %u %u\n",op,target,id,rights,epoch,values[v],reply[1],ds_broker_u32(reply+12),ds_broker_u32(reply+4),b.counters[0],b.counters[1],b.next_sequence[0]);}
 for(unsigned code=0;code<1024;++code){struct ds_broker b={0};ds_broker_init(&b);unsigned script=code;for(unsigned n=0;n<5;++n){unsigned action=script%4;script/=4;unsigned op=action==2?DS_B_DEPUTY:action==3?DS_B_DERIVE:DS_B_ADD;unsigned char p[16],reply[16];ds_broker_request(p,op,action==2?1:0,action==3?1:0,action==1?2:1,n+1,1,action==3?0:7);CHECK(ds_broker_apply(&b,0,p,reply)==DS_OK);printf("H %u %u %u %u %u %u\n",code,n,reply[1],b.counters[0],b.counters[1],ds_broker_u32(reply+4));}}
 return 0;
}
int main(int argc,char **argv){if(argc<2)return 2;test=argv[1];if(!strcmp(test,"layout")){printf("%zu %zu %u\n",sizeof(struct ds_broker),sizeof(struct ds_broker_grant),DS_BROKER_FRAME);return 0;}if(!strcmp(test,"broker"))return checks();if(!strcmp(test,"oracle"))return oracle();return 2;}
