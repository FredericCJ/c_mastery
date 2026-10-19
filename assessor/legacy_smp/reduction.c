#include "reduction.h"
#include <limits.h>
#ifndef SR_MUTANT
#define SR_MUTANT 0
#endif
_Static_assert(SIZE_MAX<=UINT64_MAX,"Selected counter representation needs size_t no wider than uint64_t");
int sr_partition(size_t n,unsigned workers,unsigned worker,SrRange *out){
 if(!workers||workers>SR_WORKERS||worker>=workers||!out)return SR_INVALID;
 size_t q=n/workers,r=n%workers,begin=q*worker+(worker<r?worker:r),end=begin+q+(worker<r);
 if(SR_MUTANT==1&&worker&&begin)begin--;
 if(SR_MUTANT==2&&worker+1==workers&&r&&end>begin)end--;
 *out=(SrRange){begin,end};return SR_OK;
}
int sr_init(SrWorld *w,const void *input,atomic_uint *visits,size_t count,unsigned workers,unsigned mode){
 if(!w||!workers||workers>SR_WORKERS||mode>SR_BYTES||(count&&(!input||!visits)))return SR_INVALID;
 w->input=input;w->visits=visits;w->count=count;w->workers=workers;w->mode=mode;
 for(unsigned i=0;i<SR_WORKERS;i++){atomic_init(&w->slot[i].state,0);w->slot[i].error=SR_OK;w->slot[i].value=(SrResult){0};}
 for(size_t i=0;i<count;i++)atomic_init(visits+i,0);
 return SR_OK;
}
static int sr_signed(int64_t a,int64_t b,int64_t *out){if((b>0&&a>INT64_MAX-b)||(b<0&&a<INT64_MIN-b))return SR_OVERFLOW;*out=a+b;return SR_OK;}
static int sr_unsigned(uint64_t a,uint64_t b,uint64_t *out){if(SR_MUTANT!=3&&a>UINT64_MAX-b)return SR_OVERFLOW;*out=a+b;return SR_OK;}
int sr_work(SrWorld *w,unsigned worker,size_t fail_after){
 if(!w||worker>=w->workers)return SR_INVALID;
 SrRange range;if(sr_partition(w->count,w->workers,worker,&range)!=SR_OK)return SR_INVALID;
 SrSlot *slot=&w->slot[worker];unsigned expected=0;
 if(!atomic_compare_exchange_strong_explicit(&slot->state,&expected,1,memory_order_acquire,memory_order_relaxed))return SR_BUSY;
 SrResult value={0};value.power=1;unsigned error=SR_OK;
 for(size_t i=range.begin;i<range.end;i++){
  if(fail_after!=SIZE_MAX&&value.count==fail_after){error=SR_FAILED;break;}
  atomic_fetch_add_explicit(w->visits+i,1,memory_order_relaxed);
  if(w->mode==SR_UNSIGNED){if(sr_unsigned(value.unsigned_sum,((const uint64_t *)w->input)[i],&value.unsigned_sum)!=SR_OK){error=SR_OVERFLOW;break;}}
  else if(w->mode==SR_SIGNED){if(sr_signed(value.signed_sum,((const int64_t *)w->input)[i],&value.signed_sum)!=SR_OK){error=SR_OVERFLOW;break;}}
  else{unsigned byte=((const uint8_t *)w->input)[i];if(byte>=SR_BINS){error=SR_INVALID;break;}value.bins[byte]++;value.hash=value.hash*UINT64_C(257)+byte+1;value.power*=UINT64_C(257);}
  value.count++;
 }
 if(error==SR_OK&&fail_after!=SIZE_MAX&&value.count==fail_after)error=SR_FAILED;
 slot->value=value;slot->error=error;atomic_store_explicit(&slot->state,error==SR_OK?2:3,memory_order_release);return (int)error;
}
int sr_finish(const SrWorld *w,SrResult *out){
 if(!w||!out||!w->workers||w->workers>SR_WORKERS)return SR_INVALID;
 for(unsigned i=0;i<w->workers;i++){unsigned state=atomic_load_explicit(&w->slot[i].state,memory_order_acquire);if(state<2&&SR_MUTANT!=5)return SR_INCOMPLETE;}
 SrResult result={0};result.power=1;
 for(unsigned i=0;i<w->workers;i++){
  unsigned selected=SR_MUTANT==6?w->workers-1-i:i;const SrSlot *slot=&w->slot[selected];
  /* The early-completion control reads no concurrently writable plain field. */
  if(SR_MUTANT==5&&atomic_load_explicit(&slot->state,memory_order_acquire)<2)continue;
  if(slot->error!=SR_OK){if(SR_MUTANT==4)continue;return (int)slot->error;}
  const SrResult *value=&slot->value;
  if(SIZE_MAX-result.count<value->count)return SR_OVERFLOW;
  result.count+=value->count;
  if(w->mode==SR_UNSIGNED){if(sr_unsigned(result.unsigned_sum,value->unsigned_sum,&result.unsigned_sum)!=SR_OK)return SR_OVERFLOW;}
  else if(w->mode==SR_SIGNED){if(sr_signed(result.signed_sum,value->signed_sum,&result.signed_sum)!=SR_OK)return SR_OVERFLOW;}
  else{result.hash=result.hash*value->power+value->hash;result.power*=value->power;for(unsigned bin=0;bin<SR_BINS;bin++)if(sr_unsigned(result.bins[bin],value->bins[bin],&result.bins[bin])!=SR_OK)return SR_OVERFLOW;}
 }
 if(SR_MUTANT!=4&&SR_MUTANT!=5){if(result.count!=w->count)return SR_FAILED;for(size_t i=0;i<w->count;i++)if(atomic_load_explicit(w->visits+i,memory_order_relaxed)!=1)return SR_FAILED;}
 *out=result;return SR_OK;
}
