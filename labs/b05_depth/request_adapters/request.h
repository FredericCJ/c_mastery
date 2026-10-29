#ifndef REQUEST_RESET_H
#define REQUEST_RESET_H
#include <stdint.h>
enum { RQ_FREE, RQ_INFLIGHT, RQ_CANCELLED, RQ_UNKNOWN, RQ_RESETTING,
       RQ_DONE, RQ_RETIRED };
enum { RQ_SUBMIT, RQ_CANCEL, RQ_TIMEOUT, RQ_COMPLETE, RQ_RESET_BEGIN,
       RQ_RESET_ACK, RQ_REAP };
enum { RQ_OK=0, RQ_BUSY=-20, RQ_STALE=-21, RQ_INVALID=-22,
       RQ_UNCERTAIN=-23, RQ_UNIMPLEMENTED=-29 };
struct b5_rq { uint32_t generation,phase; int result; unsigned terminals; };
/* Zero initialized; serialized calls; only the trusted adapter supplies COMPLETE
 * and RESET_ACK. Error leaves all fields unchanged. No byte/DMA access here. */
int rq_apply(struct b5_rq *r,unsigned operation,uint32_t generation,int result);
int rq_owned(const struct b5_rq *r);
#endif
