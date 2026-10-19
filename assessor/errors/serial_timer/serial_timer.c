#include "serial_timer.h"
#ifndef EAT_MUTATION
#define EAT_MUTATION 0
#endif
static int width(unsigned bits){return bits>=2&&bits<=16;}
static uint32_t modulus(unsigned bits){return UINT32_C(1)<<bits;}
er_code eat_serial(unsigned bits,uint32_t a,uint32_t b,int *order){
 if(!width(bits)||!order)return ER_INVALID;
 uint32_t m=modulus(bits);if(a>=m||b>=m)return ER_INVALID;
 if(bits==16)return er_serial16(a,b,order);
 uint32_t d=(a+m-b)&(m-1);if(d==m/2)return ER_RANGE;
 *order=d==0?0:d<m/2?1:-1;return ER_OK;
}
er_code eat_window_init(EatWindow *q,unsigned bits,uint32_t now){
 if(!q||!width(bits)||now>=modulus(bits))return ER_INVALID;
 *q=(EatWindow){.bits=bits,.now=now};return ER_OK;
}
er_code eat_window_arm(EatWindow *q,uint32_t id,uint32_t delay){
 if(!q)return ER_INVALID;
 uint32_t m=modulus(q->bits);if(delay>=m/2)return ER_RANGE;
 for(size_t i=0;i<q->count;++i)if(q->entry[i].id==id)return ER_INVALID;
 if(q->count==4)return ER_RANGE;
 q->entry[q->count++]=(EatWindowEntry){id,(q->now+delay)&(m-1),0};return ER_OK;
}
er_code eat_window_advance(EatWindow *q,uint32_t elapsed){
 if(!q)return ER_INVALID;
 uint32_t m=modulus(q->bits);if(elapsed>=m/2)return ER_RANGE;
 if(EAT_MUTATION==1)q->now=(q->now+elapsed)&(m-1);
 for(size_t i=0;i<q->count;++i)if(EAT_MUTATION!=2&&q->entry[i].age+elapsed>=m/2)return ER_RANGE;
 if(EAT_MUTATION!=1)q->now=(q->now+elapsed)&(m-1);
 for(size_t i=0;i<q->count;++i)q->entry[i].age+=elapsed;
 return ER_OK;
}
er_code eat_window_pop(EatWindow *q,uint32_t *id){
 if(!q||!id)return ER_INVALID;
 uint32_t m=modulus(q->bits),late=0;size_t selected=q->count;
 for(size_t i=0;i<q->count;++i){int order=0;er_code code=eat_serial(q->bits,q->now,q->entry[i].deadline,&order);if(code!=ER_OK)return code;
  if(EAT_MUTATION==3)order=q->now>=q->entry[i].deadline?1:-1;
  uint32_t lag=(q->now+m-q->entry[i].deadline)&(m-1);
  if(order>=0&&(selected==q->count||lag>late||(EAT_MUTATION==4&&lag==late))){selected=i;late=lag;}
 }
 if(selected==q->count)return ER_AGAIN;
 uint32_t value=q->entry[selected].id;for(size_t i=selected+1;i<q->count;++i)q->entry[i-1]=q->entry[i];--q->count;*id=value;return ER_OK;
}
er_code eat_epoch_init(EatEpoch *q,unsigned bits,uint64_t now){
 if(!q||!width(bits))return ER_INVALID;
 *q=(EatEpoch){.bits=bits,.now=now};return ER_OK;
}
er_code eat_epoch_arm(EatEpoch *q,uint32_t id,uint64_t delay){
 if(!q)return ER_INVALID;
 for(size_t i=0;i<q->count;++i)if(q->entry[i].id==id)return ER_INVALID;
 if(q->count==4||delay>UINT64_MAX-q->now)return ER_RANGE;
 q->entry[q->count++]=(EatEpochEntry){id,q->now+delay};return ER_OK;
}
er_code eat_epoch_advance(EatEpoch *q,uint64_t elapsed){
 if(!q)return ER_INVALID;
 if(EAT_MUTATION==5)q->now+=elapsed;
 if(EAT_MUTATION!=5&&elapsed>UINT64_MAX-q->now)return ER_RANGE;
 if(EAT_MUTATION!=5)q->now+=elapsed;
 return ER_OK;
}
er_code eat_epoch_pop(EatEpoch *q,uint32_t *id){
 if(!q||!id)return ER_INVALID;
 size_t selected=q->count;uint64_t first=UINT64_MAX;
 for(size_t i=0;i<q->count;++i){int due=q->entry[i].deadline<=q->now;
  if(EAT_MUTATION==6){int order=0;uint32_t mask=modulus(q->bits)-1;(void)eat_serial(q->bits,(uint32_t)(q->now&mask),(uint32_t)(q->entry[i].deadline&mask),&order);due=order>=0;}
  if(due&&(selected==q->count||q->entry[i].deadline<first)){selected=i;first=q->entry[i].deadline;}
 }
 if(selected==q->count)return ER_AGAIN;
 uint32_t value=q->entry[selected].id;for(size_t i=selected+1;i<q->count;++i)q->entry[i-1]=q->entry[i];--q->count;*id=value;return ER_OK;
}
