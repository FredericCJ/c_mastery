#include "dma.h"
#include "h745_cache.h"
#include <stdio.h>
#ifndef PD_PEER_M7
#define PD_PEER_M7 1
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PD_PORT_CHECK:%u\n", (unsigned)__LINE__);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static struct pd_notice ram, local;
static unsigned locked, mask = 1, invalid, cleans, barriers, cache_calls, bad,
                        payload, force_busy;
uint32_t pd_port_mask(void) {
  unsigned old = mask;
  mask = 1;
  return old;
}
void pd_port_restore(uint32_t v) { mask = v; }
void pd_port_barrier(void) { barriers++; }
uint32_t pd_port_read(uint32_t a) {
  if (a != 0x58026484u || !mask || locked)
    bad++;
  if (force_busy)
    return 0x80000100u + ((PD_PEER_M7 ? 3u : 1u) == 1 ? 512 : 0);
  locked = 1;
  return 0x80000000u | ((PD_PEER_M7 ? 3u : 1u) << 8);
}
void pd_port_write(uint32_t a, uint32_t v) {
  if (a != 0x58026404u || v != ((PD_PEER_M7 ? 3u : 1u) << 8) || !locked ||
      !mask)
    bad++;
  locked = 0;
}
struct pd_notice *pd_port_memory(void) {
  if (!locked || !mask)
    bad++;
  return &local;
}
int fw_h745_cache(uint32_t a, uint32_t n, enum fw_cache_op op) {
  cache_calls++;
  if (!locked || !mask)
    bad++;
  if (a == 0x30040020u && n == 32) {
    if (op == H7_INVALIDATE) {
      local = ram;
      invalid++;
    } else if (op == FW_CLEAN) {
      ram = local;
      cleans++;
    } else
      bad++;
  } else if (a == 0x30040100u && n == 32 && op == H7_INVALIDATE)
    payload++;
  else
    bad++;
  return 0;
}
int main(void) {
  CHECK(pd_h745_notice_cold_start(0) == PD_INVALID && !cache_calls);
  CHECK(pd_h745_notice_cold_start(9) == PD_OK && ram.epoch == 9 && mask == 1 &&
        !locked);
  struct pd_notice m = {9, 1, 1, PD_OK, 32, 32, 0x30040100, 32}, out = {0};
  mask = 0;
  force_busy = 1;
  unsigned before = cache_calls;
  CHECK(pd_h745_notice(PD_NOTICE_PUBLISH, 9, 1, &m, 0) == PD_BUSY &&
        cache_calls == before && mask == 0 && !locked);
  force_busy = 0;
  local.epoch = 77;
  CHECK(pd_h745_notice(PD_NOTICE_PUBLISH, 9, 1, &m, 0) == PD_OK &&
        ram.state == 1 && mask == 0 && !locked);
  local.state = 0;
  CHECK(pd_h745_notice(PD_NOTICE_TAKE, 9, 1, 0, &out) == PD_OK &&
        out.complete == 32 && ram.state == 2 && payload == 1);
  CHECK(pd_h745_notice(PD_NOTICE_QUERY, 9, 1, 0, 0) == PD_BUSY);
  CHECK(pd_h745_notice(PD_NOTICE_ACK, 9, 1, 0, 0) == PD_OK && ram.state == 3);
  CHECK(pd_h745_notice(PD_NOTICE_QUERY, 9, 1, 0, 0) == PD_OK && !bad &&
        invalid == 6 && cleans == 6 && barriers == 18);
  puts("PD_PORT_PASS");
  return 0;
}
