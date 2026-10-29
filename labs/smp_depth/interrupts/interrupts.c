#include "interrupts.h"
int si_round_init(struct si_round *r,uint64_t g,unsigned mask){(void)r;(void)g;(void)mask;return SI_INVALID;}
int si_round_accept(struct si_round *r,unsigned cpu,uint64_t g){(void)r;(void)cpu;(void)g;return SI_INVALID;}
void si_init(struct si_box *box) { (void)box; }
int si_publish(struct si_box *box,const uint32_t *items,size_t n,uint64_t *ticket) {
    (void)box;(void)items;(void)n;(void)ticket;return SI_INVALID;
}
int si_drain(struct si_box *box,uint64_t ticket,si_consume_fn consume,void *context) {
    (void)box;(void)ticket;(void)consume;(void)context;return SI_INVALID;
}
int si_status(const struct si_box *box,uint64_t ticket) {(void)box;(void)ticket;return SI_INVALID;}
int si_timeout(struct si_box *box,uint64_t ticket) {(void)box;(void)ticket;return SI_INVALID;}
