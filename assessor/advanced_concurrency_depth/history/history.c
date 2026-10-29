#include "history.h"

struct ag_object
{
    unsigned values[4], length;
};

static int step(struct ag_object *q, const struct ah_operation *op,
                unsigned cap, unsigned spec, unsigned *result, unsigned *value)
{
    unsigned r, v = op->value, found = q->length;
    for (unsigned i = 0; i < q->length; i++)
    {
        if (q->values[i] == v)
        {
            found = i;
        }
    }
    if (op->operation == AH_INSERT)
    {
        if (spec == AH_SET && found < q->length)
        {
            r = AH_EXISTS;
        }
        else if (q->length == cap)
        {
            r = AH_FULL;
        }
        else
        {
            q->values[q->length++] = v;
            r = AH_VALUE;
        }
    }
    else if (spec == AH_SET)
    {
        if (found == q->length)
        {
            r = AH_ABSENT;
        }
        else
        {
            q->length--;
            for (unsigned i = found; i < q->length; i++)
            {
                q->values[i] = q->values[i + 1];
            }
            r = AH_VALUE;
        }
    }
    else if (!q->length)
    {
        r = AH_EMPTY;
    }
    else
    {
#ifdef AH_MUTANT_ORDER
        v = q->values[q->length - 1];
        q->length--;
#else
        v = q->values[0];
        q->length--;
        for (unsigned i = 0; i < q->length; i++)
        {
            q->values[i] = q->values[i + 1];
        }
#endif
        r = AH_VALUE;
    }
    *result = r;
    *value = v;
    return op->respond == AH_PENDING ||
           (op->result == r && (spec == AH_SET || op->operation == AH_INSERT ||
                                r != AH_VALUE || op->value == v));
}

static int search(const struct ah_operation *ops, size_t n, unsigned remaining,
                  unsigned cap, unsigned spec, struct ag_object q,
                  struct ah_witness *w, unsigned depth)
{
    if (!remaining)
    {
        w->length = depth;
        return 1;
    }
    for (unsigned i = 0; i < n; i++)
    {
        if (remaining & (1u << i))
        {
            unsigned blocked = 0;
            for (unsigned j = 0; j < n; j++)
            {
                if ((remaining & (1u << j)) && ops[j].respond != AH_PENDING &&
                    ops[j].respond < ops[i].invoke)
                {
                    blocked = 1;
                }
            }
#ifdef AH_MUTANT_REALTIME
            blocked = 0;
#endif
            if (blocked)
            {
                continue;
            }
            struct ag_object next = q;
            if (!step(&next, &ops[i], cap, spec, &w->result[i], &w->value[i]))
            {
                continue;
            }
            w->order[depth] = i;
            if (search(ops, n, remaining & ~(1u << i), cap, spec, next, w,
                       depth + 1))
            {
                return 1;
            }
        }
    }
    return 0;
}

int ah_history(const struct ah_operation *ops, size_t n, unsigned cap,
               unsigned spec, unsigned policy, struct ah_witness *out)
{
    if ((n && !ops) || !out || n > 7 || !cap || cap > 4 || spec > AH_SET ||
        policy > AH_EXTEND_PENDING)
    {
        return AH_INVALID;
    }
    unsigned pending = 0, complete = 0;
    for (unsigned i = 0; i < n; i++)
    {
        const struct ah_operation *p = &ops[i];
        if (p->invoke == AH_PENDING ||
            (p->respond != AH_PENDING && p->respond <= p->invoke) ||
            p->operation > AH_REMOVE || p->participant > 7 ||
            (p->respond != AH_PENDING && p->result > AH_ABSENT))
        {
            return AH_INVALID;
        }
        for (unsigned j = 0; j < i; j++)
        {
            const struct ah_operation *q = &ops[j];
            if (p->invoke == q->invoke ||
                (q->respond != AH_PENDING && p->invoke == q->respond) ||
                (p->respond != AH_PENDING &&
                 (p->respond == q->invoke || p->respond == q->respond)))
            {
                return AH_INVALID;
            }
            if (p->participant == q->participant &&
                !(p->respond != AH_PENDING && p->respond < q->invoke) &&
                !(q->respond != AH_PENDING && q->respond < p->invoke))
            {
                return AH_INVALID;
            }
        }
        if (p->respond == AH_PENDING)
        {
            pending |= 1u << i;
        }
        else
        {
            complete |= 1u << i;
        }
    }
    if (pending && policy == AH_COMPLETE_ONLY)
    {
        return AH_INCOMPLETE;
    }
#ifdef AH_MUTANT_PENDING
    pending = 0;
#endif
    unsigned subset = pending;
    for (;;)
    {
        struct ag_object q = {{0}, 0};
        struct ah_witness w = {0};
        w.completed_pending = subset;
        if (search(ops, n, complete | subset, cap, spec, q, &w, 0))
        {
#ifdef AH_MUTANT_WITNESS
            if (w.length)
            {
                w.order[0] = 7;
            }
#endif
            *out = w;
            return AH_VALID;
        }
        if (!subset)
        {
            break;
        }
        subset = (subset - 1u) & pending;
    }
    return AH_NONLINEAR;
}

int ah_retry(unsigned attempts, unsigned budget, unsigned changed)
{
    if (!budget || attempts > budget || changed > 1)
    {
        return AH_INVALID;
    }
    if (changed)
    {
        return AH_VALID;
    }
#ifdef AH_MUTANT_RETRY
    if (attempts == budget)
    {
        return AH_VALID;
    }
#endif
    return attempts == budget ? AH_INCOMPLETE : AH_NONLINEAR;
}
