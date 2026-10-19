#include "durable.h"
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
int cpx_sink_accept(CPXSink *s,CPQueue *q){
 if(!s || !s->storage || !q)return SDR_INVALID;
 if(s->active)return SDR_CONFLICT;
 if(!q->count)return SDR_STALE;
 const CPMessage *m=&q->items[q->head];if(!m->sequence || m->length!=4)return SDR_INVALID;
 if(cp_take(q,&s->held)!=CP_OK)return SDR_STORAGE;
 s->active=1;s->durable=0;s->last_error=0;return SDR_OK;
}
int cpx_sink_commit(CPXSink *s){
 if(!s || !s->active || !s->storage)return SDR_INVALID;
 if(s->durable)return SDR_OK;
 const unsigned char *p=s->held.payload;uint32_t raw=(uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;
 int32_t delta=raw<=INT32_MAX?(int32_t)raw:INT32_MIN+(int32_t)(raw-UINT32_C(0x80000000));
 int32_t result=0;int status=sdr_apply(s->storage,1,1,s->held.sequence,delta,&result);s->last_error=status;
 if(status==SDR_OK || CPX_MUTANT==9){s->result=result;s->durable=1;}
 if(status!=SDR_OK && CPX_MUTANT==10)s->active=0;
 return status;
}
int cpx_sink_acknowledge(CPXSink *s,int32_t *out){if(!s || !out || !s->active)return SDR_INVALID;if(!s->durable)return SDR_UNCERTAIN;*out=s->result;s->active=0;s->durable=0;return SDR_OK;}
