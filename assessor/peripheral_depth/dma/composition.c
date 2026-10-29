#define main public_tests
#include "../../../labs/peripheral_depth/dma/tests.c"
#undef main
#include "firmware_io.h"
struct dg_old {
  unsigned enabled, flush;
};
static uint32_t old_read(void *p, unsigned r) {
  (void)r;
  return ((struct dg_old *)p)->enabled;
}
static void old_write(void *p, unsigned r, uint32_t v) {
  (void)r;
  ((struct dg_old *)p)->enabled = v;
}
int main(void) {
  struct dg_old old = {0};
  struct fw_dma d = {0};
  struct fw_io legacy = {&old, old_read, old_write};
  CHECK(fw_dma(&d, legacy, 0, 0) == 0);
  old.flush = 1;
  CHECK(fw_dma(&d, legacy, 1, 1) == 0 && fw_dma(&d, legacy, 2, 1) == 0 &&
        old.flush);
  /* The baseline's abstract FW_DMA==0 contract must denote full quiescence.
     Binding it directly to EN violates that contract; do not alter baseline. */
  struct dg_model m = {0};
  struct pd_transfer state = {0};
  struct pd_io io = bus(&m);
  struct pd_extent e = item(PD_F439, 16);
  struct pd_result r;
  CHECK(pd_submit(&state, io, PD_F439, 0, &e, 1, 1) == PD_OK);
  CHECK(pd_cancel(&state, io, 1) == PD_BUSY);
  m.snap = (struct pd_snapshot){1, 0, 8, 0, 0};
  CHECK(pd_poll(&state, io, 1) == PD_BUSY &&
        pd_finalize(&state, io, 1, &r) == PD_BUSY);
  m.snap.flags = PD_TC;
  m.snap.fifo_empty = 1;
  CHECK(pd_poll(&state, io, 1) == PD_OK &&
        pd_finalize(&state, io, 1, &r) == PD_OK && r.complete_bytes == 8);
  puts("PD_COMPOSE_PASS");
  return 0;
}
