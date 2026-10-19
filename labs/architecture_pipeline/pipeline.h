#ifndef ARCHITECTURE_PIPELINE_H
#define ARCHITECTURE_PIPELINE_H
#include <stddef.h>
#include <stdint.h>
#include "systems.h"
enum APStatus { AP_OK,AP_INVALID,AP_LIMIT };
typedef struct { unsigned op,dst,left,right,target,taken; } APInstruction;
typedef struct { unsigned forwarding,memory_cycles,write_first,branch_in_id,unified_port; } APConfig;
typedef struct {
 unsigned cycles,instances,retired_count,flushed_count,data_stalls,structural_stalls,port_stalls;
 int slots[128][5];unsigned pc[64],flush_cycle[64],resolve_cycle[64],retired[64],retire_cycle[64];
} APSchedule;
typedef struct { unsigned cycles,issue[16],complete[16],retire[16];uint8_t registers[8],value[16];int left_tag[16],right_tag[16]; } APOoo;
enum APStatus ap_schedule(const APInstruction *,size_t,APConfig,APSchedule *);
enum APStatus ap_ooo(const APInstruction *,size_t,const uint8_t [8],const uint8_t [16],unsigned,APOoo *);
int ap_test(const char *);
void ap_failure(const char *,unsigned);
void ap_emit(const char *,const int *,unsigned);
#endif
