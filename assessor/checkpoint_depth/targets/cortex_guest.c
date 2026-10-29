#include "checkpoint_firmware.h"
static volatile uint32_t initialized=UINT32_C(0x13579bdf),zeroed;
static struct cpf_ring ring;
static volatile unsigned calls;
void __aeabi_memclr4(void *,size_t);
static void semihost(uint32_t operation,const void *argument){
    register uint32_t r0 __asm__("r0")=operation;register const void *r1 __asm__("r1")=argument;
    __asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");
}
__attribute__((noreturn)) static void stop(int failure){
    semihost(4,failure?"CP_CORTEX_FAIL\n":"CP_CORTEX_PASS\n");
    uint32_t exit_arguments[2]={0x20026u,(uint32_t)failure};semihost(0x20,exit_arguments);for(;;){}
}
void cp_fault(void){stop(2);}
#define REQUIRE(x) do{if(!(x))stop(1);}while(0)
void cp_pendsv(void){
    unsigned ipsr;__asm__ volatile("mrs %0,ipsr":"=r"(ipsr));REQUIRE(ipsr==14);
    unsigned n=calls;(void)cpf_irq_push(&ring,(struct cpf_sample){n,(uint16_t)(100+n),(uint16_t)(65530+n)});calls=n+1;
}
void cp_target_main(void){
    REQUIRE(initialized==UINT32_C(0x13579bdf) && zeroed==0 && calls==0);
    for(size_t count=0;count<=31;++count){
        uint32_t storage[9];volatile unsigned char *bytes=(volatile unsigned char *)storage;
        for(size_t i=0;i<sizeof storage;++i)bytes[i]=0xa5;
        __aeabi_memclr4(storage,count);
        for(size_t i=0;i<sizeof storage;++i)REQUIRE(bytes[i]==(i<count?0:0xa5));
    }
    REQUIRE(cpf_ring_init(&ring,4));
    /* PendSV priority is explicitly selected; reset priority is not assumed low. */
    *(volatile uint8_t *)UINT32_C(0xe000ed22)=255;
    __asm__ volatile("cpsie i":::"memory");
    for(unsigned i=0;i<5;++i){
        *(volatile uint32_t *)UINT32_C(0xe000ed04)=UINT32_C(1)<<28;
        __asm__ volatile("dsb\nisb":::"memory");
        unsigned budget=100000;while(calls!=i+1 && --budget){}REQUIRE(budget);
    }
    REQUIRE(ring.dropped==2);
    for(unsigned i=0;i<3;++i){
        struct cpf_sample sample;REQUIRE(cpf_take(&ring,&sample) && sample.value==100+i && sample.sequence==i);
        unsigned char wire[CP_WIRE];size_t n,used;REQUIRE(cpf_frame(&sample,wire,sizeof wire,&n)==CP_OK);
        struct cp_parser parser;cp_parser_init(&parser);REQUIRE(cp_feed(&parser,wire,n,&used)==CP_FRAME && used==n);
        struct cp_message message;REQUIRE(cp_peek(&parser,&message)==CP_FRAME && message.payload[6]==(unsigned char)(65530+i));
    }
    struct cpf_sample sample;REQUIRE(!cpf_take(&ring,&sample));uint16_t elapsed;REQUIRE(cpf_elapsed16(65530,6,&elapsed) && elapsed==12);
    struct cpf_clock clock;uint64_t stamp;REQUIRE(cpf_clock_init(&clock) && cpf_clock_write(&clock,UINT64_C(0x100000000)) && cpf_clock_read(&clock,3,&stamp) && stamp==UINT64_C(0x100000000));
    stop(0);
}
