#include "coverage.h"

int cv_allow(struct cv_cv *c, size_t n, int a)
{
    (void)c;
    (void)n;
    (void)a;
    return 0;
}

int cv_push(struct cv_cv *c, int x)
{
    (void)c;
    (void)x;
    return 0;
}

void cv_cleanup(struct cv_cv *c)
{
    (void)c;
}

int cv_decision(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}

int cv_changed(int a, int b, int c)
{
    (void)a;
    (void)b;
    (void)c;
    return 0;
}
