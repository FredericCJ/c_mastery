#include "writer.h"
struct cwf_bridge {struct cwf_byte *state;f_writer_fn writer;void *context;};
static ptrdiff_t byte_write(void *context,const uint8_t *p,size_t n)
{
    struct cwf_bridge *b=context;ptrdiff_t r=b->writer(b->context,p,n);++b->state->calls;
    if(r>0 && (size_t)r<=n)b->state->error.accepted+=(size_t)r;
    if(r<0 && r!=-2 && !b->state->error.primary)b->state->error.primary=1001;
    return r;
}
int cwf_byte_step(algo_ring *q,struct cwf_byte *s,f_writer_fn write,void *context,unsigned budget)
{
    int r;size_t before;struct cwf_bridge b;
    if(!q || !s || !write)return -1;
    if(s->failed)return -1;
    s->error.operation=42;before=s->error.accepted;b=(struct cwf_bridge){s,write,context};r=in_drain(q,&s->active,byte_write,&b,budget);
    if(r<0)s->failed=1;
#ifdef CWF_MUTANT_REQUEUE
    if(!s->requeued && s->error.accepted>before && !s->active.valid){if(algo_ring_put(q,s->active.pending)!=ALGO_OK)return -1;s->requeued=1;}
#else
    (void)before;
#endif
    return r;
}
int cwf_byte_recover(struct cwf_byte *s)
{if(!s || !s->failed)return -1;s->failed=0;++s->recoveries;return 0;}
int cwf_span_begin(struct cwf_span *s,const unsigned char *p,size_t n,unsigned operation)
{
    if(!s || s->active || (!p && n) || n>sizeof s->bytes)return -1;
    for(size_t i=0;i<n;++i)s->bytes[i]=p[i];
    s->count=n;s->active=1;return cpw_begin(&s->transfer,s->bytes,n,operation)==CPW_INVALID?-1:0;
}
int cwf_span_take(algo_ring *q,struct cwf_span *s,size_t n,unsigned operation)
{
    algo_ring candidate;unsigned char bytes[16];
    if(!q || !s || s->active || n>sizeof bytes || n>q->count)return -1;
    candidate=*q;
    for(size_t i=0;i<n;++i){int32_t v;if(algo_ring_take(&candidate,&v)!=ALGO_OK || v<0 || v>255)return -1;bytes[i]=(unsigned char)v;}
    if(cwf_span_begin(s,bytes,n,operation))return -1;
    *q=candidate;return 0;
}
enum cpw_status cwf_span_step(struct cwf_span *s,cpw_write_fn write,void *context)
{
    enum cpw_status status;if(!s || !s->active || !write)return CPW_INVALID;
    status=cpw_step(&s->transfer,write,context);
#ifdef CWF_MUTANT_SUFFIX
    if(status==CPW_FAILED)s->transfer.accepted=0;
#endif
    if(status==CPW_DONE)s->active=0;
    return status;
}
enum cpw_status cwf_span_recover(struct cwf_span *s)
{return s && s->active?cpw_recover(&s->transfer):CPW_INVALID;}
int cwf_error(const struct cpw_transfer *t,struct cwf_error *out)
{
    if(!t || !out || (t->status!=CPW_PENDING && t->status!=CPW_DONE && t->status!=CPW_FAILED))return -1;
    *out=(struct cwf_error){t->status==CPW_PENDING?CWF_WAIT:t->status==CPW_DONE?CWF_COMPLETE:CWF_IO,t->error.operation,t->error.accepted,t->error.primary,t->error.cleanup};return 0;
}
