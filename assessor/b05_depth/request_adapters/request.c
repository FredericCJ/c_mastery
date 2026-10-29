#include "request.h"
int rq_owned(const struct b5_rq *r){return r->phase>=RQ_INFLIGHT&&r->phase<=RQ_RESETTING;}
int rq_apply(struct b5_rq *r,unsigned op,uint32_t generation,int result){
  if(!r||op>RQ_REAP)return RQ_INVALID;
  if(op==RQ_SUBMIT){
    if(r->phase!=RQ_FREE)return RQ_BUSY;
    if(generation!=r->generation)return RQ_STALE;
    r->phase=RQ_INFLIGHT;r->result=RQ_UNCERTAIN;r->terminals=0;return RQ_OK;
  }
  if(generation!=r->generation)return RQ_STALE;
  switch(op){
  case RQ_CANCEL:
    if(r->phase!=RQ_INFLIGHT&&r->phase!=RQ_CANCELLED)return RQ_BUSY;
#ifdef RQ_MUTANT_CANCEL
    r->phase=RQ_DONE;
#else
    r->phase=RQ_CANCELLED;
#endif
    return RQ_OK;
  case RQ_TIMEOUT:
    if(r->phase!=RQ_INFLIGHT&&r->phase!=RQ_CANCELLED)return RQ_BUSY;
    r->phase=RQ_UNKNOWN;return RQ_OK;
  case RQ_COMPLETE:
    if(r->phase!=RQ_INFLIGHT&&r->phase!=RQ_CANCELLED&&r->phase!=RQ_UNKNOWN)return RQ_BUSY;
    r->result=result;r->phase=RQ_DONE;r->terminals++;return RQ_OK;
  case RQ_RESET_BEGIN:
    if(!rq_owned(r)||r->phase==RQ_RESETTING)return RQ_BUSY;
    r->phase=RQ_RESETTING;
#ifdef RQ_MUTANT_RESET
    r->phase=RQ_DONE;
#endif
    return RQ_OK;
  case RQ_RESET_ACK:
    if(r->phase!=RQ_RESETTING)return RQ_BUSY;
    r->phase=RQ_DONE;r->result=RQ_UNCERTAIN;r->terminals++;return RQ_OK;
  case RQ_REAP:
    if(r->phase!=RQ_DONE)return RQ_BUSY;
    if(r->generation==UINT32_MAX)r->phase=RQ_RETIRED;
    else{r->generation++;r->phase=RQ_FREE;}
    return RQ_OK;
  default:return RQ_INVALID;
  }
}
