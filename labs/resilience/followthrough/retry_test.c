#include "retry.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do{if(!(c)){fprintf(stderr,"RZ_CHECK: %s:%d %s\n",__FILE__,__LINE__,#c);exit(1);}}while(0)
struct rz_allocations {unsigned made,dropped;};
static void *allocate(void *p,size_t n){void *v=malloc(n);if(v)++((struct rz_allocations *)p)->made;return v;}
static void deallocate(void *p,void *v){++((struct rz_allocations *)p)->dropped;free(v);}
static void w16(unsigned char *p,unsigned v){p[0]=(unsigned char)(v>>8);p[1]=(unsigned char)v;}
static void w32(unsigned char *p,uint32_t v){w16(p,v>>16);w16(p+2,v);}
/* Byte-oriented test encoder; the actual imported net_receive verifies both checksums. */
static unsigned checksum(const unsigned char *p,size_t n,uint32_t sum){
    for(size_t i=0;i<n;++i)sum+=(uint32_t)p[i]<<(i%2 ? 0 : 8);
    while(sum>65535)sum=(sum&65535)+(sum>>16);
    return 65535-sum;
}
static void acknowledgement(struct net_packet *p,const struct net_stack *s,bool stale) {
    *p=(struct net_packet){0};
    const unsigned char localmac[6]={2,0,0,0,0,2},peermac[6]={2,0,0,0,0,1};
    const unsigned char peerip[4]={192,0,2,1},localip[4]={192,0,2,2};
    for(unsigned i=0;i<6;++i){p->bytes[i]=localmac[i];p->bytes[i+6]=peermac[i];}
    w16(p->bytes+12,0x800);unsigned char *ip=p->bytes+14;
    ip[0]=0x45;w16(ip+2,40);w16(ip+6,0x4000);ip[8]=64;ip[9]=6;
    for(unsigned i=0;i<4;++i){ip[12+i]=peerip[i];ip[16+i]=localip[i];}
    w16(ip+10,checksum(ip,20,0));unsigned char *tcp=ip+20;
    w16(tcp,40000);w16(tcp+2,8080);w32(tcp+4,s->rcv_nxt);
    w32(tcp+8,stale ? s->snd_una : s->snd_nxt);tcp[12]=0x50;tcp[13]=16;w16(tcp+14,512);
    w16(tcp+16,checksum(tcp,20,0xc000+0x0201+0xc000+0x0202+6+20));p->length=60;
}
static void transport(struct net_stack *s) {
    net_init(s,100);s->state=NET_ESTABLISHED;s->flight=1;s->peer_known=1;s->peer_port=40000;
    s->peer_window=512;s->arp_expires=30000;s->snd_una=100;s->snd_nxt=103;s->rcv_nxt=1000;
    s->rto=1000;s->due=1000;s->retained.length=3;
    s->retained.bytes[0]=0;s->retained.bytes[1]=1;s->retained.bytes[2]=3;
}
struct rz_sink {
    struct rz_receiver receiver;
    unsigned calls,effects,duplicate,drop_mask;
    uint64_t now;
};
static int sink(void *context,const struct net_packet *p) {
    struct rz_sink *s=context;unsigned call=s->calls++;
    CHECK(p->length==3 && p->bytes[0]==0 && p->bytes[1]==1 && p->bytes[2]==3);
    if(s->drop_mask&(1u<<call))return 1;
    int result=99;bool replay=false;
    /* Receiver clock unit is1000 sender milliseconds, retention16 units. */
    CHECK(rz_receive(&s->receiver,s->now/1000,0,p->bytes[1],1,p->bytes[2],&result,&replay)==RZ_OK);
    s->effects+=!replay;s->duplicate+=replay;CHECK(result==3);
    return 0;
}
static void make(struct rzr_retry *r,struct net_stack *net,struct rzf_bucket *bucket,struct rzf_scope **scope,struct rz_allocations *a,
                 unsigned capacity,bool safe,uint64_t budget,uint32_t seed) {
    transport(net);CHECK(rzf_bucket_init(bucket,capacity,1,65535,0)==RZ_OK);
    CHECK(rzf_scope_create(NULL,0,budget,(struct rzf_allocator){a,allocate,deallocate},scope)==RZ_OK);
    CHECK(rzr_init(r,*scope,bucket,net,3,2,8,safe,seed)==RZ_OK);
}
static void tests(void) {
    for(uint32_t span=1;span<=32;++span) {
        unsigned bins[32]={0},accepted=0;
        for(uint32_t sample=0;sample<256;++sample) {
            uint32_t value=99;int status=rzr_reduce(sample,8,span,&value);
            if(sample<256%span)CHECK(status==RZ_PENDING && value==99);
            else {CHECK(status==RZ_OK && value<span);++bins[value];++accepted;}
        }
        for(uint32_t i=0;i<span;++i)CHECK(bins[i]==accepted/span);
    }
    uint32_t value=99;CHECK(rzr_reduce(256,8,3,&value)==RZ_INVALID && value==99);
    struct rzr_random random={0,UINT64_MAX};CHECK(rzr_uniform(&random,3,&value)==RZ_FULL && value==99);
    for(unsigned mask=0;mask<16;++mask) {
        struct rz_allocations a={0};struct rzf_scope *scope=NULL;struct rzf_bucket bucket;struct net_stack net;struct rzr_retry retry;
        make(&retry,&net,&bucket,&scope,&a,4,true,10000,mask);
        struct rz_sink s={0};s.drop_mask=mask;CHECK(rz_receiver_init(&s.receiver,0,16)==RZ_OK);
        int original=rzr_original(&retry,0,sink,&s);CHECK(original==((mask&1)?RZ_PENDING:RZ_OK));
        CHECK(retry.dispatched==1 && bucket.tokens==3);
        CHECK(rzr_finish(&retry)==RZ_BUSY);
        for(unsigned i=0;i<3;++i) {
            uint64_t due=99;s.now=net.due;
            CHECK(rzr_plan(&retry,s.now,0,RZR_FULL_JITTER,&due)==RZ_OK);
            CHECK(due>=s.now && due-s.now<=(UINT64_C(2)<<i));
            if(due>s.now)CHECK(rzr_dispatch(&retry,due-1,sink,&s)==RZ_PENDING);
            s.now=due;int status=rzr_dispatch(&retry,due,sink,&s);
            CHECK(status==((mask&(1u<<(i+1)))?RZ_PENDING:RZ_OK));
            CHECK(net.flight && net.retries==i+1 && net.retransmitted);
        }
        CHECK(retry.dispatched==4 && retry.retry.attempts==3 && bucket.tokens==0 && s.calls==4);
        CHECK(s.effects==(mask==15?0u:1u));CHECK(s.receiver.total==(mask==15?0:3));
        uint64_t refused=99;CHECK(rzr_plan(&retry,net.now,0,RZR_FULL_JITTER,&refused)==RZ_FULL && refused==99);
        struct net_packet packet,out;acknowledgement(&packet,&net,true);
        CHECK(rzr_ack_packet(&retry,packet.bytes,packet.length,net.now,&out)==RZ_OK && net.flight);
        acknowledgement(&packet,&net,false);
        rz_cancel(&scope->core);rzf_scope_release(&scope);CHECK(a.dropped==0);
        CHECK(rzr_ack_packet(&retry,packet.bytes,packet.length,net.now,&out)==RZ_OK && !net.flight);
        CHECK(rzr_finish(&retry)==RZ_OK && a.dropped==1 && retry.closed);
        CHECK(rzr_finish(&retry)==RZ_INVALID && a.made==a.dropped);
    }
    struct rz_allocations a={0};struct rzf_scope *scope=NULL;struct rzf_bucket bucket;struct net_stack net;struct rzr_retry retry;
    make(&retry,&net,&bucket,&scope,&a,1,true,10000,0);
    struct rz_sink s={0};CHECK(rz_receiver_init(&s.receiver,0,16)==RZ_OK);
    CHECK(rzr_original(&retry,0,sink,&s)==RZ_OK);
    uint64_t due;CHECK(rzr_plan(&retry,1000,0,RZR_NO_JITTER,&due)==RZ_OK);
    CHECK(rzr_dispatch(&retry,due,sink,&s)==RZ_DENIED && s.calls==1 && retry.pending && net.retries==0);
    rz_cancel(&scope->core);
    CHECK(rzr_dispatch(&retry,due,sink,&s)==RZ_CANCELLED && !retry.pending && net.flight);
    CHECK(rzr_finish(&retry)==RZ_BUSY);rzf_scope_release(&scope);
    CHECK(rzr_abort(&retry)==RZ_OK && a.made==a.dropped);
    make(&retry,&net,&bucket,&scope,&a,4,true,10000,0);
    CHECK(rzr_original(&retry,0,sink,&s)==RZ_OK);
    due=99;CHECK(rzr_plan(&retry,1000,9000,RZR_FULL_JITTER,&due)==RZ_EXPIRED && due==99 && retry.retry.attempts==0);
    CHECK(net.flight && rzr_finish(&retry)==RZ_BUSY);
    struct net_packet delayed_ack,delayed_out;acknowledgement(&delayed_ack,&net,false);
    rzf_scope_release(&scope);CHECK(a.made==a.dropped+1);
    CHECK(rzr_ack_packet(&retry,delayed_ack.bytes,delayed_ack.length,10001,&delayed_out)==RZ_OK && !net.flight);
    CHECK(rzr_finish(&retry)==RZ_OK && a.made==a.dropped);
    make(&retry,&net,&bucket,&scope,&a,4,false,10000,0);
    CHECK(rzr_original(&retry,0,sink,&s)==RZ_OK);
    CHECK(rzr_plan(&retry,1000,0,RZR_FULL_JITTER,&due)==RZ_DENIED);
    CHECK(rzr_abort(&retry)==RZ_OK);rzf_scope_release(&scope);CHECK(a.made==a.dropped);
}
static void random_oracle(void) {
    uint32_t seed,span;unsigned count;
    while(scanf("%"SCNu32" %"SCNu32" %u",&seed,&span,&count)==3) {
        struct rzr_random random={seed,0};
        for(unsigned i=0;i<count;++i) {
            uint32_t value=99;int status=rzr_uniform(&random,span,&value);
            printf("%d:%"PRIu32":%"PRIu32":%"PRIu64",",status,value,random.state,random.draws);
        }
        puts("");
    }
}
static void plan_oracle(void) {
    uint32_t seed;unsigned policy,attempt;uint64_t now,after,budget;
    while(scanf("%"SCNu32" %u %u %"SCNu64" %"SCNu64" %"SCNu64,
                &seed,&policy,&attempt,&now,&after,&budget)==6) {
        struct rz_allocations a={0};struct rzf_scope *scope=NULL;struct rzf_bucket bucket;struct net_stack net;struct rzr_retry retry;
        make(&retry,&net,&bucket,&scope,&a,4,true,budget,seed);
        /* A controlled admitted-history state, with the same attempt count in both layers. */
        retry.original=1;retry.retry.attempts=attempt;net.retries=attempt;
        uint64_t due=99;int status=rzr_plan(&retry,now,after,(enum rzr_jitter)policy,&due);
        printf("%d %"PRIu64" %u %u %"PRIu32" %"PRIu64"\n",status,due,
               retry.retry.attempts,retry.pending,retry.random.state,retry.random.draws);
        CHECK(rzr_abort(&retry)==RZ_OK);rzf_scope_release(&scope);CHECK(a.made==a.dropped);
    }
}
static void history_oracle(void) {
    unsigned budget,draws[4];
    while(scanf("%u %u %u %u %u",&budget,&draws[0],&draws[1],&draws[2],&draws[3])==5) {
        struct rz_scope scope;struct rz_retry retry;uint64_t now=0,out=99;
        CHECK(rz_scope_init(&scope,NULL,0,budget)==RZ_OK);
        CHECK(rz_retry_init(&retry,&scope,3,2,8,true)==RZ_OK);
        for(unsigned i=0;i<4;++i) {
            int status=rz_retry_next(&retry,now,draws[i],&out);
            if(status==RZ_OK)now=out;
            printf("%d:%u:%"PRIu64":%"PRIu64",",status,retry.attempts,out,now);
        }
        puts("");
    }
}
struct population_sink {uint64_t now,cutoff;unsigned sent,delivered;};
static int population_sink(void *context,const struct net_packet *packet) {
    struct population_sink *s=context;CHECK(packet->length==3);++s->sent;
    if(s->now<s->cutoff)return 1;
    ++s->delivered;return 0;
}
static void population_oracle(void) {
    unsigned policy,correlated,early;uint64_t cutoff;
    while(scanf("%u %u %u %"SCNu64,&policy,&correlated,&early,&cutoff)==4) {
        enum {CLIENTS=32};struct rzr_retry retries[CLIENTS];struct net_stack nets[CLIENTS];
        struct rzf_scope *scopes[CLIENTS]={0};uint64_t next[CLIENTS];unsigned stage[CLIENTS];
        struct rz_allocations a={0};struct rzf_bucket limiter;struct population_sink sink_state={0,cutoff,0,0};
        CHECK(rzf_bucket_init(&limiter,128,1,65535,0)==RZ_OK);
        for(unsigned i=0;i<CLIENTS;++i) {
            transport(&nets[i]);CHECK(rzf_scope_create(NULL,0,10000,(struct rzf_allocator){&a,allocate,deallocate},&scopes[i])==RZ_OK);
            CHECK(rzr_init(&retries[i],scopes[i],&limiter,&nets[i],3,2,8,true,correlated?0:i)==RZ_OK);
            CHECK(rzr_original(&retries[i],0,population_sink,&sink_state)==RZ_PENDING);
            rzf_scope_release(&scopes[i]);next[i]=early?0:1000;stage[i]=1;
        }
        unsigned remaining=CLIENTS;
        while(remaining) {
            unsigned i=CLIENTS;uint64_t now=UINT64_MAX;
            for(unsigned k=0;k<CLIENTS;++k)if(stage[k] && (next[k]<now || i==CLIENTS)){i=k;now=next[k];}
            CHECK(i<CLIENTS);sink_state.now=now;
            if(stage[i]==1) {
                uint64_t due;CHECK(rzr_plan(&retries[i],now,0,(enum rzr_jitter)policy,&due)==RZ_OK);
                stage[i]=2;next[i]=due;
            } else {
                int status=rzr_dispatch(&retries[i],now,population_sink,&sink_state);
                CHECK(status==RZ_OK || status==RZ_PENDING);
                printf("%u:%"PRIu64":%d,",i,now,status);
                if(status==RZ_OK) {
                    struct net_packet packet,out;acknowledgement(&packet,&nets[i],false);
                    CHECK(rzr_ack_packet(&retries[i],packet.bytes,packet.length,now,&out)==RZ_OK);
                    CHECK(rzr_finish(&retries[i])==RZ_OK);stage[i]=0;--remaining;
                } else if(nets[i].retries==3) {
                    CHECK(rzr_abort(&retries[i])==RZ_OK);stage[i]=0;--remaining;
                } else {stage[i]=1;next[i]=early?now:nets[i].due;}
            }
        }
        printf("S%u:D%u:T%"PRIu64":A%u:F%u\n",sink_state.sent,sink_state.delivered,limiter.tokens,a.made,a.dropped);
        CHECK(a.made==a.dropped);
    }
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    if(!strcmp(argv[1],"test"))tests();
    else if(!strcmp(argv[1],"random"))random_oracle();
    else if(!strcmp(argv[1],"plan"))plan_oracle();
    else if(!strcmp(argv[1],"history"))history_oracle();
    else if(!strcmp(argv[1],"population"))population_oracle();
    else return 2;
    return 0;
}
