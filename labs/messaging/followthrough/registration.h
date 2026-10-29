#ifndef CMASTERY_MESSAGING_REGISTRATION_H
#define CMASTERY_MESSAGING_REGISTRATION_H
#include "messaging.h"
#include <time.h>

struct mfr_waiter;

struct mfr_hub
{
    pthread_mutex_t channel_mutex[2], registry;
    pthread_cond_t changed;
    struct msg_channel channel[2];
    struct mfr_waiter *waiters;
    size_t waiting;
    uint64_t sleeps;
    unsigned next;
    bool stop;
};

typedef void (*mfr_probe_fn)(void *);
enum msg_status mfr_init(struct mfr_hub *, size_t, size_t);
enum msg_status mfr_send(struct mfr_hub *, unsigned, unsigned);
enum msg_status mfr_select(struct mfr_hub *, unsigned, bool, unsigned *,
                           unsigned *);
enum msg_status mfr_select_probe(struct mfr_hub *, unsigned, unsigned *,
                                 unsigned *, const struct timespec *,
                                 mfr_probe_fn, void *);
enum msg_status mfr_close(struct mfr_hub *, unsigned);
void mfr_cancel(struct mfr_hub *);
void mfr_observe(struct mfr_hub *, size_t *, uint64_t *);
enum msg_status mfr_destroy(struct mfr_hub *);
#endif
