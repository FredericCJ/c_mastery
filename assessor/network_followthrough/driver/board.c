#include "board.h"
static int failed(NbBoard *b) {
  b->phase = NB_BOARD_FAILED;
  return NB_FAULT;
}
int nb_board_begin(NbBoard *b, EnMemory *m, uint32_t base, EpMmioClock clock,
                   NaConfig c, uint32_t epoch, uint32_t isn,
                   unsigned capacity) {
  if (!b || !m || !clock.clock || b->phase != NB_BOARD_COLD ||
      b->target.opened || !epoch || !capacity || capacity > 512 ||
      (uintptr_t)m != base || en_range(base, sizeof *m) != ED_OK)
    return NB_INVALID;
  b->clock = clock;
  b->config = c;
  b->memory = m;
  b->base = base;
  b->epoch = epoch;
  b->isn = isn;
  b->capacity = capacity;
  if (en_target_open(&b->target) != ED_OK)
    return failed(b);
  /* Partial hardware preparation is retained for diagnosis. No automatic
   * 'cleanup' rewrites clocks that an incompletely observed engine may use. */
  if (en_target_cold(&b->target) != ED_OK)
    return failed(b);
  EDBus phy_bus;
  if (ep_f439_bus(&b->clock, &phy_bus) != ED_OK)
    return failed(b);
  if (ep_init(&b->phy, &phy_bus, 168000000, 0, 1024, 10) != ED_OK ||
      ep_begin(&b->phy, 512, 500) != ED_BUSY)
    return failed(b);
  b->start = b->last = clock.clock(clock.context);
  b->phase = NB_BOARD_PHY;
  return NB_WAIT;
}
int nb_board_step(NbBoard *b) {
  if (!b || !b->clock.clock)
    return NB_INVALID;
  if (b->phase == NB_BOARD_FAILED)
    return NB_FAULT;
  if (b->phase == NB_BOARD_READY)
    return NB_OK;
  if (b->phase != NB_BOARD_PHY && b->phase != NB_BOARD_RESET)
    return NB_INVALID;
  uint32_t now = b->clock.clock(b->clock.context);
  if (now < b->last || now - b->start >= 500)
    return failed(b);
  b->last = now;
  if (b->phase == NB_BOARD_PHY) {
    int e = ep_step(&b->phy);
    if (e == ED_BUSY)
      return NB_WAIT;
    if (e != ED_OK)
      return failed(b);
    EnBus bus = en_target_bus(&b->target);
    if (en_init(&b->ring, b->memory, b->base, &bus, &b->phy,
                b->config.local_mac) != ED_OK ||
        en_reset_begin(&b->ring, 0, 100, 100) != ED_BUSY)
      return failed(b);
    b->start = now;
    b->phase = NB_BOARD_RESET;
    return NB_WAIT;
  }
  int e = en_reset_step(&b->ring, now - b->start);
  if (e == ED_BUSY)
    return NB_WAIT;
  if (e != ED_OK || en_start(&b->ring) != ED_OK ||
      nb_init(&b->net, &b->ring, b->config, b->epoch, b->isn, b->capacity,
              now) != NB_OK)
    return failed(b);
  b->phase = NB_BOARD_READY;
  return NB_OK;
}
