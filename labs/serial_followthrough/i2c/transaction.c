#include "i2c_depth.h"
#include "recovery.h"
void ia_note(const char *);void ia_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){ia_note("IA_TRANSACTION_ASSERT: " #x);return 41;}}while(0)
typedef struct {
 uint32_t r[10];unsigned tx[3],ntx,reads,stops,violation,count,receiver,calls;
 uint8_t incoming[2];
} Model;
static uint32_t rd(void *p,unsigned reg){
 Model *m=p;m->calls++;
 if(reg==PI_SR2 && (m->r[PI_SR1]&PI_ADDR)){
  if(m->receiver && ((m->r[PI_CR1]&PI_ACK) || ((m->r[PI_CR1]&PI_POS)!=(m->count==2?PI_POS:0))))m->violation=1;
  m->r[PI_SR1]&=~PI_ADDR;
 }
 if(reg==PI_DR){if(m->reads>=m->count){m->violation=1;return 0;}return m->incoming[m->reads++];}
 return m->r[reg];
}
static void wr(void *p,unsigned reg,uint32_t value){
 Model *m=p;m->calls++;
 if(reg==PI_CR1){if(m->r[reg]&(PI_START|PI_STOP|PI_PEC))m->violation=1;if(value&PI_STOP)m->stops++;}
 if(reg==PI_SR1){m->r[reg]&=value|~0xdf00u;return;}
 if(reg==PI_DR){if(m->ntx==3){m->violation=1;return;}m->tx[m->ntx++]=value;m->r[PI_SR1]=0;return;}
 m->r[reg]=value;
}
static PiIo io(Model *m){PiIo x={m,rd,wr};return x;}
static int event(PiI2c *s,Model *m,unsigned sr,uint32_t now){
 m->r[PI_SR1]=sr;
 if(sr&PI_SB){m->r[PI_CR1]&=~PI_START;m->r[PI_SR2]=3;}
 return pi_step(s,io(m),s->token,now);
}
static int prefix(PiI2c *s,Model *m,uint32_t start){
 const unsigned events[]={PI_SB,PI_ADDR,PI_TXE,PI_BTF,PI_SB};
 for(unsigned i=0;i<5;i++){CHECK(event(s,m,events[i],start+i)==PI_BUSY);}
 CHECK(m->ntx==3 && m->tx[0]==0xa0 && m->tx[1]==7 && m->tx[2]==0xa1);
 return 0;
}
int ia_transaction(void){
 PuClock clock={16000000,1,1,1,1,0};
 for(unsigned count=1;count<=2;count++){
  PiI2c s={0};Model m={0};m.count=count;m.incoming[0]=0x5a;m.incoming[1]=0xc3;uint8_t out[2]={0xcc,0xcc};
  CHECK(pi_open(&s,io(&m),&clock)==PI_OK);
  CHECK(pi_begin(&s,io(&m),1,1,0x50,7,count,out,0,100)==PI_OK);
  CHECK(prefix(&s,&m,1)==0);m.receiver=1;
  CHECK(event(&s,&m,PI_ADDR,6)==PI_BUSY && !m.violation);
  if(count==2){CHECK(event(&s,&m,PI_RXNE,7)==PI_BUSY && !m.reads && !m.stops);CHECK(event(&s,&m,PI_BTF|PI_RXNE,8)==PI_BUSY);}
  else CHECK(event(&s,&m,PI_RXNE,8)==PI_BUSY);
  CHECK(m.reads==count && m.stops==1 && out[0]==0xcc && !m.violation);
  m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=0;
  CHECK(event(&s,&m,0,9)==PI_OK && out[0]==0x5a && out[1]==(count==1?0xcc:0xc3));
  ia_values("transaction",count,out[0],out[1],m.stops);
  /* New request has different output storage; an old completion may not even read registers. */
  uint8_t next[2]={0x11,0x22};m.receiver=m.reads=m.ntx=m.stops=0;
  CHECK(pi_begin(&s,io(&m),1,2,0x50,7,count,next,10,100)==PI_OK);
  unsigned calls=m.calls;unsigned phase=s.phase;
  CHECK(pi_step(&s,io(&m),1,11)==PI_STALE && m.calls==calls && s.phase==phase && next[0]==0x11 && next[1]==0x22);
  CHECK(prefix(&s,&m,11)==0);m.receiver=1;
  CHECK(event(&s,&m,PI_ADDR,16)==PI_BUSY);
  CHECK(event(&s,&m,count==1?PI_RXNE:PI_BTF|PI_RXNE,17)==PI_BUSY);
  m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=0;
  CHECK(event(&s,&m,0,18)==PI_OK && next[0]==0x5a && next[1]==(count==1?0x22:0xc3));
  ia_values("late",count,next[0],next[1],s.last_token);
 }
 return 0;
}
int ia_failures(void){
 PuClock clock={16000000,1,1,1,1,0};
 for(unsigned kind=0;kind<5;kind++){
  PiI2c s={0};Model m={0};uint8_t out=77;m.count=1;
  CHECK(pi_open(&s,io(&m),&clock)==PI_OK && pi_begin(&s,io(&m),1,1,0x50,7,1,&out,0,10)==PI_OK);
  CHECK(event(&s,&m,PI_SB,1)==PI_BUSY);
  int result=PI_BUSY;unsigned expected;
  if(kind==0 || kind==1 || kind==2){
   if(kind==1){CHECK(event(&s,&m,PI_ADDR,2)==PI_BUSY);CHECK(event(&s,&m,PI_TXE,3)==PI_BUSY);}
   if(kind==2){CHECK(event(&s,&m,PI_ADDR,2)==PI_BUSY);CHECK(event(&s,&m,PI_TXE,3)==PI_BUSY);CHECK(event(&s,&m,PI_BTF,4)==PI_BUSY);CHECK(event(&s,&m,PI_SB,5)==PI_BUSY);}
   expected=kind==1?PI_DATA_NACK:PI_ADDRESS_NACK;CHECK(event(&s,&m,PI_AF,6)==PI_BUSY && s.result==expected && m.stops==1);
   m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=0;result=event(&s,&m,0,7);
  }else if(kind==3){expected=PI_ARBITRATION;m.r[PI_SR2]=2;result=event(&s,&m,PI_ARLO|PI_AF,2);CHECK(m.stops==0);}
  else{
   expected=PI_TIMEOUT;CHECK(event(&s,&m,0,10)==PI_BUSY && s.result==expected);
   CHECK(event(&s,&m,0,11)==PI_BUSY && m.stops==1);m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=0;result=event(&s,&m,0,12);
  }
  CHECK(result==(int)expected && out==77 && !m.violation && !s.phase);
  ia_values("failure",kind,(unsigned)result,m.stops,out);
 }
 return 0;
}
int ia_late(void){
 PuClock clock={16000000,1,1,1,1,0};PiI2c s={0};Model m={0};uint8_t old=0x44,next=0x11;
 m.count=1;m.incoming[0]=0x5a;
 CHECK(pi_open(&s,io(&m),&clock)==PI_OK);
 CHECK(pi_begin(&s,io(&m),1,1,0x50,7,1,&old,0,100)==PI_OK);
 CHECK(prefix(&s,&m,1)==0);m.receiver=1;CHECK(event(&s,&m,PI_ADDR,6)==PI_BUSY);
 CHECK(event(&s,&m,PI_RXNE,7)==PI_BUSY);m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=0;
 CHECK(event(&s,&m,0,8)==PI_OK && old==0x5a);
 m.receiver=m.reads=m.ntx=m.stops=0;m.incoming[0]=0xc3;
 CHECK(pi_begin(&s,io(&m),1,2,0x50,7,1,&next,10,100)==PI_OK);
 CHECK(prefix(&s,&m,11)==0);m.receiver=1;CHECK(event(&s,&m,PI_ADDR,16)==PI_BUSY);
 CHECK(event(&s,&m,PI_RXNE,17)==PI_BUSY && next==0x11);
 m.r[PI_CR1]&=~PI_STOP;m.r[PI_SR2]=m.r[PI_SR1]=0;
 unsigned calls=m.calls;int result=pi_step(&s,io(&m),1,18);
 ia_values("late_publication",(unsigned)result,next,s.phase,m.calls-calls);
 CHECK(result==PI_STALE && next==0x11 && s.phase==9 && m.calls==calls);
 CHECK(pi_step(&s,io(&m),2,18)==PI_OK && next==0xc3 && old==0x5a);return 0;
}
typedef struct {unsigned release[2],held,after,pulses,writes;} Lines;
static unsigned lr(void *p,unsigned line){Lines *l=p;return l->release[line] && (line==PI_SCL?!l->held:l->pulses>=l->after);}
static void lw(void *p,unsigned line,unsigned release){Lines *l=p;l->writes++;if(line==PI_SCL && release && !l->release[line])l->pulses++;l->release[line]=release;}
int ia_recovery(void){
 for(unsigned after=0;after<=10;after++){
  PiRecovery r={0};Lines l={{1,1},0,after,0,0};PiLines io={&l,lr,lw};
  int status=ia_clear_begin(&r,io,1,0,UINT32_MAX-3u,200,2);CHECK(status==PI_OK);
  if(after==0){CHECK(!r.active && !l.writes);}
  else{
   status=PI_BUSY;
   for(unsigned t=0;t<200 && status==PI_BUSY;t++)status=pi_recover_step(&r,io,UINT32_MAX-3u+t);
   CHECK(status==(after==10?PI_IO:PI_OK) && r.pulses==(after==10?9:after));
  }
  CHECK(l.release[0] && l.release[1]);ia_values("recovery",after,(unsigned)status,r.pulses,l.writes);
 }
 PiRecovery r={0};Lines l={{1,1},1,3,0,0};PiLines io={&l,lr,lw};
 CHECK(ia_clear_begin(&r,io,0,0,0,30,2)==PI_FORBIDDEN && !l.writes);
 CHECK(ia_clear_begin(&r,io,1,1,0,30,2)==PI_FORBIDDEN && !l.writes);
 CHECK(ia_clear_begin(&r,io,1,0,0,30,2)==PI_OK);
 CHECK(pi_recover_step(&r,io,29)==PI_BUSY);
 CHECK(pi_recover_step(&r,io,30)==PI_TIMEOUT && r.pulses==0 && l.release[0] && l.release[1]);
 ia_values("held",PI_TIMEOUT,r.pulses,l.release[0],l.release[1]);return 0;
}
