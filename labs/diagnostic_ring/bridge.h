#ifndef DIAGNOSTIC_RING_BRIDGE_H
#define DIAGNOSTIC_RING_BRIDGE_H
#include "ethernet.h"
#include "ring.h"
typedef struct {
  EnRing *ring;
  uint32_t epoch;
  unsigned phase;
  EnTicket ticket;
} DgRing;
/* phase0 idle,1 copied/prepared,2 published,3 quarantined. */
int dr_open(DgRing *, EnRing *);
int dr_send(DgRing *, const DGMessage *);
int dr_poll(DgRing *, uint32_t *);
int dr_receive(DgRing *, DGSession *, DGMessage *);
#endif
