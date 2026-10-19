#ifndef CHECKPOINT_FIRMWARE_H
#define CHECKPOINT_FIRMWARE_H
#include "checkpoint_depth.h"
#include <stdatomic.h>
typedef struct {uint32_t sequence;uint16_t value,tick;} CPFSample;
typedef struct {CPFSample samples[4];atomic_uint head,tail;unsigned capacity,dropped;} CPFRing;
int cpf_ring_init(CPFRing *,unsigned);
int cpf_irq_push(CPFRing *,CPFSample);
int cpf_take(CPFRing *,CPFSample *);
CPStatus cpf_frame(const CPFSample *,unsigned char *,size_t,size_t *);
typedef enum { CPF_IDLE,CPF_POLL,CPF_DMA,CPF_WIRE,CPF_ABORT,CPF_DONE,CPF_RETIRED } CPFTxState;
typedef enum { CPF_OK,CPF_PENDING,CPF_STALE,CPF_INVALID,CPF_FAILED } CPFStatus;
typedef struct {
    unsigned char wire[CP_WIRE];size_t length,accepted;unsigned generation,limit,retries,retry_limit;
    CPFTxState state;int retained,dma,error;uint32_t fault_generation,fault_code;
} CPFTx;
typedef void (*CPFWrite)(void *,unsigned char);
void cpf_tx_init(CPFTx *,unsigned,unsigned);
CPFStatus cpf_tx_begin(CPFTx *,const CPMessage *,int);
CPFStatus cpf_tx_poll(CPFTx *,unsigned,int,int,CPFWrite,void *);
CPFStatus cpf_tx_dma(CPFTx *,unsigned,int,size_t,int,int);
CPFStatus cpf_tx_timeout(CPFTx *,unsigned,uint32_t);
CPFStatus cpf_tx_recover(CPFTx *,unsigned,int,int);
CPFStatus cpf_tx_next(CPFTx *);
typedef struct {atomic_uint sequence,low,high;} CPFClock;
int cpf_clock_init(CPFClock *);
int cpf_clock_write(CPFClock *,uint64_t);
int cpf_clock_read(const CPFClock *,unsigned,uint64_t *);
int cpf_elapsed16(uint16_t,uint16_t,uint16_t *);
#endif
