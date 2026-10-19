#ifndef PROTOCOL_FOLLOW_H
#define PROTOCOL_FOLLOW_H
#include "protocols.h"
#include "firmware_io.h"
typedef void (*PFFree)(void *,void *);
typedef struct PFDriver PFDriver;
typedef struct {PFDriver *owner;uint32_t epoch,ticket;} PFRecovery;
struct PFDriver {
 PTDriver protocol;FwDma dma;FwWatchdog watchdog;FwIo io;
 unsigned char *owned;size_t length;void *free_context;PFFree release;
 uint32_t epoch,loan_epoch;unsigned reset_pending,old_epoch_idle;
};
int pf_driver_init(PFDriver *,FwIo,uint32_t,PFFree,void *);
int pf_driver_ready(PFDriver *);
int pf_driver_start(PFDriver *,unsigned char **,size_t,unsigned,PFRecovery *);
int pf_driver_reset(PFRecovery *);
int pf_driver_recover(PFRecovery *,uint32_t observed_epoch,unsigned idle);
int pf_driver_complete(PFRecovery *,uint32_t observed_epoch,unsigned idle);
enum {PF_POLL=0,PF_DMA=1};
typedef struct {PTBuilder base;unsigned mode_set,mode,callback;} PFBuilder;
typedef struct {PTConfig base;unsigned mode,callback;} PFConfig;
int pf_builder_mode(PFBuilder *,unsigned,unsigned);
int pf_builder_replace(PFBuilder *,unsigned,unsigned);
int pf_builder_finish(const PFBuilder *,PFConfig *);
enum {PF_SEND=0,PF_APPLY=1,PF_ACK=2,PF_RETRY=3,PF_CANCEL=4,PF_CRASH=5,PF_RECONNECT=6};
typedef struct {unsigned phase,closed,crashed;uint32_t next,pending,last;int32_t pending_value,total,last_value;} PFSession;
int pf_session(PFSession *,unsigned,uint32_t,int32_t);
typedef void *(*PFAlloc)(void *,size_t);
typedef struct {unsigned pc,header_pc,body_pc;size_t length,used;unsigned char checksum,*buffer;uint64_t deadline,last_time;PFAlloc allocate;PFFree release;void *context;} PFDecoder;
int pf_decoder_init(PFDecoder *,uint64_t,PFAlloc,PFFree,void *);
int pf_decoder_step(PFDecoder *,unsigned,unsigned char,uint64_t,unsigned char **,size_t *);
int pf_decoder_cancel(PFDecoder *);
#endif
