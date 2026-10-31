#include "progress.h"

enum
{
    QUEUE_CAPACITY = 3,
    MAX_OPERATIONS = 6,
    MAX_THREAD_ID = 3
};

/* Sequential model: a FIFO of at most QUEUE_CAPACITY values. */
struct tw_queue
{
    unsigned value[QUEUE_CAPACITY];
    unsigned length;
};

/*
 * Applies op to the model queue. *r and *v receive the result and value the
 * sequential queue would produce. Returns 1 when that agrees with the
 * recorded response (a pending operation, response 0, agrees with anything).
 */
static int step(struct tw_queue *q, const struct tp_op *op, unsigned *r,
                unsigned *v)
{
    unsigned result;
    unsigned value = 0;
    if (op->kind == TP_ENQUEUE)
    {
        result = q->length == QUEUE_CAPACITY ? TP_FULL : TP_VALUE;
        if (result == TP_VALUE)
        {
            q->value[q->length++] = op->value;
        }
    }
    else
    {
        result = q->length ? TP_VALUE : TP_EMPTY;
        if (result == TP_VALUE)
        {
            value = q->value[0];
            q->length--;
            for (unsigned i = 0; i < q->length; i++)
            {
                q->value[i] = q->value[i + 1];
            }
        }
    }
    *r = result;
    *v = value;
    return !op->response ||
           (op->result == result && (op->kind == TP_ENQUEUE ||
                                     result != TP_VALUE || op->value == value));
}

/*
 * Depth-first search for a sequential order of the operations in `remaining`
 * (a bitmask) that respects real time: an operation may go next only if no
 * other remaining operation responded before it was invoked. On success the
 * order is left in w->operation[0 .. depth).
 */
static int search(const struct tp_op *ops, size_t n, unsigned remaining,
                  struct tw_queue queue, struct tp_witness *w, unsigned depth)
{
    if (!remaining)
    {
        w->length = depth;
        return 1;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (remaining & (1u << i))
        {
            int ready = 1;
            for (size_t j = 0; j < n; j++)
            {
                if ((remaining & (1u << j)) && ops[j].response &&
                    ops[j].response < ops[i].invocation)
                {
                    ready = 0;
                }
            }
#ifdef TP_MUTANT_HISTORY
            ready = 1;
#endif
            if (!ready)
            {
                continue;
            }
            struct tw_queue next = queue;
            if (!step(&next, &ops[i], &w->result[i], &w->value[i]))
            {
                continue;
            }
            w->operation[depth] = (unsigned)i;
            if (search(ops, n, remaining & ~(1u << i), next, w, depth + 1))
            {
                return 1;
            }
        }
    }
    return 0;
}

/* One operation is malformed on its own: bad timestamps, kind, thread or
 * (when completed) result. */
static int op_malformed(const struct tp_op *p)
{
    return !p->invocation || (p->response && p->response <= p->invocation) ||
           p->kind > TP_DEQUEUE || p->thread > MAX_THREAD_ID ||
           (p->response && p->result > TP_FULL);
}

/* Two operations clash when they share a timestamp (timestamps are a strict
 * total order) or when one thread has them overlapping in time. */
static int ops_clash(const struct tp_op *p, const struct tp_op *q)
{
    if (p->invocation == q->invocation || p->invocation == q->response ||
        (p->response &&
         (p->response == q->invocation || p->response == q->response)))
    {
        return 1;
    }
    return p->thread == q->thread &&
           !(p->response && p->response < q->invocation) &&
           !(q->response && q->response < p->invocation);
}

/*
 * Linearizability check of a FIFO history. Completed operations must all
 * appear in the order; pending ones (response 0) may be omitted or, under
 * TP_ALLOW_EXTENSION, included with a synthesized result. Returns TP_OK with
 * a witness in *out, TP_NONLINEAR, TP_INCOMPLETE (pending operations under
 * TP_REQUIRE_COMPLETE) or TP_INVALID; *out changes only on TP_OK.
 */
int tp_history(const struct tp_op *ops, size_t n, unsigned policy,
               struct tp_witness *out)
{
    if ((n && !ops) || !out || n > MAX_OPERATIONS ||
        policy > TP_ALLOW_EXTENSION)
    {
        return TP_INVALID;
    }
    unsigned pending = 0;
    unsigned complete = 0;
    for (size_t i = 0; i < n; i++)
    {
        const struct tp_op *p = &ops[i];
        if (op_malformed(p))
        {
            return TP_INVALID;
        }
        for (size_t j = 0; j < i; j++)
        {
            if (ops_clash(p, &ops[j]))
            {
                return TP_INVALID;
            }
        }
        if (p->response)
        {
            complete |= 1u << i;
        }
        else
        {
            pending |= 1u << i;
        }
    }
    if (pending && policy == TP_REQUIRE_COMPLETE)
    {
        return TP_INCOMPLETE;
    }
    unsigned subset = pending;
    for (;;)
    {
        struct tp_witness w = {0};
        struct tw_queue q = {{0}, 0};
        w.completed_pending_mask = subset;
        if (search(ops, n, complete | subset, q, &w, 0))
        {
            *out = w;
            return TP_OK;
        }
        if (!subset)
        {
            break;
        }
        subset = (subset - 1u) & pending;
    }
    return TP_NONLINEAR;
}
