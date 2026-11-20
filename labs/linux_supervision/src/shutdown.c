#include "supervision.h"

#include <stdio.h>

int supervise_shutdown(const char *variant)
{
    /* TODO: Establish a startup handshake, account for the pipe ends and signal
     * dispositions, and collect the terminal status within one deadline. */
    fprintf(stderr, "shutdown/%s requires a learner implementation\n", variant);
    return 1;
}
