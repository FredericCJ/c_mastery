#include "supervision.h"

#include <stdio.h>

int supervise_namespace(const char *variant)
{
    /* TODO: Fork a worker, sample both process identities and the private mount marker,
     * then read the actual cgroup membership and leaf policy. */
    fprintf(stderr, "namespace/%s requires a learner implementation\n", variant);
    return 1;
}
