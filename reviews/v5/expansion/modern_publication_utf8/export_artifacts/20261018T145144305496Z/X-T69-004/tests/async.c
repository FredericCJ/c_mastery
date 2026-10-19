#define _POSIX_C_SOURCE 200809L
#include "dispatch_async.h"
#include "capture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static pthread_mutex_t pause_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pause_condition=PTHREAD_COND_INITIALIZER;
static unsigned entered,released;
static const char *case_name;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DS_CHECK:%s:%d:%s\n",case_name,__LINE__,#x);exit(1);}}while(0)
void ds_async_pause(void){
 (void)pthread_mutex_lock(&pause_mutex);entered=1;(void)pthread_cond_broadcast(&pause_condition);
 while(!released)(void)pthread_cond_wait(&pause_condition,&pause_mutex);
 (void)pthread_mutex_unlock(&pause_mutex);
}
static void wait_entered(void){
 (void)pthread_mutex_lock(&pause_mutex);while(!entered)(void)pthread_cond_wait(&pause_condition,&pause_mutex);(void)pthread_mutex_unlock(&pause_mutex);
}
static void release_pause(void){(void)pthread_mutex_lock(&pause_mutex);released=1;(void)pthread_cond_broadcast(&pause_condition);(void)pthread_mutex_unlock(&pause_mutex);}
static void wait_done(DSAsyncToken token){DSAsyncRegistry *r=token.registry;(void)pthread_mutex_lock(&r->mutex);while(!r->entries[token.slot].done)(void)pthread_cond_wait(&r->condition,&r->mutex);(void)pthread_mutex_unlock(&r->mutex);}
static unsigned gate_calls,gate_fail;
static int gate(void *context){unsigned *calls=context;++*calls;return *calls==gate_fail?DS_FAILED:DS_OK;}
static int registration(const char *path){
 DSAsyncRegistry registry;DSLoaded module={0};CHECK(ds_async_init(&registry)==DS_OK);CHECK(ds_loaded_open(&module,path)==DS_OK);
 DSAsyncToken token={0};CHECK(ds_async_register(&registry,&module,&token)==DS_OK && module.borrows==1);
 CHECK(ds_async_queue(token,17,3)==DS_OK && module.borrows==2);CHECK(ds_async_destroy(&registry)==DS_BUSY);
 int removed=ds_async_remove(token);unsigned cancel_borrows=module.borrows;
 wait_done(token);int out=91;int drained=ds_async_drain(token,&out);unsigned drained_borrows=module.borrows;int acknowledged=ds_async_remove(token);
 /* Observe the violation first, then join the canceled worker before rejecting
    a negative control. A semantic failure must not manufacture a thread leak. */
 CHECK(removed==DS_BUSY && cancel_borrows==2);
 CHECK(drained==DS_STALE && out==91 && drained_borrows==1 && registry.invocations==0);
 CHECK(acknowledged==DS_OK && module.borrows==0);CHECK(ds_async_start(token)==DS_STALE);CHECK(ds_async_remove(token)==DS_STALE);
 CHECK(ds_async_register(&registry,&module,&token)==DS_OK);CHECK(ds_async_queue(token,17,3)==DS_OK);CHECK(ds_async_start(token)==DS_OK);wait_done(token);CHECK(ds_async_drain(token,&out)==DS_OK && out==20);CHECK(ds_async_drain(token,&out)==DS_EMPTY);CHECK(ds_async_remove(token)==DS_OK);
 CHECK(ds_async_register(&registry,&module,&token)==DS_OK);CHECK(ds_async_queue(token,17,99)==DS_OK);CHECK(ds_async_start(token)==DS_OK);wait_entered();
 CHECK(ds_async_remove(token)==DS_BUSY && module.borrows==2);out=91;CHECK(ds_async_drain(token,&out)==DS_BUSY && out==91);CHECK(ds_loaded_close(&module)==DS_BUSY);
 release_pause();wait_done(token);CHECK(ds_async_drain(token,&out)==DS_STALE && out==91);CHECK(ds_async_remove(token)==DS_OK);
 for(unsigned i=3;i<12;++i){CHECK(ds_async_register(&registry,&module,&token)==DS_OK);CHECK(ds_async_remove(token)==DS_OK);}
 DSAsyncToken unchanged={&registry,99,99};CHECK(ds_async_register(&registry,&module,&unchanged)==DS_FULL && unchanged.slot==99);
 CHECK(registry.registrations==12 && registry.acknowledgements==12 && registry.invocations==2 && module.borrows==0);
 CHECK(ds_loaded_close(&module)==DS_OK);CHECK(ds_async_destroy(&registry)==DS_OK);return 0;
}
static int transition(const char *one,const char *two,const char *bad,unsigned n,unsigned failure,int print){
 DSAsyncRegistry registry;CHECK(ds_async_init(&registry)==DS_OK);OwAllocator allocator={0};DSReload h;ds_reload_init(&h,&registry,ds_counted_bytes(&allocator));
 int32_t values[8]={1,2,3,4,5,6,7,8};if(failure==2 && n)values[n-1]=600;
 CHECK(ds_reload_change(&h,one,n?values:NULL,n)==DS_OK && h.current==0 && h.generation==1 && registry.registrations==2);
 if(failure==1)allocator.fail_at=allocator.attempts+1;
 if(failure==3 || failure==4)registry.fail_at=registry.attempts+failure-2;
 const char *path=failure==5?"./missing-original-cmastery-plugin.so":failure==6?bad:two;
 int rc=ds_reload_change(&h,path,NULL,0);
 int expected=failure==5?DS_FAILED:failure==6?DS_INVALID:failure==2 && n?DS_RANGE:failure==1 && n>4?DS_FAILED:failure==3 || failure==4?DS_FAILED:DS_OK;
 CHECK(rc==expected);
 unsigned success=rc==DS_OK,spill=n>4;
 CHECK(h.generation==1+success && h.current==success && h.retired==(success?0:2));
 CHECK(allocator.live==spill*(1+success) && allocator.invalid_releases==0);
 CHECK(registry.registrations==2+(success?2:failure==4?1:0));
 CHECK(registry.acknowledgements==(success?2:failure==4?1:0));
 if(!success)CHECK(!h.modules[1].handle && h.modules[0].borrows==2 && h.states[1].length==0);
 int32_t first=91;int state_status=ds_reload_get(&h,0,&first);CHECK(state_status==(n?DS_OK:DS_RANGE) && first==(n?values[0]*(success?2:1):91));
 int32_t sentinel=91;CHECK(ds_reload_get(&h,n,&sentinel)==DS_RANGE && sentinel==91);
 if(print)printf("R %u %u %d %u %u %zu %u %u %d\n",n,failure,rc,h.generation,h.retired,allocator.live,registry.registrations,registry.acknowledgements,first);
 gate_calls=0;gate_fail=0;CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_OK);CHECK(ds_reload_stop(&h,gate,&gate_calls)==DS_OK);
 CHECK(allocator.live==0 && allocator.invalid_releases==0 && registry.registrations==registry.acknowledgements);CHECK(ds_async_destroy(&registry)==DS_OK);return 0;
}
static int retirement(const char *one,const char *two,const char *bad,int oracle){
 for(unsigned n=0;n<=8;++n)for(unsigned failure=0;failure<7;++failure)if(transition(one,two,bad,n,failure,oracle))return 1;
 if(oracle)return 0;
 DSAsyncRegistry registry;CHECK(ds_async_init(&registry)==DS_OK);OwAllocator allocator={0};DSReload h;ds_reload_init(&h,&registry,ds_counted_bytes(&allocator));int32_t values[8]={1,2,3,4,5,6,7,8};
 CHECK(ds_reload_change(&h,one,values,8)==DS_OK);DSAsyncToken running=h.registrations[0][0],queued=h.registrations[0][1];
 CHECK(ds_async_queue(running,17,99)==DS_OK);CHECK(ds_async_queue(queued,17,3)==DS_OK);CHECK(h.modules[0].borrows==4);CHECK(ds_async_start(running)==DS_OK);wait_entered();
 CHECK(ds_reload_change(&h,two,NULL,0)==DS_OK && h.current==1 && h.retired==0 && h.modules[0].borrows==4 && h.modules[1].borrows==2 && allocator.live==2 && allocator.peak==2);
 gate_calls=0;gate_fail=1;CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_BUSY && gate_calls==0);CHECK(ds_reload_change(&h,one,NULL,0)==DS_BUSY);
 wait_done(queued);int out=91;CHECK(ds_async_drain(queued,&out)==DS_STALE && out==91 && h.modules[0].borrows==3);
 CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_BUSY && gate_calls==0);release_pause();wait_done(running);CHECK(ds_async_drain(running,&out)==DS_STALE && out==91);
 CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_FAILED && gate_calls==1 && h.retired==0 && h.modules[0].handle && h.modules[0].borrows==0 && h.states[0].length==32 && allocator.live==2);
 CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_OK && gate_calls==2 && h.retired==2 && allocator.live==1 && !h.modules[0].handle);CHECK(ds_async_queue(running,17,3)==DS_STALE);
 int32_t value=0;for(size_t i=0;i<8;++i)CHECK(ds_reload_get(&h,i,&value)==DS_OK && value==2*values[i]);
 CHECK(ds_reload_change(&h,one,NULL,0)==DS_OK && h.generation==3);CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_OK);CHECK(ds_reload_change(&h,two,NULL,0)==DS_FULL && h.generation==3);
 CHECK(ds_reload_stop(&h,gate,&gate_calls)==DS_OK && allocator.live==0 && registry.registrations==registry.acknowledgements);CHECK(ds_async_destroy(&registry)==DS_OK);return 0;
}
static int measurements(const char *one,const char *two){
 const unsigned counts[4]={0,4,5,8};
 for(unsigned c=0;c<4;++c)for(unsigned sample=0;sample<21;++sample){
  unsigned n=counts[c];DSAsyncRegistry registry;CHECK(ds_async_init(&registry)==DS_OK);OwAllocator allocator={0};DSReload h;ds_reload_init(&h,&registry,ds_counted_bytes(&allocator));int32_t values[8]={1,2,3,4,5,6,7,8};
  CHECK(ds_reload_change(&h,one,n?values:NULL,n)==DS_OK);struct timespec begin,end;CHECK(clock_gettime(CLOCK_MONOTONIC,&begin)==0);
  CHECK(ds_reload_change(&h,two,NULL,0)==DS_OK);CHECK(clock_gettime(CLOCK_MONOTONIC,&end)==0);
  long long elapsed=(long long)(end.tv_sec-begin.tv_sec)*1000000000LL+end.tv_nsec-begin.tv_nsec;
  CHECK(h.retired==0 && h.current==1 && h.states[0].length==4*n && h.states[1].length==4*n);
  gate_calls=0;gate_fail=1;CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_FAILED);
  printf("M %u %u %lld %zu %zu %zu %zu\n",n,sample,elapsed,h.states[0].length+h.states[1].length,allocator.live,allocator.peak,allocator.attempts);
  CHECK(ds_reload_collect(&h,gate,&gate_calls)==DS_OK);CHECK(ds_reload_stop(&h,gate,&gate_calls)==DS_OK && !allocator.live);CHECK(ds_async_destroy(&registry)==DS_OK);
 }
 return 0;
}
int main(int argc,char **argv){if(argc!=5)return 2;case_name=argv[1];
 if(!strcmp(case_name,"async_registry"))return registration(argv[2]);
 if(!strcmp(case_name,"reload_owned"))return retirement(argv[2],argv[3],argv[4],0);
 if(!strcmp(case_name,"oracle"))return retirement(argv[2],argv[3],argv[4],1);
 if(!strcmp(case_name,"measure"))return measurements(argv[2],argv[3]);
 if(!strcmp(case_name,"layout")){printf("%zu %zu %zu %zu %zu\n",sizeof(DSReload),sizeof(DSAsyncRegistry),sizeof(DSAsyncEntry),sizeof(DSAsyncToken),sizeof(DSmall));return 0;}
 return 2;
}
