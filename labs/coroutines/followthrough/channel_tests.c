#include "channel_follow.h"
#include <stdio.h>
#include <stdlib.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CC_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static void*allocate(void*v,size_t n){(void)v;return malloc(n);}
static void drop(void*v,void*p){(void)v;free(p);}
typedef struct{CATicket*t;MsgHub*h;int result;} Work;
static void*receive(void*v){Work*w=v;w->result=ca_channel_deliver(w->t,w->h,1);return NULL;}
int main(void){
 MsgHub hub;C(msg_hub_init(&hub,1,0)==MSG_OK);CAFrame*f=NULL;C(ca_create((COAllocator){NULL,allocate,drop},1,1,&f)==0);CATicket t;C(ca_reserve(f,&t)==0);
 C(ca_channel_deliver(&t,&hub,0)==CO_PENDING&&t.owner==f&&f->inflight==1);
 unsigned char source=0xA5;C(msg_send(&hub,0,source,false)==MSG_OK);source=0;C(msg_send(&hub,0,source,false)==MSG_FULL);
 Work w={&t,&hub,99};pthread_t thread;C(pthread_create(&thread,NULL,receive,&w)==0&&pthread_join(thread,NULL)==0&&w.result==0&&!t.owner);
 C(ca_ack(f)==0);unsigned char out[8]={91};size_t n=92;C(ca_poll(f,out,&n)==CO_DONE&&n==1&&out[0]==0xA5);C(ca_destroy(&f)==0);
 C(ca_create((COAllocator){NULL,allocate,drop},1,2,&f)==0&&ca_reserve(f,&t)==0);w=(Work){&t,&hub,99};C(pthread_create(&thread,NULL,receive,&w)==0);C(msg_close(&hub,0)==MSG_OK);C(pthread_join(thread,NULL)==0&&w.result==CO_STATE&&!t.owner);C(ca_ack(f)==0);n=92;C(ca_poll(f,out,&n)==CO_STATE&&n==92);C(ca_destroy(&f)==0);msg_hub_destroy(&hub);
 puts("CC_COPY:actual MsgHub copy survives source overwrite;full backpressure;threaded receive;closed-channel quiescence");return 0;
}
