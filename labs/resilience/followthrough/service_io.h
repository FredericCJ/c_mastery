#ifndef CMASTERY_RESILIENCE_SERVICE_IO_H
#define CMASTERY_RESILIENCE_SERVICE_IO_H
#include "outbox.h"
#include "breaker.h"
#include "fractional.h"
#define RZV_FRAME_BYTES 18u
struct rzv_frame {unsigned char bytes[RZV_FRAME_BYTES];size_t used;bool malformed;};
struct rzv_reply {unsigned status;bool duplicate;int result;uint64_t epoch;};
int rzv_request_encode(struct rzj_request,unsigned char [RZV_FRAME_BYTES]);
void rzv_frame_init(struct rzv_frame *);
int rzv_frame_feed(struct rzv_frame *,const unsigned char *,size_t,size_t *,struct rzj_request *);
int rzv_frame_end(struct rzv_frame *);
int rzv_reply_decode(const unsigned char [RZV_FRAME_BYTES],struct rzv_reply *);
int rzv_reply_encode(struct rzv_reply,unsigned char [RZV_FRAME_BYTES]);
struct rzv_service {
    pthread_mutex_t mutex;
    struct rzs_outbox *outbox;
    struct rzb_gate gate;
    struct rzf_bucket limiter;
    unsigned active;
    bool closed;
};
struct rzv_observation {size_t read,written;int admission,effect,result;bool charged,committed,duplicate,cleaned;};
int rzv_now(uint64_t *);
int rzv_init(struct rzv_service *,struct rzs_outbox *,unsigned,uint64_t);
int rzv_serve(struct rzv_service *,struct rzf_scope *,int,int,size_t,struct rzv_observation *);
int rzv_close(struct rzv_service *);
int rzv_destroy(struct rzv_service *);
int rzv_notify(struct rzv_service *,unsigned,unsigned,struct rzs_event *);
#endif
