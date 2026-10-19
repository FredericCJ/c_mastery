#include "service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CAP_ASSERT:%s:%d\n",#x,__LINE__);return 1;}}while(0)
static const unsigned char golden[]={0xa5,1,2,1,3,0,0,0x41,0xff,0x62,0xf6,0x8b,0x76};
typedef struct { unsigned char bytes[512];size_t count,call;int mode; } Sink;
static CsIO sink(void *context,const unsigned char *bytes,size_t n){
    Sink *s=context;size_t k=n;
    if(s->mode==1){static const size_t sizes[]={2,0,0,0,3,8};k=sizes[s->call];}
    if(s->mode==2 && s->call==0)k=5;
    if(s->mode==3)return(CsIO){n+1,CS_IO_PROGRESS,0};
    if(s->mode==2 && s->call==1){++s->call;return(CsIO){0,CS_IO_ERROR,5};}
    if(k>n || s->count+k>sizeof s->bytes)abort();
    for(size_t i=0;i<k;++i)s->bytes[s->count++]=bytes[i];
    CsIOKind kind=CS_IO_PROGRESS;
    if(s->mode==1 && s->call==1)kind=CS_IO_AGAIN;
    if(s->mode==1 && s->call==3)kind=CS_IO_INTERRUPTED;
    ++s->call;return(CsIO){k,kind,0};
}
static CPMessage message(unsigned seq,size_t n){
    CPMessage m={0};m.sequence=(uint16_t)seq;m.length=n;
    for(size_t i=0;i<n;++i)m.payload[i]=(unsigned char)(i*7u+0xa5u);
    return m;
}
static int feed_all(CsService *s,unsigned session,const unsigned char *bytes,size_t n,int authorized){
    size_t cursor=0;for(unsigned turns=0;turns<100;++turns){
        size_t used=0;CsStatus st=cs_feed(s,session,bytes+cursor,n-cursor,&used,authorized,0);cursor+=used;
        if(st==CS_MORE)return cursor==n?0:1;
        if(st!=CS_REJECT && st!=CS_FRAME && st!=CS_FULL && st!=CS_DENIED)return 1;
        if(cursor==n && !s->input[session].used)return 0;
    }return 1;
}
static int framing(void){
    size_t cases=0;
    for(size_t len=0;len<=32;++len){CPMessage m=message(258,len);unsigned char bytes[42];size_t n=0;CHECK(cp_encode(&m,bytes,42,&n)==CP_OK);
        for(size_t split=0;split<=n;++split){CsService s;CHECK(cs_init(&s,3,1000)==CS_OK);CHECK(!feed_all(&s,0,bytes,split,1));
            CHECK(s.accepted==(split==n?1u:0u));CHECK(!feed_all(&s,0,bytes+split,n-split,1));CHECK(s.accepted==1 && s.count==1);
            CHECK(cs_begin(&s)==CS_OK);Sink out={0};CHECK(cs_step(&s,sink,&out)==CS_DONE);CHECK(out.count==n && !memcmp(out.bytes,bytes,n));CHECK(cs_conserved(&s));++cases;
        }
    }
    CsService s;CHECK(cs_init(&s,3,100)==CS_OK);CHECK(!feed_all(&s,0,golden,sizeof golden,0));CHECK(s.denied==1 && s.accepted==0 && s.count==0);
    CPMessage m=message(9,32);m.payload[0]=1;unsigned char wire[42];size_t n;CHECK(cp_encode(&m,wire,42,&n)==CP_OK);
    CHECK(!feed_all(&s,1,wire,n,0));CHECK(s.denied==2 && !s.accepted);/* A claimed payload permission grants nothing. */
    wire[n-1]^=1;CHECK(!feed_all(&s,0,wire,n,1));CHECK(!s.accepted);wire[n-1]^=1;
    CHECK(!feed_all(&s,0,wire,n,1));CHECK(s.accepted==1 && cs_conserved(&s));
    printf("framing-splits=%zu authorization=trusted-not-payload\n",cases);return 0;
}
static int progress(void){
    CsService s;CHECK(cs_init(&s,3,100)==CS_OK);CHECK(!feed_all(&s,0,golden,sizeof golden,1));CHECK(cs_begin(&s)==CS_OK);
    Sink out={.mode=1};const size_t expected[]={2,2,2,2,5,13};
    for(unsigned i=0;i<6;++i){CsStatus st=cs_step(&s,sink,&out);CHECK(st==(i==5?CS_DONE:CS_PENDING));CHECK(s.offset==expected[i]);CHECK(out.call==i+1 && cs_conserved(&s));}
    CHECK(out.count==13 && !memcmp(out.bytes,golden,13) && s.delivered==1);
    CHECK(!feed_all(&s,1,golden,13,1));CHECK(cs_begin(&s)==CS_OK);out=(Sink){.mode=3};CHECK(cs_step(&s,sink,&out)==CS_FAILED && s.offset==0 && s.first_cause==4096);
    CHECK(cs_recover(&s)==CS_OK);out=(Sink){.mode=2};CHECK(cs_step(&s,sink,&out)==CS_PENDING && s.offset==5);CHECK(cs_step(&s,sink,&out)==CS_FAILED && s.offset==5 && s.first_cause==4096);
    CHECK(cs_step(&s,sink,&out)==CS_FAILED && out.call==2);CHECK(cs_recover(&s)==CS_OK);out.mode=0;CHECK(cs_step(&s,sink,&out)==CS_DONE && out.count==13 && !memcmp(out.bytes,golden,13));
    puts("output-cursors=2,2,2,2,5,13 suffix-recovery=5 first-cause-preserved");return 0;
}
static int pressure(void){
    CsService s;CHECK(cs_init(&s,3,100)==CS_OK);for(unsigned id=1;id<=3;++id){CPMessage m=message(id,32);CHECK(cs_offer(&s,&m,1,0,0)==CS_FRAME);}
    CPMessage m=message(4,32),before=m;CHECK(cs_offer(&s,&m,1,0,0)==CS_FULL && m.sequence==before.sequence && m.length==before.length && !memcmp(m.payload,before.payload,m.length));CHECK(cs_begin(&s)==CS_OK);m.sequence=5;CHECK(cs_offer(&s,&m,1,0,0)==CS_FRAME);
    CHECK(cs_close(&s,0)==CS_PENDING);m.sequence=6;CHECK(cs_offer(&s,&m,1,0,0)==CS_CLOSED);Sink out={0};CHECK(cs_step(&s,sink,&out)==CS_DONE);CHECK(cs_begin(&s)==CS_OK && s.active.sequence==2);CHECK(cs_step(&s,sink,&out)==CS_DONE);
    CHECK(cs_begin(&s)==CS_OK && s.active.sequence==3);const unsigned char *borrow;size_t n;unsigned g;CHECK(cs_lease(&s,&borrow,&n,&g)==CS_OK);
    CHECK(cs_close(&s,1)==CS_PENDING);CHECK(s.has_active && s.borrowed && s.releases==3 && borrow[0]==0xa5 && cs_conserved(&s));
    CHECK(cs_complete(&s,g,0,0,0,0)==CS_PENDING && s.releases==3);CHECK(cs_complete(&s,g-1,1,0,0,0)==CS_STALE);
    CHECK(cs_complete(&s,g,1,0,0,0)==CS_CLOSED && s.releases==4 && s.delivered==2 && s.discarded==2 && cs_conserved(&s));
    CHECK(cs_close(&s,1)==CS_CLOSED && cs_complete(&s,g,1,0,0,0)==CS_STALE && s.releases==4);
    /* Only explicitly replaceable telemetry may evict an acknowledged item. */
    CHECK(cs_init(&s,1,100)==CS_OK);m.sequence=1;CHECK(cs_offer(&s,&m,1,1,0)==CS_FRAME);m.sequence=2;
    CHECK(cs_offer(&s,&m,1,0,1)==CS_FULL && s.discarded==0);CHECK(cs_offer(&s,&m,1,1,1)==CS_FRAME && s.discarded==1 && s.count==1 && cs_conserved(&s));
    /* Both real parser sessions submit maximum frames while output is stalled. */
    CHECK(cs_init(&s,3,100)==CS_OK);unsigned char wire[42];size_t used=0;n=0;
    for(unsigned id=10;id<13;++id){m=message(id,32);CHECK(cp_encode(&m,wire,42,&n)==CP_OK);CHECK(!feed_all(&s,id%2,wire,n,1));}
    CHECK(cs_begin(&s)==CS_OK);m.sequence=13;CHECK(cp_encode(&m,wire,42,&n)==CP_OK && !feed_all(&s,1,wire,n,1));
    for(unsigned session=0;session<2;++session){m.sequence=(uint16_t)(14+session);CHECK(cp_encode(&m,wire,42,&n)==CP_OK);CHECK(cs_feed(&s,session,wire,n,&used,1,0)==CS_FULL && used==42);}
    CHECK(s.accepted==4 && s.rejected==2 && s.count==3 && s.has_active && cs_conserved(&s));
    printf("accepted=1,2,3,5 delivered=1,2 discarded=3,5 releases=4 backing=%zu metadata=%zu\n",(size_t)(CS_SESSIONS+CS_QUEUE+1)*CS_WIRE,sizeof(CsService)-(CS_SESSIONS+CS_QUEUE+1)*CS_WIRE);return 0;
}
typedef struct { CsService *owner;int called; } Final;
static void final_free(void *p,uint16_t id,CsStatus status){Final *f=p;if(id!=19 || status!=CS_CLOSED)abort();++f->called;free(f->owner);f->owner=NULL;}
static int terminal(void){
    Final f={malloc(sizeof(CsService)),0};CHECK(f.owner && cs_init(f.owner,1,2)==CS_OK);CPMessage m=message(19,1);CHECK(cs_offer(f.owner,&m,1,0,0)==CS_FRAME && cs_begin(f.owner)==CS_OK);
    const unsigned char *bytes;size_t n;unsigned g;CHECK(cs_lease(f.owner,&bytes,&n,&g)==CS_OK && n==11);CHECK(cs_close(f.owner,1)==CS_PENDING);CHECK(bytes[0]==0xa5);
    CHECK(cs_complete(f.owner,g,1,0,final_free,&f)==CS_CLOSED && !f.owner && f.called==1);
    puts("terminal-notification=last-access capacity1=retained-until-quiescence");return 0;
}
int main(int argc,char **argv){
    if(argc!=2)return 2;
    if(!strcmp(argv[1],"framing"))return framing();
    if(!strcmp(argv[1],"progress"))return progress();
    if(!strcmp(argv[1],"pressure"))return pressure();
    if(!strcmp(argv[1],"terminal"))return terminal();
    return 2;
}
