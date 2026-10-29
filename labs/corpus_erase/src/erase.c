#include "erase.h"

/* Compiling incomplete learner component. */
int er_init(struct er_owner *a, const struct er_item *b, size_t n)
{
    (void)b;
    (void)n;
    if (a)
        *a = (struct er_owner){0};
    return ER_INVALID;
}

int er_compact(int *a, size_t n, int v, size_t *out, size_t *m)
{
    (void)a;
    (void)n;
    (void)v;
    (void)out;
    (void)m;
    return ER_INVALID;
}

int er_remove_if(struct er_owner *a, er_predicate_fn p, void *c,
                 struct er_tail *out)
{
    (void)a;
    (void)p;
    (void)c;
    (void)out;
    return ER_INVALID;
}

int er_erase(struct er_owner *a, struct er_tail t, er_drop_fn d, void *c)
{
    (void)a;
    (void)t;
    (void)d;
    (void)c;
    return ER_INVALID;
}

int er_clear(struct er_owner *a, er_drop_fn d, void *c)
{
    (void)a;
    (void)d;
    (void)c;
    return ER_INVALID;
}
