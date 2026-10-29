#define _POSIX_C_SOURCE 200809L
#include "dispatch_modules.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>
struct ds_counter {unsigned calls;int value;};
static int read_counter(void *p,int *out){struct ds_counter *c=p;++c->calls;*out=c->value;return DS_OK;}
static uint64_t nanos(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
int main(void){
 printf("L %zu %zu %zu %zu %zu %zu\n",sizeof(struct ds_module_entry),sizeof(struct ds_module_registry),sizeof(struct ds_module_lease),sizeof(struct ow_scope),sizeof(struct ds_user_map),sizeof(struct ds_metric_map));
 struct ds_counter c={0,7};struct ds_module *module=NULL;if(ds_module_open((struct ds_service){read_counter,&c},&module)!=DS_OK)return 1;
 for(unsigned trial=0;trial<8;++trial){uint64_t begin=nanos();long checksum=0;unsigned before=c.calls;
  for(unsigned n=0;n<200000;++n){int out=91;if(ds_module_twice(module,&out)!=DS_OK)return 1;checksum+=out;}
  uint64_t elapsed=nanos()-begin;if(checksum!=2800000 || c.calls-before!=400000)return 1;
  if(trial)printf("T %u %llu %ld %u\n",trial,(unsigned long long)elapsed,checksum,c.calls-before);
 }
 ds_module_close(module);return 0;
}
