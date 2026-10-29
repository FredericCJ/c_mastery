#include "progress.h"

struct tw_queue
{
    unsigned value[3], length;
};

static int step(struct tw_queue *q, const struct tp_op *op, unsigned *r,
                unsigned *v)
{
    unsigned result, value = 0;
    if (op->kind == TP_ENQUEUE)
    {
        result = q->length == 3 ? TP_FULL : TP_VALUE;
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

int tp_history(const struct tp_op *ops, size_t n, unsigned policy,
               struct tp_witness *out)
{
    if ((n && !ops) || !out || n > 6 || policy > TP_ALLOW_EXTENSION)
    {
        return TP_INVALID;
    }
    unsigned pending = 0, complete = 0;
    for (size_t i = 0; i < n; i++)
    {
        const struct tp_op *p = &ops[i];
        if (!p->invocation || (p->response && p->response <= p->invocation) ||
            p->kind > TP_DEQUEUE || p->thread > 3 ||
            (p->response && p->result > TP_FULL))
        {
            return TP_INVALID;
        }
        for (size_t j = 0; j < i; j++)
        {
            const struct tp_op *q = &ops[j];
            if (p->invocation == q->invocation ||
                p->invocation == q->response ||
                (p->response &&
                 (p->response == q->invocation || p->response == q->response)))
            {
                return TP_INVALID;
            }
            if (p->thread == q->thread &&
                !(p->response && p->response < q->invocation) &&
                !(q->response && q->response < p->invocation))
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
