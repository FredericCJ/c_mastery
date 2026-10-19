#include "lease.h"
#ifndef XD_MUTANT
#define XD_MUTANT 0
#endif
static int same(XdTicket a, XdTicket b) {
  return a.epoch == b.epoch && a.generation == b.generation;
}
static int current(const XdLease *s, XdTicket t) {
  return s && t.epoch && t.generation &&
         (XD_MUTANT == 2 || same(t, (XdTicket){s->epoch, s->generation}));
}
int xd_recover(XdLease *s, const XdFence *f) {
  if (!s || !f || f->stopped > 1 || f->writes_drained > 1)
    return XD_INVALID;
  if (s->phase != XD_QUARANTINE)
    return XD_BUSY;
  if (f->old_epoch == UINT32_MAX)
    return XD_EXHAUSTED;
  if (f->old_epoch < s->epoch || f->new_epoch != f->old_epoch + 1)
    return XD_STALE;
  if (!f->stopped || (!f->writes_drained && XD_MUTANT != 5))
    return XD_BUSY;
  *s = (XdLease){.epoch = f->new_epoch, .phase = XD_CPU, .quiescent = 1};
  return XD_OK;
}
int xd_submit(XdLease *s, uint32_t n, XdTicket *out) {
  if (!s || !out || !n || n > 16384)
    return XD_INVALID;
  if (s->phase != XD_CPU || !s->quiescent)
    return XD_BUSY;
  if (s->generation == UINT32_MAX)
    return XD_EXHAUSTED;
  XdLease next = {.epoch = s->epoch,
                  .generation = s->generation + 1,
                  .phase = XD_DEVICE,
                  .length = n};
  *s = next;
  *out = (XdTicket){next.epoch, next.generation};
  return XD_OK;
}
int xd_cancel(XdLease *s, XdTicket t) {
  if (!s)
    return XD_INVALID;
  if (!current(s, t) || s->phase == XD_CPU || s->phase == XD_QUARANTINE)
    return XD_STALE;
  if (s->phase == XD_STOPPING || s->phase == XD_SERVICE)
    return XD_DUPLICATE;
  s->phase = XD_MUTANT == 1 ? XD_CPU : XD_STOPPING;
  return XD_OK;
}
int xd_observe(XdLease *s, XdTicket t, unsigned event, const XdProof *p) {
  if (!s || !p || event < XD_COMPLETE || event > XD_ERROR)
    return XD_INVALID;
  if (!current(s, t) || s->phase == XD_CPU || s->phase == XD_QUARANTINE)
    return XD_STALE;
  if (!same(t, p->ticket))
    return XD_STALE;
  if (s->phase == XD_SERVICE) {
    if (XD_MUTANT == 3)
      ++s->terminal_count;
    return event == s->terminal ||
                   (s->terminal == XD_COMPLETE && event == XD_STOPPED)
               ? XD_DUPLICATE
               : XD_STALE;
  }
  if (p->enabled > 1 || p->visibility_done > 1 || p->complete > p->possible ||
      p->possible > s->length)
    return XD_INVALID;
  if (event == XD_COMPLETE &&
      (p->complete != s->length || p->possible != s->length))
    return XD_INVALID;
  if (event == XD_STOPPED && s->phase != XD_STOPPING)
    return XD_INVALID;
  if (p->enabled || (p->writes_pending && XD_MUTANT != 4) ||
      !p->visibility_done)
    return XD_BUSY;
  s->phase = XD_SERVICE;
  s->terminal = event;
  s->terminal_count = 1;
  s->complete = p->complete;
  s->possible = p->possible;
  s->quiescent = 1;
  return XD_OK;
}
int xd_reclaim(XdLease *s, XdTicket t, XdResult *out) {
  if (!s || !out)
    return XD_INVALID;
  if (!current(s, t) || s->phase == XD_CPU || s->phase == XD_QUARANTINE)
    return XD_STALE;
  if (s->phase != XD_SERVICE || !s->quiescent)
    return XD_BUSY;
  XdResult r = {t, s->terminal, s->complete, s->possible};
  s->phase = XD_CPU;
  *out = r;
  return XD_OK;
}
void xd_quarantine(XdLease *s) {
  if (s) {
    s->phase = XD_QUARANTINE;
    s->quiescent = 0;
  }
}
