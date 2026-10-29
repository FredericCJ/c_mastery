#include "ipc.h"

int ei_init(struct ei_request *r, const unsigned char p[8], unsigned epoch,
            unsigned id)
{
    (void)r;
    (void)p;
    (void)epoch;
    (void)id;
    return -1;
}

int ei_transition(struct ei_request *r, enum ei_event e)
{
    (void)r;
    (void)e;
    return -1;
}

int ei_invariant(const struct ei_request *r)
{
    (void)r;
    return 0;
}

int ei_enqueue(struct ei_queue *q, struct ei_request *r)
{
    (void)q;
    (void)r;
    return -1;
}

int ei_dequeue(struct ei_queue *q, struct ei_request **r)
{
    (void)q;
    (void)r;
    return -1;
}

void ei_reply(const unsigned char in[8], unsigned char out[4])
{
    (void)in;
    (void)out;
}
