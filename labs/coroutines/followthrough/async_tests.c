#include "async_follow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CA_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
typedef struct{unsigned live;} Alloc;
static void*allocate(void*v,size_t n){void*p=malloc(n);if(p)((Alloc*)v)->live++;return p;}
static void drop(void*v,void*p){((Alloc*)v)->live--;free(p);}
typedef struct{CATicket ticket;int result;} Work;
static void*deliver(void*v){Work*w=v;const unsigned char b='q';w->result=ca_notify(&w->ticket,&b,1);return NULL;}
static int edges(void){
 Alloc a={0};COAllocator port={&a,allocate,drop};CAFrame*f=NULL;
 C(ca_create(port,0,1,&f)==CO_RANGE&&!f&&!a.live);
 C(ca_create(port,9,1,&f)==CO_RANGE&&!f&&!a.live);
 C(ca_create(port,1,0,&f)==CO_INVALID&&!f&&!a.live);
 C(ca_create(port,1,1,&f)==0);CATicket t[4],extra={0};
 C(ca_ack(f)==CO_STATE);for(unsigned i=0;i<4;i++)C(ca_reserve(f,t+i)==0);
 C(ca_reserve(f,&extra)==CO_RANGE&&!extra.owner&&f->inflight==4);
 CATicket wrong=t[0];wrong.generation++;
 C(ca_notify(&wrong,(const unsigned char*)"x",1)==CO_STATE&&f->inflight==4);
 C(ca_notify(&t[0],NULL,0)==0&&f->length==0&&f->inflight==3);
 C(ca_notify(&t[1],(const unsigned char*)"xx",2)==CO_RANGE&&f->closing);
 C(ca_reserve(f,&extra)==CO_STATE&&!extra.owner&&ca_ack(f)==CO_PENDING);
 C(ca_notify(&t[2],NULL,0)==CO_STATE&&ca_notify(&t[3],NULL,0)==CO_STATE);
 C(ca_ack(f)==0);unsigned char out[8]={91};size_t n=92;
 C(ca_poll(f,out,&n)==CO_IO&&out[0]==91&&n==92);C(ca_destroy(&f)==0&&!a.live);
 C(ca_create(port,1,2,&f)==0);f->next=UINT32_MAX-1;
 C(ca_reserve(f,&t[0])==0&&t[0].serial==UINT32_MAX-1);
 C(ca_reserve(f,&extra)==CO_RANGE&&!extra.owner);C(ca_cancel(f)==0);
 C(ca_notify(&t[0],NULL,0)==CO_STATE&&ca_ack(f)==0&&ca_destroy(&f)==0&&!a.live);
 return 0;
}
int main(void){
 C(edges()==0);
 const unsigned orders[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
 for(unsigned p=0;p<6;p++){
  Alloc a={0};CAFrame*f=NULL;C(ca_create((COAllocator){&a,allocate,drop},2,7,&f)==0);CATicket t[2],copy;C(ca_reserve(f,&t[0])==0&&ca_reserve(f,&t[1])==0);copy=t[0];
  C(ca_destroy(&f)==CO_STATE&&f&&a.live==1);unsigned expected=0,received=0;unsigned char out[8]={91};size_t n=92;
  for(unsigned i=0;i<3;i++){
   unsigned op=orders[p][i];if(op==2){int r=ca_cancel(f);if(!expected){expected=1;C(r==0);}else C(r==CO_STATE);}
   else{unsigned char b=(unsigned char)('a'+op);int r=ca_notify(&t[op],&b,1);if(!expected){C(r==0);if(++received==2)expected=2;}else C(r==CO_STATE);}
   C(ca_poll(f,out,&n)==CO_PENDING&&out[0]==91&&n==92);
   if(f->closing&&f->inflight)C(ca_ack(f)==CO_PENDING);
  }
  C(ca_notify(&copy,(const unsigned char*)"x",1)==CO_STATE);C(ca_ack(f)==0);int r=ca_poll(f,out,&n);C(r==(expected==2?CO_DONE:CO_STATE));if(expected==2)C(n==2&&out[0]!=91);else C(n==92&&out[0]==91);
  n=77;C(ca_poll(f,out,&n)==r&&n==77);C(ca_destroy(&f)==0&&!f&&!a.live);
 }
 for(unsigned i=0;i<200;i++){
  Alloc a={0};CAFrame*f=NULL;C(ca_create((COAllocator){&a,allocate,drop},2,i+1,&f)==0);Work work[2];C(ca_reserve(f,&work[0].ticket)==0&&ca_reserve(f,&work[1].ticket)==0);pthread_t threads[2];
  C(pthread_create(&threads[0],NULL,deliver,&work[0])==0&&pthread_create(&threads[1],NULL,deliver,&work[1])==0);int cancelled=ca_cancel(f);C(cancelled==0||cancelled==CO_STATE);
  C(pthread_join(threads[0],NULL)==0&&pthread_join(threads[1],NULL)==0);C(ca_ack(f)==0);unsigned char out[8]={91};size_t n=92;
  C(ca_poll(f,out,&n)==(cancelled==0?CO_STATE:CO_DONE));if(cancelled==0)C(out[0]==91&&n==92);else C(n==2&&out[0]=='q'&&out[1]=='q');C(ca_destroy(&f)==0&&!a.live);
 }
 puts("CA_ORDERS:6 serialized terminal orders;200 actual two-notifier cancellation schedules");return 0;
}
