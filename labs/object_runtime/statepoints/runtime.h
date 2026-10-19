#ifndef OR_STATEPOINT_RUNTIME_H
#define OR_STATEPOINT_RUNTIME_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern const unsigned char __sp_maps[],__sp_maps_end[];
extern uint64_t sp_entry(unsigned char *,uint64_t,uint64_t);
extern unsigned char old_space[16],new_space[16];
extern unsigned polls,slots,mutant;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STATEPOINT_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
void relocate_roots(uintptr_t caller_sp,uintptr_t return_address);
#endif
