#include "i2c_depth.h"
void ia_note(const char *);
void ia_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){ia_note("IA_ASSERT: " #x);return 41;}}while(0)
int ia_electrical(void){
 IaInterval x={99,88};
 CHECK(ia_nominal(3300,400,3000,300,100,&x)==IA_OK && x.minimum==967 && x.maximum==3540);
 ia_values("nominal",100,x.minimum,x.maximum,IA_OK);
 CHECK(ia_nominal(3300,400,3000,300,400,&x)==IA_EMPTY && x.minimum==967 && x.maximum==885);
 ia_values("nominal",400,x.minimum,x.maximum,IA_EMPTY);
 for(unsigned c=1;c<=10000;c++){
  int r=ia_nominal(3300,400,3000,300,c,&x);CHECK(r==(x.minimum<=x.maximum?IA_OK:IA_EMPTY));
  CHECK((uint64_t)x.minimum*3000>=2900000 && (uint64_t)(x.minimum-1)*3000<2900000);
  CHECK((uint64_t)x.maximum*8473*c<=3000000000u && (uint64_t)(x.maximum+1)*8473*c>3000000000u);
 }
 x.minimum=99;x.maximum=88;
 CHECK(ia_nominal(0,400,3000,300,100,&x)==IA_INVALID && x.minimum==99 && x.maximum==88);
 unsigned cases=0;
 for(unsigned voltage=1800;voltage<=5000;voltage+=800)
 for(unsigned tolerance=0;tolerance<=250000;tolerance+=50000)
 for(unsigned leak=0;leak<=200;leak+=50){
  IaEnvelope e={voltage,voltage+100,400,3000,300,100,leak,leak,tolerance};
  int r=ia_guaranteed(&e,&x);CHECK(r==(x.minimum<=x.maximum?IA_OK:IA_EMPTY));
  /* Independent inequality check at worst-case corners, without rounding a bound. */
  uint64_t actual_min=(uint64_t)x.minimum*(1000000-tolerance);
  CHECK(actual_min*(3000-leak)>=(uint64_t)(voltage+100-400)*1000*1000000);
  uint64_t actual_max=(uint64_t)x.maximum*(1000000+tolerance);
  CHECK(actual_max*((uint64_t)4*voltage*100+(uint64_t)10*leak*300)<=(uint64_t)3000*voltage*300*1000000);
  ia_values("guaranteed",voltage,tolerance,leak,x.minimum);
  ia_values("upper",voltage,tolerance,leak,x.maximum);cases++;
 }
 IaEnvelope e={6000,6000,0,20000,1000000,1,0,0,0};
 CHECK(ia_guaranteed(&e,&x)==IA_OK && x.maximum==750000000);
 e.source_leak_ua=20000;CHECK(ia_guaranteed(&e,&x)==IA_EMPTY && x.minimum>x.maximum);
 ia_values("electrical",cases,10000,750000000,IA_EMPTY);return 0;
}
int ia_arbitration(void){
 uint8_t b[3][32]={{0}};unsigned low[3]={4,7,5},high[3]={6,3,8};IaTrace r;
 unsigned cases=0;
 for(unsigned a=0;a<256;a++)for(unsigned c=0;c<256;c++){
  for(unsigned k=0;k<8;k++){b[0][k]=(uint8_t)((a>>(7-k))&1);b[1][k]=(uint8_t)((c>>(7-k))&1);}
  CHECK(ia_arbitrate((const uint8_t (*)[32])b,2,8,low,high,&r)==IA_OK);
  CHECK(r.survivors==(a==c?3:a<c?1:2) && r.low==7 && r.high==3);
  unsigned wire=0;for(unsigned k=0;k<8;k++)wire=(wire<<1)|r.bus[k];
  CHECK(wire==(a<c?a:c));cases++;
 }
 for(unsigned j=0;j<3;j++)for(unsigned k=0;k<32;k++)b[j][k]=1;
 b[1][31]=0;CHECK(ia_arbitrate((const uint8_t (*)[32])b,3,32,low,high,&r)==IA_OK && r.survivors==2 && r.lost[31]==5);
 b[0][0]=2;r.survivors=77;
 CHECK(ia_arbitrate((const uint8_t (*)[32])b,3,32,low,high,&r)==IA_UNSUPPORTED && r.survivors==77);
 b[0][0]=3;CHECK(ia_arbitrate((const uint8_t (*)[32])b,3,32,low,high,&r)==IA_UNSUPPORTED);
 b[0][0]=4;CHECK(ia_arbitrate((const uint8_t (*)[32])b,3,32,low,high,&r)==IA_INVALID);
 ia_values("arbitration",cases,7,3,5);return 0;
}
