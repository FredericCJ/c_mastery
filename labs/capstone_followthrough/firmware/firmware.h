#ifndef CAPSTONE_FIRMWARE_H
#define CAPSTONE_FIRMWARE_H
#include "service.h"
#include "checkpoint_firmware.h"
#include "dma.h"
enum cf_state { CF_BOOT,CF_RUNNING,CF_RECOVERING,CF_SAFE_STOP };
enum cf_status { CF_OK,CF_PENDING,CF_INVALID,CF_DENIED,CF_REPLAY,CF_FULL,CF_FAILED };
struct cf_firmware {
    struct cs_service core;struct cpf_ring samples;struct pd_transfer transfer;
    unsigned char dma_source[64],dma_destination[64];
    enum cf_state state;uint32_t first_fault,dma_token;unsigned attempts,feeds,actuator;
    uint32_t prior_accepted,prior_delivered,prior_discarded;
    unsigned command_valid;uint16_t last_command;unsigned last_value;
};
struct cf_slot { unsigned char payload[8];uint16_t generation;uint32_t compatible,crc,verified,commit_a,commit_b; };
struct cf_crash { uint32_t sequence,cause,check,commit; };
struct cf_register { unsigned flags; };
unsigned cf_status_read(struct cf_register *);
void cf_status_write(struct cf_register *,unsigned);
int cf_status_ack(struct cf_register *,unsigned);
int cf_init(struct cf_firmware *);
enum cf_status cf_boot(struct cf_firmware *,int (*ready)(void *),void *,unsigned);
enum cf_status cf_acquire(struct cf_firmware *,unsigned,const struct cpf_sample *);
enum cf_status cf_foreground(struct cf_firmware *,int);
enum cf_status cf_command(struct cf_firmware *,const struct cp_message *,int);
enum cf_status cf_dma_begin(struct cf_firmware *,struct pd_io,uint32_t,uint32_t,uint32_t);
enum cf_status cf_dma_cancel(struct cf_firmware *,struct pd_io,uint32_t);
enum cf_status cf_dma_finish(struct cf_firmware *,struct pd_io,uint32_t);
unsigned cf_watchdog(struct cf_firmware *,unsigned);
void cf_fault(struct cf_firmware *,uint32_t);
enum cf_status cf_recover(struct cf_firmware *);
int cf_budget(uint32_t,uint32_t,uint32_t *,uint32_t *);
void cf_candidate(struct cf_slot *,uint16_t,unsigned,unsigned,unsigned,unsigned);
int cf_select(const struct cf_slot *,const struct cf_slot *,unsigned *);
void cf_crash(struct cf_crash *,uint32_t,uint32_t,unsigned);
int cf_crash_select(const struct cf_crash *,const struct cf_crash *,unsigned *);
int cf_copy_request(const unsigned char *,size_t,size_t,size_t,unsigned char[8]);
int cf_elapsed(uint16_t,uint16_t,unsigned,uint32_t *);
#endif
