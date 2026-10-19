#include "service.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CAP_TRANSFER:%s:%d\n",#x,__LINE__);return 1;}}while(0)
static int ownership_counterexample(void){CPParser p;cp_parser_init(&p);CPMessage m={1,1,{17}};unsigned char wire[42];size_t n,used;CHECK(cp_encode(&m,wire,42,&n)==CP_OK && cp_feed(&p,wire,n,&used)==CP_FRAME);
    const unsigned char *borrow=p.message.payload;CPMessage copy;CHECK(cp_peek(&p,&copy)==CP_FRAME);cp_parser_init(&p);m.payload[0]=93;CHECK(cp_encode(&m,wire,42,&n)==CP_OK && cp_feed(&p,wire,n,&used)==CP_FRAME);
    CHECK(*borrow==93 && copy.payload[0]==17);puts("borrowed-scratch-counterexample=93 copied-message=17 live-object-no-UB");return 0;}
int main(void){CHECK(!ownership_counterexample());
    for(unsigned split=11;split<=23;split+=12){CsService s;CHECK(cs_init(&s,3,10)==CS_OK);
        for(unsigned session=0;session<2;++session){CPMessage m={(uint16_t)(300+session),32,{0}};for(unsigned i=0;i<32;++i)m.payload[i]=(unsigned char)(i%5==0?0xa5:i+session);unsigned char wire[42];size_t n,used;CHECK(cp_encode(&m,wire,42,&n)==CP_OK);
            CHECK(cs_feed(&s,session,wire,split,&used,1,0)==CS_MORE && used==split);CHECK(cs_feed(&s,session,wire+split,42-split,&used,1,0)==CS_FRAME && used==42-split);}
        const unsigned char *b;size_t n;unsigned first,second;CHECK(cs_begin(&s)==CS_OK && cs_lease(&s,&b,&n,&first)==CS_OK);CHECK(cs_complete(&s,first,0,0,NULL,NULL)==CS_PENDING);
        CHECK(cs_complete(&s,first,1,n,NULL,NULL)==CS_DONE);CHECK(cs_begin(&s)==CS_OK && cs_lease(&s,&b,&n,&second)==CS_OK && second!=first);
        CHECK(cs_complete(&s,first,1,0,NULL,NULL)==CS_STALE && s.borrowed && s.delivered==1);CHECK(cs_close(&s,1)==CS_PENDING && b[6]==0xa5);
        CHECK(cs_complete(&s,second,0,0,NULL,NULL)==CS_PENDING);CHECK(cs_complete(&s,second,1,5,NULL,NULL)==CS_CLOSED && s.offset==5);
        CHECK(s.delivered==1 && s.discarded==1 && s.releases==2 && cs_conserved(&s));
    }puts("CAP_TRANSFER splits=11,23 payload-A5 stale-first-after-second partial5-on-cancel");return 0;}
