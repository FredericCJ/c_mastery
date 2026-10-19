#include "memory.h"
#include "checkpoint_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPX_CHECK: %s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
typedef struct { unsigned attempts,fail,live,releases,subscribes,unsubscribes,async;void *blocks[8]; } Backend;
static void *allocate(void *context,size_t n){
 Backend *b=context;if(++b->attempts==b->fail)return NULL;
 void *p=malloc(n);CHECK(p!=NULL);
 for(unsigned i=0;i<8;++i)if(!b->blocks[i]){b->blocks[i]=p;++b->live;return p;}
 CHECK(0);return NULL;
}
static void release(void *context,void *p){
 Backend *b=context;for(unsigned i=0;i<8;++i)if(b->blocks[i]==p){b->blocks[i]=NULL;--b->live;++b->releases;free(p);return;}
 CHECK(0);
}
static int subscribe(void *context,uint64_t *cookie){Backend *b=context;++b->subscribes;if(++b->attempts==b->fail)return 0;*cookie=7;return 1;}
static int unsubscribe(void *context,uint64_t cookie){Backend *b=context;CHECK(cookie==7);++b->unsubscribes;return !b->async;}
static CPXBackend api(Backend *b){return (CPXBackend){b,allocate,release,subscribe,unsubscribe};}
static unsigned live(const CPXStore *s){unsigned n=0;for(unsigned i=0;i<s->capacity;++i)n+=s->state[i]==1;return n;}
static CPXRecord record(int value,size_t length){CPXRecord r={0};CHECK(length<=sizeof r.bytes);r.value=value;r.length=length;for(size_t i=0;i<sizeof r.bytes;++i)if(i<length)r.bytes[i]=(unsigned char)(value+(int)i);return r;}
static void construction(void){
 for(unsigned fail=1;fail<=3;++fail){Backend b={0};b.fail=fail;CPXStore s={0};CHECK(cpx_init(&s,api(&b),4,4)!=(CPXStatus)CPX_OK);CHECK(b.live==0 && b.releases==(fail==1?0:fail==2?1:2));CHECK(cpx_close(&s)==CPX_OK && cpx_close(&s)==CPX_OK);}
 Backend b={0};b.async=1;CPXStore s={0};CHECK(cpx_init(&s,api(&b),4,4)==CPX_OK);CHECK(b.live==2);
 CHECK(cpx_close(&s)==CPX_PENDING && b.live==2 && b.unsubscribes==1);
 CHECK(cpx_close(&s)==CPX_PENDING && b.unsubscribes==1);
 CHECK(cpx_acknowledge(&s,6,1)==CPX_INVALID && b.live==2);
 CHECK(cpx_acknowledge(&s,7,0)==CPX_PENDING && b.live==2);
 CHECK(cpx_acknowledge(&s,7,1)==CPX_OK && b.live==0 && b.releases==2);
 CHECK(cpx_acknowledge(&s,7,1)==CPX_INVALID && cpx_close(&s)==CPX_OK);
}
static void linked_and_half(void){
 CPXRecord records[4]={record(11,1),record(22,4),record(33,8),record(44,16)};
 for(unsigned capacity=2;capacity<=4;capacity+=2){
  Backend b={0};CPXStore s={0};CHECK(cpx_init(&s,api(&b),capacity,capacity)==CPX_OK);
  size_t accepted=99;CPXStatus status=cpx_batch(&s,records,4,&accepted);
  CHECK(accepted==capacity && status==(capacity==4?CPX_OK:CPX_FULL) && s.count==capacity);
  for(unsigned i=0;i<capacity;++i){CPXHandle h;const CPXRecord *p;CHECK(cpx_pop(&s,&h)==CPX_OK && cpx_lookup(&s,h,&p)==CPX_OK);CHECK(p->value==records[i].value && p->length==records[i].length && !memcmp(p->bytes,records[i].bytes,p->length));CHECK(cpx_release(&s,h)==CPX_OK);}
  CHECK(cpx_close(&s)==CPX_OK && b.live==0);
 }
 for(unsigned failure=1;failure<=4;++failure){
  Backend b={0};CPXStore s={0};CHECK(cpx_init(&s,api(&b),4,4)==CPX_OK);s.node_fail_at=failure;
  size_t accepted;CHECK(cpx_batch(&s,records,4,&accepted)==CPX_FULL);CHECK(accepted==failure-1 && live(&s)==accepted && s.count==accepted);
  while(s.count){CPXHandle h;CHECK(cpx_pop(&s,&h)==CPX_OK && cpx_release(&s,h)==CPX_OK);}
  CHECK(cpx_close(&s)==CPX_OK && b.live==0);
 }
 Backend b={0};CPXStore s={0};CHECK(cpx_init(&s,api(&b),4,2)==CPX_OK);size_t accepted;
 CHECK(cpx_batch(&s,records,4,&accepted)==CPX_FULL && accepted==2 && live(&s)==2);
 while(s.count){CPXHandle h;CHECK(cpx_pop(&s,&h)==CPX_OK && cpx_release(&s,h)==CPX_OK);}CHECK(cpx_close(&s)==CPX_OK);
}
static void relocation(void){
 Backend b={0};CPXStore s={0};CPXHandle h;CPXRecord r=record(77,16);const CPXRecord *p;
 CHECK(cpx_init(&s,api(&b),4,4)==CPX_OK && cpx_allocate(&s,&r,&h)==CPX_OK && cpx_pin(&s,h)==CPX_OK);
 CHECK(cpx_relocate(&s)==CPX_BUSY && cpx_close(&s)==CPX_BUSY && cpx_release(&s,h)==CPX_BUSY);
 CHECK(cpx_unpin(&s,h)==CPX_OK);b.fail=b.attempts+1;
 CPXRecord *original=s.records;CHECK(cpx_relocate(&s)==CPX_FULL && s.records==original && b.live==2);
 b.fail=0;CHECK(cpx_relocate(&s)==CPX_OK && b.live==2 && cpx_lookup(&s,h,&p)==CPX_OK && p->value==77 && p->bytes[15]==92);
 /* Do not compare/dereference the expired pointer after successful relocation. */
 CHECK(cpx_release(&s,h)==CPX_OK && cpx_lookup(&s,h,&p)==CPX_INVALID);
 for(unsigned generation=2;generation<=3;++generation){CHECK(cpx_allocate(&s,&r,&h)==CPX_OK && h.slot==0 && h.generation==generation);CHECK(cpx_release(&s,h)==CPX_OK);}
 CHECK(s.state[0]==2 && cpx_allocate(&s,&r,&h)==CPX_OK && h.slot==1);CHECK(cpx_release(&s,h)==CPX_OK);
 CHECK(cpx_close(&s)==CPX_OK && b.live==0);
}
static void distribution(void){
 for(size_t length=1;length<=16;length+=15){
  unsigned char bytes[64];CPMArena arena;CHECK(cpm_arena_init(&arena,bytes,sizeof bytes,1,1)==CPM_OK);
  Backend b={0};CPXStore store={0};CHECK(cpx_init(&store,api(&b),4,4)==CPX_OK);
  unsigned accepted=0;CPXHandle handles[4];CPMSpan span;
  for(unsigned i=0;i<4;++i){CPXRecord r=record((int)i,length);CHECK(cpx_allocate(&store,&r,&handles[i])==CPX_OK);if(cpm_reserve(&arena,length,&span)==CPM_OK)++accepted;}
  CHECK(accepted==(length==1?4u:3u));size_t used=arena.used;
  for(unsigned i=0;i<4;++i)CHECK(cpx_release(&store,handles[i])==CPX_OK);
  CHECK(live(&store)==0 && arena.used==used);
  printf("DISTRIBUTION length=%zu arena_capacity=64 arena_accepted=%u arena_payload=%zu arena_header_bytes=%u arena_retained=%zu pool_accepted=4 pool_live_after_release=0 pool_payload_capacity=64 pool_record_storage=%zu node_storage=%zu\n",length,accepted,length*accepted,3*accepted,used,4*sizeof(CPXRecord),4*sizeof(CPXNode));
  CHECK(cpm_reset(&arena)==CPM_OK && arena.used==0 && cpx_close(&store)==CPX_OK);
 }
}
static void lease(void){
 unsigned char bytes[32];CPMArena arena;CPMSpan span;CPXLease callback={0};
 CHECK(cpm_arena_init(&arena,bytes,sizeof bytes,1,1)==CPM_OK && cpm_reserve(&arena,9,&span)==CPM_OK);
 CHECK(cpm_pin(&arena,span)==CPM_OK && cpx_lease_begin(&callback,&arena,span,71)==CPX_OK && arena.pins==2);
 CHECK(cpm_unpin(&arena)==CPM_OK && cpx_lease_cancel(&callback)==CPX_PENDING && arena.pins==1 && cpm_reset(&arena)==CPM_BUSY);
 CHECK(cpx_lease_complete(&callback,70,1)==CPX_INVALID && cpx_lease_complete(&callback,71,0)==CPX_PENDING && arena.pins==1);
 CHECK(cpx_lease_complete(&callback,71,1)==CPX_OK && cpm_reset(&arena)==CPM_OK && cpx_lease_complete(&callback,71,1)==CPX_INVALID);
}
static void growth(void){
 for(unsigned failure=1;failure<=2;++failure){Backend b={0};CPXStore s={0};CHECK(cpx_init(&s,api(&b),2,2)==CPX_OK);CPXRecord r=record(12,4);CPXHandle h;const CPXRecord *p;CHECK(cpx_allocate(&s,&r,&h)==CPX_OK);
  b.fail=b.attempts+failure;CHECK(cpx_grow(&s,4)==CPX_FULL && s.capacity==2 && b.live==2 && cpx_lookup(&s,h,&p)==CPX_OK && p->value==12);
  b.fail=0;CHECK(cpx_pin(&s,h)==CPX_OK && cpx_grow(&s,4)==CPX_BUSY && cpx_unpin(&s,h)==CPX_OK);
  CHECK(cpx_grow(&s,4)==CPX_OK && s.capacity==4 && b.live==2 && cpx_lookup(&s,h,&p)==CPX_OK && p->value==12);
  CHECK(cpx_release(&s,h)==CPX_OK && cpx_close(&s)==CPX_OK && b.live==0);
 }
}
int main(void){construction();linked_and_half();relocation();distribution();lease();growth();printf("CPX_PASS allocated records, linked-node exhaustion, capacity change, relocation, asynchronous teardown\n");return 0;}
