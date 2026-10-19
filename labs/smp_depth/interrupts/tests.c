#include "interrupts.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"SI_CHECK:%s:%d\n",#x,__LINE__);return 1;}}while(0)
typedef struct {SiBox *box;uint64_t ticket,sum;unsigned calls,early;} Seen;
static void consume(void *context,uint32_t item) {
    Seen *s=context;
    if(atomic_load_explicit(&s->box->acknowledged,memory_order_acquire)==s->ticket)s->early++;
    s->sum+=item;s->calls++;
}
int main(void) {
    SiRound round={99,99,99};CHECK(si_round_init(&round,0,3)==SI_INVALID&&round.generation==99);
    CHECK(si_round_init(&round,7,3)==SI_OK);
    CHECK(si_round_accept(&round,0,6)==SI_STALE&&round.completed==0);
    CHECK(si_round_accept(&round,0,7)==SI_OK&&round.completed==1);
    CHECK(si_round_accept(&round,0,7)==SI_DUPLICATE&&round.completed==1);
    CHECK(si_round_accept(&round,2,7)==SI_INVALID&&round.completed==1);
    CHECK(si_round_accept(&round,1,7)==SI_OK&&round.completed==round.required);
    SiBox box;si_init(&box);uint32_t items[]={7,8,9,10};uint64_t ticket=999;
    CHECK(atomic_is_lock_free(&box.published));CHECK(si_status(&box,0)==SI_INVALID);
    CHECK(si_publish(&box,items,0,&ticket)==SI_INVALID&&ticket==999);
    CHECK(si_publish(&box,items,5,&ticket)==SI_INVALID&&ticket==999);
    CHECK(si_publish(&box,items,1,&ticket)==SI_OK&&ticket==1);
    Seen seen={&box,ticket,0,0,0};CHECK(si_status(&box,ticket)==SI_PENDING);
    uint64_t untouched=777;CHECK(si_publish(&box,items,4,&untouched)==SI_BUSY&&untouched==777&&box.count==1);
    CHECK(si_drain(&box,2,consume,&seen)==SI_STALE&&seen.calls==0);
    CHECK(si_drain(&box,ticket,consume,&seen)==SI_OK&&seen.sum==7&&seen.early==0);
    CHECK(si_drain(&box,ticket,consume,&seen)==SI_DUPLICATE&&seen.calls==1);
    for(unsigned generation=2;generation<=6;generation++) {
        CHECK(si_publish(&box,items,1,&ticket)==SI_OK&&ticket==generation);seen.ticket=ticket;
        CHECK(si_drain(&box,ticket,consume,&seen)==SI_OK);
    }
    CHECK(si_publish(&box,items,4,&ticket)==SI_OK&&ticket==7);seen.ticket=ticket;
    CHECK(si_drain(&box,6,consume,&seen)==SI_STALE&&seen.calls==6);
    CHECK(si_timeout(&box,7)==SI_TIMED_OUT&&si_status(&box,7)==SI_TIMED_OUT);
    CHECK(si_publish(&box,items,1,&untouched)==SI_BUSY&&untouched==777);
    CHECK(si_drain(&box,7,consume,&seen)==SI_OK&&seen.calls==10&&seen.sum==76&&seen.early==0);
    CHECK(si_status(&box,7)==SI_OK&&box.timed_out==1);
    CHECK(si_drain(&box,7,consume,&seen)==SI_DUPLICATE&&seen.calls==10);
    /* Controlled initialized fixture: terminal generation may finish; only new
       publication is refused. No live authority is restored from untrusted data. */
    si_init(&box);box.sequence=UINT64_MAX-1;atomic_store(&box.published,box.sequence);atomic_store(&box.acknowledged,box.sequence);
    CHECK(si_publish(&box,items,1,&ticket)==SI_OK&&ticket==UINT64_MAX);seen.ticket=ticket;
    CHECK(si_timeout(&box,ticket)==SI_TIMED_OUT&&si_drain(&box,ticket,consume,&seen)==SI_OK);
    CHECK(si_status(&box,ticket)==SI_OK&&si_publish(&box,items,1,&untouched)==SI_FULL&&untouched==777);
    puts("SI_PASS:publication-work-before-ack-coalescing-timeout-terminal-generation");return 0;
}
