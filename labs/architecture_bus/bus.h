#ifndef ARCHITECTURE_BUS_H
#define ARCHITECTURE_BUS_H
#include <stddef.h>
#include <stdint.h>
#include "dma.h"
enum { MB_F439,MB_F407 };
enum { MB_CCM,MB_SRAM1,MB_SRAM2,MB_SRAM3,MB_APB1,MB_APB2 };
enum { MB_CPU_D,MB_CPU_S,MB_DMA1_M,MB_DMA1_P,MB_DMA2_M,MB_DMA2_P };
enum { MB_OK,MB_INVALID,MB_UNREACHABLE,MB_EXTENT };
struct mb_region {uint32_t base,bytes;unsigned present,masters;};
struct mb_map {unsigned part,manual_revision;struct mb_region regions[6];uint32_t ordinary_bytes;};
struct mb_lease {struct pd_transfer transfer;unsigned char*source,*destination;uint32_t bytes,source_address,destination_address;unsigned bound;};
struct mb_bus_config {unsigned active_mask,beats[3],wait_cycles[3],rounds,max_beats,max_wait;};
struct mb_bus_trace {unsigned count,bound,master[24],start[24],finish[24],wait[24];};
int mb_map(unsigned,unsigned,struct mb_map*);
int mb_place(unsigned,unsigned,unsigned,unsigned,uint32_t,uint32_t,unsigned,uint32_t*);
int mb_bind(struct mb_lease*,unsigned char*,unsigned char*,uint32_t,uint32_t,unsigned,uint32_t,unsigned);
int mb_give(struct mb_lease*,struct pd_io,uint32_t);
int mb_cpu_write(struct mb_lease*,uint32_t,unsigned char);
int mb_cpu_read(const struct mb_lease*,uint32_t,unsigned char*);
int mb_poll(struct mb_lease*,struct pd_io,uint32_t);
int mb_finish(struct mb_lease*,struct pd_io,uint32_t,struct pd_result*);
int mb_arbitrate(struct mb_bus_config,struct mb_bus_trace*);
int mb_bridge(unsigned,uint32_t,uint32_t*);
int mb_usart_width(unsigned);
int mb_test(const char*);
void mb_emit(const char*,const uint32_t*,size_t);
void mb_failure(const char*,unsigned);
#endif
