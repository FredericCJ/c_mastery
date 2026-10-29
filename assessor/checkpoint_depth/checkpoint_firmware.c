#include "checkpoint_firmware.h"
#include <limits.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
int cpf_ring_init(struct cpf_ring *r,unsigned capacity){
    if(!r || capacity<2 || capacity>4)return 0;
    atomic_init(&r->head,0);atomic_init(&r->tail,0);r->capacity=capacity;r->dropped=0;
    return atomic_is_lock_free(&r->head) && atomic_is_lock_free(&r->tail);
}
int cpf_irq_push(struct cpf_ring *r,struct cpf_sample sample){
    unsigned head=atomic_load_explicit(&r->head,memory_order_relaxed),next=head+1==r->capacity?0:head+1;
    if(next==atomic_load_explicit(&r->tail,memory_order_acquire)){
        if(r->dropped!=UINT_MAX)++r->dropped;
        if(CP_MUTANT==33)atomic_store_explicit(&r->tail,next+1==r->capacity?0:next+1,memory_order_release);
        else return 0;
    }
    r->samples[head]=sample;atomic_store_explicit(&r->head,next,memory_order_release);return 1;
}
int cpf_take(struct cpf_ring *r,struct cpf_sample *sample){
    unsigned tail=atomic_load_explicit(&r->tail,memory_order_relaxed);
    if(tail==atomic_load_explicit(&r->head,memory_order_acquire))return 0;
    *sample=r->samples[tail];atomic_store_explicit(&r->tail,tail+1==r->capacity?0:tail+1,memory_order_release);return 1;
}
enum cp_status cpf_frame(const struct cpf_sample *s,unsigned char *out,size_t capacity,size_t *written){
    struct cp_message message={0};message.sequence=(uint16_t)s->sequence;message.length=8;
    for(unsigned i=0;i<4;++i)message.payload[i]=(unsigned char)(s->sequence>>(i*8));
    message.payload[4]=(unsigned char)s->value;message.payload[5]=(unsigned char)(s->value>>8);
    message.payload[6]=(unsigned char)s->tick;message.payload[7]=(unsigned char)(s->tick>>8);
    if(CP_MUTANT==34)message.payload[6]=0;
    return cp_encode(&message,out,capacity,written);
}
void cpf_tx_init(struct cpf_tx *t,unsigned limit,unsigned retry_limit){
    t->length=t->accepted=0;t->generation=1;t->limit=limit;t->retries=0;t->retry_limit=retry_limit;
    t->state=limit?CPF_IDLE:CPF_RETIRED;t->retained=t->dma=t->error=0;t->fault_generation=t->fault_code=0;
}
enum cpf_status cpf_tx_begin(struct cpf_tx *t,const struct cp_message *message,int dma){
    if(!t || t->state!=CPF_IDLE)return CPF_INVALID;
    size_t written;enum cp_status status=cp_encode(message,t->wire,sizeof t->wire,&written);
    if(status!=CP_OK)return CPF_INVALID;
    t->length=written;t->accepted=0;t->retained=1;t->error=0;t->dma=!!dma;t->state=dma?CPF_DMA:CPF_POLL;return CPF_OK;
}
static enum cpf_status done(struct cpf_tx *t){t->state=CPF_DONE;t->retained=0;return CPF_OK;}
enum cpf_status cpf_tx_poll(struct cpf_tx *t,unsigned generation,int txe,int tc,cpf_write_fn write,void *context){
    if(generation!=t->generation)return CPF_STALE;
    if(t->state==CPF_WIRE && tc)return done(t);
    if(t->state==CPF_WIRE)return CPF_PENDING;
    if(t->state!=CPF_POLL || !write)return CPF_INVALID;
    if(!txe)return CPF_PENDING;
    write(context,t->wire[t->accepted++]);
    if(t->accepted==t->length){t->state=CPF_WIRE;if(CP_MUTANT==35)return done(t);}
    return CPF_PENDING;
}
enum cpf_status cpf_tx_dma(struct cpf_tx *t,unsigned generation,int enabled,size_t remaining,int tc,int failed){
    if(generation!=t->generation)return CPF_STALE;
    if(t->state!=CPF_DMA && t->state!=CPF_WIRE)return CPF_INVALID;
    if(failed){t->error=1;t->state=CPF_ABORT;t->fault_generation=generation;t->fault_code=1;return CPF_FAILED;}
    if(t->state==CPF_DMA && !enabled && !remaining){t->accepted=t->length;t->state=CPF_WIRE;}
    if(t->state==CPF_WIRE && tc)return done(t);
    return CPF_PENDING;
}
enum cpf_status cpf_tx_timeout(struct cpf_tx *t,unsigned generation,uint32_t code){
    if(generation!=t->generation)return CPF_STALE;
    if(t->state!=CPF_DMA && t->state!=CPF_POLL && t->state!=CPF_WIRE && t->state!=CPF_ABORT)return CPF_INVALID;
    if(t->state!=CPF_ABORT){t->fault_generation=generation;t->fault_code=code;}
    t->state=CPF_ABORT;t->error=1;return CPF_PENDING;
}
enum cpf_status cpf_tx_recover(struct cpf_tx *t,unsigned generation,int dma_enabled,int uart_enabled){
    if(generation!=t->generation)return CPF_STALE;
    if(t->state!=CPF_ABORT)return CPF_INVALID;
    if(CP_MUTANT!=36 && (dma_enabled || uart_enabled))return CPF_PENDING;
    t->retained=0;++t->retries;
    if(t->generation==t->limit || t->retries>=t->retry_limit){t->state=CPF_RETIRED;return CPF_FAILED;}
    ++t->generation;t->state=CPF_IDLE;return CPF_OK;
}
enum cpf_status cpf_tx_next(struct cpf_tx *t){
    if(t->state!=CPF_DONE)return CPF_INVALID;
    if(t->generation==t->limit){t->state=CPF_RETIRED;return CPF_FAILED;}
    ++t->generation;t->state=CPF_IDLE;return CPF_OK;
}
int cpf_clock_init(struct cpf_clock *c){
    atomic_init(&c->sequence,0);atomic_init(&c->low,0);atomic_init(&c->high,0);
    return sizeof(unsigned)==4 && atomic_is_lock_free(&c->sequence) && atomic_is_lock_free(&c->low) && atomic_is_lock_free(&c->high);
}
int cpf_clock_write(struct cpf_clock *c,uint64_t value){
    unsigned sequence=atomic_load(&c->sequence);
    if((sequence&1) || sequence>UINT_MAX-2)return 0;
    atomic_store(&c->sequence,sequence+1);atomic_store(&c->low,(uint32_t)value);atomic_store(&c->high,(uint32_t)(value>>32));atomic_store(&c->sequence,sequence+2);return 1;
}
int cpf_clock_read(const struct cpf_clock *c,unsigned attempts,uint64_t *out){
    if(!out)return 0;
    for(unsigned i=0;i<attempts;++i){
        unsigned before=atomic_load(&c->sequence);if((before&1) && CP_MUTANT!=37)continue;
        uint32_t low=atomic_load(&c->low),high=atomic_load(&c->high);unsigned after=atomic_load(&c->sequence);
        if(before==after){*out=((uint64_t)high<<32)|low;return 1;}
    }
    return 0;
}
int cpf_elapsed16(uint16_t start,uint16_t end,uint16_t *elapsed){
    uint16_t difference=(uint16_t)(end-start);
    if(!elapsed || (CP_MUTANT!=38 && difference>=UINT16_C(32768)))return 0;
    *elapsed=difference;return 1;
}
