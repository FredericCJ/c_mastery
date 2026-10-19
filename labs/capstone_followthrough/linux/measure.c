#define _POSIX_C_SOURCE 200809L
#include "service.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>
typedef struct {size_t bytes,calls;uint32_t sum;} Count;
static CsIO sink(void *p,const unsigned char *b,size_t n){Count *c=p;for(size_t i=0;i<n;++i)c->sum+=b[i];c->bytes+=n;++c->calls;return(CsIO){n,CS_IO_PROGRESS,0};}
static int drain(CsService *s,Count *c){while(s->count){if(cs_begin(s)!=CS_OK || cs_step(s,sink,c)!=CS_DONE)return 1;}return 0;}
static uint64_t nanos(struct timespec t){return(uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
int main(void){uint32_t expected=0;
    for(unsigned repetition=0;repetition<9;++repetition)for(unsigned order=0;order<2;++order){unsigned cap=((repetition+order)%2)?3:1;CsService s;if(cs_init(&s,cap,10000)!=CS_OK)return 1;Count c={0};CPMessage m={0,32,{0}};for(unsigned i=0;i<32;++i)m.payload[i]=(unsigned char)i;
        struct timespec start,end;if(clock_gettime(CLOCK_MONOTONIC,&start))return 2;
        for(unsigned i=0;i<6000;++i){m.sequence=(uint16_t)i;if(s.count==s.capacity && drain(&s,&c))return 3;if(cs_offer(&s,&m,1,0,0)!=CS_FRAME)return 4;}
        if(drain(&s,&c) || clock_gettime(CLOCK_MONOTONIC,&end))return 5;
        if(c.bytes!=252000 || c.calls!=6000 || s.delivered!=6000 || !cs_conserved(&s))return 6;
        if(!repetition && !order)expected=c.sum;else if(c.sum!=expected)return 7;
        printf("CAP_SAMPLE repetition=%u capacity=%u frames=6000 bytes=%zu callbacks=%zu checksum=%u ns=%llu\n",repetition,cap,c.bytes,c.calls,c.sum,(unsigned long long)(nanos(end)-nanos(start)));
    }
    return 0;
}
