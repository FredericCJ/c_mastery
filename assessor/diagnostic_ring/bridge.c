#include "bridge.h"
#ifndef DR_MUTANT
#define DR_MUTANT 0
#endif
static int current(const DgRing *s) {
  return s && s->ring && s->epoch && s->ring->epoch == s->epoch;
}
int dr_open(DgRing *s, EnRing *r) {
  if (!s || !r || !r->initialized || r->phase != EN_RUNNING || !r->epoch)
    return DG_INVALID;
  if (s->ring && (s->ring != r || r->epoch <= s->epoch))
    return DG_BUSY;
  DgRing next = {0};
  next.ring = r;
  next.epoch = r->epoch;
  *s = next;
  return DG_OK;
}
int dr_send(DgRing *s, const DGMessage *m) {
  if (!s || !m)
    return DG_INVALID;
  if (!current(s))
    return DG_STALE;
  if (s->phase)
    return DG_BUSY;
  uint8_t frame[60];
  size_t n;
  int rc = dg_ethernet_build(m, frame, sizeof frame, &n);
  if (rc)
    return rc;
  EnTicket t;
  rc = en_tx_copy(s->ring, frame, n, &t);
  if (rc)
    return rc == ED_FULL    ? DG_FULL
           : rc == ED_BUSY  ? DG_BUSY
           : rc == ED_STALE ? DG_STALE
                            : DG_IO;
  s->ticket = t;
  s->phase = DR_MUTANT == 1 ? 0 : 1;
  return DG_OK;
}
int dr_poll(DgRing *s, uint32_t *raw) {
  if (!s || !raw)
    return DG_INVALID;
  if (!current(s))
    return DG_STALE;
  if (!s->phase)
    return DG_STALE;
  if (s->phase == 3)
    return DG_IO;
  if (s->phase == 1) {
    int rc = en_tx_publish(s->ring, s->ticket);
    if (rc == ED_BUSY)
      return DG_BUSY;
    if (rc) {
      s->phase = DR_MUTANT == 2 ? 0 : 3;
      return DG_IO;
    }
    s->phase = 2;
    if (DR_MUTANT == 3) {
      s->phase = 0;
      return DG_OK;
    }
    return DG_BUSY;
  }
  uint32_t status;
  int rc = en_tx_reap(s->ring, s->ticket, &status);
  if (rc == ED_BUSY)
    return DG_BUSY;
  if (rc == ED_OK || rc == ED_IO) {
    s->phase = 0;
    *raw = status;
    return rc == ED_OK ? DG_OK : DG_IO;
  }
  s->phase = 3;
  return DG_IO;
}
int dr_receive(DgRing *s, DGSession *session, DGMessage *out) {
  if (!s || !session || !out)
    return DG_INVALID;
  if (!current(s))
    return DG_STALE;
  EnFrame frame;
  int rc = en_rx_take(s->ring, &frame);
  if (rc)
    return rc == ED_BUSY    ? DG_BUSY
           : rc == ED_FULL  ? DG_FULL
           : rc == ED_STALE ? DG_STALE
                            : DG_IO;
  DGSession pending = *session;
  DGMessage message;
  rc = dg_ethernet_accept(&pending, frame.bytes, frame.length, &message);
  int released = DR_MUTANT == 4 ? ED_OK : en_rx_release(s->ring, frame.ticket);
  if (released != ED_OK && DR_MUTANT != 5)
    return DG_IO;
  if (rc == DG_OK) {
    *session = pending;
    *out = message;
  }
  return rc;
}
