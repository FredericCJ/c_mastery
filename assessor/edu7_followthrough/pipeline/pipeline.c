#include "pipeline.h"
#include <limits.h>
#include <string.h>
#ifndef EW_MUTANT
#define EW_MUTANT 0
#endif
static struct ep_tx *tx(struct ew_port *p){return p->master?&p->m.tx:&p->s.tx;}
int ew_init(struct ew_node *s,unsigned node,unsigned profile,uint16_t incarnation,const struct r5_io *io,const struct pu_clock *clock,uint32_t now){
 if(!s||!io||!clock||s->node||(node!=17&&node!=34&&node!=51)||(profile!=1&&profile!=2)||!incarnation)return EW_INVALID;
 struct ew_node t={0};t.node=node;t.profile=profile;t.ports=node==34?2:1;t.last=now;
 if(node!=34&&es_init(&t.endpoint,node,profile,incarnation))return EW_INVALID;
 for(unsigned i=0;i<t.ports;i++){t.port[i].master=node==17||(node==34&&i==1);struct r5_timing timing={R5_THVD1400,0,1000,200000};if(ep_open(tx(&t.port[i]),io[i],clock,115200,&timing,now))return EW_INVALID;}
 *s=t;return EW_OK;
}
int ew_queue(struct ew_node *s,unsigned port,const struct edu_frame *f){
 if(!s||!f||port>=s->ports||f->kind||f->length>40)return EW_INVALID;
 struct ew_port *p=&s->port[port];unsigned src=p->master?1:2;
 if(f->source!=src||f->destination!=3-src)return EW_INVALID;
 if(p->queued)return EW_BUSY;
 p->queue=*f;p->queued=1;return EW_OK;
}
static int packet(struct ew_node *s,const uint8_t *p,size_t n){
 if(!p||n<6||n>40||p[4]!=(s->profile==1?4:0x84))return EW_INVALID;
 uint8_t wire[40];memcpy(wire,p,n);wire[4]=4;struct edu_datagram d;
 if(edu_network_decode(wire,n,&d)||!d.hop)return EW_INVALID;
 return EW_OK;
}
static int net(struct ew_node *s,unsigned port,const struct edu_frame *f,uint32_t now){
 if(packet(s,f->payload,f->length))return EW_INVALID;
 if(s->network==UINT32_MAX)return EW_BUSY;
 s->network++;memcpy(s->incoming,f->payload,f->length);s->incoming_length=f->length;
 if(s->node==34){
  if(s->route.active||s->route_drops==UINT32_MAX)return EW_BUSY;
  uint8_t wire[40];memcpy(wire,f->payload,f->length);wire[4]=4;uint32_t token;
  if(ef_enqueue(&s->route,wire,f->length,port,1,1,&token)){s->route_drops++;return EW_INVALID;}
  s->route_protocol=f->payload[4];return EW_OK;
 }
 if(f->payload[2]!=s->node)return EW_INVALID;
 unsigned extra=s->profile==2?2:0;
 if(f->length<14+extra)return EW_INVALID;
 unsigned type=f->payload[6+extra];int r;
 if(type==2)r=es_ack(&s->endpoint,f->payload,f->length,now/1000);
 else {
  uint8_t ack[40];size_t n=0;r=es_receive(&s->endpoint,f->payload,f->length,now/1000,ack,sizeof ack,&n);
  if(r==E7_OK||r==ET_COMPLETE){struct edu_frame reply={.source=(uint8_t)(s->port[0].master?1:2),.destination=(uint8_t)(s->port[0].master?2:1),.length=(uint8_t)n};memcpy(reply.payload,ack,n);if(ew_queue(s,0,&reply)){if(s->ack_drops==UINT32_MAX)return EW_BUSY;s->ack_drops++;}return EW_OK;}
 }
 if(r){if(s->transport_drops==UINT32_MAX)return EW_BUSY;s->transport_drops++;return EW_INVALID;}return EW_OK;
}
static int receive(struct ew_node *s,unsigned index,uint32_t now){
 struct ew_port *p=&s->port[index];struct pu_rx rx={0};struct pu_byte b;int r=pu_rx_poll(&rx,tx(p)->io.uart,1);if(r==PU_TIMEOUT)return EW_OK;if(r||pu_take(&rx,&b))return EW_INVALID;
 if(p->bytes==UINT32_MAX||p->faults==UINT32_MAX||p->drops==UINT32_MAX||p->frames==UINT32_MAX)return EW_BUSY;
 p->bytes++;if(b.error)p->faults++;struct edu_frame f;
 if(p->master){r=ep_receive(&p->m,PU_RXNE|b.error,b.value,now,&f);if(r!=EP_FRAME)return EW_OK;}
 else{
  int grant=ep_slave_receive(&p->s,PU_RXNE|b.error,b.value,now);
  if(grant==EP_OK)p->data_seen=0;
  r=ef_frame_byte(&p->parser,1,b.value,b.error!=0,&f);if(r!=1)return EW_OK;
  if(f.kind!=0)return EW_OK;
  unsigned deliver=0;if(p->s.tx.phase||p->s.granted||p->data_seen||f.source!=1||e7_link_admit(0,2,f.destination,1,0,b.error!=0,&deliver)||!deliver){p->drops++;return EW_OK;}
  p->data_seen=1;
 }
 p->frames++;if(net(s,index,&f,now))p->drops++;return EW_OK;
}
static int link(struct ew_port *p,uint32_t now,unsigned paused){
 struct ep_tx *t=tx(p);int r;
 if(p->master){
  if(p->mode==1){r=ep_step(t,t->token,now);if(r==EP_OK){r=ep_poll(&p->m,t->token+1,now);if(r)return EW_INVALID;p->mode=2;}else if(r!=EP_BUSY){p->mode=0;p->drops++;}}
  else if(p->mode){r=ep_tick(&p->m,now);if(r==EP_INVALID||r==EP_FULL)return EW_INVALID;if(!p->m.polling)p->mode=3;}
  if(!paused&&!t->phase&&!p->m.grant.phase&&now>=p->m.grant.turn_due&&p->mode!=1){
   if(t->token==UINT32_MAX)return EW_BUSY;
   if(p->queued){uint8_t bytes[96];size_t n;if(edu_frame_encode(&p->queue,bytes,sizeof bytes,&n)||ep_send(t,t->token+1,bytes,n,now,10000))return EW_INVALID;p->queued=0;p->mode=1;}
   else{if(ep_poll(&p->m,t->token+1,now))return EW_INVALID;p->mode=2;}
  }
 }else{
  if(t->phase){r=ep_step(t,t->token,now);if(r!=EP_OK&&r!=EP_BUSY&&r!=EP_DROP&&r!=EP_TIMEOUT)return EW_INVALID;}
  if(!t->phase&&p->s.granted){struct edu_frame f=p->queued?p->queue:(struct edu_frame){.kind=2,.source=2,.destination=1};if(t->token==UINT32_MAX)return EW_BUSY;r=ep_reply(&p->s,t->token+1,&f,now);if(r==EP_OK)p->queued=0;else if(r!=EP_BUSY&&r!=EP_TIMEOUT)return EW_INVALID;}
 }
 return EW_OK;
}
static int forward(struct ew_node *s){
 if(!s->route.active)return EW_OK;
 if(s->forwarded==UINT32_MAX)return EW_BUSY;
 struct edu_frame f;if(ef_peek(&s->route,s->route.generation,&f))return EW_INVALID;
 f.payload[4]=(uint8_t)s->route_protocol;
 if(EW_MUTANT==1)f.payload[3]++;
 if(EW_MUTANT==2)f.destination=f.payload[2];
 if(ew_queue(s,s->route.egress,&f))return EW_BUSY;
 s->forwarded++;memcpy(s->outgoing,f.payload,f.length);s->outgoing_length=f.length;
 return ef_release(&s->route,s->route.generation)?EW_INVALID:EW_OK;
}
int ew_tick(struct ew_node *s,uint32_t now){
 if(!s||!s->node||now<s->last||now>UINT32_MAX-60000)return EW_INVALID;
 s->last=now;
 for(unsigned i=0;i<s->ports;i++){if(link(&s->port[i],now,s->paused)||receive(s,i,now))return EW_INVALID;}
 if(s->node==34){int r=forward(s);return r==EW_INVALID?r:EW_OK;}
 if(!s->event_ready&&s->endpoint.rx.inner.ready){
  struct es_event e;int r=es_deliver(&s->endpoint,80,&e);
  if(r==ES_DELIVERED||r==ES_DUPLICATE){if(EW_MUTANT==4&&s->node==51&&e.type==3&&e.length>=12&&e.reply[3]==3){uint16_t v=(uint16_t)s->endpoint.setpoint;e.reply[10]=(uint8_t)(v>>8);e.reply[11]=(uint8_t)v;}s->event=e;s->event_ready=1;}
  else if(r==E7_BUSY&&s->endpoint.rx.inner.bytes[0]==3&&s->endpoint.core.holder!=s->endpoint.peer&&!s->endpoint.core.token_pending&&EW_MUTANT!=6){if(s->upper_drops==UINT32_MAX)return EW_BUSY;s->upper_drops++;(void)et_release(&s->endpoint.rx,s->endpoint.rx.identity.epoch,s->endpoint.rx.inner.completed);}
  else if(r!=E7_BUSY&&r!=E7_FULL){if(s->upper_drops==UINT32_MAX)return EW_BUSY;s->upper_drops++;}
 }
 if(!s->paused&&s->endpoint.tx.inner.active){
  uint8_t bytes[40];size_t n;uint32_t token;int r=es_offer(&s->endpoint,now/1000,bytes,sizeof bytes,&n,&token);
  if(!r){struct edu_frame f={.source=(uint8_t)(s->port[0].master?1:2),.destination=(uint8_t)(s->port[0].master?2:1),.length=(uint8_t)n};memcpy(f.payload,bytes,n);unsigned accepted=ew_queue(s,0,&f)==EW_OK;
   r=es_admit(&s->endpoint,s->endpoint.tx.identity.epoch,token,accepted,now/1000);if(r!=E7_OK&&r!=E7_BUSY)return EW_INVALID;
  }else if(r!=E7_WAIT&&r!=E7_BUSY&&r!=E7_TIMEOUT)return EW_INVALID;
 }
 return EW_OK;
}
int ew_take(struct ew_node *s,struct es_event *e){if(!s||!e||s->node==34)return EW_INVALID;if(!s->event_ready)return EW_BUSY;*e=s->event;s->event_ready=0;return EW_OK;}
int ew_physical_verdict(unsigned software_ok,unsigned mask){if(software_ok>1||mask>127)return EW_INVALID;return software_ok&&(mask==127||EW_MUTANT==3)?EW_OK:EW_BUSY;}

