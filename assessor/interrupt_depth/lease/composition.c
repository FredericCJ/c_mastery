#include "bridge.h"
#include "h745_cache.h"
#ifdef XD_TARGET
void xd_print(const char *);
#define LOG(s) xd_print(s "\n")
#else
#include <stdio.h>
#define LOG(s) puts(s)
#endif
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      LOG("XD_COMPOSE_FAIL " #x);                                              \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#ifndef XD_ENGINE_MUTANT
#define XD_ENGINE_MUTANT 0
#endif
typedef struct {
  PdSnapshot snap;
  uint32_t memory[64], cache[64], valid[2], dirty[2];
  uint32_t cached, base, barriers, invalidations, stop_requests, pending;
} Engine;
static void line(void *p, enum FWCacheOp op, uint32_t address) {
  Engine *e = p;
  if (op == FW_DSB || op == FW_ISB) {
    ++e->barriers;
    return;
  }
  unsigned n = (address - e->base) / 32;
  if (op == FW_CLEAN && e->dirty[n]) {
    for (unsigned i = 32 * n; i < 32 * n + 32; ++i)
      e->memory[i] = e->cache[i];
    e->dirty[n] = 0;
  }
  if (op == H7_INVALIDATE) {
    e->valid[n] = e->dirty[n] = 0;
    ++e->invalidations;
  }
}
static unsigned read_cpu(Engine *e, unsigned offset) {
  unsigned n = offset / 32;
  if (!e->cached)
    return e->memory[offset];
  if (!e->valid[n]) {
    for (unsigned i = 32 * n; i < 32 * n + 32; ++i)
      e->cache[i] = e->memory[i];
    e->valid[n] = 1;
  }
  return e->cache[offset];
}
static void write_cpu(Engine *e, unsigned offset, unsigned value) {
  if (!e->cached) {
    e->memory[offset] = value;
    return;
  }
  (void)read_cpu(e, offset);
  e->cache[offset] = value;
  e->dirty[offset / 32] = 1;
}
static int configure(void *p, const PdExtent *extent, uint32_t token) {
  Engine *e = p;
  if (e->snap.enabled || e->pending)
    return 0;
  e->snap = (PdSnapshot){token, 0, extent->bytes, 0, 1};
  return 1;
}
static void enable(void *p) { ((Engine *)p)->snap.enabled = 1; }
static void stop(void *p) {
  Engine *e = p;
  e->snap.enabled = 0;
  ++e->stop_requests;
}
static PdSnapshot snapshot(void *p) { return ((Engine *)p)->snap; }
static void visibility(void *p, unsigned action, uint32_t address,
                       uint32_t bytes) {
  Engine *e = p;
  if (action == PD_CLEAN || action == PD_INVALIDATE) {
    (void)fw_cache_range(
        address, bytes, action == PD_CLEAN ? FW_CLEAN : H7_INVALIDATE, line, e);
  } else
    ++e->barriers;
}
static PdIo io_for(Engine *e) {
  return (PdIo){e, configure, enable, stop, snapshot, visibility};
}
static int ordinary(unsigned cached, unsigned partial, unsigned error) {
  Engine e = {0};
  e.cached = cached;
  e.base = cached ? 0x30040040u : 0x20000000u;
  PdTransfer pd = {0};
  XdLease lease = {0};
  XdFence fence = {0, 1, 1, 1};
  XdTicket ticket;
  PdExtent extent = {e.base, e.base + 32, 32, 32, 32, 1};
  PdIo io = io_for(&e);
  XdResult result = {0};
  REQUIRE(xd_recover(&lease, &fence) == XD_OK);
  write_cpu(&e, 0, 42);
  write_cpu(&e, 32, 9);
  write_cpu(&e, 63, 55);
  REQUIRE(xd_submit(&lease, 32, &ticket) == XD_OK);
  REQUIRE(pd_submit(&pd, io, cached ? PD_H745 : PD_F439, cached, &extent, 1,
                    ticket.generation) == PD_OK);
  REQUIRE(e.memory[0] == 42 && e.memory[63] == 55);
  /* Models speculative refill only. Application CPU reads remain forbidden. */
  REQUIRE(read_cpu(&e, 32) == 9);
  e.pending = 1;
  e.snap.fifo_empty = 0;
  REQUIRE(xd_pd_cancel(&lease, ticket, &pd, io) == XD_OK);
  REQUIRE(!e.snap.enabled && e.pending && lease.phase == XD_STOPPING);
  REQUIRE(xd_pd_finish(&lease, ticket, &pd, io) == XD_BUSY);
  REQUIRE(xd_reclaim(&lease, ticket, &result) == XD_BUSY);
  /* Delayed device write after EN cleared, before FIFO-empty/TC observation. */
  e.memory[32] = e.memory[0];
  e.pending = 0;
  e.snap.fifo_empty = 1;
  e.snap.remaining = partial ? 24 : 0;
  e.snap.flags = PD_TC | (error ? PD_TE : 0);
  REQUIRE(xd_pd_finish(&lease, ticket, &pd, io) == XD_OK);
  REQUIRE(xd_reclaim(&lease, ticket, &result) == XD_OK);
  REQUIRE(read_cpu(&e, 32) == 42 && read_cpu(&e, 63) == 55);
  REQUIRE(result.terminal == (error     ? XD_ERROR
                              : partial ? XD_STOPPED
                                        : XD_COMPLETE));
  REQUIRE(result.complete == (error ? 0 : partial ? 8 : 32));
  REQUIRE(result.possible == (error ? 32 : partial ? 8 : 32));
  REQUIRE(pd.result == (error ? PD_IO : PD_CANCELLED));
  if (cached)
    REQUIRE(e.invalidations == 2 && e.barriers == 15);
  else
    REQUIRE(!e.invalidations && e.barriers == 3);
  /* Reuse does not require delivery of every old metadata callback. */
  XdTicket old = ticket;
  REQUIRE(xd_submit(&lease, 32, &ticket) == XD_OK);
  PdTransfer before = pd;
  REQUIRE(xd_pd_finish(&lease, old, &pd, io) == XD_STALE);
  REQUIRE(pd.phase == before.phase && lease.phase == XD_DEVICE);
  XdProof stale = {old, 0, 0, 1, 32, 32};
  REQUIRE(xd_observe(&lease, old, XD_COMPLETE, &stale) == XD_STALE);
  return 0;
}
static int reset_domain(void) {
  /* Supervisor + engine survive. Only the software lease is zeroed. */
  Engine engine = {0};
  XdLease software = {0};
  XdFence fence = {0, 1, 1, 1};
  XdTicket old;
  uint32_t supervisor_epoch = 1;
  REQUIRE(xd_recover(&software, &fence) == XD_OK);
  REQUIRE(xd_submit(&software, 32, &old) == XD_OK);
  engine.snap.enabled = 1;
  engine.pending = 1;
  engine.memory[32] = 7;
  software = (XdLease){0};
  XdTicket unused;
  REQUIRE(xd_submit(&software, 32, &unused) == XD_BUSY);
  stop(&engine); /* EN clears; the pending write remains real in this model. */
  fence = (XdFence){supervisor_epoch, supervisor_epoch + 1, 1,
                    XD_ENGINE_MUTANT == 1 ? 1u : !engine.pending};
  REQUIRE(xd_recover(&software, &fence) == XD_BUSY);
  REQUIRE(software.phase == XD_QUARANTINE && engine.memory[32] == 7);
  engine.memory[32] = 42;
  engine.pending = 0;
  fence.writes_drained = 1;
  REQUIRE(xd_recover(&software, &fence) == XD_OK);
  supervisor_epoch = fence.new_epoch;
  REQUIRE(xd_submit(&software, 32, &unused) == XD_OK &&
          unused.epoch == supervisor_epoch);
  XdProof stale = {old, 0, 0, 1, 32, 32};
  REQUIRE(xd_observe(&software, old, XD_COMPLETE, &stale) == XD_STALE);
  /* Deliberately defective local teaching counterexample: a fabricated fence
   * cannot prevent a future engine write. No external address is touched. */
  engine.pending = 1;
  engine.memory[32] = 99;
  engine.memory[32] = 42;
  engine.pending = 0;
  REQUIRE(engine.memory[32] != 99);
  return 0;
}
static int shared(void) {
  FWPeer peer = {.generation = 7};
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_BEGIN) == H7_OK);
  REQUIRE(fw_peer_step(&peer, 1, 7, FW_BEGIN) == H7_OK);
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_END) == H7_OK);
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_HEALTHY) == H7_OK);
  REQUIRE(fw_peer_step(&peer, 1, 7, FW_HEALTHY) == H7_BUSY);
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_CONFIRM) == H7_BUSY);
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_FAIL_PEER) == H7_OK);
  REQUIRE(peer.outstanding[1] == 1 &&
          fw_peer_step(&peer, 0, 7, FW_CONFIRM) == H7_FAILED);
  REQUIRE(fw_peer_step(&peer, 1, 8, FW_END) == H7_STALE);
  REQUIRE(fw_peer_step(&peer, 1, 7, FW_END) == H7_OK);
  /* A peer failure is not quiescence, even after its software count drains. */
  REQUIRE(fw_peer_step(&peer, 0, 7, FW_CONFIRM) == H7_FAILED);
  Engine e = {.cached = 1, .base = 0x30040040u};
  write_cpu(&e, 31, 77);
  e.memory[0] = 42;
  REQUIRE(fw_cache_range(e.base, 32, H7_INVALIDATE, line, &e));
  REQUIRE(read_cpu(&e, 31) ==
          0); /* dirty neighbour lost: whole-line owner required */
  uint32_t total = 999;
  PdExtent bad = {e.base, e.base + 33, 1, 32, 1, 1};
  REQUIRE(pd_validate(PD_H745, 1, &bad, 1, &total) == PD_INVALID &&
          total == 999);
  REQUIRE(!fw_cache_range(e.base + 1, 32, FW_CLEAN, line, &e));
  return 0;
}
int xd_composition(void) {
  for (unsigned cached = 0; cached < 2; ++cached)
    for (unsigned partial = 0; partial < 2; ++partial)
      for (unsigned error = 0; error < 2; ++error)
        if (ordinary(cached, partial, error))
          return 1;
  if (reset_domain() || shared())
    return 1;
  LOG("XD_COMPOSE_PASS scenarios=8 reset_domains peer_failure dirty_neighbour");
  return 0;
}
#ifndef XD_TARGET
int main(void) { return xd_composition(); }
#endif
