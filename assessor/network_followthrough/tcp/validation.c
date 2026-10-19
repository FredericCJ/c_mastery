#include "test_wire.h"
static int close_states(void) {
  NcStack s;
  NcPacket o;
  unsigned char p[1500], d[4] = {1, 2, 3, 4};
  CHECK(!nc_establish(&s, 16, 1000, 0xfffffff0, 100));
  CHECK(nc_close(&s) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(s.state == NC_ESTABLISHED && s.snd_nxt == 1001);
  CHECK(nc_submit(&s, o.ticket, 1, 100) == NC_OK);
  CHECK(s.state == NC_FIN_WAIT_1 && s.snd_nxt == 1002);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  size_t n = nc_wire(p, s.rcv_nxt, 1001, 0x11, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 101) == NC_OK);
  CHECK(s.state == NC_CLOSING && s.rcv_nxt == 0xfffffff2);
  n = nc_wire(p, s.rcv_nxt, 1002, 16, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 102) == NC_OK);
  CHECK(s.state == NC_TIME_WAIT && s.timewait_until == 240102);
  n = nc_wire(p, s.rcv_nxt - 1, 1002, 0x11, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 1000) == NC_OK);
  CHECK(s.timewait_until == 241000);
  CHECK(nc_tick(&s, 240999) == NC_OK && s.state == NC_TIME_WAIT);
  CHECK(nc_tick(&s, 241000) == NC_OK && s.state == NC_CLOSED);
  CHECK(!nc_establish(&s, 16, 1000, 30, 100));
  CHECK(nc_close(&s) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 100) == NC_OK);
  n = nc_wire(p, 31, 1002, 16, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 101) == NC_OK);
  CHECK(s.state == NC_FIN_WAIT_2 && !s.flight && s.dma);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  n = nc_wire(p, 31, 1002, 0x11, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 102) == NC_OK && s.state == NC_TIME_WAIT);
  CHECK(!nc_establish(&s, 4, 1000, 30, 100));
  n = nc_wire(p, 31, 1001, 0x11, 512, d, 4, 0);
  CHECK(nc_receive(&s, p, n, 81) == NC_OK);
  CHECK(s.rcv_nxt == 35 && !s.peer_fin && s.rx.count == 4);
  CHECK(nc_consume(&s, 4) == NC_OK);
  n = nc_wire(p, 35, 1001, 0x11, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 82) == NC_OK);
  CHECK(s.state == NC_CLOSE_WAIT && s.rcv_nxt == 36);
  CHECK(nc_close(&s) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 83) == NC_OK);
  CHECK(s.state == NC_LAST_ACK);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  n = nc_wire(p, 36, 1002, 16, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 84) == NC_OK && s.state == NC_CLOSED);
  return 0;
}
static int timer_states(void) {
  NcStack s;
  NcPacket o;
  unsigned char p[300], data[32] = {0};
  NiConfig c = {0xc0000202, 0xc0000201};
  CHECK(nc_init(&s, c, 9, 1000, 512, 0) == NC_OK);
  size_t n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
  CHECK(nc_receive(&s, p, n, 0) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 0) == NC_OK);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  const uint64_t times[3] = {1000, 3000, 7000}, due[3] = {3000, 7000, 15000};
  for (unsigned i = 0; i < 3; i++) {
    CHECK(nc_tick(&s, times[i] - 1) == NC_OK);
    CHECK(nc_prepare(&s, &o) == NC_WAIT);
    CHECK(nc_tick(&s, times[i]) == NC_OK);
    CHECK(nc_prepare(&s, &o) == NC_OK);
    CHECK(nc_submit(&s, o.ticket, 1, times[i]) == NC_OK);
    CHECK(s.due == due[i]);
    CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  }
  CHECK(nc_tick(&s, 14999) == NC_OK);
  CHECK(nc_tick(&s, 15000) == NC_ABORT);
  CHECK(!s.flight && s.state == NC_CLOSED && !s.rx.count && !s.tx_length);
  CHECK(nc_init(&s, c, 10, 1000, 512, 0) == NC_OK);
  n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
  CHECK(nc_receive(&s, p, n, 0) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 0) == NC_OK);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  CHECK(nc_tick(&s, 1000) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 0, 8000) == NC_WAIT);
  CHECK(s.retries == 0 && s.due == 1000);
  CHECK(nc_submit(&s, o.ticket, 1, 8001) == NC_OK);
  CHECK(s.due == 10001 && s.retries == 1);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  n = nc_wire(p, 31, 1001, 16, 512, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 8010) == NC_OK);
  CHECK(s.rto == 3000 && s.rtt.samples == 0);
  CHECK(nc_offer(&s, data, 32) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 8020) == NC_OK);
  n = nc_wire(p, 31, s.snd_una + 16, 16, 8, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 8030) == NC_OK);
  CHECK(s.flight && s.due == 11030 && nc_usable(&s) == 0);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_WAIT);
  CHECK(nc_tick(&s, 11030) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, 11030) == NC_OK);
  CHECK(s.rto == 6000);
  CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
  s.rto = 40000;
  CHECK(nc_tick(&s, s.due) == NC_OK);
  CHECK(nc_prepare(&s, &o) == NC_OK);
  CHECK(nc_submit(&s, o.ticket, 1, s.now) == NC_OK);
  CHECK(s.rto == 60000);
  NcTicket old = o.ticket;
  CHECK(nc_dma_done(&s, old) == NC_OK);
  CHECK(nc_dma_done(&s, old) == NC_STALE);
  old.epoch++;
  CHECK(nc_submit(&s, old, 1, s.now) == NC_STALE);
  s.serial = UINT32_MAX;
  s.pending_valid = 0;
  s.ack_needed = 1;
  CHECK(nc_prepare(&s, &o) == NC_RANGE);
  for (unsigned timer_first = 0; timer_first < 2; timer_first++) {
    CHECK(!nc_establish(&s, 512, 20, 30, 256));
    CHECK(nc_offer(&s, data, 32) == NC_OK);
    CHECK(nc_prepare(&s, &o) == NC_OK);
    CHECK(nc_submit(&s, o.ticket, 1, 100) == NC_OK);
    CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
    CHECK(s.due == 1100);
    if (timer_first) {
      CHECK(nc_tick(&s, 1100) == NC_OK);
      CHECK(nc_prepare(&s, &o) == NC_OK);
      CHECK(nc_submit(&s, o.ticket, 1, 1100) == NC_OK);
      CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
    }
    n = nc_wire(p, s.rcv_nxt, s.snd_nxt, 16, 512, 0, 0, 0);
    CHECK(nc_receive(&s, p, n, 1100) == NC_OK);
    CHECK(nc_tick(&s, 1100) == NC_OK);
    CHECK(!s.flight && s.rtt.samples == (timer_first ? 1u : 2u));
  }
  return 0;
}
static int admission(void) {
  NcStack s, before;
  unsigned char p[1500], data[8] = {0};
  NcPacket out;
  CHECK(!nc_establish(&s, 32, 0xfffffff0, 0xfffffff0, 100));
  /* WL1 is deliberately newer than the current receive sequence: the state
   * arose from an admissible out-of-order pure ACK, which stores no bytes. */
  size_t n = nc_wire(p, 0x10, s.snd_nxt, 16, 40, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 81) == NC_WAIT);
  CHECK(s.wl1 == 0x10 && s.peer_window == 40);
  n = nc_wire(p, 0xfffffff1, s.snd_nxt, 16, 0, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 82) == NC_OK);
  CHECK(s.peer_window == 40);
  n = nc_wire(p, 0x10, s.snd_nxt + 1, 16, 0, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 83) == NC_WAIT);
  CHECK(s.peer_window == 40 && s.snd_una == 0xfffffff1);
  s.snd_una = 0xfffffff0;
  s.snd_nxt = 0x10;
  s.wl1 = 0xfffffff1;
  s.wl2 = 0xfffffff0;
  s.flight = 1;
  s.due = 1000;
  n = nc_wire(p, 0xfffffff1, 0x10, 16, 8, 0, 0, 0);
  CHECK(nc_receive(&s, p, n, 84) == NC_OK);
  CHECK(s.wl2 == 0x10 && s.peer_window == 8 && !s.flight);
  CHECK(nc_prepare(&s, &out) == NC_OK);
  CHECK(nc_submit(&s, out.ticket, 1, 85) == NC_OK);
  CHECK(nc_dma_done(&s, out.ticket) == NC_OK);
  CHECK(!nc_establish(&s, 32, 20, 30, 256));
  n = nc_wire(p, 31, 21, 0x18, 512, data, 8, 0);
  unsigned rejected = 0;
  for (size_t cut = 0; cut < n; cut++) {
    before = s;
    CHECK(nc_receive(&s, p, cut, 81) == NC_INVALID);
    CHECK(!memcmp(&s, &before, sizeof s));
    rejected++;
  }
  for (unsigned byte = 0; byte < n; byte++) {
    p[byte] ^= 1;
    before = s;
    CHECK(nc_receive(&s, p, n, 81) == NC_INVALID);
    CHECK(!memcmp(&s, &before, sizeof s));
    p[byte] ^= 1;
    rejected++;
  }
  CHECK(rejected == 96);
  /* A reset before RCV.NXT with payload overlapping the window is still an
   * outside-window reset. Do not derive reset admission from payload extent. */
  s.ack_needed = 0;
  n = nc_wire(p, 30, 21, 4, 512, data, 8, 0);
  CHECK(nc_receive(&s, p, n, 81) == NC_WAIT);
  CHECK(s.state == NC_ESTABLISHED && !s.ack_needed && s.rx.count == 0);
  for (unsigned test = 0; test < 8; test++) {
    NiConfig c = {0xc0000202, 0xc0000201};
    CHECK(nc_init(&s, c, 8, 20, 32, 0) == NC_OK);
    n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
    switch (test) {
    case 0:
      p[41] = 1;
      break;
    case 1:
      p[41] = 5;
      break;
    case 2:
      p[42] = p[43] = 0;
      break;
    case 3:
      p[32] = 0x40;
      break;
    case 4:
      p[32] = 0xf0;
      break;
    case 5:
      p[33] |= 0x20;
      break;
    case 6:
      p[44] = 1;
      n++;
      nc_w16(p + 2, (unsigned)n);
      break;
    case 7:
      nc_w16(p + 22, 8081);
      break;
    }
    nc_fix(p, n);
    before = s;
    CHECK(nc_receive(&s, p, n, 0) == NC_INVALID);
    CHECK(!memcmp(&s, &before, sizeof s));
  }
  return 0;
}
static int local_abort(void) {
  NcStack s, before;
  NcPacket packet;
  unsigned char payload[4] = {1,2,3,4};
  CHECK(!nc_establish(&s, 16, 1000, 30, 100));
  CHECK(nc_offer(&s, payload, sizeof payload) == NC_OK);
  CHECK(nc_prepare(&s, &packet) == NC_OK);
  CHECK(nc_submit(&s, packet.ticket, 1, 100) == NC_OK);
  before = s;
  CHECK(nc_abort(&s, 99) == NC_RANGE && !memcmp(&s, &before, sizeof s));
  CHECK(nc_abort(&s, 101) == NC_OK && s.state == NC_CLOSED && s.dma &&
        !s.flight && !s.pending_valid && !s.tx_length && !s.rx.count);
  CHECK(nc_abort(&s, 101) == NC_OK && s.dma);
  CHECK(nc_submit(&s, packet.ticket, 1, 101) == NC_STALE);
  CHECK(nc_dma_done(&s, packet.ticket) == NC_OK && !s.dma);
  CHECK(nc_dma_done(&s, packet.ticket) == NC_STALE);
  CHECK(!nc_establish(&s, 16, 1000, 30, 100));
  CHECK(nc_offer(&s, payload, sizeof payload) == NC_OK);
  CHECK(nc_prepare(&s, &packet) == NC_OK);
  CHECK(nc_abort(&s, 100) == NC_OK && !s.dma);
  CHECK(nc_submit(&s, packet.ticket, 1, 100) == NC_STALE);
  puts("NC_LOCAL_ABORT_PASS logical_close_preserves_DMA_owner_and_stale_send");
  return 0;
}
int main(void) {
  CHECK(!close_states());
  CHECK(!timer_states());
  CHECK(!admission());
  CHECK(!local_abort());
  puts("NC_VALIDATION_PASS closes_timers_WL_wrap_104_rejections");
  return 0;
}
