#include "retry.h"
#ifndef SR_MUTANT
#define SR_MUTANT 0
#endif
static uint32_t mod(unsigned b){return b==8?256u:b==16?65536u:0u;}
int sr_timer_begin(SrTimer *t,unsigned b,uint32_t now,uint32_t budget){uint32_t m=mod(b);if(!t||!m||now>=m||!budget||budget>=m/2)return SR_INVALID;*t=(SrTimer){b,now,budget};return SR_OK;}
int sr_timer_step(SrTimer *t,uint32_t now,uint32_t *delta){
 if(!t||!delta||!mod(t->bits)||now>=mod(t->bits)||t->last>=mod(t->bits))return SR_INVALID;
 uint32_t d=0;if(ct_elapsed(t->bits,t->last,now,1,&d)!=CT_OK)return SR_INVALID;
 if(d>=mod(t->bits)/2)return SR_AMBIGUOUS;
 t->last=now;*delta=d;
 if(d>=t->remaining){t->remaining=0;return SR_TIMEOUT;}
 if(SR_MUTANT!=1)t->remaining-=d;
 return SR_OK;
}
int sr_byte_begin(SrByte *s,unsigned byte,unsigned b,uint32_t now,uint32_t budget,uint32_t stretch){
 if(!s||byte>255||!stretch||stretch>=mod(b)/2)return SR_INVALID;
 SrByte q={0};int r=sr_timer_begin(&q.timer,b,now,budget);if(r)return r;q.bit=(SI2c){byte,0,S_I2C_DATA};q.stretch=stretch;*s=q;return SR_OK;
}
int sr_byte_sample(SrByte *s,uint32_t now,unsigned scl,unsigned sda,unsigned edge){
 if(!s||scl>1||sda>1||edge>1)return SR_INVALID;
 if(s->terminal)return s->terminal;
 SrByte q=*s;uint32_t delta;int r=sr_timer_step(&q.timer,now,&delta);if(r==SR_AMBIGUOUS||r==SR_INVALID)return r;
 if(r==SR_TIMEOUT){q.terminal=SR_TIMEOUT;*s=q;return r;}
 if(q.low){q.low_ticks+=delta;if(q.low_ticks>=q.stretch&&SR_MUTANT!=2){q.terminal=SR_TIMEOUT;*s=q;return SR_TIMEOUT;}}
 if(!scl){q.low=1;*s=q;return SR_PENDING;}
 q.low=0;q.low_ticks=0;
 if(edge){r=s_i2c_sample(&q.bit,scl,sda);if(r==S_CONFLICT)return SR_INVALID;if(r!=S_OK)return SR_INVALID;}
 if(q.bit.state==S_I2C_LOST)q.terminal=SR_LOST;else if(q.bit.state==S_I2C_NACK)q.terminal=SR_NACK;else if(q.bit.state==S_I2C_DONE)q.terminal=SR_DONE;
 *s=q;return q.terminal?q.terminal:SR_PENDING;
}
int sr_txn_begin(SrTxn *s,uint32_t token,uint8_t addr,uint8_t reg,unsigned bits,uint32_t now,uint32_t budget){
 if(!s||!token||addr>127)return SR_INVALID;
 SrTxn q={0};int r=sr_timer_begin(&q.timer,bits,now,budget);if(r)return r;uint8_t action,wire;
 if(sd_i2c_begin(&q.bus,token,addr,reg,&action,&wire)!=SD_OK)return SR_INVALID;
 q.token=token;q.address=addr;q.wire=wire;q.action=action;q.bit=(SI2c){wire,0,S_I2C_DATA};*s=q;return SR_OK;
}
static int tick(SrTxn *q,uint32_t token,uint32_t now){
 if(token!=q->token)return SR_STALE;
 if(q->terminal)return q->terminal;
 uint32_t d;int r=sr_timer_step(&q->timer,now,&d);
 if(r==SR_TIMEOUT){uint8_t a=0,v=0;(void)sd_i2c_event(&q->bus,token,SD_I2C_TIMEOUT,0,&a,&v);q->action=a;q->terminal=r;}return r;
}
int sr_txn_bit(SrTxn *s,uint32_t token,uint32_t now,unsigned scl,unsigned sda){
 if(!s||scl>1||sda>1)return SR_INVALID;
 SrTxn q=*s;int r=tick(&q,token,now);if(r){if(r==SR_TIMEOUT)*s=q;return r;}if(q.action==SD_READ_NACK_STOP)return SR_INVALID;
 r=s_i2c_sample(&q.bit,scl,sda);if(r==S_CONFLICT)return SR_INVALID;if(r==S_PENDING){*s=q;return SR_PENDING;}if(r!=S_OK)return SR_INVALID;
 uint8_t action=0,value=0;
 if(q.bit.state==S_I2C_LOST){(void)sd_i2c_event(&q.bus,token,SD_I2C_ARB,0,&action,&value);q.action=action;q.terminal=SR_LOST;}
 else if(q.bit.state==S_I2C_NACK){(void)sd_i2c_event(&q.bus,token,SD_I2C_NACK,0,&action,&value);q.action=action;q.terminal=SR_NACK;}
 else if(q.bit.state==S_I2C_DONE){
  if(sd_i2c_event(&q.bus,token,SD_I2C_ACK,0,&action,&value)!=SD_OK)return SR_INVALID;
  q.action=action;if(action==SD_RESTART_READ)value=(uint8_t)((q.address<<1)|(SR_MUTANT==3?0u:1u));
  if(action!=SD_READ_NACK_STOP){q.wire=value;q.bit=(SI2c){value,0,S_I2C_DATA};}
 }
 *s=q;return q.terminal?q.terminal:q.action==SD_READ_NACK_STOP?SR_READY:SR_PENDING;
}
int sr_txn_read(SrTxn *s,uint32_t token,uint32_t now,uint8_t value,uint8_t *out){
 if(!s||!out)return SR_INVALID;
 SrTxn q=*s;int r=tick(&q,token,now);if(r){if(r==SR_TIMEOUT)*s=q;return r;}if(q.action!=SD_READ_NACK_STOP)return SR_INVALID;
 uint8_t action=0,v=0;if(sd_i2c_event(&q.bus,token,SD_I2C_BYTE,value,&action,&v)!=SD_OK)return SR_INVALID;q.action=action;q.terminal=SR_DONE;*s=q;*out=v;return SR_DONE;
}
int sr_retry_begin(SrRetry *s,unsigned bits,uint32_t now,uint32_t budget,uint32_t token,unsigned command,unsigned maximum,uint32_t bus_free,uint32_t backoff){
 if(!s||!token||(command!=0x10&&command!=0x11)||!maximum||maximum>16||token>UINT32_MAX-maximum||!bus_free||bus_free>=mod(bits)/2||backoff>=mod(bits)/2)return SR_INVALID;
 SrRetry q={0};int r=sr_timer_begin(&q.timer,bits,now,budget);if(r)return r;q.token=token;q.command=command;q.maximum=maximum;q.bus_free=bus_free;q.backoff=backoff;*s=q;return SR_OK;
}
int sr_retry_poll(SrRetry *s,uint32_t now,unsigned scl,unsigned sda,unsigned idle){
 if(!s||scl>1||sda>1||idle>1)return SR_INVALID;
 if(s->state>1)return (int)s->state;
 SrRetry q=*s;uint32_t d;int r=sr_timer_step(&q.timer,now,&d);if(r==SR_INVALID||r==SR_AMBIGUOUS)return r;if(r==SR_TIMEOUT){q.state=SR_TIMEOUT;*s=q;return r;}
 if(q.state==1){*s=q;return SR_PENDING;}
 q.wait_ticks+=d;
 if(scl&&sda&&(idle||SR_MUTANT==7)){if(q.free)q.free_ticks+=d;else {q.free=1;q.free_ticks=0;}}
 else if(SR_MUTANT!=4){q.free=0;q.free_ticks=0;}
 if(q.free&&q.free_ticks>=q.bus_free&&q.wait_ticks>=q.backoff){q.attempts++;q.token++;q.state=1;*s=q;return SR_READY;}
 *s=q;return SR_PENDING;
}
int sr_retry_complete(SrRetry *s,uint32_t token,uint32_t now,unsigned status,unsigned applied){
 if(!s||applied>1)return SR_INVALID;
 if(token!=s->token)return SR_STALE;
 if(s->state!=1)return SR_INVALID;
 int policy=pi_retry_policy(s->command,applied,status);if(policy==PI_INVALID)return SR_INVALID;
 SrRetry q=*s;uint32_t d;int r=sr_timer_step(&q.timer,now,&d);if(r==SR_INVALID||r==SR_AMBIGUOUS)return r;
 if(r==SR_TIMEOUT)q.state=SR_TIMEOUT;
 else if(policy==PI_OK)q.state=SR_DONE;
 else if(policy==PI_UNCERTAIN&&SR_MUTANT!=5)q.state=SR_UNCERTAIN;
 else if(q.attempts==q.maximum&&SR_MUTANT!=6)q.state=SR_EXHAUSTED;
 else{q.state=0;q.free=q.free_ticks=q.wait_ticks=0;}
 *s=q;return q.state?(int)q.state:SR_PENDING;
}
