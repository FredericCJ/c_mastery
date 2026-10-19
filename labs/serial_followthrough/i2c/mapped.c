#include "i2c_depth.h"
#include "recovery.h"
void ia_note(const char *);void ia_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){ia_note("IA_MAP_ASSERT: " #x);return 41;}}while(0)
static uint32_t addresses[32],values[32];static unsigned used,critical,violation;
uint32_t pi_test_primask;
volatile uint32_t *pi_test_register(uint32_t a){
 if(critical && !pi_test_primask)violation=1;
 for(unsigned i=0;i<used;i++)if(addresses[i]==a)return values+i;
 if(used==32)return 0;
 addresses[used]=a;return values+used++;
}
#define R(a) (*pi_test_register(a))
#define B 0x40005400u
static int event(PiI2c *s,unsigned flags,uint32_t now){
 R(B+20)=flags;if(flags&PI_SB){R(B)&=~PI_START;R(B+24)=3;}
 critical=1;int r=pi_f439_step(s,s->token,now);critical=0;return r;
}
int ia_mapped(void){
 PiI2c s={0};PuClock c={16000000,1,1,1,1,0};uint8_t out=77;
 R(0x40020424u)=0xffffffffu;pi_f439_pins();
 CHECK(R(0x40020424u)==0xffffff44u && (R(0x40020404u)&0x300u)==0x300u);
 CHECK((R(0x40020400u)&0xf0000u)==0xa0000u && (R(0x40023840u)&(1u<<21)));
 critical=1;CHECK(pi_f439_open(&s,&c)==PI_OK);critical=0;
 CHECK(pi_test_primask==0 && !violation && R(B+4)==16 && R(B+28)==100 && R(B+32)==17);
 pi_test_primask=1;critical=1;CHECK(pi_f439_begin(&s,1,1,0x50,7,1,&out,0,100)==PI_OK);critical=0;
 CHECK(pi_test_primask==1 && !violation);pi_test_primask=0;
 CHECK(pi_f439_recovery_prepare(&s,1,0)==PI_BUSY);
 CHECK(event(&s,PI_SB,1)==PI_BUSY && R(B+16)==0xa0);
 CHECK(event(&s,PI_ADDR,2)==PI_BUSY);
 CHECK(event(&s,PI_TXE,3)==PI_BUSY && R(B+16)==7);
 CHECK(event(&s,PI_BTF,4)==PI_BUSY);
 CHECK(event(&s,PI_SB,5)==PI_BUSY && R(B+16)==0xa1);
 CHECK(event(&s,PI_ADDR,6)==PI_BUSY && !(R(B)&PI_ACK) && (R(B)&PI_STOP));
 R(B+16)=0x5a;CHECK(event(&s,PI_RXNE,7)==PI_BUSY && out==77);
 R(B)&=~PI_STOP;R(B+24)=0;
 CHECK(event(&s,0,8)==PI_OK && out==0x5a && !violation && pi_test_primask==0);
 CHECK(pi_f439_recovery_prepare(&s,1,1)==PI_FORBIDDEN && s.ready);
 CHECK(pi_f439_recovery_prepare(&s,1,0)==PI_OK && !s.ready && (R(0x40020400u)&0xf0000u)==0x50000u);
 critical=1;CHECK(pi_f439_open(&s,&c)==PI_OK);critical=0;
 CHECK((R(0x40020400u)&0xf0000u)==0xa0000u && !violation && pi_test_primask==0);
 ia_values("mapped",R(0x40020424u),R(B+28),R(B+32),out);return 0;
}
