#include "waiting.h"
#include <errno.h>

/* Intentionally unfinished. Tests compile before rejecting the missing
 * behavior. */
int tw_bank_init(struct tw_bank *p)
{
    (void)p;
    return ENOSYS;
}

int tw_transfer(struct tw_bank *p, int n, tw_hook_fn h, void *a)
{
    (void)p;
    (void)n;
    (void)h;
    (void)a;
    return ENOSYS;
}

int tw_bank_total(struct tw_bank *p)
{
    (void)p;
    return -1;
}

void tw_bank_destroy(struct tw_bank *p)
{
    (void)p;
}

void tw_snapshot_init(struct tw_snapshot *p)
{
    (void)p;
}

int tw_snapshot_write(struct tw_snapshot *p, int n)
{
    (void)p;
    (void)n;
    return ENOSYS;
}

int tw_snapshot_read(struct tw_snapshot *p, int *n)
{
    (void)p;
    (void)n;
    return ENOSYS;
}

int tw_key_next(uint64_t *p, uint64_t *q)
{
    (void)p;
    (void)q;
    return ENOSYS;
}

int tw_order(uint64_t a, uint64_t b)
{
    (void)a;
    (void)b;
    return 0;
}

int tw_queue_init(struct tw_queue *p, unsigned n, struct tw_init *r)
{
    (void)p;
    (void)n;
    (void)r;
    return ENOSYS;
}

int tw_ref(struct tw_queue *p)
{
    (void)p;
    return ENOSYS;
}

int tw_unref(struct tw_queue *p)
{
    (void)p;
    return ENOSYS;
}

int tw_close(struct tw_queue *p)
{
    (void)p;
    return ENOSYS;
}

int tw_retire(struct tw_queue *p)
{
    (void)p;
    return ENOSYS;
}

void tw_root_destroy(struct tw_queue *p)
{
    (void)p;
}

int tw_push(struct tw_queue *p, int n, const struct timespec *t)
{
    (void)p;
    (void)n;
    (void)t;
    return ENOSYS;
}

int tw_pop(struct tw_queue *p, int *n, const struct timespec *t, tw_hook_fn h,
           void *a)
{
    (void)p;
    (void)n;
    (void)t;
    (void)h;
    (void)a;
    return ENOSYS;
}

int tw_noise(struct tw_queue *p)
{
    (void)p;
    return ENOSYS;
}

int tw_wait_entries(struct tw_queue *p, unsigned n)
{
    (void)p;
    (void)n;
    return ENOSYS;
}

int tw_counts(struct tw_queue *p, unsigned *r, unsigned *a, size_t *c)
{
    (void)p;
    (void)r;
    (void)a;
    (void)c;
    return ENOSYS;
}

int tw_barrier_init(struct tw_barrier *p, unsigned n)
{
    (void)p;
    (void)n;
    return ENOSYS;
}

int tw_barrier_wait(struct tw_barrier *p)
{
    (void)p;
    return ENOSYS;
}

int tw_barrier_break(struct tw_barrier *p)
{
    (void)p;
    return ENOSYS;
}

void tw_barrier_destroy(struct tw_barrier *p)
{
    (void)p;
}

int tw_permits_init(struct tw_permits *p)
{
    (void)p;
    return ENOSYS;
}

int tw_permit_take(struct tw_permits *p, unsigned *t)
{
    (void)p;
    (void)t;
    return ENOSYS;
}

int tw_permit_release(struct tw_permits *p, unsigned t)
{
    (void)p;
    (void)t;
    return ENOSYS;
}

void tw_permits_destroy(struct tw_permits *p)
{
    (void)p;
}

int tw_reader_admit(unsigned r, unsigned w, unsigned n, int p)
{
    (void)r;
    (void)w;
    (void)n;
    (void)p;
    return 0;
}

int tw_writer_admit(unsigned r, unsigned w)
{
    (void)r;
    (void)w;
    return 0;
}

int tw_select(unsigned n)
{
    (void)n;
    return -1;
}
