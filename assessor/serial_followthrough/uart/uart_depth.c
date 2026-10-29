#include "uart_depth.h"
#include <limits.h>
#ifndef UA_MUTANT
#define UA_MUTANT 0
#endif
int ua_format(uint32_t clock,uint32_t baud,unsigned over8,unsigned width,
              unsigned parity,unsigned stop,uint16_t value,struct ua_format *out) {
  if (!out || (width==7 && parity==0)) return PU_INVALID;
  struct ua_format r={0};
  if (sd_uart(clock,baud,over8,width,parity,stop,value,&r.wire)) return PU_INVALID;
  r.cr1=0x2000u|(over8<<15)|(parity?0x400u:0)|(parity==2?0x200u:0);
  if (width==8 && parity && UA_MUTANT!=1) r.cr1|=0x1000u;
  r.cr2=stop==2?0x2000u:0;r.payload_mask=(1u<<width)-1;
  *out=r;return PU_OK;
}
int ua_apply(struct pu_driver *d,struct pu_io io,const struct ua_format *f) {
  if (!d || !f || !io.read || !io.write || !d->ready ||
      (f->cr1&~0xb600u) || !(f->cr1&0x2000u) ||
      (f->cr2!=0 && f->cr2!=0x2000u) ||
      (f->payload_mask!=127 && f->payload_mask!=255) || !f->wire.brr)
    return PU_INVALID;
  /* f is a successful ua_format result, held immutable by its caller. */
  if (d->active || d->memory_owned || d->line_owned ||
      (io.read(io.context,PU_CR1)&12u)) return PU_BUSY;
  io.write(io.context,PU_CR1,f->cr1);
  io.write(io.context,PU_CR2,f->cr2);io.write(io.context,PU_CR3,0);
  io.write(io.context,PU_BRR,f->wire.brr);
  io.write(io.context,PU_CR1,f->cr1|12u);
  return PU_OK;
}
int ua_begin(struct ua_tx *s,struct pu_io io,uint32_t gen,uint32_t token,
             const uint8_t *data,unsigned count,uint32_t now,uint32_t budget) {
  if (!s || !io.read || !io.write || !data || !count || count>16 ||
      !token || !budget || budget>=0x80000000u) return PU_INVALID;
  if (s->active) return PU_BUSY;
  if (token<=s->token || (count+7)/8>UINT32_MAX-s->byte.token) return PU_STALE;
  if ((io.read(io.context,PU_CR1)&0x360cu)!=0x200cu) return PU_INVALID;
  unsigned chunk=count>8?8:count;
  int r=pu_submit(&s->byte,io,gen,s->byte.token+1,PU_POLL,data,chunk,now,budget);
  if (r) return r;
  s->data=data;s->count=count;s->offset=0;s->chunk=chunk;s->active=1;
  s->stopping=0;s->result=PU_BUSY;s->token=token;s->deadline=now+budget;s->last=now;
  return PU_OK;
}
int ua_cancel(struct ua_tx *s,struct pu_io io,uint32_t token) {
  if (!s || !io.read || !io.write) return PU_INVALID;
  if (!s->active || s->token!=token) return PU_STALE;
  s->stopping=1;if (s->result==PU_BUSY) s->result=PU_CANCELLED;
  if (s->byte.active) return pu_cancel(&s->byte,io,s->byte.token);
  return PU_OK;
}
int ua_step(struct ua_tx *s,struct pu_io io,uint32_t token,uint32_t now) {
  if (!s || !io.read || !io.write) return PU_INVALID;
  if (!s->active || s->token!=token) return PU_STALE;
  if (now-s->last>=0x80000000u) return PU_INVALID;
  s->last=now;
  int r=pu_step(&s->byte,io,s->byte.token,now);
  if (r==PU_BUSY) return PU_BUSY;
  if (s->byte.active || s->byte.memory_owned || s->byte.line_owned) return r;
  if (r!=PU_OK || s->stopping) {
    if (s->result==PU_BUSY) s->result=(unsigned)r;
    s->active=0;return (int)s->result;
  }
  s->offset+=s->chunk;
  if (s->offset==s->count) { s->active=0;s->result=PU_OK;return PU_OK; }
  if (UA_MUTANT!=2 && now-s->deadline<0x80000000u) {
    s->active=0;s->result=PU_TIMEOUT;return PU_TIMEOUT;
  }
  s->chunk=s->count-s->offset;
  if (s->chunk>8) s->chunk=8;
  uint32_t remaining=UA_MUTANT==2?100:s->deadline-now;
  r=pu_submit(&s->byte,io,s->byte.configured_generation,s->byte.token+1,
              PU_POLL,s->data+s->offset,s->chunk,now,remaining);
  if (r) {s->active=0;s->result=(unsigned)r;return r;}
  return PU_BUSY;
}
static int rx_valid(const struct ua_rx *s) {
  return s && s->slots && s->capacity && s->capacity<=32 && s->head<s->capacity &&
    s->count<=s->capacity && s->bridge.count==0;
}
int ua_rx(struct ua_rx *s,struct pu_io io) {
  if (!rx_valid(s) || !io.read || !io.write) return PU_INVALID;
  if (s->count==s->capacity || s->bridge.errors==UINT32_MAX) return PU_FULL;
  int r=pu_rx_poll(&s->bridge,io,1);if (r) return r;
  struct pu_byte b;if (pu_take(&s->bridge,&b)) return PU_IO;
  if (UA_MUTANT==3) b.error=0;
  s->slots[(s->head+s->count)%s->capacity]=b;s->count++;
  return PU_OK;
}
int ua_take(struct ua_rx *s,struct pu_byte *out) {
  if (!rx_valid(s) || !out) return PU_INVALID;
  if (!s->count) return PU_BUSY;
  *out=s->slots[s->head];s->head=(s->head+1)%s->capacity;s->count--;return PU_OK;
}
