/*
 * Reference implementations of the transfer containers: the bounded pool
 * queue, the stable-address segmented array, the move-only owner and the
 * three-slot deque.
 *
 * Shared contract (full text in labs/containers_depth/CONTRACTS.md): control
 * objects are caller-owned and externally serialized; a refused operation
 * leaves the container and the caller's input unchanged.
 *
 * The three queue functions are copied by assessor/algorithm_invariants/
 * instrument.py, which adds observation counters to exact statement spellings.
 * They therefore use literals rather than file-scope names, and call no
 * helper.  Do not reformat their statements without migrating that census.
 */
#include "containers.h"

/* Pool queue geometry: six node objects live in the owner; the first segment
 * of three is admitted initially and the second segment on refill. */
void cd_queue_init(struct cd_queue *q)
{
    q->head = q->tail = -1;
    q->count = 0;
    q->available = 3;
    for (unsigned i = 0; i < 6; ++i)
    {
        q->nodes[i] = (struct cd_queue_node){0, -1, 0};
    }
}

/* A full first segment grows to six nodes only when refill is requested.
 * `fail` is consulted only in that case (first segment full, refill
 * requested); it simulates the acquisition failing, returning CD_NOMEM before
 * availability or any link is published.  Values leave in FIFO order. */
enum cd_status cd_queue_push(struct cd_queue *q, int value, unsigned refill,
                             unsigned fail)
{
    if (!q || refill > 1)
    {
        return CD_INVALID;
    }
    if (q->count == q->available)
    {
        if (!refill || q->available == 6)
        {
            return CD_FULL;
        }
        if (fail)
        {
            return CD_NOMEM;
        }
        q->available = 6;
    }
    unsigned at = 0;
    while (q->nodes[at].live)
    {
        ++at;
    }
    q->nodes[at] = (struct cd_queue_node){value, -1, 1};
    if (q->tail >= 0)
    {
        q->nodes[q->tail].next = (int)at;
    }
    else
    {
        q->head = (int)at;
    }
    q->tail = (int)at;
    ++q->count;
    return CD_OK;
}

enum cd_status cd_queue_pop(struct cd_queue *q, int *out)
{
    if (!q || !out)
    {
        return CD_INVALID;
    }
    if (!q->count)
    {
        return CD_EMPTY;
    }
    unsigned at = (unsigned)q->head;
    int value = q->nodes[at].value;
    q->head = q->nodes[at].next;
    q->nodes[at].live = 0;
    --q->count;
    if (!q->count)
    {
        q->tail = -1;
    }
    *out = value;
    return CD_OK;
}

/* Segmented layout: SEGMENT_COUNT blocks of SEGMENT_SIZE ints, all reserved up
 * front inside the owner, so a returned address stays valid while the owner
 * is live and unmoved. */
enum
{
    SEGMENT_SIZE = 4,
    SEGMENT_COUNT = 4,
    SEGMENT_CAPACITY = SEGMENT_SIZE * SEGMENT_COUNT
};

/* *out receives the stable address of the new element. */
enum cd_status cd_segmented_append(struct cd_segmented *s, int value, int **out)
{
    if (!s || !out)
    {
        return CD_INVALID;
    }
    if (s->count == SEGMENT_CAPACITY)
    {
        return CD_FULL;
    }
    int *p = &s->blocks[s->count / SEGMENT_SIZE][s->count % SEGMENT_SIZE];
    *p = value;
    ++s->count;
    *out = p;
    return CD_OK;
}

/* Move-only owner of up to OWNED_LIMIT resource tokens.  Each resource id may
 * appear once.  `cancel` injects a cancellation point: 1 cancels before
 * staging, 2 after staging but before publication; both leave the input token
 * intact (CD_FAILED).  No callback or cancellation can run between
 * publishing the candidate and clearing input->resource; the two copies of
 * the token do exist in memory during that window. */
enum
{
    OWNED_LIMIT = 8,
    OWNED_CANCEL_BEFORE_STAGING = 1,
    OWNED_CANCEL_AFTER_STAGING = 2
};

enum cd_status cd_owned_append(struct cd_owned *c, struct cd_move_only *input,
                               unsigned cancel)
{
    if (!c || !input || !input->resource || cancel > OWNED_CANCEL_AFTER_STAGING)
    {
        return CD_INVALID;
    }
    if (c->count == OWNED_LIMIT)
    {
        return CD_FULL;
    }
    for (unsigned i = 0; i < c->count; ++i)
    {
        if (c->values[i].resource == input->resource)
        {
            return CD_DUPLICATE;
        }
    }
    if (cancel == OWNED_CANCEL_BEFORE_STAGING)
    {
        return CD_FAILED;
    }
    struct cd_move_only candidate = *input;
    if (cancel == OWNED_CANCEL_AFTER_STAGING)
    {
        return CD_FAILED;
    }
    c->values[c->count++] = candidate;
    input->resource = 0;
    return CD_OK;
}

/* Deque: a three-slot ring over head/count.  Back/back is a stack and
 * back/front a queue. */
enum
{
    DEQUE_SLOTS = 3
};

static int deque_valid(const struct cd_deque *d)
{
    return d->head < DEQUE_SLOTS && d->count <= DEQUE_SLOTS;
}

enum cd_status cd_deque_push(struct cd_deque *d, int value, unsigned front)
{
    if (!d || front > 1 || !deque_valid(d))
    {
        return CD_INVALID;
    }
    if (d->count == DEQUE_SLOTS)
    {
        return CD_FULL;
    }
    unsigned at;
    if (front)
    {
        /* Step the head back one slot, wrapping to the last. */
        d->head = d->head ? d->head - 1 : DEQUE_SLOTS - 1;
        at = d->head;
    }
    else
    {
        at = (d->head + d->count) % DEQUE_SLOTS;
    }
    d->values[at] = value;
    ++d->count;
    return CD_OK;
}

enum cd_status cd_deque_pop(struct cd_deque *d, int *out, unsigned front)
{
    if (!d || !out || front > 1 || !deque_valid(d))
    {
        return CD_INVALID;
    }
    if (!d->count)
    {
        return CD_EMPTY;
    }
    unsigned at = front ? d->head : (d->head + d->count - 1) % DEQUE_SLOTS;
    int value = d->values[at];
    if (front)
    {
        d->head = (d->head + 1) % DEQUE_SLOTS;
    }
    --d->count;
    *out = value;
    return CD_OK;
}
