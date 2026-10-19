#include "admission.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPA_ASSERT:%s\n",#x);return 41;}}while(0)
int main(int argc,char **argv)
{
    if(argc!=5)return 2;
    size_t chars=strlen(argv[1]),chunk=(size_t)strtoul(argv[2],NULL,10),capacity=(size_t)strtoul(argv[3],NULL,10),pause=(size_t)strtoul(argv[4],NULL,10);
    if(chars%2 || chars>512 || !chunk || chunk>256 || !capacity || capacity>CP_QUEUE || pause>256)return 2;
    unsigned char input[256];size_t n=chars/2;for(size_t i=0;i<n;++i){unsigned b;if(sscanf(argv[1]+2*i,"%2x",&b)!=1)return 2;input[i]=(unsigned char)b;}
    CPParser p;CPQueue q;cp_parser_init(&p);cp_queue_init(&q);
    for(size_t i=0;i<capacity;++i){p.ready=1;p.message.sequence=(uint16_t)(900+i);p.message.length=0;CHECK(cpa_admit(&p,&q,capacity)==CP_OK);}
    size_t offset=0,refused=0,peak=q.count,bound=8*(n+CP_WIRE+pause+capacity+1);
    for(size_t turn=0;turn<bound;++turn){
        if(turn>=pause){CPMessage out;if(cp_take(&q,&out)==CP_OK){printf("%u:",(unsigned)out.sequence);for(size_t i=0;i<out.length;++i)printf("%02x",out.payload[i]);putchar('\n');}}
        size_t offer=n-offset;if(offer>chunk)offer=chunk;size_t used=999;CPStatus status=cp_feed(&p,input+offset,offer,&used);CHECK(used<=offer && p.used<=CP_WIRE);offset+=used;
        if(status==CP_FRAME){CPMessage before;CHECK(cp_peek(&p,&before)==CP_FRAME);CPStatus a=cpa_admit(&p,&q,capacity);CHECK(a==CP_OK || a==CP_FULL);if(a==CP_FULL){++refused;CPMessage after;CHECK(cp_peek(&p,&after)==CP_FRAME && after.sequence==before.sequence && after.length==before.length && !memcmp(after.payload,before.payload,before.length));size_t extra=9;CHECK(cp_feed(&p,input+offset,n-offset,&extra)==CP_FRAME && extra==0);}}
        else CHECK(status==CP_NEED_MORE || status==CP_REJECT);
        CHECK(q.count<=capacity);if(q.count>peak)peak=q.count;
        if(offset==n && status==CP_NEED_MORE && !p.ready && !q.count){fprintf(stderr,"CPA_OBS capacity=%zu peak=%zu refused=%zu turns=%zu bound=%zu bytes=%zu\n",capacity,peak,refused,turn+1,bound,n);return 0;}
    }
    CHECK(0);return 41;
}
