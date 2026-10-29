#include "checkpoint_firmware.h"
#include "clock.h"
#include "handoff.h"
#include "lease.h"
#include "peripheral.h"
#ifdef XK_TARGET
void xk_print(const char *);
#define LOG(s) xk_print(s "\n")
#else
#include <stdio.h>
#define LOG(s) puts(s)
#endif
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      LOG("XK_INTEGRATE_FAIL " #x);                                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int xk_integration(void) {
  struct cpf_clock clock;
  uint64_t sample = 999;
  REQUIRE(cpf_clock_init(&clock));
  REQUIRE(cpf_clock_write(&clock, UINT64_C(0x7ffffffff)));
  REQUIRE(cpf_clock_read(&clock, 2, &sample) &&
          sample == UINT64_C(0x7ffffffff));
  /* A controlled partial writer state, not a call claiming atomic 64-bit
   * hardware capture: equal high words7 around low0 would yield7:0. */
  atomic_store(&clock.sequence, 3);
  atomic_store(&clock.low, 0);
  REQUIRE(!cpf_clock_read(&clock, 2, &sample) &&
          sample == UINT64_C(0x7ffffffff));
  atomic_store(&clock.high, 8);
  atomic_store(&clock.sequence, 4);
  REQUIRE(cpf_clock_read(&clock, 2, &sample) &&
          sample == UINT64_C(0x800000000));
  atomic_store(&clock.sequence, UINT32_MAX - 1);
  REQUIRE(!cpf_clock_write(&clock, 1));
  LOG("XK_SCHEDULE software-writer-paused odd-refused retire-before-wrap");
  struct xi_queue q;
  struct xi_record out;
  struct xk_sequence sequence = {0};
  REQUIRE(xi_init(&q, 4) == XI_OK);
  for (unsigned i = 0; i < 6; ++i) {
    struct xi_record r = {.sequence = i, .timestamp = 100 + i};
    int rc = xi_push(&q, &r);
    REQUIRE(rc == (i < 3 ? XI_OK : XI_FULL));
  }
  /* Dequeue acquisition does not release capacity until the consumer commits.
   * A timestamp's copy being coherent is not permission to advance ownership. */
  REQUIRE(xi_take(&q, &out) == XI_OK && out.sequence == 0);
  REQUIRE(xi_reserve(&q) == XI_FULL);
  REQUIRE(xi_release(&q) == XI_OK);
  REQUIRE(xk_sequence(&sequence, 1, out.sequence, 16) == XK_OK);
  for (unsigned i = 1; i < 3; ++i) {
    REQUIRE(xi_pop(&q, &out) == XI_OK && out.sequence == i);
    REQUIRE(xk_sequence(&sequence, 1, out.sequence, 16) == XK_OK);
  }
  struct xi_record r = {.sequence = 6, .timestamp = 200};
  REQUIRE(xi_push(&q, &r) == XI_OK && xi_pop(&q, &out) == XI_OK);
  REQUIRE(xk_sequence(&sequence, 1, out.sequence, 16) == XK_GAP &&
          sequence.missing == 3);
  LOG("XK_SCHEDULE delayed-service queue-full missing=3 "
      "stable-counter-not-completeness");
  struct xd_lease lease = {0};
  struct xd_fence fence = {0, 1, 1, 1};
  struct xd_ticket ticket;
  struct xd_result result;
  REQUIRE(xd_recover(&lease, &fence) == XD_OK &&
          xd_submit(&lease, 32, &ticket) == XD_OK);
  REQUIRE(xd_cancel(&lease, ticket) == XD_OK && lease.phase == XD_STOPPING);
  struct xd_proof proof = {ticket, 0, 0, 1, 32, 32};
  REQUIRE(xd_observe(&lease, ticket, XD_COMPLETE, &proof) == XD_OK);
  REQUIRE(xd_observe(&lease, ticket, XD_STOPPED, &proof) == XD_DUPLICATE);
  REQUIRE(xd_reclaim(&lease, ticket, &result) == XD_OK &&
          result.terminal == XD_COMPLETE);
  LOG("XK_SCHEDULE cancel-before-complete complete-before-stop "
      "terminal=complete");
  struct xd_ticket old = ticket;
  REQUIRE(xd_submit(&lease, 32, &ticket) == XD_OK);
  REQUIRE(xd_observe(&lease, old, XD_COMPLETE, &proof) == XD_STALE);
  REQUIRE(xd_cancel(&lease, ticket) == XD_OK);
  proof = (struct xd_proof){ticket, 0, 0, 1, 8, 8};
  REQUIRE(xd_observe(&lease, ticket, XD_STOPPED, &proof) == XD_OK);
  proof.complete = proof.possible = 32;
  REQUIRE(xd_observe(&lease, ticket, XD_COMPLETE, &proof) == XD_STALE);
  REQUIRE(xd_reclaim(&lease, ticket, &result) == XD_OK &&
          result.terminal == XD_STOPPED);
  LOG("XK_SCHEDULE stale-generation stop-before-complete terminal=stopped");
  REQUIRE(xd_submit(&lease, 32, &ticket) == XD_OK);
  lease = (struct xd_lease){0};
  struct xd_fence pending = {1, 2, 1, 0};
  REQUIRE(xd_recover(&lease, &pending) == XD_BUSY);
  pending.writes_drained = 1;
  REQUIRE(xd_recover(&lease, &pending) == XD_OK);
  REQUIRE(xd_submit(&lease, 32, &ticket) == XD_OK && ticket.epoch == 2);
  REQUIRE(xd_observe(&lease, old, XD_COMPLETE, &proof) == XD_STALE);
  LOG("XK_SCHEDULE software-reset-domain supervisor-fence-before-reuse");
  /* Late-return adapter after PR-T31-03: identify a degenerate calibration
   * supported by both actual APIs; no promotion of MODEL to MEASURED. */
  for (unsigned tick = 100; tick < 132; ++tick) {
    struct dp_calibration dp = {1000, 100, 10, 2, DP_MODEL};
    struct dp_correlation dc;
    struct xk_calibration xk = {100, 998, 1002, 10, 0, 7};
    struct xk_interval xc;
    REQUIRE(dp_correlate(&dp, tick, 0, UINT64_MAX, &dc) == DP_OK &&
            dc.provenance == DP_MODEL);
    REQUIRE(xk_calibrate(&xk, tick, tick, &xc) == XK_OK &&
            xc.lower == dc.earliest_ns && xc.upper == dc.latest_ns);
  }
  LOG("XK_INTEGRATE_PASS actual_Xi_Xd_CPF_Dp clock_comparisons=32");
  return 0;
}
#ifndef XK_TARGET
int main(void) { return xk_integration(); }
#endif
