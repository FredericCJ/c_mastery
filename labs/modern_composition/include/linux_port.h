#ifndef CMASTERY_MC_LINUX_PORT_H
#define CMASTERY_MC_LINUX_PORT_H
#include "composition.h"
#include <pthread.h>
/* One owner thread calls every function; a private worker only publishes a
   mutex-protected notification. Neither its borrow nor a queued notification
   is released by a cancellation request. Stable address until destroy. */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    pthread_t worker;
    McRead *read;
    int fd, wake[2];
    unsigned initialized, started, stopping, notification, references;
    unsigned worker_exits, queued, consumed, discarded, joined;
    uint32_t generation;
} McLinuxPort;
int mc_linux_init(McLinuxPort *, McRead *, int nonblocking_fd);
COAsyncPort mc_linux_interface(McLinuxPort *);
int mc_linux_wait(McLinuxPort *, unsigned timeout_ms);
int mc_linux_dispatch(McLinuxPort *);
int mc_linux_request_cancel(McLinuxPort *);
MsgStatus mc_linux_finish_cancel(McLinuxPort *);
int mc_linux_quiescent(McLinuxPort *);
int mc_linux_destroy(McLinuxPort *);
#endif
