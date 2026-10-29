#include "session_v1.h"

/* Intentional learner scaffold, including failure and ownership behavior. */
int ac_opaque_create(uint32_t i, uint32_t n, struct ac_allocate a,
                     struct ac_opaque **o)
{
    (void)i;
    (void)n;
    (void)a;
    (void)o;
    return 2;
}

int ac_opaque_get(const struct ac_opaque *s, struct ac_exposed_v1 *o)
{
    (void)s;
    (void)o;
    return 1;
}

void ac_opaque_destroy(struct ac_opaque *s)
{
    (void)s;
}

size_t ac_opaque_size(void)
{
    return 0;
}

size_t ac_opaque_alignment(void)
{
    return 0;
}
