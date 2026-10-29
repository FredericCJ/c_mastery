#include "frames.h"
#include <linux/can/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CAN-CHECK:%d:%s\n", __LINE__, #x);                      \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
static void decode(void) {
  struct can_frame raw = {.can_id = 0x123, .len = 2, .data = {0x12, 0x34}};
  unsigned char bytes[CANFD_MTU + 1] = {0};
  memcpy(bytes + 1, &raw, sizeof raw);
  struct ca_frame out;
  memset(&out, 0x55, sizeof out);
  unsigned char before[sizeof out];
  memcpy(before, &out, sizeof out);
  for (size_t n = 0; n <= CANFD_MTU; ++n)
    if (n != CAN_MTU) {
      CHECK(ca_decode(bytes + 1, n, 0, &out) == -1 &&
            !memcmp(&out, before, sizeof out));
    }
  CHECK(ca_decode(bytes + 1, CAN_MTU, MSG_TRUNC, &out) == -1);
  CHECK(ca_decode(bytes + 1, CAN_MTU, 0, &out) == 0 && out.kind == CA_DATA &&
        out.id == 0x123 && out.data[0] == 0x12 && out.data[1] == 0x34);
  for (unsigned n = 0; n <= 255; ++n) {
    raw.len = (unsigned char)n;
    int result = ca_decode(&raw, sizeof raw, 0, &out);
    CHECK(result == (n <= 8 ? 0 : -1));
  }
  raw.len = 8;
  for (unsigned n = 0; n <= 255; ++n) {
    raw.len8_dlc = (unsigned char)n;
    CHECK(ca_decode(&raw, sizeof raw, 0, &out) ==
          ((n == 0 || (n >= 9 && n <= 15)) ? 0 : -1));
  }
  raw.len8_dlc = 0;
  raw.can_id = CAN_RTR_FLAG | 0x123;
  CHECK(ca_decode(&raw, sizeof raw, MSG_DONTROUTE | MSG_CONFIRM, &out) == 0 &&
        out.kind == CA_REMOTE && out.length == 8 && out.local && out.own);
  for (unsigned i = 0; i < 8; ++i)
    CHECK(out.data[i] == 0);
  raw.can_id = CAN_ERR_FLAG | CAN_ERR_BUSOFF;
  CHECK(ca_decode(&raw, sizeof raw, 0, &out) == 0 && out.kind == CA_ERROR &&
        out.id == CAN_ERR_BUSOFF);
  struct can_filter f;
  CHECK(ca_filter(0x123, 0, 0, &f) == 0);
  CHECK(ca_accept(&out, &f, 0) == 0 &&
        ca_accept(&out, &f, CAN_ERR_BUSOFF) == 1);
  CHECK(ca_encode(&out, &raw) == -1);
  raw.can_id = CAN_ERR_FLAG | CAN_EFF_FLAG;
  CHECK(ca_decode(&raw, sizeof raw, 0, &out) == -1);
  raw.can_id = 0x800;
  CHECK(ca_decode(&raw, sizeof raw, 0, &out) == -1);
}
static void roundtrip(void) {
  unsigned cases = 0;
  for (unsigned e = 0; e < 2; ++e)
    for (unsigned r = 0; r < 2; ++r)
      for (unsigned n = 0; n <= 8; ++n) {
        struct ca_frame f = {.kind = r ? CA_REMOTE : CA_DATA,
                     .extended = e,
                     .id = e ? CAN_EFF_MASK : CAN_SFF_MASK,
                     .length = n},
                out;
        for (unsigned i = 0; i < 8; ++i)
          f.data[i] = (unsigned char)(i * 31 + n);
        struct can_frame raw;
        CHECK(ca_encode(&f, &raw) == 0);
        CHECK(ca_decode(&raw, sizeof raw, 0, &out) == 0 && out.id == f.id &&
              out.extended == e && out.length == n && out.kind == f.kind);
        for (unsigned i = 0; i < 8; ++i)
          CHECK(out.data[i] == (!r && i < n ? f.data[i] : 0));
        ++cases;
      }
  CHECK(cases == 36);
}
static void filters(void) {
  struct can_filter exact;
  CHECK(ca_filter(0x123, 0, 0, &exact) == 0);
  for (unsigned id = 0; id < 2048; ++id)
    for (unsigned e = 0; e < 2; ++e)
      for (unsigned r = 0; r < 2; ++r) {
        struct ca_frame f = {.kind = r ? CA_REMOTE : CA_DATA, .id = id, .extended = e};
        CHECK(ca_accept(&f, &exact, 0) == (id == 0x123 && !e && !r));
      }
  struct ca_frame extended = {.kind = CA_DATA, .id = 0x1abc0123, .extended = 1};
  struct can_filter broad = {0x123, CAN_SFF_MASK};
  CHECK(ca_accept(&extended, &broad, 0) == 1 &&
        ca_accept(&extended, &exact, 0) == 0);
  for (unsigned sender = 0; sender < 2; ++sender)
    for (unsigned receiver = 0; receiver < 2; ++receiver)
      for (unsigned enabled = 0; enabled < 2; ++enabled)
        for (unsigned own = 0; own < 2; ++own) {
          struct ca_frame f = {.kind = CA_DATA, .id = 0x123}, out = {.id = 99};
          int want = enabled && (sender != receiver || own);
          CHECK(ca_loopback(sender, receiver, enabled, own, &f, &exact, 0,
                            &out) == want);
          CHECK(want ? out.local && out.own == (sender == receiver)
                     : out.id == 99);
        }
}
int main(void) {
  decode();
  roundtrip();
  filters();
  puts("CAN-PASS:lengths=256;raw_dlc=256;roundtrips=36;filter_cells=8192;"
       "loopback_model_cells=16;physical=0;");
}
