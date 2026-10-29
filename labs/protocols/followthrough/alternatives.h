#ifndef CMASTERY_PROTOCOL_ALTERNATIVES_H
#define CMASTERY_PROTOCOL_ALTERNATIVES_H
#include "protocols.h"

struct pa_empty
{
    struct pt_builder value;
};

struct pa_address
{
    struct pt_builder value;
};

struct pa_length
{
    struct pt_builder value;
};

struct pa_timeout
{
    struct pt_builder value;
};

int pa_address(struct pa_empty, unsigned, struct pa_address *);
int pa_length(struct pa_address, unsigned, struct pa_length *);
int pa_timeout(struct pa_length, unsigned, struct pa_timeout *);
int pa_finish(struct pa_timeout, struct pt_config *);
int pa_thread_sum(struct pt_thread *, unsigned, uint32_t, uint32_t *);
int pa_session_pair(unsigned, unsigned *);
#endif
