#include "release.h"
#ifndef DT_MUTANT
#define DT_MUTANT 0
#endif
static int valid(const DtTrace*q){return q&&q->head<3&&q->count<=3;}
int dt_emit(DtTrace*q,uint32_t kind,uint32_t timestamp,uint32_t payload){
 if(!valid(q))return DT_INVALID;
 if(q->next==UINT32_MAX)return DT_EXHAUSTED;
 if(q->count==3){if(q->dropped==UINT32_MAX)return DT_EXHAUSTED;q->dropped++;if(DT_MUTANT!=1)q->next++;return DT_FULL;}
 unsigned at=(q->head+q->count)%3;q->records[at]=(DtRecord){q->next,kind,timestamp,payload};q->next++;q->count++;return DT_OK;
}
int dt_take(DtTrace*q,DtRecord*out){
 if(!valid(q)||!out)return DT_INVALID;
 if(!q->count)return DT_EMPTY;
 *out=q->records[q->head];q->head=(q->head+1)%3;q->count--;return DT_OK;
}
int dt_irq_emit(DtTrace*q,DtCritical c,uint32_t k,uint32_t t,uint32_t p){
 if(!q||!c.enter||!c.leave)return DT_INVALID;
 uint32_t saved=c.enter(c.context);int result=dt_emit(q,k,t,p);c.leave(c.context,DT_MUTANT==2?0:saved);return result;
}
int dt_irq_take(DtTrace*q,DtCritical c,DtRecord*out){
 if(!q||!out||!c.enter||!c.leave)return DT_INVALID;
 uint32_t saved=c.enter(c.context);int result=dt_take(q,out);c.leave(c.context,saved);return result;
}
int dt_duration(const DtInterval*v,size_t n,DtDuration*out){
 DtDuration r={0};if(!out||!v||!n||n>8)return DT_INVALID;
 for(size_t i=0;i<n;i++){
  if(!v[i].width||v[i].width>32||!v[i].frequency)return DT_INVALID;
  uint32_t mask=v[i].width==32?UINT32_MAX:((UINT32_C(1)<<v[i].width)-1);
  if(v[i].before>mask||v[i].after>mask)return DT_INVALID;
  uint32_t ticks=(v[i].after-v[i].before)&mask;
  uint64_t numer=(uint64_t)ticks*UINT64_C(1000000000),lo=numer/v[i].frequency,hi=lo+(numer%v[i].frequency!=0);
  if(DT_MUTANT==3&&i)lo=hi=0;
  if(lo>UINT64_MAX-r.lower_ns||hi>UINT64_MAX-r.upper_ns)return DT_RANGE;
  r.lower_ns+=lo;r.upper_ns+=hi;
 }
 *out=r;return DT_OK;
}
int dt_image(uint32_t base,uint32_t bytes,uint32_t offset,DtImage*out){
 if(!out||bytes<8||offset>bytes-8||base&3u||offset&3u)return DT_INVALID;
 if(bytes>UINT32_MAX-base)return DT_RANGE;
 DtImage r={base+offset,base+bytes};*out=r;return DT_OK;
}
int dt_record(DtRetained*r,uint32_t image,uint32_t code){
 if(!r||!image)return DT_INVALID;
 *r=(DtRetained){1,0x44545231u,image,code,image^code^0x72657431u};return DT_OK;
}
int dt_startup(DtRam*r,const unsigned char initial[8],unsigned warm,uint32_t image){
 if(!r||!initial||warm>1||!image)return DT_INVALID;
 DtRetained saved=r->crash;
 if(!warm||saved.version!=1||saved.valid!=0x44545231u||saved.image!=image||saved.check!=(saved.image^saved.code^0x72657431u))saved=(DtRetained){0};
 for(unsigned i=0;i<8;i++){r->data[i]=initial[i];if(DT_MUTANT!=4)r->bss[i]=0;}
 r->crash=saved;return DT_OK;
}
