#include "connection.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){printf("DC_CHECK:%d\n",__LINE__);return 1;}}while(0)
int main(void){
 struct dc_manifest m={255,1,2,3,4,5,0x413,6,7,0,0,0};struct dc_session s;CHECK(dc_init(&s,&m)==DC_OK);
 CHECK(dc_halt(&s,1)==DC_ORDER&&dc_program_authorized(&s)==DC_ORDER);CHECK(dc_identify(&s,0x419)==DC_MISMATCH&&s.phase==0);
 CHECK(dc_identify(&s,0x413)==DC_OK&&dc_halt(&s,0)==DC_UNKNOWN&&s.phase==1);CHECK(dc_halt(&s,1)==DC_OK&&dc_reset(&s,0)==DC_UNKNOWN&&s.phase==2);CHECK(dc_reset(&s,1)==DC_OK&&dc_program_authorized(&s)==DC_UNKNOWN);
 m.flash_driver=8;m.known|=DC_FLASH_DRIVER;CHECK(dc_reconfigure(&s,&m)==DC_OK&&s.phase==0&&!s.observed_target);
 CHECK(dc_identify(&s,0x413)==DC_OK&&dc_halt(&s,1)==DC_OK&&dc_reset(&s,1)==DC_OK&&dc_program_authorized(&s)==DC_OK);
 CHECK(dc_reconfigure(&s,&m)==DC_OK&&s.phase==3);++m.board;CHECK(dc_reconfigure(&s,&m)==DC_OK&&s.phase==0);++m.reset;CHECK(dc_reconfigure(&s,&m)==DC_OK&&s.phase==0);
 m.known|=DC_TRANSPORT;m.transport=9;m.latency_ns=123456;CHECK(dc_reconfigure(&s,&m)==DC_OK&&s.manifest.latency_ns==123456);
 m.power=0;m.known&=~DC_POWER;CHECK(dc_reconfigure(&s,&m)==DC_OK&&dc_identify(&s,0x413)==DC_UNKNOWN);m.power=7;CHECK(dc_reconfigure(&s,&m)==DC_INVALID);
 unsigned stage=99;CHECK(dc_classify(0,0,0,&stage)==DC_OK&&stage==DC_CONNECTION);CHECK(dc_classify(1,0,0,&stage)==DC_OK&&stage==DC_FLASH);CHECK(dc_classify(1,1,0,&stage)==DC_OK&&stage==DC_RESET_APPLICATION);CHECK(dc_classify(0,1,0,&stage)==DC_INVALID&&stage==DC_RESET_APPLICATION);
 unsigned valid=9;CHECK(dc_vector(0x08000000,1024,0x08000081,&valid)==DC_OK&&valid);CHECK(dc_vector(0x08000000,1024,0x08000401,&valid)==DC_OK&&!valid);CHECK(dc_vector(UINT32_MAX-1,4,1,&valid)==DC_INVALID&&!valid);
 puts("DC_PASS:connection,flash,reset-application identity=revoked driver=separate unknown=retained vector=checked remote=synthetic");return 0;
}
