#include "coverage.h"
#ifndef CV_MUTANT
#define CV_MUTANT 0
#endif
int cv_allow(struct cv_cv *c, size_t length, int authorized)
{
    if (length <= 4 && (CV_MUTANT == 2 ? !authorized : authorized))
        return 1;
    c->rejection_count += CV_MUTANT == 4 ? 0u : 1u;
    return 0;
}

int cv_push(struct cv_cv *c, int value)
{
    if (c->count > 4)
        return -1; /* Defensive unreachable under initializer/API invariant. */
    if (CV_MUTANT == 1 ? c->count + 1 < 4 : c->count + 1 <= 4)
    {
        c->queue[c->count++] = value;
        return 1;
    }
    return 0;
}

void cv_cleanup(struct cv_cv *c)
{
    c->live -= CV_MUTANT == 3 ? 0u : c->live;
}

int cv_decision(int a, int b)
{
    return a && b;
}

int cv_changed(int a, int b, int c)
{
    return (a && b) || c;
}
