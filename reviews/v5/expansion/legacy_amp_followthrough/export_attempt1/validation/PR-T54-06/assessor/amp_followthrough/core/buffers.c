#include "buffers.h"
int af_buffers_init(AfBuffers *s,const XdFence *f){
 if(!s||!f||s->transfer.last_token||s->transfer.phase||s->slot[0].phase||s->slot[1].phase||s->slot[0].lease.epoch||s->slot[1].lease.epoch)return AF_BUSY;
 AfBuffers t={0};if(xd_recover(&t.slot[0].lease,f)!=XD_OK||xd_recover(&t.slot[1].lease,f)!=XD_OK)return AF_INVALID;
 *s=t;return AF_OK;
}
int af_fill(AfBuffers *s,unsigned i){if(!s||i>1)return AF_INVALID;if(s->slot[i].phase!=AF_FREE)return AF_BUSY;s->slot[i].phase=AF_FILLING;return AF_OK;}
int af_submit(AfBuffers *s,PdIo io,unsigned i,const PdExtent *extent){
 if(!s||i>1||!extent)return AF_INVALID;
 uint32_t base=UINT32_C(0x30042000)+128*i;
 if(extent->source!=base||extent->destination!=base+64||extent->bytes!=64||extent->source_capacity!=64||extent->destination_capacity!=64)return AF_INVALID;
 if(s->transfer.phase||s->slot[i].phase!=AF_FILLING)return AF_BUSY;
 if(s->transfer.last_token==UINT32_MAX)return AF_EXHAUSTED;
 AfBuffer b=s->slot[i];b.lease.generation=s->transfer.last_token;
 if(xd_submit(&b.lease,extent->bytes,&b.ticket)!=XD_OK)return AF_INVALID;
 int r=pd_submit(&s->transfer,io,PD_H745,1,extent,1,b.ticket.generation);
 if(r!=PD_OK)return r==PD_BUSY?AF_BUSY:AF_INVALID;
 b.phase=AF_DEVICE;s->slot[i]=b;s->active=i;return AF_OK;
}
int af_timeout(AfBuffers *s,PdIo io){
 if(!s||!s->transfer.phase)return AF_INVALID;
 AfBuffer *b=&s->slot[s->active];int r=xd_pd_cancel(&b->lease,b->ticket,&s->transfer,io);
 return r==XD_OK||r==XD_DUPLICATE?AF_BUSY:AF_INVALID;
}
int af_complete(AfBuffers *s,PdIo io){
 if(!s||!s->transfer.phase)return AF_INVALID;
 AfBuffer *b=&s->slot[s->active];int r=xd_pd_finish(&b->lease,b->ticket,&s->transfer,io);
 if(r!=XD_OK)return r==XD_BUSY?AF_BUSY:AF_INVALID;
 if(xd_reclaim(&b->lease,b->ticket,&b->result)!=XD_OK)return AF_INVALID;
 b->phase=AF_COMPLETED;return AF_OK;
}
int af_consume(AfBuffers *s,unsigned i){
 if(!s||i>1)return AF_INVALID;
 AfBuffer *b=&s->slot[i];if(b->phase!=AF_COMPLETED)return AF_BUSY;
 if(b->result.terminal!=XD_COMPLETE)return AF_INVALID;
 b->phase=AF_CONSUMING;return AF_OK;
}
int af_release(AfBuffers *s,unsigned i){
 if(!s||i>1)return AF_INVALID;
 AfBuffer *b=&s->slot[i];if((b->phase!=AF_CONSUMING&&b->phase!=AF_COMPLETED)||!b->lease.quiescent)return AF_BUSY;
 b->phase=AF_FREE;return AF_OK;
}
