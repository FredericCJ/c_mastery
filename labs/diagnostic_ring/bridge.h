#ifndef DIAGNOSTIC_RING_BRIDGE_H
#define DIAGNOSTIC_RING_BRIDGE_H
#include "ethernet.h"
#include "ring.h"

struct dg_ring
{
    struct en_ring *ring;
    uint32_t epoch;
    unsigned phase;
    struct en_ticket ticket;
};

/* phase0 idle,1 copied/prepared,2 published,3 quarantined. */
int dr_open(struct dg_ring *, struct en_ring *);
int dr_send(struct dg_ring *, const struct dg_message *);
int dr_poll(struct dg_ring *, uint32_t *);
int dr_receive(struct dg_ring *, struct dg_session *, struct dg_message *);
#endif
