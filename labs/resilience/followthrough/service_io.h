#ifndef CMASTERY_RESILIENCE_SERVICE_IO_H
#define CMASTERY_RESILIENCE_SERVICE_IO_H
#include "outbox.h"
#include "breaker.h"
#include "fractional.h"
#define RZV_FRAME_BYTES 18u
typedef struct {unsigned char bytes[RZV_FRAME_BYTES];size_t used;bool malformed;} RzvFrame;
typedef struct {unsigned status;bool duplicate;int result;uint64_t epoch;} RzvReply;
int rzv_request_encode(RzjRequest,unsigned char [RZV_FRAME_BYTES]);
void rzv_frame_init(RzvFrame *);
int rzv_frame_feed(RzvFrame *,const unsigned char *,size_t,size_t *,RzjRequest *);
int rzv_frame_end(RzvFrame *);
int rzv_reply_decode(const unsigned char [RZV_FRAME_BYTES],RzvReply *);
int rzv_reply_encode(RzvReply,unsigned char [RZV_FRAME_BYTES]);
typedef struct {
    pthread_mutex_t mutex;
    RzsOutbox *outbox;
    RzbGate gate;
    RzfBucket limiter;
    unsigned active;
    bool closed;
} RzvService;
typedef struct {size_t read,written;int admission,effect,result;bool charged,committed,duplicate,cleaned;} RzvObservation;
int rzv_now(uint64_t *);
int rzv_init(RzvService *,RzsOutbox *,unsigned,uint64_t);
int rzv_serve(RzvService *,RzfScope *,int,int,size_t,RzvObservation *);
int rzv_close(RzvService *);
int rzv_destroy(RzvService *);
int rzv_notify(RzvService *,unsigned,unsigned,RzsEvent *);
#endif