int ew_pause(struct ew_node *s){
 if(!s||!s->node)return EW_INVALID;
 if(s->route.active||s->event_ready||s->endpoint.tx.inner.active||s->endpoint.outgoing_kind||s->endpoint.rx.inner.active||s->endpoint.rx.inner.ready)return EW_BUSY;
 for(unsigned i=0;i<s->ports;i++)if(s->port[i].queued)return EW_BUSY;
 s->paused=1;return EW_OK;
}
int ew_drained(const struct ew_node *s){
 if(!s||!s->paused||s->route.active||s->event_ready||s->endpoint.tx.inner.active||s->endpoint.outgoing_kind||s->endpoint.rx.inner.active||s->endpoint.rx.inner.ready)return 0;
 for(unsigned i=0;i<s->ports;i++){const struct ew_port *p=&s->port[i];if(p->queued||(p->master?(p->m.tx.phase||p->m.grant.phase):p->s.tx.phase||p->s.granted))return 0;}
 return 1;
}
int ew_rebind(struct ew_node *s,uint16_t incarnation){
 if(!s||!incarnation||s->profile!=2||s->node==34)return EW_INVALID;
 if(!ew_drained(s)&&EW_MUTANT!=5)return EW_BUSY;
 struct es_peer t=s->endpoint;int r=et_tx_open(&t.tx,2,incarnation,s->node);if(r)return r;r=et_rx_open(&t.rx,2,incarnation,s->node);if(r)return r;
 s->endpoint=t;return EW_OK;
}
int ew_run(struct ew_node *s){if(!s)return EW_INVALID;if(!ew_drained(s))return EW_BUSY;s->paused=0;return EW_OK;}
