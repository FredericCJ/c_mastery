#include "writer.h"
struct cwf_sink {unsigned call,used;unsigned char bytes[2];};
static ptrdiff_t write_byte(void *context,const uint8_t *p,size_t n)
{struct cwf_sink *s=context;if(n!=1)return -1;unsigned call=s->call++;if(call==0)return -2;if(call==2)return -1;s->bytes[s->used++]=*p;return 1;}
int cpw_image(void)
{
    algo_ring q;struct cwf_byte t={0};struct cwf_sink sink={0};
    if(algo_ring_init(&q,2)!=ALGO_OK || algo_ring_put(&q,65)!=ALGO_OK || algo_ring_put(&q,66)!=ALGO_OK)return 81;
    if(cwf_byte_step(&q,&t,write_byte,&sink,1)!=1 || !t.active.valid)return 82;
    if(cwf_byte_step(&q,&t,write_byte,&sink,1)<0 || t.active.valid)return 83;
    if(cwf_byte_step(&q,&t,write_byte,&sink,1)!=-1 || !t.failed || t.active.pending!=66)return 84;
    if(cwf_byte_recover(&t) || cwf_byte_step(&q,&t,write_byte,&sink,1)<0)return 85;
    if(q.count || t.active.valid || sink.used!=2 || sink.bytes[0]!=65 || sink.bytes[1]!=66 || t.error.primary!=1001)return 86;
    return 0;
}
