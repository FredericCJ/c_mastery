#ifndef CAPSTONE_FIRMWARE_H
#define CAPSTONE_FIRMWARE_H
#include "service.h"
#include "checkpoint_firmware.h"
#include "dma.h"
typedef enum { CF_BOOT,CF_RUNNING,CF_RECOVERING,CF_SAFE_STOP } CfState;
typedef enum { CF_OK,CF_PENDING,CF_INVALID,CF_DENIED,CF_REPLAY,CF_FULL,CF_FAILED } CfStatus;
typedef struct {
    CsService core;CPFRing samples;PdTransfer transfer;
    unsigned char dma_source[64],dma_destination[64];
    CfState state;uint32_t first_fault,dma_token;unsigned attempts,feeds,actuator;
    uint32_t prior_accepted,prior_delivered,prior_discarded;
    unsigned command_valid;uint16_t last_command;unsigned last_value;
} CfFirmware;
typedef struct { unsigned char payload[8];uint16_t generation;uint32_t compatible,crc,verified,commit_a,commit_b; } CfSlot;
typedef struct { uint32_t sequence,cause,check,commit; } CfCrash;
typedef struct { unsigned flags; } CfRegister;
unsigned cf_status_read(CfRegister *);
void cf_status_write(CfRegister *,unsigned);
int cf_status_ack(CfRegister *,unsigned);
int cf_init(CfFirmware *);
CfStatus cf_boot(CfFirmware *,int (*ready)(void *),void *,unsigned);
CfStatus cf_acquire(CfFirmware *,unsigned,const CPFSample *);
CfStatus cf_foreground(CfFirmware *,int);
CfStatus cf_command(CfFirmware *,const CPMessage *,int);
CfStatus cf_dma_begin(CfFirmware *,PdIo,uint32_t,uint32_t,uint32_t);
CfStatus cf_dma_cancel(CfFirmware *,PdIo,uint32_t);
CfStatus cf_dma_finish(CfFirmware *,PdIo,uint32_t);
unsigned cf_watchdog(CfFirmware *,unsigned);
void cf_fault(CfFirmware *,uint32_t);
CfStatus cf_recover(CfFirmware *);
int cf_budget(uint32_t,uint32_t,uint32_t *,uint32_t *);
void cf_candidate(CfSlot *,uint16_t,unsigned,unsigned,unsigned,unsigned);
int cf_select(const CfSlot *,const CfSlot *,unsigned *);
void cf_crash(CfCrash *,uint32_t,uint32_t,unsigned);
int cf_crash_select(const CfCrash *,const CfCrash *,unsigned *);
int cf_copy_request(const unsigned char *,size_t,size_t,size_t,unsigned char[8]);
int cf_elapsed(uint16_t,uint16_t,unsigned,uint32_t *);
#endif
