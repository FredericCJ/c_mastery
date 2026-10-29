#ifndef CMASTERY_MESSAGING_ACTOR_EXECUTOR_H
#define CMASTERY_MESSAGING_ACTOR_EXECUTOR_H
#include "owned.h"
struct mfe_executor;
struct mfe_envelope {
    struct mfe_executor *owner;struct mfo_value value;uint64_t generation;unsigned destination;bool used;
};
struct mfe_executor {
    struct mfc_hub input;struct mfa_actor actor[2];pthread_mutex_t pool;
    struct mfe_envelope envelopes[8];uint64_t published[2];
};
struct mfe_event {unsigned destination;uint64_t generation;enum msg_status delivery;};
enum msg_status mfe_init(struct mfe_executor *,size_t,size_t,mfa_handler_fn,void *[2],mfa_dead_letter_fn,void *);
enum msg_status mfe_send(struct mfe_executor *,unsigned,uint64_t,struct mfo_value *,bool);
enum msg_status mfe_pump(struct mfe_executor *,bool,struct mfe_event *);
enum msg_status mfe_generation(struct mfe_executor *,unsigned,uint64_t *);
enum msg_status mfe_restart(struct mfe_executor *,unsigned);
enum msg_status mfe_close(struct mfe_executor *,unsigned);
void mfe_cancel(struct mfe_executor *);
enum msg_status mfe_destroy(struct mfe_executor *);
#endif
