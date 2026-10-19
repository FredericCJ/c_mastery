#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#ifndef PF_RECORD_CAP
#define PF_RECORD_CAP 8192
#endif
typedef struct {void *fn,*caller;uint64_t ns;unsigned enter;} Event;
static Event events[PF_RECORD_CAP];static unsigned count,dropped,active;
#define EX __attribute__((no_instrument_function))
static EX void record(void*f,void*c,unsigned e){if(!active)return;if(count==PF_RECORD_CAP){++dropped;return;}struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t)){++dropped;return;}events[count++]=(Event){f,c,(uint64_t)t.tv_sec*1000000000u+(unsigned)t.tv_nsec,e};}
EX void __cyg_profile_func_enter(void*f,void*c){record(f,c,1);}
EX void __cyg_profile_func_exit(void*f,void*c){record(f,c,0);}
EX void pf_record_begin(void){count=dropped=0;active=1;}
EX void pf_record_end(void){active=0;printf("HOOK_BEGIN:%u:%u\n",count,dropped);for(unsigned i=0;i<count;++i)printf("H:%u:%p:%p:%llu\n",events[i].enter,events[i].fn,events[i].caller,(unsigned long long)events[i].ns);puts("HOOK_END");}
