#include "checkpoint_firmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CP_CHECK: %s:%d %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
void cp_test_firmware_ring(void){
    for(unsigned capacity=2;capacity<=4;++capacity){
        CPFRing r;CHECK(cpf_ring_init(&r,capacity));
        for(unsigned i=0;i<5;++i)CHECK(cpf_irq_push(&r,(CPFSample){i,100+i,65530+i})==(i<capacity-1));
        CHECK(r.dropped==6-capacity);
        for(unsigned i=0;i<capacity-1;++i){CPFSample sample;CHECK(cpf_take(&r,&sample) && sample.sequence==i && sample.value==100+i && sample.tick==65530+i);
            unsigned char bytes[CP_WIRE];size_t count;CHECK(cpf_frame(&sample,bytes,sizeof bytes,&count)==CP_OK);
            CPParser p;cp_parser_init(&p);size_t used;CHECK(cp_feed(&p,bytes,count,&used)==CP_FRAME && used==count);
            CPMessage message;CHECK(cp_peek(&p,&message)==CP_FRAME && message.length==8);
            CHECK(message.payload[4]==100+i && message.payload[5]==0 && message.payload[6]==(unsigned char)(65530+i) && message.payload[7]==255);
        }
        CPFSample sample;CHECK(!cpf_take(&r,&sample));
        for(unsigned i=0;i<100;++i){CHECK(cpf_irq_push(&r,(CPFSample){i,1,2}));CHECK(cpf_take(&r,&sample) && sample.sequence==i);}
        r.dropped=UINT_MAX;for(unsigned i=0;i<capacity-1;++i)CHECK(cpf_irq_push(&r,(CPFSample){0,0,0}));CHECK(!cpf_irq_push(&r,(CPFSample){0,0,0}) && r.dropped==UINT_MAX);
    }
}
typedef struct {unsigned char bytes[CP_WIRE];size_t count;} Sink;
static void write_byte(void *context,unsigned char byte){Sink *sink=context;CHECK(sink->count<sizeof sink->bytes);sink->bytes[sink->count++]=byte;}
void cp_test_firmware_tx(void){
    CPFTx t;CPMessage m={.sequence=7,.length=2,.payload={0x41,0x42}};cpf_tx_init(&t,5,2);CHECK(cpf_tx_begin(&t,&m,0)==CPF_OK);Sink sink={0};
    CHECK(cpf_tx_poll(&t,1,0,0,write_byte,&sink)==CPF_PENDING && !sink.count && t.retained);
    for(size_t i=0;i<t.length;++i)CHECK(cpf_tx_poll(&t,1,1,1,write_byte,&sink)==CPF_PENDING && t.retained);
    CHECK(t.state==CPF_WIRE && !memcmp(sink.bytes,t.wire,t.length));CHECK(cpf_tx_poll(&t,1,1,0,write_byte,&sink)==CPF_PENDING);
    CHECK(cpf_tx_poll(&t,1,1,1,write_byte,&sink)==CPF_OK && !t.retained && cpf_tx_next(&t)==CPF_OK);
    CHECK(cpf_tx_begin(&t,&m,1)==CPF_OK);unsigned generation=t.generation;
    CHECK(cpf_tx_dma(&t,generation-1,0,0,1,0)==CPF_STALE && t.retained);
    CHECK(cpf_tx_dma(&t,generation,1,0,1,0)==CPF_PENDING && t.retained);
    CHECK(cpf_tx_dma(&t,generation,0,0,0,0)==CPF_PENDING && t.state==CPF_WIRE && t.retained);
    CHECK(cpf_tx_dma(&t,generation,0,0,1,0)==CPF_OK && !t.retained && cpf_tx_next(&t)==CPF_OK);
    CHECK(cpf_tx_begin(&t,&m,1)==CPF_OK);generation=t.generation;
    CHECK(cpf_tx_timeout(&t,generation,17)==CPF_PENDING && cpf_tx_timeout(&t,generation,18)==CPF_PENDING && t.fault_code==17);
    CHECK(cpf_tx_recover(&t,generation,1,1)==CPF_PENDING && t.retained && t.generation==generation);
    CHECK(cpf_tx_recover(&t,generation,0,1)==CPF_PENDING && t.retained);
    CHECK(cpf_tx_recover(&t,generation,0,0)==CPF_OK && !t.retained && t.generation==generation+1);
    CHECK(cpf_tx_begin(&t,&m,1)==CPF_OK && cpf_tx_dma(&t,generation,0,0,1,0)==CPF_STALE);
    CHECK(cpf_tx_dma(&t,t.generation,1,4,0,1)==CPF_FAILED && t.retained);
    CHECK(cpf_tx_recover(&t,t.generation,0,0)==CPF_FAILED && t.state==CPF_RETIRED && !t.retained);
    CHECK(cpf_tx_begin(&t,&m,0)==CPF_INVALID);
}
void cp_test_firmware_clock(void){
    uint16_t ticks=99;CHECK(cpf_elapsed16(65530,6,&ticks) && ticks==12);
    CHECK(!cpf_elapsed16(0,32768,&ticks) && ticks==12);CHECK(cpf_elapsed16(3,3,&ticks) && !ticks);
    CPFClock clock;CHECK(cpf_clock_init(&clock));uint64_t value=99;
    CHECK(cpf_clock_write(&clock,UINT64_C(0xffffffff)) && cpf_clock_read(&clock,1,&value) && value==UINT64_C(0xffffffff));
    /* Explicit cut inside low-word rollover: naive hi/lo gives 0, not either committed value. */
    atomic_store(&clock.sequence,3);atomic_store(&clock.low,0);
    CHECK(!cpf_clock_read(&clock,4,&value) && value==UINT64_C(0xffffffff));
    atomic_store(&clock.high,1);atomic_store(&clock.sequence,4);
    CHECK(cpf_clock_read(&clock,1,&value) && value==UINT64_C(0x100000000));
    atomic_store(&clock.sequence,UINT_MAX-1);CHECK(!cpf_clock_write(&clock,3));
    CHECK(!cpf_clock_read(&clock,0,&value) && value==UINT64_C(0x100000000));
}
