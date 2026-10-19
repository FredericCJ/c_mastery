#include "posted.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PL_POST_CHECK:%u\n", (unsigned)__LINE__);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  for (unsigned immediate = 0; immediate < 2; immediate++)
    for (unsigned delay = 0; delay < 4; delay++) {
      PlPosted m = {0};
      m.device.revision = 1;
      m.immediate = immediate;
      m.device.rx[0] = 42;
      m.device.rx[1] = 99;
      m.device.rx_count = 2;
      PlExchange s = {0};
      uint8_t out = 77;
      CHECK(pl_exchange_begin(&s, &m, 7, 8) == PL_OK && m.device.control == 1 &&
            !m.count);
      for (unsigned k = 0; k < delay; k++)
        CHECK(pl_exchange_step(&s, &m, &out) == PL_BUSY && out == 77 &&
              !m.device.tx_count);
      m.device.status = 5;
      CHECK(pl_exchange_step(&s, &m, &out) == PL_OK && out == 42 &&
            m.device.tx_count == 1 && m.device.last_tx == 7 &&
            m.device.rx_count == 1 && m.device.rx[0] == 99 && !m.count);
      CHECK(pl_exchange_step(&s, &m, &out) == PL_STALE &&
            m.device.rx_count == 1);
    }
  {
    PlPosted m = {0};
    m.device.revision = 1;
    m.device.status = 31;
    PlExchange s = {0};
    uint8_t out = 77;
    CHECK(pl_exchange_begin(&s, &m, 7, 3) == PL_OK);
    CHECK(pl_exchange_step(&s, &m, &out) == PL_IO && m.device.status == 29 &&
          out == 77 && !m.device.tx_count);
  }
  {
    PlPosted m = {0};
    m.device.revision = 1;
    uint32_t out = 777;
    CHECK(pl_post_write(&m, PM_CTRL, 32, 1) == PL_OK && m.device.control == 0);
    CHECK(pl_post_read(&m, PM_CTRL, 32, &out) == PL_OK && out == 0);
    CHECK(pl_flush(&m) == PL_OK && m.device.control == 1);
  }
  {
    PlPosted m = {0};
    m.device.revision = 1;
    m.device.status = 1;
    CHECK(pl_post_write(&m, PM_CTRL, 32, 1) == PL_OK &&
          pl_post_write(&m, PM_DATA, 8, 7) == PL_OK &&
          pl_post_write(&m, PM_DATA, 8, 9) == PL_OK);
    CHECK(pl_flush(&m) == PL_IO && m.device.tx_count == 1 &&
          m.device.last_tx == 7 && m.count == 1 && m.delivered == 2);
    m.device.status |= 1;
    CHECK(pl_flush(&m) == PL_OK && m.device.tx_count == 2 &&
          m.device.last_tx == 9);
  }
  /* A rejected posted publication is quarantined, never requeued by step.
   * Resolving the original queued write may still cause its irreversible
   * effect. */
  {
    PlPosted m = {0};
    m.device.revision = 1;
    PlExchange s = {0};
    uint8_t out = 77;
    CHECK(pl_exchange_begin(&s, &m, 7, 3) == PL_OK);
    CHECK(pl_post_write(&m, PM_DATA, 8, 9) == PL_OK);
    m.device.status = 1;
    CHECK(pl_exchange_step(&s, &m, &out) == PL_IO && s.phase == 4 &&
          m.device.tx_count == 1 && m.device.last_tx == 9 && m.count == 1);
    CHECK(pl_exchange_step(&s, &m, &out) == PL_STALE && m.count == 1);
    CHECK(pl_exchange_begin(&s, &m, 8, 3) == PL_BUSY);
    m.device.status = 1;
    CHECK(pl_flush(&m) == PL_OK && m.device.tx_count == 2 &&
          m.device.last_tx == 7);
    CHECK(pl_exchange_begin(&s, &m, 8, 3) == PL_OK);
  }
  puts("PL_POST_PASS");
  return 0;
}
