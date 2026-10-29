#include "arch.h"
#include <stdint.h>
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
void ar_machine_init(struct ar_machine *m) {
  if (m)
    *m = (struct ar_machine){.incarnation = 1};
}
int ar_acquire(struct ar_machine *m, uint32_t *id) {
  if (!m || !id)
    return AR_INVALID;
  if (m->stage || m->next_request == UINT32_MAX)
    return AR_STATE;
  m->request = ++m->next_request;
  m->stage = 1;
  *id = m->request;
  return AR_OK;
}
int ar_submit(struct ar_machine *m) {
  if (!m || m->stage != 1)
    return AR_STATE;
  m->stage = 2;
  return AR_OK;
}
static void release(struct ar_machine *m) {
  m->stage = 0;
  m->cancelling = 0;
  ++m->releases;
}
int ar_machine_cancel(struct ar_machine *m) {
  if (!m || !m->stage)
    return AR_STATE;
  if (m->stage == 1) {
    release(m);
    return AR_CANCELLED;
  }
  m->cancelling = 1;
  return AR_PENDING;
}
int ar_event(struct ar_machine *m, struct ar_event e) {
  if (!m || e.type > 1)
    return AR_INVALID;
  if (e.incarnation != m->incarnation || e.request != m->request ||
      m->stage != 2)
    return AR_STATE;
  if (e.type == 1) {
    if (!m->cancelling || m->ack_present)
      return AR_STATE;
    m->acknowledgement = e;
    m->ack_present = 1;
    return AR_OK;
  }
  if (m->queued == 2)
    return AR_FULL;
  m->queue[m->queued++] = e;
  return AR_OK;
}
int ar_step(struct ar_machine *m) {
  if (!m)
    return AR_INVALID;
  struct ar_event e;
  if (m->ack_present && AR_MUTANT != 4) {
    e = m->acknowledgement;
    m->ack_present = 0;
  } else {
    if (!m->queued)
      return AR_EMPTY;
    e = m->queue[0];
    m->queue[0] = m->queue[1];
    --m->queued;
  }
  if (e.incarnation != m->incarnation || e.request != m->request ||
      m->stage != 2)
    return AR_STATE;
  release(m);
  return e.type ? AR_CANCELLED : AR_OK;
}
int ar_restart(struct ar_machine *m) {
  if (!m || m->stage || m->incarnation == UINT32_MAX)
    return AR_STATE;
  ++m->incarnation;
  return AR_OK;
}
