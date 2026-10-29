/* Unsolved learner implementation: intentionally rejects semantic tests. */
#define _POSIX_C_SOURCE 200809L
#include "cl.h"

void cl_queue_init(struct cl_queue *q, struct cl_message storage[2])
{
    (void)q;
    (void)storage;
}

enum cl_result cl_send(struct cl_queue *q, uint32_t p, uint32_t seq,
                       const unsigned char *in, size_t n)
{
    (void)q;
    (void)p;
    (void)seq;
    (void)in;
    (void)n;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_recv(struct cl_queue *q, unsigned char *out, size_t cap,
                       size_t *n, uint32_t *p, uint32_t *seq)
{
    (void)q;
    (void)out;
    (void)cap;
    (void)n;
    (void)p;
    (void)seq;
    return CL_UNIMPLEMENTED;
}

void cl_close(struct cl_queue *q)
{
    (void)q;
}

uint64_t cl_abort(struct cl_queue *q)
{
    (void)q;
    return 0;
}

void cl_pool_init(struct cl_pool *p)
{
    (void)p;
}

enum cl_result cl_alloc(struct cl_pool *p, unsigned n, struct cl_buffer *out)
{
    (void)p;
    (void)n;
    (void)out;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_buffer_write(struct cl_pool *p, struct cl_buffer h,
                               const unsigned char *in, unsigned n)
{
    (void)p;
    (void)h;
    (void)in;
    (void)n;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_buffer_read(const struct cl_pool *p, struct cl_buffer h,
                              unsigned char *out, unsigned cap, unsigned *n)
{
    (void)p;
    (void)h;
    (void)out;
    (void)cap;
    (void)n;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_release(struct cl_pool *p, struct cl_buffer h)
{
    (void)p;
    (void)h;
    return CL_UNIMPLEMENTED;
}

void cl_owned_init(struct cl_owned_queue *q)
{
    (void)q;
}

enum cl_result cl_owned_send(struct cl_pool *p, struct cl_owned_queue *q,
                             struct cl_buffer h)
{
    (void)p;
    (void)q;
    (void)h;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_owned_recv(struct cl_pool *p, struct cl_owned_queue *q,
                             struct cl_buffer *out)
{
    (void)p;
    (void)q;
    (void)out;
    return CL_UNIMPLEMENTED;
}

void cl_owned_close(struct cl_owned_queue *q)
{
    (void)q;
}

unsigned cl_owned_abort(struct cl_pool *p, struct cl_owned_queue *q)
{
    (void)p;
    (void)q;
    return 0;
}

void cl_life_init(struct cl_life *l)
{
    (void)l;
}

enum cl_result cl_acquire(struct cl_life *l, struct cl_lease *out)
{
    (void)l;
    (void)out;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_transition(struct cl_life *l, struct cl_lease h,
                             enum cl_lease_state from, enum cl_lease_state to)
{
    (void)l;
    (void)h;
    (void)from;
    (void)to;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_drop(struct cl_life *l, struct cl_lease h)
{
    (void)l;
    (void)h;
    return CL_UNIMPLEMENTED;
}

void cl_life_close(struct cl_life *l)
{
    (void)l;
}

enum cl_result cl_life_drained(struct cl_life *l)
{
    (void)l;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_destroy(struct cl_life *l)
{
    (void)l;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_restart(struct cl_life *l)
{
    (void)l;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_restart_group(struct cl_life *objects, unsigned count)
{
    (void)objects;
    (void)count;
    return CL_UNIMPLEMENTED;
}

void cl_record_init(struct cl_record *r, uint32_t v)
{
    (void)r;
    (void)v;
}

enum cl_result cl_record_begin(struct cl_record *r, uint32_t owner)
{
    (void)r;
    (void)owner;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_record_stage(struct cl_record *r, uint32_t owner, uint32_t v)
{
    (void)r;
    (void)owner;
    (void)v;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_record_commit(struct cl_record *r, uint32_t owner)
{
    (void)r;
    (void)owner;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_record_dead(struct cl_record *r, uint32_t dead)
{
    (void)r;
    (void)dead;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_repair_begin(struct cl_record *r, uint32_t owner)
{
    (void)r;
    (void)owner;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_repair_finish(struct cl_record *r, uint32_t owner,
                                unsigned repair)
{
    (void)r;
    (void)owner;
    (void)repair;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_record_read(const struct cl_record *r, uint32_t *v)
{
    (void)r;
    (void)v;
    return CL_UNIMPLEMENTED;
}

void cl_ack_init(struct cl_ack *a, uint32_t epoch)
{
    (void)a;
    (void)epoch;
}

enum cl_result cl_request(struct cl_ack *a, uint32_t epoch, uint32_t id,
                          uint32_t delta, uint32_t *out)
{
    (void)a;
    (void)epoch;
    (void)id;
    (void)delta;
    (void)out;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_apply(struct cl_ack *a, uint32_t id)
{
    (void)a;
    (void)id;
    return CL_UNIMPLEMENTED;
}

enum cl_result cl_acknowledge(struct cl_ack *a, uint32_t id)
{
    (void)a;
    (void)id;
    return CL_UNIMPLEMENTED;
}
