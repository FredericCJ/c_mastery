#ifndef CMASTERY_MESSAGING_STREAM_FOLLOW_H
#define CMASTERY_MESSAGING_STREAM_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
enum mfq_policy {MFQ_WAIT,MFQ_DROP,MFQ_FAIL};
struct mfq_stats {
    uint64_t accepted,delivered,discarded,rejected;
    unsigned queued,inflight,handles,waiting;
    bool accepting,finished,joined;
};
struct mfq_stream {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    pthread_t worker;
    struct msg_hub channel;
    struct msg_stream core;
    struct mfo_value context;
    msg_next_fn next;
    msg_terminal_fn terminal;
    uint64_t requests;
    bool cancel_pending,invalid_pending,end_pending,idle;
    enum msg_status end_status;
    unsigned capacity;
    struct mfq_stats stats;
};
struct mfq_handle {struct mfq_stream *owner;};
enum msg_status mfq_init(struct mfq_stream *,unsigned,struct mfo_value *,msg_next_fn,msg_terminal_fn,bool);
enum msg_status mfq_acquire(struct mfq_stream *,struct mfq_handle *);
void mfq_release(struct mfq_handle *);
enum msg_status mfq_request(struct mfq_handle *,uint64_t);
enum msg_status mfq_submit(struct mfq_handle *,unsigned,enum mfq_policy);
enum msg_status mfq_end(struct mfq_handle *,enum msg_status);
enum msg_status mfq_cancel(struct mfq_handle *);
enum msg_status mfq_close(struct mfq_stream *);
enum msg_status mfq_join(struct mfq_stream *);
enum msg_status mfq_idle(struct mfq_stream *);
enum msg_status mfq_stats(struct mfq_stream *,struct mfq_stats *);
enum msg_status mfq_destroy(struct mfq_stream *);
#endif
