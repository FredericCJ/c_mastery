#include "supervision.h"

#include <stdio.h>

int supervise_pids(const char *variant)
{
    /* TODO: Keep the first worker alive across the next fork. Measure population
     * and max events around that attempt, then release and reap all children. */
    fprintf(stderr, "pids/%s requires a learner implementation\n", variant);
    return 1;
}
