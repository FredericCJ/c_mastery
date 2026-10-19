#include "bridge.h"
#ifndef XD_BRIDGE_MUTANT
#define XD_BRIDGE_MUTANT 0
#endif
static int compatible(const XdLease *s, XdTicket t, const PdTransfer *p,
                      PdIo io) {
  return s && p && io.configure && io.enable && io.stop && io.snapshot &&
         io.visibility && t.epoch == s->epoch &&
         t.generation == s->generation && p->token == t.generation &&
         p->total == s->length && p->phase &&
         (s->phase == XD_DEVICE || s->phase == XD_STOPPING);
}
int xd_pd_cancel(XdLease *s, XdTicket t, PdTransfer *p, PdIo io) {
  if (!compatible(s, t, p, io))
    return XD_STALE;
  int status = xd_cancel(s, t);
  if (status != XD_OK && status != XD_DUPLICATE)
    return status;
  (void)pd_cancel(p, io, t.generation);
  return status;
}
int xd_pd_finish(XdLease *s, XdTicket t, PdTransfer *p, PdIo io) {
  if (!compatible(s, t, p, io))
    return XD_STALE;
  if (pd_poll(p, io, t.generation) != PD_OK)
    return XD_BUSY;
  PdResult r;
  if (pd_finalize(p, io, t.generation, &r) != PD_OK)
    return XD_BUSY;
  /* Pd records the cancel request. Xd reports the completed byte contract.
   * An I/O error never becomes success merely because the count is zero. */
  unsigned event = r.status == PD_IO
                       ? XD_ERROR
                       : (r.complete_bytes == s->length && XD_BRIDGE_MUTANT != 1
                              ? XD_COMPLETE
                          : r.status == PD_CANCELLED ? XD_STOPPED
                                                     : XD_COMPLETE);
  XdProof proof = {t, 0, 0, 1, r.complete_bytes, r.possibly_written_bytes};
  return xd_observe(s, t, event, &proof);
}
