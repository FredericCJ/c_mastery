#ifndef REQUEST_RESET_H
#define REQUEST_RESET_H
#include <stdint.h>
enum { RQ_FREE, RQ_INFLIGHT, RQ_CANCELLED, RQ_UNKNOWN, RQ_RESETTING,
       RQ_DONE, RQ_RETIRED };
enum { RQ_SUBMIT, RQ_CANCEL, RQ_TIMEOUT, RQ_COMPLETE, RQ_RESET_BEGIN,
       RQ_RESET_ACK, RQ_REAP };
enum { RQ_OK=0, RQ_BUSY=-20, RQ_STALE=-21, RQ_INVALID=-22,
       RQ_UNCERTAIN=-23, RQ_UNIMPLEMENTED=-29 };
typedef struct { uint32_t generation,phase; int result; unsigned terminals; } Rq;
/* Zero initialized; serialized calls; only the trusted adapter supplies COMPLETE
 * and RESET_ACK. Error leaves all fields unchanged. No byte/DMA access here. */
int rq_apply(Rq *r,unsigned operation,uint32_t generation,int result);
int rq_owned(const Rq *r);
#endif
