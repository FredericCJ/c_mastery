#ifndef CM_RAW_RING_H
#define CM_RAW_RING_H
#include <linux/io_uring.h>
#include <stddef.h>

struct cm_raw_ring
{
    int fd;
    void *sq, *cq;
    struct io_uring_sqe *entries;
    size_t sq_bytes, cq_bytes, entries_bytes;
    unsigned *sq_head, *sq_tail, *sq_mask, *sq_array;
    unsigned *cq_head, *cq_tail, *cq_mask;
    struct io_uring_cqe *completions;
    unsigned local_tail, unsubmitted, capacity;
    int shared_mapping;
};

/* Linux single issuer, no SQPOLL; payload lifetime extends to terminal CQE.
 * Return 0 or -errno. init failure leaves a safely closable empty ring.
 * push copies one SQE; -EAGAIN leaves state unchanged. submit retries EINTR,
 * retains unsubmitted entries after partial progress. wait copies and consumes
 * one CQE, retrying EINTR. close requires all operation terminal CQEs consumed. */
int cm_ring_init(struct cm_raw_ring *, unsigned);
int cm_ring_push(struct cm_raw_ring *, const struct io_uring_sqe *);
int cm_ring_submit(struct cm_raw_ring *);
int cm_ring_wait(struct cm_raw_ring *, struct io_uring_cqe *);
void cm_ring_close(struct cm_raw_ring *);
#endif
