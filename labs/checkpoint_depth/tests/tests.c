void cp_test_linux_reader(void);
void cp_test_firmware_ring(void);
void cp_test_firmware_tx(void);
void cp_test_firmware_clock(void);
void cp_test_operation(void);
void cp_test_async_admission(void);
void cp_test_async_lifecycle(void);
void cp_test_async_wait(void);
void cp_test_writer(void);
void cp_test_writer_ownership(void);
void cp_test_numeric_transaction(void);
void cp_test_dispatch(void);
void cp_test_vector(void);
#include "checkpoint_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void cp_test_memory(void);
void cp_test_ownership(void);
void cp_test_numeric(void);
void cp_test_numeric_policy(void);
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CP_CHECK: %s:%d %s\n",__FILE__,__LINE__,#x); exit(1); } } while (0)
static const unsigned char known[] = {0xa5,1,2,1,3,0,0,0x41,0xff,0x62,0xf6,0x8b,0x76};
static const unsigned char nested[] = {0xa5,1,1,0,32,0,0xa5,1,7,0,0,0,0x14,0xe6,0x95,0x66,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x31,0x28,0xf1,0xec,
  0xa5,1,2,1,3,0,0,0x41,0xff,0x62,0xf6,0x8b,0x76};
static int same(const CPMessage *a,const CPMessage *b) {
    return a->sequence==b->sequence && a->length==b->length && !memcmp(a->payload,b->payload,a->length);
}
static size_t drain(CPParser *p,const unsigned char *bytes,size_t n,CPMessage *messages,size_t count) {
    size_t offset=0, budget=2*n+2*CP_WIRE+2;
    CPQueue q; cp_queue_init(&q);
    while (budget--) {
        size_t used=999;
        CPStatus s=cp_feed(p,bytes+offset,n-offset,&used);
        CHECK(used<=n-offset); offset+=used;
        CHECK(p->used<=CP_WIRE);
        if (s==CP_FRAME) {
            CHECK(count<32); CHECK(cp_deliver(p,&q)==CP_OK);
            CHECK(cp_take(&q,&messages[count])==CP_OK); ++count;
        } else if(s==CP_REJECT) {
            CHECK(!p->ready);
        } else {
            CHECK(s==CP_NEED_MORE); CHECK(offset==n); return count;
        }
    }
    CHECK(0); return count;
}
static void codec(void) {
    CHECK(cp_crc32((const unsigned char*)"123456789",9)==UINT32_C(0xcbf43926));
    CHECK(cp_crc32(NULL,0)==0);
    CPMessage m={258,3,{0,0x41,0xff}};
    unsigned char out[44]; memset(out,0x5a,sizeof out); size_t written=777;
    CHECK(cp_encode(&m,out+1,12,&written)==CP_FULL); CHECK(written==777);
    for(size_t i=0;i<sizeof out;++i) CHECK(out[i]==0x5a);
    CHECK(cp_encode(&m,out+1,42,&written)==CP_OK); CHECK(written==13);
    CHECK(!memcmp(out+1,known,13)); CHECK(out[0]==0x5a && out[14]==0x5a);
    m.length=33; written=777; CHECK(cp_encode(&m,out+1,42,&written)==CP_INVALID); CHECK(written==777);
    for(size_t n=0;n<=32;++n) for(unsigned seq=0;seq<3;++seq) {
        m.length=n; m.sequence=seq==0?0:seq==1?65535:258;
        for(size_t i=0;i<n;++i) m.payload[i]=(unsigned char)(i*17);
        CHECK(cp_encode(&m,out+1,42,&written)==CP_OK); CHECK(written==n+10);
        CPParser p; CPMessage decoded[32]; cp_parser_init(&p);
        CHECK(drain(&p,out+1,written,decoded,0)==1); CHECK(same(&m,&decoded[0]));
    }
    /* Unsupported and oversized headers never publish; truncated CRC stays incomplete. */
    unsigned char bad[]={0xa5,2,0,0,0,0,0,0,0,0};
    CPParser p; CPMessage decoded[32], sentinel={123,1,{77}}; cp_parser_init(&p);
    CHECK(drain(&p,bad,sizeof bad,decoded,0)==0); CHECK(cp_peek(&p,&sentinel)==CP_NEED_MORE); CHECK(sentinel.sequence==123);
    bad[1]=1; bad[4]=33; cp_parser_init(&p); CHECK(drain(&p,bad,sizeof bad,decoded,0)==0);
    cp_parser_init(&p); CHECK(drain(&p,known,sizeof known-1,decoded,0)==0);
    CHECK(cp_peek(&p,&sentinel)==CP_NEED_MORE && sentinel.sequence==123);
    /* CRC checks integrity only: re-encoding an attacker-changed synthetic payload is valid. */
    m.sequence=258; m.length=3; m.payload[0]=1; m.payload[1]=0x41; m.payload[2]=0xff;
    CHECK(cp_encode(&m,out,sizeof out,&written)==CP_OK);
    cp_parser_init(&p); CHECK(drain(&p,out,written,decoded,0)==1 && decoded[0].payload[0]==1);
}
static void stream(void) {
    CPMessage got[32];
    /* Every composition of the 13-byte fixture, not just every single split. */
    for(unsigned mask=0;mask<(1u<<12);++mask) {
        CPParser p; cp_parser_init(&p); size_t count=0,start=0;
        for(size_t i=1;i<=sizeof known;++i) if(i==sizeof known || (mask&(1u<<(i-1)))) {
            count=drain(&p,known+start,i-start,got,count); start=i;
        }
        CHECK(count==1 && got[0].sequence==258 && got[0].length==3);
    }
    for(size_t chunk=1;chunk<=sizeof nested;++chunk) {
        CPParser p; cp_parser_init(&p); size_t count=0;
        for(size_t pos=0;pos<sizeof nested;pos+=chunk) {
            size_t n=sizeof nested-pos; if(n>chunk)n=chunk;
            count=drain(&p,nested+pos,n,got,count);
        }
        CHECK(count==2 && got[0].sequence==7 && got[0].length==0 && got[1].sequence==258);
    }
    CPParser p; CPQueue q; cp_parser_init(&p); cp_queue_init(&q);
    for(unsigned i=0;i<3;++i) {
        size_t used; CHECK(cp_feed(&p,known,sizeof known,&used)==CP_FRAME && used==sizeof known);
        if(i<2) CHECK(cp_deliver(&p,&q)==CP_OK);
    }
    CPMessage before,after; CHECK(cp_peek(&p,&before)==CP_FRAME);
    CHECK(cp_deliver(&p,&q)==CP_FULL);
    size_t used=9; CHECK(cp_feed(&p,known,sizeof known,&used)==CP_FRAME && used==0);
    CHECK(cp_peek(&p,&after)==CP_FRAME && same(&before,&after));
    CHECK(cp_take(&q,&after)==CP_OK); CHECK(cp_deliver(&p,&q)==CP_OK);
    CHECK(cp_deliver(&p,&q)==CP_NEED_MORE); CHECK(q.count==2);
    CHECK(cp_take(&q,&after)==CP_OK); CHECK(cp_take(&q,&after)==CP_OK); CHECK(cp_take(&q,&after)==CP_NEED_MORE);
    /* New bytes after acceptance produce exactly one more message; duplicate sequence is legal. */
    CHECK(cp_feed(&p,known,sizeof known,&used)==CP_FRAME && used==sizeof known);
    CHECK(cp_deliver(&p,&q)==CP_OK && q.count==1);
    CHECK(cp_take(&q,&after)==CP_OK && after.sequence==258);
    unsigned char hostile[256]; memset(hostile,0xa5,sizeof hostile); cp_parser_init(&p);
    CHECK(drain(&p,hostile,sizeof hostile,got,0)==0);
}
int main(int argc,char **argv) {
    if(argc<2)return 2;
    if(!strcmp(argv[1],"codec")){codec();return 0;}
#ifndef CP_STREAM_ONLY
    if(!strcmp(argv[1],"firmware_ring")){cp_test_firmware_ring();return 0;}
    if(!strcmp(argv[1],"firmware_tx")){cp_test_firmware_tx();return 0;}
    if(!strcmp(argv[1],"firmware_clock")){cp_test_firmware_clock();return 0;}
    if(!strcmp(argv[1],"linux_reader")){cp_test_linux_reader();return 0;}
    if(!strcmp(argv[1],"operation")){cp_test_operation();return 0;}
    if(!strcmp(argv[1],"async_admission")){cp_test_async_admission();return 0;}
    if(!strcmp(argv[1],"async_lifecycle")){cp_test_async_lifecycle();return 0;}
    if(!strcmp(argv[1],"async_wait")){cp_test_async_wait();return 0;}
    if(!strcmp(argv[1],"memory")){cp_test_memory();return 0;}
    if(!strcmp(argv[1],"ownership")){cp_test_ownership();return 0;}
    if(!strcmp(argv[1],"numeric")){cp_test_numeric();return 0;}
    if(!strcmp(argv[1],"dispatch")){cp_test_dispatch();return 0;}
    if(!strcmp(argv[1],"vector")){cp_test_vector();return 0;}
    if(!strcmp(argv[1],"numeric_transaction")){cp_test_numeric_transaction();return 0;}
    if(!strcmp(argv[1],"writer")){cp_test_writer();return 0;}
    if(!strcmp(argv[1],"writer_ownership")){cp_test_writer_ownership();return 0;}
    if(!strcmp(argv[1],"numeric_policy")){cp_test_numeric_policy();return 0;}
#endif
    if(!strcmp(argv[1],"stream")){stream();return 0;}
    if(!strcmp(argv[1],"oracle") && argc==4) {
        size_t chars=strlen(argv[2]),chunk=(size_t)strtoul(argv[3],NULL,10);
        if(chars%2 || chars>512 || !chunk || chunk>256)return 2;
        unsigned char input[256]; size_t n=chars/2;
        for(size_t i=0;i<n;++i){unsigned b;if(sscanf(argv[2]+2*i,"%2x",&b)!=1)return 2;input[i]=(unsigned char)b;}
        CPParser p; CPMessage messages[32]; cp_parser_init(&p);size_t count=0;
        for(size_t i=0;i<n;i+=chunk){size_t take=n-i;if(take>chunk)take=chunk;count=drain(&p,input+i,take,messages,count);}
        for(size_t i=0;i<count;++i){printf("%u:",(unsigned)messages[i].sequence);for(size_t j=0;j<messages[i].length;++j)printf("%02x",messages[i].payload[j]);putchar('\n');}
        return 0;
    }
    return 2;
}
