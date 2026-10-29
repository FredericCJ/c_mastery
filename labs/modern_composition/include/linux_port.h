#ifndef CMASTERY_MC_LINUX_PORT_H
#define CMASTERY_MC_LINUX_PORT_H
#include "composition.h"
#include <pthread.h>
/* One owner thread calls every function; a private worker only publishes a
   mutex-protected notification. Neither its borrow nor a queued notification
   is released by a cancellation request. Stable address until destroy. */
struct mc_linux_port {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    pthread_t worker;
    struct mc_read *read;
    int fd, wake[2];
    unsigned initialized, started, stopping, notification, references;
    unsigned worker_exits, queued, consumed, discarded, joined;
    uint32_t generation;
};
int mc_linux_init(struct mc_linux_port *, struct mc_read *, int nonblocking_fd);
struct co_async_port mc_linux_interface(struct mc_linux_port *);
int mc_linux_wait(struct mc_linux_port *, unsigned timeout_ms);
int mc_linux_dispatch(struct mc_linux_port *);
int mc_linux_request_cancel(struct mc_linux_port *);
enum msg_status mc_linux_finish_cancel(struct mc_linux_port *);
int mc_linux_quiescent(struct mc_linux_port *);
int mc_linux_destroy(struct mc_linux_port *);
#endif
