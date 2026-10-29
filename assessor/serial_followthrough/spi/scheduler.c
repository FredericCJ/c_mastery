#include "scheduler.h"
#ifndef SS_MUTANT
#define SS_MUTANT 0
#endif
int ss_open(struct ss_scheduler *q,struct ps_io io,const struct pu_clock *c,unsigned mode,unsigned divider){
 if(!q)return PS_INVALID;
 if(q->active || q->slots[0].state || q->slots[1].state)return PS_BUSY;
 int r=ps_open(&q->driver,io,c,mode,divider);if(r!=PS_OK)return r;
 if(!q->initialized){if(algo_ring_init(&q->pending,2)!=ALGO_OK)return PS_IO;q->initialized=1;}
 return PS_OK;
}
int ss_enqueue(struct ss_scheduler *q,uint32_t token,const uint8_t *tx,unsigned n,unsigned dma,unsigned budget,unsigned setup,unsigned hold){
 if(!q || !tx || !token || !n || n>16 || dma>1 || !budget || budget>=UINT32_C(0x80000000) || setup>=budget || hold>=budget)return PS_INVALID;
 if(!q->initialized || !q->driver.ready || token<=q->last_token)return PS_STALE;
 unsigned i=0;while(i<2 && q->slots[i].state)i++;
 if(i==2)return PS_BUSY;
 struct ss_slot s={0};s.token=token;s.state=SS_QUEUED;s.count=n;s.dma=dma;s.budget=budget;s.setup=setup;s.hold=hold;
 for(unsigned j=0;j<n;j++)s.tx[j]=tx[j];
 if(ps_pack(s.tx,s.tx_words,n)!=PS_OK)return PS_IO;
 if(algo_ring_put(&q->pending,(int32_t)i)!=ALGO_OK)return PS_IO;
 q->slots[i]=s;q->last_token=token;return PS_OK;
}
int ss_event(struct ss_scheduler *q,struct ps_io io,uint32_t token,uint32_t now){
 if(!q || !io.read || !io.write)return PS_INVALID;
 if(!q->active || q->active>2)return PS_STALE;
 struct ss_slot *s=&q->slots[q->active-1];
 if((token!=s->token && SS_MUTANT!=1) || s->state!=SS_RUNNING)return PS_STALE;
 /* Event identity is validated before converting it to the active driver's token. */
 int r=ps_step(&q->driver,io,s->token,now);
 if(r!=PS_BUSY && r!=PS_INVALID && r!=PS_STALE){
  if(q->driver.phase || q->driver.tx_owned || q->driver.rx_owned || q->driver.cs_owned)return PS_IO;
  s->result=(unsigned)r;s->state=SS_DONE;
 }
 return r;
}
int ss_run(struct ss_scheduler *q,struct ps_io io,ss_submit_fn submit,uint32_t now){
 if(!q || !io.read || !io.write || !submit)return PS_INVALID;
 if(!q->initialized)return PS_STALE;
 if(!q->active){int32_t i;int r=algo_ring_take(&q->pending,&i);if(r==ALGO_MISSING)return PS_STALE;if(r!=ALGO_OK || i<0 || i>1)return PS_IO;q->active=(unsigned)i+1;}
 struct ss_slot *s=&q->slots[q->active-1];
 if(s->state==SS_DONE)return (int)s->result;
 if(s->state==SS_QUEUED){
  int r=submit(&q->driver,io,q->driver.generation,s->token,s->tx,s->rx,s->tx_words,s->rx_words,s->count,s->dma,now,s->budget,s->setup,s->hold);
  if(r==PS_OK){s->state=SS_RUNNING;return PS_BUSY;}
  if(r!=PS_BUSY){s->state=SS_DONE;s->result=(unsigned)r;}
  return r;
 }
 return ss_event(q,io,s->token,now);
}
int ss_cancel(struct ss_scheduler *q,struct ps_io io,uint32_t token){
 if(!q || !io.read || !io.write)return PS_INVALID;
 if(!q->active || q->active>2)return PS_STALE;
 struct ss_slot *s=&q->slots[q->active-1];if(s->token!=token || s->state!=SS_RUNNING)return PS_STALE;
 return ps_cancel(&q->driver,io,token);
}
int ss_reset(struct ss_scheduler *q,struct ps_io io,uint32_t token,uint32_t now){
 if(!q || !io.read || !io.write)return PS_INVALID;
 if(!q->active || q->active>2)return PS_STALE;
 struct ss_slot *s=&q->slots[q->active-1];if(s->token!=token || s->state!=SS_RUNNING)return PS_STALE;
 return ps_abort_reset(&q->driver,io,token,now);
}
int ss_take(struct ss_scheduler *q,uint32_t token,struct ss_result *out,uint8_t bytes[16]){
 if(!q || !out || !bytes)return PS_INVALID;
 if(!q->active || q->active>2)return PS_STALE;
 struct ss_slot *s=&q->slots[q->active-1];if(s->token!=token)return PS_STALE;
 if(s->state!=SS_DONE && SS_MUTANT!=2)return PS_BUSY;
 if(q->driver.phase || q->driver.tx_owned || q->driver.rx_owned || q->driver.cs_owned){if(SS_MUTANT!=2)return PS_BUSY;}
 struct ss_result r={s->result,0,0};
 if(q->driver.token==s->token){r.sent=q->driver.sent;r.received=q->driver.received;}
 if(r.received>s->count || r.sent>s->count)return PS_IO;
 if(s->dma && r.received && ps_unpack(s->rx_words,s->rx,r.received)!=PS_OK)return PS_IO;
 for(unsigned i=0;i<r.received;i++)bytes[i]=s->rx[i];
 *out=r;s->state=SS_FREE;q->active=0;return PS_OK;
}
