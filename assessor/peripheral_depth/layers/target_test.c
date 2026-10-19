/* Actual prior target register adapter, simulated address map, original driver.
 */
#define main prior_target_tests
#include "../uart/target_test.c"
#undef main
#include "target.h"
int main(void) {
  R(0x40023800) = 2;
  R(0x40023808) = 0;
  PuClock clock = {16000000, 1, 1, 1, 1, 0};
  for (unsigned backend = 0; backend < 2; backend++) {
    PlUart uart = {0};
    CHECK(pl_f439_open(&uart, &clock, 115200, backend, 2) == PL_OK);
    PlService service = {0};
    PlTransport t = pl_f439_transport(&uart);
    PlResult result = {99, 99, 99};
    CHECK(pl_send(&service, t, 4, 5, 1, 0, 100) == PL_OK);
    PlParser parser = {0};
    PlCommand command = {99, 99};
    for (unsigned i = 0; i < 8; i++) {
      u.sr = PU_TXE;
      CHECK(pl_poll(&service, t, 1, i + 1, &result) == PL_BUSY);
      CHECK(pl_feed(&parser, (uint8_t)u.dr, &command) ==
            (i == 7 ? PL_OK : PL_BUSY));
    }
    CHECK(command.sequence == 4 && command.value == 5);
    u.sr = PU_TC;
    CHECK(pl_poll(&service, t, 1, 9, &result) == PL_OK &&
          result.status == PL_OK && !service.active);
  }
  {
    PlUart uart = {0};
    CHECK(pl_f439_open(&uart, &clock, 115200, PU_DMA, 2) == PL_OK);
    PlTransport t = pl_f439_transport(&uart);
    for (unsigned i = 0; i < 3; i++) {
      uintptr_t bad[] = {0x10000000u, 0x2002fff9u, 0x20030000u};
      CHECK(t.submit(t.context, (const uint8_t *)bad[i], 8, 1, 0, 100) ==
                PL_INVALID &&
            !uart.driver.active);
    }
    /* Synthetic numerical SRAM pointer is never dereferenced on this host. */
    CHECK(t.submit(t.context, (const uint8_t *)(uintptr_t)0x20000100u, 8, 1, 0,
                   100) == PL_OK);
    CHECK(R(0x40026064) == 0x20000100u && R(0x4002605c) == 8);
    CHECK(t.cancel(t.context, 1) == PL_OK && uart.driver.memory_owned);
    R(0x40026058) = 0;
    u.sr = PU_TC;
    CHECK(t.poll(t.context, 1, 1) == PL_CANCELLED &&
          !uart.driver.memory_owned && !uart.driver.line_owned);
  }
  puts("PL_TARGET_PASS");
  return 0;
}
