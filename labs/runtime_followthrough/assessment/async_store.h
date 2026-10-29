#ifndef CMASTERY_VM_ASYNC_STORE_H
#define CMASTERY_VM_ASYNC_STORE_H
#include "execution.h"
#include "services.h"
#include <pthread.h>

enum
{
    VA_READY = 64,
    VA_BUSY,
    VA_STATE,
    VA_UNIMPLEMENTED
};

struct vg_va
{
    pthread_mutex_t mutex;
    pthread_t worker;
    struct vg_vs *session;
    struct vs_request request;
    struct vg_vh host;
    int32_t result;
    int status;
    unsigned initialized, active, done, cancelled, joined;
};

/* init requires uninitialized storage (never a live mutex); destroy before
 * reinit. One owner thread, stable address until destroy. submit borrows a
 * pending Vs and exclusively borrows h.database until poll has joined the
 * worker. The worker receives a copied request/config; it never accesses Vs
 * itself. Caller retains all I/O context and descriptor ownership during that
 * borrow. cancel requests result suppression; it cannot roll back a running
 * transaction or interrupt arbitrary kernel I/O. poll returns BUSY until done,
 * then joins before calling vs_complete once. A failure may have persisted an
 * effect.
 */
int va_init(struct vg_va *);
int va_submit(struct vg_va *, struct vg_vs *, const struct vg_vh *);
int va_cancel(struct vg_va *);
int va_poll(struct vg_va *);
int va_destroy(struct vg_va *);
#endif
