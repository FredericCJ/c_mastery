#include "request.h"
/* Learner scaffold: implement the contract and its trusted-event boundary. */
int rq_owned(const struct b5_rq *r){(void)r;return 0;}
int rq_apply(struct b5_rq *r,unsigned op,uint32_t generation,int result){
  (void)r;(void)op;(void)generation;(void)result;return RQ_UNIMPLEMENTED;
}
