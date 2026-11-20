#include "supervision.h"

#include <stdio.h>

int supervise_network(const char *variant)
{
    /* TODO: Bind numeric loopback, synchronize the live listeners via fd 3, and
     * check both the same-namespace bind and the framed request/reply. */
    fprintf(stderr, "network/%s requires a learner implementation\n", variant);
    return 1;
}
