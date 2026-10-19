#include "interrupts.h"
#ifndef SI_MUTANT
#define SI_MUTANT 0
#endif
int si_round_init(SiRound *round,uint64_t generation,unsigned mask) {
    if(!round||!generation||!mask||(mask&~15u))return SI_INVALID;
    *round=(SiRound){generation,mask,0};return SI_OK;
}
int si_round_accept(SiRound *round,unsigned cpu,uint64_t generation) {
    if(!round||cpu>=4||!(round->required&(1u<<cpu)))return SI_INVALID;
    if(SI_MUTANT!=4&&generation!=round->generation)return SI_STALE;
    unsigned bit=1u<<cpu;
    if(round->completed&bit)return SI_DUPLICATE;
    round->completed|=bit;return SI_OK;
}
void si_init(SiBox *box) {
    atomic_init(&box->published,0);atomic_init(&box->acknowledged,0);
    box->sequence=0;box->count=0;box->timed_out=0;
    for(unsigned i=0;i<SI_CAPACITY;i++)box->items[i]=0;
}
int si_publish(SiBox *box,const uint32_t *items,size_t n,uint64_t *ticket) {
    if(!box||!items||!ticket||!n||n>SI_CAPACITY)return SI_INVALID;
    if(SI_MUTANT!=2&&atomic_load_explicit(&box->acknowledged,memory_order_acquire)!=box->sequence)return SI_BUSY;
    if(box->sequence==UINT64_MAX)return SI_FULL;
    uint64_t next=box->sequence+1;
    for(size_t i=0;i<n;i++)box->items[i]=items[i];
    box->count=(unsigned)n;box->sequence=next;box->timed_out=0;
    atomic_store_explicit(&box->published,next,memory_order_release);
    *ticket=next;return SI_OK;
}
int si_drain(SiBox *box,uint64_t ticket,SiConsume consume,void *context) {
    if(!box||!consume||!ticket)return SI_INVALID;
    uint64_t published=atomic_load_explicit(&box->published,memory_order_acquire);
    if(SI_MUTANT!=3&&ticket!=published)return SI_STALE;
    if(ticket<=atomic_load_explicit(&box->acknowledged,memory_order_relaxed))return SI_DUPLICATE;
    if(SI_MUTANT==1)atomic_store_explicit(&box->acknowledged,ticket,memory_order_release);
    for(unsigned i=0;i<box->count;i++)consume(context,box->items[i]);
    atomic_store_explicit(&box->acknowledged,ticket,memory_order_release);
    return SI_OK;
}
int si_status(const SiBox *box,uint64_t ticket) {
    if(!box||!ticket)return SI_INVALID;
    if(ticket!=box->sequence)return SI_STALE;
    if(atomic_load_explicit(&box->acknowledged,memory_order_acquire)==ticket)return SI_OK;
    return box->timed_out?SI_TIMED_OUT:SI_PENDING;
}
int si_timeout(SiBox *box,uint64_t ticket) {
    int status=si_status(box,ticket);
    if(status==SI_PENDING){box->timed_out=1;return SI_TIMED_OUT;}
    return status;
}
