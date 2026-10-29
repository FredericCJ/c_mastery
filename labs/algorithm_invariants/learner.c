#include "invariants.h"

enum ai_status ai_step(struct ai_state s, unsigned m, struct ai_state *out)
{
    (void)s;
    (void)m;
    (void)out;
    return AI_UNIMPLEMENTED;
}

enum ai_status ai_permutation(const struct ai_record *a,
                              const struct ai_record *b, size_t n,
                              unsigned *out)
{
    (void)a;
    (void)b;
    (void)n;
    (void)out;
    return AI_UNIMPLEMENTED;
}

enum ai_status ai_growth(unsigned n, unsigned a, struct ai_growth *out)
{
    (void)n;
    (void)a;
    (void)out;
    return AI_UNIMPLEMENTED;
}

void ai_queues_init(struct ai_queues *q)
{
    (void)q;
}

enum cd_status ai_queue_event(struct ai_queues *q, unsigned p, int v, int *out)
{
    (void)q;
    (void)p;
    (void)v;
    (void)out;
    return CD_INVALID;
}
