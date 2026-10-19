#ifndef CMASTERY_MESSAGING_REGISTRATION_H
#define CMASTERY_MESSAGING_REGISTRATION_H
#include "messaging.h"
#include <time.h>

typedef struct MfrWaiter MfrWaiter;
typedef struct {
    pthread_mutex_t channel_mutex[2], registry;
    pthread_cond_t changed;
    MsgChannel channel[2];
    MfrWaiter *waiters;
    size_t waiting;
    uint64_t sleeps;
    unsigned next;
    bool stop;
} MfrHub;
typedef void (*MfrProbe)(void *);
MsgStatus mfr_init(MfrHub *,size_t,size_t);
MsgStatus mfr_send(MfrHub *,unsigned,unsigned);
MsgStatus mfr_select(MfrHub *,unsigned,bool,unsigned *,unsigned *);
MsgStatus mfr_select_probe(MfrHub *,unsigned,unsigned *,unsigned *,
                           const struct timespec *,MfrProbe,void *);
MsgStatus mfr_close(MfrHub *,unsigned);
void mfr_cancel(MfrHub *);
void mfr_observe(MfrHub *,size_t *,uint64_t *);
MsgStatus mfr_destroy(MfrHub *);
#endif
