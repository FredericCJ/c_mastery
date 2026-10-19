#include "protocol_follow.h"
#include <limits.h>
#ifndef PF_MUTANT
#define PF_MUTANT 0
#endif
static int start(void *ctx,const unsigned char *p,size_t n)
{PFDriver *d=ctx;(void)p;(void)n;return fw_dma(&d->dma,d->io,0,0);}
static int stop(void *ctx)
{
 PFDriver *d=ctx;
 if(!d->old_epoch_idle && PF_MUTANT!=1)return 1;
 if(d->dma.state && fw_dma(&d->dma,d->io,2,d->dma.generation))return 1;
 return 0;
}
static void watch(void *ctx)
{PFDriver *d=ctx;(void)fw_watchdog(&d->watchdog,d->io,1,0);}
static int token(PFRecovery *r)
{return r&&r->owner&&r->owner->owned&&r->epoch==r->owner->loan_epoch&&(PF_MUTANT==5||r->ticket==r->owner->dma.generation);}
static void release(PFDriver *d,PFRecovery *r)
{unsigned char *p=d->owned;d->owned=0;d->length=0;r->owner=0;d->release(d->free_context,p);}
int pf_driver_init(PFDriver *d,FwIo io,uint32_t epoch,PFFree free_fn,void *ctx)
{
 if(!d||!io.read||!io.write||!epoch||!free_fn)return PT_INVALID;
 *d=(PFDriver){0};d->io=io;d->epoch=epoch;d->release=free_fn;d->free_context=ctx;d->protocol.port=(PTPort){d,start,stop,watch};d->watchdog.required=1;
 PTClosed c;PTReady r;pt_device_init(&d->protocol.device,&c);return pt_configure(&c,0,&r);
}
int pf_driver_start(PFDriver *d,unsigned char **owned,size_t n,unsigned fail_after_start,PFRecovery *out)
{
 if(!d||!owned||!*owned||!out||!n||n>64||fail_after_start>1)return PT_INVALID;
 if(d->owned||d->reset_pending||d->protocol.device.state!=PT_READY)return PT_STATE;
 if(d->dma.generation==UINT32_MAX||d->protocol.device.generation==UINT32_MAX)return PT_RANGE;
 d->owned=*owned;d->length=n;d->loan_epoch=d->epoch;d->old_epoch_idle=0;*owned=0;
 int r=pt_driver_start(&d->protocol,d->owned,n);
 *out=(PFRecovery){d,d->loan_epoch,d->dma.generation};
 if(fail_after_start||r){d->protocol.device.state=PT_RECOVERY_REQUIRED;return PT_RECOVERY;}
 return PT_OK;
}
int pf_driver_ready(PFDriver *d)
{
 if(!d||d->owned||d->reset_pending)return PT_STATE;
 PTClosed closed={&d->protocol.device,d->protocol.device.generation};PTReady ready;
 return pt_configure(&closed,0,&ready);
}
int pf_driver_reset(PFRecovery *r)
{
 if(!token(r))return PT_STATE;PFDriver *d=r->owner;
 d->reset_pending=1;d->protocol.device.state=PT_RECOVERY_REQUIRED;
 if(d->dma.state)(void)fw_dma(&d->dma,d->io,1,d->dma.generation);
 return PT_PENDING;
}
int pf_driver_recover(PFRecovery *r,uint32_t epoch,unsigned idle)
{
 if(!token(r)||idle>1)return PT_STATE;PFDriver *d=r->owner;
 if(epoch!=d->loan_epoch)return PT_STATE;
 if(d->protocol.device.state!=PT_RECOVERY_REQUIRED)return PT_STATE;
 d->old_epoch_idle=idle;int result=pt_driver_recover(&d->protocol);
 if(result)return result;
 if(d->reset_pending){if(d->epoch==UINT32_MAX)d->reset_pending=2;else{++d->epoch;d->reset_pending=0;}}
 release(d,r);return PT_OK;
}
int pf_driver_complete(PFRecovery *r,uint32_t epoch,unsigned idle)
{
 if(!token(r)||idle>1)return PT_STATE;PFDriver *d=r->owner;
 if(epoch!=d->loan_epoch)return PT_STATE;
 if(!idle)return PT_PENDING;
 if(fw_dma(&d->dma,d->io,2,d->dma.generation))return PT_PENDING;
 int result=pt_driver_complete(&d->protocol,0,1);
 if(result!=PT_OK)return result;
 release(d,r);return PT_OK;
}
int pf_builder_mode(PFBuilder *b,unsigned mode,unsigned callback)
{
 if(!b||mode>PF_DMA||callback>1)return PT_INVALID;
 if(b->mode_set)return PT_STATE;
 if((mode==PF_DMA&&!callback)||(mode==PF_POLL&&callback))return PT_RANGE;
 b->mode=mode;b->callback=callback;b->mode_set=1;return PT_OK;
}
int pf_builder_replace(PFBuilder *b,unsigned field,unsigned value)
{
 if(!b||field>2||!(b->base.mask&(1u<<field)))return PT_STATE;
 PFBuilder copy=*b;copy.base.mask&=~(1u<<field);int r=pt_builder_set(&copy.base,field,value);
 if(r)return r;
 if(PF_MUTANT==6)*b=copy;
 if(copy.base.mask==7u&&copy.mode_set){PFConfig out;r=pf_builder_finish(&copy,&out);if(r)return r;}
 *b=copy;return PT_OK;
}
int pf_builder_finish(const PFBuilder *b,PFConfig *out)
{
 if(!b||!out)return PT_INVALID;
 if(!b->mode_set)return PT_STATE;
 PTConfig c;int r=pt_builder_finish(&b->base,&c);if(r)return r;
 if(PF_MUTANT!=2 && ((b->mode==PF_DMA&&((c.address&15u)||(c.length&15u)||!b->callback))||(b->mode==PF_POLL&&b->callback)))return PT_RANGE;
 *out=(PFConfig){c,b->mode,b->callback};return PT_OK;
}
int pf_session(PFSession *s,unsigned event,uint32_t sequence,int32_t value)
{
 if(!s||event>PF_RECONNECT||s->phase>2||s->closed>1||s->crashed>1)return PT_INVALID;
 if(s->closed)return PT_STATE;
 if(event==PF_CANCEL){if(PF_MUTANT!=7)s->closed=1;return PT_OK;}
 if(event==PF_CRASH){s->crashed=1;return PT_OK;}
 if(event==PF_RECONNECT){if(!s->crashed)return PT_STATE;s->crashed=0;if(s->phase)s->phase=1;return PT_OK;}
 if(s->crashed)return PT_PENDING;
 if(event==PF_SEND){if(s->phase||!s->next||s->next==UINT32_MAX||sequence!=s->next)return PT_STATE;s->pending=sequence;s->pending_value=value;s->phase=1;return PT_OK;}
 if(!s->phase||sequence!=s->pending)return PT_STATE;
 if(event==PF_RETRY){s->phase=1;return PT_OK;}
 if(event==PF_APPLY){
  if(s->phase!=1||value!=s->pending_value)return PT_STATE;
  if(PF_MUTANT!=3 && s->last==sequence){if(s->last_value!=value)return PT_STATE;}
  else {int64_t sum=(int64_t)s->total+value;if(sum<INT32_MIN||sum>INT32_MAX)return PT_RANGE;s->total=(int32_t)sum;s->last=sequence;s->last_value=value;}
  s->phase=2;return PT_OK;
 }
 if(event==PF_ACK){if(s->phase!=2)return PT_STATE;++s->next;s->phase=0;return PT_OK;}
 return PT_STATE;
}
int pf_decoder_init(PFDecoder *d,uint64_t deadline,PFAlloc allocate,PFFree free_fn,void *ctx)
{if(!d||!allocate||!free_fn||!deadline)return PT_INVALID;*d=(PFDecoder){.deadline=deadline,.allocate=allocate,.release=free_fn,.context=ctx};return PT_OK;}
static void drop(PFDecoder *d)
{if(d->buffer){unsigned char *p=d->buffer;d->buffer=0;if(PF_MUTANT!=8)d->release(d->context,p);}}
int pf_decoder_cancel(PFDecoder *d)
{if(!d)return PT_INVALID;if(d->pc==2||d->pc==3)return PT_DONE;drop(d);d->pc=3;return PT_DONE;}
/* Two separately resumable children share no automatic local across returns. */
static int header(PFDecoder *d,unsigned char byte)
{if(byte>8)return PT_RANGE;unsigned char *p=d->allocate(d->context,byte?byte:1);if(!p)return PT_RANGE;d->buffer=p;d->length=byte;d->header_pc=1;return PT_DONE;}
static int body(PFDecoder *d,unsigned char byte)
{if(d->used<d->length){d->buffer[d->used++]=byte;d->checksum=(unsigned char)(d->checksum+byte);d->body_pc=d->used==d->length?1:0;return PT_PENDING;}if(byte!=d->checksum)return PT_IO;d->body_pc=2;return PT_DONE;}
int pf_decoder_step(PFDecoder *d,unsigned available,unsigned char byte,uint64_t now,unsigned char **out,size_t *length)
{
 if(!d||!out||!length||available>1||now<d->last_time)return PT_INVALID;
 if(d->pc==2)return PT_DONE;if(d->pc==3)return PT_STATE;
 if(PF_MUTANT!=4 && now>=d->deadline){drop(d);d->pc=3;d->last_time=now;return PT_IO;}
 if(!available){d->last_time=now;return PT_PENDING;}
 int r;if(!d->header_pc){r=header(d,byte);if(r!=PT_DONE)return r;d->pc=1;d->last_time=now;return PT_PENDING;}
 r=body(d,byte);d->last_time=now;
 if(r==PT_IO){drop(d);d->pc=3;return r;}
 if(r==PT_DONE){*out=d->buffer;*length=d->length;d->buffer=0;d->pc=2;}
 return r;
}
