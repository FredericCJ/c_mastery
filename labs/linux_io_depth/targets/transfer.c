#include "transfer.h"

int it_progress(const struct iovec *v, unsigned n, size_t *p, it_write_fn f,
                void *c)
{
    (void)v;
    (void)n;
    (void)p;
    (void)f;
    (void)c;
    return -1;
}

int it_queue_init(struct it_queue *q)
{
    (void)q;
    return -1;
}

int it_push(struct it_queue *q, uint64_t v)
{
    (void)q;
    (void)v;
    return 0;
}

int it_pop(struct it_queue *q, uint64_t *v)
{
    (void)q;
    (void)v;
    return 0;
}

void it_queue_close(struct it_queue *q)
{
    (void)q;
}

void it_queue_destroy(struct it_queue *q)
{
    (void)q;
}

int it_time(struct it_loop *q, uint64_t n, int p)
{
    (void)q;
    (void)n;
    (void)p;
    return -1;
}

int it_choose(struct it_loop *q)
{
    (void)q;
    return -1;
}

void it_drained(struct it_loop *q, unsigned p)
{
    (void)q;
    (void)p;
}

int it_direct_ok(uintptr_t p, size_t n, uint64_t o, unsigned m, unsigned a)
{
    (void)p;
    (void)n;
    (void)o;
    (void)m;
    (void)a;
    return 0;
}
