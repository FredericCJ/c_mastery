#include "channel.h"
#include <limits.h>
#ifndef R5_MUTANT
#define R5_MUTANT 0
#endif
#define HALF UINT32_C(0x80000000)
static uint32_t rounded(uint32_t ns,uint32_t tick){return ns/tick+(ns%tick!=0);}
int r5_delays(const struct r5_timing *t,struct r5_delays *out) {
    struct r5_delays d;uint32_t enable;
    if(!t||!out||t->device>R5_THVD1400||t->receiver_disabled>1||!t->tick_ns||
       t->tick_ns>1000000||t->guard_ns>=HALF)return R5_INVALID;
    enable=t->device==R5_THVD1410?(t->receiver_disabled?11000u:600u):
                                    (t->receiver_disabled?10000u:650u);
    if(R5_MUTANT==1)enable=600;
    d.enable=rounded(enable,t->tick_ns);d.disable=rounded(200,t->tick_ns);
    d.guard=rounded(t->guard_ns,t->tick_ns);
    if(d.disable>=HALF-d.guard)return R5_INVALID;
    *out=d;return R5_OK;
}
int r5_load(unsigned n,unsigned bias,unsigned *out){
    if(!out||n>256||bias>256||n>256-bias)return R5_INVALID;
    *out=n+bias;return R5_OK;
}
static int valid(const struct r5_channel *s){
    return s&&s->phase>=R5_IDLE&&s->phase<=R5_TURNAROUND&&s->io.uart.read&&
        s->io.uart.write&&s->io.drive&&s->rx_capacity>=1&&s->rx_capacity<=16&&
        s->event_capacity>=3&&s->event_capacity<=32&&s->head<s->event_capacity&&
        s->queued<=s->event_capacity&&s->reserved<=s->event_capacity-s->queued&&
        s->rx_count<=s->rx_capacity;
}
static void event(struct r5_channel *s,unsigned kind,uint8_t value,uint8_t error,uint32_t token){
    unsigned at=(s->head+s->queued)%s->event_capacity;
    s->events[at]=(struct r5_event){kind,value,error,token};++s->queued;
    if(kind==R5_BYTE)++s->rx_count;
}
static int clock_ok(const struct r5_channel *s,uint32_t now){return now-s->last<HALF;}
static int reached(uint32_t now,uint32_t due){return now-due<HALF;}
int r5_open(struct r5_channel *s,struct r5_io io,const struct pu_clock *clock,uint32_t baud,
            const struct r5_timing *timing,unsigned rx_cap,unsigned event_cap,uint32_t now){
    struct r5_delays delays;
    if(!s||s->phase!=R5_CLOSED||!io.uart.read||!io.uart.write||!io.drive||
       rx_cap<1||rx_cap>16||event_cap<3||event_cap>32||
       r5_delays(timing,&delays)!=R5_OK)return R5_INVALID;
    int r=pu_open(&s->tx.byte,io.uart,clock,baud,0,1);
    if(r!=PU_OK)return R5_UNAVAILABLE;
    s->io=io;s->delays=delays;s->rx_capacity=rx_cap;s->event_capacity=event_cap;
    s->phase=R5_IDLE;s->last=now;io.drive(io.context,0);return R5_OK;
}
int r5_send(struct r5_channel *s,uint32_t token,const uint8_t *data,size_t n,
            unsigned backend,uint32_t now,uint32_t budget,size_t *accepted){
    if(!valid(s)||!accepted||!data||!n||n>16||!budget||budget>=HALF||
       (backend!=PU_POLL&&backend!=PU_DMA)||!clock_ok(s,now))return R5_INVALID;
    if(s->phase!=R5_IDLE)return R5_BUSY;
    if(!token||(R5_MUTANT!=5&&token<=s->token)||(n+7)/8>UINT32_MAX-s->tx.byte.token)return R5_STALE;
    if(s->event_capacity-s->queued<2)return R5_FULL;
    if(s->delays.enable>=budget)return R5_INVALID;
    for(size_t i=0;i<n;++i)s->tx_bytes[i]=data[i];
    s->count=(unsigned)n;s->offset=0;s->chunk=0;s->backend=backend;s->token=token;
    s->deadline=now+budget;s->due=now+s->delays.enable;s->last=now;
    s->reserved=R5_MUTANT==4?1u:2u;s->unavailable=0;s->result=PU_BUSY;s->phase=R5_ENABLING;
    s->io.drive(s->io.context,1);*accepted=n;return R5_OK;
}
static void turn(struct r5_channel *s,uint32_t now,unsigned result){
    s->result=result;s->io.drive(s->io.context,0);s->phase=R5_TURNAROUND;
    s->due=now+s->delays.disable+s->delays.guard;
}
static int start_lower(struct r5_channel *s,uint32_t now){
    uint32_t remaining=s->deadline-now;
    if(reached(now,s->deadline)){turn(s,now,PU_TIMEOUT);return R5_BUSY;}
    int r;
    if(s->backend==PU_POLL)r=ua_begin(&s->tx,s->io.uart,s->tx.byte.configured_generation,
                                     s->token,s->tx_bytes,s->count,now,remaining);
    else{
        s->chunk=s->count-s->offset;if(s->chunk>8)s->chunk=8;
        r=pu_submit(&s->tx.byte,s->io.uart,s->tx.byte.configured_generation,
                    s->tx.byte.token+1,PU_DMA,s->tx_bytes+s->offset,s->chunk,now,remaining);
    }
    if(r!=PU_OK){turn(s,now,(unsigned)r);return R5_UNAVAILABLE;}
    s->phase=R5_ACTIVE;return R5_BUSY;
}
int r5_step(struct r5_channel *s,uint32_t token,uint32_t now){
    if(!valid(s)||!clock_ok(s,now))return R5_INVALID;
    if(s->phase==R5_IDLE||token!=s->token)return R5_STALE;
    s->last=now;
    if(s->phase==R5_ENABLING){if(!reached(now,s->due))return R5_BUSY;return start_lower(s,now);}
    if(s->phase==R5_TURNAROUND){
        if(!reached(now,s->due))return R5_BUSY;
        --s->reserved;event(s,s->result==PU_OK?R5_TX_DONE:R5_ABORTED,0,0,s->token);
        s->reserved=0;s->phase=R5_IDLE;return R5_OK;
    }
    int r=s->backend==PU_POLL?ua_step(&s->tx,s->io.uart,s->token,now):
                              pu_step(&s->tx.byte,s->io.uart,s->tx.byte.token,now);
    unsigned owned=s->tx.byte.active||s->tx.byte.memory_owned||s->tx.byte.line_owned||
        (s->backend==PU_POLL&&s->tx.active);
    if(R5_MUTANT==2 && s->tx.byte.position==s->tx.byte.count)owned=0;
    if(owned){
        if((reached(now,s->deadline)||s->result==PU_CANCELLED)&&!s->unavailable){
            s->unavailable=1;--s->reserved;event(s,R5_DEVICE_UNAVAILABLE,0,0,s->token);
        }
        return s->unavailable?R5_UNAVAILABLE:R5_BUSY;
    }
    if(s->backend==PU_POLL)s->offset=s->tx.offset;
    else if(r==PU_OK){s->offset+=s->chunk;if(s->offset<s->count&&s->result!=PU_CANCELLED)return start_lower(s,now);}
    turn(s,now,s->result==PU_CANCELLED?PU_CANCELLED:(unsigned)r);return R5_BUSY;
}
int r5_cancel(struct r5_channel *s,uint32_t token,uint32_t now){
    if(!valid(s)||!clock_ok(s,now))return R5_INVALID;
    if(s->phase==R5_IDLE||s->token!=token)return R5_STALE;
    s->last=now;
    if(s->phase==R5_TURNAROUND)return R5_BUSY;
    s->result=PU_CANCELLED;
    if(s->phase==R5_ENABLING){turn(s,now,PU_CANCELLED);return R5_OK;}
    int r=s->backend==PU_POLL?ua_cancel(&s->tx,s->io.uart,s->token):
                              pu_cancel(&s->tx.byte,s->io.uart,s->tx.byte.token);
    return r==PU_OK?R5_OK:R5_UNAVAILABLE;
}
int r5_receive(struct r5_channel *s,unsigned status,uint8_t value,size_t *consumed){
    unsigned errors=status&15u,need=(status&PU_RXNE?1u:0u)+(errors&7u?1u:0u)+(errors&PU_ORE?1u:0u);
    if(!valid(s)||!consumed||(status&~(15u|PU_RXNE))||!need)return R5_INVALID;
    if(s->phase!=R5_IDLE){if(s->echoes==UINT_MAX)return R5_FULL;++s->echoes;*consumed=1;return R5_OK;}
    if(need>s->event_capacity-s->queued-s->reserved ||
       ((status&PU_RXNE)&&s->rx_count==s->rx_capacity))return R5_FULL;
    if(status&PU_RXNE)event(s,R5_BYTE,value,(uint8_t)errors,0);
    if(errors&7u)event(s,R5_FRAMING,value,(uint8_t)errors,0);
    if(errors&PU_ORE)event(s,R5_OVERRUN,value,(uint8_t)errors,0);
    *consumed=1;return R5_OK;
}
int r5_receive_poll(struct r5_channel *s){
    if(!valid(s))return R5_INVALID;
    if(R5_MUTANT!=6&&(s->event_capacity-s->queued-s->reserved<3||s->rx_count==s->rx_capacity))return R5_FULL;
    int r=pu_rx_poll(&s->rx,s->io.uart,1);if(r==PU_TIMEOUT)return R5_BUSY;if(r!=PU_OK)return R5_UNAVAILABLE;
    struct pu_byte b;if(pu_take(&s->rx,&b)!=PU_OK)return R5_UNAVAILABLE;
    size_t consumed=0;return r5_receive(s,PU_RXNE|b.error,b.value,&consumed);
}
int r5_take(struct r5_channel *s,struct r5_event *out,size_t *consumed){
    if(!valid(s)||!out||!consumed)return R5_INVALID;
    if(!s->queued){*consumed=0;return R5_BUSY;}
    *out=s->events[s->head];s->head=(s->head+1)%s->event_capacity;--s->queued;
    if(out->kind==R5_BYTE)--s->rx_count;
    *consumed=1;return R5_OK;
}
int r5_loss(struct r5_channel *s){
    if(!valid(s))return R5_INVALID;
    if(s->queued+s->reserved==s->event_capacity)return R5_FULL;
    event(s,R5_LOSS,0,0,0);return R5_OK;
}
int r5_link(struct r5_link *s,const struct r5_event *e,struct edu_frame *out){
    if(!s||!e||!out||s->local<1||s->local>2||s->grant>1||e->kind<R5_BYTE||e->kind>R5_DEVICE_UNAVAILABLE)return R5_INVALID;
    if(e->kind==R5_FRAMING||e->kind==R5_OVERRUN||e->kind==R5_LOSS||
       (e->kind==R5_BYTE&&e->error)){
        if(s->faults==UINT_MAX)return R5_FULL;
        ++s->faults;
        if(R5_MUTANT!=3){struct edu_frame ignored;(void)edu_frame_byte(&s->parser,0,1,&ignored);}
        return R5_OK;
    }
    if(e->kind!=R5_BYTE)return R5_OK;
    if(s->delivered==UINT_MAX||s->rejected==UINT_MAX)return R5_FULL;
    struct edu_frame f;int r=edu_frame_byte(&s->parser,e->value,0,&f);
    if(r<0){++s->rejected;return R5_OK;}
    if(r==1){
        if(f.destination!=s->local||!s->grant){++s->rejected;return R5_OK;}
        ++s->delivered;*out=f;return R5_FRAME;
    }
    return R5_BUSY;
}
