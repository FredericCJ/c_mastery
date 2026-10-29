#include "transport.h"
#include <limits.h>
#include <string.h>
#ifndef ET_MUTANT
#define ET_MUTANT 0
#endif
static int identity(struct et_identity *i,unsigned profile,uint16_t epoch,unsigned node){
 if((profile!=1&&profile!=2)||!epoch||(node!=17&&node!=51))return E7_INVALID;
 *i=(struct et_identity){epoch,(uint8_t)profile,(uint8_t)node,(uint8_t)(node==17?51:17),(uint8_t)(node==17?7:8),(uint8_t)(node==17?8:7)};return E7_OK;
}
static int reopen(const struct et_identity *old,unsigned profile,uint16_t epoch){return epoch>old->epoch&&(!old->epoch||profile==2);}
int et_tx_open(struct et_tx *s,unsigned profile,uint16_t epoch,unsigned node){
 struct et_identity i;if(!s||identity(&i,profile,epoch,node))return E7_INVALID;
 if(s->inner.active||s->inner.offered)return E7_BUSY;
 if(!reopen(&s->identity,profile,epoch))return E7_STALE;
 uint32_t token=s->inner.offer_token,last=s->inner.last;s->inner=(struct e7_sender){0};s->inner.offer_token=token;s->inner.last=last;s->identity=i;s->peer_credit=1;return E7_OK;
}
int et_rx_open(struct et_rx *s,unsigned profile,uint16_t epoch,unsigned node){
 struct et_identity i;if(!s||identity(&i,profile,epoch,node))return E7_INVALID;
 if(s->inner.active||s->inner.ready)return E7_BUSY;
 if(!reopen(&s->identity,profile,epoch))return E7_STALE;
 unsigned publications=s->inner.publications;s->inner=(struct edu_rx){0};s->inner.publications=publications;s->identity=i;s->retired=0;return E7_OK;
}
static int wrap(const struct et_identity *i,const struct edu_fragment *f,uint8_t *out,size_t cap,size_t *length){
 uint8_t fragment[32],wire[40];size_t n=0,total=0;
 if(edu_fragment_encode(f,fragment,&n))return E7_INVALID;
 struct edu_datagram d={.source=i->node,.destination=i->peer,.hop=4};
 unsigned extra=i->profile==2?2:0;d.length=(uint8_t)(n+extra);
 if(extra){d.payload[0]=(uint8_t)(i->epoch>>8);d.payload[1]=(uint8_t)i->epoch;}
 memcpy(d.payload+extra,fragment,n);if(edu_network_encode(&d,wire,&total))return E7_INVALID;
 wire[4]=i->profile==1?4:0x84;if(cap<total&&ET_MUTANT!=8)return E7_FULL;memcpy(out,wire,total);*length=total;return E7_OK;
}
static int unwrap(const struct et_identity *i,const uint8_t *p,size_t n,struct edu_fragment *out){
 if(!p||n<6||n>40||p[4]!=(i->profile==1?4:0x84))return E7_REJECTED;
 uint8_t bytes[40];memcpy(bytes,p,n);bytes[4]=4;struct edu_datagram d;
 if(edu_network_decode(bytes,n,&d)||d.source!=i->peer||d.destination!=i->node||!d.hop)return E7_REJECTED;
 unsigned extra=i->profile==2?2:0;
 if(d.length<extra)return E7_REJECTED;
 if(extra&&ET_MUTANT!=1&&((unsigned)d.payload[0]*256+d.payload[1])!=i->epoch)return E7_STALE;
 struct edu_fragment f;if(edu_fragment_decode(d.payload+extra,d.length-extra,&f)||f.source_port!=i->peer_port||f.destination_port!=i->port)return E7_REJECTED;
 *out=f;return E7_OK;
}
static struct edu_fragment convert(const struct e7_fragment *f){struct edu_fragment g={f->type,f->source,f->destination,f->index,f->count,f->credit,f->id,{0},f->length};memcpy(g.data,f->data,f->length);return g;}
static struct e7_fragment convert_ack(const struct edu_fragment *f){struct e7_fragment a={0};a.type=f->type;a.source=f->source_port;a.destination=f->destination_port;a.id=f->id;a.index=f->index;a.count=f->count;a.credit=f->credit;a.length=(uint8_t)f->length;memcpy(a.data,f->data,f->length);return a;}
int et_begin(struct et_tx *s,uint16_t id,const uint8_t *p,size_t n,uint32_t now){
 if(!s||!s->identity.epoch)return E7_INVALID;
 int r=e7_send_begin(&s->inner,id,s->identity.port,s->identity.peer_port,p,n,now);
 if(!r&&!s->peer_credit){s->inner.credit=0;s->inner.blocked=1;s->inner.blocked_since=now;s->inner.due=now+100;}
 return r;
}
int et_offer(struct et_tx *s,uint32_t now,uint8_t *out,size_t cap,size_t *n,uint32_t *token){
 if(!s||!out||!n||!token||!s->identity.epoch)return E7_INVALID;
 struct e7_sender trial=s->inner;struct e7_fragment f;uint32_t t;int r=e7_send_offer(&trial,now,&f,&t);
 if(r){if(r==E7_TIMEOUT)s->inner=trial;return r;}
 struct edu_fragment g=convert(&f);r=wrap(&s->identity,&g,out,cap,n);if(r)return r;
 if(ET_MUTANT==2){trial.due=now+100;trial.waiting=1;}
 s->inner=trial;*token=t;return E7_OK;
}
int et_accept(struct et_tx *s,uint16_t epoch,uint32_t token,unsigned accepted,uint32_t now){
 if(!s||!s->identity.epoch)return E7_INVALID;
 if(epoch!=s->identity.epoch)return E7_STALE;
 int r=e7_send_accept(&s->inner,token,accepted,now);if(ET_MUTANT==9&&!accepted&&r==E7_BUSY)s->inner.retries++;return r;
}
int et_ack(struct et_tx *s,const uint8_t *p,size_t n,uint32_t now){
 if(!s||!s->identity.epoch)return E7_INVALID;
 struct edu_fragment f;int r=unwrap(&s->identity,p,n,&f);if(r)return r;
 struct e7_fragment a=convert_ack(&f);r=e7_send_ack(&s->inner,&a,now);if(!r)s->peer_credit=f.credit;return r;
}
int et_expire(struct et_rx *s,uint32_t now){
 if(!s||!s->identity.epoch||now<s->last||now>UINT32_MAX-500)return E7_INVALID;
 if(s->inner.active&&now-s->started>=500){
  if(s->expired==UINT32_MAX)return E7_FULL;
  if(ET_MUTANT!=3)s->retired=s->inner.id;
  s->inner.active=0;s->inner.length=0;s->inner.index=0;s->expired++;s->last=now;return E7_TIMEOUT;
 }
 s->last=now;return E7_OK;
}
int et_receive(struct et_rx *s,const uint8_t *p,size_t n,uint32_t now,uint8_t *ack,size_t cap,size_t *ack_length){
 if(!s||!p||!ack||!ack_length)return E7_INVALID;
 struct et_rx trial=*s;int r=et_expire(&trial,now);if(r){if(r==E7_TIMEOUT)*s=trial;return r;}
 struct edu_fragment f,a;r=unwrap(&trial.identity,p,n,&f);if(r)return r;
 if(f.type==1&&f.id<=trial.retired&&f.id!=trial.inner.completed)return E7_STALE;
 if(f.type==1&&f.id>trial.inner.completed&&trial.inner.publications==UINT_MAX)return E7_FULL;
 unsigned active=trial.inner.active;r=edu_rx_accept(&trial.inner,&f,&a);if(r<0)return E7_REJECTED;
 if(trial.inner.active&&(!active||ET_MUTANT==4))trial.started=now;
 if(ET_MUTANT==6&&r==0&&f.type==1&&f.id==trial.inner.completed)trial.inner.publications++;
 if(ET_MUTANT==5&&r==1)a.credit=1;
 int w=wrap(&trial.identity,&a,ack,cap,ack_length);if(w)return w;
 *s=trial;return r==1?ET_COMPLETE:E7_OK;
}
int et_peek(const struct et_rx *s,uint8_t *out,size_t cap,size_t *n){
 if(!s||!out||!n||!s->identity.epoch)return E7_INVALID;
 if(!s->inner.ready&&ET_MUTANT!=7)return E7_WAIT;
 if(cap<s->inner.length)return E7_FULL;
 memcpy(out,s->inner.bytes,s->inner.length);*n=s->inner.length;return E7_OK;
}
int et_release(struct et_rx *s,uint16_t epoch,uint16_t id){
 if(!s)return E7_INVALID;
 if(epoch!=s->identity.epoch||!s->inner.ready||id!=s->inner.completed)return E7_STALE;
 edu_rx_release(&s->inner);return E7_OK;
}
