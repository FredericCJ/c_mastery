#ifndef CHECKPOINT_FIRMWARE_H
#define CHECKPOINT_FIRMWARE_H
#include "checkpoint_depth.h"
#include <stdatomic.h>
struct cpf_sample {uint32_t sequence;uint16_t value,tick;};
struct cpf_ring {struct cpf_sample samples[4];atomic_uint head,tail;unsigned capacity,dropped;};
int cpf_ring_init(struct cpf_ring *,unsigned);
int cpf_irq_push(struct cpf_ring *,struct cpf_sample);
int cpf_take(struct cpf_ring *,struct cpf_sample *);
enum cp_status cpf_frame(const struct cpf_sample *,unsigned char *,size_t,size_t *);
enum cpf_tx_state { CPF_IDLE,CPF_POLL,CPF_DMA,CPF_WIRE,CPF_ABORT,CPF_DONE,CPF_RETIRED };
enum cpf_status { CPF_OK,CPF_PENDING,CPF_STALE,CPF_INVALID,CPF_FAILED };
struct cpf_tx {
    unsigned char wire[CP_WIRE];size_t length,accepted;unsigned generation,limit,retries,retry_limit;
    enum cpf_tx_state state;int retained,dma,error;uint32_t fault_generation,fault_code;
};
typedef void (*cpf_write_fn)(void *,unsigned char);
void cpf_tx_init(struct cpf_tx *,unsigned,unsigned);
enum cpf_status cpf_tx_begin(struct cpf_tx *,const struct cp_message *,int);
enum cpf_status cpf_tx_poll(struct cpf_tx *,unsigned,int,int,cpf_write_fn,void *);
enum cpf_status cpf_tx_dma(struct cpf_tx *,unsigned,int,size_t,int,int);
enum cpf_status cpf_tx_timeout(struct cpf_tx *,unsigned,uint32_t);
enum cpf_status cpf_tx_recover(struct cpf_tx *,unsigned,int,int);
enum cpf_status cpf_tx_next(struct cpf_tx *);
struct cpf_clock {atomic_uint sequence,low,high;};
int cpf_clock_init(struct cpf_clock *);
int cpf_clock_write(struct cpf_clock *,uint64_t);
int cpf_clock_read(const struct cpf_clock *,unsigned,uint64_t *);
int cpf_elapsed16(uint16_t,uint16_t,uint16_t *);
#endif
