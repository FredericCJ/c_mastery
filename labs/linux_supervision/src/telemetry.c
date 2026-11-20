#include "supervision.h"

#include <stdio.h>

int supervise_telemetry(const char *variant)
{
    /* TODO: Exchange the finite checksum requests and replies over private TCP,
     * forward the replies to the gated pipe, and measure refusal and CPU service. */
    fprintf(stderr, "telemetry/%s requires a learner implementation\n", variant);
    return 1;
}
