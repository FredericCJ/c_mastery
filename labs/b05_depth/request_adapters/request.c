#include "request.h"
/* Learner scaffold: implement the contract and its trusted-event boundary. */
int rq_owned(const Rq *r){(void)r;return 0;}
int rq_apply(Rq *r,unsigned op,uint32_t generation,int result){
  (void)r;(void)op;(void)generation;(void)result;return RQ_UNIMPLEMENTED;
}
