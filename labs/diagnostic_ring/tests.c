#include "bridge.h"
#include "ethernet_followthrough/phy/model.h"
#include "model.h"
#include <string.h>
void dr_note(const char *);
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      dr_note("DR_ASSERT:" #x "\n");                                           \
      return 41;                                                               \
    }                                                                          \
  } while (0)
static EnMemory memory;
static EnModel model;
static EpModel pm;
static EpPhy phy;
static EnRing ring;
static DgRing bridge;
static int boot(void) {
  memset(&ring, 0, sizeof ring);
  memset(&phy, 0, sizeof phy);
  memset(&bridge, 0, sizeof bridge);
  ep_model_setup(&pm, 0);
  EDBus p = ep_model_bus(&pm);
  CHECK(ep_init(&phy, &p, 168000000, 0, 4, 100) == ED_OK);
  CHECK(ep_begin(&phy, 16, 1000) == ED_BUSY);
  int rc = ED_BUSY;
  for (unsigned i = 0; i < 16 && rc == ED_BUSY; i++)
    rc = ep_step(&phy);
  CHECK(rc == ED_OK);
  en_model_init(&model, &memory, 0x20001000);
  EnBus b = en_model_bus(&model);
  unsigned char mac[6] = {2, 0, 0, 0, 0, 2};
  CHECK(en_init(&ring, &memory, 0x20001000, &b, &phy, mac) == ED_OK);
  CHECK(en_reset_begin(&ring, 0, 4, 100) == ED_BUSY);
  CHECK(en_reset_step(&ring, 1) == ED_BUSY &&
        en_reset_step(&ring, 2) == ED_BUSY &&
        en_reset_step(&ring, 3) == ED_OK && en_start(&ring) == ED_OK);
  CHECK(dr_open(&bridge, &ring) == DG_OK);
  return 0;
}
static int transmit(void) {
  CHECK(!boot());
  DGMessage m = {7, 3, {1, 2, 3}};
  CHECK(dr_send(&bridge, &m) == DG_OK && bridge.phase == 1);
  unsigned slot = bridge.ticket.slot;
  m.payload[0] = 99;
  uint32_t raw = 123;
  CHECK(dr_poll(&bridge, &raw) == DG_BUSY && bridge.phase == 2 && raw == 123);
  CHECK(dr_send(&bridge, &m) == DG_BUSY);
  CHECK(dr_poll(&bridge, &raw) == DG_BUSY && raw == 123);
  CHECK(en_model_tx(&model, 0) == ED_OK);
  CHECK(model.wire_length == 60 && model.wire[18] == 1);
  CHECK(dr_poll(&bridge, &raw) == DG_OK && !bridge.phase && raw == 0 &&
        ring.tx_phase[slot] == 0);
  CHECK(dr_poll(&bridge, &raw) == DG_STALE);
  CHECK(dr_send(&bridge, &m) == DG_OK);
  CHECK(dr_poll(&bridge, &raw) == DG_BUSY);
  CHECK(en_model_tx(&model, 1) == ED_OK);
  CHECK(dr_poll(&bridge, &raw) == DG_IO && !bridge.phase && raw == 0x8002);
  CHECK(dr_send(&bridge, &m) == DG_OK);
  model.fail_at = model.calls + 1;
  CHECK(dr_poll(&bridge, &raw) == DG_IO && bridge.phase == 3 &&
        ring.tx_phase[bridge.ticket.slot] == 2);
  CHECK(memory.tx[bridge.ticket.slot].word[0] & 0x80000000u);
  CHECK(dr_send(&bridge, &m) == DG_BUSY && dr_poll(&bridge, &raw) == DG_IO);
  CHECK(dr_open(&bridge, &ring) == DG_INVALID);
  model.fail_at = 0;
  model.stop_stuck = 1;
  CHECK(en_stop_begin(&ring, 10, 4, 100) == ED_BUSY);
  CHECK(en_stop_step(&ring, 11) == ED_BUSY && bridge.phase == 3);
  model.stop_stuck = 0;
  CHECK(en_stop_begin(&ring, 12, 4, 100) == ED_BUSY);
  CHECK(en_stop_step(&ring, 13) == ED_OK);
  CHECK(en_reset_begin(&ring, 14, 4, 100) == ED_BUSY);
  CHECK(en_reset_step(&ring, 15) == ED_BUSY &&
        en_reset_step(&ring, 16) == ED_BUSY &&
        en_reset_step(&ring, 17) == ED_OK && en_start(&ring) == ED_OK);
  CHECK(dr_poll(&bridge, &raw) == DG_STALE);
  CHECK(dr_open(&bridge, &ring) == DG_OK && !bridge.phase);
  CHECK(!model.violation);
  dr_note("DR_VALUES tx 60 1 3 2\nDR_TX_PASS\n");
  return 0;
}
static int receive(void) {
  CHECK(!boot());
  DGSession session = {0};
  DGMessage m = {1, 2, {0xaa, 0x55}}, out = {99, 1, {9}};
  unsigned char f[64] = {0};
  size_t n = 0;
  CHECK(dg_ethernet_build(&m, f, 60, &n) == DG_OK && n == 60);
  CHECK(en_model_rx(&model, f, 64, (64u << 16) | 0x300u) == ED_OK);
  CHECK(dr_receive(&bridge, &session, &out) == DG_OK && session.last == 1 &&
        out.payload[1] == 0x55 && ring.released == 1 && ring.rx_held[0] == 0);
  CHECK(memory.rx[0].word[0] == 0x80000000u);
  for (unsigned i = 0; i < 8; i++) {
    CHECK(en_model_rx(&model, f, 64, (64u << 16) | 0x300u) == ED_OK);
    DGMessage prior;
    memcpy(&prior, &out, sizeof out);
    CHECK(dr_receive(&bridge, &session, &out) == DG_STALE &&
          !memcmp(&prior, &out, sizeof out) && session.accepted == 1 &&
          ring.released == i + 2);
  }
  f[0] = 3;
  CHECK(en_model_rx(&model, f, 64, (64u << 16) | 0x300u) == ED_OK);
  CHECK(dr_receive(&bridge, &session, &out) == DG_FRAME &&
        ring.released == 10 && session.last == 1);
  f[0] = 2;
  CHECK(en_model_rx(&model, f, 64, (64u << 16) | 0x8300u) == ED_OK);
  CHECK(dr_receive(&bridge, &session, &out) == DG_IO && session.last == 1 &&
        ring.dropped == 1);
  m.sequence = 2;
  CHECK(dg_ethernet_build(&m, f, 60, &n) == DG_OK);
  CHECK(en_model_rx(&model, f, 64, (64u << 16) | 0x300u) == ED_OK);
  model.fail_at = model.calls + 1;
  DGMessage prior;
    memcpy(&prior, &out, sizeof out);
  CHECK(dr_receive(&bridge, &session, &out) == DG_IO && session.last == 1 &&
        !memcmp(&prior, &out, sizeof out));
  CHECK(ring.released == 11 && ring.phase == EN_FAULT);
  dr_note("DR_VALUES rx 1 11 1 2\nDR_RX_PASS\n");
  return 0;
}
int dr_test(unsigned n) {
  return n == 0 ? transmit() : n == 1 ? receive() : 41;
}
