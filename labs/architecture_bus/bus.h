#ifndef ARCHITECTURE_BUS_H
#define ARCHITECTURE_BUS_H
#include <stddef.h>
#include <stdint.h>
#include "dma.h"
enum { MB_F439,MB_F407 };
enum { MB_CCM,MB_SRAM1,MB_SRAM2,MB_SRAM3,MB_APB1,MB_APB2 };
enum { MB_CPU_D,MB_CPU_S,MB_DMA1_M,MB_DMA1_P,MB_DMA2_M,MB_DMA2_P };
enum { MB_OK,MB_INVALID,MB_UNREACHABLE,MB_EXTENT };
typedef struct {uint32_t base,bytes;unsigned present,masters;} MBRegion;
typedef struct {unsigned part,manual_revision;MBRegion regions[6];uint32_t ordinary_bytes;} MBMap;
typedef struct {PdTransfer transfer;unsigned char*source,*destination;uint32_t bytes,source_address,destination_address;unsigned bound;} MBLease;
typedef struct {unsigned active_mask,beats[3],wait_cycles[3],rounds,max_beats,max_wait;} MBBusConfig;
typedef struct {unsigned count,bound,master[24],start[24],finish[24],wait[24];} MBBusTrace;
int mb_map(unsigned,unsigned,MBMap*);
int mb_place(unsigned,unsigned,unsigned,unsigned,uint32_t,uint32_t,unsigned,uint32_t*);
int mb_bind(MBLease*,unsigned char*,unsigned char*,uint32_t,uint32_t,unsigned,uint32_t,unsigned);
int mb_give(MBLease*,PdIo,uint32_t);
int mb_cpu_write(MBLease*,uint32_t,unsigned char);
int mb_cpu_read(const MBLease*,uint32_t,unsigned char*);
int mb_poll(MBLease*,PdIo,uint32_t);
int mb_finish(MBLease*,PdIo,uint32_t,PdResult*);
int mb_arbitrate(MBBusConfig,MBBusTrace*);
int mb_bridge(unsigned,uint32_t,uint32_t*);
int mb_usart_width(unsigned);
int mb_test(const char*);
void mb_emit(const char*,const uint32_t*,size_t);
void mb_failure(const char*,unsigned);
#endif
