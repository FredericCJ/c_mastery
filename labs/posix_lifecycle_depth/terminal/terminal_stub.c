#include "terminal.h"
#include <errno.h>

void pt_guard_init(struct pt_guard *g)
{
    (void)g;
}

int pt_enter(struct pt_guard *g, int f, enum pt_mode m, unsigned n, unsigned t)
{
    (void)g;
    (void)f;
    (void)m;
    (void)n;
    (void)t;
    return ENOSYS;
}

int pt_restore(struct pt_guard *g)
{
    (void)g;
    return ENOSYS;
}

bool pt_attributes_equal(const struct termios *a, const struct termios *b)
{
    (void)a;
    (void)b;
    return false;
}

int pt_pipeline(size_t n, bool e, unsigned d, struct pt_pipeline *o)
{
    (void)n;
    (void)e;
    (void)d;
    (void)o;
    return ENOSYS;
}
