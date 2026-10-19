#include "interrupts.h"
int si_round_init(SiRound *r,uint64_t g,unsigned mask){(void)r;(void)g;(void)mask;return SI_INVALID;}
int si_round_accept(SiRound *r,unsigned cpu,uint64_t g){(void)r;(void)cpu;(void)g;return SI_INVALID;}
void si_init(SiBox *box) { (void)box; }
int si_publish(SiBox *box,const uint32_t *items,size_t n,uint64_t *ticket) {
    (void)box;(void)items;(void)n;(void)ticket;return SI_INVALID;
}
int si_drain(SiBox *box,uint64_t ticket,SiConsume consume,void *context) {
    (void)box;(void)ticket;(void)consume;(void)context;return SI_INVALID;
}
int si_status(const SiBox *box,uint64_t ticket) {(void)box;(void)ticket;return SI_INVALID;}
int si_timeout(SiBox *box,uint64_t ticket) {(void)box;(void)ticket;return SI_INVALID;}
