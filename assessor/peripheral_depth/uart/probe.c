#define main contract_main
#include "../../../labs/peripheral_depth/uart/tests.c"
#undef main
#include "serial_depth.h"
int main(void) {
  unsigned ahb[] = {1, 2, 4, 8, 16, 64, 128, 256, 512},
           apb[] = {1, 2, 4, 8, 16};
  uint32_t sys[] = {16000000, 168000000, 180000000};
  for (unsigned s = 0; s < 3; s++)
    for (unsigned h = 0; h < 9; h++)
      for (unsigned a = 0; a < 5; a++)
        for (unsigned b = 0; b < 5; b++)
          for (unsigned t = 0; t < 2; t++) {
            PuClock c = {sys[s], 1, ahb[h], apb[a], apb[b], t};
            PuRates r = {101, 102, 103, 104, 105};
            int rc = pu_rates(&c, &r);
            printf("C %u %u %u %u %u %d %u %u %u %u %u\n", c.sys_hz, c.ahb,
                   c.apb1, c.apb2, t, rc, r.core, r.apb1, r.apb2, r.timer1,
                   r.timer2);
          }
  for (unsigned clock = 1; clock <= 255; clock++)
    for (unsigned baud = 1; baud <= 32; baud++)
      for (unsigned over = 0; over < 2; over++) {
        SDUart out = {0};
        int rc = sd_uart(clock, baud, over, 8, 0, 1, 0, &out);
        printf("B %u %u %u %d %u %u\n", clock, baud, over, rc, out.divisor,
               out.brr);
      }
  for (unsigned code = 0; code < 4096; code++) {
    unsigned x = code;
    PuRx q = {0};
    PuByte v = {0};
    q.backpressure = 1;
    q.reserve = 1;
    for (unsigned i = 0; i < 6; i++) {
      unsigned event = x % 4;
      x /= 4;
      if (event < 2)
        (void)pu_receive(&q, PU_RXNE | (event ? PU_ORE : 0), event ? 255 : 0);
      else if (event == 2)
        (void)pu_take(&q, &v);
      else
        (void)pu_receive(&q, 0, 99);
    }
    unsigned ready;
    pu_peer_ready(&q, &ready);
    printf("Q %u %u %u %u %u", code, q.count, q.dropped, q.errors, ready);
    while (pu_take(&q, &v) == PU_OK)
      printf(" %u %u", v.value, v.error);
    puts("");
  }
  for (unsigned code = 0; code < 7776; code++) {
    Device m = {0};
    m.source_after = 1;
    m.sr = PU_TXE | PU_TC;
    PuIo io = {&m, rd, wr};
    PuDriver d = {0};
    PuClock c = {16000000, 1, 1, 1, 1, 0};
    uint8_t data[3] = {0, 65, 255};
    pu_open(&d, io, &c, 115200, 0, 1);
    pu_submit(&d, io, 1, 1, PU_DMA, data, 3, 0, 3);
    unsigned x = code;
    for (unsigned i = 1; i <= 5; i++) {
      unsigned event = x % 6;
      x /= 6;
      if (event == 1) {
        m.dma_busy = 0;
        m.dma_count = 0;
        m.dma_flags = 1;
      } else if (event == 2)
        m.sr |= PU_TC;
      else if (event == 3)
        (void)pu_cancel(&d, io, 1);
      else if (event == 4)
        (void)pu_reset_notice(&d, io);
      else if (event == 5)
        m.dma_flags |= 2;
      int rc = pu_step(&d, io, 1, i);
      printf("D %u %u %d %u %u %u %u %u %u\n", code, i, rc, d.active,
             d.memory_owned, d.line_owned, d.cancel, d.result, d.ready);
    }
  }
  for (unsigned clock = 8000000; clock <= 16000000; clock *= 2)
    for (unsigned n = 1; n <= 8; n++)
      for (unsigned div = 8; div <= 65535; div = div == 8     ? 139
                                                 : div == 139 ? 65535
                                                              : 65536) {
        for (unsigned us = 0; us < 40000; us += 31) {
          unsigned fits = 99;
          int rc = pu_deadline_fit(clock, div, n, us, &fits);
          printf("F %u %u %u %u %d %u\n", clock, n, div, us, rc, fits);
        }
      }
  return 0;
}
