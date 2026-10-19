#include "writer.h"
typedef struct {CWFByte *state;FWriter writer;void *context;} Bridge;
static ptrdiff_t byte_write(void *context,const uint8_t *p,size_t n)
{
    Bridge *b=context;ptrdiff_t r=b->writer(b->context,p,n);++b->state->calls;
    if(r>0 && (size_t)r<=n)b->state->error.accepted+=(size_t)r;
    if(r<0 && r!=-2 && !b->state->error.primary)b->state->error.primary=1001;
    return r;
}
int cwf_byte_step(algo_ring *q,CWFByte *s,FWriter write,void *context,unsigned budget)
{
    int r;size_t before;Bridge b;
    if(!q || !s || !write)return -1;
    if(s->failed)return -1;
    s->error.operation=42;before=s->error.accepted;b=(Bridge){s,write,context};r=in_drain(q,&s->active,byte_write,&b,budget);
    if(r<0)s->failed=1;
#ifdef CWF_MUTANT_REQUEUE
    if(!s->requeued && s->error.accepted>before && !s->active.valid){if(algo_ring_put(q,s->active.pending)!=ALGO_OK)return -1;s->requeued=1;}
#else
    (void)before;
#endif
    return r;
}
int cwf_byte_recover(CWFByte *s)
{if(!s || !s->failed)return -1;s->failed=0;++s->recoveries;return 0;}
int cwf_span_begin(CWFSpan *s,const unsigned char *p,size_t n,unsigned operation)
{
    if(!s || s->active || (!p && n) || n>sizeof s->bytes)return -1;
    for(size_t i=0;i<n;++i)s->bytes[i]=p[i];
    s->count=n;s->active=1;return cpw_begin(&s->transfer,s->bytes,n,operation)==CPW_INVALID?-1:0;
}
int cwf_span_take(algo_ring *q,CWFSpan *s,size_t n,unsigned operation)
{
    algo_ring candidate;unsigned char bytes[16];
    if(!q || !s || s->active || n>sizeof bytes || n>q->count)return -1;
    candidate=*q;
    for(size_t i=0;i<n;++i){int32_t v;if(algo_ring_take(&candidate,&v)!=ALGO_OK || v<0 || v>255)return -1;bytes[i]=(unsigned char)v;}
    if(cwf_span_begin(s,bytes,n,operation))return -1;
    *q=candidate;return 0;
}
CPWStatus cwf_span_step(CWFSpan *s,CPWWrite write,void *context)
{
    CPWStatus status;if(!s || !s->active || !write)return CPW_INVALID;
    status=cpw_step(&s->transfer,write,context);
#ifdef CWF_MUTANT_SUFFIX
    if(status==CPW_FAILED)s->transfer.accepted=0;
#endif
    if(status==CPW_DONE)s->active=0;
    return status;
}
CPWStatus cwf_span_recover(CWFSpan *s)
{return s && s->active?cpw_recover(&s->transfer):CPW_INVALID;}
int cwf_error(const CPWTransfer *t,CWFError *out)
{
    if(!t || !out || (t->status!=CPW_PENDING && t->status!=CPW_DONE && t->status!=CPW_FAILED))return -1;
    *out=(CWFError){t->status==CPW_PENDING?CWF_WAIT:t->status==CPW_DONE?CWF_COMPLETE:CWF_IO,t->error.operation,t->error.accepted,t->error.primary,t->error.cleanup};return 0;
}
