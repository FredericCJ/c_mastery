#define _GNU_SOURCE
#include "raw_ring.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifndef CM_MUTANT
#define CM_MUTANT 0
#endif
static unsigned acquire(unsigned *p)
{
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}

static void release(unsigned *p, unsigned v)
{
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
}

void cm_ring_close(struct cm_raw_ring *r)
{
    if (r->entries)
        munmap(r->entries, r->entries_bytes);
    if (r->cq && !r->shared_mapping)
        munmap(r->cq, r->cq_bytes);
    if (r->sq)
        munmap(r->sq, r->sq_bytes);
    if (r->fd >= 0)
        close(r->fd);
    memset(r, 0, sizeof *r);
    r->fd = -1;
}

int cm_ring_init(struct cm_raw_ring *r, unsigned n)
{
    struct io_uring_params p;
    memset(r, 0, sizeof *r);
    r->fd = -1;
    memset(&p, 0, sizeof p);
    int fd = (int)syscall(__NR_io_uring_setup, n, &p);
    if (fd < 0)
        return -errno;
    r->fd = fd;
    r->sq_bytes = p.sq_off.array + p.sq_entries * sizeof(unsigned);
    r->cq_bytes = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);
    r->shared_mapping = (p.features & IORING_FEAT_SINGLE_MMAP) != 0;
    if (r->shared_mapping && r->cq_bytes > r->sq_bytes)
        r->sq_bytes = r->cq_bytes;
    r->sq = mmap(NULL, r->sq_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                 IORING_OFF_SQ_RING);
    if (r->sq == MAP_FAILED)
    {
        r->sq = NULL;
        goto fail;
    }
    r->cq = r->shared_mapping ? r->sq
                              : mmap(NULL, r->cq_bytes, PROT_READ | PROT_WRITE,
                                     MAP_SHARED, fd, IORING_OFF_CQ_RING);
    if (r->cq == MAP_FAILED)
    {
        r->cq = NULL;
        goto fail;
    }
    r->entries_bytes = p.sq_entries * sizeof *r->entries;
    r->entries = mmap(NULL, r->entries_bytes, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, IORING_OFF_SQES);
    if (r->entries == MAP_FAILED)
    {
        r->entries = NULL;
        goto fail;
    }
#define AT(base, off, type) ((type *)((unsigned char *)(base) + (off)))
    r->sq_head = AT(r->sq, p.sq_off.head, unsigned);
    r->sq_tail = AT(r->sq, p.sq_off.tail, unsigned);
    r->sq_mask = AT(r->sq, p.sq_off.ring_mask, unsigned);
    r->sq_array = AT(r->sq, p.sq_off.array, unsigned);
    r->cq_head = AT(r->cq, p.cq_off.head, unsigned);
    r->cq_tail = AT(r->cq, p.cq_off.tail, unsigned);
    r->cq_mask = AT(r->cq, p.cq_off.ring_mask, unsigned);
    r->completions = AT(r->cq, p.cq_off.cqes, struct io_uring_cqe);
    r->capacity = p.sq_entries;
    r->local_tail = *r->sq_tail;
    return 0;
fail:
{
    int e = errno;
    cm_ring_close(r);
    return -e;
}
}

int cm_ring_push(struct cm_raw_ring *r, const struct io_uring_sqe *s)
{
    unsigned head = acquire(r->sq_head);
    if (CM_MUTANT != 1 && r->local_tail - head >= r->capacity)
        return -EAGAIN;
    unsigned i = r->local_tail & *r->sq_mask;
    r->entries[i] = *s;
    r->sq_array[i] = i;
    r->local_tail++;
    r->unsubmitted++;
    return 0;
}

int cm_ring_submit(struct cm_raw_ring *r)
{
    release(r->sq_tail, r->local_tail);
    while (r->unsubmitted)
    {
        long n =
            syscall(__NR_io_uring_enter, r->fd, r->unsubmitted, 0, 0, NULL, 0);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            return -errno;
        }
        if (!n)
            return -EAGAIN;
        r->unsubmitted -= (unsigned)n;
    }
    return 0;
}

int cm_ring_wait(struct cm_raw_ring *r, struct io_uring_cqe *c)
{
    for (;;)
    {
        unsigned head = *r->cq_head;
        if (head != acquire(r->cq_tail))
        {
            *c = r->completions[head & *r->cq_mask];
            if (CM_MUTANT == 2)
                c->flags = 0;
            release(r->cq_head, head + 1);
            return 0;
        }
        if (syscall(__NR_io_uring_enter, r->fd, 0, 1, IORING_ENTER_GETEVENTS,
                    NULL, 0) < 0 &&
            errno != EINTR)
            return -errno;
    }
}
